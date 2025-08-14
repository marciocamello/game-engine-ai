#pragma once

#include "Animation/AnimationLOD.h"
#include "Animation/AnimationController.h"
#include "Core/Math.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace GameEngine {
namespace Animation {

    /**
     * Performance optimization settings for animation system
     */
    struct AnimationPerformanceSettings {
        // LOD settings
        bool enableLOD = true;
        float lodDistanceHigh = 25.0f;
        float lodDistanceMedium = 50.0f;
        float lodDistanceLow = 100.0f;
        float lodTransitionTime = 0.5f;
        
        // Culling settings
        bool enableCulling = true;
        bool enableFrustumCulling = true;
        bool enableDistanceCulling = true;
        bool enableOcclusionCulling = false;
        float cullingDistance = 150.0f;
        
        // Performance scaling
        bool enableAdaptiveScaling = true;
        float targetFrameTime = 16.67f; // 60 FPS
        float performanceThreshold = 1.2f; // 20% over target
        
        // Update frequency scaling
        bool enableUpdateFrequencyScaling = true;
        float highLODUpdateFrequency = 1.0f;    // Every frame
        float mediumLODUpdateFrequency = 0.5f;  // Every 2 frames
        float lowLODUpdateFrequency = 0.25f;    // Every 4 frames
        
        // Bone reduction for LOD
        bool enableBoneReduction = true;
        float highLODBoneRatio = 1.0f;    // All bones
        float mediumLODBoneRatio = 0.75f; // 75% of bones
        float lowLODBoneRatio = 0.5f;     // 50% of bones
    };

    /**
     * Animation instance with performance tracking
     */
    struct ManagedAnimationInstance {
        std::shared_ptr<AnimationController> controller;
        uint32_t lodInstanceId = 0;
        
        // Performance tracking
        float lastUpdateTime = 0.0f;
        float updateAccumulator = 0.0f;
        bool needsUpdate = true;
        
        // LOD-specific data
        std::vector<int> activeBones;      // Bones active at current LOD
        std::vector<int> reducedBoneSet;   // Reduced bone set for lower LOD
        
        // Statistics
        float averageUpdateTime = 0.0f;
        int updateCount = 0;
        
        ManagedAnimationInstance() = default;
        ManagedAnimationInstance(std::shared_ptr<AnimationController> ctrl) : controller(ctrl) {}
    };

    /**
     * System-wide animation performance statistics
     */
    struct AnimationSystemStats {
        // Instance counts
        size_t totalInstances = 0;
        size_t activeInstances = 0;
        size_t culledInstances = 0;
        
        // LOD distribution
        size_t highLODInstances = 0;
        size_t mediumLODInstances = 0;
        size_t lowLODInstances = 0;
        size_t disabledInstances = 0;
        
        // Performance metrics
        float totalUpdateTime = 0.0f;
        float averageUpdateTime = 0.0f;
        float frameTime = 0.0f;
        float cpuUsage = 0.0f;
        float memoryUsage = 0.0f;
        
        // Optimization effectiveness
        float performanceGain = 0.0f;      // Percentage improvement
        size_t bonesReduced = 0;           // Total bones reduced by LOD
        size_t updatesSkipped = 0;         // Updates skipped due to frequency scaling
    };

    /**
     * Comprehensive animation performance management system
     */
    class AnimationPerformanceManager {
    public:
        // Lifecycle
        AnimationPerformanceManager();
        ~AnimationPerformanceManager();
        bool Initialize(const AnimationPerformanceSettings& settings = AnimationPerformanceSettings{});
        void Shutdown();

        // Instance management
        uint32_t RegisterAnimationController(std::shared_ptr<AnimationController> controller,
                                           const Math::Vec3& worldPosition,
                                           float boundingRadius = 1.0f,
                                           float importance = 1.0f);
        void UnregisterAnimationController(uint32_t instanceId);
        void UpdateInstanceTransform(uint32_t instanceId, const Math::Vec3& worldPosition);
        void UpdateInstanceImportance(uint32_t instanceId, float importance);

        // Camera and scene management
        void SetCamera(const CameraData& camera);
        void SetSceneBounds(const Math::Vec3& min, const Math::Vec3& max);

        // Performance settings
        void UpdateSettings(const AnimationPerformanceSettings& settings);
        const AnimationPerformanceSettings& GetSettings() const { return m_settings; }

        // Main update loop
        void Update(float deltaTime);
        void UpdateAnimations(float deltaTime);

        // Performance monitoring
        void BeginPerformanceFrame();
        void EndPerformanceFrame();
        const AnimationSystemStats& GetSystemStats() const { return m_stats; }

        // LOD and culling queries
        AnimationLODLevel GetInstanceLOD(uint32_t instanceId) const;
        bool IsInstanceCulled(uint32_t instanceId) const;
        const ManagedAnimationInstance* GetManagedInstance(uint32_t instanceId) const;

        // Performance optimization controls
        void SetPerformanceMode(bool highPerformance);
        void ForceUpdateAllInstances();
        void OptimizeBoneHierarchies();

        // Debug and profiling
        void SetDebugVisualization(bool enabled);
        bool IsDebugVisualizationEnabled() const;
        std::vector<uint32_t> GetInstancesByLOD(AnimationLODLevel lod) const;

        // Callbacks for performance events
        using PerformanceEventCallback = std::function<void(const std::string& event, const AnimationSystemStats& stats)>;
        void SetPerformanceEventCallback(PerformanceEventCallback callback) { m_performanceCallback = callback; }

    private:
        // Core systems
        std::unique_ptr<AnimationLODSystem> m_lodSystem;
        AnimationPerformanceSettings m_settings;

        // Instance management
        std::unordered_map<uint32_t, ManagedAnimationInstance> m_managedInstances;
        uint32_t m_nextInstanceId = 1;

        // Performance tracking
        AnimationSystemStats m_stats;
        std::chrono::high_resolution_clock::time_point m_frameStartTime;
        std::chrono::high_resolution_clock::time_point m_lastStatsUpdate;
        
        // Performance history for adaptive scaling
        std::vector<float> m_frameTimeHistory;
        size_t m_maxHistorySize = 60; // 1 second at 60 FPS
        
        // Debug and callbacks
        bool m_debugVisualization = false;
        PerformanceEventCallback m_performanceCallback;

        // Helper methods
        void UpdateInstanceLOD(uint32_t instanceId, ManagedAnimationInstance& instance);
        void ApplyLODToInstance(ManagedAnimationInstance& instance, AnimationLODLevel lod);
        void UpdateInstanceFrequency(ManagedAnimationInstance& instance, float deltaTime);
        bool ShouldUpdateInstance(const ManagedAnimationInstance& instance, float deltaTime) const;
        
        // Bone optimization
        void CalculateReducedBoneSet(ManagedAnimationInstance& instance, float boneRatio);
        void ApplyBoneReduction(ManagedAnimationInstance& instance, AnimationLODLevel lod);
        
        // Performance analysis
        void UpdatePerformanceStats();
        void AnalyzePerformanceTrends();
        void TriggerPerformanceEvent(const std::string& event);
        
        // Adaptive scaling
        void ApplyAdaptiveScaling();
        float CalculatePerformancePressure() const;
        void AdjustLODDistances(float pressure);
        void AdjustUpdateFrequencies(float pressure);
    };

    /**
     * Utility class for animation performance profiling
     */
    class AnimationProfiler {
    public:
        struct ProfileData {
            std::string name;
            float totalTime = 0.0f;
            float averageTime = 0.0f;
            float minTime = std::numeric_limits<float>::max();
            float maxTime = 0.0f;
            int callCount = 0;
        };

        static AnimationProfiler& GetInstance();
        
        void BeginProfile(const std::string& name);
        void EndProfile(const std::string& name);
        
        const ProfileData* GetProfileData(const std::string& name) const;
        std::vector<ProfileData> GetAllProfileData() const;
        void ClearProfileData();
        
        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

    private:
        AnimationProfiler() = default;
        
        std::unordered_map<std::string, ProfileData> m_profileData;
        std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_startTimes;
        bool m_enabled = false;
    };

    // Profiling macros for easy use
    #define ANIMATION_PROFILE_SCOPE(name) \
        AnimationProfiler::GetInstance().BeginProfile(name); \
        struct ProfileScopeGuard { \
            std::string profileName; \
            ProfileScopeGuard(const std::string& name) : profileName(name) {} \
            ~ProfileScopeGuard() { AnimationProfiler::GetInstance().EndProfile(profileName); } \
        } profileGuard(name);

    #define ANIMATION_PROFILE_FUNCTION() ANIMATION_PROFILE_SCOPE(__FUNCTION__)

} // namespace Animation
} // namespace GameEngine