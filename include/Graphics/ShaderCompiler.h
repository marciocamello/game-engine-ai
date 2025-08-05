#pragma once

#include "Core/Math.h"
#include "Graphics/Shader.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace GameEngine {
    struct ShaderCompilationError {
        std::string shaderName;
        std::string errorMessage;
        int lineNumber = -1;
        std::string sourceFile;
        Shader::Type shaderType;
        
        ShaderCompilationError(const std::string& name, const std::string& error, 
                             int line = -1, const std::string& file = "", 
                             Shader::Type type = Shader::Type::Fragment)
            : shaderName(name), errorMessage(error), lineNumber(line), 
              sourceFile(file), shaderType(type) {}
    };

    struct ShaderCompilationStats {
        size_t totalCompilations = 0;
        size_t successfulCompilations = 0;
        size_t failedCompilations = 0;
        float averageCompileTime = 0.0f;
        float totalCompileTime = 0.0f;
        size_t optimizedShaders = 0;
        size_t validatedShaders = 0;
        
        void Reset() {
            totalCompilations = 0;
            successfulCompilations = 0;
            failedCompilations = 0;
            averageCompileTime = 0.0f;
            totalCompileTime = 0.0f;
            optimizedShaders = 0;
            validatedShaders = 0;
        }
    };

    struct ShaderOptimizationSettings {
        bool enableOptimization = true;
        bool removeUnusedVariables = true;
        bool optimizeConstants = true;
        bool inlineFunctions = false;
        bool stripComments = true;
        bool stripWhitespace = false;
        int optimizationLevel = 1; // 0 = none, 1 = basic, 2 = aggressive
    };

    struct ShaderValidationSettings {
        bool enableValidation = true;
        bool checkSyntax = true;
        bool checkSemantics = true;
        bool checkPerformance = false;
        bool warnUnusedUniforms = true;
        bool warnUnusedAttributes = true;
        bool strictMode = false;
    };

    class ShaderCompiler {
    public:
        // Lifecycle
        ShaderCompiler();
        ~ShaderCompiler();
        
        bool Initialize();
        void Shutdown();

        // Compilation methods
        std::shared_ptr<Shader> CompileFromFiles(const std::string& name, 
                                                const std::string& vertexPath, 
                                                const std::string& fragmentPath);
        std::shared_ptr<Shader> CompileFromSource(const std::string& name,
                                                 const std::string& vertexSource,
                                                 const std::string& fragmentSource);
        std::shared_ptr<Shader> CompileComputeFromFile(const std::string& name,
                                                      const std::string& computePath);
        std::shared_ptr<Shader> CompileComputeFromSource(const std::string& name,
                                                        const std::string& computeSource);

        // Multi-stage shader compilation
        std::shared_ptr<Shader> CompileMultiStage(const std::string& name,
                                                 const std::unordered_map<Shader::Type, std::string>& shaderSources);
        std::shared_ptr<Shader> CompileMultiStageFromFiles(const std::string& name,
                                                          const std::unordered_map<Shader::Type, std::string>& shaderPaths);

        // Optimization and validation
        std::string OptimizeShaderSource(const std::string& source, Shader::Type type);
        bool ValidateShaderSource(const std::string& source, Shader::Type type, 
                                 std::vector<std::string>& warnings);
        
        // Error handling and reporting
        std::vector<ShaderCompilationError> GetCompilationErrors() const { return m_compilationErrors; }
        std::string GetLastErrorMessage() const;
        void ClearErrors();
        
        // Performance monitoring and statistics
        ShaderCompilationStats GetCompilationStats() const { return m_stats; }
        void ResetStats();
        float GetLastCompileTime() const { return m_lastCompileTime; }
        
        // Configuration
        void SetOptimizationSettings(const ShaderOptimizationSettings& settings) { m_optimizationSettings = settings; }
        void SetValidationSettings(const ShaderValidationSettings& settings) { m_validationSettings = settings; }
        ShaderOptimizationSettings GetOptimizationSettings() const { return m_optimizationSettings; }
        ShaderValidationSettings GetValidationSettings() const { return m_validationSettings; }
        
        // Debug and logging
        void SetDebugMode(bool enabled) { m_debugMode = enabled; }
        bool IsDebugMode() const { return m_debugMode; }
        void SetVerboseLogging(bool enabled) { m_verboseLogging = enabled; }
        
        // Preprocessor support
        void AddGlobalDefine(const std::string& name, const std::string& value = "1");
        void RemoveGlobalDefine(const std::string& name);
        void ClearGlobalDefines();
        std::string PreprocessShader(const std::string& source, 
                                   const std::unordered_map<std::string, std::string>& defines = {});

    private:
        // Internal compilation methods
        uint32_t CompileShaderStage(const std::string& source, Shader::Type type, const std::string& name);
        bool LinkShaderProgram(uint32_t programId, const std::vector<uint32_t>& shaderIds, const std::string& name);
        
        // Error parsing and handling
        void ParseCompilationError(const std::string& errorLog, const std::string& shaderName, 
                                  Shader::Type type, const std::string& sourceFile = "");
        void ParseLinkingError(const std::string& errorLog, const std::string& shaderName);
        
        // Optimization methods
        std::string RemoveComments(const std::string& source);
        std::string RemoveUnusedVariables(const std::string& source, Shader::Type type);
        std::string OptimizeConstants(const std::string& source);
        std::string StripWhitespace(const std::string& source);
        
        // Validation methods
        bool ValidateSyntax(const std::string& source, Shader::Type type, std::vector<std::string>& warnings);
        bool ValidateSemantics(const std::string& source, Shader::Type type, std::vector<std::string>& warnings);
        bool ValidatePerformance(const std::string& source, Shader::Type type, std::vector<std::string>& warnings);
        void CheckUnusedUniforms(uint32_t programId, std::vector<std::string>& warnings);
        void CheckUnusedAttributes(uint32_t programId, std::vector<std::string>& warnings);
        
        // Utility methods
        std::string LoadShaderFile(const std::string& filepath);
        uint32_t GetGLShaderType(Shader::Type type);
        std::string GetShaderTypeName(Shader::Type type);
        void UpdateCompilationStats(bool success, float compileTime);
        
        // Member variables
        bool m_initialized = false;
        bool m_debugMode = false;
        bool m_verboseLogging = false;
        
        ShaderOptimizationSettings m_optimizationSettings;
        ShaderValidationSettings m_validationSettings;
        
        std::vector<ShaderCompilationError> m_compilationErrors;
        ShaderCompilationStats m_stats;
        float m_lastCompileTime = 0.0f;
        
        std::unordered_map<std::string, std::string> m_globalDefines;
        
        // Performance timing
        std::chrono::high_resolution_clock::time_point m_compileStartTime;
    };
}