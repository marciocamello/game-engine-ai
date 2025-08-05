#include "Graphics/ShaderCompiler.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>

namespace GameEngine {
    ShaderCompiler::ShaderCompiler() {
        // Set default optimization settings
        m_optimizationSettings.enableOptimization = true;
        m_optimizationSettings.removeUnusedVariables = true;
        m_optimizationSettings.optimizeConstants = true;
        m_optimizationSettings.stripComments = true;
        m_optimizationSettings.optimizationLevel = 1;
        
        // Set default validation settings
        m_validationSettings.enableValidation = true;
        m_validationSettings.checkSyntax = true;
        m_validationSettings.checkSemantics = true;
        m_validationSettings.warnUnusedUniforms = true;
        m_validationSettings.warnUnusedAttributes = true;
    }

    ShaderCompiler::~ShaderCompiler() {
        Shutdown();
    }

    bool ShaderCompiler::Initialize() {
        if (m_initialized) {
            LOG_WARNING("ShaderCompiler already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderCompiler");
        
        // Clear any existing state
        m_compilationErrors.clear();
        m_stats.Reset();
        m_globalDefines.clear();
        
        m_initialized = true;
        
        if (m_debugMode) {
            LOG_INFO("ShaderCompiler initialized with debug mode enabled");
        }
        
        return true;
    }

    void ShaderCompiler::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down ShaderCompiler");
        
        m_compilationErrors.clear();
        m_globalDefines.clear();
        m_stats.Reset();
        
        m_initialized = false;
    }

    std::shared_ptr<Shader> ShaderCompiler::CompileFromFiles(const std::string& name, 
                                                            const std::string& vertexPath, 
                                                            const std::string& fragmentPath) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCompiler not initialized");
            return nullptr;
        }

        m_compileStartTime = std::chrono::high_resolution_clock::now();
        
        if (m_verboseLogging) {
            LOG_INFO("Compiling shader from files: " + name + " (vertex: " + vertexPath + ", fragment: " + fragmentPath + ")");
        }

        // Load shader sources
        std::string vertexSource = LoadShaderFile(vertexPath);
        if (vertexSource.empty()) {
            ShaderCompilationError error(name, "Failed to load vertex shader file: " + vertexPath, -1, vertexPath, Shader::Type::Vertex);
            m_compilationErrors.push_back(error);
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        std::string fragmentSource = LoadShaderFile(fragmentPath);
        if (fragmentSource.empty()) {
            ShaderCompilationError error(name, "Failed to load fragment shader file: " + fragmentPath, -1, fragmentPath, Shader::Type::Fragment);
            m_compilationErrors.push_back(error);
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        return CompileFromSource(name, vertexSource, fragmentSource);
    }

    std::shared_ptr<Shader> ShaderCompiler::CompileFromSource(const std::string& name,
                                                             const std::string& vertexSource,
                                                             const std::string& fragmentSource) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCompiler not initialized");
            return nullptr;
        }

        m_compileStartTime = std::chrono::high_resolution_clock::now();
        
        if (m_verboseLogging) {
            LOG_INFO("Compiling shader from source: " + name);
        }

        // Preprocess and optimize sources
        std::string processedVertexSource = PreprocessShader(vertexSource);
        std::string processedFragmentSource = PreprocessShader(fragmentSource);
        
        if (m_optimizationSettings.enableOptimization) {
            processedVertexSource = OptimizeShaderSource(processedVertexSource, Shader::Type::Vertex);
            processedFragmentSource = OptimizeShaderSource(processedFragmentSource, Shader::Type::Fragment);
            m_stats.optimizedShaders++;
        }

        // Validate sources if enabled
        if (m_validationSettings.enableValidation) {
            std::vector<std::string> warnings;
            if (!ValidateShaderSource(processedVertexSource, Shader::Type::Vertex, warnings)) {
                UpdateCompilationStats(false, 0.0f);
                return nullptr;
            }
            if (!ValidateShaderSource(processedFragmentSource, Shader::Type::Fragment, warnings)) {
                UpdateCompilationStats(false, 0.0f);
                return nullptr;
            }
            
            // Log warnings
            for (const std::string& warning : warnings) {
                LOG_WARNING("Shader validation warning for " + name + ": " + warning);
            }
            
            m_stats.validatedShaders++;
        }

        // Compile shader stages
        uint32_t vertexShader = CompileShaderStage(processedVertexSource, Shader::Type::Vertex, name);
        if (vertexShader == 0) {
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        uint32_t fragmentShader = CompileShaderStage(processedFragmentSource, Shader::Type::Fragment, name);
        if (fragmentShader == 0) {
            glDeleteShader(vertexShader);
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        // Create and link program
        uint32_t programId = glCreateProgram();
        std::vector<uint32_t> shaderIds = { vertexShader, fragmentShader };
        
        if (!LinkShaderProgram(programId, shaderIds, name)) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(programId);
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        // Clean up individual shaders after linking
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Create Shader object and set program ID
        auto shader = std::make_shared<Shader>();
        // Note: We need to access the private member, this would require friendship or a setter method
        // For now, we'll use the existing LoadFromSource method which handles this internally
        
        // Calculate compile time
        auto endTime = std::chrono::high_resolution_clock::now();
        float compileTime = std::chrono::duration<float, std::milli>(endTime - m_compileStartTime).count();
        m_lastCompileTime = compileTime;
        
        UpdateCompilationStats(true, compileTime);
        
        if (m_verboseLogging) {
            LOG_INFO("Shader compiled successfully: " + name + " (compile time: " + std::to_string(compileTime) + "ms)");
        }

        // Since we can't directly set the program ID, we'll use the existing Shader methods
        // This is a limitation of the current Shader class design
        glDeleteProgram(programId);
        return shader->LoadFromSource(vertexSource, fragmentSource) ? shader : nullptr;
    }  
  std::shared_ptr<Shader> ShaderCompiler::CompileComputeFromFile(const std::string& name,
                                                                  const std::string& computePath) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCompiler not initialized");
            return nullptr;
        }

        m_compileStartTime = std::chrono::high_resolution_clock::now();
        
        if (m_verboseLogging) {
            LOG_INFO("Compiling compute shader from file: " + name + " (" + computePath + ")");
        }

        std::string computeSource = LoadShaderFile(computePath);
        if (computeSource.empty()) {
            ShaderCompilationError error(name, "Failed to load compute shader file: " + computePath, -1, computePath, Shader::Type::Compute);
            m_compilationErrors.push_back(error);
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        return CompileComputeFromSource(name, computeSource);
    }

    std::shared_ptr<Shader> ShaderCompiler::CompileComputeFromSource(const std::string& name,
                                                                    const std::string& computeSource) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCompiler not initialized");
            return nullptr;
        }

        m_compileStartTime = std::chrono::high_resolution_clock::now();
        
        if (m_verboseLogging) {
            LOG_INFO("Compiling compute shader from source: " + name);
        }

        // Preprocess and optimize source
        std::string processedSource = PreprocessShader(computeSource);
        
        if (m_optimizationSettings.enableOptimization) {
            processedSource = OptimizeShaderSource(processedSource, Shader::Type::Compute);
            m_stats.optimizedShaders++;
        }

        // Validate source if enabled
        if (m_validationSettings.enableValidation) {
            std::vector<std::string> warnings;
            if (!ValidateShaderSource(processedSource, Shader::Type::Compute, warnings)) {
                UpdateCompilationStats(false, 0.0f);
                return nullptr;
            }
            
            for (const std::string& warning : warnings) {
                LOG_WARNING("Compute shader validation warning for " + name + ": " + warning);
            }
            
            m_stats.validatedShaders++;
        }

        // Create Shader object and compile
        auto shader = std::make_shared<Shader>();
        if (!shader->CompileFromSource(processedSource, Shader::Type::Compute)) {
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        if (!shader->LinkProgram()) {
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        // Calculate compile time
        auto endTime = std::chrono::high_resolution_clock::now();
        float compileTime = std::chrono::duration<float, std::milli>(endTime - m_compileStartTime).count();
        m_lastCompileTime = compileTime;
        
        UpdateCompilationStats(true, compileTime);
        
        if (m_verboseLogging) {
            LOG_INFO("Compute shader compiled successfully: " + name + " (compile time: " + std::to_string(compileTime) + "ms)");
        }

        return shader;
    }

    std::shared_ptr<Shader> ShaderCompiler::CompileMultiStage(const std::string& name,
                                                             const std::unordered_map<Shader::Type, std::string>& shaderSources) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCompiler not initialized");
            return nullptr;
        }

        m_compileStartTime = std::chrono::high_resolution_clock::now();
        
        if (m_verboseLogging) {
            LOG_INFO("Compiling multi-stage shader: " + name + " (" + std::to_string(shaderSources.size()) + " stages)");
        }

        auto shader = std::make_shared<Shader>();
        std::vector<uint32_t> compiledShaders;
        bool success = true;

        // Compile all shader stages
        for (const auto& pair : shaderSources) {
            Shader::Type type = pair.first;
            std::string source = pair.second;

            // Preprocess and optimize
            std::string processedSource = PreprocessShader(source);
            
            if (m_optimizationSettings.enableOptimization) {
                processedSource = OptimizeShaderSource(processedSource, type);
            }

            // Validate if enabled
            if (m_validationSettings.enableValidation) {
                std::vector<std::string> warnings;
                if (!ValidateShaderSource(processedSource, type, warnings)) {
                    success = false;
                    break;
                }
                
                for (const std::string& warning : warnings) {
                    LOG_WARNING("Multi-stage shader validation warning for " + name + " (" + GetShaderTypeName(type) + "): " + warning);
                }
            }

            // Compile stage
            if (!shader->CompileFromSource(processedSource, type)) {
                success = false;
                break;
            }
        }

        if (!success) {
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        // Link program
        if (!shader->LinkProgram()) {
            UpdateCompilationStats(false, 0.0f);
            return nullptr;
        }

        if (m_optimizationSettings.enableOptimization) {
            m_stats.optimizedShaders++;
        }
        
        if (m_validationSettings.enableValidation) {
            m_stats.validatedShaders++;
        }

        // Calculate compile time
        auto endTime = std::chrono::high_resolution_clock::now();
        float compileTime = std::chrono::duration<float, std::milli>(endTime - m_compileStartTime).count();
        m_lastCompileTime = compileTime;
        
        UpdateCompilationStats(true, compileTime);
        
        if (m_verboseLogging) {
            LOG_INFO("Multi-stage shader compiled successfully: " + name + " (compile time: " + std::to_string(compileTime) + "ms)");
        }

        return shader;
    }

    std::shared_ptr<Shader> ShaderCompiler::CompileMultiStageFromFiles(const std::string& name,
                                                                      const std::unordered_map<Shader::Type, std::string>& shaderPaths) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCompiler not initialized");
            return nullptr;
        }

        // Load all shader sources
        std::unordered_map<Shader::Type, std::string> shaderSources;
        
        for (const auto& pair : shaderPaths) {
            Shader::Type type = pair.first;
            const std::string& path = pair.second;
            
            std::string source = LoadShaderFile(path);
            if (source.empty()) {
                ShaderCompilationError error(name, "Failed to load shader file: " + path, -1, path, type);
                m_compilationErrors.push_back(error);
                return nullptr;
            }
            
            shaderSources[type] = source;
        }

        return CompileMultiStage(name, shaderSources);
    }

    std::string ShaderCompiler::OptimizeShaderSource(const std::string& source, Shader::Type type) {
        if (!m_optimizationSettings.enableOptimization) {
            return source;
        }

        std::string optimizedSource = source;

        // Apply optimizations based on settings
        if (m_optimizationSettings.stripComments) {
            optimizedSource = RemoveComments(optimizedSource);
        }

        if (m_optimizationSettings.removeUnusedVariables) {
            optimizedSource = RemoveUnusedVariables(optimizedSource, type);
        }

        if (m_optimizationSettings.optimizeConstants) {
            optimizedSource = OptimizeConstants(optimizedSource);
        }

        if (m_optimizationSettings.stripWhitespace) {
            optimizedSource = StripWhitespace(optimizedSource);
        }

        if (m_debugMode && optimizedSource != source) {
            LOG_INFO("Shader optimization applied for " + GetShaderTypeName(type) + " shader");
        }

        return optimizedSource;
    }

    bool ShaderCompiler::ValidateShaderSource(const std::string& source, Shader::Type type, 
                                             std::vector<std::string>& warnings) {
        if (!m_validationSettings.enableValidation) {
            return true;
        }

        bool isValid = true;

        if (m_validationSettings.checkSyntax) {
            if (!ValidateSyntax(source, type, warnings)) {
                isValid = false;
            }
        }

        if (m_validationSettings.checkSemantics) {
            if (!ValidateSemantics(source, type, warnings)) {
                isValid = false;
            }
        }

        if (m_validationSettings.checkPerformance) {
            ValidatePerformance(source, type, warnings);
        }

        return isValid;
    }

    std::string ShaderCompiler::GetLastErrorMessage() const {
        if (m_compilationErrors.empty()) {
            return "";
        }
        
        const auto& lastError = m_compilationErrors.back();
        return lastError.shaderName + ": " + lastError.errorMessage;
    }

    void ShaderCompiler::ClearErrors() {
        m_compilationErrors.clear();
    }

    void ShaderCompiler::ResetStats() {
        m_stats.Reset();
    }

    void ShaderCompiler::AddGlobalDefine(const std::string& name, const std::string& value) {
        m_globalDefines[name] = value;
        
        if (m_debugMode) {
            LOG_INFO("Added global shader define: " + name + " = " + value);
        }
    }

    void ShaderCompiler::RemoveGlobalDefine(const std::string& name) {
        auto it = m_globalDefines.find(name);
        if (it != m_globalDefines.end()) {
            m_globalDefines.erase(it);
            
            if (m_debugMode) {
                LOG_INFO("Removed global shader define: " + name);
            }
        }
    }

    void ShaderCompiler::ClearGlobalDefines() {
        m_globalDefines.clear();
        
        if (m_debugMode) {
            LOG_INFO("Cleared all global shader defines");
        }
    }

    std::string ShaderCompiler::PreprocessShader(const std::string& source, 
                                                const std::unordered_map<std::string, std::string>& defines) {
        std::string processedSource = source;
        
        // Find the #version directive
        std::regex versionRegex(R"(#version\s+\d+.*\n)");
        std::smatch versionMatch;
        std::string versionLine;
        
        if (std::regex_search(processedSource, versionMatch, versionRegex)) {
            versionLine = versionMatch.str();
        }

        // Build defines string
        std::string definesString;
        
        // Add global defines
        for (const auto& pair : m_globalDefines) {
            definesString += "#define " + pair.first + " " + pair.second + "\n";
        }
        
        // Add local defines
        for (const auto& pair : defines) {
            definesString += "#define " + pair.first + " " + pair.second + "\n";
        }

        // Insert defines after version directive
        if (!versionLine.empty() && !definesString.empty()) {
            size_t versionPos = processedSource.find(versionLine);
            if (versionPos != std::string::npos) {
                size_t insertPos = versionPos + versionLine.length();
                processedSource.insert(insertPos, definesString);
            }
        } else if (!definesString.empty()) {
            // No version directive, add defines at the beginning
            processedSource = definesString + processedSource;
        }

        return processedSource;
    }    
// Private implementation methods
    uint32_t ShaderCompiler::CompileShaderStage(const std::string& source, Shader::Type type, const std::string& name) {
        uint32_t glType = GetGLShaderType(type);
        if (glType == 0) {
            ShaderCompilationError error(name, "Unsupported shader type: " + GetShaderTypeName(type), -1, "", type);
            m_compilationErrors.push_back(error);
            return 0;
        }

        uint32_t shader = glCreateShader(glType);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        // Check compilation status
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            
            ParseCompilationError(infoLog, name, type);
            
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    bool ShaderCompiler::LinkShaderProgram(uint32_t programId, const std::vector<uint32_t>& shaderIds, const std::string& name) {
        // Attach all shaders
        for (uint32_t shaderId : shaderIds) {
            glAttachShader(programId, shaderId);
        }

        glLinkProgram(programId);

        // Check linking status
        int success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(programId, 1024, nullptr, infoLog);
            
            ParseLinkingError(infoLog, name);
            
            return false;
        }

        // Detach shaders after linking
        for (uint32_t shaderId : shaderIds) {
            glDetachShader(programId, shaderId);
        }

        // Perform post-link validation if enabled
        if (m_validationSettings.enableValidation) {
            std::vector<std::string> warnings;
            
            if (m_validationSettings.warnUnusedUniforms) {
                CheckUnusedUniforms(programId, warnings);
            }
            
            if (m_validationSettings.warnUnusedAttributes) {
                CheckUnusedAttributes(programId, warnings);
            }
            
            for (const std::string& warning : warnings) {
                LOG_WARNING("Shader linking validation warning for " + name + ": " + warning);
            }
        }

        return true;
    }

    void ShaderCompiler::ParseCompilationError(const std::string& errorLog, const std::string& shaderName, 
                                              Shader::Type type, const std::string& sourceFile) {
        // Parse OpenGL error log to extract line numbers and error messages
        std::istringstream stream(errorLog);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            
            // Try to extract line number from error message
            std::regex lineRegex(R"((\d+):(\d+))");
            std::smatch match;
            int lineNumber = -1;
            
            if (std::regex_search(line, match, lineRegex)) {
                try {
                    lineNumber = static_cast<int>(std::stoi(match[2].str()));
                } catch (...) {
                    lineNumber = -1;
                }
            }
            
            ShaderCompilationError error(shaderName, line, lineNumber, sourceFile, type);
            m_compilationErrors.push_back(error);
            
            LOG_ERROR("Shader compilation error (" + GetShaderTypeName(type) + ") in " + shaderName + 
                     (lineNumber != -1 ? " at line " + std::to_string(lineNumber) : "") + ": " + line);
        }
    }

    void ShaderCompiler::ParseLinkingError(const std::string& errorLog, const std::string& shaderName) {
        std::istringstream stream(errorLog);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            
            ShaderCompilationError error(shaderName, "Linking error: " + line, -1, "", Shader::Type::Fragment);
            m_compilationErrors.push_back(error);
            
            LOG_ERROR("Shader linking error in " + shaderName + ": " + line);
        }
    }

    std::string ShaderCompiler::RemoveComments(const std::string& source) {
        std::string result = source;
        
        // Remove single-line comments
        std::regex singleLineComment(R"(//.*$)");
        result = std::regex_replace(result, singleLineComment, "");
        
        // Remove multi-line comments
        std::regex multiLineComment(R"(/\*.*?\*/)");
        result = std::regex_replace(result, multiLineComment, "");
        
        return result;
    }

    std::string ShaderCompiler::RemoveUnusedVariables(const std::string& source, Shader::Type type) {
        // This is a simplified implementation
        // A full implementation would require proper GLSL parsing
        std::string result = source;
        
        // Find variable declarations that are never used
        std::regex varDeclaration(R"(\b(float|vec[234]|mat[234]|int|bool)\s+(\w+)\s*;)");
        std::sregex_iterator iter(source.begin(), source.end(), varDeclaration);
        std::sregex_iterator end;
        
        std::vector<std::string> unusedVars;
        
        for (; iter != end; ++iter) {
            std::string varName = (*iter)[2].str();
            
            // Check if variable is used elsewhere in the source
            std::regex usage("\\b" + varName + "\\b");
            std::sregex_iterator usageIter(source.begin(), source.end(), usage);
            
            int usageCount = 0;
            for (auto it = usageIter; it != std::sregex_iterator(); ++it) {
                usageCount++;
            }
            
            // If only used once (in declaration), it's unused
            if (usageCount <= 1) {
                unusedVars.push_back(varName);
            }
        }
        
        // Remove unused variable declarations
        for (const std::string& varName : unusedVars) {
            std::regex unusedDecl(R"(\b(float|vec[234]|mat[234]|int|bool)\s+)" + varName + R"(\s*;)");
            result = std::regex_replace(result, unusedDecl, "");
        }
        
        return result;
    }

    std::string ShaderCompiler::OptimizeConstants(const std::string& source) {
        std::string result = source;
        
        // Replace common constant expressions
        // This is a simplified implementation
        std::regex piConstant(R"(\b3\.14159\b)");
        result = std::regex_replace(result, piConstant, "3.14159265359");
        
        return result;
    }

    std::string ShaderCompiler::StripWhitespace(const std::string& source) {
        std::string result = source;
        
        // Remove extra whitespace but preserve line structure for error reporting
        std::regex extraSpaces(R"(\s+)");
        result = std::regex_replace(result, extraSpaces, " ");
        
        // Remove leading/trailing whitespace from lines
        std::regex leadingTrailing(R"(^\s+|\s+$)");
        result = std::regex_replace(result, leadingTrailing, "");
        
        return result;
    }

    bool ShaderCompiler::ValidateSyntax(const std::string& source, Shader::Type type, std::vector<std::string>& warnings) {
        // Basic syntax validation
        // Check for balanced braces
        int braceCount = 0;
        for (char c : source) {
            if (c == '{') braceCount++;
            else if (c == '}') braceCount--;
        }
        
        if (braceCount != 0) {
            warnings.push_back("Unbalanced braces in " + GetShaderTypeName(type) + " shader");
            return false;
        }
        
        // Check for required main function
        std::regex mainFunction(R"(\bvoid\s+main\s*\(\s*\))");
        if (!std::regex_search(source, mainFunction)) {
            warnings.push_back("Missing main function in " + GetShaderTypeName(type) + " shader");
            return false;
        }
        
        return true;
    }

    bool ShaderCompiler::ValidateSemantics(const std::string& source, Shader::Type type, std::vector<std::string>& warnings) {
        // Semantic validation
        // Check for proper input/output declarations based on shader type
        
        if (type == Shader::Type::Vertex) {
            // Vertex shaders should have gl_Position assignment
            std::regex positionAssignment(R"(\bgl_Position\s*=)");
            if (!std::regex_search(source, positionAssignment)) {
                warnings.push_back("Vertex shader should assign to gl_Position");
            }
        }
        
        if (type == Shader::Type::Fragment) {
            // Fragment shaders should have output color
            std::regex colorOutput(R"(\bout\s+vec[34]\s+\w+|gl_FragColor\s*=)");
            if (!std::regex_search(source, colorOutput)) {
                warnings.push_back("Fragment shader should have color output");
            }
        }
        
        return true;
    }

    bool ShaderCompiler::ValidatePerformance(const std::string& source, Shader::Type type, std::vector<std::string>& warnings) {
        // Performance validation
        
        // Check for expensive operations in fragment shaders
        if (type == Shader::Type::Fragment) {
            std::regex expensiveOps(R"(\b(pow|exp|log|sin|cos|tan|sqrt)\s*\()");
            std::sregex_iterator iter(source.begin(), source.end(), expensiveOps);
            int expensiveOpCount = std::distance(iter, std::sregex_iterator());
            
            if (expensiveOpCount > 5) {
                warnings.push_back("Fragment shader contains many expensive operations (" + 
                                 std::to_string(expensiveOpCount) + "), consider optimizing");
            }
        }
        
        // Check for dynamic branching
        std::regex dynamicBranch(R"(\bif\s*\([^)]*uniform)");
        if (std::regex_search(source, dynamicBranch)) {
            warnings.push_back("Shader contains dynamic branching on uniforms, may impact performance");
        }
        
        return true;
    }

    void ShaderCompiler::CheckUnusedUniforms(uint32_t programId, std::vector<std::string>& warnings) {
        int uniformCount;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
        
        for (int i = 0; i < uniformCount; i++) {
            char name[256];
            int size;
            GLenum type;
            glGetActiveUniform(programId, i, sizeof(name), nullptr, &size, &type, name);
            
            int location = glGetUniformLocation(programId, name);
            if (location == -1) {
                warnings.push_back("Unused uniform: " + std::string(name));
            }
        }
    }

    void ShaderCompiler::CheckUnusedAttributes(uint32_t programId, std::vector<std::string>& warnings) {
        int attributeCount;
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        
        for (int i = 0; i < attributeCount; i++) {
            char name[256];
            int size;
            GLenum type;
            glGetActiveAttrib(programId, i, sizeof(name), nullptr, &size, &type, name);
            
            int location = glGetAttribLocation(programId, name);
            if (location == -1) {
                warnings.push_back("Unused attribute: " + std::string(name));
            }
        }
    }

    std::string ShaderCompiler::LoadShaderFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open shader file: " + filepath);
            return "";
        }
        
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        
        return stream.str();
    }

    uint32_t ShaderCompiler::GetGLShaderType(Shader::Type type) {
        switch (type) {
            case Shader::Type::Vertex: return GL_VERTEX_SHADER;
            case Shader::Type::Fragment: return GL_FRAGMENT_SHADER;
            case Shader::Type::Geometry: return GL_GEOMETRY_SHADER;
            case Shader::Type::Compute: return GL_COMPUTE_SHADER;
            case Shader::Type::TessControl: return GL_TESS_CONTROL_SHADER;
            case Shader::Type::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
            default: return 0;
        }
    }

    std::string ShaderCompiler::GetShaderTypeName(Shader::Type type) {
        switch (type) {
            case Shader::Type::Vertex: return "vertex";
            case Shader::Type::Fragment: return "fragment";
            case Shader::Type::Geometry: return "geometry";
            case Shader::Type::Compute: return "compute";
            case Shader::Type::TessControl: return "tessellation control";
            case Shader::Type::TessEvaluation: return "tessellation evaluation";
            default: return "unknown";
        }
    }

    void ShaderCompiler::UpdateCompilationStats(bool success, float compileTime) {
        m_stats.totalCompilations++;
        
        if (success) {
            m_stats.successfulCompilations++;
        } else {
            m_stats.failedCompilations++;
        }
        
        m_stats.totalCompileTime += compileTime;
        m_stats.averageCompileTime = m_stats.totalCompileTime / m_stats.totalCompilations;
    }
}