#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>

namespace GameEngine {
    struct FramebufferSpec {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t colorAttachments = 1;
        bool hasDepthAttachment = true;
        bool hasStencilAttachment = false;
        uint32_t samples = 1; // For MSAA
        uint32_t colorFormat = 0x8058; // GL_RGBA8
        uint32_t depthFormat = 0x81A5; // GL_DEPTH_COMPONENT24
        
        bool operator==(const FramebufferSpec& other) const {
            return width == other.width && height == other.height &&
                   colorAttachments == other.colorAttachments &&
                   hasDepthAttachment == other.hasDepthAttachment &&
                   hasStencilAttachment == other.hasStencilAttachment &&
                   samples == other.samples &&
                   colorFormat == other.colorFormat &&
                   depthFormat == other.depthFormat;
        }
    };

    struct FramebufferEntry {
        uint32_t framebufferId = 0;
        std::vector<uint32_t> colorTextures;
        uint32_t depthTexture = 0;
        uint32_t stencilTexture = 0;
        FramebufferSpec spec;
        std::chrono::steady_clock::time_point lastUsed;
        std::chrono::steady_clock::time_point created;
        size_t useCount = 0;
        bool inUse = false;
    };

    struct FramebufferPoolStats {
        size_t totalFramebuffers = 0;
        size_t activeFramebuffers = 0;
        size_t pooledFramebuffers = 0;
        size_t memoryUsage = 0;
        size_t peakMemoryUsage = 0;
        size_t totalCreated = 0;
        size_t totalDestroyed = 0;
        float averageLifetime = 0.0f;
    };

    class FramebufferPool {
    public:
        static FramebufferPool& GetInstance();

        bool Initialize(size_t maxPoolSize = 64, size_t maxUnusedTime = 180);
        void Shutdown();

        // Framebuffer management
        std::shared_ptr<FramebufferEntry> AcquireFramebuffer(const FramebufferSpec& spec);
        void ReleaseFramebuffer(std::shared_ptr<FramebufferEntry> framebuffer);
        
        // Pool management
        void CleanupUnusedFramebuffers();
        void ForceCleanup();
        size_t GetPoolSize() const;
        size_t GetActiveFramebufferCount() const;

        // Statistics and monitoring
        FramebufferPoolStats GetStats() const;
        void ResetStats();
        void SetMaxPoolSize(size_t size);
        void SetMaxUnusedTime(size_t seconds);

        // Configuration
        void EnableAutoCleanup(bool enable) { m_autoCleanupEnabled = enable; }
        bool IsAutoCleanupEnabled() const { return m_autoCleanupEnabled; }
        void SetCleanupInterval(float seconds) { m_cleanupInterval = seconds; }

    private:
        FramebufferPool() = default;
        ~FramebufferPool() = default;
        FramebufferPool(const FramebufferPool&) = delete;
        FramebufferPool& operator=(const FramebufferPool&) = delete;

        std::shared_ptr<FramebufferEntry> CreateFramebuffer(const FramebufferSpec& spec);
        void DestroyFramebuffer(std::shared_ptr<FramebufferEntry> framebuffer);
        std::string GetSpecKey(const FramebufferSpec& spec) const;
        void UpdateStats();
        void PerformCleanup();
        bool ShouldCleanupFramebuffer(const FramebufferEntry& entry) const;
        size_t EstimateFramebufferMemory(const FramebufferSpec& spec) const;

        mutable std::mutex m_poolMutex;
        std::unordered_map<std::string, std::queue<std::shared_ptr<FramebufferEntry>>> m_framebufferPool;
        std::vector<std::weak_ptr<FramebufferEntry>> m_activeFramebuffers;

        size_t m_maxPoolSize = 64;
        size_t m_maxUnusedTime = 180; // seconds
        bool m_autoCleanupEnabled = true;
        float m_cleanupInterval = 30.0f; // seconds
        std::chrono::steady_clock::time_point m_lastCleanup;

        // Statistics
        mutable std::mutex m_statsMutex;
        FramebufferPoolStats m_stats;
        std::atomic<bool> m_initialized{false};
    };
}