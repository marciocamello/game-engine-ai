#include "Graphics/FramebufferPool.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <algorithm>
#include <sstream>

namespace GameEngine {
    FramebufferPool& FramebufferPool::GetInstance() {
        static FramebufferPool instance;
        return instance;
    }

    bool FramebufferPool::Initialize(size_t maxPoolSize, size_t maxUnusedTime) {
        if (m_initialized.load()) {
            LOG_WARNING("FramebufferPool already initialized");
            return true;
        }

        LOG_INFO("Initializing FramebufferPool");

        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        m_maxPoolSize = maxPoolSize;
        m_maxUnusedTime = maxUnusedTime;
        m_lastCleanup = std::chrono::steady_clock::now();
        
        // Initialize statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = FramebufferPoolStats{};
        }

        m_initialized.store(true);
        LOG_INFO("FramebufferPool initialized with max pool size: " + std::to_string(maxPoolSize));
        return true;
    }

    void FramebufferPool::Shutdown() {
        if (!m_initialized.load()) {
            return;
        }

        LOG_INFO("Shutting down FramebufferPool");

        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        // Clean up all pooled framebuffers
        for (auto& pair : m_framebufferPool) {
            while (!pair.second.empty()) {
                auto framebuffer = pair.second.front();
                pair.second.pop();
                DestroyFramebuffer(framebuffer);
            }
        }
        m_framebufferPool.clear();
        m_activeFramebuffers.clear();

        // Reset statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = FramebufferPoolStats{};
        }

        m_initialized.store(false);
        LOG_INFO("FramebufferPool shutdown complete");
    }

    std::shared_ptr<FramebufferEntry> FramebufferPool::AcquireFramebuffer(const FramebufferSpec& spec) {
        if (!m_initialized.load()) {
            LOG_ERROR("FramebufferPool not initialized");
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(m_poolMutex);

        std::string specKey = GetSpecKey(spec);
        auto poolIt = m_framebufferPool.find(specKey);
        
        if (poolIt != m_framebufferPool.end() && !poolIt->second.empty()) {
            // Reuse framebuffer from pool
            auto framebuffer = poolIt->second.front();
            poolIt->second.pop();

            framebuffer->inUse = true;
            framebuffer->lastUsed = std::chrono::steady_clock::now();
            framebuffer->useCount++;

            // Add to active framebuffers
            m_activeFramebuffers.push_back(framebuffer);

            UpdateStats();
            return framebuffer;
        }

        // Create new framebuffer
        auto framebuffer = CreateFramebuffer(spec);
        if (!framebuffer) {
            LOG_ERROR("Failed to create framebuffer");
            return nullptr;
        }

        framebuffer->inUse = true;
        framebuffer->lastUsed = std::chrono::steady_clock::now();
        framebuffer->useCount = 1;

        // Add to active framebuffers
        m_activeFramebuffers.push_back(framebuffer);

        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalCreated++;
        }

        UpdateStats();
        return framebuffer;
    }

    void FramebufferPool::ReleaseFramebuffer(std::shared_ptr<FramebufferEntry> framebuffer) {
        if (!m_initialized.load() || !framebuffer) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_poolMutex);

        framebuffer->inUse = false;
        framebuffer->lastUsed = std::chrono::steady_clock::now();

        // Remove from active framebuffers
        m_activeFramebuffers.erase(
            std::remove_if(m_activeFramebuffers.begin(), m_activeFramebuffers.end(),
                [](const std::weak_ptr<FramebufferEntry>& weak) { return weak.expired(); }),
            m_activeFramebuffers.end()
        );

        // Check if we should pool this framebuffer
        size_t totalPooled = 0;
        for (const auto& pair : m_framebufferPool) {
            totalPooled += pair.second.size();
        }

        if (totalPooled < m_maxPoolSize) {
            // Add to pool for reuse
            std::string specKey = GetSpecKey(framebuffer->spec);
            m_framebufferPool[specKey].push(framebuffer);
        } else {
            // Pool is full, destroy framebuffer
            DestroyFramebuffer(framebuffer);
        }

        UpdateStats();
    }

    void FramebufferPool::CleanupUnusedFramebuffers() {
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

    void FramebufferPool::ForceCleanup() {
        if (!m_initialized.load()) {
            return;
        }

        PerformCleanup();
        m_lastCleanup = std::chrono::steady_clock::now();
    }

    void FramebufferPool::PerformCleanup() {
        std::lock_guard<std::mutex> lock(m_poolMutex);

        size_t cleanedCount = 0;

        // Clean up expired weak pointers in active framebuffers
        m_activeFramebuffers.erase(
            std::remove_if(m_activeFramebuffers.begin(), m_activeFramebuffers.end(),
                [](const std::weak_ptr<FramebufferEntry>& weak) { return weak.expired(); }),
            m_activeFramebuffers.end()
        );

        // Clean up unused framebuffers from pool
        for (auto& pair : m_framebufferPool) {
            auto& queue = pair.second;
            std::queue<std::shared_ptr<FramebufferEntry>> newQueue;

            while (!queue.empty()) {
                auto framebuffer = queue.front();
                queue.pop();

                if (!ShouldCleanupFramebuffer(*framebuffer)) {
                    newQueue.push(framebuffer);
                } else {
                    DestroyFramebuffer(framebuffer);
                    cleanedCount++;
                }
            }

            pair.second = std::move(newQueue);
        }

        // Remove empty entries
        for (auto it = m_framebufferPool.begin(); it != m_framebufferPool.end();) {
            if (it->second.empty()) {
                it = m_framebufferPool.erase(it);
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
            LOG_INFO("FramebufferPool cleaned up " + std::to_string(cleanedCount) + " unused framebuffers");
        }
    }

    bool FramebufferPool::ShouldCleanupFramebuffer(const FramebufferEntry& entry) const {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastUse = std::chrono::duration_cast<std::chrono::seconds>(now - entry.lastUsed).count();
        
        return timeSinceLastUse > static_cast<long long>(m_maxUnusedTime);
    }

    size_t FramebufferPool::GetPoolSize() const {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        size_t totalSize = 0;
        for (const auto& pair : m_framebufferPool) {
            totalSize += pair.second.size();
        }
        return totalSize;
    }

    size_t FramebufferPool::GetActiveFramebufferCount() const {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        return m_activeFramebuffers.size();
    }

    FramebufferPoolStats FramebufferPool::GetStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    void FramebufferPool::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = FramebufferPoolStats{};
    }

    void FramebufferPool::SetMaxPoolSize(size_t size) {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        m_maxPoolSize = size;
        
        // Force cleanup if current pool exceeds new limit
        if (GetPoolSize() > size) {
            PerformCleanup();
        }
    }

    void FramebufferPool::SetMaxUnusedTime(size_t seconds) {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        m_maxUnusedTime = seconds;
    }

    std::shared_ptr<FramebufferEntry> FramebufferPool::CreateFramebuffer(const FramebufferSpec& spec) {
        auto entry = std::make_shared<FramebufferEntry>();
        entry->spec = spec;
        entry->created = std::chrono::steady_clock::now();

        // Create framebuffer
        glGenFramebuffers(1, &entry->framebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, entry->framebufferId);

        // Create color attachments
        entry->colorTextures.resize(spec.colorAttachments);
        for (uint32_t i = 0; i < spec.colorAttachments; ++i) {
            glGenTextures(1, &entry->colorTextures[i]);
            glBindTexture(GL_TEXTURE_2D, entry->colorTextures[i]);
            
            if (spec.samples > 1) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.samples, spec.colorFormat, 
                                      spec.width, spec.height, GL_TRUE);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, spec.colorFormat, spec.width, spec.height, 
                           0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 
                                 entry->colorTextures[i], 0);
        }

        // Create depth attachment if needed
        if (spec.hasDepthAttachment) {
            glGenTextures(1, &entry->depthTexture);
            glBindTexture(GL_TEXTURE_2D, entry->depthTexture);
            
            if (spec.samples > 1) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.samples, spec.depthFormat, 
                                      spec.width, spec.height, GL_TRUE);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, spec.depthFormat, spec.width, spec.height, 
                           0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            }
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
                                 entry->depthTexture, 0);
        }

        // Create stencil attachment if needed
        if (spec.hasStencilAttachment) {
            glGenTextures(1, &entry->stencilTexture);
            glBindTexture(GL_TEXTURE_2D, entry->stencilTexture);
            
            if (spec.samples > 1) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.samples, GL_STENCIL_INDEX8, 
                                      spec.width, spec.height, GL_TRUE);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, spec.width, spec.height, 
                           0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 
                                 entry->stencilTexture, 0);
        }

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer is not complete");
            DestroyFramebuffer(entry);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return nullptr;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return entry;
    }

    void FramebufferPool::DestroyFramebuffer(std::shared_ptr<FramebufferEntry> framebuffer) {
        if (!framebuffer) {
            return;
        }

        // Delete color textures
        for (uint32_t textureId : framebuffer->colorTextures) {
            if (textureId != 0) {
                glDeleteTextures(1, &textureId);
            }
        }

        // Delete depth texture
        if (framebuffer->depthTexture != 0) {
            glDeleteTextures(1, &framebuffer->depthTexture);
        }

        // Delete stencil texture
        if (framebuffer->stencilTexture != 0) {
            glDeleteTextures(1, &framebuffer->stencilTexture);
        }

        // Delete framebuffer
        if (framebuffer->framebufferId != 0) {
            glDeleteFramebuffers(1, &framebuffer->framebufferId);
        }

        // Clear the entry
        framebuffer->framebufferId = 0;
        framebuffer->colorTextures.clear();
        framebuffer->depthTexture = 0;
        framebuffer->stencilTexture = 0;
    }

    std::string FramebufferPool::GetSpecKey(const FramebufferSpec& spec) const {
        std::stringstream ss;
        ss << spec.width << "x" << spec.height 
           << "_c" << spec.colorAttachments
           << "_d" << (spec.hasDepthAttachment ? 1 : 0)
           << "_s" << (spec.hasStencilAttachment ? 1 : 0)
           << "_ms" << spec.samples
           << "_cf" << spec.colorFormat
           << "_df" << spec.depthFormat;
        return ss.str();
    }

    void FramebufferPool::UpdateStats() {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        
        m_stats.activeFramebuffers = GetActiveFramebufferCount();
        m_stats.pooledFramebuffers = GetPoolSize();
        m_stats.totalFramebuffers = m_stats.activeFramebuffers + m_stats.pooledFramebuffers;
        
        // Estimate memory usage
        m_stats.memoryUsage = 0;
        for (const auto& pair : m_framebufferPool) {
            auto queue = pair.second; // Copy to avoid holding lock too long
            while (!queue.empty()) {
                auto framebuffer = queue.front();
                queue.pop();
                m_stats.memoryUsage += EstimateFramebufferMemory(framebuffer->spec);
            }
        }
        
        // Add active framebuffer memory
        for (const auto& weak : m_activeFramebuffers) {
            if (auto framebuffer = weak.lock()) {
                m_stats.memoryUsage += EstimateFramebufferMemory(framebuffer->spec);
            }
        }
        
        if (m_stats.memoryUsage > m_stats.peakMemoryUsage) {
            m_stats.peakMemoryUsage = m_stats.memoryUsage;
        }
        
        // Calculate average lifetime
        if (m_stats.totalDestroyed > 0) {
            m_stats.averageLifetime = static_cast<float>(m_stats.totalCreated) / static_cast<float>(m_stats.totalDestroyed);
        }
    }

    size_t FramebufferPool::EstimateFramebufferMemory(const FramebufferSpec& spec) const {
        size_t pixelCount = spec.width * spec.height * spec.samples;
        size_t colorMemory = pixelCount * spec.colorAttachments * 4; // Assume 4 bytes per pixel (RGBA8)
        size_t depthMemory = spec.hasDepthAttachment ? pixelCount * 3 : 0; // 3 bytes for depth24
        size_t stencilMemory = spec.hasStencilAttachment ? pixelCount * 1 : 0; // 1 byte for stencil
        
        return colorMemory + depthMemory + stencilMemory;
    }
}