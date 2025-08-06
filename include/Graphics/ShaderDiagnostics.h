#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <chrono>

namespace GameEngine {
    class Shader;
    class Material;

    /**
     * Shader operation types for detailed logging
     * Requirements: 10.2, 10.7, 8.5
     */
    enum class ShaderOperation {
        Compilation,
        Linking,
        UniformUpdate,
        TextureBinding,
        StateChange,
        Validation,
        HotReload,
        VariantCreation,
        CacheAccess,
        PerformanceCheck
    };

    /**
     * Diagnostic severity levels
     * Requirements: 10.2, 10.7, 8.5
     */
    enum class DiagnosticSeverity {
        Info,
        Warning,
        Error,
        Critical,
        Debug,
        Performance
    };

    /**
     * Detailed diagnostic information
     * Requirements: 10.2, 10.7, 8.5
     */
    struct DiagnosticInfo {
        DiagnosticSeverity severity;
        ShaderOperation operation;
        std::string shaderName;
        std::string message;
        std::string detailedDescription;
        std::string suggestion;
        std::string sourceLocation; // File:line for shader source issues
        std::chrono::system_clock::time_point timestamp;
        
        // Additional context
        std::unordered_map<std::string, std::string> context;
        
        DiagnosticInfo() : timestamp(std::chrono::system_clock::now()) {}
        
        DiagnosticInfo(DiagnosticSeverity sev, ShaderOperation op, const std::string& shader, 
                      const std::string& msg)
            : severity(sev), operation(op), shaderName(shader), message(msg),
              timestamp(std::chrono::system_clock::now()) {}
    };

    /**
     * Shader state tracking for diagnostics
     * Requirements: 10.2, 10.7, 8.5
     */
    struct ShaderStateInfo {
        std::string name;
        uint32_t programId;
        bool isValid;
        bool isActive;
        
        // Compilation state
        std::string lastCompileError;
        std::string lastLinkError;
        std::chrono::system_clock::time_point lastCompileTime;
        std::chrono::system_clock::time_point lastLinkTime;
        
        // Usage statistics
        uint64_t useCount;
        uint64_t uniformUpdateCount;
        uint64_t textureBindCount;
        uint64_t stateChangeCount;
        
        // Performance metrics
        double totalCompileTime;
        double totalLinkTime;
        double averageFrameTime;
        
        // Resource usage
        int activeUniforms;
        int activeAttributes;
        int textureUnits;
        size_t memoryUsage;
        
        ShaderStateInfo() : programId(0), isValid(false), isActive(false), 
                           useCount(0), uniformUpdateCount(0), textureBindCount(0), 
                           stateChangeCount(0), totalCompileTime(0.0), totalLinkTime(0.0),
                           averageFrameTime(0.0), activeUniforms(0), activeAttributes(0),
                           textureUnits(0), memoryUsage(0) {}
    };

    /**
     * Comprehensive shader logging and diagnostics system
     * Requirements: 10.2, 10.7, 8.5
     */
    class ShaderDiagnostics {
    public:
        using DiagnosticCallback = std::function<void(const DiagnosticInfo&)>;
        
        // Singleton access
        static ShaderDiagnostics& GetInstance();
        
        // Diagnostic logging
        static void LogOperation(ShaderOperation operation, const std::string& shaderName, 
                               const std::string& message, DiagnosticSeverity severity = DiagnosticSeverity::Info);
        static void LogError(const std::string& shaderName, const std::string& error, 
                           const std::string& suggestion = "");
        static void LogWarning(const std::string& shaderName, const std::string& warning, 
                             const std::string& suggestion = "");
        static void LogInfo(const std::string& shaderName, const std::string& info);
        static void LogPerformance(const std::string& shaderName, const std::string& metric, 
                                  double value, const std::string& unit = "");
        
        // Detailed operation logging
        static void LogCompilation(const std::string& shaderName, bool success, 
                                 double timeMs, const std::string& log = "");
        static void LogLinking(const std::string& shaderName, bool success, 
                             double timeMs, const std::string& log = "");
        
        // Shader state tracking
        static void RegisterShader(const std::string& shaderName, uint32_t programId);
        static void UnregisterShader(const std::string& shaderName);
        static ShaderStateInfo GetShaderState(const std::string& shaderName);
        static std::vector<std::string> GetTrackedShaders();
        
        // Diagnostic retrieval
        static std::vector<DiagnosticInfo> GetDiagnostics(DiagnosticSeverity minSeverity = DiagnosticSeverity::Info);
        static std::vector<DiagnosticInfo> GetShaderDiagnostics(const std::string& shaderName, 
                                                               DiagnosticSeverity minSeverity = DiagnosticSeverity::Info);
        
        // Report generation
        static std::string GenerateDiagnosticReport();
        static std::string GenerateShaderReport(const std::string& shaderName);
        
        // Diagnostic configuration
        static void SetDiagnosticCallback(DiagnosticCallback callback);
        static void SetMinimumSeverity(DiagnosticSeverity severity);
        static void EnableVerboseLogging(bool enable);
        
        // Diagnostic management
        static void ClearDiagnostics();
        
        // Developer assistance
        static std::string GetErrorSuggestion(const std::string& errorMessage);
        static std::string GetPerformanceSuggestion(const std::string& shaderName, 
                                                   const std::string& metric, double value);
        
    private:
        ShaderDiagnostics() = default;
        ~ShaderDiagnostics() = default;
        ShaderDiagnostics(const ShaderDiagnostics&) = delete;
        ShaderDiagnostics& operator=(const ShaderDiagnostics&) = delete;
        
        // Internal storage
        std::vector<DiagnosticInfo> m_diagnostics;
        std::unordered_map<std::string, ShaderStateInfo> m_shaderStates;
        
        // Configuration
        DiagnosticCallback m_callback;
        DiagnosticSeverity m_minSeverity = DiagnosticSeverity::Info;
        size_t m_maxHistory = 10000;
        bool m_verboseLogging = false;
        
        // Helper methods
        void AddDiagnostic(const DiagnosticInfo& diagnostic);
        std::string SeverityToString(DiagnosticSeverity severity) const;
        std::string OperationToString(ShaderOperation operation) const;
        std::string FormatTimestamp(const std::chrono::system_clock::time_point& timestamp) const;
        void TrimDiagnosticHistory();
    };

}