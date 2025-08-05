#include "Graphics/PostProcessingPipeline.h"
#include "Graphics/PostProcessEffects.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <chrono>
#include <algorithm>

namespace GameEngine {

    // FramebufferManager Implementation
    FramebufferManager::FramebufferManager() {
        for (int i = 0; i < MAX_FRAMEBUFFERS; ++i) {
            m_framebuffers[i] = {};
        }
    }

    FramebufferManager::~FramebufferManager() {
        Shutdown();
    }

    bool FramebufferManager::Initialize(int width, int height) {
        if (m_initialized) {
            Shutdown();
        }

        m_width = width;
        m_height = height;

        // Create initial framebuffers (we'll create more as needed)
        for (int i = 0; i < 2; ++i) {
            CreateFramebuffer(i);
        }

        m_initialized = true;
        LOG_INFO("FramebufferManager initialized with size " + std::to_string(width) + "x" + std::to_string(height));
        return true;
    }

    void FramebufferManager::Shutdown() {
        if (!m_initialized) return;

        for (int i = 0; i < MAX_FRAMEBUFFERS; ++i) {
            DeleteFramebuffer(i);
        }

        m_initialized = false;
        LOG_INFO("FramebufferManager shutdown");
    }

    void FramebufferManager::Resize(int width, int height) {
        if (!m_initialized) return;

        m_width = width;
        m_height = height;

        // Recreate all existing framebuffers
        for (int i = 0; i < MAX_FRAMEBUFFERS; ++i) {
            if (m_framebuffers[i].framebuffer != 0) {
                DeleteFramebuffer(i);
                CreateFramebuffer(i);
            }
        }

        LOG_INFO("FramebufferManager resized to " + std::to_string(width) + "x" + std::to_string(height));
    }

    void FramebufferManager::CreateFramebuffer(int index) {
        if (index < 0 || index >= MAX_FRAMEBUFFERS) return;

        auto& fb = m_framebuffers[index];

        // Generate framebuffer
        glGenFramebuffers(1, &fb.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, fb.framebuffer);

        // Create color texture
        glGenTextures(1, &fb.colorTexture);
        glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorTexture, 0);

        // Create depth texture
        glGenTextures(1, &fb.depthTexture);
        glBindTexture(GL_TEXTURE_2D, fb.depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthTexture, 0);

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer " + std::to_string(index) + " is not complete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void FramebufferManager::DeleteFramebuffer(int index) {
        if (index < 0 || index >= MAX_FRAMEBUFFERS) return;

        auto& fb = m_framebuffers[index];
        
        if (fb.framebuffer != 0) {
            glDeleteFramebuffers(1, &fb.framebuffer);
            fb.framebuffer = 0;
        }
        
        if (fb.colorTexture != 0) {
            glDeleteTextures(1, &fb.colorTexture);
            fb.colorTexture = 0;
        }
        
        if (fb.depthTexture != 0) {
            glDeleteTextures(1, &fb.depthTexture);
            fb.depthTexture = 0;
        }
    }

    uint32_t FramebufferManager::GetFramebuffer(int index) {
        if (index < 0 || index >= MAX_FRAMEBUFFERS) return 0;
        
        // Create framebuffer if it doesn't exist
        if (m_framebuffers[index].framebuffer == 0) {
            CreateFramebuffer(index);
        }
        
        return m_framebuffers[index].framebuffer;
    }

    uint32_t FramebufferManager::GetColorTexture(int index) {
        if (index < 0 || index >= MAX_FRAMEBUFFERS) return 0;
        
        // Create framebuffer if it doesn't exist
        if (m_framebuffers[index].framebuffer == 0) {
            CreateFramebuffer(index);
        }
        
        return m_framebuffers[index].colorTexture;
    }

    uint32_t FramebufferManager::GetDepthTexture(int index) {
        if (index < 0 || index >= MAX_FRAMEBUFFERS) return 0;
        
        // Create framebuffer if it doesn't exist
        if (m_framebuffers[index].framebuffer == 0) {
            CreateFramebuffer(index);
        }
        
        return m_framebuffers[index].depthTexture;
    }

    void FramebufferManager::SwapFramebuffers() {
        m_currentFramebuffer = (m_currentFramebuffer + 1) % 2;
    }

    uint32_t FramebufferManager::GetCurrentFramebuffer() const {
        return m_framebuffers[m_currentFramebuffer].framebuffer;
    }

    uint32_t FramebufferManager::GetCurrentColorTexture() const {
        return m_framebuffers[m_currentFramebuffer].colorTexture;
    }

    uint32_t FramebufferManager::GetPreviousFramebuffer() const {
        int prevIndex = (m_currentFramebuffer + 1) % 2;
        return m_framebuffers[prevIndex].framebuffer;
    }

    uint32_t FramebufferManager::GetPreviousColorTexture() const {
        int prevIndex = (m_currentFramebuffer + 1) % 2;
        return m_framebuffers[prevIndex].colorTexture;
    }

    // PostProcessingPipeline Implementation
    PostProcessingPipeline::PostProcessingPipeline() 
        : m_framebuffers(std::make_unique<FramebufferManager>()) {
    }

    PostProcessingPipeline::~PostProcessingPipeline() {
        Shutdown();
    }

    bool PostProcessingPipeline::Initialize(int width, int height) {
        if (m_initialized) {
            Shutdown();
        }

        m_width = width;
        m_height = height;

        // Initialize framebuffer manager
        if (!m_framebuffers->Initialize(width, height)) {
            LOG_ERROR("Failed to initialize framebuffer manager");
            return false;
        }

        // Setup fullscreen quad
        SetupFullscreenQuad();

        // Create built-in effects
        CreateBuiltInEffects();

        m_initialized = true;
        LOG_INFO("PostProcessingPipeline initialized with size " + std::to_string(width) + "x" + std::to_string(height));
        return true;
    }

    void PostProcessingPipeline::Shutdown() {
        if (!m_initialized) return;

        // Clear effects
        m_effects.clear();
        m_effectMap.clear();

        // Shutdown framebuffer manager
        if (m_framebuffers) {
            m_framebuffers->Shutdown();
        }

        // Cleanup fullscreen quad
        CleanupFullscreenQuad();

        m_initialized = false;
        LOG_INFO("PostProcessingPipeline shutdown");
    }

    void PostProcessingPipeline::Resize(int width, int height) {
        if (!m_initialized) return;

        m_width = width;
        m_height = height;

        // Resize framebuffer manager
        m_framebuffers->Resize(width, height);

        // Resize all effects
        for (auto& effect : m_effects) {
            if (effect) {
                effect->Resize(width, height);
            }
        }

        LOG_INFO("PostProcessingPipeline resized to " + std::to_string(width) + "x" + std::to_string(height));
    }

    void PostProcessingPipeline::AddEffect(std::shared_ptr<PostProcessEffect> effect) {
        if (!effect) return;

        const std::string& name = effect->GetName();
        
        // Remove existing effect with same name
        RemoveEffect(name);

        // Initialize the effect
        if (m_initialized) {
            effect->Initialize(m_width, m_height);
        }

        // Add to collections
        m_effects.push_back(effect);
        m_effectMap[name] = effect;

        LOG_INFO("Added post-processing effect: " + name);
    }

    void PostProcessingPipeline::RemoveEffect(const std::string& name) {
        auto it = m_effectMap.find(name);
        if (it != m_effectMap.end()) {
            // Shutdown the effect
            it->second->Shutdown();

            // Remove from vector
            m_effects.erase(
                std::remove(m_effects.begin(), m_effects.end(), it->second),
                m_effects.end()
            );

            // Remove from map
            m_effectMap.erase(it);

            LOG_INFO("Removed post-processing effect: " + name);
        }
    }

    void PostProcessingPipeline::SetEffectEnabled(const std::string& name, bool enabled) {
        auto it = m_effectMap.find(name);
        if (it != m_effectMap.end()) {
            it->second->SetEnabled(enabled);
            LOG_INFO("Set effect " + name + " enabled: " + (enabled ? "true" : "false"));
        }
    }

    void PostProcessingPipeline::SetEffectOrder(const std::vector<std::string>& order) {
        std::vector<std::shared_ptr<PostProcessEffect>> newOrder;
        
        // Add effects in specified order
        for (const std::string& name : order) {
            auto it = m_effectMap.find(name);
            if (it != m_effectMap.end()) {
                newOrder.push_back(it->second);
            }
        }

        // Add any remaining effects not in the order list
        for (auto& effect : m_effects) {
            if (std::find(newOrder.begin(), newOrder.end(), effect) == newOrder.end()) {
                newOrder.push_back(effect);
            }
        }

        m_effects = std::move(newOrder);
        LOG_INFO("Updated effect order");
    }

    std::shared_ptr<PostProcessEffect> PostProcessingPipeline::GetEffect(const std::string& name) {
        auto it = m_effectMap.find(name);
        return (it != m_effectMap.end()) ? it->second : nullptr;
    }

    void PostProcessingPipeline::Process(uint32_t inputTexture, uint32_t outputTexture) {
        if (!m_initialized || m_effects.empty()) {
            // No effects, just copy input to output
            // This would require a simple copy shader, for now we'll skip
            return;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        uint32_t currentInput = inputTexture;
        bool useFramebuffer = true;

        // Process each enabled effect
        for (size_t i = 0; i < m_effects.size(); ++i) {
            auto& effect = m_effects[i];
            if (!effect || !effect->IsEnabled()) continue;

            uint32_t currentOutput;
            
            // Determine output target
            if (i == m_effects.size() - 1) {
                // Last effect outputs to final target
                currentOutput = outputTexture;
                useFramebuffer = false;
            } else {
                // Intermediate effect outputs to framebuffer
                currentOutput = m_framebuffers->GetCurrentColorTexture();
                
                // Bind framebuffer for rendering
                glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffers->GetCurrentFramebuffer());
                glViewport(0, 0, m_width, m_height);
            }

            // Process the effect
            effect->Process(currentInput, currentOutput);

            // Prepare for next iteration
            if (useFramebuffer) {
                currentInput = currentOutput;
                m_framebuffers->SwapFramebuffers();
            }
        }

        // Restore default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Update statistics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        m_stats.totalProcessingTime = duration.count() / 1000.0f; // Convert to milliseconds
        UpdateStats();
    }

    void PostProcessingPipeline::ProcessToScreen(uint32_t inputTexture) {
        // Process to screen (framebuffer 0)
        Process(inputTexture, 0);
    }

    void PostProcessingPipeline::EnableToneMapping(bool enable, ToneMappingType type) {
        auto effect = std::dynamic_pointer_cast<ToneMappingEffect>(GetEffect("ToneMapping"));
        if (effect) {
            effect->SetEnabled(enable);
            effect->SetToneMappingType(type);
            LOG_INFO("ToneMapping enabled: " + std::string(enable ? "true" : "false") + ", type: " + std::to_string(static_cast<int>(type)));
        } else {
            LOG_ERROR("ToneMapping effect not found");
        }
    }

    void PostProcessingPipeline::EnableFXAA(bool enable, float quality) {
        auto effect = std::dynamic_pointer_cast<FXAAEffect>(GetEffect("FXAA"));
        if (effect) {
            effect->SetEnabled(enable);
            effect->SetQuality(quality);
            LOG_INFO("FXAA enabled: " + std::string(enable ? "true" : "false") + ", quality: " + std::to_string(quality));
        } else {
            LOG_ERROR("FXAA effect not found");
        }
    }

    void PostProcessingPipeline::EnableBloom(bool enable, float threshold, float intensity) {
        auto effect = std::dynamic_pointer_cast<BloomEffect>(GetEffect("Bloom"));
        if (effect) {
            effect->SetEnabled(enable);
            effect->SetThreshold(threshold);
            effect->SetIntensity(intensity);
            LOG_INFO("Bloom enabled: " + std::string(enable ? "true" : "false") + ", threshold: " + std::to_string(threshold) + ", intensity: " + std::to_string(intensity));
        } else {
            LOG_ERROR("Bloom effect not found");
        }
    }

    void PostProcessingPipeline::CreateBuiltInEffects() {
        // Create tone mapping effect
        auto toneMappingEffect = std::make_shared<ToneMappingEffect>();
        if (m_initialized) {
            toneMappingEffect->Initialize(m_width, m_height);
        }
        m_effectMap["ToneMapping"] = toneMappingEffect;

        // Create FXAA effect
        auto fxaaEffect = std::make_shared<FXAAEffect>();
        if (m_initialized) {
            fxaaEffect->Initialize(m_width, m_height);
        }
        m_effectMap["FXAA"] = fxaaEffect;

        // Create bloom effect
        auto bloomEffect = std::make_shared<BloomEffect>();
        if (m_initialized) {
            bloomEffect->Initialize(m_width, m_height);
        }
        m_effectMap["Bloom"] = bloomEffect;

        LOG_INFO("Built-in post-processing effects created");
    }

    void PostProcessingPipeline::UpdateStats() {
        m_stats.activeEffects = 0;
        m_stats.memoryUsage = 0;

        for (const auto& effect : m_effects) {
            if (effect && effect->IsEnabled()) {
                m_stats.activeEffects++;
            }
        }

        // Estimate memory usage (simplified)
        m_stats.framebuffersUsed = 2; // We use 2 framebuffers for ping-pong
        m_stats.memoryUsage = m_width * m_height * 4 * sizeof(float) * m_stats.framebuffersUsed; // RGBA16F
    }

    void PostProcessingPipeline::SetupFullscreenQuad() {
        // Fullscreen quad vertices (position and texture coordinates)
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        
        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        
        // Texture coordinate attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        glBindVertexArray(0);
    }

    void PostProcessingPipeline::CleanupFullscreenQuad() {
        if (m_quadVAO != 0) {
            glDeleteVertexArrays(1, &m_quadVAO);
            m_quadVAO = 0;
        }
        
        if (m_quadVBO != 0) {
            glDeleteBuffers(1, &m_quadVBO);
            m_quadVBO = 0;
        }
    }

    void PostProcessingPipeline::RenderFullscreenQuad() {
        if (m_quadVAO != 0) {
            glBindVertexArray(m_quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }

} // namespace GameEngine