#pragma once

#include "IEngineModule.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace GameEngine {

    enum class ConfigError {
        FileNotFound,
        InvalidJson,
        MissingRequiredField,
        InvalidFieldType,
        InvalidModuleName,
        InvalidVersion,
        DuplicateModule,
        UnknownParameter
    };

    struct ConfigValidationResult {
        bool isValid = false;
        ConfigError errorType;
        std::string errorMessage;
        std::string fieldName;
        int lineNumber = -1;
    };

    class ModuleConfigLoader {
    public:
        // Load configuration from JSON file
        static std::optional<EngineConfig> LoadFromFile(const std::string& filePath);
        static std::optional<EngineConfig> LoadFromString(const std::string& jsonString);

        // Save configuration to JSON file
        static bool SaveToFile(const EngineConfig& config, const std::string& filePath);
        static std::string SaveToString(const EngineConfig& config);

        // Validation
        static ConfigValidationResult ValidateConfig(const EngineConfig& config);
        static ConfigValidationResult ValidateModuleConfig(const ModuleConfig& moduleConfig);

        // Error handling
        static std::string GetErrorMessage(ConfigError error);
        static std::string GetDetailedErrorMessage(const ConfigValidationResult& result);

        // Configuration utilities
        static EngineConfig CreateDefaultConfig();
        static ModuleConfig CreateDefaultModuleConfig(const std::string& name, ModuleType type);

    private:
        // Internal validation helpers
        static bool IsValidModuleName(const std::string& name);
        static bool IsValidVersion(const std::string& version);
        static bool IsValidParameterName(const std::string& paramName);
        static bool IsValidParameterValue(const std::string& paramValue);

        // JSON parsing helpers
        static std::optional<ModuleConfig> ParseModuleConfig(const nlohmann::json& moduleJson);
        static nlohmann::json SerializeModuleConfig(const ModuleConfig& config);
        static std::optional<EngineConfig> ParseEngineConfig(const nlohmann::json& engineJson);
        static nlohmann::json SerializeEngineConfig(const EngineConfig& config);
    };
}