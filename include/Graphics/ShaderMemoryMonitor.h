#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

namespace GameEngine {
    class Shader;

    struct ShaderMemoryInfo {
        std::string name;
        size_t programSize = 0;
        size_t uniformBufferSize = 0;
        size_t textureMemory = 0;
        size_t totalMemory = 0;
        std::chrono::steady_clock::time_point lastAccessed;
        size_t accessCount = 0;
        bool isActive = false;
    };

    struct ShaderMemoryStats {
        size_t totalShaderMemory = 0;
        size_t totalUniformMemory = 0;
        size_t totalTextureMemory = 0;
        size_t peakMemoryUsage = 0;
        size_t activeShaders = 0;
        size_t inactiveShaders = 0;
        float memoryFragmentation = 0.0f;
        std::vector<std::string> topMemoryConsumers;
    };

    using MemoryThresholdCallback = std::function<void(size_t currentMemory, size_t threshold)>;
    using ShaderUnloadCallback = std::function<void(const std::string& shaderName)>;

    class ShaderMemoryMonitor {
    public:
        static ShaderMemoryMonitor& GetInstance();

        bool Initialize();
        void Shutdown();
        void Update();

        // Memory tracking
        void RegisterShader(const std::string& name, std::shared_ptr<Shader> shader);
        void UnregisterShader(const std::string& name);
        void UpdateShaderAccess(const std::string& name);
        void UpdateShaderMemoryUsage(const std::string& name, size_t programSize, size_t uniformSize, size_t textureSize);

        // Memory optimization
        void OptimizeMemoryUsage();
        std::vector<std::string> GetUnusedShaders(size_t maxUnusedTime = 300) const;
        std::vector<std::string> GetLowPriorityShaders() const;
        void SuggestShadersForUnload(size_t targetMemoryReduction, std::vector<std::string>& suggestions) const;

        // Statistics and monitoring
        ShaderMemoryStats GetMemoryStats() const;
        ShaderMemoryInfo GetShaderMemoryInfo(const std::string& name) const;
        size_t GetTotalMemoryUsage() const;
        size_t GetShaderMemoryUsage(const std::string& name) const;

        // Thresholds and callbacks
        void SetMemoryThreshold(size_t bytes);
        void SetMemoryThresholdCallback(MemoryThresholdCallback callback);
        void SetShaderUnloadCallback(ShaderUnloadCallback callback);
        void EnableAutoOptimization(bool enable) { m_autoOptimizationEnabled = enable; }
        bool IsAutoOptimizationEnabled() const { return m_autoOptimizationEnabled; }

        // Configuration
        void SetOptimizationInterval(float seconds) { m_optimizationInterval = seconds; }
        void SetMemoryReportInterval(float seconds) { m_reportInterval = seconds; }
        void EnableDetailedTracking(bool enable) { m_detailedTrackingEnabled = enable; }

    private:
        ShaderMemoryMonitor() = default;
        ~ShaderMemoryMonitor() = default;
        ShaderMemoryMonitor(const ShaderMemoryMonitor&) = delete;
        ShaderMemoryMonitor& operator=(const ShaderMemoryMonitor&) = delete;

        void CheckMemoryThreshold();
        void GenerateMemoryReport();
        void CalculateMemoryFragmentation();
        size_t EstimateShaderMemoryUsage(std::shared_ptr<Shader> shader) const;

        mutable std::mutex m_monitorMutex;
        std::unordered_map<std::string, ShaderMemoryInfo> m_shaderMemoryInfo;
        std::unordered_map<std::string, std::weak_ptr<Shader>> m_trackedShaders;

        // Configuration
        size_t m_memoryThreshold = 256 * 1024 * 1024; // 256MB default
        bool m_autoOptimizationEnabled = true;
        bool m_detailedTrackingEnabled = true;
        float m_optimizationInterval = 60.0f; // seconds
        float m_reportInterval = 300.0f; // seconds

        // Callbacks
        MemoryThresholdCallback m_thresholdCallback;
        ShaderUnloadCallback m_unloadCallback;

        // Timing
        std::chrono::steady_clock::time_point m_lastOptimization;
        std::chrono::steady_clock::time_point m_lastReport;

        // Statistics
        mutable std::mutex m_statsMutex;
        ShaderMemoryStats m_stats;
        std::atomic<bool> m_initialized{false};
    };
}