#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace GameEngine {

    /**
     * Configuration structure for individual game projects
     * Defines project metadata, module dependencies, and project-specific settings
     */
    struct ProjectConfig {
        // Project metadata
        std::string projectName;
        std::string projectVersion;
        std::string description;
        std::string author;

        // Module dependencies
        std::vector<std::string> requiredModules;
        std::vector<std::string> optionalModules;

        // Project-specific settings
        std::unordered_map<std::string, std::string> projectSettings;

        // Asset and configuration paths
        std::string assetPath = "assets/";
        std::string configPath = "config/";
        std::string buildPath = "build/";

        // Engine compatibility
        std::string minEngineVersion;
        std::string maxEngineVersion;

        // Default constructor
        ProjectConfig() = default;
    };

    /**
     * Error types for project configuration validation
     */
    enum class ProjectConfigError {
        FileNotFound,
        InvalidJson,
        MissingRequiredField,
        InvalidFieldType,
        InvalidProjectName,
        InvalidVersion,
        InvalidModuleName,
        DuplicateModule,
        IncompatibleEngineVersion,
        InvalidPath,
        UnknownSetting
    };

    /**
     * Result structure for project configuration validation
     */
    struct ProjectConfigValidationResult {
        bool isValid = false;
        ProjectConfigError errorType;
        std::string errorMessage;
        std::string fieldName;
        int lineNumber = -1;
    };

}