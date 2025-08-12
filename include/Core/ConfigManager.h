#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#ifdef GAMEENGINE_HAS_JSON
#include <nlohmann/json.hpp>
#endif

namespace GameEngine {
namespace Core {

/**
 * Configuration management system
 * Handles loading and merging of configuration files with fallback hierarchy
 */
class ConfigManager {
public:
    static ConfigManager& GetInstance();

    // Configuration loading
    bool LoadEngineConfig(const std::string& projectName = "");
    bool LoadProjectConfig(const std::string& projectName);
    bool LoadModuleDefaults();

    // Configuration access
    std::string GetEngineConfigValue(const std::string& key, const std::string& defaultValue = "") const;
    std::string GetProjectConfigValue(const std::string& key, const std::string& defaultValue = "") const;
    std::string GetModuleConfigValue(const std::string& moduleName, const std::string& parameter, const std::string& defaultValue = "") const;

    bool GetEngineConfigBool(const std::string& key, bool defaultValue = false) const;
    int GetEngineConfigInt(const std::string& key, int defaultValue = 0) const;
    float GetEngineConfigFloat(const std::string& key, float defaultValue = 0.0f) const;

    bool GetProjectConfigBool(const std::string& key, bool defaultValue = false) const;
    int GetProjectConfigInt(const std::string& key, int defaultValue = 0) const;
    float GetProjectConfigFloat(const std::string& key, float defaultValue = 0.0f) const;

    // Configuration paths
    void SetProjectConfigPath(const std::string& projectName);
    void SetSharedConfigPath(const std::string& sharedPath);
    
    std::string GetProjectConfigPath() const { return m_projectConfigPath; }
    std::string GetSharedConfigPath() const { return m_sharedConfigPath; }

    // Module configuration
    struct ModuleConfig {
        std::string name;
        std::string version;
        bool enabled;
        std::unordered_map<std::string, std::string> parameters;
    };

    std::vector<ModuleConfig> GetEnabledModules() const;
    ModuleConfig GetModuleConfig(const std::string& moduleName) const;
    bool IsModuleEnabled(const std::string& moduleName) const;

    // Configuration validation
    bool ValidateEngineConfig() const;
    bool ValidateProjectConfig() const;
    std::vector<std::string> GetConfigurationErrors() const;

    // Configuration file management
    bool SaveEngineConfig(const std::string& projectName = "") const;
    bool SaveProjectConfig(const std::string& projectName) const;
    bool CreateDefaultConfigs(const std::string& projectName) const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

#ifdef GAMEENGINE_HAS_JSON
    nlohmann::json m_engineConfig;
    nlohmann::json m_projectConfig;
    nlohmann::json m_moduleDefaults;
    nlohmann::json m_defaultEngineConfig;
    nlohmann::json m_defaultProjectConfig;
#else
    // Fallback storage when JSON is not available
    std::unordered_map<std::string, std::string> m_engineConfigMap;
    std::unordered_map<std::string, std::string> m_projectConfigMap;
#endif

    std::string m_projectConfigPath;
    std::string m_sharedConfigPath;
    std::vector<std::string> m_configErrors;

    bool LoadJsonFile(const std::string& filePath, void* jsonObject) const;
    bool SaveJsonFile(const std::string& filePath, const void* jsonObject) const;
    std::string ResolveConfigPath(const std::string& relativePath) const;
    
    // Configuration hierarchy resolution
    std::string GetConfigValue(const std::string& key, const std::string& defaultValue,
                              bool useProject, bool useEngine, bool useDefaults) const;
    
    void LoadDefaultConfigs();
    void MergeConfigurations();
};

} // namespace Core
} // namespace GameEngine