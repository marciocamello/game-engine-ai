#pragma once

#include <string>
#include <vector>

namespace GameEngine {

    struct ShaderValidationResult {
        bool isValid = true;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };

    class ShaderValidator {
    public:
        // Validate shader source code
        static ShaderValidationResult ValidateShaderSource(const std::string& source, const std::string& shaderType);
        
        // Validate compiled shader program
        static ShaderValidationResult ValidateShaderProgram(uint32_t programId);
        
        // Analyze shader performance
        static ShaderValidationResult AnalyzeShaderPerformance(const std::string& source, const std::string& shaderType);
        
        // Get OpenGL shader compilation log
        static std::string GetShaderCompileLog(uint32_t shaderId);
        
        // Get OpenGL program link log
        static std::string GetProgramLinkLog(uint32_t programId);

    private:
        // Helper methods for validation
        static bool CheckShaderSyntax(const std::string& source, const std::string& shaderType, std::vector<std::string>& errors);
        static bool CheckShaderUniforms(const std::string& source, std::vector<std::string>& warnings);
        static bool CheckShaderPerformance(const std::string& source, std::vector<std::string>& warnings);
    };

}