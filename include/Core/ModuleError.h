#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>

namespace GameEngine {
    
    // Forward declarations
    struct ModuleConfig;
    struct EngineConfig;

    enum class ModuleErrorType {
        None,
        ModuleNotFound,
        DependencyMissing,
        CircularDependency,
        InitializationFailed,
        ConfigurationInvalid,
        VersionMismatch,
        LoadingFailed,
        ValidationFailed,
        RuntimeError
    };

    struct ModuleError {
        ModuleErrorType type = ModuleErrorType::None;
        std::string moduleName;
        std::string message;
        std::string details;
        std::vector<std::string> affectedModules;
        
        ModuleError() = default;
        ModuleError(ModuleErrorType errorType, const std::string& module, 
                   const std::string& msg, const std::string& detail = "")
            : type(errorType), moduleName(module), message(msg), details(detail) {}
        
        bool HasError() const { return type != ModuleErrorType::None; }
        std::string GetFormattedMessage() const;
    };

    class ModuleErrorCollector {
    public:
        void AddError(const ModuleError& error);
        void AddError(ModuleErrorType type, const std::string& moduleName, 
                     const std::string& message, const std::string& details = "");
        
        bool HasErrors() const { return !m_errors.empty(); }
        bool HasCriticalErrors() const;
        size_t GetErrorCount() const { return m_errors.size(); }
        
        const std::vector<ModuleError>& GetErrors() const { return m_errors; }
        std::vector<ModuleError> GetErrorsByType(ModuleErrorType type) const;
        std::vector<ModuleError> GetErrorsByModule(const std::string& moduleName) const;
        
        void Clear() { m_errors.clear(); }
        std::string GetSummary() const;
        void LogAllErrors() const;
        
    private:
        std::vector<ModuleError> m_errors;
        bool IsCriticalError(ModuleErrorType type) const;
    };

    enum class ValidationResult {
        Valid,
        Warning,
        Error,
        Critical
    };

    struct ValidationIssue {
        ValidationResult severity = ValidationResult::Valid;
        std::string field;
        std::string message;
        std::string suggestion;
        
        ValidationIssue() = default;
        ValidationIssue(ValidationResult sev, const std::string& fieldName, 
                       const std::string& msg, const std::string& suggest = "")
            : severity(sev), field(fieldName), message(msg), suggestion(suggest) {}
    };

    class ConfigurationValidator {
    public:
        struct ValidationContext {
            std::vector<ValidationIssue> issues;
            bool hasErrors = false;
            bool hasCriticalErrors = false;
            
            void AddIssue(const ValidationIssue& issue);
            void AddIssue(ValidationResult severity, const std::string& field, 
                         const std::string& message, const std::string& suggestion = "");
            
            bool IsValid() const { return !hasErrors && !hasCriticalErrors; }
            std::string GetSummary() const;
        };
        
        static ValidationContext ValidateModuleConfig(const ModuleConfig& config);
        static ValidationContext ValidateEngineConfig(const EngineConfig& config);
        static ValidationContext ValidateModuleDependencies(const std::vector<std::string>& dependencies);
        
    private:
        static bool IsValidModuleName(const std::string& name);
        static bool IsValidVersion(const std::string& version);
        static bool IsValidParameter(const std::string& key, const std::string& value);
    };
}