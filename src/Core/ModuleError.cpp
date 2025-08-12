#include "Core/ModuleError.h"
#include "Core/IEngineModule.h"
#include "Core/Logger.h"
#include <sstream>
#include <algorithm>
#include <regex>

namespace GameEngine {

    std::string ModuleError::GetFormattedMessage() const {
        std::ostringstream oss;
        
        // Error type prefix
        switch (type) {
            case ModuleErrorType::ModuleNotFound:
                oss << "[MODULE NOT FOUND] ";
                break;
            case ModuleErrorType::DependencyMissing:
                oss << "[DEPENDENCY MISSING] ";
                break;
            case ModuleErrorType::CircularDependency:
                oss << "[CIRCULAR DEPENDENCY] ";
                break;
            case ModuleErrorType::InitializationFailed:
                oss << "[INITIALIZATION FAILED] ";
                break;
            case ModuleErrorType::ConfigurationInvalid:
                oss << "[INVALID CONFIGURATION] ";
                break;
            case ModuleErrorType::VersionMismatch:
                oss << "[VERSION MISMATCH] ";
                break;
            case ModuleErrorType::LoadingFailed:
                oss << "[LOADING FAILED] ";
                break;
            case ModuleErrorType::ValidationFailed:
                oss << "[VALIDATION FAILED] ";
                break;
            case ModuleErrorType::RuntimeError:
                oss << "[RUNTIME ERROR] ";
                break;
            default:
                oss << "[UNKNOWN ERROR] ";
                break;
        }
        
        // Module name
        if (!moduleName.empty()) {
            oss << "Module '" << moduleName << "': ";
        }
        
        // Main message
        oss << message;
        
        // Details
        if (!details.empty()) {
            oss << " - " << details;
        }
        
        // Affected modules
        if (!affectedModules.empty()) {
            oss << " (Affects: ";
            for (size_t i = 0; i < affectedModules.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << affectedModules[i];
            }
            oss << ")";
        }
        
        return oss.str();
    }

    void ModuleErrorCollector::AddError(const ModuleError& error) {
        m_errors.push_back(error);
    }

    void ModuleErrorCollector::AddError(ModuleErrorType type, const std::string& moduleName, 
                                       const std::string& message, const std::string& details) {
        m_errors.emplace_back(type, moduleName, message, details);
    }

    bool ModuleErrorCollector::HasCriticalErrors() const {
        return std::any_of(m_errors.begin(), m_errors.end(), 
            [this](const ModuleError& error) {
                return IsCriticalError(error.type);
            });
    }

    std::vector<ModuleError> ModuleErrorCollector::GetErrorsByType(ModuleErrorType type) const {
        std::vector<ModuleError> result;
        std::copy_if(m_errors.begin(), m_errors.end(), std::back_inserter(result),
            [type](const ModuleError& error) {
                return error.type == type;
            });
        return result;
    }

    std::vector<ModuleError> ModuleErrorCollector::GetErrorsByModule(const std::string& moduleName) const {
        std::vector<ModuleError> result;
        std::copy_if(m_errors.begin(), m_errors.end(), std::back_inserter(result),
            [&moduleName](const ModuleError& error) {
                return error.moduleName == moduleName;
            });
        return result;
    }

    std::string ModuleErrorCollector::GetSummary() const {
        if (m_errors.empty()) {
            return "No module errors detected.";
        }
        
        std::ostringstream oss;
        oss << "Module Error Summary (" << m_errors.size() << " errors):\n";
        
        // Count errors by type
        std::map<ModuleErrorType, int> errorCounts;
        for (const auto& error : m_errors) {
            errorCounts[error.type]++;
        }
        
        // Display counts
        for (const auto& pair : errorCounts) {
            oss << "  - ";
            switch (pair.first) {
                case ModuleErrorType::ModuleNotFound:
                    oss << "Module Not Found: " << pair.second;
                    break;
                case ModuleErrorType::DependencyMissing:
                    oss << "Missing Dependencies: " << pair.second;
                    break;
                case ModuleErrorType::CircularDependency:
                    oss << "Circular Dependencies: " << pair.second;
                    break;
                case ModuleErrorType::InitializationFailed:
                    oss << "Initialization Failures: " << pair.second;
                    break;
                case ModuleErrorType::ConfigurationInvalid:
                    oss << "Configuration Issues: " << pair.second;
                    break;
                case ModuleErrorType::VersionMismatch:
                    oss << "Version Mismatches: " << pair.second;
                    break;
                case ModuleErrorType::LoadingFailed:
                    oss << "Loading Failures: " << pair.second;
                    break;
                case ModuleErrorType::ValidationFailed:
                    oss << "Validation Failures: " << pair.second;
                    break;
                case ModuleErrorType::RuntimeError:
                    oss << "Runtime Errors: " << pair.second;
                    break;
                default:
                    oss << "Unknown Errors: " << pair.second;
                    break;
            }
            oss << "\n";
        }
        
        return oss.str();
    }

    void ModuleErrorCollector::LogAllErrors() const {
        if (m_errors.empty()) {
            return;
        }
        
        LOG_ERROR("Module Error Report:");
        for (const auto& error : m_errors) {
            if (IsCriticalError(error.type)) {
                LOG_CRITICAL(error.GetFormattedMessage());
            } else {
                LOG_ERROR(error.GetFormattedMessage());
            }
        }
        
        LOG_ERROR(GetSummary());
    }

    bool ModuleErrorCollector::IsCriticalError(ModuleErrorType type) const {
        switch (type) {
            case ModuleErrorType::CircularDependency:
            case ModuleErrorType::ConfigurationInvalid:
            case ModuleErrorType::ValidationFailed:
                return true;
            default:
                return false;
        }
    }

    void ConfigurationValidator::ValidationContext::AddIssue(const ValidationIssue& issue) {
        issues.push_back(issue);
        
        if (issue.severity == ValidationResult::Error) {
            hasErrors = true;
        } else if (issue.severity == ValidationResult::Critical) {
            hasCriticalErrors = true;
        }
    }

    void ConfigurationValidator::ValidationContext::AddIssue(ValidationResult severity, 
                                                            const std::string& field, 
                                                            const std::string& message, 
                                                            const std::string& suggestion) {
        AddIssue(ValidationIssue(severity, field, message, suggestion));
    }

    std::string ConfigurationValidator::ValidationContext::GetSummary() const {
        if (issues.empty()) {
            return "Configuration validation passed with no issues.";
        }
        
        std::ostringstream oss;
        oss << "Configuration Validation Summary (" << issues.size() << " issues):\n";
        
        int warnings = 0, errors = 0, critical = 0;
        for (const auto& issue : issues) {
            switch (issue.severity) {
                case ValidationResult::Warning: warnings++; break;
                case ValidationResult::Error: errors++; break;
                case ValidationResult::Critical: critical++; break;
                default: break;
            }
        }
        
        if (critical > 0) oss << "  - Critical Issues: " << critical << "\n";
        if (errors > 0) oss << "  - Errors: " << errors << "\n";
        if (warnings > 0) oss << "  - Warnings: " << warnings << "\n";
        
        oss << "\nDetailed Issues:\n";
        for (const auto& issue : issues) {
            oss << "  [";
            switch (issue.severity) {
                case ValidationResult::Warning: oss << "WARNING"; break;
                case ValidationResult::Error: oss << "ERROR"; break;
                case ValidationResult::Critical: oss << "CRITICAL"; break;
                default: oss << "INFO"; break;
            }
            oss << "] " << issue.field << ": " << issue.message;
            if (!issue.suggestion.empty()) {
                oss << " (Suggestion: " << issue.suggestion << ")";
            }
            oss << "\n";
        }
        
        return oss.str();
    }

    ConfigurationValidator::ValidationContext ConfigurationValidator::ValidateModuleConfig(const ModuleConfig& config) {
        ValidationContext context;
        
        // Validate module name
        if (config.name.empty()) {
            context.AddIssue(ValidationResult::Critical, "name", 
                           "Module name cannot be empty", 
                           "Provide a valid module name");
        } else if (!IsValidModuleName(config.name)) {
            context.AddIssue(ValidationResult::Error, "name", 
                           "Module name contains invalid characters", 
                           "Use only alphanumeric characters, hyphens, and underscores");
        }
        
        // Validate version
        if (config.version.empty()) {
            context.AddIssue(ValidationResult::Warning, "version", 
                           "Module version is empty", 
                           "Consider specifying a version for better compatibility tracking");
        } else if (!IsValidVersion(config.version)) {
            context.AddIssue(ValidationResult::Warning, "version", 
                           "Module version format may not be standard", 
                           "Consider using semantic versioning (e.g., 1.0.0)");
        }
        
        // Validate parameters
        for (const auto& param : config.parameters) {
            if (param.first.empty()) {
                context.AddIssue(ValidationResult::Error, "parameters", 
                               "Parameter key cannot be empty", 
                               "Remove empty parameter keys");
            } else if (!IsValidParameter(param.first, param.second)) {
                context.AddIssue(ValidationResult::Warning, "parameters." + param.first, 
                               "Parameter value may be invalid", 
                               "Check parameter value format and constraints");
            }
        }
        
        return context;
    }

    ConfigurationValidator::ValidationContext ConfigurationValidator::ValidateEngineConfig(const EngineConfig& config) {
        ValidationContext context;
        
        // Validate config version
        if (config.configVersion.empty()) {
            context.AddIssue(ValidationResult::Warning, "configVersion", 
                           "Configuration version is not specified", 
                           "Specify a configuration version for compatibility tracking");
        }
        
        // Validate engine version
        if (config.engineVersion.empty()) {
            context.AddIssue(ValidationResult::Warning, "engineVersion", 
                           "Engine version is not specified", 
                           "Specify the target engine version");
        }
        
        // Validate modules
        if (config.modules.empty()) {
            context.AddIssue(ValidationResult::Warning, "modules", 
                           "No modules specified in configuration", 
                           "Add module configurations as needed");
        }
        
        // Check for duplicate module names
        std::set<std::string> moduleNames;
        for (const auto& moduleConfig : config.modules) {
            if (moduleNames.find(moduleConfig.name) != moduleNames.end()) {
                context.AddIssue(ValidationResult::Error, "modules", 
                               "Duplicate module name: " + moduleConfig.name, 
                               "Remove duplicate module configurations");
            }
            moduleNames.insert(moduleConfig.name);
            
            // Validate individual module config
            auto moduleValidation = ValidateModuleConfig(moduleConfig);
            for (const auto& issue : moduleValidation.issues) {
                context.AddIssue(issue);
            }
        }
        
        return context;
    }

    ConfigurationValidator::ValidationContext ConfigurationValidator::ValidateModuleDependencies(const std::vector<std::string>& dependencies) {
        ValidationContext context;
        
        // Check for empty dependencies
        for (const auto& dep : dependencies) {
            if (dep.empty()) {
                context.AddIssue(ValidationResult::Error, "dependencies", 
                               "Empty dependency name found", 
                               "Remove empty dependency entries");
            } else if (!IsValidModuleName(dep)) {
                context.AddIssue(ValidationResult::Error, "dependencies", 
                               "Invalid dependency name: " + dep, 
                               "Use valid module name format");
            }
        }
        
        // Check for duplicate dependencies
        std::set<std::string> uniqueDeps;
        for (const auto& dep : dependencies) {
            if (uniqueDeps.find(dep) != uniqueDeps.end()) {
                context.AddIssue(ValidationResult::Warning, "dependencies", 
                               "Duplicate dependency: " + dep, 
                               "Remove duplicate dependency entries");
            }
            uniqueDeps.insert(dep);
        }
        
        return context;
    }

    bool ConfigurationValidator::IsValidModuleName(const std::string& name) {
        if (name.empty() || name.length() > 64) {
            return false;
        }
        
        // Allow alphanumeric characters, hyphens, and underscores
        std::regex namePattern("^[a-zA-Z0-9_-]+$");
        return std::regex_match(name, namePattern);
    }

    bool ConfigurationValidator::IsValidVersion(const std::string& version) {
        if (version.empty() || version.length() > 32) {
            return false;
        }
        
        // Basic version pattern (allows semantic versioning and other common formats)
        std::regex versionPattern("^[0-9]+([.-][0-9a-zA-Z]+)*$");
        return std::regex_match(version, versionPattern);
    }

    bool ConfigurationValidator::IsValidParameter(const std::string& key, const std::string& value) {
        // Basic validation - key should not be empty and not too long
        if (key.empty() || key.length() > 128 || value.length() > 1024) {
            return false;
        }
        
        // Key should be alphanumeric with underscores and dots
        std::regex keyPattern("^[a-zA-Z0-9_.]+$");
        return std::regex_match(key, keyPattern);
    }
}