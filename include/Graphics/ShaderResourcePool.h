#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>

namespace GameEngine {
    class Shader;

    struct ShaderResourceStats {
        size_t totalAllocated = 0;
        size_t activeShaders = 0;
        size_t pooledShaders = 0;
        size_t memoryUsage = 0;
        size_t peakMemoryUsage = 0;
        float averageLifetime = 0.0f;
        size_t totalCreated = 0;
        size_t totalDestroyed = 0;
    };

    struct ShaderPoolEntry {
        std::shared_ptr<Shader> shader;
        std::chrono::steady_clock::time_point lastUsed;
        std::chrono::steady_clock::time_point created;
        size_t useCount = 0;
        bool inUse = false;
    };

    class ShaderResourcePool {
    public:
        static ShaderResourcePool& GetInstance();

        bool Initialize(size_t maxPoolSize = 256, size_t maxUnusedTime = 300);
        void Shutdown();

        // Shader resource management
        std::shared_ptr<Shader> AcquireShader(const std::string& key);
        void ReleaseShader(const std::string& key, std::shared_ptr<Shader> shader);
        void RegisterShader(const std::string& key, std::shared_ptr<Shader> shader);

        // Memory management
        void CleanupUnusedShaders();
        void ForceCleanup();
        size_t GetPoolSize() const;
        size_t GetActiveShaderCount() const;

        // Statistics and monitoring
        ShaderResourceStats GetStats() const;
        void ResetStats();
        void SetMaxPoolSize(size_t size);
        void SetMaxUnusedTime(size_t seconds);

        // Configuration
        void EnableAutoCleanup(bool enable) { m_autoCleanupEnabled = enable; }
        bool IsAutoCleanupEnabled() const { return m_autoCleanupEnabled; }
        void SetCleanupInterval(float seconds) { m_cleanupInterval = seconds; }

    private:
        ShaderResourcePool() = default;
        ~ShaderResourcePool() = default;
        ShaderResourcePool(const ShaderResourcePool&) = delete;
        ShaderResourcePool& operator=(const ShaderResourcePool&) = delete;

        void UpdateStats();
        void PerformCleanup();
        bool ShouldCleanupShader(const ShaderPoolEntry& entry) const;

        mutable std::mutex m_poolMutex;
        std::unordered_map<std::string, std::queue<ShaderPoolEntry>> m_shaderPool;
        std::unordered_map<std::string, std::vector<std::weak_ptr<Shader>>> m_activeShaders;

        size_t m_maxPoolSize = 256;
        size_t m_maxUnusedTime = 300; // seconds
        bool m_autoCleanupEnabled = true;
        float m_cleanupInterval = 30.0f; // seconds
        std::chrono::steady_clock::time_point m_lastCleanup;

        // Statistics
        mutable std::mutex m_statsMutex;
        ShaderResourceStats m_stats;
        std::atomic<bool> m_initialized{false};
    };
}