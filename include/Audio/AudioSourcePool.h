#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <unordered_set>

#ifdef GAMEENGINE_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace GameEngine {

    class AudioSource;

    // Pool of reusable audio sources to reduce allocation overhead
    class AudioSourcePool {
    public:
        AudioSourcePool();
        ~AudioSourcePool();

        // Initialization (call after OpenAL is ready)
        void Initialize();

        // Source management
        uint32_t AcquireSource();
        void ReleaseSource(uint32_t sourceId);
        
        // Pool configuration
        void SetPoolSize(size_t minSize, size_t maxSize);
        void PreallocateSources(size_t count);
        
        // Pool maintenance
        void Update(); // Call regularly to manage pool
        void CleanupIdleSources();
        
        // Statistics
        size_t GetActiveSourceCount() const { return m_activeSources.size(); }
        size_t GetAvailableSourceCount() const { return m_availableSources.size(); }
        size_t GetTotalSourceCount() const { return m_allSources.size(); }
        float GetPoolUtilization() const;
        
        // Pool state
        bool IsSourceActive(uint32_t sourceId) const;
        AudioSource* GetSource(uint32_t sourceId) const;
        void Clear();

    private:
        std::vector<std::unique_ptr<AudioSource>> m_allSources;
        std::queue<uint32_t> m_availableSources; // Ready to use
        std::unordered_set<uint32_t> m_activeSources; // Currently in use
        
        size_t m_minPoolSize = 8;  // Minimum sources to keep available
        size_t m_maxPoolSize = 32; // Maximum total sources
        uint32_t m_nextSourceId = 1;
        
        // Internal methods
        uint32_t CreateNewSource();
        void ExpandPool();
        void ShrinkPool();
        bool CanExpandPool() const;
        bool ShouldShrinkPool() const;
    };

} // namespace GameEngine