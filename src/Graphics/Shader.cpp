#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace GameEngine {
    Shader::Shader() {
    }

    Shader::~Shader() {
        if (m_programID) {
            glDeleteProgram(m_programID);
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

    uint32_t Shader::CompileShader(const std::string& source, uint32_t type) {
        uint32_t shader = glCreateShader(type);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            LOG_ERROR("Shader compilation failed: " + std::string(infoLog));
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
        }
    }

    void Shader::Unuse() const {
        glUseProgram(0);
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

    void Shader::SetBool(const std::string& name, bool value) {
        glUniform1i(GetUniformLocation(name), static_cast<int>(value));
    }

    void Shader::SetInt(const std::string& name, int value) {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::SetFloat(const std::string& name, float value) {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::SetVec2(const std::string& name, const Math::Vec2& value) {
        glUniform2fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetVec3(const std::string& name, const Math::Vec3& value) {
        glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetVec4(const std::string& name, const Math::Vec4& value) {
        glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetMat3(const std::string& name, const Math::Mat3& value) {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetMat4(const std::string& name, const Math::Mat4& value) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }
}