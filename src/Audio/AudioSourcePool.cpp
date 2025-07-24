#include "Audio/AudioSourcePool.h"
#include "Audio/AudioEngine.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {

    AudioSourcePool::AudioSourcePool() {
        LOG_DEBUG("AudioSourcePool initialized (min: " + std::to_string(m_minPoolSize) + 
                 ", max: " + std::to_string(m_maxPoolSize) + ")");
        
        // Don't preallocate sources in constructor - wait for OpenAL to be ready
        // PreallocateSources will be called later when OpenAL is initialized
    }

    void AudioSourcePool::Initialize() {
        LOG_INFO("AudioSourcePool initializing with OpenAL ready");
        
        // Now that OpenAL is ready, preallocate minimum number of sources
        PreallocateSources(m_minPoolSize);
    }

    AudioSourcePool::~AudioSourcePool() {
        Clear();
        LOG_DEBUG("AudioSourcePool destroyed");
    }

    uint32_t AudioSourcePool::AcquireSource() {
        // Try to get an available source first
        if (!m_availableSources.empty()) {
            uint32_t sourceId = m_availableSources.front();
            m_availableSources.pop();
            m_activeSources.insert(sourceId);
            
            LOG_DEBUG("AudioSourcePool acquired existing source: " + std::to_string(sourceId));
            return sourceId;
        }
        
        // No available sources, try to expand pool
        if (CanExpandPool()) {
            uint32_t newSourceId = CreateNewSource();
            if (newSourceId != 0) {
                m_activeSources.insert(newSourceId);
                LOG_DEBUG("AudioSourcePool created and acquired new source: " + std::to_string(newSourceId));
                return newSourceId;
            }
        }
        
        LOG_WARNING("AudioSourcePool failed to acquire source (pool exhausted)");
        return 0; // Failed to acquire
    }

    void AudioSourcePool::ReleaseSource(uint32_t sourceId) {
        auto it = m_activeSources.find(sourceId);
        if (it == m_activeSources.end()) {
            LOG_WARNING("AudioSourcePool attempted to release non-active source: " + std::to_string(sourceId));
            return;
        }
        
        // Find the source and stop it
        auto sourceIt = std::find_if(m_allSources.begin(), m_allSources.end(),
            [sourceId](const std::unique_ptr<AudioSource>& source) {
                return source && source->GetId() == sourceId; // Assuming AudioSource has GetId method
            });
        
        if (sourceIt != m_allSources.end()) {
            // Stop the source and reset its state
            (*sourceIt)->Stop();
            // Reset any other properties to defaults if needed
        }
        
        // Move from active to available
        m_activeSources.erase(it);
        m_availableSources.push(sourceId);
        
        LOG_DEBUG("AudioSourcePool released source: " + std::to_string(sourceId) + 
                 " (available: " + std::to_string(m_availableSources.size()) + 
                 ", active: " + std::to_string(m_activeSources.size()) + ")");
    }

    void AudioSourcePool::SetPoolSize(size_t minSize, size_t maxSize) {
        if (minSize > maxSize) {
            LOG_WARNING("AudioSourcePool min size (" + std::to_string(minSize) + 
                       ") greater than max size (" + std::to_string(maxSize) + "), swapping");
            std::swap(minSize, maxSize);
        }
        
        m_minPoolSize = minSize;
        m_maxPoolSize = maxSize;
        
        LOG_INFO("AudioSourcePool size limits updated (min: " + std::to_string(m_minPoolSize) + 
                ", max: " + std::to_string(m_maxPoolSize) + ")");
        
        // Adjust current pool size if needed
        if (GetTotalSourceCount() < m_minPoolSize) {
            PreallocateSources(m_minPoolSize - GetTotalSourceCount());
        } else if (GetTotalSourceCount() > m_maxPoolSize) {
            ShrinkPool();
        }
    }

    void AudioSourcePool::PreallocateSources(size_t count) {
        LOG_INFO("AudioSourcePool preallocating " + std::to_string(count) + " sources");
        
        for (size_t i = 0; i < count; ++i) {
            if (GetTotalSourceCount() >= m_maxPoolSize) {
                LOG_DEBUG("AudioSourcePool reached max size during preallocation");
                break;
            }
            
            uint32_t sourceId = CreateNewSource();
            if (sourceId != 0) {
                m_availableSources.push(sourceId);
            } else {
                LOG_WARNING("AudioSourcePool failed to preallocate source " + std::to_string(i + 1));
                break;
            }
        }
        
        LOG_INFO("AudioSourcePool preallocation complete (total: " + std::to_string(GetTotalSourceCount()) + 
                ", available: " + std::to_string(GetAvailableSourceCount()) + ")");
    }

    void AudioSourcePool::Update() {
        // Check if we need to adjust pool size
        if (ShouldShrinkPool()) {
            ShrinkPool();
        } else if (GetAvailableSourceCount() < m_minPoolSize / 2 && CanExpandPool()) {
            ExpandPool();
        }
        
        // Clean up any sources that might be in invalid states
        CleanupIdleSources();
    }

    void AudioSourcePool::CleanupIdleSources() {
        // Check active sources to see if any have finished playing
        std::vector<uint32_t> finishedSources;
        
        for (uint32_t sourceId : m_activeSources) {
            auto sourceIt = std::find_if(m_allSources.begin(), m_allSources.end(),
                [sourceId](const std::unique_ptr<AudioSource>& source) {
                    return source && source->GetId() == sourceId;
                });
            
            if (sourceIt != m_allSources.end()) {
                // Check if source has finished playing
                if (!(*sourceIt)->IsPlaying() && !(*sourceIt)->IsPaused()) {
                    finishedSources.push_back(sourceId);
                }
            }
        }
        
        // Auto-release finished sources
        for (uint32_t sourceId : finishedSources) {
            LOG_DEBUG("AudioSourcePool auto-releasing finished source: " + std::to_string(sourceId));
            ReleaseSource(sourceId);
        }
    }

    float AudioSourcePool::GetPoolUtilization() const {
        size_t totalSources = GetTotalSourceCount();
        if (totalSources == 0) {
            return 0.0f;
        }
        
        return static_cast<float>(GetActiveSourceCount()) / totalSources;
    }

    bool AudioSourcePool::IsSourceActive(uint32_t sourceId) const {
        return m_activeSources.find(sourceId) != m_activeSources.end();
    }

    AudioSource* AudioSourcePool::GetSource(uint32_t sourceId) const {
        // Find the source in our collection
        for (const auto& source : m_allSources) {
            if (source->GetId() == sourceId) {
                return source.get();
            }
        }
        return nullptr;
    }

    void AudioSourcePool::Clear() {
        LOG_INFO("AudioSourcePool clearing all sources (total: " + std::to_string(GetTotalSourceCount()) + ")");
        
        m_activeSources.clear();
        
        // Clear available sources queue
        while (!m_availableSources.empty()) {
            m_availableSources.pop();
        }
        
        // Clear all sources (destructors will handle OpenAL cleanup)
        m_allSources.clear();
        
        m_nextSourceId = 1;
    }

    uint32_t AudioSourcePool::CreateNewSource() {
        try {
            uint32_t sourceId = m_nextSourceId++;
            auto source = std::make_unique<AudioSource>(sourceId);
            
            if (!source) {
                LOG_ERROR("AudioSourcePool failed to create AudioSource");
                return 0;
            }
            
            m_allSources.push_back(std::move(source));
            
            LOG_DEBUG("AudioSourcePool created new source: " + std::to_string(sourceId) + 
                     " (total: " + std::to_string(GetTotalSourceCount()) + ")");
            
            return sourceId;
            
        } catch (const std::exception& e) {
            LOG_ERROR("AudioSourcePool exception creating source: " + std::string(e.what()));
            return 0;
        }
    }

    void AudioSourcePool::ExpandPool() {
        size_t currentSize = GetTotalSourceCount();
        size_t targetSize = std::min(currentSize + (m_minPoolSize / 2), m_maxPoolSize);
        size_t toCreate = targetSize - currentSize;
        
        if (toCreate > 0) {
            LOG_DEBUG("AudioSourcePool expanding by " + std::to_string(toCreate) + " sources");
            PreallocateSources(toCreate);
        }
    }

    void AudioSourcePool::ShrinkPool() {
        // Only shrink available sources, never active ones
        size_t availableCount = GetAvailableSourceCount();
        size_t targetAvailable = m_minPoolSize;
        
        if (availableCount > targetAvailable) {
            size_t toRemove = availableCount - targetAvailable;
            
            LOG_DEBUG("AudioSourcePool shrinking by " + std::to_string(toRemove) + " sources");
            
            // Remove sources from the back of available queue
            std::vector<uint32_t> toKeep;
            
            // Keep only the target number of sources
            for (size_t i = 0; i < targetAvailable && !m_availableSources.empty(); ++i) {
                toKeep.push_back(m_availableSources.front());
                m_availableSources.pop();
            }
            
            // Remove the rest from m_allSources
            while (!m_availableSources.empty()) {
                uint32_t sourceId = m_availableSources.front();
                m_availableSources.pop();
                
                // Find and remove from m_allSources
                auto it = std::find_if(m_allSources.begin(), m_allSources.end(),
                    [sourceId](const std::unique_ptr<AudioSource>& source) {
                        return source && source->GetId() == sourceId;
                    });
                
                if (it != m_allSources.end()) {
                    m_allSources.erase(it);
                }
            }
            
            // Restore the sources we want to keep
            for (uint32_t sourceId : toKeep) {
                m_availableSources.push(sourceId);
            }
            
            LOG_DEBUG("AudioSourcePool shrink complete (total: " + std::to_string(GetTotalSourceCount()) + 
                     ", available: " + std::to_string(GetAvailableSourceCount()) + ")");
        }
    }

    bool AudioSourcePool::CanExpandPool() const {
        return GetTotalSourceCount() < m_maxPoolSize;
    }

    bool AudioSourcePool::ShouldShrinkPool() const {
        // Shrink if we have too many available sources and total exceeds minimum
        return GetAvailableSourceCount() > m_minPoolSize * 2 && 
               GetTotalSourceCount() > m_minPoolSize;
    }

} // namespace GameEngine