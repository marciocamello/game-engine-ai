#include "Core/ProjectConfigLoader.h"
#include "Core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_set>
#include <filesystem>

using json = nlohmann::json;

namespace GameEngine {

    std::optional<ProjectConfig> ProjectConfigLoader::LoadFromFile(const std::string& filePath) {
        try {
            if (!std::filesystem::exists(filePath)) {
                LOG_ERROR("Project configuration file not found: " + filePath);
                return std::nullopt;
            }

            std::ifstream file(filePath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open project configuration file: " + filePath);
                return std::nullopt;
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();

            return LoadFromString(content);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while loading project config file '" + filePath + "': " + e.what());
            return std::nullopt;
        }
    }

    std::optional<ProjectConfig> ProjectConfigLoader::LoadFromString(const std::string& jsonString) {
        try {
            json configJson = json::parse(jsonString);
            return ParseProjectConfig(configJson);
        }
        catch (const json::parse_error& e) {
            LOG_ERROR("JSON parse error in project config: " + std::string(e.what()));
            return std::nullopt;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while parsing project config string: " + std::string(e.what()));
            return std::nullopt;
        }
    }

    bool ProjectConfigLoader::SaveToFile(const ProjectConfig& config, const std::string& filePath) {
        try {
            std::string jsonString = SaveToString(config);
            
            // Create directory if it doesn't exist
            std::filesystem::path path(filePath);
            if (path.has_parent_path()) {
                std::filesystem::create_directories(path.parent_path());
            }

            std::ofstream file(filePath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file for writing project config: " + filePath);
                return false;
            }

            file << jsonString;
            file.close();
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while saving project config to file '" + filePath + "': " + e.what());
            return false;
        }
    }

    std::string ProjectConfigLoader::SaveToString(const ProjectConfig& config) {
        try {
            json configJson = SerializeProjectConfig(config);
            return configJson.dump(4); // Pretty print with 4 spaces
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while serializing project config: " + std::string(e.what()));
            return "";
        }
    }

    ProjectConfigValidationResult ProjectConfigLoader::ValidateConfig(const ProjectConfig& config) {
        ProjectConfigValidationResult result;

        // Validate project name
        if (config.projectName.empty()) {
            result.errorType = ProjectConfigError::MissingRequiredField;
            result.errorMessage = "Project name is required";
            result.fieldName = "projectName";
            return result;
        }

        if (!IsValidProjectName(config.projectName)) {
            result.errorType = ProjectConfigError::InvalidProjectName;
            result.errorMessage = "Invalid project name format: " + config.projectName;
            result.fieldName = "projectName";
            return result;
        }

        // Validate project version
        if (config.projectVersion.empty()) {
            result.errorType = ProjectConfigError::MissingRequiredField;
            result.errorMessage = "Project version is required";
            result.fieldName = "projectVersion";
            return result;
        }

        if (!IsValidVersion(config.projectVersion)) {
            result.errorType = ProjectConfigError::InvalidVersion;
            result.errorMessage = "Invalid project version format: " + config.projectVersion;
            result.fieldName = "projectVersion";
            return result;
        }

        // Validate engine version constraints
        if (!config.minEngineVersion.empty() && !IsValidVersion(config.minEngineVersion)) {
            result.errorType = ProjectConfigError::InvalidVersion;
            result.errorMessage = "Invalid minimum engine version format: " + config.minEngineVersion;
            result.fieldName = "minEngineVersion";
            return result;
        }

        if (!config.maxEngineVersion.empty() && !IsValidVersion(config.maxEngineVersion)) {
            result.errorType = ProjectConfigError::InvalidVersion;
            result.errorMessage = "Invalid maximum engine version format: " + config.maxEngineVersion;
            result.fieldName = "maxEngineVersion";
            return result;
        }

        // Validate module dependencies
        ProjectConfigValidationResult moduleResult = ValidateModuleDependencies(
            config.requiredModules, config.optionalModules);
        if (!moduleResult.isValid) {
            return moduleResult;
        }

        // Validate paths
        if (!IsValidPath(config.assetPath)) {
            result.errorType = ProjectConfigError::InvalidPath;
            result.errorMessage = "Invalid asset path: " + config.assetPath;
            result.fieldName = "assetPath";
            return result;
        }

        if (!IsValidPath(config.configPath)) {
            result.errorType = ProjectConfigError::InvalidPath;
            result.errorMessage = "Invalid config path: " + config.configPath;
            result.fieldName = "configPath";
            return result;
        }

        if (!IsValidPath(config.buildPath)) {
            result.errorType = ProjectConfigError::InvalidPath;
            result.errorMessage = "Invalid build path: " + config.buildPath;
            result.fieldName = "buildPath";
            return result;
        }

        // Validate project settings
        for (const auto& setting : config.projectSettings) {
            if (!IsValidSettingName(setting.first)) {
                result.errorType = ProjectConfigError::UnknownSetting;
                result.errorMessage = "Invalid setting name: " + setting.first;
                result.fieldName = "projectSettings." + setting.first;
                return result;
            }

            if (!IsValidSettingValue(setting.second)) {
                result.errorType = ProjectConfigError::InvalidFieldType;
                result.errorMessage = "Invalid setting value for: " + setting.first;
                result.fieldName = "projectSettings." + setting.first;
                return result;
            }
        }

        result.isValid = true;
        return result;
    }

    ProjectConfigValidationResult ProjectConfigLoader::ValidateModuleDependencies(
        const std::vector<std::string>& requiredModules,
        const std::vector<std::string>& optionalModules) {
        
        ProjectConfigValidationResult result;
        std::unordered_set<std::string> moduleNames;

        // Validate required modules
        for (const auto& moduleName : requiredModules) {
            if (!IsValidModuleName(moduleName)) {
                result.errorType = ProjectConfigError::InvalidModuleName;
                result.errorMessage = "Invalid required module name: " + moduleName;
                result.fieldName = "requiredModules";
                return result;
            }

            if (moduleNames.find(moduleName) != moduleNames.end()) {
                result.errorType = ProjectConfigError::DuplicateModule;
                result.errorMessage = "Duplicate module in dependencies: " + moduleName;
                result.fieldName = "requiredModules";
                return result;
            }
            moduleNames.insert(moduleName);
        }

        // Validate optional modules
        for (const auto& moduleName : optionalModules) {
            if (!IsValidModuleName(moduleName)) {
                result.errorType = ProjectConfigError::InvalidModuleName;
                result.errorMessage = "Invalid optional module name: " + moduleName;
                result.fieldName = "optionalModules";
                return result;
            }

            if (moduleNames.find(moduleName) != moduleNames.end()) {
                result.errorType = ProjectConfigError::DuplicateModule;
                result.errorMessage = "Module appears in both required and optional: " + moduleName;
                result.fieldName = "optionalModules";
                return result;
            }
            moduleNames.insert(moduleName);
        }

        result.isValid = true;
        return result;
    }

    std::string ProjectConfigLoader::GetErrorMessage(ProjectConfigError error) {
        switch (error) {
            case ProjectConfigError::FileNotFound:
                return "Project configuration file not found";
            case ProjectConfigError::InvalidJson:
                return "Invalid JSON format";
            case ProjectConfigError::MissingRequiredField:
                return "Missing required field";
            case ProjectConfigError::InvalidFieldType:
                return "Invalid field type";
            case ProjectConfigError::InvalidProjectName:
                return "Invalid project name";
            case ProjectConfigError::InvalidVersion:
                return "Invalid version format";
            case ProjectConfigError::InvalidModuleName:
                return "Invalid module name";
            case ProjectConfigError::DuplicateModule:
                return "Duplicate module definition";
            case ProjectConfigError::IncompatibleEngineVersion:
                return "Incompatible engine version";
            case ProjectConfigError::InvalidPath:
                return "Invalid path format";
            case ProjectConfigError::UnknownSetting:
                return "Unknown project setting";
            default:
                return "Unknown project configuration error";
        }
    }

    std::string ProjectConfigLoader::GetDetailedErrorMessage(const ProjectConfigValidationResult& result) {
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

    ProjectConfig ProjectConfigLoader::CreateDefaultConfig(const std::string& projectName) {
        ProjectConfig config;
        config.projectName = projectName;
        config.projectVersion = "1.0.0";
        config.description = "A game project built with Game Engine Kiro";
        config.author = "";
        
        // Default required modules for any game project
        config.requiredModules = {"Core", "Graphics", "Input"};
        
        // Common optional modules
        config.optionalModules = {"Physics", "Audio", "Scripting"};
        
        // Default paths
        config.assetPath = "assets/";
        config.configPath = "config/";
        config.buildPath = "build/";
        
        // Engine compatibility
        config.minEngineVersion = "1.0.0";
        
        // Default project settings
        config.projectSettings["windowTitle"] = projectName;
        config.projectSettings["windowWidth"] = "1920";
        config.projectSettings["windowHeight"] = "1080";
        config.projectSettings["fullscreen"] = "false";
        
        return config;
    }

    ProjectConfig ProjectConfigLoader::CreateBasicGameConfig(const std::string& projectName) {
        ProjectConfig config = CreateDefaultConfig(projectName);
        
        // Basic game needs physics and audio
        config.requiredModules = {"Core", "Graphics", "Input", "Physics", "Audio"};
        config.optionalModules = {"Scripting"};
        
        // Additional game-specific settings
        config.projectSettings["enablePhysicsDebug"] = "false";
        config.projectSettings["masterVolume"] = "1.0";
        config.projectSettings["mouseSensitivity"] = "1.0";
        
        return config;
    }

    ProjectConfig ProjectConfigLoader::CreateMinimalConfig(const std::string& projectName) {
        ProjectConfig config;
        config.projectName = projectName;
        config.projectVersion = "1.0.0";
        config.description = "A minimal project built with Game Engine Kiro";
        
        // Minimal requirements
        config.requiredModules = {"Core", "Graphics"};
        config.optionalModules = {};
        
        // Minimal settings
        config.projectSettings["windowTitle"] = projectName;
        
        return config;
    }

    bool ProjectConfigLoader::IsValidModuleName(const std::string& moduleName) {
        if (moduleName.empty() || moduleName.length() > 64) {
            return false;
        }
        
        // Module name should start with letter and contain only alphanumeric characters and underscores
        std::regex namePattern("^[A-Za-z][A-Za-z0-9_]*$");
        return std::regex_match(moduleName, namePattern);
    }

    bool ProjectConfigLoader::IsCompatibleEngineVersion(const std::string& engineVersion, 
                                                       const std::string& minVersion, 
                                                       const std::string& maxVersion) {
        if (engineVersion.empty()) {
            return false;
        }

        // Check minimum version constraint
        if (!minVersion.empty()) {
            if (CompareVersions(engineVersion, minVersion) < 0) {
                return false;
            }
        }

        // Check maximum version constraint
        if (!maxVersion.empty()) {
            if (CompareVersions(engineVersion, maxVersion) > 0) {
                return false;
            }
        }

        return true;
    }

    bool ProjectConfigLoader::IsValidProjectName(const std::string& name) {
        if (name.empty() || name.length() > 128) {
            return false;
        }
        
        // Project name should start with letter or number and contain only alphanumeric characters, spaces, and common symbols
        std::regex namePattern("^[A-Za-z0-9][A-Za-z0-9 _\\-\\.]*$");
        return std::regex_match(name, namePattern);
    }

    bool ProjectConfigLoader::IsValidVersion(const std::string& version) {
        if (version.empty()) {
            return false;
        }
        
        // Version should follow semantic versioning pattern (major.minor.patch)
        std::regex versionPattern("^\\d+\\.\\d+\\.\\d+$");
        return std::regex_match(version, versionPattern);
    }

    bool ProjectConfigLoader::IsValidPath(const std::string& path) {
        if (path.empty() || path.length() > 256) {
            return false;
        }
        
        // Path should not contain invalid characters and should be relative
        std::regex pathPattern("^[A-Za-z0-9_\\-\\./ ]+[/]?$");
        return std::regex_match(path, pathPattern) && path.find("..") == std::string::npos;
    }

    bool ProjectConfigLoader::IsValidSettingName(const std::string& settingName) {
        if (settingName.empty() || settingName.length() > 64) {
            return false;
        }
        
        // Setting name should contain only alphanumeric characters and underscores
        std::regex settingPattern("^[A-Za-z][A-Za-z0-9_]*$");
        return std::regex_match(settingName, settingPattern);
    }

    bool ProjectConfigLoader::IsValidSettingValue(const std::string& settingValue) {
        // Setting values can be any non-empty string up to 512 characters
        return !settingValue.empty() && settingValue.length() <= 512;
    }

    std::optional<ProjectConfig> ProjectConfigLoader::ParseProjectConfig(const json& projectJson) {
        try {
            ProjectConfig config;
            
            // Required fields
            if (!projectJson.contains("projectName") || !projectJson["projectName"].is_string()) {
                LOG_ERROR("Project config missing required 'projectName' field");
                return std::nullopt;
            }
            config.projectName = projectJson["projectName"];
            
            if (!projectJson.contains("projectVersion") || !projectJson["projectVersion"].is_string()) {
                LOG_ERROR("Project config missing required 'projectVersion' field");
                return std::nullopt;
            }
            config.projectVersion = projectJson["projectVersion"];
            
            // Optional metadata fields
            if (projectJson.contains("description") && projectJson["description"].is_string()) {
                config.description = projectJson["description"];
            }
            
            if (projectJson.contains("author") && projectJson["author"].is_string()) {
                config.author = projectJson["author"];
            }
            
            // Module dependencies
            if (projectJson.contains("requiredModules") && projectJson["requiredModules"].is_array()) {
                for (const auto& module : projectJson["requiredModules"]) {
                    if (module.is_string()) {
                        config.requiredModules.push_back(module);
                    }
                }
            }
            
            if (projectJson.contains("optionalModules") && projectJson["optionalModules"].is_array()) {
                for (const auto& module : projectJson["optionalModules"]) {
                    if (module.is_string()) {
                        config.optionalModules.push_back(module);
                    }
                }
            }
            
            // Path configurations
            if (projectJson.contains("assetPath") && projectJson["assetPath"].is_string()) {
                config.assetPath = projectJson["assetPath"];
            }
            
            if (projectJson.contains("configPath") && projectJson["configPath"].is_string()) {
                config.configPath = projectJson["configPath"];
            }
            
            if (projectJson.contains("buildPath") && projectJson["buildPath"].is_string()) {
                config.buildPath = projectJson["buildPath"];
            }
            
            // Engine version constraints
            if (projectJson.contains("minEngineVersion") && projectJson["minEngineVersion"].is_string()) {
                config.minEngineVersion = projectJson["minEngineVersion"];
            }
            
            if (projectJson.contains("maxEngineVersion") && projectJson["maxEngineVersion"].is_string()) {
                config.maxEngineVersion = projectJson["maxEngineVersion"];
            }
            
            // Project settings
            if (projectJson.contains("projectSettings") && projectJson["projectSettings"].is_object()) {
                for (const auto& setting : projectJson["projectSettings"].items()) {
                    if (setting.value().is_string()) {
                        config.projectSettings[setting.key()] = setting.value();
                    }
                }
            }
            
            return config;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while parsing project config: " + std::string(e.what()));
            return std::nullopt;
        }
    }

    json ProjectConfigLoader::SerializeProjectConfig(const ProjectConfig& config) {
        json projectJson;
        
        // Required fields
        projectJson["projectName"] = config.projectName;
        projectJson["projectVersion"] = config.projectVersion;
        
        // Optional metadata
        if (!config.description.empty()) {
            projectJson["description"] = config.description;
        }
        
        if (!config.author.empty()) {
            projectJson["author"] = config.author;
        }
        
        // Module dependencies
        if (!config.requiredModules.empty()) {
            projectJson["requiredModules"] = config.requiredModules;
        }
        
        if (!config.optionalModules.empty()) {
            projectJson["optionalModules"] = config.optionalModules;
        }
        
        // Paths (only include if different from defaults)
        if (config.assetPath != "assets/") {
            projectJson["assetPath"] = config.assetPath;
        }
        
        if (config.configPath != "config/") {
            projectJson["configPath"] = config.configPath;
        }
        
        if (config.buildPath != "build/") {
            projectJson["buildPath"] = config.buildPath;
        }
        
        // Engine version constraints
        if (!config.minEngineVersion.empty()) {
            projectJson["minEngineVersion"] = config.minEngineVersion;
        }
        
        if (!config.maxEngineVersion.empty()) {
            projectJson["maxEngineVersion"] = config.maxEngineVersion;
        }
        
        // Project settings
        if (!config.projectSettings.empty()) {
            projectJson["projectSettings"] = config.projectSettings;
        }
        
        return projectJson;
    }

    int ProjectConfigLoader::CompareVersions(const std::string& version1, const std::string& version2) {
        int major1, minor1, patch1;
        int major2, minor2, patch2;
        
        if (!ParseVersion(version1, major1, minor1, patch1) || 
            !ParseVersion(version2, major2, minor2, patch2)) {
            return 0; // Invalid versions are considered equal
        }
        
        if (major1 != major2) return major1 - major2;
        if (minor1 != minor2) return minor1 - minor2;
        return patch1 - patch2;
    }

    bool ProjectConfigLoader::ParseVersion(const std::string& version, int& major, int& minor, int& patch) {
        std::regex versionPattern("^(\\d+)\\.(\\d+)\\.(\\d+)$");
        std::smatch matches;
        
        if (std::regex_match(version, matches, versionPattern)) {
            major = std::stoi(matches[1]);
            minor = std::stoi(matches[2]);
            patch = std::stoi(matches[3]);
            return true;
        }
        
        return false;
    }

}