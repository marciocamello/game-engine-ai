#pragma once

#include "Core/Math.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <array>

namespace GameEngine {
namespace Animation {

    // Forward declarations
    class AnimationController;
    class AnimationSkeleton;

    /**
     * Level of Detail settings for animation optimization
     */
    enum class AnimationLODLevel {
        High = 0,    // Full animation quality
        Medium = 1,  // Reduced bone count or update frequency
        Low = 2,     // Minimal animation updates
        Disabled = 3 // No animation updates
    };

    /**
     * Animation culling reasons for debugging and optimization
     */
    enum class AnimationCullingReason {
        None = 0,
        Distance,      // Too far from camera
        Frustum,       // Outside camera frustum
        Occlusion,     // Occluded by other objects
        Performance,   // System performance scaling
        Manual         // Manually disabled
    };

    /**
     * Performance metrics for animation system scaling
     */
    struct AnimationPerformanceMetrics {
        float frameTime = 0.0f;           // Current frame time in ms
        float targetFrameTime = 16.67f;   // Target frame time (60 FPS)
        int activeAnimations = 0;         // Number of active animations
        int culledAnimations = 0;         // Number of culled animations
        float cpuUsagePercent = 0.0f;     // CPU usage percentage
        float memoryUsageMB = 0.0f;       // Memory usage in MB
        
        // Performance scaling factors
        float lodBias = 1.0f;             // LOD bias multiplier
        float cullingDistance = 100.0f;   // Maximum culling distance
        bool adaptiveScaling = true;      // Enable adaptive performance scaling
    };

    /**
     * Animation instance data for LOD and culling calculations
     */
    struct AnimationInstance {
        std::shared_ptr<AnimationController> controller;
        Math::Vec3 worldPosition = Math::Vec3(0.0f);
        float boundingRadius = 1.0f;
        AnimationLODLevel currentLOD = AnimationLODLevel::High;
        AnimationCullingReason cullingReason = AnimationCullingReason::None;
        bool isCulled = false;
        float distanceToCamera = 0.0f;
        float screenSize = 1.0f;          // Approximate screen size (0-1)
        float importance = 1.0f;          // Animation importance factor
        uint32_t instanceId = 0;
        
        // LOD transition data
        float lodTransitionTime = 0.0f;
        AnimationLODLevel targetLOD = AnimationLODLevel::High;
        bool isTransitioning = false;
    };

    /**
     * Camera data for culling calculations
     */
    struct CameraData {
        Math::Vec3 position = Math::Vec3(0.0f);
        Math::Vec3 forward = Math::Vec3(0.0f, 0.0f, -1.0f);
        Math::Mat4 viewMatrix = Math::Mat4(1.0f);
        Math::Mat4 projectionMatrix = Math::Mat4(1.0f);
        Math::Mat4 viewProjectionMatrix = Math::Mat4(1.0f);
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        float fov = 45.0f;
        float aspectRatio = 16.0f / 9.0f;
        
        // Frustum planes for culling
        std::array<Math::Vec4, 6> frustumPlanes;
        
        void UpdateFrustumPlanes();
        bool IsPointInFrustum(const Math::Vec3& point, float radius = 0.0f) const;
    };

    /**
     * Animation LOD and culling system for performance optimization
     */
    class AnimationLODSystem {
    public:
        // Lifecycle
        AnimationLODSystem();
        ~AnimationLODSystem();
        bool Initialize();
        void Shutdown();

        // Instance management
        uint32_t RegisterAnimationInstance(std::shared_ptr<AnimationController> controller,
                                         const Math::Vec3& worldPosition,
                                         float boundingRadius = 1.0f,
                                         float importance = 1.0f);
        void UnregisterAnimationInstance(uint32_t instanceId);
        void UpdateInstancePosition(uint32_t instanceId, const Math::Vec3& worldPosition);
        void UpdateInstanceImportance(uint32_t instanceId, float importance);

        // Camera management
        void SetCamera(const CameraData& camera);
        const CameraData& GetCamera() const { return m_camera; }

        // LOD configuration
        void SetLODDistances(float highToMedium, float mediumToLow, float lowToDisabled);
        void SetLODTransitionTime(float transitionTime) { m_lodTransitionTime = transitionTime; }
        void SetScreenSizeThresholds(float highThreshold, float mediumThreshold, float lowThreshold);

        // Culling configuration
        void SetCullingDistance(float distance) { m_performanceMetrics.cullingDistance = distance; }
        void SetFrustumCullingEnabled(bool enabled) { m_frustumCullingEnabled = enabled; }
        void SetOcclusionCullingEnabled(bool enabled) { m_occlusionCullingEnabled = enabled; }

        // Performance scaling
        void SetPerformanceScalingEnabled(bool enabled) { m_performanceMetrics.adaptiveScaling = enabled; }
        void SetTargetFrameTime(float targetMs) { m_performanceMetrics.targetFrameTime = targetMs; }
        void UpdatePerformanceMetrics(float frameTime, float cpuUsage, float memoryUsage);

        // Update and evaluation
        void Update(float deltaTime);
        void EvaluateLOD();
        void EvaluateCulling();

        // Query methods
        AnimationLODLevel GetInstanceLOD(uint32_t instanceId) const;
        bool IsInstanceCulled(uint32_t instanceId) const;
        AnimationCullingReason GetInstanceCullingReason(uint32_t instanceId) const;
        const AnimationInstance* GetInstance(uint32_t instanceId) const;

        // Statistics and debugging
        const AnimationPerformanceMetrics& GetPerformanceMetrics() const { return m_performanceMetrics; }
        std::vector<uint32_t> GetActiveInstances() const;
        std::vector<uint32_t> GetCulledInstances() const;
        size_t GetInstanceCount() const { return m_instances.size(); }

        // Debug visualization
        void SetDebugVisualization(bool enabled) { m_debugVisualization = enabled; }
        bool IsDebugVisualizationEnabled() const { return m_debugVisualization; }

        // Callbacks for LOD changes
        using LODChangeCallback = std::function<void(uint32_t instanceId, AnimationLODLevel oldLOD, AnimationLODLevel newLOD)>;
        using CullingChangeCallback = std::function<void(uint32_t instanceId, bool culled, AnimationCullingReason reason)>;
        
        void SetLODChangeCallback(LODChangeCallback callback) { m_lodChangeCallback = callback; }
        void SetCullingChangeCallback(CullingChangeCallback callback) { m_cullingChangeCallback = callback; }

    private:
        // Instance storage
        std::unordered_map<uint32_t, AnimationInstance> m_instances;
        uint32_t m_nextInstanceId = 1;

        // Camera and culling data
        CameraData m_camera;
        bool m_frustumCullingEnabled = true;
        bool m_occlusionCullingEnabled = false; // Requires additional occlusion system

        // LOD configuration
        float m_lodDistanceHighToMedium = 25.0f;
        float m_lodDistanceMediumToLow = 50.0f;
        float m_lodDistanceLowToDisabled = 100.0f;
        float m_lodTransitionTime = 0.5f;

        // Screen size thresholds for LOD
        float m_screenSizeHighThreshold = 0.1f;   // 10% of screen
        float m_screenSizeMediumThreshold = 0.05f; // 5% of screen
        float m_screenSizeLowThreshold = 0.01f;    // 1% of screen

        // Performance metrics and scaling
        AnimationPerformanceMetrics m_performanceMetrics;

        // Debug and callbacks
        bool m_debugVisualization = false;
        LODChangeCallback m_lodChangeCallback;
        CullingChangeCallback m_cullingChangeCallback;

        // Helper methods
        float CalculateDistanceToCamera(const Math::Vec3& position) const;
        float CalculateScreenSize(const Math::Vec3& position, float boundingRadius) const;
        AnimationLODLevel CalculateLODFromDistance(float distance, float screenSize, float importance) const;
        bool ShouldCullInstance(const AnimationInstance& instance) const;
        void TransitionInstanceLOD(AnimationInstance& instance, AnimationLODLevel targetLOD);
        void UpdateLODTransitions(float deltaTime);
        void ApplyPerformanceScaling();
        
        // Culling tests
        bool IsFrustumCulled(const AnimationInstance& instance) const;
        bool IsDistanceCulled(const AnimationInstance& instance) const;
        bool IsOcclusionCulled(const AnimationInstance& instance) const;
    };

    /**
     * Animation culling system for managing visibility and performance
     */
    class AnimationCullingSystem {
    public:
        // Lifecycle
        AnimationCullingSystem();
        ~AnimationCullingSystem();
        bool Initialize();
        void Shutdown();

        // Culling evaluation
        void EvaluateCulling(const std::vector<AnimationInstance*>& instances, const CameraData& camera);
        
        // Culling configuration
        void SetCullingDistance(float distance) { m_cullingDistance = distance; }
        void SetFrustumCullingEnabled(bool enabled) { m_frustumCullingEnabled = enabled; }
        void SetOcclusionCullingEnabled(bool enabled) { m_occlusionCullingEnabled = enabled; }
        
        // Statistics
        size_t GetCulledCount() const { return m_culledCount; }
        size_t GetVisibleCount() const { return m_visibleCount; }

    private:
        float m_cullingDistance = 100.0f;
        bool m_frustumCullingEnabled = true;
        bool m_occlusionCullingEnabled = false;
        
        size_t m_culledCount = 0;
        size_t m_visibleCount = 0;
        
        // Culling methods
        bool PerformFrustumCulling(const AnimationInstance& instance, const CameraData& camera) const;
        bool PerformDistanceCulling(const AnimationInstance& instance, const CameraData& camera) const;
        bool PerformOcclusionCulling(const AnimationInstance& instance, const CameraData& camera) const;
    };

} // namespace Animation
} // namespace GameEngine