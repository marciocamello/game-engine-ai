#include "Graphics/ShaderDiagnostics.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Core/Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>

namespace GameEngine {

    // ShaderDiagnostics implementation
    ShaderDiagnostics& ShaderDiagnostics::GetInstance() {
        static ShaderDiagnostics instance;
        return instance;
    }

    void ShaderDiagnostics::LogOperation(ShaderOperation operation, const std::string& shaderName, 
                                        const std::string& message, DiagnosticSeverity severity) {
        auto& instance = GetInstance();
        
        DiagnosticInfo diagnostic(severity, operation, shaderName, message);
        
        // Add verbose details if enabled
        if (instance.m_verboseLogging) {
            diagnostic.context["timestamp"] = instance.FormatTimestamp(diagnostic.timestamp);
            diagnostic.context["thread_id"] = std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }
        
        instance.AddDiagnostic(diagnostic);
        
        // Log to engine logger based on severity
        std::string logMessage = "[" + instance.OperationToString(operation) + "] " + 
                                shaderName + ": " + message;
        
        switch (severity) {
            case DiagnosticSeverity::Critical:
            case DiagnosticSeverity::Error:
                LOG_ERROR(logMessage);
                break;
            case DiagnosticSeverity::Warning:
                LOG_WARNING(logMessage);
                break;
            case DiagnosticSeverity::Performance:
                LOG_INFO("[PERF] " + logMessage);
                break;
            case DiagnosticSeverity::Debug:
                if (instance.m_verboseLogging) {
                    LOG_INFO("[DEBUG] " + logMessage);
                }
                break;
            default:
                LOG_INFO(logMessage);
                break;
        }
    }

    void ShaderDiagnostics::LogError(const std::string& shaderName, const std::string& error, 
                                    const std::string& suggestion) {
        DiagnosticInfo diagnostic(DiagnosticSeverity::Error, ShaderOperation::Validation, 
                                 shaderName, error);
        diagnostic.suggestion = suggestion.empty() ? GetErrorSuggestion(error) : suggestion;
        diagnostic.detailedDescription = "Shader error occurred during operation";
        
        GetInstance().AddDiagnostic(diagnostic);
    }

    void ShaderDiagnostics::LogWarning(const std::string& shaderName, const std::string& warning, 
                                      const std::string& suggestion) {
        DiagnosticInfo diagnostic(DiagnosticSeverity::Warning, ShaderOperation::Validation, 
                                 shaderName, warning);
        diagnostic.suggestion = suggestion;
        diagnostic.detailedDescription = "Shader warning - potential issue detected";
        
        GetInstance().AddDiagnostic(diagnostic);
    }

    void ShaderDiagnostics::LogInfo(const std::string& shaderName, const std::string& info) {
        LogOperation(ShaderOperation::Validation, shaderName, info, DiagnosticSeverity::Info);
    }

    void ShaderDiagnostics::LogPerformance(const std::string& shaderName, const std::string& metric, 
                                          double value, const std::string& unit) {
        std::string message = metric + ": " + std::to_string(value) + unit;
        DiagnosticInfo diagnostic(DiagnosticSeverity::Performance, ShaderOperation::PerformanceCheck, 
                                 shaderName, message);
        
        diagnostic.context["metric"] = metric;
        diagnostic.context["value"] = std::to_string(value);
        diagnostic.context["unit"] = unit;
        diagnostic.suggestion = GetPerformanceSuggestion(shaderName, metric, value);
        
        GetInstance().AddDiagnostic(diagnostic);
    }

    void ShaderDiagnostics::LogCompilation(const std::string& shaderName, bool success, 
                                          double timeMs, const std::string& log) {
        DiagnosticSeverity severity = success ? DiagnosticSeverity::Info : DiagnosticSeverity::Error;
        std::string message = success ? "Compilation successful" : "Compilation failed";
        
        if (timeMs > 0) {
            message += " (" + std::to_string(timeMs) + "ms)";
        }
        
        DiagnosticInfo diagnostic(severity, ShaderOperation::Compilation, shaderName, message);
        diagnostic.context["success"] = success ? "true" : "false";
        diagnostic.context["time_ms"] = std::to_string(timeMs);
        
        if (!log.empty()) {
            diagnostic.detailedDescription = log;
            if (!success) {
                diagnostic.suggestion = GetErrorSuggestion(log);
            }
        }
        
        if (!success && timeMs > 1000) { // Slow compilation
            diagnostic.suggestion += " Consider simplifying shader complexity for faster compilation.";
        }
        
        GetInstance().AddDiagnostic(diagnostic);
        
        // Update shader state
        auto& instance = GetInstance();
        auto it = instance.m_shaderStates.find(shaderName);
        if (it != instance.m_shaderStates.end()) {
            it->second.lastCompileTime = std::chrono::system_clock::now();
            it->second.totalCompileTime += timeMs;
            if (!success) {
                it->second.lastCompileError = log;
            }
        }
    }

    void ShaderDiagnostics::LogLinking(const std::string& shaderName, bool success, 
                                      double timeMs, const std::string& log) {
        DiagnosticSeverity severity = success ? DiagnosticSeverity::Info : DiagnosticSeverity::Error;
        std::string message = success ? "Linking successful" : "Linking failed";
        
        if (timeMs > 0) {
            message += " (" + std::to_string(timeMs) + "ms)";
        }
        
        DiagnosticInfo diagnostic(severity, ShaderOperation::Linking, shaderName, message);
        diagnostic.context["success"] = success ? "true" : "false";
        diagnostic.context["time_ms"] = std::to_string(timeMs);
        
        if (!log.empty()) {
            diagnostic.detailedDescription = log;
            if (!success) {
                diagnostic.suggestion = GetErrorSuggestion(log);
            }
        }
        
        GetInstance().AddDiagnostic(diagnostic);
        
        // Update shader state
        auto& instance = GetInstance();
        auto it = instance.m_shaderStates.find(shaderName);
        if (it != instance.m_shaderStates.end()) {
            it->second.lastLinkTime = std::chrono::system_clock::now();
            it->second.totalLinkTime += timeMs;
            it->second.isValid = success;
            if (!success) {
                it->second.lastLinkError = log;
            }
        }
    }

    void ShaderDiagnostics::RegisterShader(const std::string& shaderName, uint32_t programId) {
        auto& instance = GetInstance();
        
        ShaderStateInfo state;
        state.name = shaderName;
        state.programId = programId;
        state.isValid = (programId != 0);
        
        instance.m_shaderStates[shaderName] = state;
        
        LogOperation(ShaderOperation::Validation, shaderName, "Shader registered for diagnostics");
    }

    void ShaderDiagnostics::UnregisterShader(const std::string& shaderName) {
        auto& instance = GetInstance();
        instance.m_shaderStates.erase(shaderName);
        
        LogOperation(ShaderOperation::Validation, shaderName, "Shader unregistered from diagnostics");
    }

    ShaderStateInfo ShaderDiagnostics::GetShaderState(const std::string& shaderName) {
        auto& instance = GetInstance();
        auto it = instance.m_shaderStates.find(shaderName);
        if (it != instance.m_shaderStates.end()) {
            return it->second;
        }
        return ShaderStateInfo();
    }

    std::vector<std::string> ShaderDiagnostics::GetTrackedShaders() {
        auto& instance = GetInstance();
        std::vector<std::string> shaders;
        for (const auto& [name, state] : instance.m_shaderStates) {
            shaders.push_back(name);
        }
        return shaders;
    }

    std::vector<DiagnosticInfo> ShaderDiagnostics::GetDiagnostics(DiagnosticSeverity minSeverity) {
        auto& instance = GetInstance();
        std::vector<DiagnosticInfo> filtered;
        
        for (const auto& diagnostic : instance.m_diagnostics) {
            if (static_cast<int>(diagnostic.severity) >= static_cast<int>(minSeverity)) {
                filtered.push_back(diagnostic);
            }
        }
        
        return filtered;
    }

    std::vector<DiagnosticInfo> ShaderDiagnostics::GetShaderDiagnostics(const std::string& shaderName, 
                                                                        DiagnosticSeverity minSeverity) {
        auto& instance = GetInstance();
        std::vector<DiagnosticInfo> filtered;
        
        for (const auto& diagnostic : instance.m_diagnostics) {
            if (diagnostic.shaderName == shaderName && 
                static_cast<int>(diagnostic.severity) >= static_cast<int>(minSeverity)) {
                filtered.push_back(diagnostic);
            }
        }
        
        return filtered;
    }

    std::string ShaderDiagnostics::GenerateDiagnosticReport() {
        auto& instance = GetInstance();
        std::stringstream report;
        
        report << "=== Shader Diagnostics Report ===\n";
        report << "Generated: " << instance.FormatTimestamp(std::chrono::system_clock::now()) << "\n";
        report << "Total Diagnostics: " << instance.m_diagnostics.size() << "\n";
        report << "Tracked Shaders: " << instance.m_shaderStates.size() << "\n\n";
        
        // Shader states
        report << "Shader States:\n";
        for (const auto& [name, state] : instance.m_shaderStates) {
            report << "  " << name << " (ID: " << state.programId << ")\n";
            report << "    Valid: " << (state.isValid ? "Yes" : "No") << "\n";
            report << "    Active: " << (state.isActive ? "Yes" : "No") << "\n";
            report << "    Use Count: " << state.useCount << "\n";
            report << "    Memory Usage: " << state.memoryUsage << " bytes\n";
        }
        
        return report.str();
    }

    std::string ShaderDiagnostics::GenerateShaderReport(const std::string& shaderName) {
        auto diagnostics = GetShaderDiagnostics(shaderName);
        auto state = GetShaderState(shaderName);
        
        std::stringstream report;
        report << "=== Shader Report: " << shaderName << " ===\n";
        report << "Program ID: " << state.programId << "\n";
        report << "Valid: " << (state.isValid ? "Yes" : "No") << "\n";
        report << "Diagnostics Count: " << diagnostics.size() << "\n\n";
        
        if (!diagnostics.empty()) {
            report << "Recent Diagnostics:\n";
            for (const auto& diagnostic : diagnostics) {
                report << "  [" << GetInstance().SeverityToString(diagnostic.severity) << "] ";
                report << diagnostic.message << "\n";
            }
        }
        
        return report.str();
    }

    void ShaderDiagnostics::SetDiagnosticCallback(DiagnosticCallback callback) {
        GetInstance().m_callback = callback;
    }

    void ShaderDiagnostics::SetMinimumSeverity(DiagnosticSeverity severity) {
        GetInstance().m_minSeverity = severity;
    }

    void ShaderDiagnostics::EnableVerboseLogging(bool enable) {
        GetInstance().m_verboseLogging = enable;
        LOG_INFO("Shader diagnostics verbose logging " + std::string(enable ? "enabled" : "disabled"));
    }

    void ShaderDiagnostics::ClearDiagnostics() {
        GetInstance().m_diagnostics.clear();
        LOG_INFO("Shader diagnostics cleared");
    }

    std::string ShaderDiagnostics::GetErrorSuggestion(const std::string& errorMessage) {
        // Basic pattern matching for error suggestions
        std::string lowerError = errorMessage;
        std::transform(lowerError.begin(), lowerError.end(), lowerError.begin(), ::tolower);
        
        if (lowerError.find("undeclared") != std::string::npos) {
            return "Check for typos in variable names and ensure all variables are declared";
        } else if (lowerError.find("syntax error") != std::string::npos) {
            return "Check for missing semicolons, brackets, or incorrect syntax";
        } else if (lowerError.find("version") != std::string::npos) {
            return "Ensure #version directive is the first line in your shader";
        } else if (lowerError.find("linking") != std::string::npos) {
            return "Check that vertex and fragment shader interfaces match";
        }
        
        return "Review shader source code for common GLSL errors";
    }

    std::string ShaderDiagnostics::GetPerformanceSuggestion(const std::string& shaderName, 
                                                           const std::string& metric, double value) {
        if (metric == "frame_time" && value > 16.67) {
            return "Frame time exceeds 60 FPS target - consider shader optimization";
        } else if (metric == "compile_time" && value > 1000) {
            return "Slow compilation detected - consider simplifying shader complexity";
        } else if (metric == "uniform_updates" && value > 100) {
            return "High uniform update frequency - consider using Uniform Buffer Objects";
        } else if (metric == "texture_bindings" && value > 16) {
            return "Many texture bindings - consider texture atlasing or arrays";
        } else if (metric == "memory_usage" && value > 1024 * 1024) {
            return "High memory usage - optimize textures and data structures";
        }
        
        return "Monitor performance metrics and optimize as needed";
    }

    // Private helper methods
    void ShaderDiagnostics::AddDiagnostic(const DiagnosticInfo& diagnostic) {
        if (static_cast<int>(diagnostic.severity) < static_cast<int>(m_minSeverity)) {
            return;
        }
        
        m_diagnostics.push_back(diagnostic);
        
        // Trigger callback if set
        if (m_callback) {
            m_callback(diagnostic);
        }
        
        // Trim history if needed
        TrimDiagnosticHistory();
    }

    std::string ShaderDiagnostics::SeverityToString(DiagnosticSeverity severity) const {
        switch (severity) {
            case DiagnosticSeverity::Info: return "INFO";
            case DiagnosticSeverity::Warning: return "WARNING";
            case DiagnosticSeverity::Error: return "ERROR";
            case DiagnosticSeverity::Critical: return "CRITICAL";
            case DiagnosticSeverity::Debug: return "DEBUG";
            case DiagnosticSeverity::Performance: return "PERFORMANCE";
            default: return "UNKNOWN";
        }
    }

    std::string ShaderDiagnostics::OperationToString(ShaderOperation operation) const {
        switch (operation) {
            case ShaderOperation::Compilation: return "COMPILATION";
            case ShaderOperation::Linking: return "LINKING";
            case ShaderOperation::UniformUpdate: return "UNIFORM_UPDATE";
            case ShaderOperation::TextureBinding: return "TEXTURE_BINDING";
            case ShaderOperation::StateChange: return "STATE_CHANGE";
            case ShaderOperation::Validation: return "VALIDATION";
            case ShaderOperation::HotReload: return "HOT_RELOAD";
            case ShaderOperation::VariantCreation: return "VARIANT_CREATION";
            case ShaderOperation::CacheAccess: return "CACHE_ACCESS";
            case ShaderOperation::PerformanceCheck: return "PERFORMANCE_CHECK";
            default: return "UNKNOWN";
        }
    }

    std::string ShaderDiagnostics::FormatTimestamp(const std::chrono::system_clock::time_point& timestamp) const {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    void ShaderDiagnostics::TrimDiagnosticHistory() {
        if (m_diagnostics.size() > m_maxHistory) {
            size_t toRemove = m_diagnostics.size() - m_maxHistory;
            m_diagnostics.erase(m_diagnostics.begin(), m_diagnostics.begin() + toRemove);
        }
    }

}