#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"

namespace GameEngine {
    Material::Material() {
        // Set default PBR values
        SetFloat("u_metallic", 0.0f);
        SetFloat("u_roughness", 0.5f);
        SetFloat("u_ao", 1.0f);
        SetVec3("u_albedo", Math::Vec3(0.8f, 0.8f, 0.8f));
    }

    Material::~Material() {
        // Clear all property maps
        m_textures.clear();
        m_floatProperties.clear();
        m_intProperties.clear();
        m_boolProperties.clear();
        m_vec2Properties.clear();
        m_vec3Properties.clear();
        m_vec4Properties.clear();
        m_mat3Properties.clear();
        m_mat4Properties.clear();
    }

    void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture) {
        if (texture) {
            m_textures[name] = texture;
        } else {
            LOG_WARNING("Attempted to set null texture for material property: " + name);
        }
    }

    std::shared_ptr<Texture> Material::GetTexture(const std::string& name) const {
        auto it = m_textures.find(name);
        if (it != m_textures.end()) {
            return it->second;
        }
        return nullptr;
    }

    void Material::RemoveTexture(const std::string& name) {
        auto it = m_textures.find(name);
        if (it != m_textures.end()) {
            m_textures.erase(it);
        }
    }

    // Float property methods
    void Material::SetFloat(const std::string& name, float value) {
        m_floatProperties[name] = value;
    }

    float Material::GetFloat(const std::string& name) const {
        auto it = m_floatProperties.find(name);
        if (it != m_floatProperties.end()) {
            return it->second;
        }
        LOG_WARNING("Float property not found: " + name);
        return 0.0f;
    }

    // Int property methods
    void Material::SetInt(const std::string& name, int value) {
        m_intProperties[name] = value;
    }

    int Material::GetInt(const std::string& name) const {
        auto it = m_intProperties.find(name);
        if (it != m_intProperties.end()) {
            return it->second;
        }
        LOG_WARNING("Int property not found: " + name);
        return 0;
    }

    // Bool property methods
    void Material::SetBool(const std::string& name, bool value) {
        m_boolProperties[name] = value;
    }

    bool Material::GetBool(const std::string& name) const {
        auto it = m_boolProperties.find(name);
        if (it != m_boolProperties.end()) {
            return it->second;
        }
        LOG_WARNING("Bool property not found: " + name);
        return false;
    }

    // Vec2 property methods
    void Material::SetVec2(const std::string& name, const Math::Vec2& value) {
        m_vec2Properties[name] = value;
    }

    Math::Vec2 Material::GetVec2(const std::string& name) const {
        auto it = m_vec2Properties.find(name);
        if (it != m_vec2Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Vec2 property not found: " + name);
        return Math::Vec2(0.0f);
    }

    // Vec3 property methods
    void Material::SetVec3(const std::string& name, const Math::Vec3& value) {
        m_vec3Properties[name] = value;
    }

    Math::Vec3 Material::GetVec3(const std::string& name) const {
        auto it = m_vec3Properties.find(name);
        if (it != m_vec3Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Vec3 property not found: " + name);
        return Math::Vec3(0.0f);
    }

    // Vec4 property methods
    void Material::SetVec4(const std::string& name, const Math::Vec4& value) {
        m_vec4Properties[name] = value;
    }

    Math::Vec4 Material::GetVec4(const std::string& name) const {
        auto it = m_vec4Properties.find(name);
        if (it != m_vec4Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Vec4 property not found: " + name);
        return Math::Vec4(0.0f);
    }

    // Mat3 property methods
    void Material::SetMat3(const std::string& name, const Math::Mat3& value) {
        m_mat3Properties[name] = value;
    }

    Math::Mat3 Material::GetMat3(const std::string& name) const {
        auto it = m_mat3Properties.find(name);
        if (it != m_mat3Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Mat3 property not found: " + name);
        return Math::Mat3(1.0f);
    }

    // Mat4 property methods
    void Material::SetMat4(const std::string& name, const Math::Mat4& value) {
        m_mat4Properties[name] = value;
    }

    Math::Mat4 Material::GetMat4(const std::string& name) const {
        auto it = m_mat4Properties.find(name);
        if (it != m_mat4Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Mat4 property not found: " + name);
        return Math::Mat4(1.0f);
    }

    void Material::Use() const {
        if (m_shader) {
            m_shader->Use();
            ApplyUniforms();
        } else {
            LOG_WARNING("Material has no shader assigned");
        }
    }

    void Material::ApplyUniforms() const {
        if (!m_shader) {
            return;
        }

        // Apply float properties
        for (const auto& [name, value] : m_floatProperties) {
            m_shader->SetFloat(name, value);
        }

        // Apply int properties
        for (const auto& [name, value] : m_intProperties) {
            m_shader->SetInt(name, value);
        }

        // Apply bool properties
        for (const auto& [name, value] : m_boolProperties) {
            m_shader->SetBool(name, value);
        }

        // Apply Vec2 properties
        for (const auto& [name, value] : m_vec2Properties) {
            m_shader->SetVec2(name, value);
        }

        // Apply Vec3 properties
        for (const auto& [name, value] : m_vec3Properties) {
            m_shader->SetVec3(name, value);
        }

        // Apply Vec4 properties
        for (const auto& [name, value] : m_vec4Properties) {
            m_shader->SetVec4(name, value);
        }

        // Apply Mat3 properties
        for (const auto& [name, value] : m_mat3Properties) {
            m_shader->SetMat3(name, value);
        }

        // Apply Mat4 properties
        for (const auto& [name, value] : m_mat4Properties) {
            m_shader->SetMat4(name, value);
        }

        // Apply textures
        int textureUnit = 0;
        for (const auto& [name, texture] : m_textures) {
            if (texture) {
                // Bind texture to texture unit
                // Note: This would need proper OpenGL texture binding implementation
                m_shader->SetInt(name, textureUnit);
                textureUnit++;
            }
        }
    }
}