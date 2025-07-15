#pragma once

#include "Core/Math.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace GameEngine {
    class Shader;
    class Texture;

    class Material {
    public:
        Material();
        ~Material();

        void SetShader(std::shared_ptr<Shader> shader) { m_shader = shader; }
        std::shared_ptr<Shader> GetShader() const { return m_shader; }

        // Texture management
        void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
        std::shared_ptr<Texture> GetTexture(const std::string& name) const;
        void RemoveTexture(const std::string& name);

        // Property setters
        void SetFloat(const std::string& name, float value);
        void SetInt(const std::string& name, int value);
        void SetBool(const std::string& name, bool value);
        void SetVec2(const std::string& name, const Math::Vec2& value);
        void SetVec3(const std::string& name, const Math::Vec3& value);
        void SetVec4(const std::string& name, const Math::Vec4& value);
        void SetMat3(const std::string& name, const Math::Mat3& value);
        void SetMat4(const std::string& name, const Math::Mat4& value);

        // Property getters
        float GetFloat(const std::string& name) const;
        int GetInt(const std::string& name) const;
        bool GetBool(const std::string& name) const;
        Math::Vec2 GetVec2(const std::string& name) const;
        Math::Vec3 GetVec3(const std::string& name) const;
        Math::Vec4 GetVec4(const std::string& name) const;
        Math::Mat3 GetMat3(const std::string& name) const;
        Math::Mat4 GetMat4(const std::string& name) const;

        // Material application
        void Use() const;
        void ApplyUniforms() const;

        // PBR properties
        void SetAlbedo(const Math::Vec3& albedo) { SetVec3("u_albedo", albedo); }
        void SetMetallic(float metallic) { SetFloat("u_metallic", metallic); }
        void SetRoughness(float roughness) { SetFloat("u_roughness", roughness); }
        void SetAO(float ao) { SetFloat("u_ao", ao); }

        Math::Vec3 GetAlbedo() const { return GetVec3("u_albedo"); }
        float GetMetallic() const { return GetFloat("u_metallic"); }
        float GetRoughness() const { return GetFloat("u_roughness"); }
        float GetAO() const { return GetFloat("u_ao"); }

    private:
        std::shared_ptr<Shader> m_shader;
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
        
        // Property storage
        std::unordered_map<std::string, float> m_floatProperties;
        std::unordered_map<std::string, int> m_intProperties;
        std::unordered_map<std::string, bool> m_boolProperties;
        std::unordered_map<std::string, Math::Vec2> m_vec2Properties;
        std::unordered_map<std::string, Math::Vec3> m_vec3Properties;
        std::unordered_map<std::string, Math::Vec4> m_vec4Properties;
        std::unordered_map<std::string, Math::Mat3> m_mat3Properties;
        std::unordered_map<std::string, Math::Mat4> m_mat4Properties;
    };
}