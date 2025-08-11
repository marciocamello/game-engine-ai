#include "Core/ModuleConfigLoader.h"
#include "Core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>

using json = nlohmann::json;

namespace GameEngine {

    std::optional<EngineConfig> ModuleConfigLoader::LoadFromFile(const std::string& filePath) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open configuration file: " + filePath);
                return std::nullopt;
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();

            return LoadFromString(content);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while loading config file '" + filePath + "': " + e.what());
            return std::nullopt;
        }
    }

    std::optional<EngineConfig> ModuleConfigLoader::LoadFromString(const std::string& jsonString) {
        try {
            json configJson = json::parse(jsonString);
            return ParseEngineConfig(configJson);
        }
        catch (const json::parse_error& e) {
            LOG_ERROR("JSON parse error: " + std::string(e.what()));
            return std::nullopt;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while parsing config string: " + std::string(e.what()));
            return std::nullopt;
        }
    }

    bool ModuleConfigLoader::SaveToFile(const EngineConfig& config, const std::string& filePath) {
        try {
            std::string jsonString = SaveToString(config);
            
            std::ofstream file(filePath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for writing: " + filePath);
                return false;
            }

            file << jsonString;
            file.close();
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while saving config to file '" + filePath + "': " + e.what());
            return false;
        }
    }

    std::string ModuleConfigLoader::SaveToString(const EngineConfig& config) {
        try {
            json configJson = SerializeEngineConfig(config);
            return configJson.dump(4); // Pretty print with 4 spaces
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while serializing config: " + std::string(e.what()));
            return "";
        }
    }

    ConfigValidationResult ModuleConfigLoader::ValidateConfig(const EngineConfig& config) {
        ConfigValidationResult result;

        // Validate engine version
        if (config.engineVersion.empty()) {
            result.errorType = ConfigError::MissingRequiredField;
            result.errorMessage = "Engine version is required";
            result.fieldName = "engineVersion";
            return result;
        }

        if (!IsValidVersion(config.engineVersion)) {
            result.errorType = ConfigError::InvalidVersion;
            result.errorMessage = "Invalid engine version format: " + config.engineVersion;
            result.fieldName = "engineVersion";
            return result;
        }

        // Validate config version
        if (config.configVersion.empty()) {
            result.errorType = ConfigError::MissingRequiredField;
            result.errorMessage = "Config version is required";
            result.fieldName = "configVersion";
            return result;
        }

        if (!IsValidVersion(config.configVersion)) {
            result.errorType = ConfigError::InvalidVersion;
            result.errorMessage = "Invalid config version format: " + config.configVersion;
            result.fieldName = "configVersion";
            return result;
        }

        // Validate modules
        std::unordered_set<std::string> moduleNames;
        for (const auto& moduleConfig : config.modules) {
            // Check for duplicate module names
            if (moduleNames.find(moduleConfig.name) != moduleNames.end()) {
                result.errorType = ConfigError::DuplicateModule;
                result.errorMessage = "Duplicate module name: " + moduleConfig.name;
                result.fieldName = "modules." + moduleConfig.name;
                return result;
            }
            moduleNames.insert(moduleConfig.name);

            // Validate individual module config
            ConfigValidationResult moduleResult = ValidateModuleConfig(moduleConfig);
            if (!moduleResult.isValid) {
                moduleResult.fieldName = "modules." + moduleConfig.name + "." + moduleResult.fieldName;
                return moduleResult;
            }
        }

        result.isValid = true;
        return result;
    }

    ConfigValidationResult ModuleConfigLoader::ValidateModuleConfig(const ModuleConfig& moduleConfig) {
        ConfigValidationResult result;

        // Validate module name
        if (moduleConfig.name.empty()) {
            result.errorType = ConfigError::MissingRequiredField;
            result.errorMessage = "Module name is required";
            result.fieldName = "name";
            return result;
        }

        if (!IsValidModuleName(moduleConfig.name)) {
            result.errorType = ConfigError::InvalidModuleName;
            result.errorMessage = "Invalid module name format: " + moduleConfig.name;
            result.fieldName = "name";
            return result;
        }

        // Validate version
        if (moduleConfig.version.empty()) {
            result.errorType = ConfigError::MissingRequiredField;
            result.errorMessage = "Module version is required";
            result.fieldName = "version";
            return result;
        }

        if (!IsValidVersion(moduleConfig.version)) {
            result.errorType = ConfigError::InvalidVersion;
            result.errorMessage = "Invalid version format: " + moduleConfig.version;
            result.fieldName = "version";
            return result;
        }

        // Validate parameters
        for (const auto& param : moduleConfig.parameters) {
            if (!IsValidParameterName(param.first)) {
                result.errorType = ConfigError::UnknownParameter;
                result.errorMessage = "Invalid parameter name: " + param.first;
                result.fieldName = "parameters." + param.first;
                return result;
            }

            if (!IsValidParameterValue(param.second)) {
                result.errorType = ConfigError::InvalidFieldType;
                result.errorMessage = "Invalid parameter value for: " + param.first;
                result.fieldName = "parameters." + param.first;
                return result;
            }
        }

        result.isValid = true;
        return result;
    }

    std::string ModuleConfigLoader::GetErrorMessage(ConfigError error) {
        switch (error) {
            case ConfigError::FileNotFound:
                return "Configuration file not found";
            case ConfigError::InvalidJson:
                return "Invalid JSON format";
            case ConfigError::MissingRequiredField:
                return "Missing required field";
            case ConfigError::InvalidFieldType:
                return "Invalid field type";
            case ConfigError::InvalidModuleName:
                return "Invalid module name";
            case ConfigError::InvalidVersion:
                return "Invalid version format";
            case ConfigError::DuplicateModule:
                return "Duplicate module definition";
            case ConfigError::UnknownParameter:
                return "Unknown parameter";
            default:
                return "Unknown configuration error";
        }
    }

    std::string ModuleConfigLoader::GetDetailedErrorMessage(const ConfigValidationResult& result) {
        std::stringstream ss;
        ss << GetErrorMessage(result.errorType);
        
        if (!result.fieldName.empty()) {
            ss << " in field '" << result.fieldName << "'";
        }
        
        if (result.lineNumber > 0) {
            ss << " at line " << result.lineNumber;
        }
        
        if (!result.errorMessage.empty()) {
            ss << ": " << result.errorMessage;
        }
        
        return ss.str();
    }

    EngineConfig ModuleConfigLoader::CreateDefaultConfig() {
        EngineConfig config;
        config.engineVersion = "1.0.0";
        config.configVersion = "1.0.0";
        
        // Add default core module
        ModuleConfig coreModule;
        coreModule.name = "Core";
        coreModule.version = "1.0.0";
        coreModule.enabled = true;
        config.modules.push_back(coreModule);
        
        return config;
    }

    ModuleConfig ModuleConfigLoader::CreateDefaultModuleConfig(const std::string& name, ModuleType type) {
        ModuleConfig config;
        config.name = name;
        config.version = "1.0.0";
        config.enabled = true;
        
        // Add type-specific default parameters
        switch (type) {
            case ModuleType::Graphics:
                config.parameters["renderer"] = "OpenGL";
                config.parameters["vsync"] = "true";
                break;
            case ModuleType::Physics:
                config.parameters["engine"] = "Bullet";
                config.parameters["gravity"] = "-9.81";
                break;
            case ModuleType::Audio:
                config.parameters["backend"] = "OpenAL";
                config.parameters["maxSources"] = "32";
                break;
            default:
                break;
        }
        
        return config;
    }

    bool ModuleConfigLoader::IsValidModuleName(const std::string& name) {
        if (name.empty() || name.length() > 64) {
            return false;
        }
        
        // Module name should start with letter and contain only alphanumeric characters and underscores
        std::regex namePattern("^[A-Za-z][A-Za-z0-9_]*$");
        return std::regex_match(name, namePattern);
    }

    bool ModuleConfigLoader::IsValidVersion(const std::string& version) {
        if (version.empty()) {
            return false;
        }
        
        // Version should follow semantic versioning pattern (major.minor.patch)
        std::regex versionPattern("^\\d+\\.\\d+\\.\\d+$");
        return std::regex_match(version, versionPattern);
    }

    bool ModuleConfigLoader::IsValidParameterName(const std::string& paramName) {
        if (paramName.empty() || paramName.length() > 32) {
            return false;
        }
        
        // Parameter name should contain only alphanumeric characters and underscores
        std::regex paramPattern("^[A-Za-z0-9_]+$");
        return std::regex_match(paramName, paramPattern);
    }

    bool ModuleConfigLoader::IsValidParameterValue(const std::string& paramValue) {
        // Parameter values can be any non-empty string up to 256 characters
        return !paramValue.empty() && paramValue.length() <= 256;
    }

    std::optional<ModuleConfig> ModuleConfigLoader::ParseModuleConfig(const json& moduleJson) {
        try {
            ModuleConfig config;
            
            // Required fields
            if (!moduleJson.contains("name") || !moduleJson["name"].is_string()) {
                LOG_ERROR("Module config missing required 'name' field");
                return std::nullopt;
            }
            config.name = moduleJson["name"];
            
            if (!moduleJson.contains("version") || !moduleJson["version"].is_string()) {
                LOG_ERROR("Module config missing required 'version' field");
                return std::nullopt;
            }
            config.version = moduleJson["version"];
            
            // Optional fields
            if (moduleJson.contains("enabled")) {
                if (!moduleJson["enabled"].is_boolean()) {
                    LOG_ERROR("Module 'enabled' field must be boolean");
                    return std::nullopt;
                }
                config.enabled = moduleJson["enabled"];
            }
            
            // Parameters
            if (moduleJson.contains("parameters")) {
                if (!moduleJson["parameters"].is_object()) {
                    LOG_ERROR("Module 'parameters' field must be an object");
                    return std::nullopt;
                }
                
                for (const auto& param : moduleJson["parameters"].items()) {
                    if (!param.value().is_string()) {
                        LOG_ERROR("Parameter value must be a string: " + param.key());
                        return std::nullopt;
                    }
                    config.parameters[param.key()] = param.value();
                }
            }
            
            return config;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while parsing module config: " + std::string(e.what()));
            return std::nullopt;
        }
    }

    json ModuleConfigLoader::SerializeModuleConfig(const ModuleConfig& config) {
        json moduleJson;
        moduleJson["name"] = config.name;
        moduleJson["version"] = config.version;
        moduleJson["enabled"] = config.enabled;
        
        if (!config.parameters.empty()) {
            moduleJson["parameters"] = config.parameters;
        }
        
        return moduleJson;
    }

    std::optional<EngineConfig> ModuleConfigLoader::ParseEngineConfig(const json& engineJson) {
        try {
            EngineConfig config;
            
            // Required fields
            if (!engineJson.contains("engineVersion") || !engineJson["engineVersion"].is_string()) {
                LOG_ERROR("Engine config missing required 'engineVersion' field");
                return std::nullopt;
            }
            config.engineVersion = engineJson["engineVersion"];
            
            if (!engineJson.contains("configVersion") || !engineJson["configVersion"].is_string()) {
                LOG_ERROR("Engine config missing required 'configVersion' field");
                return std::nullopt;
            }
            config.configVersion = engineJson["configVersion"];
            
            // Modules array
            if (engineJson.contains("modules")) {
                if (!engineJson["modules"].is_array()) {
                    LOG_ERROR("Engine config 'modules' field must be an array");
                    return std::nullopt;
                }
                
                for (const auto& moduleJson : engineJson["modules"]) {
                    auto moduleConfig = ParseModuleConfig(moduleJson);
                    if (!moduleConfig) {
                        LOG_ERROR("Failed to parse module config");
                        return std::nullopt;
                    }
                    config.modules.push_back(*moduleConfig);
                }
            }
            
            return config;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while parsing engine config: " + std::string(e.what()));
            return std::nullopt;
        }
    }

    json ModuleConfigLoader::SerializeEngineConfig(const EngineConfig& config) {
        json engineJson;
        engineJson["engineVersion"] = config.engineVersion;
        engineJson["configVersion"] = config.configVersion;
        
        json modulesArray = json::array();
        for (const auto& moduleConfig : config.modules) {
            modulesArray.push_back(SerializeModuleConfig(moduleConfig));
        }
        engineJson["modules"] = modulesArray;
        
        return engineJson;
    }
}