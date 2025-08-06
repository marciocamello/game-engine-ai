#include "Graphics/ShaderResourcePool.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
    ShaderResourcePool& ShaderResourcePool::GetInstance() {
        static ShaderResourcePool instance;
        return instance;
    }

    bool ShaderResourcePool::Initialize(size_t maxPoolSize, size_t maxUnusedTime) {
        if (m_initialized.load()) {
            LOG_WARNING("ShaderResourcePool already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderResourcePool");

        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        m_maxPoolSize = maxPoolSize;
        m_maxUnusedTime = maxUnusedTime;
        m_lastCleanup = std::chrono::steady_clock::now();
        
        // Initialize statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = ShaderResourceStats{};
        }

        m_initialized.store(true);
        LOG_INFO("ShaderResourcePool initialized with max pool size: " + std::to_string(maxPoolSize));
        return true;
    }

    void ShaderResourcePool::Shutdown() {
        if (!m_initialized.load()) {
            return;
        }

        LOG_INFO("Shutting down ShaderResourcePool");

        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        // Clear all pooled shaders
        for (auto& pair : m_shaderPool) {
            while (!pair.second.empty()) {
                pair.second.pop();
            }
        }
        m_shaderPool.clear();
        m_activeShaders.clear();

        // Reset statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = ShaderResourceStats{};
        }

        m_initialized.store(false);
        LOG_INFO("ShaderResourcePool shutdown complete");
    }

    std::shared_ptr<Shader> ShaderResourcePool::AcquireShader(const std::string& key) {
        if (!m_initialized.load()) {
            LOG_ERROR("ShaderResourcePool not initialized");
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(m_poolMutex);

        auto poolIt = m_shaderPool.find(key);
        if (poolIt != m_shaderPool.end() && !poolIt->second.empty()) {
            // Reuse shader from pool
            auto entry = poolIt->second.front();
            poolIt->second.pop();

            entry.inUse = true;
            entry.lastUsed = std::chrono::steady_clock::now();
            entry.useCount++;

            // Add to active shaders
            m_activeShaders[key].push_back(entry.shader);

            UpdateStats();
            return entry.shader;
        }

        // No shader available in pool, caller needs to create new one
        return nullptr;
    }

    void ShaderResourcePool::ReleaseShader(const std::string& key, std::shared_ptr<Shader> shader) {
        if (!m_initialized.load() || !shader) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_poolMutex);

        // Remove from active shaders
        auto activeIt = m_activeShaders.find(key);
        if (activeIt != m_activeShaders.end()) {
            auto& activeList = activeIt->second;
            activeList.erase(
                std::remove_if(activeList.begin(), activeList.end(),
                    [](const std::weak_ptr<Shader>& weak) { return weak.expired(); }),
                activeList.end()
            );
        }

        // Check if we should pool this shader
        size_t totalPooled = 0;
        for (const auto& pair : m_shaderPool) {
            totalPooled += pair.second.size();
        }

        if (totalPooled < m_maxPoolSize) {
            // Add to pool for reuse
            ShaderPoolEntry entry;
            entry.shader = shader;
            entry.lastUsed = std::chrono::steady_clock::now();
            entry.inUse = false;

            m_shaderPool[key].push(entry);
        }

        UpdateStats();
    }

    void ShaderResourcePool::RegisterShader(const std::string& key, std::shared_ptr<Shader> shader) {
        if (!m_initialized.load() || !shader) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        // Add to active shaders tracking
        m_activeShaders[key].push_back(shader);

        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalCreated++;
        }

        UpdateStats();
    }

    void ShaderResourcePool::CleanupUnusedShaders() {
        if (!m_initialized.load()) {
            return;
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastCleanup).count() < m_cleanupInterval) {
            return;
        }

        PerformCleanup();
        m_lastCleanup = now;
    }

    void ShaderResourcePool::ForceCleanup() {
        if (!m_initialized.load()) {
            return;
        }

        PerformCleanup();
        m_lastCleanup = std::chrono::steady_clock::now();
    }

    void ShaderResourcePool::PerformCleanup() {
        std::lock_guard<std::mutex> lock(m_poolMutex);

        size_t cleanedCount = 0;
        auto now = std::chrono::steady_clock::now();

        // Clean up expired weak pointers in active shaders
        for (auto& pair : m_activeShaders) {
            auto& activeList = pair.second;
            activeList.erase(
                std::remove_if(activeList.begin(), activeList.end(),
                    [](const std::weak_ptr<Shader>& weak) { return weak.expired(); }),
                activeList.end()
            );
        }

        // Clean up unused shaders from pool
        for (auto& pair : m_shaderPool) {
            auto& queue = pair.second;
            std::queue<ShaderPoolEntry> newQueue;

            while (!queue.empty()) {
                auto entry = queue.front();
                queue.pop();

                if (!ShouldCleanupShader(entry)) {
                    newQueue.push(entry);
                } else {
                    cleanedCount++;
                }
            }

            pair.second = std::move(newQueue);
        }

        // Remove empty entries
        for (auto it = m_shaderPool.begin(); it != m_shaderPool.end();) {
            if (it->second.empty()) {
                it = m_shaderPool.erase(it);
            } else {
                ++it;
            }
        }

        for (auto it = m_activeShaders.begin(); it != m_activeShaders.end();) {
            if (it->second.empty()) {
                it = m_activeShaders.erase(it);
            } else {
                ++it;
            }
        }

        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalDestroyed += cleanedCount;
        }

        UpdateStats();

        if (cleanedCount > 0) {
            LOG_INFO("ShaderResourcePool cleaned up " + std::to_string(cleanedCount) + " unused shaders");
        }
    }

    bool ShaderResourcePool::ShouldCleanupShader(const ShaderPoolEntry& entry) const {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastUse = std::chrono::duration_cast<std::chrono::seconds>(now - entry.lastUsed).count();
        
        return timeSinceLastUse > static_cast<long long>(m_maxUnusedTime);
    }

    size_t ShaderResourcePool::GetPoolSize() const {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        size_t totalSize = 0;
        for (const auto& pair : m_shaderPool) {
            totalSize += pair.second.size();
        }
        return totalSize;
    }

    size_t ShaderResourcePool::GetActiveShaderCount() const {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        size_t totalActive = 0;
        for (const auto& pair : m_activeShaders) {
            totalActive += pair.second.size();
        }
        return totalActive;
    }

    ShaderResourceStats ShaderResourcePool::GetStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    void ShaderResourcePool::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = ShaderResourceStats{};
    }

    void ShaderResourcePool::SetMaxPoolSize(size_t size) {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        m_maxPoolSize = size;
        
        // Force cleanup if current pool exceeds new limit
        if (GetPoolSize() > size) {
            PerformCleanup();
        }
    }

    void ShaderResourcePool::SetMaxUnusedTime(size_t seconds) {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        m_maxUnusedTime = seconds;
    }

    void ShaderResourcePool::UpdateStats() {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        
        m_stats.activeShaders = GetActiveShaderCount();
        m_stats.pooledShaders = GetPoolSize();
        m_stats.totalAllocated = m_stats.activeShaders + m_stats.pooledShaders;
        
        // Estimate memory usage (rough calculation)
        m_stats.memoryUsage = m_stats.totalAllocated * 2048; // ~2KB per shader estimate
        
        if (m_stats.memoryUsage > m_stats.peakMemoryUsage) {
            m_stats.peakMemoryUsage = m_stats.memoryUsage;
        }
        
        // Calculate average lifetime
        if (m_stats.totalDestroyed > 0) {
            m_stats.averageLifetime = static_cast<float>(m_stats.totalCreated) / static_cast<float>(m_stats.totalDestroyed);
        }
    }
}