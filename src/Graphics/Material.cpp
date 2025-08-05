#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <fstream>
#include <stdexcept>

namespace GameEngine {
    // MaterialProperty implementation
    MaterialProperty::MaterialProperty(float value) : m_type(Type::Float), m_value(value) {}
    MaterialProperty::MaterialProperty(int value) : m_type(Type::Int), m_value(value) {}
    MaterialProperty::MaterialProperty(bool value) : m_type(Type::Bool), m_value(value) {}
    MaterialProperty::MaterialProperty(const Math::Vec2& value) : m_type(Type::Vec2), m_value(value) {}
    MaterialProperty::MaterialProperty(const Math::Vec3& value) : m_type(Type::Vec3), m_value(value) {}
    MaterialProperty::MaterialProperty(const Math::Vec4& value) : m_type(Type::Vec4), m_value(value) {}
    MaterialProperty::MaterialProperty(const Math::Mat3& value) : m_type(Type::Mat3), m_value(value) {}
    MaterialProperty::MaterialProperty(const Math::Mat4& value) : m_type(Type::Mat4), m_value(value) {}
    MaterialProperty::MaterialProperty(std::shared_ptr<Texture> texture) : m_type(Type::Texture), m_value(texture) {}

    float MaterialProperty::AsFloat() const {
        if (m_type != Type::Float) {
            throw std::runtime_error("MaterialProperty is not a float");
        }
        return std::get<float>(m_value);
    }

    int MaterialProperty::AsInt() const {
        if (m_type != Type::Int) {
            throw std::runtime_error("MaterialProperty is not an int");
        }
        return std::get<int>(m_value);
    }

    bool MaterialProperty::AsBool() const {
        if (m_type != Type::Bool) {
            throw std::runtime_error("MaterialProperty is not a bool");
        }
        return std::get<bool>(m_value);
    }

    Math::Vec2 MaterialProperty::AsVec2() const {
        if (m_type != Type::Vec2) {
            throw std::runtime_error("MaterialProperty is not a Vec2");
        }
        return std::get<Math::Vec2>(m_value);
    }

    Math::Vec3 MaterialProperty::AsVec3() const {
        if (m_type != Type::Vec3) {
            throw std::runtime_error("MaterialProperty is not a Vec3");
        }
        return std::get<Math::Vec3>(m_value);
    }

    Math::Vec4 MaterialProperty::AsVec4() const {
        if (m_type != Type::Vec4) {
            throw std::runtime_error("MaterialProperty is not a Vec4");
        }
        return std::get<Math::Vec4>(m_value);
    }

    Math::Mat3 MaterialProperty::AsMat3() const {
        if (m_type != Type::Mat3) {
            throw std::runtime_error("MaterialProperty is not a Mat3");
        }
        return std::get<Math::Mat3>(m_value);
    }

    Math::Mat4 MaterialProperty::AsMat4() const {
        if (m_type != Type::Mat4) {
            throw std::runtime_error("MaterialProperty is not a Mat4");
        }
        return std::get<Math::Mat4>(m_value);
    }

    std::shared_ptr<Texture> MaterialProperty::AsTexture() const {
        if (m_type != Type::Texture) {
            throw std::runtime_error("MaterialProperty is not a Texture");
        }
        return std::get<std::shared_ptr<Texture>>(m_value);
    }

    nlohmann::json MaterialProperty::Serialize() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(m_type);

        switch (m_type) {
            case Type::Float:
                json["value"] = AsFloat();
                break;
            case Type::Int:
                json["value"] = AsInt();
                break;
            case Type::Bool:
                json["value"] = AsBool();
                break;
            case Type::Vec2: {
                auto vec = AsVec2();
                json["value"] = {vec.x, vec.y};
                break;
            }
            case Type::Vec3: {
                auto vec = AsVec3();
                json["value"] = {vec.x, vec.y, vec.z};
                break;
            }
            case Type::Vec4: {
                auto vec = AsVec4();
                json["value"] = {vec.x, vec.y, vec.z, vec.w};
                break;
            }
            case Type::Mat3: {
                auto mat = AsMat3();
                json["value"] = nlohmann::json::array();
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        json["value"].push_back(mat[i][j]);
                    }
                }
                break;
            }
            case Type::Mat4: {
                auto mat = AsMat4();
                json["value"] = nlohmann::json::array();
                for (int i = 0; i < 4; ++i) {
                    for (int j = 0; j < 4; ++j) {
                        json["value"].push_back(mat[i][j]);
                    }
                }
                break;
            }
            case Type::Texture:
                // For textures, we'll store the path if available
                json["value"] = ""; // Placeholder - would need texture path
                break;
        }

        return json;
    }

    bool MaterialProperty::Deserialize(const nlohmann::json& json) {
        if (!json.contains("type") || !json.contains("value")) {
            return false;
        }

        m_type = static_cast<Type>(json["type"].get<int>());

        try {
            switch (m_type) {
                case Type::Float:
                    m_value = json["value"].get<float>();
                    break;
                case Type::Int:
                    m_value = json["value"].get<int>();
                    break;
                case Type::Bool:
                    m_value = json["value"].get<bool>();
                    break;
                case Type::Vec2: {
                    auto arr = json["value"];
                    m_value = Math::Vec2(arr[0].get<float>(), arr[1].get<float>());
                    break;
                }
                case Type::Vec3: {
                    auto arr = json["value"];
                    m_value = Math::Vec3(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>());
                    break;
                }
                case Type::Vec4: {
                    auto arr = json["value"];
                    m_value = Math::Vec4(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>(), arr[3].get<float>());
                    break;
                }
                case Type::Mat3: {
                    auto arr = json["value"];
                    Math::Mat3 mat;
                    int idx = 0;
                    for (int i = 0; i < 3; ++i) {
                        for (int j = 0; j < 3; ++j) {
                            mat[i][j] = arr[idx++].get<float>();
                        }
                    }
                    m_value = mat;
                    break;
                }
                case Type::Mat4: {
                    auto arr = json["value"];
                    Math::Mat4 mat;
                    int idx = 0;
                    for (int i = 0; i < 4; ++i) {
                        for (int j = 0; j < 4; ++j) {
                            mat[i][j] = arr[idx++].get<float>();
                        }
                    }
                    m_value = mat;
                    break;
                }
                case Type::Texture:
                    // Texture deserialization would need resource manager integration
                    m_value = std::shared_ptr<Texture>(nullptr);
                    break;
            }
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize MaterialProperty: " + std::string(e.what()));
            return false;
        }
    }

    Material::Material(const std::string& path, Type type) : Resource(path), m_type(type), m_name("Material") {
        ApplyTemplate();
    }

    Material::~Material() {
        // Clear all property maps
        m_properties.clear();
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

    // Advanced property system implementation
    void Material::SetProperty(const std::string& name, const MaterialProperty& value) {
        m_properties[name] = value;
        
        // Also update legacy storage for backward compatibility
        switch (value.GetType()) {
            case MaterialProperty::Type::Float:
                m_floatProperties[name] = value.AsFloat();
                break;
            case MaterialProperty::Type::Int:
                m_intProperties[name] = value.AsInt();
                break;
            case MaterialProperty::Type::Bool:
                m_boolProperties[name] = value.AsBool();
                break;
            case MaterialProperty::Type::Vec2:
                m_vec2Properties[name] = value.AsVec2();
                break;
            case MaterialProperty::Type::Vec3:
                m_vec3Properties[name] = value.AsVec3();
                break;
            case MaterialProperty::Type::Vec4:
                m_vec4Properties[name] = value.AsVec4();
                break;
            case MaterialProperty::Type::Mat3:
                m_mat3Properties[name] = value.AsMat3();
                break;
            case MaterialProperty::Type::Mat4:
                m_mat4Properties[name] = value.AsMat4();
                break;
            case MaterialProperty::Type::Texture:
                m_textures[name] = value.AsTexture();
                break;
        }
    }

    MaterialProperty Material::GetProperty(const std::string& name) const {
        auto it = m_properties.find(name);
        if (it != m_properties.end()) {
            return it->second;
        }
        
        LOG_WARNING("Material property not found: " + name);
        return MaterialProperty(0.0f); // Default to float 0
    }

    bool Material::HasProperty(const std::string& name) const {
        return m_properties.find(name) != m_properties.end();
    }

    void Material::RemoveProperty(const std::string& name) {
        auto it = m_properties.find(name);
        if (it != m_properties.end()) {
            // Remove from advanced property system
            m_properties.erase(it);
            
            // Remove from legacy storage
            m_floatProperties.erase(name);
            m_intProperties.erase(name);
            m_boolProperties.erase(name);
            m_vec2Properties.erase(name);
            m_vec3Properties.erase(name);
            m_vec4Properties.erase(name);
            m_mat3Properties.erase(name);
            m_mat4Properties.erase(name);
            m_textures.erase(name);
        }
    }

    void Material::SetTexture(const std::string& name, std::shared_ptr<Texture> texture) {
        if (texture) {
            m_textures[name] = texture;
            SetProperty(name, MaterialProperty(texture));
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
            RemoveProperty(name);
        }
    }

    // Legacy property methods (for backward compatibility)
    void Material::SetFloat(const std::string& name, float value) {
        m_floatProperties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    float Material::GetFloat(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsFloat();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_floatProperties.find(name);
        if (it != m_floatProperties.end()) {
            return it->second;
        }
        LOG_WARNING("Float property not found: " + name);
        return 0.0f;
    }

    void Material::SetInt(const std::string& name, int value) {
        m_intProperties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    int Material::GetInt(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsInt();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_intProperties.find(name);
        if (it != m_intProperties.end()) {
            return it->second;
        }
        LOG_WARNING("Int property not found: " + name);
        return 0;
    }

    void Material::SetBool(const std::string& name, bool value) {
        m_boolProperties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    bool Material::GetBool(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsBool();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_boolProperties.find(name);
        if (it != m_boolProperties.end()) {
            return it->second;
        }
        LOG_WARNING("Bool property not found: " + name);
        return false;
    }

    void Material::SetVec2(const std::string& name, const Math::Vec2& value) {
        m_vec2Properties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    Math::Vec2 Material::GetVec2(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsVec2();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_vec2Properties.find(name);
        if (it != m_vec2Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Vec2 property not found: " + name);
        return Math::Vec2(0.0f);
    }

    void Material::SetVec3(const std::string& name, const Math::Vec3& value) {
        m_vec3Properties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    Math::Vec3 Material::GetVec3(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsVec3();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_vec3Properties.find(name);
        if (it != m_vec3Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Vec3 property not found: " + name);
        return Math::Vec3(0.0f);
    }

    void Material::SetVec4(const std::string& name, const Math::Vec4& value) {
        m_vec4Properties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    Math::Vec4 Material::GetVec4(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsVec4();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_vec4Properties.find(name);
        if (it != m_vec4Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Vec4 property not found: " + name);
        return Math::Vec4(0.0f);
    }

    void Material::SetMat3(const std::string& name, const Math::Mat3& value) {
        m_mat3Properties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    Math::Mat3 Material::GetMat3(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsMat3();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_mat3Properties.find(name);
        if (it != m_mat3Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Mat3 property not found: " + name);
        return Math::Mat3(1.0f);
    }

    void Material::SetMat4(const std::string& name, const Math::Mat4& value) {
        m_mat4Properties[name] = value;
        SetProperty(name, MaterialProperty(value));
    }

    Math::Mat4 Material::GetMat4(const std::string& name) const {
        if (HasProperty(name)) {
            try {
                return GetProperty(name).AsMat4();
            } catch (const std::exception&) {
                // Fall back to legacy storage
            }
        }
        
        auto it = m_mat4Properties.find(name);
        if (it != m_mat4Properties.end()) {
            return it->second;
        }
        LOG_WARNING("Mat4 property not found: " + name);
        return Math::Mat4(1.0f);
    }

    void Material::Bind() const {
        if (m_shader) {
            m_shader->Use();
            ApplyToShader(m_shader);
        } else {
            LOG_WARNING("Material has no shader assigned");
        }
    }

    void Material::Unbind() const {
        // In OpenGL, we typically don't need to explicitly unbind materials
        // This is here for API consistency and future extensions
    }

    void Material::ApplyToShader(std::shared_ptr<Shader> shader) const {
        if (!shader) {
            return;
        }

        // Apply properties using the advanced property system
        for (const auto& [name, property] : m_properties) {
            switch (property.GetType()) {
                case MaterialProperty::Type::Float:
                    shader->SetFloat(name, property.AsFloat());
                    break;
                case MaterialProperty::Type::Int:
                    shader->SetInt(name, property.AsInt());
                    break;
                case MaterialProperty::Type::Bool:
                    shader->SetBool(name, property.AsBool());
                    break;
                case MaterialProperty::Type::Vec2:
                    shader->SetVec2(name, property.AsVec2());
                    break;
                case MaterialProperty::Type::Vec3:
                    shader->SetVec3(name, property.AsVec3());
                    break;
                case MaterialProperty::Type::Vec4:
                    shader->SetVec4(name, property.AsVec4());
                    break;
                case MaterialProperty::Type::Mat3:
                    shader->SetMat3(name, property.AsMat3());
                    break;
                case MaterialProperty::Type::Mat4:
                    shader->SetMat4(name, property.AsMat4());
                    break;
                case MaterialProperty::Type::Texture:
                    // Texture binding would be handled separately
                    break;
            }
        }

        // Apply textures
        int textureUnit = 0;
        for (const auto& [name, texture] : m_textures) {
            if (texture) {
                // Bind texture to texture unit
                // Note: This would need proper OpenGL texture binding implementation
                shader->SetInt(name, textureUnit);
                textureUnit++;
            }
        }
    }

    void Material::Use() const {
        // Legacy method for backward compatibility
        Bind();
    }

    void Material::ApplyUniforms() const {
        // Legacy method for backward compatibility
        if (m_shader) {
            ApplyToShader(m_shader);
        }
    }

    bool Material::LoadFromFile(const std::string& filepath) {
        LOG_INFO("Loading material from file: " + filepath);
        
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open material file: " + filepath);
                return false;
            }
            
            nlohmann::json json;
            file >> json;
            file.close();
            
            bool success = Deserialize(json);
            if (success) {
                LOG_INFO("Material loaded successfully: " + filepath);
            } else {
                LOG_ERROR("Failed to deserialize material from file: " + filepath);
            }
            
            return success;
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while loading material file: " + std::string(e.what()));
            return false;
        }
    }

    void Material::CreateDefault() {
        LOG_INFO("Creating default material");
        
        // Clear all properties
        m_properties.clear();
        m_textures.clear();
        m_floatProperties.clear();
        m_intProperties.clear();
        m_boolProperties.clear();
        m_vec2Properties.clear();
        m_vec3Properties.clear();
        m_vec4Properties.clear();
        m_mat3Properties.clear();
        m_mat4Properties.clear();
        
        // Apply template based on material type
        ApplyTemplate();
        
        LOG_INFO("Default material created");
    }

    size_t Material::GetMemoryUsage() const {
        size_t totalMemory = sizeof(*this);
        
        // Add memory for advanced property system
        totalMemory += m_properties.size() * (sizeof(std::string) + sizeof(MaterialProperty));
        for (const auto& [name, property] : m_properties) {
            totalMemory += name.capacity();
        }
        
        // Add memory for legacy property maps (for backward compatibility)
        totalMemory += m_floatProperties.size() * (sizeof(std::string) + sizeof(float));
        totalMemory += m_intProperties.size() * (sizeof(std::string) + sizeof(int));
        totalMemory += m_boolProperties.size() * (sizeof(std::string) + sizeof(bool));
        totalMemory += m_vec2Properties.size() * (sizeof(std::string) + sizeof(Math::Vec2));
        totalMemory += m_vec3Properties.size() * (sizeof(std::string) + sizeof(Math::Vec3));
        totalMemory += m_vec4Properties.size() * (sizeof(std::string) + sizeof(Math::Vec4));
        totalMemory += m_mat3Properties.size() * (sizeof(std::string) + sizeof(Math::Mat3));
        totalMemory += m_mat4Properties.size() * (sizeof(std::string) + sizeof(Math::Mat4));
        
        // Add memory for texture references (not the actual texture data)
        totalMemory += m_textures.size() * (sizeof(std::string) + sizeof(std::shared_ptr<Texture>));
        
        // Add string capacity for legacy property names
        for (const auto& pair : m_floatProperties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_intProperties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_boolProperties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_vec2Properties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_vec3Properties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_vec4Properties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_mat3Properties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_mat4Properties) {
            totalMemory += pair.first.capacity();
        }
        for (const auto& pair : m_textures) {
            totalMemory += pair.first.capacity();
        }
        
        return totalMemory;
    }

    // Material template system
    std::shared_ptr<Material> Material::CreateFromTemplate(Type type, const std::string& name) {
        auto material = std::make_shared<Material>("", type);
        if (!name.empty()) {
            material->SetName(name);
        }
        return material;
    }

    void Material::ApplyTemplate() {
        switch (m_type) {
            case Type::PBR:
                SetupPBRTemplate();
                break;
            case Type::Unlit:
                SetupUnlitTemplate();
                break;
            case Type::Custom:
                SetupCustomTemplate();
                break;
            case Type::PostProcess:
                SetupPostProcessTemplate();
                break;
        }
    }

    void Material::SetupPBRTemplate() {
        // Set default PBR values
        SetProperty("u_albedo", MaterialProperty(Math::Vec3(0.8f, 0.8f, 0.8f)));
        SetProperty("u_metallic", MaterialProperty(0.0f));
        SetProperty("u_roughness", MaterialProperty(0.5f));
        SetProperty("u_ao", MaterialProperty(1.0f));
        SetProperty("u_emission", MaterialProperty(Math::Vec3(0.0f, 0.0f, 0.0f)));
        SetProperty("u_emissionStrength", MaterialProperty(1.0f));
        SetProperty("u_normalStrength", MaterialProperty(1.0f));
        SetProperty("u_alphaCutoff", MaterialProperty(0.5f));
        SetProperty("u_useAlphaCutoff", MaterialProperty(false));
    }

    void Material::SetupUnlitTemplate() {
        // Set default unlit values
        SetProperty("u_color", MaterialProperty(Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f)));
        SetProperty("u_brightness", MaterialProperty(1.0f));
    }

    void Material::SetupCustomTemplate() {
        // Custom materials start with minimal properties
        SetProperty("u_color", MaterialProperty(Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    }

    void Material::SetupPostProcessTemplate() {
        // Post-process materials have screen-space properties
        SetProperty("u_screenTexture", MaterialProperty(0));
        SetProperty("u_exposure", MaterialProperty(1.0f));
        SetProperty("u_gamma", MaterialProperty(2.2f));
    }

    // Serialization methods
    void Material::SaveToFile(const std::string& filepath) const {
        try {
            nlohmann::json json = Serialize();
            std::ofstream file(filepath);
            if (file.is_open()) {
                file << json.dump(4);
                file.close();
                LOG_INFO("Material saved to file: " + filepath);
            } else {
                LOG_ERROR("Failed to open file for writing: " + filepath);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save material to file: " + std::string(e.what()));
        }
    }

    nlohmann::json Material::Serialize() const {
        nlohmann::json json;
        
        json["name"] = m_name;
        json["type"] = static_cast<int>(m_type);
        
        // Serialize properties
        json["properties"] = nlohmann::json::object();
        for (const auto& [name, property] : m_properties) {
            json["properties"][name] = property.Serialize();
        }
        
        // Serialize texture references (paths would need to be stored)
        json["textures"] = nlohmann::json::object();
        for (const auto& [name, texture] : m_textures) {
            if (texture) {
                // For now, we'll store a placeholder
                // In a full implementation, this would store the texture path
                json["textures"][name] = "texture_path_placeholder";
            }
        }
        
        return json;
    }

    bool Material::Deserialize(const nlohmann::json& json) {
        try {
            if (json.contains("name")) {
                m_name = json["name"].get<std::string>();
            }
            
            if (json.contains("type")) {
                m_type = static_cast<Type>(json["type"].get<int>());
            }
            
            // Clear existing properties
            m_properties.clear();
            m_textures.clear();
            
            // Deserialize properties
            if (json.contains("properties")) {
                for (const auto& [name, propJson] : json["properties"].items()) {
                    MaterialProperty property;
                    if (property.Deserialize(propJson)) {
                        SetProperty(name, property);
                    }
                }
            }
            
            // Deserialize textures (would need resource manager integration)
            if (json.contains("textures")) {
                for (const auto& [name, texturePath] : json["textures"].items()) {
                    // In a full implementation, this would load the texture from the path
                    LOG_INFO("Texture reference found for property: " + name + " -> " + texturePath.get<std::string>());
                }
            }
            
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to deserialize material: " + std::string(e.what()));
            return false;
        }
    }}
