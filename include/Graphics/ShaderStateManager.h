#pragma once

#include "Core/Math.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>

namespace GameEngine {
    class Shader;
    class Texture;

    // Uniform value types for batching
    using UniformValue = std::variant<
        bool, int, float,
        Math::Vec2, Math::Vec3, Math::Vec4,
        Math::Mat3, Math::Mat4,
        std::vector<float>, std::vector<int>,
        std::vector<Math::Vec3>, std::vector<Math::Vec4>,
        std::vector<Math::Mat4>
    >;

    struct UniformUpdate {
        std::string name;
        UniformValue value;
        int location = -1; // Cached location
    };

    struct TextureBinding {
        std::string name;
        uint32_t textureId;
        uint32_t slot;
        uint32_t target = 0x0DE1; // GL_TEXTURE_2D default
        bool isDirty = true;
    };

    struct ShaderState {
        uint32_t programId = 0;
        std::unordered_map<std::string, UniformValue> uniforms;
        std::unordered_map<std::string, TextureBinding> textures;
        std::unordered_map<std::string, uint32_t> uniformBuffers;
        std::unordered_map<std::string, uint32_t> storageBuffers;
        bool isDirty = true;
    };

    class ShaderStateManager {
    public:
        // Singleton access
        static ShaderStateManager& GetInstance();

        // Lifecycle
        bool Initialize();
        void Shutdown();

        // State management
        void BeginFrame();
        void EndFrame();
        void FlushPendingUpdates();
        
        // Shader registration (alternative to shared_ptr approach)
        void RegisterShader(uint32_t programId, const std::string& name);
        void UnregisterShader(uint32_t programId);
        void SetActiveShaderByProgramId(uint32_t programId);

        // Shader state caching
        void SetActiveShader(std::shared_ptr<Shader> shader);
        std::shared_ptr<Shader> GetActiveShader() const { return m_activeShader; }
        bool IsShaderActive(uint32_t programId) const;

        // Batch uniform updates
        void QueueUniformUpdate(const std::string& name, const UniformValue& value);
        void QueueUniformUpdate(const std::string& name, bool value);
        void QueueUniformUpdate(const std::string& name, int value);
        void QueueUniformUpdate(const std::string& name, float value);
        void QueueUniformUpdate(const std::string& name, const Math::Vec2& value);
        void QueueUniformUpdate(const std::string& name, const Math::Vec3& value);
        void QueueUniformUpdate(const std::string& name, const Math::Vec4& value);
        void QueueUniformUpdate(const std::string& name, const Math::Mat3& value);
        void QueueUniformUpdate(const std::string& name, const Math::Mat4& value);
        void QueueUniformArrayUpdate(const std::string& name, const std::vector<Math::Mat4>& values);
        void QueueUniformArrayUpdate(const std::string& name, const std::vector<Math::Vec3>& values);
        void QueueUniformArrayUpdate(const std::string& name, const std::vector<Math::Vec4>& values);
        void QueueUniformArrayUpdate(const std::string& name, const std::vector<float>& values);
        void QueueUniformArrayUpdate(const std::string& name, const std::vector<int>& values);

        // Texture binding optimization
        void QueueTextureBinding(const std::string& name, uint32_t textureId, uint32_t target = 0x0DE1);
        void QueueTextureBinding(const std::string& name, const Texture& texture);
        uint32_t AllocateTextureSlot(const std::string& name);
        void ReleaseTextureSlot(const std::string& name);
        void ResetTextureSlots();
        uint32_t GetTextureSlot(const std::string& name) const;

        // Buffer binding optimization
        void QueueUniformBufferBinding(const std::string& name, uint32_t bufferId, uint32_t binding);
        void QueueStorageBufferBinding(const std::string& name, uint32_t bufferId, uint32_t binding);

        // State validation and debugging
        void ValidateState() const;
        void LogStateChanges(bool enable) { m_logStateChanges = enable; }
        size_t GetStateChangeCount() const { return m_stateChangeCount; }
        void ResetStateChangeCount() { m_stateChangeCount = 0; }

        // Performance statistics
        struct PerformanceStats {
            size_t totalStateChanges = 0;
            size_t avoidedStateChanges = 0;
            size_t batchedUniforms = 0;
            size_t textureSlotOptimizations = 0;
            float averageBatchSize = 0.0f;
        };
        
        PerformanceStats GetPerformanceStats() const { return m_performanceStats; }
        void ResetPerformanceStats();

    private:
        ShaderStateManager() = default;
        ~ShaderStateManager() = default;
        ShaderStateManager(const ShaderStateManager&) = delete;
        ShaderStateManager& operator=(const ShaderStateManager&) = delete;

        // Internal state management
        void ApplyShaderState();
        void ApplyUniformUpdates();
        void ApplyTextureBindings();
        void ApplyBufferBindings();
        
        bool IsUniformValueEqual(const UniformValue& a, const UniformValue& b) const;
        void ApplyUniformValue(const std::string& name, const UniformValue& value, int location);
        int GetUniformLocation(const std::string& name);
        
        // Texture slot management
        uint32_t FindAvailableTextureSlot() const;
        void UpdateTextureSlotUsage();

        // Member variables
        std::shared_ptr<Shader> m_activeShader;
        uint32_t m_activeProgramId = 0;
        std::unordered_map<uint32_t, std::string> m_registeredShaders; // programId -> name mapping
        ShaderState m_currentState;
        ShaderState m_pendingState;
        
        std::vector<UniformUpdate> m_pendingUniforms;
        std::vector<TextureBinding> m_pendingTextures;
        std::unordered_map<std::string, uint32_t> m_pendingUniformBuffers;
        std::unordered_map<std::string, uint32_t> m_pendingStorageBuffers;

        // Texture slot management
        std::unordered_map<std::string, uint32_t> m_textureSlots;
        std::vector<bool> m_textureSlotUsage; // Track which slots are in use
        uint32_t m_maxTextureSlots = 32; // Default, will be queried from OpenGL
        uint32_t m_nextAvailableSlot = 0;

        // State tracking
        bool m_initialized = false;
        bool m_logStateChanges = false;
        size_t m_stateChangeCount = 0;
        PerformanceStats m_performanceStats;

        // Cached uniform locations for active shader
        std::unordered_map<std::string, int> m_uniformLocationCache;
    };
}