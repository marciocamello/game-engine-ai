#pragma once

#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace GameEngine {
    class Shader;
    class Texture;

    enum class ToneMappingType {
        None,
        Reinhard,
        ACES,
        Filmic
    };

    enum class QualityLevel {
        Low,
        Medium,
        High,
        Ultra
    };

    struct PostProcessStats {
        float totalProcessingTime = 0.0f;
        int activeEffects = 0;
        int framebuffersUsed = 0;
        size_t memoryUsage = 0;
    };

    // Base class for all post-processing effects
    class PostProcessEffect {
    public:
        virtual ~PostProcessEffect() = default;

        // Effect interface
        virtual bool Initialize(int width, int height) = 0;
        virtual void Shutdown() = 0;
        virtual void Resize(int width, int height) = 0;
        virtual void Process(uint32_t inputTexture, uint32_t outputTexture) = 0;

        // Properties
        virtual const std::string& GetName() const = 0;
        virtual void SetEnabled(bool enabled) { m_enabled = enabled; }
        virtual bool IsEnabled() const { return m_enabled; }

        // Parameters
        virtual void SetParameter(const std::string& /*name*/, float /*value*/) {}
        virtual void SetParameter(const std::string& /*name*/, const Math::Vec3& /*value*/) {}
        virtual void SetParameter(const std::string& /*name*/, const Math::Vec4& /*value*/) {}

    protected:
        bool m_enabled = true;
    };

    // Framebuffer manager for post-processing
    class FramebufferManager {
    public:
        FramebufferManager();
        ~FramebufferManager();

        bool Initialize(int width, int height);
        void Shutdown();
        void Resize(int width, int height);

        // Get framebuffer for intermediate processing
        uint32_t GetFramebuffer(int index);
        uint32_t GetColorTexture(int index);
        uint32_t GetDepthTexture(int index);

        // Swap framebuffers for ping-pong rendering
        void SwapFramebuffers();
        uint32_t GetCurrentFramebuffer() const;
        uint32_t GetCurrentColorTexture() const;
        uint32_t GetPreviousFramebuffer() const;
        uint32_t GetPreviousColorTexture() const;

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        void CreateFramebuffer(int index);
        void DeleteFramebuffer(int index);

        struct FramebufferData {
            uint32_t framebuffer = 0;
            uint32_t colorTexture = 0;
            uint32_t depthTexture = 0;
        };

        static const int MAX_FRAMEBUFFERS = 4;
        FramebufferData m_framebuffers[MAX_FRAMEBUFFERS];
        int m_width = 0;
        int m_height = 0;
        int m_currentFramebuffer = 0;
        bool m_initialized = false;
    };

    // Main post-processing pipeline
    class PostProcessingPipeline {
    public:
        PostProcessingPipeline();
        ~PostProcessingPipeline();

        // Lifecycle
        bool Initialize(int width, int height);
        void Shutdown();
        void Resize(int width, int height);

        // Effect management
        void AddEffect(std::shared_ptr<PostProcessEffect> effect);
        void RemoveEffect(const std::string& name);
        void SetEffectEnabled(const std::string& name, bool enabled);
        void SetEffectOrder(const std::vector<std::string>& order);
        std::shared_ptr<PostProcessEffect> GetEffect(const std::string& name);

        // Processing
        void Process(uint32_t inputTexture, uint32_t outputTexture);
        void ProcessToScreen(uint32_t inputTexture);

        // Built-in effects
        void EnableToneMapping(bool enable, ToneMappingType type = ToneMappingType::ACES);
        void EnableFXAA(bool enable, float quality = 0.75f);
        void EnableBloom(bool enable, float threshold = 1.0f, float intensity = 0.5f);

        // Configuration
        void SetGlobalExposure(float exposure) { m_globalExposure = exposure; }
        void SetGlobalGamma(float gamma) { m_globalGamma = gamma; }
        float GetGlobalExposure() const { return m_globalExposure; }
        float GetGlobalGamma() const { return m_globalGamma; }

        // Performance
        PostProcessStats GetStats() const { return m_stats; }
        void SetQualityLevel(QualityLevel level) { m_qualityLevel = level; }
        QualityLevel GetQualityLevel() const { return m_qualityLevel; }

        // Utility
        bool IsInitialized() const { return m_initialized; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        void CreateBuiltInEffects();
        void UpdateStats();
        void RenderFullscreenQuad();
        void SetupFullscreenQuad();
        void CleanupFullscreenQuad();

        std::vector<std::shared_ptr<PostProcessEffect>> m_effects;
        std::unordered_map<std::string, std::shared_ptr<PostProcessEffect>> m_effectMap;
        std::unique_ptr<FramebufferManager> m_framebuffers;

        int m_width = 0;
        int m_height = 0;
        float m_globalExposure = 1.0f;
        float m_globalGamma = 2.2f;

        PostProcessStats m_stats;
        QualityLevel m_qualityLevel = QualityLevel::High;

        // Fullscreen quad for rendering
        uint32_t m_quadVAO = 0;
        uint32_t m_quadVBO = 0;

        bool m_initialized = false;
    };
}