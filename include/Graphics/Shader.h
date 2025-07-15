#pragma once

#include "Core/Math.h"
#include <string>
#include <unordered_map>

namespace GameEngine {
    class Shader {
    public:
        Shader();
        ~Shader();

        bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
        bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
        
        void Use() const;
        void Unuse() const;
        
        // Uniform setters
        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetFloat(const std::string& name, float value);
        void SetVec2(const std::string& name, const Math::Vec2& value);
        void SetVec3(const std::string& name, const Math::Vec3& value);
        void SetVec4(const std::string& name, const Math::Vec4& value);
        void SetMat3(const std::string& name, const Math::Mat3& value);
        void SetMat4(const std::string& name, const Math::Mat4& value);
        
        uint32_t GetProgramID() const { return m_programID; }
        bool IsValid() const { return m_programID != 0; }

    private:
        uint32_t CompileShader(const std::string& source, uint32_t type);
        bool LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
        int GetUniformLocation(const std::string& name);
        
        uint32_t m_programID = 0;
        std::unordered_map<std::string, int> m_uniformCache;
    };
}