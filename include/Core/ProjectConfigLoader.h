#pragma once

#include "ProjectConfig.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace GameEngine {

    /**
     * Utility class for loading, saving, and validating project configurations
     * Handles JSON serialization/deserialization and validation of project settings
     */
    class ProjectConfigLoader {
    public:
        // Load configuration from JSON file
        static std::optional<ProjectConfig> LoadFromFile(const std::string& filePath);
        static std::optional<ProjectConfig> LoadFromString(const std::string& jsonString);

        // Save configuration to JSON file
        static bool SaveToFile(const ProjectConfig& config, const std::string& filePath);
        static std::string SaveToString(const ProjectConfig& config);

        // Validation
        static ProjectConfigValidationResult ValidateConfig(const ProjectConfig& config);
        static ProjectConfigValidationResult ValidateModuleDependencies(
            const std::vector<std::string>& requiredModules,
            const std::vector<std::string>& optionalModules);

        // Error handling
        static std::string GetErrorMessage(ProjectConfigError error);
        static std::string GetDetailedErrorMessage(const ProjectConfigValidationResult& result);

        // Configuration utilities
        static ProjectConfig CreateDefaultConfig(const std::string& projectName);
        static ProjectConfig CreateBasicGameConfig(const std::string& projectName);
        static ProjectConfig CreateMinimalConfig(const std::string& projectName);

        // Module dependency utilities
        static bool IsValidModuleName(const std::string& moduleName);
        static bool IsCompatibleEngineVersion(const std::string& engineVersion, 
                                            const std::string& minVersion, 
                                            const std::string& maxVersion);

    private:
        // Internal validation helpers
        static bool IsValidProjectName(const std::string& name);
        static bool IsValidVersion(const std::string& version);
        static bool IsValidPath(const std::string& path);
        static bool IsValidSettingName(const std::string& settingName);
        static bool IsValidSettingValue(const std::string& settingValue);

        // JSON parsing helpers
        static std::optional<ProjectConfig> ParseProjectConfig(const nlohmann::json& projectJson);
        static nlohmann::json SerializeProjectConfig(const ProjectConfig& config);
        
        // Version comparison utilities
        static int CompareVersions(const std::string& version1, const std::string& version2);
        static bool ParseVersion(const std::string& version, int& major, int& minor, int& patch);
    };

}