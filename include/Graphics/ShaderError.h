#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

namespace GameEngine {

    /**
     * Detailed shader error information
     */
    struct ShaderError {
        int lineNumber = -1;
        std::string message;
        std::string shaderType;
        std::string context; // Surrounding code context
        
        ShaderError() = default;
        ShaderError(int line, const std::string& msg, const std::string& type = "", const std::string& ctx = "")
            : lineNumber(line), message(msg), shaderType(type), context(ctx) {}
    };

    /**
     * Exception class for shader compilation errors
     * Requirements: 8.1, 8.4, 10.2
     */
    class ShaderCompilationError : public std::runtime_error {
    public:
        ShaderCompilationError(const std::string& shaderName, const std::string& error, int line = -1);
        ShaderCompilationError(const std::string& shaderName, const std::vector<ShaderError>& errors);

        const std::string& GetShaderName() const { return m_shaderName; }
        int GetLineNumber() const { return m_lineNumber; }
        const std::vector<ShaderError>& GetErrors() const { return m_errors; }
        
        // Get formatted error message with line numbers and context
        std::string GetFormattedError() const;
        
        // Get developer-friendly error message with suggestions
        std::string GetDeveloperMessage() const;

    private:
        std::string m_shaderName;
        int m_lineNumber;
        std::vector<ShaderError> m_errors;
        
        std::string FormatErrorMessage() const;
    };

    /**
     * Shader error handler with callback system
     * Requirements: 8.1, 8.4, 10.2
     */
    class ShaderErrorHandler {
    public:
        using ErrorCallback = std::function<void(const ShaderCompilationError&)>;
        using WarningCallback = std::function<void(const std::string& shaderName, const std::string& warning)>;

        // Error handling methods
        static void HandleCompilationError(const std::string& shaderName, const std::string& log);
        static void HandleLinkingError(const std::string& shaderName, const std::string& log);
        static void HandleRuntimeError(const std::string& shaderName, const std::string& error);
        static void HandleWarning(const std::string& shaderName, const std::string& warning);

        // Callback management
        static void SetErrorCallback(ErrorCallback callback);
        static void SetWarningCallback(WarningCallback callback);
        static void ClearCallbacks();

        // Error parsing utilities
        static std::vector<ShaderError> ParseErrorLog(const std::string& log, const std::string& shaderType = "");
        static std::string ExtractLineContext(const std::string& source, int lineNumber, int contextLines = 2);
        
        // Developer-friendly error suggestions
        static std::string GetErrorSuggestion(const std::string& errorMessage);
        
        // Error statistics
        static void ResetErrorStats();
        static int GetCompilationErrorCount();
        static int GetLinkingErrorCount();
        static int GetRuntimeErrorCount();

    private:
        static ErrorCallback s_errorCallback;
        static WarningCallback s_warningCallback;
        static int s_compilationErrors;
        static int s_linkingErrors;
        static int s_runtimeErrors;

        // Error parsing helpers
        static ShaderError ParseSingleError(const std::string& errorLine, const std::string& shaderType);
        static int ExtractLineNumber(const std::string& errorLine);
        static std::string CleanErrorMessage(const std::string& message);
    };

    /**
     * Shader validation utilities
     * Requirements: 6.2, 6.3, 6.5
     */
    class ShaderValidator {
    public:
        struct ValidationResult {
            bool isValid = true;
            std::vector<std::string> errors;
            std::vector<std::string> warnings;
            std::vector<std::string> suggestions;
        };

        // Shader source validation
        static ValidationResult ValidateShaderSource(const std::string& source, const std::string& shaderType);
        static ValidationResult ValidateVertexShader(const std::string& source);
        static ValidationResult ValidateFragmentShader(const std::string& source);
        static ValidationResult ValidateComputeShader(const std::string& source);

        // Shader program validation
        static ValidationResult ValidateShaderProgram(uint32_t programId);
        
        // Performance analysis
        static ValidationResult AnalyzeShaderPerformance(const std::string& source, const std::string& shaderType);
        
        // Common validation checks
        static bool CheckForCommonMistakes(const std::string& source, std::vector<std::string>& warnings);
        static bool CheckForPerformanceIssues(const std::string& source, std::vector<std::string>& warnings);
        static bool CheckForCompatibilityIssues(const std::string& source, std::vector<std::string>& warnings);

    private:
        // Validation helper methods
        static bool HasRequiredVersionDirective(const std::string& source);
        static bool HasValidMainFunction(const std::string& source, const std::string& shaderType);
        static bool CheckUniformUsage(const std::string& source, std::vector<std::string>& issues);
        static bool CheckTextureUsage(const std::string& source, std::vector<std::string>& issues);
        static bool CheckControlFlow(const std::string& source, std::vector<std::string>& issues);
    };

}