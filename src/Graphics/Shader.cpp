#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/ShaderError.h"
#include "Graphics/ShaderProfiler.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <chrono>

namespace GameEngine {
    Shader::Shader() {
    }

    Shader::~Shader() {
        if (m_programID) {
            glDeleteProgram(m_programID);
        }
        
        // Clean up individual shaders
        for (auto& pair : m_shaders) {
            if (pair.second != 0) {
                glDeleteShader(pair.second);
            }
        }
    }

    bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSource, fragmentSource;
        
        // Read vertex shader
        std::ifstream vertexFile(vertexPath);
        if (!vertexFile.is_open()) {
            LOG_ERROR("Failed to open vertex shader file: " + vertexPath);
            return false;
        }
        std::stringstream vertexStream;
        vertexStream << vertexFile.rdbuf();
        vertexSource = vertexStream.str();
        vertexFile.close();

        // Read fragment shader
        std::ifstream fragmentFile(fragmentPath);
        if (!fragmentFile.is_open()) {
            LOG_ERROR("Failed to open fragment shader file: " + fragmentPath);
            return false;
        }
        std::stringstream fragmentStream;
        fragmentStream << fragmentFile.rdbuf();
        fragmentSource = fragmentStream.str();
        fragmentFile.close();

        return LoadFromSource(vertexSource, fragmentSource);
    }

    bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
        uint32_t vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
        if (vertexShader == 0) return false;

        uint32_t fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
        if (fragmentShader == 0) {
            glDeleteShader(vertexShader);
            return false;
        }

        bool success = LinkProgram(vertexShader, fragmentShader);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return success;
    }

    bool Shader::CompileFromSource(const std::string& source, Type type) {
        m_state = State::Compiling;
        m_compileLog.clear();
        
        // Start timing compilation
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Store source for validation
        m_shaderSources[type] = source;
        
        uint32_t glType = GetGLShaderType(type);
        if (glType == 0) {
            m_state = State::Error;
            m_compileLog = "Unsupported shader type";
            ShaderErrorHandler::HandleRuntimeError("Unknown", "Unsupported shader type");
            return false;
        }
        
        // Validate shader source before compilation
        std::string shaderTypeName = (type == Type::Vertex) ? "vertex" : 
                                    (type == Type::Fragment) ? "fragment" :
                                    (type == Type::Compute) ? "compute" : "unknown";
        
        auto validationResult = ShaderValidator::ValidateShaderSource(source, shaderTypeName);
        
        // Report validation warnings
        for (const auto& warning : validationResult.warnings) {
            if (m_warningCallback) {
                m_warningCallback("Shader", warning);
            } else {
                ShaderErrorHandler::HandleWarning("Shader", warning);
            }
        }
        
        // If validation failed, don't attempt compilation
        if (!validationResult.isValid) {
            m_state = State::Error;
            m_compileLog = "Validation failed: ";
            for (const auto& error : validationResult.errors) {
                m_compileLog += error + "; ";
            }
            ShaderErrorHandler::HandleCompilationError("Shader", m_compileLog);
            return false;
        }
        
        uint32_t shader = CompileShader(source, glType);
        if (shader == 0) {
            m_state = State::Error;
            return false;
        }
        
        // Clean up existing shader of this type
        auto it = m_shaders.find(type);
        if (it != m_shaders.end() && it->second != 0) {
            glDeleteShader(it->second);
        }
        
        m_shaders[type] = shader;
        m_state = State::Compiled;
        
        // Record compilation time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double compilationTimeMs = duration.count() / 1000.0;
        
        ShaderProfiler::GetInstance().RecordCompilationTime("Shader", compilationTimeMs);
        
        return true;
    }

    bool Shader::CompileFromFile(const std::string& filepath, Type type) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            m_state = State::Error;
            m_compileLog = "Failed to open shader file: " + filepath;
            LOG_ERROR("Failed to open shader file: " + filepath);
            return false;
        }
        
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        
        return CompileFromSource(stream.str(), type);
    }

    bool Shader::LinkProgram() {
        m_linkLog.clear();
        
        // Start timing linking
        auto startTime = std::chrono::high_resolution_clock::now();
        
        if (m_programID != 0) {
            glDeleteProgram(m_programID);
        }
        
        m_programID = glCreateProgram();
        
        // Attach all compiled shaders
        for (const auto& pair : m_shaders) {
            if (pair.second != 0) {
                glAttachShader(m_programID, pair.second);
            }
        }
        
        glLinkProgram(m_programID);
        
        int success;
        glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(m_programID, 1024, nullptr, infoLog);
            m_linkLog = infoLog;
            
            // Use enhanced error handling
            if (m_errorCallback) {
                try {
                    ShaderCompilationError error("Shader", infoLog);
                    m_errorCallback(error);
                } catch (...) {
                    // Fallback to basic logging if callback fails
                    LOG_ERROR("Shader linking failed: " + m_linkLog);
                }
            } else {
                ShaderErrorHandler::HandleLinkingError("Shader", infoLog);
            }
            
            glDeleteProgram(m_programID);
            m_programID = 0;
            m_state = State::Error;
            return false;
        }
        
        // Detach shaders after linking
        for (const auto& pair : m_shaders) {
            if (pair.second != 0) {
                glDetachShader(m_programID, pair.second);
            }
        }
        
        // Validate the linked program
        auto validationResult = ShaderValidator::ValidateShaderProgram(m_programID);
        for (const auto& warning : validationResult.warnings) {
            if (m_warningCallback) {
                m_warningCallback("Shader", warning);
            } else {
                ShaderErrorHandler::HandleWarning("Shader", warning);
            }
        }
        
        m_state = State::Linked;
        
        // Record linking time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double linkingTimeMs = duration.count() / 1000.0;
        
        ShaderProfiler::GetInstance().RecordLinkingTime("Shader", linkingTimeMs);
        
        // Register shader with profiler
        ShaderProfiler::GetInstance().RegisterShader("Shader", m_programID);
        
        return true;
    }

    uint32_t Shader::CompileShader(const std::string& source, uint32_t type) {
        uint32_t shader = glCreateShader(type);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            m_compileLog = infoLog;
            
            // Use enhanced error handling
            if (m_errorCallback) {
                try {
                    ShaderCompilationError error("Shader", infoLog);
                    m_errorCallback(error);
                } catch (...) {
                    // Fallback to basic logging if callback fails
                    LOG_ERROR("Shader compilation failed: " + std::string(infoLog));
                }
            } else {
                ShaderErrorHandler::HandleCompilationError("Shader", infoLog);
            }
            
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    bool Shader::LinkProgram(uint32_t vertexShader, uint32_t fragmentShader) {
        m_programID = glCreateProgram();
        glAttachShader(m_programID, vertexShader);
        glAttachShader(m_programID, fragmentShader);
        glLinkProgram(m_programID);

        int success;
        glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
            LOG_ERROR("Shader linking failed: " + std::string(infoLog));
            glDeleteProgram(m_programID);
            m_programID = 0;
            return false;
        }

        return true;
    }

    void Shader::Use() const {
        if (m_programID) {
            glUseProgram(m_programID);
            
            // Start timing for profiling
            ShaderProfiler::GetInstance().BeginShaderTiming("Shader");
            ShaderProfiler::GetInstance().RecordDrawCall("Shader");
        }
    }

    void Shader::Unuse() const {
        glUseProgram(0);
        
        // End timing for profiling
        ShaderProfiler::GetInstance().EndShaderTiming("Shader");
    }

    int Shader::GetUniformLocation(const std::string& name) {
        auto it = m_uniformCache.find(name);
        if (it != m_uniformCache.end()) {
            return it->second;
        }

        int location = glGetUniformLocation(m_programID, name.c_str());
        m_uniformCache[name] = location;
        return location;
    }



    // Enhanced uniform setters
    void Shader::SetUniform(const std::string& name, bool value) {
        glUniform1i(GetUniformLocation(name), static_cast<int>(value));
    }

    void Shader::SetUniform(const std::string& name, int value) {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::SetUniform(const std::string& name, float value) {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::SetUniform(const std::string& name, const Math::Vec2& value) {
        glUniform2fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetUniform(const std::string& name, const Math::Vec3& value) {
        glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetUniform(const std::string& name, const Math::Vec4& value) {
        glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetUniform(const std::string& name, const Math::Mat3& value) {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetUniform(const std::string& name, const Math::Mat4& value) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetUniformArray(const std::string& name, const std::vector<Math::Mat4>& values) {
        if (!values.empty()) {
            glUniformMatrix4fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), 
                             GL_FALSE, &values[0][0][0]);
        }
    }

    void Shader::SetUniformArray(const std::string& name, const std::vector<Math::Vec3>& values) {
        if (!values.empty()) {
            glUniform3fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), &values[0][0]);
        }
    }

    void Shader::SetUniformArray(const std::string& name, const std::vector<Math::Vec4>& values) {
        if (!values.empty()) {
            glUniform4fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), &values[0][0]);
        }
    }

    void Shader::SetUniformArray(const std::string& name, const std::vector<float>& values) {
        if (!values.empty()) {
            glUniform1fv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
        }
    }

    void Shader::SetUniformArray(const std::string& name, const std::vector<int>& values) {
        if (!values.empty()) {
            glUniform1iv(GetUniformLocation(name), static_cast<GLsizei>(values.size()), values.data());
        }
    }

    // Texture binding with automatic slot management
    void Shader::BindTexture(const std::string& name, uint32_t textureId, uint32_t slot) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, textureId);
        SetUniform(name, static_cast<int>(slot));
    }

    void Shader::BindTexture(const std::string& name, const Texture& texture, uint32_t slot) {
        BindTexture(name, texture.GetID(), slot);
    }

    void Shader::BindTextureAuto(const std::string& name, uint32_t textureId) {
        // Check if this texture uniform already has a slot assigned
        auto it = m_textureSlots.find(name);
        uint32_t slot;
        
        if (it != m_textureSlots.end()) {
            // Use existing slot
            slot = it->second;
        } else {
            // Assign new slot
            slot = GetNextTextureSlot();
            m_textureSlots[name] = slot;
        }
        
        BindTexture(name, textureId, slot);
    }

    void Shader::BindTextureAuto(const std::string& name, const Texture& texture) {
        BindTextureAuto(name, texture.GetID());
    }

    void Shader::BindImageTexture(const std::string& name, uint32_t textureId, uint32_t slot, uint32_t access) {
        glBindImageTexture(slot, textureId, 0, GL_FALSE, 0, access, GL_RGBA8);
        SetUniform(name, static_cast<int>(slot));
    }

    // Storage buffer and uniform buffer binding
    void Shader::BindStorageBuffer(const std::string& name, uint32_t bufferId, uint32_t binding) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, bufferId);
        
        // Set the binding point for the named buffer block
        uint32_t blockIndex = glGetProgramResourceIndex(m_programID, GL_SHADER_STORAGE_BLOCK, name.c_str());
        if (blockIndex != GL_INVALID_INDEX) {
            glShaderStorageBlockBinding(m_programID, blockIndex, binding);
        }
    }

    void Shader::BindUniformBuffer(const std::string& name, uint32_t bufferId, uint32_t binding) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, bufferId);
        
        // Set the binding point for the named uniform block
        uint32_t blockIndex = glGetUniformBlockIndex(m_programID, name.c_str());
        if (blockIndex != GL_INVALID_INDEX) {
            glUniformBlockBinding(m_programID, blockIndex, binding);
        }
    }

    // Compute shader dispatch
    void Shader::Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) {
        glDispatchCompute(groupsX, groupsY, groupsZ);
    }

    void Shader::DispatchIndirect(uint32_t indirectBuffer) {
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer);
        glDispatchComputeIndirect(0);
    }

    // Synchronization
    void Shader::MemoryBarrier(uint32_t barriers) {
        glMemoryBarrier(barriers);
    }

    void Shader::WaitForCompletion() {
        glFinish();
    }

    // Helper methods
    uint32_t Shader::GetGLShaderType(Type type) {
        switch (type) {
            case Type::Vertex: return GL_VERTEX_SHADER;
            case Type::Fragment: return GL_FRAGMENT_SHADER;
            case Type::Geometry: return GL_GEOMETRY_SHADER;
            case Type::Compute: return GL_COMPUTE_SHADER;
            case Type::TessControl: return GL_TESS_CONTROL_SHADER;
            case Type::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
            default: return 0;
        }
    }

    uint32_t Shader::GetNextTextureSlot() {
        return m_nextTextureSlot++;
    }

    void Shader::ResetTextureSlots() {
        m_nextTextureSlot = 0;
        m_textureSlots.clear();
    }

    uint32_t Shader::GetTextureSlot(const std::string& name) const {
        auto it = m_textureSlots.find(name);
        return (it != m_textureSlots.end()) ? it->second : 0;
    }

    bool Shader::HasUniform(const std::string& name) const {
        if (m_programID == 0) return false;
        return glGetUniformLocation(m_programID, name.c_str()) != -1;
    }

    bool Shader::LinkComputeProgram(uint32_t computeShader) {
        m_programID = glCreateProgram();
        glAttachShader(m_programID, computeShader);
        glLinkProgram(m_programID);

        int success;
        glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(m_programID, 1024, nullptr, infoLog);
            m_linkLog = infoLog;
            
            // Use enhanced error handling
            if (m_errorCallback) {
                try {
                    ShaderCompilationError error("ComputeShader", infoLog);
                    m_errorCallback(error);
                } catch (...) {
                    // Fallback to basic logging if callback fails
                    LOG_ERROR("Compute shader linking failed: " + m_linkLog);
                }
            } else {
                ShaderErrorHandler::HandleLinkingError("ComputeShader", infoLog);
            }
            
            glDeleteProgram(m_programID);
            m_programID = 0;
            m_state = State::Error;
            return false;
        }

        m_state = State::Linked;
        return true;
    }

    // Error handling and debugging methods
    void Shader::SetErrorCallback(std::function<void(const ShaderCompilationError&)> callback) {
        m_errorCallback = callback;
    }

    void Shader::SetWarningCallback(std::function<void(const std::string&, const std::string&)> callback) {
        m_warningCallback = callback;
    }

    bool Shader::ValidateShader() const {
        if (m_programID == 0) {
            return false;
        }
        
        auto result = ShaderValidator::ValidateShaderProgram(m_programID);
        return result.isValid;
    }

    std::vector<std::string> Shader::GetValidationWarnings() const {
        std::vector<std::string> allWarnings;
        
        // Validate each shader source
        for (const auto& pair : m_shaderSources) {
            std::string shaderTypeName = (pair.first == Type::Vertex) ? "vertex" : 
                                        (pair.first == Type::Fragment) ? "fragment" :
                                        (pair.first == Type::Compute) ? "compute" : "unknown";
            
            auto result = ShaderValidator::ValidateShaderSource(pair.second, shaderTypeName);
            allWarnings.insert(allWarnings.end(), result.warnings.begin(), result.warnings.end());
        }
        
        // Validate the linked program if available
        if (m_programID != 0) {
            auto programResult = ShaderValidator::ValidateShaderProgram(m_programID);
            allWarnings.insert(allWarnings.end(), programResult.warnings.begin(), programResult.warnings.end());
        }
        
        return allWarnings;
    }

    std::vector<std::string> Shader::GetPerformanceWarnings() const {
        std::vector<std::string> performanceWarnings;
        
        // Analyze each shader source for performance issues
        for (const auto& pair : m_shaderSources) {
            std::string shaderTypeName = (pair.first == Type::Vertex) ? "vertex" : 
                                        (pair.first == Type::Fragment) ? "fragment" :
                                        (pair.first == Type::Compute) ? "compute" : "unknown";
            
            auto result = ShaderValidator::AnalyzeShaderPerformance(pair.second, shaderTypeName);
            performanceWarnings.insert(performanceWarnings.end(), result.warnings.begin(), result.warnings.end());
        }
        
        return performanceWarnings;
    }
}