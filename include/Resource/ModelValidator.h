#pragma once

#include "Resource/ModelLoadingException.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace GameEngine {

    /**
     * @brief Comprehensive model validation and diagnostic system
     * 
     * Provides detailed validation, error checking, and diagnostic information
     * for troubleshooting model loading issues.
     */
    class ModelValidator {
    public:
        /**
         * @brief Validation severity levels
         */
        enum class ValidationSeverity {
            Info,       // Informational message
            Warning,    // Issue that doesn't prevent usage but should be noted
            Error,      // Issue that may cause problems
            Critical    // Issue that prevents proper usage
        };

        /**
         * @brief Types of validation checks
         */
        enum class ValidationType {
            FileStructure,      // File format and structure validation
            GeometryData,       // Mesh geometry validation
            MaterialData,       // Material and texture validation
            AnimationData,      // Animation and rigging validation
            Performance,        // Performance-related checks
            Compatibility,      // Engine compatibility checks
            Standards          // Industry standard compliance
        };

        /**
         * @brief Individual validation issue
         */
        struct ValidationIssue {
            ValidationType type;
            ValidationSeverity severity;
            std::string component;      // Component that has the issue
            std::string description;    // Description of the issue
            std::string suggestion;     // Suggested fix
            std::string location;       // Location in file/model where issue occurs
            size_t lineNumber = 0;      // Line number (for text formats)
            size_t byteOffset = 0;      // Byte offset (for binary formats)
            
            // Additional context
            std::unordered_map<std::string, std::string> metadata;
        };

        /**
         * @brief Comprehensive validation report
         */
        struct ValidationReport {
            std::string filepath;
            std::string format;
            bool isValid = true;
            std::chrono::milliseconds validationTime{0};
            
            std::vector<ValidationIssue> issues;
            
            // Statistics
            size_t totalVertices = 0;
            size_t totalTriangles = 0;
            size_t totalMeshes = 0;
            size_t totalMaterials = 0;
            size_t totalTextures = 0;
            size_t totalAnimations = 0;
            size_t memoryUsageBytes = 0;
            
            // Performance metrics
            float averageTriangleArea = 0.0f;
            float minTriangleArea = 0.0f;
            float maxTriangleArea = 0.0f;
            size_t degenerateTriangles = 0;
            size_t duplicateVertices = 0;
            float cacheEfficiency = 0.0f; // ACMR score
            
            // Counts by severity
            size_t infoCount = 0;
            size_t warningCount = 0;
            size_t errorCount = 0;
            size_t criticalCount = 0;
        };

        /**
         * @brief Diagnostic information for troubleshooting
         */
        struct DiagnosticInfo {
            std::string filepath;
            std::string format;
            std::chrono::system_clock::time_point timestamp;
            
            // File information
            size_t fileSize = 0;
            std::string fileHash;
            std::chrono::system_clock::time_point lastModified;
            
            // System information
            std::string platform;
            std::string engineVersion;
            size_t availableMemory = 0;
            
            // Loading context
            std::string loadingFlags;
            std::chrono::milliseconds loadingTime{0};
            std::string errorMessage;
            std::string stackTrace;
            
            // Environment
            std::string workingDirectory;
            std::vector<std::string> searchPaths;
            std::unordered_map<std::string, std::string> environmentVars;
        };

    public:
        ModelValidator();
        ~ModelValidator();

        // Main validation interface
        ValidationReport ValidateFile(const std::string& filepath);
        ValidationReport ValidateModel(std::shared_ptr<Model> model);
        ValidationReport ValidateMesh(std::shared_ptr<Mesh> mesh, const std::string& meshName = "");

        // Specific validation methods
        std::vector<ValidationIssue> ValidateFileStructure(const std::string& filepath);
        std::vector<ValidationIssue> ValidateGeometry(std::shared_ptr<Mesh> mesh, const std::string& meshName = "");
        std::vector<ValidationIssue> ValidateMaterials(std::shared_ptr<Model> model);
        std::vector<ValidationIssue> ValidateAnimations(std::shared_ptr<Model> model);
        std::vector<ValidationIssue> ValidatePerformance(std::shared_ptr<Model> model);

        // Diagnostic tools
        DiagnosticInfo GenerateDiagnosticInfo(const std::string& filepath, const std::string& errorMessage = "");
        std::string GenerateDiagnosticReport(const DiagnosticInfo& info);
        void LogDiagnosticInfo(const DiagnosticInfo& info);

        // Report generation
        std::string GenerateValidationReport(const ValidationReport& report);
        std::string GenerateDetailedReport(const ValidationReport& report);
        void LogValidationReport(const ValidationReport& report);
        bool SaveReportToFile(const ValidationReport& report, const std::string& outputPath);

        // Configuration
        void SetValidationLevel(ValidationSeverity minSeverity);
        void EnableValidationType(ValidationType type, bool enabled = true);
        void SetPerformanceThresholds(size_t maxVertices, size_t maxTriangles, float maxMemoryMB);

        // Utility methods
        static std::string GetValidationTypeString(ValidationType type);
        static std::string GetValidationSeverityString(ValidationSeverity severity);
        static ValidationSeverity GetSeverityFromString(const std::string& severityStr);

    private:
        ValidationSeverity m_minSeverity = ValidationSeverity::Info;
        std::unordered_map<ValidationType, bool> m_enabledTypes;
        
        // Performance thresholds
        size_t m_maxVertices = 100000;
        size_t m_maxTriangles = 200000;
        float m_maxMemoryMB = 100.0f;
        
        // Internal validation methods
        void ValidateTriangleQuality(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName);
        void ValidateVertexData(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName);
        void ValidateIndexData(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName);
        void ValidateNormals(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName);
        void ValidateTextureCoordinates(std::shared_ptr<Mesh> mesh, std::vector<ValidationIssue>& issues, const std::string& meshName);
        
        // File format specific validation
        std::vector<ValidationIssue> ValidateOBJFile(const std::string& filepath);
        std::vector<ValidationIssue> ValidateFBXFile(const std::string& filepath);
        std::vector<ValidationIssue> ValidateGLTFFile(const std::string& filepath);
        
        // Utility methods
        std::string CalculateFileHash(const std::string& filepath);
        float CalculateTriangleArea(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3);
        bool IsTriangleDegenerate(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3, float epsilon = 1e-6f);
        size_t CountDuplicateVertices(std::shared_ptr<Mesh> mesh, float epsilon = 1e-6f);
        float CalculateCacheEfficiency(std::shared_ptr<Mesh> mesh);
        
        // Report formatting
        std::string FormatIssue(const ValidationIssue& issue, bool detailed = false);
        std::string FormatStatistics(const ValidationReport& report);
        void UpdateReportCounts(ValidationReport& report);
    };

    /**
     * @brief Model diagnostic logger for detailed error logging
     */
    class ModelDiagnosticLogger {
    public:
        /**
         * @brief Log levels for diagnostic information
         */
        enum class LogLevel {
            Trace,      // Very detailed tracing information
            Debug,      // Debug information
            Info,       // General information
            Warning,    // Warning messages
            Error,      // Error messages
            Critical    // Critical error messages
        };

        /**
         * @brief Log entry with detailed context
         */
        struct LogEntry {
            LogLevel level;
            std::chrono::system_clock::time_point timestamp;
            std::string message;
            std::string component;
            std::string filepath;
            size_t lineNumber = 0;
            std::string function;
            std::unordered_map<std::string, std::string> context;
        };

    public:
        static ModelDiagnosticLogger& GetInstance();

        // Logging methods
        void LogTrace(const std::string& message, const std::string& component = "", const std::string& filepath = "");
        void LogDebug(const std::string& message, const std::string& component = "", const std::string& filepath = "");
        void LogInfo(const std::string& message, const std::string& component = "", const std::string& filepath = "");
        void LogWarning(const std::string& message, const std::string& component = "", const std::string& filepath = "");
        void LogError(const std::string& message, const std::string& component = "", const std::string& filepath = "");
        void LogCritical(const std::string& message, const std::string& component = "", const std::string& filepath = "");

        // Context-aware logging
        void LogWithContext(LogLevel level, const std::string& message, const std::unordered_map<std::string, std::string>& context);
        void LogException(const ModelLoadingException& exception);
        void LogValidationIssue(const ModelValidator::ValidationIssue& issue);

        // Configuration
        void SetLogLevel(LogLevel minLevel);
        void SetOutputFile(const std::string& filepath);
        void EnableConsoleOutput(bool enabled);
        void EnableFileOutput(bool enabled);

        // Log management
        std::vector<LogEntry> GetRecentEntries(size_t count = 100);
        void ClearLog();
        void FlushLog();

        // Utility
        static std::string GetLogLevelString(LogLevel level);

    private:
        ModelDiagnosticLogger() = default;
        
        LogLevel m_minLevel = LogLevel::Info;
        std::string m_outputFile;
        bool m_consoleOutput = true;
        bool m_fileOutput = false;
        
        std::vector<LogEntry> m_entries;
        std::mutex m_logMutex;
        
        void WriteLogEntry(const LogEntry& entry);
        std::string FormatLogEntry(const LogEntry& entry);
    };

} // namespace GameEngine

// Convenience macros for diagnostic logging
#define MODEL_LOG_TRACE(msg, component, filepath) \
    GameEngine::ModelDiagnosticLogger::GetInstance().LogTrace(msg, component, filepath)

#define MODEL_LOG_DEBUG(msg, component, filepath) \
    GameEngine::ModelDiagnosticLogger::GetInstance().LogDebug(msg, component, filepath)

#define MODEL_LOG_INFO(msg, component, filepath) \
    GameEngine::ModelDiagnosticLogger::GetInstance().LogInfo(msg, component, filepath)

#define MODEL_LOG_WARNING(msg, component, filepath) \
    GameEngine::ModelDiagnosticLogger::GetInstance().LogWarning(msg, component, filepath)

#define MODEL_LOG_ERROR(msg, component, filepath) \
    GameEngine::ModelDiagnosticLogger::GetInstance().LogError(msg, component, filepath)

#define MODEL_LOG_CRITICAL(msg, component, filepath) \
    GameEngine::ModelDiagnosticLogger::GetInstance().LogCritical(msg, component, filepath)