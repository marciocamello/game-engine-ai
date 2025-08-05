#pragma once

#include "PostProcessingPipeline.h"
#include <memory>

namespace GameEngine {
    class Shader;

    // Tone Mapping Effect
    class ToneMappingEffect : public PostProcessEffect {
    public:
        ToneMappingEffect();
        ~ToneMappingEffect() override;

        // PostProcessEffect interface
        bool Initialize(int width, int height) override;
        void Shutdown() override;
        void Resize(int width, int height) override;
        void Process(uint32_t inputTexture, uint32_t outputTexture) override;
        const std::string& GetName() const override;

        // Parameters
        void SetParameter(const std::string& name, float value) override;

        // Tone mapping specific methods
        void SetToneMappingType(ToneMappingType type) { m_toneMappingType = type; }
        void SetExposure(float exposure) { m_exposure = exposure; }
        void SetGamma(float gamma) { m_gamma = gamma; }

        ToneMappingType GetToneMappingType() const { return m_toneMappingType; }
        float GetExposure() const { return m_exposure; }
        float GetGamma() const { return m_gamma; }

    private:
        void CreateShaders();
        void UpdateShaderUniforms();

        std::shared_ptr<Shader> m_shader;
        ToneMappingType m_toneMappingType = ToneMappingType::ACES;
        float m_exposure = 1.0f;
        float m_gamma = 2.2f;
        int m_width = 0;
        int m_height = 0;
        std::string m_name = "ToneMapping";
    };

    // FXAA Anti-Aliasing Effect
    class FXAAEffect : public PostProcessEffect {
    public:
        FXAAEffect();
        ~FXAAEffect() override;

        // PostProcessEffect interface
        bool Initialize(int width, int height) override;
        void Shutdown() override;
        void Resize(int width, int height) override;
        void Process(uint32_t inputTexture, uint32_t outputTexture) override;
        const std::string& GetName() const override;

        // Parameters
        void SetParameter(const std::string& name, float value) override;

        // FXAA specific methods
        void SetQuality(float quality) { m_quality = quality; }
        void SetSubPixelShift(float shift) { m_subPixelShift = shift; }
        void SetEdgeThreshold(float threshold) { m_edgeThreshold = threshold; }
        void SetEdgeThresholdMin(float threshold) { m_edgeThresholdMin = threshold; }

        float GetQuality() const { return m_quality; }
        float GetSubPixelShift() const { return m_subPixelShift; }
        float GetEdgeThreshold() const { return m_edgeThreshold; }
        float GetEdgeThresholdMin() const { return m_edgeThresholdMin; }

    private:
        void CreateShaders();
        void UpdateShaderUniforms();

        std::shared_ptr<Shader> m_shader;
        float m_quality = 0.75f;
        float m_subPixelShift = 0.25f;
        float m_edgeThreshold = 0.166f;
        float m_edgeThresholdMin = 0.0833f;
        int m_width = 0;
        int m_height = 0;
        std::string m_name = "FXAA";
    };

    // Bloom Effect
    class BloomEffect : public PostProcessEffect {
    public:
        BloomEffect();
        ~BloomEffect() override;

        // PostProcessEffect interface
        bool Initialize(int width, int height) override;
        void Shutdown() override;
        void Resize(int width, int height) override;
        void Process(uint32_t inputTexture, uint32_t outputTexture) override;
        const std::string& GetName() const override;

        // Parameters
        void SetParameter(const std::string& name, float value) override;

        // Bloom specific methods
        void SetThreshold(float threshold) { m_threshold = threshold; }
        void SetIntensity(float intensity) { m_intensity = intensity; }
        void SetRadius(float radius) { m_radius = radius; }
        void SetBlurPasses(int passes) { m_blurPasses = passes; }

        float GetThreshold() const { return m_threshold; }
        float GetIntensity() const { return m_intensity; }
        float GetRadius() const { return m_radius; }
        int GetBlurPasses() const { return m_blurPasses; }

    private:
        void CreateShaders();
        void CreateFramebuffers();
        void UpdateShaderUniforms();
        void RenderFullscreenQuad();

        std::shared_ptr<Shader> m_brightPassShader;
        std::shared_ptr<Shader> m_blurShader;
        std::shared_ptr<Shader> m_combineShader;

        // Framebuffers for bloom processing
        uint32_t m_brightPassFBO = 0;
        uint32_t m_brightPassTexture = 0;
        uint32_t m_blurFBO[2] = {0, 0};
        uint32_t m_blurTexture[2] = {0, 0};

        float m_threshold = 1.0f;
        float m_intensity = 0.5f;
        float m_radius = 1.0f;
        int m_blurPasses = 5;
        int m_width = 0;
        int m_height = 0;
        std::string m_name = "Bloom";
    };

} // namespace GameEngine