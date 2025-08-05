#pragma once

#include "Graphics/Material.h"
#include "Core/Math.h"

namespace GameEngine {
    class Texture;

    class PBRMaterial : public Material {
    public:
        struct Properties {
            Math::Vec3 albedo = Math::Vec3(1.0f);
            float metallic = 0.0f;
            float roughness = 0.5f;
            float ao = 1.0f;
            Math::Vec3 emission = Math::Vec3(0.0f);
            float emissionStrength = 1.0f;
            float normalStrength = 1.0f;
            float alphaCutoff = 0.5f;
            bool useAlphaCutoff = false;
        };

        PBRMaterial(const std::string& name = "PBRMaterial");
        ~PBRMaterial() override = default;

        // PBR-specific methods
        void SetProperties(const Properties& props);
        Properties GetProperties() const;

        // Convenience methods for individual properties
        void SetAlbedo(const Math::Vec3& albedo);
        void SetMetallic(float metallic);
        void SetRoughness(float roughness);
        void SetAO(float ao);
        void SetEmission(const Math::Vec3& emission);
        void SetEmissionStrength(float strength);
        void SetNormalStrength(float strength);
        void SetAlphaCutoff(float cutoff);
        void SetUseAlphaCutoff(bool use);

        Math::Vec3 GetAlbedo() const;
        float GetMetallic() const;
        float GetRoughness() const;
        float GetAO() const;
        Math::Vec3 GetEmission() const;
        float GetEmissionStrength() const;
        float GetNormalStrength() const;
        float GetAlphaCutoff() const;
        bool GetUseAlphaCutoff() const;

        // Texture convenience methods
        void SetAlbedoMap(std::shared_ptr<Texture> texture);
        void SetNormalMap(std::shared_ptr<Texture> texture);
        void SetMetallicRoughnessMap(std::shared_ptr<Texture> texture);
        void SetAOMap(std::shared_ptr<Texture> texture);
        void SetEmissionMap(std::shared_ptr<Texture> texture);

        std::shared_ptr<Texture> GetAlbedoMap() const;
        std::shared_ptr<Texture> GetNormalMap() const;
        std::shared_ptr<Texture> GetMetallicRoughnessMap() const;
        std::shared_ptr<Texture> GetAOMap() const;
        std::shared_ptr<Texture> GetEmissionMap() const;

        // Override material application for PBR-specific behavior
        void ApplyToShader(std::shared_ptr<Shader> shader) const override;

        // Validation
        bool ValidateProperties() const;
        void SetDefaultsForMissingProperties();

    private:
        Properties m_properties;

        // Helper methods
        void UpdatePropertySystem();
        void ValidateProperty(const std::string& name, float value, float min, float max);
    };
}