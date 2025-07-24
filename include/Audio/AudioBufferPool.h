#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <chrono>

#ifdef GAMEENGINE_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace GameEngine {

    struct AudioClip;

    // Cached audio buffer with usage tracking
    struct CachedAudioBuffer {
        std::shared_ptr<AudioClip> clip;
        std::chrono::steady_clock::time_point lastUsed;
        int useCount = 0;
        
#ifdef GAMEENGINE_HAS_OPENAL
        ALuint bufferId = 0;
#endif
        
        CachedAudioBuffer() : lastUsed(std::chrono::steady_clock::now()) {}
    };

    // High-performance audio buffer pool for frequently used sounds
    class AudioBufferPool {
    public:
        AudioBufferPool();
        ~AudioBufferPool();

        // Buffer management
        std::shared_ptr<AudioClip> GetBuffer(const std::string& filepath);
        void PreloadBuffer(const std::string& filepath);
        void UnloadBuffer(const std::string& filepath);
        
        // Pool management
        void CleanupUnusedBuffers(int maxUnusedTime = 300); // 5 minutes default
        void SetMaxPoolSize(size_t maxSize) { m_maxPoolSize = maxSize; }
        void Clear();
        
        // Statistics
        size_t GetPoolSize() const { return m_bufferCache.size(); }
        size_t GetMemoryUsage() const;
        float GetHitRatio() const;
        void ResetStatistics();
        
        // Hot buffer management (frequently accessed sounds)
        void MarkAsHot(const std::string& filepath);
        void UnmarkAsHot(const std::string& filepath);
        bool IsHot(const std::string& filepath) const;

    private:
        std::unordered_map<std::string, std::unique_ptr<CachedAudioBuffer>> m_bufferCache;
        std::unordered_set<std::string> m_hotBuffers; // Never cleanup these
        
        size_t m_maxPoolSize = 100; // Maximum number of cached buffers
        
        // Statistics
        mutable int m_cacheHits = 0;
        mutable int m_cacheMisses = 0;
        
        // Internal methods
        void EvictLeastRecentlyUsed();
        bool ShouldEvict(const CachedAudioBuffer& buffer) const;
        std::shared_ptr<AudioClip> LoadAudioClip(const std::string& filepath);
    };

} // namespace GameEngine