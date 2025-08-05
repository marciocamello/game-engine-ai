#include "Graphics/PBRMaterial.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
    PBRMaterial::PBRMaterial(const std::string& name) : Material("", Type::PBR) {
        SetName(name);
        
        // Set default PBR properties
        SetProperties(Properties{});
        
        LOG_INFO("Created PBR material: " + name);
    }

    void PBRMaterial::SetProperties(const Properties& props) {
        m_properties = props;
        UpdatePropertySystem();
        
        // Validate properties
        if (!ValidateProperties()) {
            LOG_WARNING("PBR material properties validation failed, using defaults");
            SetDefaultsForMissingProperties();
        }
    }

    PBRMaterial::Properties PBRMaterial::GetProperties() const {
        return m_properties;
    }

    // Individual property setters
    void PBRMaterial::SetAlbedo(const Math::Vec3& albedo) {
        m_properties.albedo = albedo;
        SetProperty("u_albedo", MaterialProperty(albedo));
    }

    void PBRMaterial::SetMetallic(float metallic) {
        ValidateProperty("metallic", metallic, 0.0f, 1.0f);
        m_properties.metallic = std::clamp(metallic, 0.0f, 1.0f);
        SetProperty("u_metallic", MaterialProperty(m_properties.metallic));
    }

    void PBRMaterial::SetRoughness(float roughness) {
        ValidateProperty("roughness", roughness, 0.0f, 1.0f);
        m_properties.roughness = std::clamp(roughness, 0.0f, 1.0f);
        SetProperty("u_roughness", MaterialProperty(m_properties.roughness));
    }

    void PBRMaterial::SetAO(float ao) {
        ValidateProperty("ao", ao, 0.0f, 1.0f);
        m_properties.ao = std::clamp(ao, 0.0f, 1.0f);
        SetProperty("u_ao", MaterialProperty(m_properties.ao));
    }

    void PBRMaterial::SetEmission(const Math::Vec3& emission) {
        m_properties.emission = emission;
        SetProperty("u_emission", MaterialProperty(emission));
    }

    void PBRMaterial::SetEmissionStrength(float strength) {
        ValidateProperty("emissionStrength", strength, 0.0f, 10.0f);
        m_properties.emissionStrength = std::max(0.0f, strength);
        SetProperty("u_emissionStrength", MaterialProperty(m_properties.emissionStrength));
    }

    void PBRMaterial::SetNormalStrength(float strength) {
        ValidateProperty("normalStrength", strength, 0.0f, 2.0f);
        m_properties.normalStrength = std::max(0.0f, strength);
        SetProperty("u_normalStrength", MaterialProperty(m_properties.normalStrength));
    }

    void PBRMaterial::SetAlphaCutoff(float cutoff) {
        ValidateProperty("alphaCutoff", cutoff, 0.0f, 1.0f);
        m_properties.alphaCutoff = std::clamp(cutoff, 0.0f, 1.0f);
        SetProperty("u_alphaCutoff", MaterialProperty(m_properties.alphaCutoff));
    }

    void PBRMaterial::SetUseAlphaCutoff(bool use) {
        m_properties.useAlphaCutoff = use;
        SetProperty("u_useAlphaCutoff", MaterialProperty(use));
    }

    // Individual property getters
    Math::Vec3 PBRMaterial::GetAlbedo() const {
        return m_properties.albedo;
    }

    float PBRMaterial::GetMetallic() const {
        return m_properties.metallic;
    }

    float PBRMaterial::GetRoughness() const {
        return m_properties.roughness;
    }

    float PBRMaterial::GetAO() const {
        return m_properties.ao;
    }

    Math::Vec3 PBRMaterial::GetEmission() const {
        return m_properties.emission;
    }

    float PBRMaterial::GetEmissionStrength() const {
        return m_properties.emissionStrength;
    }

    float PBRMaterial::GetNormalStrength() const {
        return m_properties.normalStrength;
    }

    float PBRMaterial::GetAlphaCutoff() const {
        return m_properties.alphaCutoff;
    }

    bool PBRMaterial::GetUseAlphaCutoff() const {
        return m_properties.useAlphaCutoff;
    }

    // Texture convenience methods
    void PBRMaterial::SetAlbedoMap(std::shared_ptr<Texture> texture) {
        SetTexture("u_albedoMap", texture);
        if (texture) {
            SetProperty("u_hasAlbedoMap", MaterialProperty(true));
        } else {
            SetProperty("u_hasAlbedoMap", MaterialProperty(false));
        }
    }

    void PBRMaterial::SetNormalMap(std::shared_ptr<Texture> texture) {
        SetTexture("u_normalMap", texture);
        if (texture) {
            SetProperty("u_hasNormalMap", MaterialProperty(true));
        } else {
            SetProperty("u_hasNormalMap", MaterialProperty(false));
        }
    }

    void PBRMaterial::SetMetallicRoughnessMap(std::shared_ptr<Texture> texture) {
        SetTexture("u_metallicRoughnessMap", texture);
        if (texture) {
            SetProperty("u_hasMetallicRoughnessMap", MaterialProperty(true));
        } else {
            SetProperty("u_hasMetallicRoughnessMap", MaterialProperty(false));
        }
    }

    void PBRMaterial::SetAOMap(std::shared_ptr<Texture> texture) {
        SetTexture("u_aoMap", texture);
        if (texture) {
            SetProperty("u_hasAOMap", MaterialProperty(true));
        } else {
            SetProperty("u_hasAOMap", MaterialProperty(false));
        }
    }

    void PBRMaterial::SetEmissionMap(std::shared_ptr<Texture> texture) {
        SetTexture("u_emissionMap", texture);
        if (texture) {
            SetProperty("u_hasEmissionMap", MaterialProperty(true));
        } else {
            SetProperty("u_hasEmissionMap", MaterialProperty(false));
        }
    }

    std::shared_ptr<Texture> PBRMaterial::GetAlbedoMap() const {
        return GetTexture("u_albedoMap");
    }

    std::shared_ptr<Texture> PBRMaterial::GetNormalMap() const {
        return GetTexture("u_normalMap");
    }

    std::shared_ptr<Texture> PBRMaterial::GetMetallicRoughnessMap() const {
        return GetTexture("u_metallicRoughnessMap");
    }

    std::shared_ptr<Texture> PBRMaterial::GetAOMap() const {
        return GetTexture("u_aoMap");
    }

    std::shared_ptr<Texture> PBRMaterial::GetEmissionMap() const {
        return GetTexture("u_emissionMap");
    }

    void PBRMaterial::ApplyToShader(std::shared_ptr<Shader> shader) const {
        if (!shader) {
            LOG_WARNING("Cannot apply PBR material to null shader");
            return;
        }

        // Apply base material properties first
        Material::ApplyToShader(shader);

        // Apply PBR-specific uniforms with proper naming for basic.frag compatibility
        shader->SetVec3("u_albedo", m_properties.albedo);
        shader->SetFloat("u_metallic", m_properties.metallic);
        shader->SetFloat("u_roughness", m_properties.roughness);
        shader->SetFloat("u_ao", m_properties.ao);
        shader->SetVec3("u_emission", m_properties.emission);
        shader->SetFloat("u_emissionStrength", m_properties.emissionStrength);
        shader->SetFloat("u_normalStrength", m_properties.normalStrength);
        shader->SetFloat("u_alphaCutoff", m_properties.alphaCutoff);
        shader->SetBool("u_useAlphaCutoff", m_properties.useAlphaCutoff);

        // Apply texture availability flags
        shader->SetBool("u_hasAlbedoMap", GetAlbedoMap() != nullptr);
        shader->SetBool("u_hasNormalMap", GetNormalMap() != nullptr);
        shader->SetBool("u_hasMetallicRoughnessMap", GetMetallicRoughnessMap() != nullptr);
        shader->SetBool("u_hasAOMap", GetAOMap() != nullptr);
        shader->SetBool("u_hasEmissionMap", GetEmissionMap() != nullptr);

        // Bind textures to specific texture units for consistency with basic.frag
        int textureUnit = 0;
        if (auto albedoMap = GetAlbedoMap()) {
            shader->SetInt("u_albedoMap", textureUnit++);
        }
        if (auto normalMap = GetNormalMap()) {
            shader->SetInt("u_normalMap", textureUnit++);
        }
        if (auto metallicRoughnessMap = GetMetallicRoughnessMap()) {
            shader->SetInt("u_metallicRoughnessMap", textureUnit++);
        }
        if (auto aoMap = GetAOMap()) {
            shader->SetInt("u_aoMap", textureUnit++);
        }
        if (auto emissionMap = GetEmissionMap()) {
            shader->SetInt("u_emissionMap", textureUnit++);
        }
    }

    bool PBRMaterial::ValidateProperties() const {
        bool valid = true;

        // Validate ranges
        if (m_properties.metallic < 0.0f || m_properties.metallic > 1.0f) {
            LOG_WARNING("PBR material metallic value out of range [0,1]: " + std::to_string(m_properties.metallic));
            valid = false;
        }

        if (m_properties.roughness < 0.0f || m_properties.roughness > 1.0f) {
            LOG_WARNING("PBR material roughness value out of range [0,1]: " + std::to_string(m_properties.roughness));
            valid = false;
        }

        if (m_properties.ao < 0.0f || m_properties.ao > 1.0f) {
            LOG_WARNING("PBR material AO value out of range [0,1]: " + std::to_string(m_properties.ao));
            valid = false;
        }

        if (m_properties.emissionStrength < 0.0f) {
            LOG_WARNING("PBR material emission strength cannot be negative: " + std::to_string(m_properties.emissionStrength));
            valid = false;
        }

        if (m_properties.normalStrength < 0.0f) {
            LOG_WARNING("PBR material normal strength cannot be negative: " + std::to_string(m_properties.normalStrength));
            valid = false;
        }

        if (m_properties.alphaCutoff < 0.0f || m_properties.alphaCutoff > 1.0f) {
            LOG_WARNING("PBR material alpha cutoff value out of range [0,1]: " + std::to_string(m_properties.alphaCutoff));
            valid = false;
        }

        return valid;
    }

    void PBRMaterial::SetDefaultsForMissingProperties() {
        // Clamp values to valid ranges
        m_properties.metallic = std::clamp(m_properties.metallic, 0.0f, 1.0f);
        m_properties.roughness = std::clamp(m_properties.roughness, 0.0f, 1.0f);
        m_properties.ao = std::clamp(m_properties.ao, 0.0f, 1.0f);
        m_properties.emissionStrength = std::max(0.0f, m_properties.emissionStrength);
        m_properties.normalStrength = std::max(0.0f, m_properties.normalStrength);
        m_properties.alphaCutoff = std::clamp(m_properties.alphaCutoff, 0.0f, 1.0f);

        // Update the property system with corrected values
        UpdatePropertySystem();
    }

    void PBRMaterial::UpdatePropertySystem() {
        // Update the advanced property system with current values
        SetProperty("u_albedo", MaterialProperty(m_properties.albedo));
        SetProperty("u_metallic", MaterialProperty(m_properties.metallic));
        SetProperty("u_roughness", MaterialProperty(m_properties.roughness));
        SetProperty("u_ao", MaterialProperty(m_properties.ao));
        SetProperty("u_emission", MaterialProperty(m_properties.emission));
        SetProperty("u_emissionStrength", MaterialProperty(m_properties.emissionStrength));
        SetProperty("u_normalStrength", MaterialProperty(m_properties.normalStrength));
        SetProperty("u_alphaCutoff", MaterialProperty(m_properties.alphaCutoff));
        SetProperty("u_useAlphaCutoff", MaterialProperty(m_properties.useAlphaCutoff));
    }

    void PBRMaterial::ValidateProperty(const std::string& name, float value, float min, float max) {
        if (value < min || value > max) {
            LOG_WARNING("PBR material property '" + name + "' value " + std::to_string(value) + 
                       " is out of range [" + std::to_string(min) + "," + std::to_string(max) + "]");
        }
    }
}