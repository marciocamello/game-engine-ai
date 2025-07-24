#include "Audio/AudioBufferPool.h"
#include "Audio/AudioEngine.h"
#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include <algorithm>
#include <unordered_set>

namespace GameEngine {

    AudioBufferPool::AudioBufferPool() {
        LOG_DEBUG("AudioBufferPool initialized with max size: " + std::to_string(m_maxPoolSize));
    }

    AudioBufferPool::~AudioBufferPool() {
        Clear();
        LOG_DEBUG("AudioBufferPool destroyed");
    }

    std::shared_ptr<AudioClip> AudioBufferPool::GetBuffer(const std::string& filepath) {
        // Check cache first
        auto it = m_bufferCache.find(filepath);
        if (it != m_bufferCache.end()) {
            // Update usage statistics
            it->second->lastUsed = std::chrono::steady_clock::now();
            it->second->useCount++;
            m_cacheHits++;
            
            LOG_DEBUG("AudioBufferPool cache hit for: " + filepath);
            return it->second->clip;
        }
        
        // Cache miss - load the audio clip
        m_cacheMisses++;
        LOG_DEBUG("AudioBufferPool cache miss for: " + filepath);
        
        // Check if we need to evict before loading
        if (m_bufferCache.size() >= m_maxPoolSize) {
            EvictLeastRecentlyUsed();
        }
        
        // Load the audio clip
        auto clip = LoadAudioClip(filepath);
        if (!clip) {
            LOG_WARNING("Failed to load audio clip for buffer pool: " + filepath);
            return nullptr;
        }
        
        // Cache the loaded clip
        auto cachedBuffer = std::make_unique<CachedAudioBuffer>();
        cachedBuffer->clip = clip;
        cachedBuffer->lastUsed = std::chrono::steady_clock::now();
        cachedBuffer->useCount = 1;
        
#ifdef GAMEENGINE_HAS_OPENAL
        cachedBuffer->bufferId = clip->bufferId;
#endif
        
        m_bufferCache[filepath] = std::move(cachedBuffer);
        
        LOG_INFO("AudioBufferPool cached new buffer: " + filepath + 
                " (pool size: " + std::to_string(m_bufferCache.size()) + ")");
        
        return clip;
    }

    void AudioBufferPool::PreloadBuffer(const std::string& filepath) {
        // Check if already cached
        if (m_bufferCache.find(filepath) != m_bufferCache.end()) {
            LOG_DEBUG("AudioBufferPool buffer already preloaded: " + filepath);
            return;
        }
        
        LOG_INFO("AudioBufferPool preloading buffer: " + filepath);
        
        // Load without affecting cache hit/miss statistics
        auto clip = LoadAudioClip(filepath);
        if (!clip) {
            LOG_WARNING("Failed to preload audio buffer: " + filepath);
            return;
        }
        
        // Check if we need to evict before caching
        if (m_bufferCache.size() >= m_maxPoolSize) {
            EvictLeastRecentlyUsed();
        }
        
        // Cache the preloaded clip
        auto cachedBuffer = std::make_unique<CachedAudioBuffer>();
        cachedBuffer->clip = clip;
        cachedBuffer->lastUsed = std::chrono::steady_clock::now();
        cachedBuffer->useCount = 0; // Preloaded, not yet used
        
#ifdef GAMEENGINE_HAS_OPENAL
        cachedBuffer->bufferId = clip->bufferId;
#endif
        
        m_bufferCache[filepath] = std::move(cachedBuffer);
        
        LOG_INFO("AudioBufferPool preloaded buffer: " + filepath);
    }

    void AudioBufferPool::UnloadBuffer(const std::string& filepath) {
        auto it = m_bufferCache.find(filepath);
        if (it != m_bufferCache.end()) {
            LOG_INFO("AudioBufferPool unloading buffer: " + filepath);
            
#ifdef GAMEENGINE_HAS_OPENAL
            // OpenAL buffer cleanup is handled by AudioClip destructor
#endif
            
            m_bufferCache.erase(it);
        }
    }

    void AudioBufferPool::CleanupUnusedBuffers(int maxUnusedTime) {
        auto now = std::chrono::steady_clock::now();
        auto maxAge = std::chrono::seconds(maxUnusedTime);
        
        std::vector<std::string> toRemove;
        
        for (const auto& pair : m_bufferCache) {
            const std::string& filepath = pair.first;
            const auto& buffer = pair.second;
            
            // Don't cleanup hot buffers
            if (IsHot(filepath)) {
                continue;
            }
            
            // Check if buffer is old enough to be cleaned up
            auto age = now - buffer->lastUsed;
            if (age > maxAge && ShouldEvict(*buffer)) {
                toRemove.push_back(filepath);
            }
        }
        
        // Remove old buffers
        for (const std::string& filepath : toRemove) {
            LOG_DEBUG("AudioBufferPool cleaning up unused buffer: " + filepath);
            UnloadBuffer(filepath);
        }
        
        if (!toRemove.empty()) {
            LOG_INFO("AudioBufferPool cleaned up " + std::to_string(toRemove.size()) + 
                    " unused buffers (pool size: " + std::to_string(m_bufferCache.size()) + ")");
        }
    }

    void AudioBufferPool::Clear() {
        LOG_INFO("AudioBufferPool clearing all buffers (count: " + std::to_string(m_bufferCache.size()) + ")");
        
#ifdef GAMEENGINE_HAS_OPENAL
        // OpenAL buffer cleanup is handled by AudioClip destructors
#endif
        
        m_bufferCache.clear();
        m_hotBuffers.clear();
        ResetStatistics();
    }

    size_t AudioBufferPool::GetMemoryUsage() const {
        size_t totalMemory = 0;
        
        for (const auto& pair : m_bufferCache) {
            const auto& buffer = pair.second;
            if (buffer->clip) {
                // Estimate memory usage based on audio data size
                totalMemory += buffer->clip->path.size(); // String storage
                // Add estimated audio data size (this would need to be tracked in AudioClip)
                totalMemory += sizeof(AudioClip); // Base object size
            }
        }
        
        return totalMemory;
    }

    float AudioBufferPool::GetHitRatio() const {
        int totalRequests = m_cacheHits + m_cacheMisses;
        if (totalRequests == 0) {
            return 0.0f;
        }
        
        return static_cast<float>(m_cacheHits) / totalRequests;
    }

    void AudioBufferPool::ResetStatistics() {
        m_cacheHits = 0;
        m_cacheMisses = 0;
    }

    void AudioBufferPool::MarkAsHot(const std::string& filepath) {
        m_hotBuffers.insert(filepath);
        LOG_DEBUG("AudioBufferPool marked as hot: " + filepath);
        
        // Preload if not already cached
        if (m_bufferCache.find(filepath) == m_bufferCache.end()) {
            PreloadBuffer(filepath);
        }
    }

    void AudioBufferPool::UnmarkAsHot(const std::string& filepath) {
        m_hotBuffers.erase(filepath);
        LOG_DEBUG("AudioBufferPool unmarked as hot: " + filepath);
    }

    bool AudioBufferPool::IsHot(const std::string& filepath) const {
        return m_hotBuffers.find(filepath) != m_hotBuffers.end();
    }

    void AudioBufferPool::EvictLeastRecentlyUsed() {
        if (m_bufferCache.empty()) {
            return;
        }
        
        // Find the least recently used buffer that's not hot
        std::string lruFilepath;
        std::chrono::steady_clock::time_point oldestTime = std::chrono::steady_clock::now();
        
        for (const auto& pair : m_bufferCache) {
            const std::string& filepath = pair.first;
            const auto& buffer = pair.second;
            
            // Skip hot buffers
            if (IsHot(filepath)) {
                continue;
            }
            
            if (buffer->lastUsed < oldestTime && ShouldEvict(*buffer)) {
                oldestTime = buffer->lastUsed;
                lruFilepath = filepath;
            }
        }
        
        // Evict the LRU buffer
        if (!lruFilepath.empty()) {
            LOG_DEBUG("AudioBufferPool evicting LRU buffer: " + lruFilepath);
            UnloadBuffer(lruFilepath);
        } else {
            LOG_WARNING("AudioBufferPool could not find buffer to evict (all may be hot)");
        }
    }

    bool AudioBufferPool::ShouldEvict(const CachedAudioBuffer& buffer) const {
        // Don't evict recently used buffers (within last 30 seconds)
        auto now = std::chrono::steady_clock::now();
        auto recentThreshold = std::chrono::seconds(30);
        
        return (now - buffer.lastUsed) > recentThreshold;
    }

    std::shared_ptr<AudioClip> AudioBufferPool::LoadAudioClip(const std::string& filepath) {
        try {
            AudioLoader loader;
            AudioData audioData = loader.LoadAudio(filepath);
            
            if (!audioData.isValid) {
                LOG_ERROR("AudioBufferPool failed to load audio data: " + filepath);
                return nullptr;
            }
            
            auto clip = std::make_shared<AudioClip>();
            clip->path = filepath;
            clip->duration = audioData.duration;
            clip->sampleRate = audioData.sampleRate;
            clip->channels = audioData.channels;
            
            // Determine format
            if (AudioLoader::IsWAVFile(filepath)) {
                clip->format = AudioFormat::WAV;
            } else if (AudioLoader::IsOGGFile(filepath)) {
                clip->format = AudioFormat::OGG;
            }
            
#ifdef GAMEENGINE_HAS_OPENAL
            // Create OpenAL buffer
            clip->bufferId = loader.CreateOpenALBuffer(audioData);
            if (clip->bufferId == 0) {
                LOG_ERROR("AudioBufferPool failed to create OpenAL buffer: " + filepath);
                return nullptr;
            }
#endif
            
            return clip;
            
        } catch (const std::exception& e) {
            LOG_ERROR("AudioBufferPool exception loading clip '" + filepath + "': " + e.what());
            return nullptr;
        }
    }

} // namespace GameEngine