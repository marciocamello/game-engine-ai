#include "Animation/AnimationStreaming.h"
#include "Core/Logger.h"
#include <algorithm>
#include <chrono>
#include <fstream>

namespace GameEngine {
namespace Animation {

    // AnimationReference implementation
    AnimationReference::AnimationReference(const std::string& id, const std::string& filePath)
        : m_id(id), m_filePath(filePath) {
        m_lastUsedTime = GetCurrentTime();
    }

    size_t AnimationReference::GetMemoryUsage() const {
        if (!m_animation) {
            return sizeof(AnimationReference);
        }
        return sizeof(AnimationReference) + m_animation->GetMemoryUsage();
    }

    float AnimationReference::GetCurrentTime() const {
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration<float>(duration).count();
    }

    // AnimationStreamingManager implementation
    AnimationStreamingManager::AnimationStreamingManager() = default;

    AnimationStreamingManager::~AnimationStreamingManager() {
        Shutdown();
    }

    bool AnimationStreamingManager::Initialize(const StreamingConfig& config) {
        LOG_INFO("Initializing Animation Streaming Manager");
        
        m_config = config;
        m_running = true;
        
        // Start streaming thread
        m_streamingThread = std::thread(&AnimationStreamingManager::StreamingThreadMain, this);
        
        LOG_INFO("Animation Streaming Manager initialized with " + 
                std::to_string(m_config.memoryLimitBytes / (1024 * 1024)) + "MB memory limit");
        
        return true;
    }

    void AnimationStreamingManager::Shutdown() {
        if (m_running) {
            LOG_INFO("Shutting down Animation Streaming Manager");
            
            m_running = false;
            m_requestCondition.notify_all();
            
            if (m_streamingThread.joinable()) {
                m_streamingThread.join();
            }
            
            UnloadAllAnimations();
            m_animations.clear();
        }
    }

    void AnimationStreamingManager::Update(float deltaTime) {
        UpdateMemoryStats();
        CheckMemoryPressure();
        
        // Process completed loading operations
        ProcessLoadRequests();
        ProcessUnloadRequests();
    }

    void AnimationStreamingManager::RegisterAnimation(const std::string& id, const std::string& filePath) {
        auto animRef = std::make_unique<AnimationReference>(id, filePath);
        m_animations[id] = std::move(animRef);
        
        LOG_INFO("Registered animation '" + id + "' from file: " + filePath);
    }

    void AnimationStreamingManager::UnregisterAnimation(const std::string& id) {
        auto it = m_animations.find(id);
        if (it != m_animations.end()) {
            UnloadAnimation(id);
            m_animations.erase(it);
            LOG_INFO("Unregistered animation '" + id + "'");
        }
    }

    bool AnimationStreamingManager::IsAnimationRegistered(const std::string& id) const {
        return m_animations.find(id) != m_animations.end();
    }

    void AnimationStreamingManager::RequestAnimation(const std::string& id, StreamingPriority priority) {
        StreamingRequest request(id, priority);
        RequestAnimation(request);
    }

    void AnimationStreamingManager::RequestAnimation(const StreamingRequest& request) {
        auto it = m_animations.find(request.animationId);
        if (it == m_animations.end()) {
            LOG_WARNING("Animation '" + request.animationId + "' not registered");
            if (request.onError) {
                request.onError("Animation not registered");
            }
            return;
        }
        
        auto& animRef = it->second;
        animRef->SetPriority(request.priority);
        animRef->MarkUsed();
        
        // If already loaded, call callback immediately
        if (animRef->IsLoaded()) {
            if (request.onLoaded) {
                request.onLoaded(animRef->GetAnimation());
            }
            return;
        }
        
        // Add to loading queue
        {
            std::lock_guard<std::mutex> lock(m_requestMutex);
            m_loadRequests.push(request);
        }
        m_requestCondition.notify_one();
        
        animRef->SetState(StreamingState::Loading);
        LOG_INFO("Queued animation '" + request.animationId + "' for loading");
    }

    void AnimationStreamingManager::UnloadAnimation(const std::string& id) {
        auto it = m_animations.find(id);
        if (it != m_animations.end() && it->second->IsLoaded()) {
            {
                std::lock_guard<std::mutex> lock(m_requestMutex);
                m_unloadRequests.push(id);
            }
            m_requestCondition.notify_one();
            
            it->second->SetState(StreamingState::Unloading);
            LOG_INFO("Queued animation '" + id + "' for unloading");
        }
    }

    void AnimationStreamingManager::UnloadUnusedAnimations() {
        auto unusedAnimations = GetUnusedAnimations();
        for (const auto& id : unusedAnimations) {
            UnloadAnimation(id);
        }
        
        if (!unusedAnimations.empty()) {
            LOG_INFO("Queued " + std::to_string(unusedAnimations.size()) + " unused animations for unloading");
        }
    }

    void AnimationStreamingManager::UnloadAllAnimations() {
        for (const auto& [id, animRef] : m_animations) {
            if (animRef->IsLoaded()) {
                UnloadAnimationData(id);
            }
        }
        LOG_INFO("Unloaded all animations");
    }

    std::shared_ptr<SkeletalAnimation> AnimationStreamingManager::GetAnimation(const std::string& id) {
        auto it = m_animations.find(id);
        if (it != m_animations.end()) {
            it->second->MarkUsed();
            return it->second->GetAnimation();
        }
        return nullptr;
    }

    bool AnimationStreamingManager::IsAnimationLoaded(const std::string& id) const {
        auto it = m_animations.find(id);
        return it != m_animations.end() && it->second->IsLoaded();
    }

    StreamingState AnimationStreamingManager::GetAnimationState(const std::string& id) const {
        auto it = m_animations.find(id);
        return it != m_animations.end() ? it->second->GetState() : StreamingState::Error;
    }

    void AnimationStreamingManager::SetMemoryLimit(size_t limitBytes) {
        m_config.memoryLimitBytes = limitBytes;
        LOG_INFO("Set animation memory limit to " + std::to_string(limitBytes / (1024 * 1024)) + "MB");
        CheckMemoryPressure();
    }

    AnimationMemoryStats AnimationStreamingManager::GetMemoryStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    void AnimationStreamingManager::ForceGarbageCollection() {
        LOG_INFO("Forcing animation garbage collection");
        UnloadUnusedAnimations();
        
        // If still over memory limit, unload low priority animations
        UpdateMemoryStats();
        if (m_stats.memoryUsagePercent > m_config.unloadThreshold) {
            auto lowPriorityAnimations = GetAnimationsByPriority(StreamingPriority::Low);
            for (const auto& id : lowPriorityAnimations) {
                UnloadAnimation(id);
                UpdateMemoryStats();
                if (m_stats.memoryUsagePercent <= m_config.reloadThreshold) {
                    break;
                }
            }
        }
    }

    void AnimationStreamingManager::StreamingThreadMain() {
        LOG_INFO("Animation streaming thread started");
        
        while (m_running) {
            std::unique_lock<std::mutex> lock(m_requestMutex);
            m_requestCondition.wait(lock, [this] {
                return !m_running || !m_loadRequests.empty() || !m_unloadRequests.empty();
            });
            
            if (!m_running) {
                break;
            }
            
            // Process load requests
            while (!m_loadRequests.empty()) {
                auto request = m_loadRequests.front();
                m_loadRequests.pop();
                lock.unlock();
                
                // Load animation
                auto it = m_animations.find(request.animationId);
                if (it != m_animations.end()) {
                    auto& animRef = it->second;
                    
                    try {
                        auto animation = LoadAnimationFromFile(animRef->GetFilePath());
                        if (animation) {
                            animRef->SetAnimation(animation);
                            animRef->SetState(StreamingState::Loaded);
                            
                            if (request.onLoaded) {
                                request.onLoaded(animation);
                            }
                            
                            if (m_onAnimationLoaded) {
                                m_onAnimationLoaded(request.animationId, animation);
                            }
                            
                            LOG_INFO("Loaded animation '" + request.animationId + "'");
                        } else {
                            animRef->SetState(StreamingState::Error);
                            if (request.onError) {
                                request.onError("Failed to load animation file");
                            }
                            LOG_ERROR("Failed to load animation '" + request.animationId + "'");
                        }
                    } catch (const std::exception& e) {
                        animRef->SetState(StreamingState::Error);
                        if (request.onError) {
                            request.onError(e.what());
                        }
                        LOG_ERROR("Exception loading animation '" + request.animationId + "': " + e.what());
                    }
                }
                
                lock.lock();
            }
            
            // Process unload requests
            while (!m_unloadRequests.empty()) {
                auto id = m_unloadRequests.front();
                m_unloadRequests.pop();
                lock.unlock();
                
                UnloadAnimationData(id);
                
                lock.lock();
            }
        }
        
        LOG_INFO("Animation streaming thread stopped");
    }

    void AnimationStreamingManager::ProcessLoadRequests() {
        // This method is called from the main thread to handle completed operations
        // The actual loading happens in the streaming thread
    }

    void AnimationStreamingManager::ProcessUnloadRequests() {
        // This method is called from the main thread to handle completed operations
        // The actual unloading happens in the streaming thread
    }

    void AnimationStreamingManager::UpdateMemoryStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        
        m_stats.totalMemoryUsed = 0;
        m_stats.loadedAnimations = 0;
        m_stats.unloadedAnimations = 0;
        m_stats.streamingAnimations = 0;
        m_stats.memoryLimit = m_config.memoryLimitBytes;
        
        for (const auto& [id, animRef] : m_animations) {
            m_stats.totalMemoryUsed += animRef->GetMemoryUsage();
            
            switch (animRef->GetState()) {
                case StreamingState::Loaded:
                    m_stats.loadedAnimations++;
                    break;
                case StreamingState::Unloaded:
                    m_stats.unloadedAnimations++;
                    break;
                case StreamingState::Loading:
                case StreamingState::Unloading:
                    m_stats.streamingAnimations++;
                    break;
                default:
                    break;
            }
        }
        
        m_stats.Calculate();
    }

    void AnimationStreamingManager::CheckMemoryPressure() {
        if (m_stats.memoryUsagePercent > m_config.unloadThreshold) {
            LOG_WARNING("Animation memory usage high: " + 
                       std::to_string(m_stats.memoryUsagePercent * 100.0f) + "%");
            UnloadUnusedAnimations();
        }
    }

    std::shared_ptr<SkeletalAnimation> AnimationStreamingManager::LoadAnimationFromFile(const std::string& filePath) {
        // This is a placeholder implementation
        // In a real implementation, this would use the actual animation file loading system
        LOG_INFO("Loading animation from file: " + filePath);
        
        // For now, create a dummy animation
        auto animation = std::make_shared<SkeletalAnimation>("loaded_animation");
        animation->SetDuration(1.0f);
        animation->SetFrameRate(30.0f);
        
        return animation;
    }

    void AnimationStreamingManager::UnloadAnimationData(const std::string& id) {
        auto it = m_animations.find(id);
        if (it != m_animations.end()) {
            it->second->SetAnimation(nullptr);
            it->second->SetState(StreamingState::Unloaded);
            
            if (m_onAnimationUnloaded) {
                m_onAnimationUnloaded(id);
            }
            
            LOG_INFO("Unloaded animation '" + id + "'");
        }
    }

    std::vector<std::string> AnimationStreamingManager::GetUnusedAnimations() const {
        std::vector<std::string> unused;
        
        for (const auto& [id, animRef] : m_animations) {
            if (animRef->IsLoaded() && animRef->IsUnused(m_config.unusedAnimationTimeout)) {
                unused.push_back(id);
            }
        }
        
        return unused;
    }

    std::vector<std::string> AnimationStreamingManager::GetAnimationsByPriority(StreamingPriority priority) const {
        std::vector<std::string> animations;
        
        for (const auto& [id, animRef] : m_animations) {
            if (animRef->GetPriority() == priority && animRef->IsLoaded()) {
                animations.push_back(id);
            }
        }
        
        return animations;
    }

    // AnimationDataCache implementation
    void AnimationDataCache::CacheAnimation(const std::string& id, std::shared_ptr<SkeletalAnimation> animation) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_cache[id] = animation;
        UpdateAccessOrder(id);
        LOG_INFO("Cached animation '" + id + "'");
    }

    std::shared_ptr<SkeletalAnimation> AnimationDataCache::GetCachedAnimation(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto it = m_cache.find(id);
        if (it != m_cache.end()) {
            UpdateAccessOrder(id);
            m_stats.hits++;
            return it->second;
        }
        
        m_stats.misses++;
        return nullptr;
    }

    void AnimationDataCache::RemoveFromCache(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto it = m_cache.find(id);
        if (it != m_cache.end()) {
            m_cache.erase(it);
            
            // Remove from access order
            auto orderIt = std::find(m_accessOrder.begin(), m_accessOrder.end(), id);
            if (orderIt != m_accessOrder.end()) {
                m_accessOrder.erase(orderIt);
            }
            
            LOG_INFO("Removed animation '" + id + "' from cache");
        }
    }

    void AnimationDataCache::ClearCache() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_cache.clear();
        m_accessOrder.clear();
        LOG_INFO("Cleared animation cache");
    }

    void AnimationDataCache::OptimizeSharedData() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        // This is a placeholder for shared data optimization
        // In a real implementation, this would identify and share common animation data
        LOG_INFO("Optimizing shared animation data");
    }

    size_t AnimationDataCache::GetCacheMemoryUsage() const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        size_t totalMemory = 0;
        for (const auto& [id, animation] : m_cache) {
            totalMemory += animation->GetMemoryUsage();
        }
        
        return totalMemory;
    }

    void AnimationDataCache::UpdateAccessOrder(const std::string& id) {
        // Remove from current position
        auto it = std::find(m_accessOrder.begin(), m_accessOrder.end(), id);
        if (it != m_accessOrder.end()) {
            m_accessOrder.erase(it);
        }
        
        // Add to front (most recently used)
        m_accessOrder.insert(m_accessOrder.begin(), id);
    }

    void AnimationDataCache::EvictLeastRecentlyUsed() {
        if (!m_accessOrder.empty()) {
            std::string lruId = m_accessOrder.back();
            RemoveFromCache(lruId);
            m_stats.evictions++;
        }
    }

    // AnimationPreloader implementation
    AnimationPreloader::AnimationPreloader(AnimationStreamingManager* streamingManager)
        : m_streamingManager(streamingManager) {
    }

    void AnimationPreloader::PredictAnimationUsage(const std::string& currentAnimation, 
                                                  const std::vector<std::string>& likelyNext) {
        // Preload likely next animations
        for (const auto& animId : likelyNext) {
            if (m_streamingManager && !m_streamingManager->IsAnimationLoaded(animId)) {
                m_streamingManager->RequestAnimation(animId, StreamingPriority::Background);
            }
        }
        
        LOG_INFO("Predicted and preloaded " + std::to_string(likelyNext.size()) + 
                " animations for '" + currentAnimation + "'");
    }

    void AnimationPreloader::PreloadAnimationsForState(const std::string& stateName,
                                                      const std::vector<std::string>& animations) {
        for (const auto& animId : animations) {
            if (m_streamingManager && !m_streamingManager->IsAnimationLoaded(animId)) {
                m_streamingManager->RequestAnimation(animId, StreamingPriority::High);
            }
        }
        
        LOG_INFO("Preloaded " + std::to_string(animations.size()) + 
                " animations for state '" + stateName + "'");
    }

    void AnimationPreloader::RecordAnimationTransition(const std::string& from, const std::string& to) {
        m_transitionCounts[from][to]++;
        m_animationUsageCounts[from]++;
    }

    std::vector<std::string> AnimationPreloader::GetPredictedAnimations(const std::string& current) const {
        std::vector<std::string> predictions;
        
        auto it = m_transitionCounts.find(current);
        if (it == m_transitionCounts.end()) {
            return predictions;
        }
        
        const auto& transitions = it->second;
        int totalTransitions = m_animationUsageCounts.at(current);
        
        for (const auto& [toAnim, count] : transitions) {
            float probability = static_cast<float>(count) / static_cast<float>(totalTransitions);
            if (probability >= m_predictionThreshold) {
                predictions.push_back(toAnim);
            }
        }
        
        // Sort by probability (highest first)
        std::sort(predictions.begin(), predictions.end(), [&](const std::string& a, const std::string& b) {
            float probA = CalculateTransitionProbability(current, a);
            float probB = CalculateTransitionProbability(current, b);
            return probA > probB;
        });
        
        // Limit to max predictions
        if (predictions.size() > m_maxPredictions) {
            predictions.resize(m_maxPredictions);
        }
        
        return predictions;
    }

    float AnimationPreloader::CalculateTransitionProbability(const std::string& from, const std::string& to) const {
        auto fromIt = m_transitionCounts.find(from);
        if (fromIt == m_transitionCounts.end()) {
            return 0.0f;
        }
        
        auto toIt = fromIt->second.find(to);
        if (toIt == fromIt->second.end()) {
            return 0.0f;
        }
        
        int transitionCount = toIt->second;
        int totalFromCount = m_animationUsageCounts.at(from);
        
        return static_cast<float>(transitionCount) / static_cast<float>(totalFromCount);
    }

} // namespace Animation
} // namespace GameEngine