#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>

namespace GameEngine {

    /**
     * @brief Base exception class for model loading errors
     * 
     * Provides detailed error categorization and context information
     * for troubleshooting model loading issues.
     */
    class ModelLoadingException : public std::runtime_error {
    public:
        /**
         * @brief Categories of model loading errors
         */
        enum class ErrorType {
            FileNotFound,           // File does not exist or cannot be accessed
            UnsupportedFormat,      // File format is not supported
            CorruptedFile,          // File is corrupted or malformed
            OutOfMemory,           // Insufficient memory for loading
            InvalidData,           // Data validation failed
            ImporterError,         // Assimp or other importer error
            PermissionDenied,      // File access permission denied
            NetworkError,          // Network-related error (for remote files)
            TimeoutError,          // Loading operation timed out
            DependencyError,       // Missing dependency or linked file
            ValidationError,       // Model validation failed
            ConversionError,       // Format conversion failed
            UnknownError          // Unspecified error
        };

        /**
         * @brief Severity levels for errors
         */
        enum class Severity {
            Info,       // Informational message
            Warning,    // Warning that doesn't prevent loading
            Error,      // Error that prevents loading
            Critical    // Critical error that may affect system stability
        };

        /**
         * @brief Context information for error diagnosis
         */
        struct ErrorContext {
            std::string filepath;                           // File being loaded
            std::string formatHint;                        // Detected or expected format
            size_t fileSize = 0;                          // File size in bytes
            std::chrono::milliseconds loadingTime{0};      // Time spent loading
            std::string systemInfo;                        // System/environment info
            std::vector<std::string> additionalInfo;       // Additional context
        };

    public:
        /**
         * @brief Construct a ModelLoadingException
         * @param type Error type category
         * @param message Detailed error message
         * @param filepath Path to the file being loaded (optional)
         * @param severity Error severity level
         */
        ModelLoadingException(ErrorType type, 
                            const std::string& message, 
                            const std::string& filepath = "",
                            Severity severity = Severity::Error);

        /**
         * @brief Construct with full context information
         */
        ModelLoadingException(ErrorType type,
                            const std::string& message,
                            const ErrorContext& context,
                            Severity severity = Severity::Error);

        // Accessors
        ErrorType GetErrorType() const { return m_errorType; }
        Severity GetSeverity() const { return m_severity; }
        const std::string& GetFilePath() const { return m_context.filepath; }
        const ErrorContext& GetContext() const { return m_context; }
        ErrorContext& GetContext() { return m_context; }

        // Utility methods
        std::string GetErrorTypeString() const;
        std::string GetSeverityString() const;
        std::string GetDetailedMessage() const;
        bool IsRecoverable() const;

        // Context manipulation
        void AddContextInfo(const std::string& info);
        void SetSystemInfo(const std::string& info);
        void SetLoadingTime(std::chrono::milliseconds time);

    private:
        ErrorType m_errorType;
        Severity m_severity;
        ErrorContext m_context;

        std::string FormatMessage(const std::string& message) const;
    };

    /**
     * @brief Specialized exception for file validation errors
     */
    class ModelValidationException : public ModelLoadingException {
    public:
        /**
         * @brief Validation error details
         */
        struct ValidationError {
            std::string component;      // Component that failed validation
            std::string description;    // Description of the validation failure
            std::string suggestion;     // Suggested fix or workaround
            bool isCritical = false;   // Whether this prevents loading
        };

    public:
        ModelValidationException(const std::string& message,
                               const std::string& filepath = "",
                               const std::vector<ValidationError>& errors = {});

        const std::vector<ValidationError>& GetValidationErrors() const { return m_validationErrors; }
        void AddValidationError(const ValidationError& error);
        std::string GetValidationSummary() const;
        bool HasCriticalErrors() const;

    private:
        std::vector<ValidationError> m_validationErrors;
    };

    /**
     * @brief Specialized exception for file corruption detection
     */
    class ModelCorruptionException : public ModelLoadingException {
    public:
        /**
         * @brief Types of corruption detected
         */
        enum class CorruptionType {
            InvalidHeader,          // File header is invalid
            TruncatedFile,         // File appears to be truncated
            InvalidChecksum,       // Checksum validation failed
            MalformedData,         // Data structure is malformed
            MissingData,           // Required data sections missing
            InconsistentData,      // Data inconsistencies detected
            UnknownCorruption      // Corruption detected but type unknown
        };

    public:
        ModelCorruptionException(const std::string& message,
                               const std::string& filepath,
                               CorruptionType corruptionType,
                               size_t corruptionOffset = 0);

        CorruptionType GetCorruptionType() const { return m_corruptionType; }
        size_t GetCorruptionOffset() const { return m_corruptionOffset; }
        std::string GetCorruptionTypeString() const;
        std::string GetRecoveryAdvice() const;

    private:
        CorruptionType m_corruptionType;
        size_t m_corruptionOffset;
    };

    /**
     * @brief Exception factory for creating appropriate exception types
     */
    class ModelExceptionFactory {
    public:
        // Factory methods for common error scenarios
        static ModelLoadingException CreateFileNotFoundError(const std::string& filepath);
        static ModelLoadingException CreateUnsupportedFormatError(const std::string& filepath, const std::string& format);
        static ModelLoadingException CreateOutOfMemoryError(const std::string& filepath, size_t requestedBytes);
        static ModelLoadingException CreateImporterError(const std::string& filepath, const std::string& importerMessage);
        static ModelValidationException CreateValidationError(const std::string& filepath, const std::vector<ModelValidationException::ValidationError>& errors);
        static ModelCorruptionException CreateCorruptionError(const std::string& filepath, ModelCorruptionException::CorruptionType type, size_t offset = 0);

        // Context enhancement
        static void EnhanceWithSystemInfo(ModelLoadingException& exception);
        static void EnhanceWithFileInfo(ModelLoadingException& exception, const std::string& filepath);
        static void EnhanceWithTimingInfo(ModelLoadingException& exception, std::chrono::milliseconds loadingTime);
    };

    /**
     * @brief Error recovery strategies
     */
    class ModelErrorRecovery {
    public:
        /**
         * @brief Recovery strategies for different error types
         */
        enum class RecoveryStrategy {
            None,                   // No recovery possible
            RetryWithDifferentFlags, // Retry with different loading flags
            FallbackToDefault,      // Use default/fallback model
            SimplifyModel,          // Try to load simplified version
            SkipCorruptedParts,     // Skip corrupted parts and load rest
            ConvertFormat,          // Try format conversion
            RepairFile             // Attempt file repair
        };

        /**
         * @brief Recovery attempt result
         */
        struct RecoveryResult {
            bool success = false;
            RecoveryStrategy strategyUsed = RecoveryStrategy::None;
            std::string message;
            std::shared_ptr<class Model> recoveredModel;
        };

    public:
        static std::vector<RecoveryStrategy> GetRecoveryStrategies(const ModelLoadingException& exception);
        static RecoveryResult AttemptRecovery(const ModelLoadingException& exception, RecoveryStrategy strategy);
        static bool IsRecoveryPossible(const ModelLoadingException& exception);
        static std::string GetRecoveryStrategyDescription(RecoveryStrategy strategy);
    };

} // namespace GameEngine