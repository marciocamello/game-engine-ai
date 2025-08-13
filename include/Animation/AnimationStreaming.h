#pragma once

#include "Animation/Animation.h"
#include "Core/Math.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace GameEngine {
namespace Animation {

    /**
     * Animation streaming priority levels
     */
    enum class StreamingPriority {
        Critical = 0,   // Must be loaded immediately
        High = 1,       // Load as soon as possible
        Normal = 2,     // Load when resources available
        Low = 3,        // Load when idle
        Background = 4  // Load in background when system is idle
    };

    /**
     * Animation streaming state
     */
    enum class StreamingState {
        Unloaded,       // Animation data not in memory
        Loading,        // Currently being loaded
        Loaded,         // Fully loaded and ready to use
        Unloading,      // Currently being unloaded
        Error           // Error occurred during loading/unloading
    };

    /**
     * Animation streaming request
     */
    struct StreamingRequest {
        std::string animationId;
        StreamingPriority priority;
        std::function<void(std::shared_ptr<Animation>)> onLoaded;
        std::function<void(const std::string&)> onError;
        
        StreamingRequest() = default;
        StreamingRequest(const std::string& id, StreamingPriority prio) 
            : animationId(id), priority(prio) {}
    };

    /**
     * Animation memory statistics
     */
    struct AnimationMemoryStats {
        size_t totalMemoryUsed = 0;        // Total memory used by animations
        size_t loadedAnimations = 0;       // Number of loaded animations
        size_t unloadedAnimations = 0;     // Number of unloaded animations
        size_t streamingAnimations = 0;    // Number of animations being streamed
        size_t memoryLimit = 0;            // Memory limit for animations
        float memoryUsagePercent = 0.0f;   // Percentage of memory limit used
        
        void Calculate() {
            if (memoryLimit > 0) {
                memoryUsagePercent = static_cast<float>(totalMemoryUsed) / static_cast<float>(memoryLimit);
            }
        }
    };

    /**
     * Animation streaming configuration
     */
    struct StreamingConfig {
        size_t memoryLimitBytes = 256 * 1024 * 1024;  // 256MB default limit
        size_t maxConcurrentLoads = 4;                // Max concurrent loading operations
        float unloadThreshold = 0.8f;                 // Unload when memory usage exceeds this
        float reloadThreshold = 0.6f;                 // Reload when memory usage drops below this
        bool enableBackgroundLoading = true;          // Enable background loading
        bool enablePredictiveLoading = true;          // Enable predictive loading
        float unusedAnimationTimeout = 30.0f;         // Seconds before unused animations are unloaded
    };

    /**
     * Animation data reference for streaming
     */
    class AnimationReference {
    public:
        AnimationReference(const std::string& id, const std::string& filePath);
        ~AnimationReference() = default;

        // Properties
        const std::string& GetId() const { return m_id; }
        const std::string& GetFilePath() const { return m_filePath; }
        StreamingState GetState() const { return m_state; }
        StreamingPriority GetPriority() const { return m_priority; }
        
        void SetPriority(StreamingPriority priority) { m_priority = priority; }
        void SetState(StreamingState state) { m_state = state; }

        // Animation access
        std::shared_ptr<Animation> GetAnimation() const { return m_animation; }
        void SetAnimation(std::shared_ptr<Animation> animation) { m_animation = animation; }
        
        // Usage tracking
        void MarkUsed() { m_lastUsedTime = GetCurrentTime(); }
        float GetTimeSinceLastUsed() const { return GetCurrentTime() - m_lastUsedTime; }
        bool IsUnused(float timeoutSeconds) const { return GetTimeSinceLastUsed() > timeoutSeconds; }
        
        // Memory usage
        size_t GetMemoryUsage() const;
        bool IsLoaded() const { return m_animation != nullptr && m_state == StreamingState::Loaded; }

    private:
        std::string m_id;
        std::string m_filePath;
        std::shared_ptr<Animation> m_animation;
        StreamingState m_state = StreamingState::Unloaded;
        StreamingPriority m_priority = StreamingPriority::Normal;
        float m_lastUsedTime = 0.0f;
        
        float GetCurrentTime() const;
    };

    /**
     * Animation streaming manager
     */
    class AnimationStreamingManager {
    public:
        AnimationStreamingManager();
        ~AnimationStreamingManager();

        // Lifecycle
        bool Initialize(const StreamingConfig& config = StreamingConfig{});
        void Shutdown();
        void Update(float deltaTime);

        // Animation registration
        void RegisterAnimation(const std::string& id, const std::string& filePath);
        void UnregisterAnimation(const std::string& id);
        bool IsAnimationRegistered(const std::string& id) const;

        // Animation loading/unloading
        void RequestAnimation(const std::string& id, StreamingPriority priority = StreamingPriority::Normal);
        void RequestAnimation(const StreamingRequest& request);
        void UnloadAnimation(const std::string& id);
        void UnloadUnusedAnimations();
        void UnloadAllAnimations();

        // Animation access
        std::shared_ptr<Animation> GetAnimation(const std::string& id);
        bool IsAnimationLoaded(const std::string& id) const;
        StreamingState GetAnimationState(const std::string& id) const;

        // Memory management
        void SetMemoryLimit(size_t limitBytes);
        size_t GetMemoryLimit() const { return m_config.memoryLimitBytes; }
        AnimationMemoryStats GetMemoryStats() const;
        void ForceGarbageCollection();

        // Configuration
        void SetConfig(const StreamingConfig& config) { m_config = config; }
        const StreamingConfig& GetConfig() const { return m_config; }

        // Callbacks
        void SetLoadCallback(std::function<void(const std::string&, std::shared_ptr<Animation>)> callback) {
            m_onAnimationLoaded = callback;
        }
        void SetUnloadCallback(std::function<void(const std::string&)> callback) {
            m_onAnimationUnloaded = callback;
        }

    private:
        StreamingConfig m_config;
        std::unordered_map<std::string, std::unique_ptr<AnimationReference>> m_animations;
        
        // Threading
        std::thread m_streamingThread;
        std::atomic<bool> m_running{false};
        std::mutex m_requestMutex;
        std::condition_variable m_requestCondition;
        std::queue<StreamingRequest> m_loadRequests;
        std::queue<std::string> m_unloadRequests;
        
        // Callbacks
        std::function<void(const std::string&, std::shared_ptr<Animation>)> m_onAnimationLoaded;
        std::function<void(const std::string&)> m_onAnimationUnloaded;
        
        // Statistics
        mutable std::mutex m_statsMutex;
        AnimationMemoryStats m_stats;

        // Internal methods
        void StreamingThreadMain();
        void ProcessLoadRequests();
        void ProcessUnloadRequests();
        void UpdateMemoryStats();
        void CheckMemoryPressure();
        
        std::shared_ptr<Animation> LoadAnimationFromFile(const std::string& filePath);
        void UnloadAnimationData(const std::string& id);
        
        std::vector<std::string> GetUnusedAnimations() const;
        std::vector<std::string> GetAnimationsByPriority(StreamingPriority priority) const;
    };

    /**
     * Animation data sharing for memory optimization
     */
    class AnimationDataCache {
    public:
        AnimationDataCache() = default;
        ~AnimationDataCache() = default;

        // Cache management
        void CacheAnimation(const std::string& id, std::shared_ptr<Animation> animation);
        std::shared_ptr<Animation> GetCachedAnimation(const std::string& id);
        void RemoveFromCache(const std::string& id);
        void ClearCache();

        // Shared data optimization
        void OptimizeSharedData();
        size_t GetCacheMemoryUsage() const;
        size_t GetCachedAnimationCount() const { return m_cache.size(); }

        // Cache statistics
        struct CacheStats {
            size_t hits = 0;
            size_t misses = 0;
            size_t evictions = 0;
            float hitRate = 0.0f;
            
            void Calculate() {
                size_t total = hits + misses;
                hitRate = total > 0 ? static_cast<float>(hits) / static_cast<float>(total) : 0.0f;
            }
        };
        
        CacheStats GetCacheStats() const { return m_stats; }
        void ResetStats() { m_stats = CacheStats{}; }

    private:
        std::unordered_map<std::string, std::shared_ptr<Animation>> m_cache;
        mutable std::mutex m_cacheMutex;
        CacheStats m_stats;
        
        // LRU tracking
        std::vector<std::string> m_accessOrder;
        void UpdateAccessOrder(const std::string& id);
        void EvictLeastRecentlyUsed();
    };

    /**
     * Animation preloader for predictive loading
     */
    class AnimationPreloader {
    public:
        AnimationPreloader(AnimationStreamingManager* streamingManager);
        ~AnimationPreloader() = default;

        // Predictive loading
        void PredictAnimationUsage(const std::string& currentAnimation, 
                                  const std::vector<std::string>& likelyNext);
        void PreloadAnimationsForState(const std::string& stateName,
                                      const std::vector<std::string>& animations);
        
        // Usage pattern learning
        void RecordAnimationTransition(const std::string& from, const std::string& to);
        std::vector<std::string> GetPredictedAnimations(const std::string& current) const;
        
        // Configuration
        void SetPredictionThreshold(float threshold) { m_predictionThreshold = threshold; }
        void SetMaxPredictions(size_t maxPredictions) { m_maxPredictions = maxPredictions; }

    private:
        AnimationStreamingManager* m_streamingManager;
        
        // Transition tracking
        std::unordered_map<std::string, std::unordered_map<std::string, int>> m_transitionCounts;
        std::unordered_map<std::string, int> m_animationUsageCounts;
        
        float m_predictionThreshold = 0.3f;  // Minimum probability for prediction
        size_t m_maxPredictions = 5;         // Maximum number of predictions
        
        float CalculateTransitionProbability(const std::string& from, const std::string& to) const;
    };

} // namespace Animation
} // namespace GameEngine