#include "Graphics/ShaderError.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <sstream>
#include <regex>
#include <algorithm>

namespace GameEngine {

    // Static member initialization
    ShaderErrorHandler::ErrorCallback ShaderErrorHandler::s_errorCallback = nullptr;
    ShaderErrorHandler::WarningCallback ShaderErrorHandler::s_warningCallback = nullptr;
    int ShaderErrorHandler::s_compilationErrors = 0;
    int ShaderErrorHandler::s_linkingErrors = 0;
    int ShaderErrorHandler::s_runtimeErrors = 0;

    // ShaderCompilationError implementation
    ShaderCompilationError::ShaderCompilationError(const std::string& shaderName, const std::string& error, int line)
        : std::runtime_error(FormatErrorMessage()), m_shaderName(shaderName), m_lineNumber(line) {
        m_errors.emplace_back(line, error);
    }

    ShaderCompilationError::ShaderCompilationError(const std::string& shaderName, const std::vector<ShaderError>& errors)
        : std::runtime_error(FormatErrorMessage()), m_shaderName(shaderName), m_lineNumber(-1), m_errors(errors) {
        if (!errors.empty() && errors[0].lineNumber != -1) {
            m_lineNumber = errors[0].lineNumber;
        }
    }

    std::string ShaderCompilationError::GetFormattedError() const {
        std::stringstream ss;
        ss << "Shader compilation failed for '" << m_shaderName << "':\n";
        
        for (size_t i = 0; i < m_errors.size(); ++i) {
            const auto& error = m_errors[i];
            ss << "  Error " << (i + 1) << ": ";
            
            if (error.lineNumber != -1) {
                ss << "Line " << error.lineNumber << ": ";
            }
            
            ss << error.message;
            
            if (!error.shaderType.empty()) {
                ss << " (in " << error.shaderType << " shader)";
            }
            
            if (!error.context.empty()) {
                ss << "\n    Context: " << error.context;
            }
            
            if (i < m_errors.size() - 1) {
                ss << "\n";
            }
        }
        
        return ss.str();
    }

    std::string ShaderCompilationError::GetDeveloperMessage() const {
        std::stringstream ss;
        ss << GetFormattedError() << "\n\nSuggestions:\n";
        
        for (const auto& error : m_errors) {
            std::string suggestion = ShaderErrorHandler::GetErrorSuggestion(error.message);
            if (!suggestion.empty()) {
                ss << "  - " << suggestion << "\n";
            }
        }
        
        return ss.str();
    }

    std::string ShaderCompilationError::FormatErrorMessage() const {
        if (m_errors.empty()) {
            return "Shader compilation failed for '" + m_shaderName + "'";
        }
        
        std::stringstream ss;
        ss << "Shader '" << m_shaderName << "' failed: " << m_errors[0].message;
        if (m_errors[0].lineNumber != -1) {
            ss << " (line " << m_errors[0].lineNumber << ")";
        }
        
        return ss.str();
    }

    // ShaderErrorHandler implementation
    void ShaderErrorHandler::HandleCompilationError(const std::string& shaderName, const std::string& log) {
        s_compilationErrors++;
        
        std::vector<ShaderError> errors = ParseErrorLog(log, "compilation");
        ShaderCompilationError error(shaderName, errors);
        
        LOG_ERROR("Shader compilation error: " + error.GetFormattedError());
        
        if (s_errorCallback) {
            s_errorCallback(error);
        }
    }

    void ShaderErrorHandler::HandleLinkingError(const std::string& shaderName, const std::string& log) {
        s_linkingErrors++;
        
        std::vector<ShaderError> errors = ParseErrorLog(log, "linking");
        ShaderCompilationError error(shaderName, errors);
        
        LOG_ERROR("Shader linking error: " + error.GetFormattedError());
        
        if (s_errorCallback) {
            s_errorCallback(error);
        }
    }

    void ShaderErrorHandler::HandleRuntimeError(const std::string& shaderName, const std::string& error) {
        s_runtimeErrors++;
        
        ShaderCompilationError runtimeError(shaderName, error);
        
        LOG_ERROR("Shader runtime error: " + runtimeError.GetFormattedError());
        
        if (s_errorCallback) {
            s_errorCallback(runtimeError);
        }
    }

    void ShaderErrorHandler::HandleWarning(const std::string& shaderName, const std::string& warning) {
        LOG_INFO("Shader warning for '" + shaderName + "': " + warning);
        
        if (s_warningCallback) {
            s_warningCallback(shaderName, warning);
        }
    }

    void ShaderErrorHandler::SetErrorCallback(ErrorCallback callback) {
        s_errorCallback = callback;
    }

    void ShaderErrorHandler::SetWarningCallback(WarningCallback callback) {
        s_warningCallback = callback;
    }

    void ShaderErrorHandler::ClearCallbacks() {
        s_errorCallback = nullptr;
        s_warningCallback = nullptr;
    }

    std::vector<ShaderError> ShaderErrorHandler::ParseErrorLog(const std::string& log, const std::string& shaderType) {
        std::vector<ShaderError> errors;
        std::istringstream stream(log);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            
            ShaderError error = ParseSingleError(line, shaderType);
            if (!error.message.empty()) {
                errors.push_back(error);
            }
        }
        
        return errors;
    }

    std::string ShaderErrorHandler::ExtractLineContext(const std::string& source, int lineNumber, int contextLines) {
        if (lineNumber <= 0) return "";
        
        std::istringstream stream(source);
        std::string line;
        std::vector<std::string> lines;
        
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }
        
        if (lineNumber > static_cast<int>(lines.size())) return "";
        
        std::stringstream context;
        int startLine = std::max(1, lineNumber - contextLines);
        int endLine = std::min(static_cast<int>(lines.size()), lineNumber + contextLines);
        
        for (int i = startLine; i <= endLine; ++i) {
            if (i == lineNumber) {
                context << ">>> " << i << ": " << lines[i - 1] << "\n";
            } else {
                context << "    " << i << ": " << lines[i - 1] << "\n";
            }
        }
        
        return context.str();
    }

    std::string ShaderErrorHandler::GetErrorSuggestion(const std::string& errorMessage) {
        std::string lowerError = errorMessage;
        std::transform(lowerError.begin(), lowerError.end(), lowerError.begin(), ::tolower);
        
        if (lowerError.find("undeclared identifier") != std::string::npos) {
            return "Check for typos in variable names and ensure all variables are declared before use";
        }
        if (lowerError.find("no matching function") != std::string::npos) {
            return "Verify function names and parameter types match the GLSL specification";
        }
        if (lowerError.find("syntax error") != std::string::npos) {
            return "Check for missing semicolons, brackets, or incorrect GLSL syntax";
        }
        if (lowerError.find("version") != std::string::npos) {
            return "Ensure #version directive is the first line and matches your OpenGL context version";
        }
        if (lowerError.find("uniform") != std::string::npos) {
            return "Check uniform variable declarations and ensure they match between shader and application";
        }
        if (lowerError.find("varying") != std::string::npos || lowerError.find("attribute") != std::string::npos) {
            return "Use 'in' and 'out' keywords instead of 'varying' and 'attribute' in modern GLSL";
        }
        if (lowerError.find("texture") != std::string::npos) {
            return "Use texture() function instead of texture2D() in modern GLSL versions";
        }
        if (lowerError.find("gl_fragcolor") != std::string::npos) {
            return "Declare output variables explicitly instead of using gl_FragColor in modern GLSL";
        }
        
        return "Check GLSL documentation and ensure shader code follows OpenGL standards";
    }

    void ShaderErrorHandler::ResetErrorStats() {
        s_compilationErrors = 0;
        s_linkingErrors = 0;
        s_runtimeErrors = 0;
    }

    int ShaderErrorHandler::GetCompilationErrorCount() {
        return s_compilationErrors;
    }

    int ShaderErrorHandler::GetLinkingErrorCount() {
        return s_linkingErrors;
    }

    int ShaderErrorHandler::GetRuntimeErrorCount() {
        return s_runtimeErrors;
    }

    ShaderError ShaderErrorHandler::ParseSingleError(const std::string& errorLine, const std::string& shaderType) {
        ShaderError error;
        error.shaderType = shaderType;
        
        // Try to extract line number using regex patterns
        std::regex lineRegex(R"((\d+):(\d+):\s*(.+))"); // Format: "0:line: message"
        std::regex altLineRegex(R"(ERROR:\s*(\d+):(\d+):\s*(.+))"); // Format: "ERROR: 0:line: message"
        std::regex simpleRegex(R"(line\s*(\d+)\s*:\s*(.+))"); // Format: "line X: message"
        
        std::smatch match;
        
        if (std::regex_search(errorLine, match, lineRegex) && match.size() >= 4) {
            error.lineNumber = std::stoi(match[2].str());
            error.message = CleanErrorMessage(match[3].str());
        }
        else if (std::regex_search(errorLine, match, altLineRegex) && match.size() >= 4) {
            error.lineNumber = std::stoi(match[2].str());
            error.message = CleanErrorMessage(match[3].str());
        }
        else if (std::regex_search(errorLine, match, simpleRegex) && match.size() >= 3) {
            error.lineNumber = std::stoi(match[1].str());
            error.message = CleanErrorMessage(match[2].str());
        }
        else {
            // No line number found, use entire line as message
            error.lineNumber = -1;
            error.message = CleanErrorMessage(errorLine);
        }
        
        return error;
    }

    int ShaderErrorHandler::ExtractLineNumber(const std::string& errorLine) {
        std::regex lineRegex(R"((\d+):(\d+))");
        std::smatch match;
        
        if (std::regex_search(errorLine, match, lineRegex) && match.size() >= 3) {
            return std::stoi(match[2].str());
        }
        
        return -1;
    }

    std::string ShaderErrorHandler::CleanErrorMessage(const std::string& message) {
        std::string cleaned = message;
        
        // Remove common prefixes
        if (cleaned.find("ERROR: ") == 0) {
            cleaned = cleaned.substr(7);
        }
        if (cleaned.find("WARNING: ") == 0) {
            cleaned = cleaned.substr(9);
        }
        
        // Trim whitespace
        cleaned.erase(0, cleaned.find_first_not_of(" \t\r\n"));
        cleaned.erase(cleaned.find_last_not_of(" \t\r\n") + 1);
        
        return cleaned;
    }

    // ShaderValidator implementation
    ShaderValidator::ValidationResult ShaderValidator::ValidateShaderSource(const std::string& source, const std::string& shaderType) {
        ValidationResult result;
        
        // Check for required version directive
        if (!HasRequiredVersionDirective(source)) {
            result.errors.push_back("Missing #version directive at the beginning of shader");
            result.isValid = false;
        }
        
        // Check for valid main function
        if (!HasValidMainFunction(source, shaderType)) {
            result.errors.push_back("Missing or invalid main() function for " + shaderType + " shader");
            result.isValid = false;
        }
        
        // Check for common mistakes
        CheckForCommonMistakes(source, result.warnings);
        
        // Check for performance issues
        CheckForPerformanceIssues(source, result.warnings);
        
        // Check for compatibility issues
        CheckForCompatibilityIssues(source, result.warnings);
        
        // Check uniform usage
        CheckUniformUsage(source, result.warnings);
        
        // Check texture usage
        CheckTextureUsage(source, result.warnings);
        
        // Check control flow
        CheckControlFlow(source, result.warnings);
        
        return result;
    }

    ShaderValidator::ValidationResult ShaderValidator::ValidateVertexShader(const std::string& source) {
        ValidationResult result = ValidateShaderSource(source, "vertex");
        
        // Vertex shader specific checks
        if (source.find("gl_Position") == std::string::npos) {
            result.warnings.push_back("Vertex shader should set gl_Position");
        }
        
        // Check for input attributes
        if (source.find("in ") == std::string::npos && source.find("attribute ") == std::string::npos) {
            result.warnings.push_back("Vertex shader typically needs input attributes");
        }
        
        return result;
    }

    ShaderValidator::ValidationResult ShaderValidator::ValidateFragmentShader(const std::string& source) {
        ValidationResult result = ValidateShaderSource(source, "fragment");
        
        // Fragment shader specific checks
        bool hasOutput = (source.find("out ") != std::string::npos) || 
                        (source.find("gl_FragColor") != std::string::npos) ||
                        (source.find("gl_FragData") != std::string::npos);
        
        if (!hasOutput) {
            result.warnings.push_back("Fragment shader should have color output");
        }
        
        // Check for modern GLSL usage
        if (source.find("gl_FragColor") != std::string::npos) {
            result.suggestions.push_back("Consider using explicit output variables instead of gl_FragColor for modern GLSL");
        }
        
        return result;
    }

    ShaderValidator::ValidationResult ShaderValidator::ValidateComputeShader(const std::string& source) {
        ValidationResult result = ValidateShaderSource(source, "compute");
        
        // Compute shader specific checks
        if (source.find("local_size_x") == std::string::npos) {
            result.errors.push_back("Compute shader must specify local work group size");
            result.isValid = false;
        }
        
        return result;
    }

    ShaderValidator::ValidationResult ShaderValidator::ValidateShaderProgram(uint32_t programId) {
        ValidationResult result;
        
        if (programId == 0) {
            result.errors.push_back("Invalid shader program ID");
            result.isValid = false;
            return result;
        }
        
        // Check if program is linked
        int linkStatus;
        glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            result.errors.push_back("Shader program is not properly linked");
            result.isValid = false;
        }
        
        // Check for validation status
        glValidateProgram(programId);
        int validateStatus;
        glGetProgramiv(programId, GL_VALIDATE_STATUS, &validateStatus);
        if (validateStatus == GL_FALSE) {
            char infoLog[512];
            glGetProgramInfoLog(programId, 512, nullptr, infoLog);
            result.warnings.push_back("Program validation failed: " + std::string(infoLog));
        }
        
        return result;
    }

    ShaderValidator::ValidationResult ShaderValidator::AnalyzeShaderPerformance(const std::string& source, const std::string& shaderType) {
        ValidationResult result;
        
        // Count expensive operations
        size_t divisionCount = 0;
        size_t sqrtCount = 0;
        size_t powCount = 0;
        size_t sinCosCount = 0;
        
        std::string::size_type pos = 0;
        while ((pos = source.find("/", pos)) != std::string::npos) {
            divisionCount++;
            pos++;
        }
        
        pos = 0;
        while ((pos = source.find("sqrt", pos)) != std::string::npos) {
            sqrtCount++;
            pos += 4;
        }
        
        pos = 0;
        while ((pos = source.find("pow", pos)) != std::string::npos) {
            powCount++;
            pos += 3;
        }
        
        pos = 0;
        while ((pos = source.find("sin", pos)) != std::string::npos || 
               (pos = source.find("cos", pos)) != std::string::npos) {
            sinCosCount++;
            pos += 3;
        }
        
        // Performance warnings
        if (divisionCount > 5) {
            result.warnings.push_back("High number of division operations (" + std::to_string(divisionCount) + ") may impact performance");
        }
        
        if (sqrtCount > 3) {
            result.warnings.push_back("Multiple sqrt operations (" + std::to_string(sqrtCount) + ") detected - consider optimization");
        }
        
        if (powCount > 2) {
            result.warnings.push_back("Multiple pow operations (" + std::to_string(powCount) + ") detected - consider using multiplication for small integer powers");
        }
        
        if (sinCosCount > 5) {
            result.warnings.push_back("High number of trigonometric operations (" + std::to_string(sinCosCount) + ") may impact performance");
        }
        
        // Check for branching in fragment shaders
        if (shaderType == "fragment") {
            if (source.find("if") != std::string::npos || source.find("for") != std::string::npos) {
                result.warnings.push_back("Dynamic branching in fragment shader may cause performance issues on some GPUs");
            }
        }
        
        return result;
    }

    bool ShaderValidator::CheckForCommonMistakes(const std::string& source, std::vector<std::string>& warnings) {
        bool foundIssues = false;
        
        // Check for deprecated functions
        if (source.find("texture2D") != std::string::npos) {
            warnings.push_back("Using deprecated texture2D() function - use texture() instead");
            foundIssues = true;
        }
        
        if (source.find("textureCube") != std::string::npos) {
            warnings.push_back("Using deprecated textureCube() function - use texture() instead");
            foundIssues = true;
        }
        
        // Check for deprecated keywords
        if (source.find("varying") != std::string::npos) {
            warnings.push_back("Using deprecated 'varying' keyword - use 'in'/'out' instead");
            foundIssues = true;
        }
        
        if (source.find("attribute") != std::string::npos) {
            warnings.push_back("Using deprecated 'attribute' keyword - use 'in' instead");
            foundIssues = true;
        }
        
        // Check for precision qualifiers in fragment shaders
        if (source.find("precision") == std::string::npos && source.find("fragment") != std::string::npos) {
            warnings.push_back("Consider adding precision qualifiers for better mobile compatibility");
            foundIssues = true;
        }
        
        return foundIssues;
    }

    bool ShaderValidator::CheckForPerformanceIssues(const std::string& source, std::vector<std::string>& warnings) {
        bool foundIssues = false;
        
        // Check for expensive operations in loops
        if (source.find("for") != std::string::npos || source.find("while") != std::string::npos) {
            if (source.find("normalize") != std::string::npos) {
                warnings.push_back("Normalize operations inside loops can be expensive - consider moving outside if possible");
                foundIssues = true;
            }
            
            if (source.find("texture") != std::string::npos) {
                warnings.push_back("Texture sampling inside loops can impact performance");
                foundIssues = true;
            }
        }
        
        // Check for redundant calculations
        size_t normalizeCount = 0;
        std::string::size_type pos = 0;
        while ((pos = source.find("normalize", pos)) != std::string::npos) {
            normalizeCount++;
            pos += 9;
        }
        
        if (normalizeCount > 3) {
            warnings.push_back("Multiple normalize operations detected - consider caching results");
            foundIssues = true;
        }
        
        return foundIssues;
    }

    bool ShaderValidator::CheckForCompatibilityIssues(const std::string& source, std::vector<std::string>& warnings) {
        bool foundIssues = false;
        
        // Check version compatibility
        if (source.find("#version 460") != std::string::npos) {
            warnings.push_back("GLSL version 460 requires OpenGL 4.6 - ensure compatibility");
            foundIssues = true;
        }
        
        // Check for extensions
        if (source.find("#extension") != std::string::npos) {
            warnings.push_back("Shader uses extensions - verify hardware support");
            foundIssues = true;
        }
        
        // Check for compute shader features in non-compute shaders
        if (source.find("local_size") != std::string::npos && source.find("compute") == std::string::npos) {
            warnings.push_back("local_size directive found in non-compute shader");
            foundIssues = true;
        }
        
        return foundIssues;
    }

    bool ShaderValidator::HasRequiredVersionDirective(const std::string& source) {
        // Check if #version is at the beginning (ignoring whitespace and comments)
        std::istringstream stream(source);
        std::string line;
        
        while (std::getline(stream, line)) {
            // Skip empty lines and comments
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            if (line.empty() || line.substr(0, 2) == "//") {
                continue;
            }
            
            // First non-comment line should be #version
            return line.substr(0, 8) == "#version";
        }
        
        return false;
    }

    bool ShaderValidator::HasValidMainFunction(const std::string& source, const std::string& shaderType) {
        // Look for main function
        std::regex mainRegex(R"(void\s+main\s*\(\s*\)\s*\{)");
        return std::regex_search(source, mainRegex);
    }

    bool ShaderValidator::CheckUniformUsage(const std::string& source, std::vector<std::string>& issues) {
        bool foundIssues = false;
        
        // Find all uniform declarations
        std::regex uniformRegex(R"(uniform\s+\w+\s+(\w+)\s*;)");
        std::sregex_iterator iter(source.begin(), source.end(), uniformRegex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::string uniformName = (*iter)[1].str();
            
            // Check if uniform is used
            if (source.find(uniformName, iter->position() + iter->length()) == std::string::npos) {
                issues.push_back("Uniform '" + uniformName + "' is declared but not used");
                foundIssues = true;
            }
        }
        
        return foundIssues;
    }

    bool ShaderValidator::CheckTextureUsage(const std::string& source, std::vector<std::string>& issues) {
        bool foundIssues = false;
        
        // Check for texture sampling without proper filtering
        if (source.find("texture") != std::string::npos) {
            if (source.find("sampler2D") == std::string::npos && 
                source.find("samplerCube") == std::string::npos &&
                source.find("sampler3D") == std::string::npos) {
                issues.push_back("Texture sampling found but no sampler uniforms declared");
                foundIssues = true;
            }
        }
        
        return foundIssues;
    }

    bool ShaderValidator::CheckControlFlow(const std::string& source, std::vector<std::string>& issues) {
        bool foundIssues = false;
        
        // Check for potential infinite loops
        if (source.find("while(true)") != std::string::npos || 
            source.find("while (true)") != std::string::npos) {
            issues.push_back("Potential infinite loop detected - ensure proper break conditions");
            foundIssues = true;
        }
        
        // Check for deeply nested control structures
        int maxNesting = 0;
        int currentNesting = 0;
        
        for (char c : source) {
            if (c == '{') {
                currentNesting++;
                maxNesting = std::max(maxNesting, currentNesting);
            } else if (c == '}') {
                currentNesting--;
            }
        }
        
        if (maxNesting > 5) {
            issues.push_back("Deep nesting detected (level " + std::to_string(maxNesting) + ") - consider refactoring");
            foundIssues = true;
        }
        
        return foundIssues;
    }

}