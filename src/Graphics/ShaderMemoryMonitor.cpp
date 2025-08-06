#include "Graphics/ShaderMemoryMonitor.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <algorithm>

namespace GameEngine {
    ShaderMemoryMonitor& ShaderMemoryMonitor::GetInstance() {
        static ShaderMemoryMonitor instance;
        return instance;
    }

    bool ShaderMemoryMonitor::Initialize() {
        if (m_initialized.load()) {
            LOG_WARNING("ShaderMemoryMonitor already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderMemoryMonitor");

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        m_lastOptimization = std::chrono::steady_clock::now();
        m_lastReport = std::chrono::steady_clock::now();

        // Initialize statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = ShaderMemoryStats{};
        }

        m_initialized.store(true);
        LOG_INFO("ShaderMemoryMonitor initialized with memory threshold: " + 
                std::to_string(m_memoryThreshold / (1024 * 1024)) + "MB");
        return true;
    }

    void ShaderMemoryMonitor::Shutdown() {
        if (!m_initialized.load()) {
            return;
        }

        LOG_INFO("Shutting down ShaderMemoryMonitor");

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        m_shaderMemoryInfo.clear();
        m_trackedShaders.clear();
        m_thresholdCallback = nullptr;
        m_unloadCallback = nullptr;

        // Reset statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = ShaderMemoryStats{};
        }

        m_initialized.store(false);
        LOG_INFO("ShaderMemoryMonitor shutdown complete");
    }

    void ShaderMemoryMonitor::Update() {
        if (!m_initialized.load()) {
            return;
        }

        auto now = std::chrono::steady_clock::now();

        // Check if we should run optimization
        if (m_autoOptimizationEnabled) {
            auto timeSinceOptimization = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastOptimization).count();
            if (timeSinceOptimization >= m_optimizationInterval) {
                OptimizeMemoryUsage();
                m_lastOptimization = now;
            }
        }

        // Check if we should generate memory report
        auto timeSinceReport = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastReport).count();
        if (timeSinceReport >= m_reportInterval) {
            GenerateMemoryReport();
            m_lastReport = now;
        }

        // Check memory threshold
        CheckMemoryThreshold();
    }

    void ShaderMemoryMonitor::RegisterShader(const std::string& name, std::shared_ptr<Shader> shader) {
        if (!m_initialized.load() || !shader) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);

        ShaderMemoryInfo info;
        info.name = name;
        info.lastAccessed = std::chrono::steady_clock::now();
        info.accessCount = 1;
        info.isActive = true;

        if (m_detailedTrackingEnabled) {
            info.totalMemory = EstimateShaderMemoryUsage(shader);
            info.programSize = info.totalMemory * 0.6f; // Rough estimate
            info.uniformBufferSize = info.totalMemory * 0.3f;
            info.textureMemory = info.totalMemory * 0.1f;
        }

        m_shaderMemoryInfo[name] = info;
        m_trackedShaders[name] = shader;

        LOG_INFO("Registered shader for memory monitoring: " + name + 
                " (estimated " + std::to_string(info.totalMemory / 1024) + "KB)");
    }

    void ShaderMemoryMonitor::UnregisterShader(const std::string& name) {
        if (!m_initialized.load()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);

        auto it = m_shaderMemoryInfo.find(name);
        if (it != m_shaderMemoryInfo.end()) {
            LOG_INFO("Unregistered shader from memory monitoring: " + name);
            m_shaderMemoryInfo.erase(it);
        }

        m_trackedShaders.erase(name);
    }

    void ShaderMemoryMonitor::UpdateShaderAccess(const std::string& name) {
        if (!m_initialized.load()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);

        auto it = m_shaderMemoryInfo.find(name);
        if (it != m_shaderMemoryInfo.end()) {
            it->second.lastAccessed = std::chrono::steady_clock::now();
            it->second.accessCount++;
            it->second.isActive = true;
        }
    }

    void ShaderMemoryMonitor::UpdateShaderMemoryUsage(const std::string& name, size_t programSize, size_t uniformSize, size_t textureSize) {
        if (!m_initialized.load()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);

        auto it = m_shaderMemoryInfo.find(name);
        if (it != m_shaderMemoryInfo.end()) {
            it->second.programSize = programSize;
            it->second.uniformBufferSize = uniformSize;
            it->second.textureMemory = textureSize;
            it->second.totalMemory = programSize + uniformSize + textureSize;
        }
    }

    void ShaderMemoryMonitor::OptimizeMemoryUsage() {
        if (!m_initialized.load()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);

        size_t totalMemory = GetTotalMemoryUsage();
        
        // Clean up expired weak pointers
        for (auto it = m_trackedShaders.begin(); it != m_trackedShaders.end();) {
            if (it->second.expired()) {
                m_shaderMemoryInfo.erase(it->first);
                it = m_trackedShaders.erase(it);
            } else {
                ++it;
            }
        }

        // Mark inactive shaders
        auto now = std::chrono::steady_clock::now();
        for (auto& pair : m_shaderMemoryInfo) {
            auto timeSinceAccess = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second.lastAccessed).count();
            pair.second.isActive = (timeSinceAccess < 60); // Consider active if accessed within last minute
        }

        // Update statistics
        CalculateMemoryFragmentation();

        LOG_INFO("Memory optimization completed. Total shader memory: " + 
                std::to_string(totalMemory / 1024) + "KB");
    }

    std::vector<std::string> ShaderMemoryMonitor::GetUnusedShaders(size_t maxUnusedTime) const {
        if (!m_initialized.load()) {
            return {};
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        std::vector<std::string> unusedShaders;
        auto now = std::chrono::steady_clock::now();

        for (const auto& pair : m_shaderMemoryInfo) {
            auto timeSinceAccess = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second.lastAccessed).count();
            if (timeSinceAccess > static_cast<long long>(maxUnusedTime)) {
                unusedShaders.push_back(pair.first);
            }
        }

        return unusedShaders;
    }

    std::vector<std::string> ShaderMemoryMonitor::GetLowPriorityShaders() const {
        if (!m_initialized.load()) {
            return {};
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        std::vector<std::pair<std::string, size_t>> shaderPriorities;

        for (const auto& pair : m_shaderMemoryInfo) {
            // Calculate priority based on access count and recency
            auto now = std::chrono::steady_clock::now();
            auto timeSinceAccess = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second.lastAccessed).count();
            
            size_t priority = pair.second.accessCount;
            if (timeSinceAccess > 0) {
                priority = priority / (timeSinceAccess / 60 + 1); // Reduce priority based on time since access
            }
            
            shaderPriorities.emplace_back(pair.first, priority);
        }

        // Sort by priority (lowest first)
        std::sort(shaderPriorities.begin(), shaderPriorities.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        std::vector<std::string> lowPriorityShaders;
        for (const auto& pair : shaderPriorities) {
            lowPriorityShaders.push_back(pair.first);
        }

        return lowPriorityShaders;
    }

    void ShaderMemoryMonitor::SuggestShadersForUnload(size_t targetMemoryReduction, std::vector<std::string>& suggestions) const {
        if (!m_initialized.load()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        auto lowPriorityShaders = GetLowPriorityShaders();
        size_t memoryFreed = 0;

        for (const std::string& shaderName : lowPriorityShaders) {
            auto it = m_shaderMemoryInfo.find(shaderName);
            if (it != m_shaderMemoryInfo.end()) {
                suggestions.push_back(shaderName);
                memoryFreed += it->second.totalMemory;
                
                if (memoryFreed >= targetMemoryReduction) {
                    break;
                }
            }
        }
    }

    ShaderMemoryStats ShaderMemoryMonitor::GetMemoryStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    ShaderMemoryInfo ShaderMemoryMonitor::GetShaderMemoryInfo(const std::string& name) const {
        if (!m_initialized.load()) {
            return ShaderMemoryInfo{};
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        auto it = m_shaderMemoryInfo.find(name);
        if (it != m_shaderMemoryInfo.end()) {
            return it->second;
        }
        
        return ShaderMemoryInfo{};
    }

    size_t ShaderMemoryMonitor::GetTotalMemoryUsage() const {
        if (!m_initialized.load()) {
            return 0;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        size_t totalMemory = 0;
        for (const auto& pair : m_shaderMemoryInfo) {
            totalMemory += pair.second.totalMemory;
        }
        
        return totalMemory;
    }

    size_t ShaderMemoryMonitor::GetShaderMemoryUsage(const std::string& name) const {
        if (!m_initialized.load()) {
            return 0;
        }

        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        auto it = m_shaderMemoryInfo.find(name);
        if (it != m_shaderMemoryInfo.end()) {
            return it->second.totalMemory;
        }
        
        return 0;
    }

    void ShaderMemoryMonitor::SetMemoryThreshold(size_t bytes) {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        m_memoryThreshold = bytes;
        LOG_INFO("Memory threshold set to " + std::to_string(bytes / (1024 * 1024)) + "MB");
    }

    void ShaderMemoryMonitor::SetMemoryThresholdCallback(MemoryThresholdCallback callback) {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        m_thresholdCallback = callback;
    }

    void ShaderMemoryMonitor::SetShaderUnloadCallback(ShaderUnloadCallback callback) {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        m_unloadCallback = callback;
    }

    void ShaderMemoryMonitor::CheckMemoryThreshold() {
        size_t currentMemory = GetTotalMemoryUsage();
        
        if (currentMemory > m_memoryThreshold) {
            if (m_thresholdCallback) {
                m_thresholdCallback(currentMemory, m_memoryThreshold);
            }
            
            // Suggest shaders for unload if callback is available
            if (m_unloadCallback) {
                std::vector<std::string> suggestions;
                size_t targetReduction = currentMemory - (m_memoryThreshold * 0.8f); // Target 80% of threshold
                SuggestShadersForUnload(targetReduction, suggestions);
                
                for (const std::string& shaderName : suggestions) {
                    m_unloadCallback(shaderName);
                }
            }
        }
    }

    void ShaderMemoryMonitor::GenerateMemoryReport() {
        std::lock_guard<std::mutex> lock(m_monitorMutex);
        
        size_t totalMemory = GetTotalMemoryUsage();
        size_t activeShaders = 0;
        size_t inactiveShaders = 0;

        for (const auto& pair : m_shaderMemoryInfo) {
            if (pair.second.isActive) {
                activeShaders++;
            } else {
                inactiveShaders++;
            }
        }

        LOG_INFO("Shader Memory Report:");
        LOG_INFO("  Total Memory: " + std::to_string(totalMemory / 1024) + "KB");
        LOG_INFO("  Active Shaders: " + std::to_string(activeShaders));
        LOG_INFO("  Inactive Shaders: " + std::to_string(inactiveShaders));
        LOG_INFO("  Memory Threshold: " + std::to_string(m_memoryThreshold / (1024 * 1024)) + "MB");
    }

    void ShaderMemoryMonitor::CalculateMemoryFragmentation() {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        
        m_stats.totalShaderMemory = 0;
        m_stats.totalUniformMemory = 0;
        m_stats.totalTextureMemory = 0;
        m_stats.activeShaders = 0;
        m_stats.inactiveShaders = 0;
        m_stats.topMemoryConsumers.clear();

        std::vector<std::pair<std::string, size_t>> memoryConsumers;

        for (const auto& pair : m_shaderMemoryInfo) {
            const auto& info = pair.second;
            
            m_stats.totalShaderMemory += info.programSize;
            m_stats.totalUniformMemory += info.uniformBufferSize;
            m_stats.totalTextureMemory += info.textureMemory;
            
            if (info.isActive) {
                m_stats.activeShaders++;
            } else {
                m_stats.inactiveShaders++;
            }
            
            memoryConsumers.emplace_back(pair.first, info.totalMemory);
        }

        // Sort by memory usage and get top consumers
        std::sort(memoryConsumers.begin(), memoryConsumers.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        size_t topCount = std::min(static_cast<size_t>(5), memoryConsumers.size());
        for (size_t i = 0; i < topCount; ++i) {
            m_stats.topMemoryConsumers.push_back(memoryConsumers[i].first);
        }

        size_t totalMemory = m_stats.totalShaderMemory + m_stats.totalUniformMemory + m_stats.totalTextureMemory;
        if (totalMemory > m_stats.peakMemoryUsage) {
            m_stats.peakMemoryUsage = totalMemory;
        }

        // Simple fragmentation calculation (inactive memory / total memory)
        if (totalMemory > 0) {
            size_t inactiveMemory = 0;
            for (const auto& pair : m_shaderMemoryInfo) {
                if (!pair.second.isActive) {
                    inactiveMemory += pair.second.totalMemory;
                }
            }
            m_stats.memoryFragmentation = static_cast<float>(inactiveMemory) / static_cast<float>(totalMemory);
        }
    }

    size_t ShaderMemoryMonitor::EstimateShaderMemoryUsage(std::shared_ptr<Shader> shader) const {
        if (!shader || !shader->IsValid()) {
            return 0;
        }

        // Basic estimation based on shader program
        // This is a rough estimate - actual memory usage would require more detailed OpenGL queries
        size_t baseSize = 2048; // Base shader program size
        
        // Add estimated uniform buffer size
        size_t uniformSize = 1024; // Estimated uniform buffer size
        
        // Add estimated texture memory (this would typically be tracked separately)
        size_t textureSize = 512; // Estimated texture binding overhead
        
        return baseSize + uniformSize + textureSize;
    }
}