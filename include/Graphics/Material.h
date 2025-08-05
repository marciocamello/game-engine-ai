#pragma once

#include "Core/Math.h"
#include "Resource/ResourceManager.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <variant>
#include <nlohmann/json.hpp>

namespace GameEngine {
    class Shader;
    class Texture;

    // Material property variant type for advanced property system
    class MaterialProperty {
    public:
        enum class Type { Float, Int, Bool, Vec2, Vec3, Vec4, Mat3, Mat4, Texture };

        MaterialProperty() = default;
        MaterialProperty(float value);
        MaterialProperty(int value);
        MaterialProperty(bool value);
        MaterialProperty(const Math::Vec2& value);
        MaterialProperty(const Math::Vec3& value);
        MaterialProperty(const Math::Vec4& value);
        MaterialProperty(const Math::Mat3& value);
        MaterialProperty(const Math::Mat4& value);
        MaterialProperty(std::shared_ptr<Texture> texture);

        Type GetType() const { return m_type; }

        float AsFloat() const;
        int AsInt() const;
        bool AsBool() const;
        Math::Vec2 AsVec2() const;
        Math::Vec3 AsVec3() const;
        Math::Vec4 AsVec4() const;
        Math::Mat3 AsMat3() const;
        Math::Mat4 AsMat4() const;
        std::shared_ptr<Texture> AsTexture() const;

        nlohmann::json Serialize() const;
        bool Deserialize(const nlohmann::json& json);

    private:
        Type m_type;
        std::variant<float, int, bool, Math::Vec2, Math::Vec3, Math::Vec4, Math::Mat3, Math::Mat4, std::shared_ptr<Texture>> m_value;
    };

    class Material : public Resource {
    public:
        enum class Type { PBR, Unlit, Custom, PostProcess };

        explicit Material(const std::string& path = "", Type type = Type::PBR);
        virtual ~Material();

        // Resource interface
        bool LoadFromFile(const std::string& filepath) override;
        void CreateDefault() override;
        size_t GetMemoryUsage() const override;

        // Shader management
        void SetShader(std::shared_ptr<Shader> shader) { m_shader = shader; }
        std::shared_ptr<Shader> GetShader() const { return m_shader; }

        // Advanced property system
        virtual void SetProperty(const std::string& name, const MaterialProperty& value);
        virtual MaterialProperty GetProperty(const std::string& name) const;
        virtual bool HasProperty(const std::string& name) const;
        virtual void RemoveProperty(const std::string& name);

        // Texture management
        void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
        std::shared_ptr<Texture> GetTexture(const std::string& name) const;
        void RemoveTexture(const std::string& name);

        // Legacy property setters (for backward compatibility)
        void SetFloat(const std::string& name, float value);
        void SetInt(const std::string& name, int value);
        void SetBool(const std::string& name, bool value);
        void SetVec2(const std::string& name, const Math::Vec2& value);
        void SetVec3(const std::string& name, const Math::Vec3& value);
        void SetVec4(const std::string& name, const Math::Vec4& value);
        void SetMat3(const std::string& name, const Math::Mat3& value);
        void SetMat4(const std::string& name, const Math::Mat4& value);

        // Legacy property getters (for backward compatibility)
        float GetFloat(const std::string& name) const;
        int GetInt(const std::string& name) const;
        bool GetBool(const std::string& name) const;
        Math::Vec2 GetVec2(const std::string& name) const;
        Math::Vec3 GetVec3(const std::string& name) const;
        Math::Vec4 GetVec4(const std::string& name) const;
        Math::Mat3 GetMat3(const std::string& name) const;
        Math::Mat4 GetMat4(const std::string& name) const;

        // Material application
        virtual void Bind() const;
        virtual void Unbind() const;
        virtual void ApplyToShader(std::shared_ptr<Shader> shader) const;
        void Use() const; // Legacy method
        void ApplyUniforms() const; // Legacy method for backward compatibility

        // Material template system
        static std::shared_ptr<Material> CreateFromTemplate(Type type, const std::string& name = "");
        virtual void ApplyTemplate();

        // Serialization
        void SaveToFile(const std::string& filepath) const;
        nlohmann::json Serialize() const;
        bool Deserialize(const nlohmann::json& json);

        // Properties
        Type GetType() const { return m_type; }
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        // PBR convenience methods (for backward compatibility)
        void SetAlbedo(const Math::Vec3& albedo) { SetVec3("u_albedo", albedo); }
        void SetMetallic(float metallic) { SetFloat("u_metallic", metallic); }
        void SetRoughness(float roughness) { SetFloat("u_roughness", roughness); }
        void SetAO(float ao) { SetFloat("u_ao", ao); }

        Math::Vec3 GetAlbedo() const { return GetVec3("u_albedo"); }
        float GetMetallic() const { return GetFloat("u_metallic"); }
        float GetRoughness() const { return GetFloat("u_roughness"); }
        float GetAO() const { return GetFloat("u_ao"); }

    protected:
        Type m_type;
        std::string m_name;
        std::shared_ptr<Shader> m_shader;
        std::unordered_map<std::string, MaterialProperty> m_properties;
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
        
        // Legacy property storage (for backward compatibility)
        std::unordered_map<std::string, float> m_floatProperties;
        std::unordered_map<std::string, int> m_intProperties;
        std::unordered_map<std::string, bool> m_boolProperties;
        std::unordered_map<std::string, Math::Vec2> m_vec2Properties;
        std::unordered_map<std::string, Math::Vec3> m_vec3Properties;
        std::unordered_map<std::string, Math::Vec4> m_vec4Properties;
        std::unordered_map<std::string, Math::Mat3> m_mat3Properties;
        std::unordered_map<std::string, Math::Mat4> m_mat4Properties;

        // Template creation helpers
        virtual void SetupPBRTemplate();
        virtual void SetupUnlitTemplate();
        virtual void SetupCustomTemplate();
        virtual void SetupPostProcessTemplate();
    };
}