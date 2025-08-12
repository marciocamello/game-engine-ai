#include "Core/ConfigManager.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace GameEngine {
namespace Core {

ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::LoadEngineConfig(const std::string& projectName) {
    m_configErrors.clear();
    
    // Load default engine config first
    LoadDefaultConfigs();
    
    std::vector<std::string> configPaths;
    
    // Project-specific engine config (highest priority)
    if (!projectName.empty()) {
        configPaths.push_back("projects/" + projectName + "/config/engine_config.json");
    }
    
    // Shared default engine config (fallback)
    if (!m_sharedConfigPath.empty()) {
        configPaths.push_back(m_sharedConfigPath + "/default_engine_config.json");
    }
    
    // Try to load from each path in order
    bool loaded = false;
    for (const auto& path : configPaths) {
        if (std::filesystem::exists(path)) {
#ifdef GAMEENGINE_HAS_JSON
            if (LoadJsonFile(path, &m_engineConfig)) {
                LOG_INFO("Loaded engine config from: " + path);
                loaded = true;
                break;
            }
#else
            LOG_WARNING("JSON support not available, using default engine configuration");
            loaded = true;
            break;
#endif
        }
    }
    
    if (!loaded) {
        LOG_WARNING("No engine config found, using built-in defaults");
#ifdef GAMEENGINE_HAS_JSON
        m_engineConfig = m_defaultEngineConfig;
#endif
        loaded = true;
    }
    
    return loaded;
}

bool ConfigManager::LoadProjectConfig(const std::string& projectName) {
    if (projectName.empty()) {
        LOG_ERROR("Project name cannot be empty");
        return false;
    }
    
    std::string configPath = "projects/" + projectName + "/config/project_config.json";
    
    if (!std::filesystem::exists(configPath)) {
        LOG_WARNING("Project config not found: " + configPath + ", using defaults");
#ifdef GAMEENGINE_HAS_JSON
        m_projectConfig = m_defaultProjectConfig;
        // Update project name in default config
        m_projectConfig["projectName"] = projectName;
#endif
        return true;
    }
    
#ifdef GAMEENGINE_HAS_JSON
    if (LoadJsonFile(configPath, &m_projectConfig)) {
        LOG_INFO("Loaded project config from: " + configPath);
        return true;
    }
#else
    LOG_WARNING("JSON support not available, using default project configuration");
    return true;
#endif
    
    return false;
}

bool ConfigManager::LoadModuleDefaults() {
    std::string defaultsPath;
    
    if (!m_sharedConfigPath.empty()) {
        defaultsPath = m_sharedConfigPath + "/module_defaults.json";
    } else {
        defaultsPath = "shared/configs/module_defaults.json";
    }
    
    if (!std::filesystem::exists(defaultsPath)) {
        LOG_WARNING("Module defaults not found: " + defaultsPath);
        return false;
    }
    
#ifdef GAMEENGINE_HAS_JSON
    if (LoadJsonFile(defaultsPath, &m_moduleDefaults)) {
        LOG_INFO("Loaded module defaults from: " + defaultsPath);
        return true;
    }
#else
    LOG_WARNING("JSON support not available, using built-in module defaults");
    return true;
#endif
    
    return false;
}

std::string ConfigManager::GetEngineConfigValue(const std::string& key, const std::string& defaultValue) const {
    return GetConfigValue(key, defaultValue, false, true, true);
}

std::string ConfigManager::GetProjectConfigValue(const std::string& key, const std::string& defaultValue) const {
    return GetConfigValue(key, defaultValue, true, false, true);
}

std::string ConfigManager::GetModuleConfigValue(const std::string& moduleName, const std::string& parameter, const std::string& defaultValue) const {
#ifdef GAMEENGINE_HAS_JSON
    try {
        // First check engine config for module settings
        if (m_engineConfig.contains("modules")) {
            for (const auto& module : m_engineConfig["modules"]) {
                if (module.contains("name") && module["name"] == moduleName) {
                    if (module.contains("parameters") && module["parameters"].contains(parameter)) {
                        return module["parameters"][parameter].get<std::string>();
                    }
                }
            }
        }
        
        // Then check module defaults
        if (m_moduleDefaults.contains("modules") && 
            m_moduleDefaults["modules"].contains(moduleName) &&
            m_moduleDefaults["modules"][moduleName].contains("parameters") &&
            m_moduleDefaults["modules"][moduleName]["parameters"].contains(parameter)) {
            
            auto paramConfig = m_moduleDefaults["modules"][moduleName]["parameters"][parameter];
            if (paramConfig.contains("default")) {
                if (paramConfig["default"].is_string()) {
                    return paramConfig["default"].get<std::string>();
                } else {
                    return paramConfig["default"].dump();
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting module config value: " + std::string(e.what()));
    }
#endif
    
    return defaultValue;
}

bool ConfigManager::GetEngineConfigBool(const std::string& key, bool defaultValue) const {
    std::string value = GetEngineConfigValue(key, defaultValue ? "true" : "false");
    return (value == "true" || value == "1" || value == "yes");
}

int ConfigManager::GetEngineConfigInt(const std::string& key, int defaultValue) const {
    std::string value = GetEngineConfigValue(key, std::to_string(defaultValue));
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

float ConfigManager::GetEngineConfigFloat(const std::string& key, float defaultValue) const {
    std::string value = GetEngineConfigValue(key, std::to_string(defaultValue));
    try {
        return std::stof(value);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

bool ConfigManager::GetProjectConfigBool(const std::string& key, bool defaultValue) const {
    std::string value = GetProjectConfigValue(key, defaultValue ? "true" : "false");
    return (value == "true" || value == "1" || value == "yes");
}

int ConfigManager::GetProjectConfigInt(const std::string& key, int defaultValue) const {
    std::string value = GetProjectConfigValue(key, std::to_string(defaultValue));
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

float ConfigManager::GetProjectConfigFloat(const std::string& key, float defaultValue) const {
    std::string value = GetProjectConfigValue(key, std::to_string(defaultValue));
    try {
        return std::stof(value);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

void ConfigManager::SetProjectConfigPath(const std::string& projectName) {
    m_projectConfigPath = "projects/" + projectName + "/config";
}

void ConfigManager::SetSharedConfigPath(const std::string& sharedPath) {
    m_sharedConfigPath = sharedPath;
}

std::vector<ConfigManager::ModuleConfig> ConfigManager::GetEnabledModules() const {
    std::vector<ModuleConfig> enabledModules;
    
#ifdef GAMEENGINE_HAS_JSON
    try {
        if (m_engineConfig.contains("modules")) {
            for (const auto& moduleJson : m_engineConfig["modules"]) {
                ModuleConfig config;
                config.name = moduleJson.value("name", "");
                config.version = moduleJson.value("version", "1.0.0");
                config.enabled = moduleJson.value("enabled", true);
                
                if (config.enabled && !config.name.empty()) {
                    if (moduleJson.contains("parameters")) {
                        for (const auto& [key, value] : moduleJson["parameters"].items()) {
                            config.parameters[key] = value.is_string() ? value.get<std::string>() : value.dump();
                        }
                    }
                    enabledModules.push_back(config);
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting enabled modules: " + std::string(e.what()));
    }
#endif
    
    return enabledModules;
}

ConfigManager::ModuleConfig ConfigManager::GetModuleConfig(const std::string& moduleName) const {
    ModuleConfig config;
    config.name = moduleName;
    config.enabled = false;
    
#ifdef GAMEENGINE_HAS_JSON
    try {
        if (m_engineConfig.contains("modules")) {
            for (const auto& moduleJson : m_engineConfig["modules"]) {
                if (moduleJson.value("name", "") == moduleName) {
                    config.version = moduleJson.value("version", "1.0.0");
                    config.enabled = moduleJson.value("enabled", false);
                    
                    if (moduleJson.contains("parameters")) {
                        for (const auto& [key, value] : moduleJson["parameters"].items()) {
                            config.parameters[key] = value.is_string() ? value.get<std::string>() : value.dump();
                        }
                    }
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting module config: " + std::string(e.what()));
    }
#endif
    
    return config;
}

bool ConfigManager::IsModuleEnabled(const std::string& moduleName) const {
    return GetModuleConfig(moduleName).enabled;
}

bool ConfigManager::ValidateEngineConfig() const {
    // Basic validation - check required fields
    // Note: We can't modify m_configErrors in a const method, so we'll use a local vector
    std::vector<std::string> errors;
    
#ifdef GAMEENGINE_HAS_JSON
    try {
        if (!m_engineConfig.contains("engineVersion")) {
            errors.push_back("Missing engineVersion in engine config");
        }
        
        if (!m_engineConfig.contains("modules")) {
            errors.push_back("Missing modules array in engine config");
        } else {
            for (const auto& module : m_engineConfig["modules"]) {
                if (!module.contains("name")) {
                    errors.push_back("Module missing name field");
                }
            }
        }
    } catch (const std::exception& e) {
        errors.push_back("JSON parsing error: " + std::string(e.what()));
    }
#endif
    
    return errors.empty();
}

bool ConfigManager::ValidateProjectConfig() const {
    std::vector<std::string> errors;
    
#ifdef GAMEENGINE_HAS_JSON
    try {
        if (!m_projectConfig.contains("projectName")) {
            errors.push_back("Missing projectName in project config");
        }
        
        if (!m_projectConfig.contains("requiredModules")) {
            errors.push_back("Missing requiredModules array in project config");
        }
    } catch (const std::exception& e) {
        errors.push_back("JSON parsing error: " + std::string(e.what()));
    }
#endif
    
    return errors.empty();
}

std::vector<std::string> ConfigManager::GetConfigurationErrors() const {
    return m_configErrors;
}

bool ConfigManager::SaveEngineConfig(const std::string& projectName) const {
#ifdef GAMEENGINE_HAS_JSON
    std::string configPath;
    if (!projectName.empty()) {
        configPath = "projects/" + projectName + "/config/engine_config.json";
    } else {
        configPath = m_sharedConfigPath + "/default_engine_config.json";
    }
    
    return SaveJsonFile(configPath, &m_engineConfig);
#else
    LOG_WARNING("JSON support not available, cannot save engine config");
    return false;
#endif
}

bool ConfigManager::SaveProjectConfig(const std::string& projectName) const {
#ifdef GAMEENGINE_HAS_JSON
    if (projectName.empty()) {
        LOG_ERROR("Project name cannot be empty");
        return false;
    }
    
    std::string configPath = "projects/" + projectName + "/config/project_config.json";
    return SaveJsonFile(configPath, &m_projectConfig);
#else
    LOG_WARNING("JSON support not available, cannot save project config");
    return false;
#endif
}

bool ConfigManager::CreateDefaultConfigs(const std::string& projectName) const {
    if (projectName.empty()) {
        LOG_ERROR("Project name cannot be empty");
        return false;
    }
    
    std::string configDir = "projects/" + projectName + "/config";
    
    try {
        // Create config directory
        std::filesystem::create_directories(configDir);
        
        // Copy default configs
        std::string sharedConfigPath = m_sharedConfigPath.empty() ? "shared/configs" : m_sharedConfigPath;
        
        std::filesystem::copy_file(
            sharedConfigPath + "/default_engine_config.json",
            configDir + "/engine_config.json",
            std::filesystem::copy_options::overwrite_existing
        );
        
        std::filesystem::copy_file(
            sharedConfigPath + "/default_project_config.json",
            configDir + "/project_config.json",
            std::filesystem::copy_options::overwrite_existing
        );
        
        // Update project name in project config
#ifdef GAMEENGINE_HAS_JSON
        nlohmann::json projectConfig;
        if (LoadJsonFile(configDir + "/project_config.json", &projectConfig)) {
            projectConfig["projectName"] = projectName;
            SaveJsonFile(configDir + "/project_config.json", &projectConfig);
        }
#endif
        
        LOG_INFO("Created default configs for project: " + projectName);
        return true;
        
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Failed to create default configs: " + std::string(e.what()));
        return false;
    }
}

bool ConfigManager::LoadJsonFile(const std::string& filePath, void* jsonObject) const {
#ifdef GAMEENGINE_HAS_JSON
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json* json = static_cast<nlohmann::json*>(jsonObject);
        file >> *json;
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load JSON file " + filePath + ": " + e.what());
        return false;
    }
#else
    LOG_WARNING("JSON support not available, cannot load: " + filePath);
    return false;
#endif
}

bool ConfigManager::SaveJsonFile(const std::string& filePath, const void* jsonObject) const {
#ifdef GAMEENGINE_HAS_JSON
    try {
        // Create directory if it doesn't exist
        std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        const nlohmann::json* json = static_cast<const nlohmann::json*>(jsonObject);
        file << json->dump(4); // Pretty print with 4 spaces
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save JSON file " + filePath + ": " + e.what());
        return false;
    }
#else
    LOG_WARNING("JSON support not available, cannot save: " + filePath);
    return false;
#endif
}

std::string ConfigManager::ResolveConfigPath(const std::string& relativePath) const {
    std::vector<std::string> searchPaths = {
        m_projectConfigPath,
        m_sharedConfigPath,
        "shared/configs"
    };
    
    for (const auto& basePath : searchPaths) {
        if (!basePath.empty()) {
            std::string fullPath = basePath + "/" + relativePath;
            if (std::filesystem::exists(fullPath)) {
                return fullPath;
            }
        }
    }
    
    return "";
}

std::string ConfigManager::GetConfigValue(const std::string& key, const std::string& defaultValue,
                                        bool useProject, bool useEngine, bool useDefaults) const {
#ifdef GAMEENGINE_HAS_JSON
    try {
        // Check project config first (if requested)
        if (useProject && m_projectConfig.contains(key)) {
            auto value = m_projectConfig[key];
            return value.is_string() ? value.get<std::string>() : value.dump();
        }
        
        // Check engine config (if requested)
        if (useEngine && m_engineConfig.contains(key)) {
            auto value = m_engineConfig[key];
            return value.is_string() ? value.get<std::string>() : value.dump();
        }
        
        // Check defaults (if requested)
        if (useDefaults) {
            if (m_defaultProjectConfig.contains(key)) {
                auto value = m_defaultProjectConfig[key];
                return value.is_string() ? value.get<std::string>() : value.dump();
            }
            
            if (m_defaultEngineConfig.contains(key)) {
                auto value = m_defaultEngineConfig[key];
                return value.is_string() ? value.get<std::string>() : value.dump();
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting config value: " + std::string(e.what()));
    }
#endif
    
    return defaultValue;
}

void ConfigManager::LoadDefaultConfigs() {
#ifdef GAMEENGINE_HAS_JSON
    // Load built-in default engine config
    m_defaultEngineConfig = nlohmann::json{
        {"engineVersion", "1.0.0"},
        {"configVersion", "1.0.0"},
        {"modules", nlohmann::json::array({
            {
                {"name", "Core"},
                {"version", "1.0.0"},
                {"enabled", true},
                {"parameters", {
                    {"logLevel", "INFO"},
                    {"maxThreads", "auto"}
                }}
            },
            {
                {"name", "Graphics"},
                {"version", "1.0.0"},
                {"enabled", true},
                {"parameters", {
                    {"renderer", "OpenGL"},
                    {"vsync", "true"},
                    {"fullscreen", "false"},
                    {"resolution", "1280x720"}
                }}
            }
        })}
    };
    
    // Load built-in default project config
    m_defaultProjectConfig = nlohmann::json{
        {"projectName", "DefaultProject"},
        {"projectVersion", "1.0.0"},
        {"requiredModules", nlohmann::json::array({"Core", "Graphics", "Input"})},
        {"optionalModules", nlohmann::json::array({"Physics", "Audio"})},
        {"assetPath", "assets/"},
        {"configPath", "config/"}
    };
#endif
}

void ConfigManager::MergeConfigurations() {
    // This method can be used to merge configurations from multiple sources
    // Currently, the hierarchy is handled in GetConfigValue
}

} // namespace Core
} // namespace GameEngine