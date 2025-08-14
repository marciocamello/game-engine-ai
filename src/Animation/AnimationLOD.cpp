#include "Animation/AnimationLOD.h"
#include "Animation/AnimationController.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Animation {

    // CameraData implementation
    void CameraData::UpdateFrustumPlanes() {
        // Calculate frustum planes from view-projection matrix
        Math::Mat4 vp = viewProjectionMatrix;
        
        // Left plane
        frustumPlanes[0] = Math::Vec4(
            vp[0][3] + vp[0][0],
            vp[1][3] + vp[1][0],
            vp[2][3] + vp[2][0],
            vp[3][3] + vp[3][0]
        );
        
        // Right plane
        frustumPlanes[1] = Math::Vec4(
            vp[0][3] - vp[0][0],
            vp[1][3] - vp[1][0],
            vp[2][3] - vp[2][0],
            vp[3][3] - vp[3][0]
        );
        
        // Bottom plane
        frustumPlanes[2] = Math::Vec4(
            vp[0][3] + vp[0][1],
            vp[1][3] + vp[1][1],
            vp[2][3] + vp[2][1],
            vp[3][3] + vp[3][1]
        );
        
        // Top plane
        frustumPlanes[3] = Math::Vec4(
            vp[0][3] - vp[0][1],
            vp[1][3] - vp[1][1],
            vp[2][3] - vp[2][1],
            vp[3][3] - vp[3][1]
        );
        
        // Near plane
        frustumPlanes[4] = Math::Vec4(
            vp[0][3] + vp[0][2],
            vp[1][3] + vp[1][2],
            vp[2][3] + vp[2][2],
            vp[3][3] + vp[3][2]
        );
        
        // Far plane
        frustumPlanes[5] = Math::Vec4(
            vp[0][3] - vp[0][2],
            vp[1][3] - vp[1][2],
            vp[2][3] - vp[2][2],
            vp[3][3] - vp[3][2]
        );
        
        // Normalize planes
        for (auto& plane : frustumPlanes) {
            float length = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
            if (length > 0.0f) {
                plane /= length;
            }
        }
    }

    bool CameraData::IsPointInFrustum(const Math::Vec3& point, float radius) const {
        for (const auto& plane : frustumPlanes) {
            float distance = plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }

    // AnimationLODSystem implementation
    AnimationLODSystem::AnimationLODSystem() {
        LOG_INFO("AnimationLODSystem created");
    }

    AnimationLODSystem::~AnimationLODSystem() {
        Shutdown();
        LOG_INFO("AnimationLODSystem destroyed");
    }

    bool AnimationLODSystem::Initialize() {
        LOG_INFO("Initializing AnimationLODSystem");
        
        // Initialize default performance metrics
        m_performanceMetrics.targetFrameTime = 16.67f; // 60 FPS
        m_performanceMetrics.adaptiveScaling = true;
        m_performanceMetrics.lodBias = 1.0f;
        m_performanceMetrics.cullingDistance = 100.0f;
        
        // Initialize camera with default values
        m_camera.UpdateFrustumPlanes();
        
        LOG_INFO("AnimationLODSystem initialized successfully");
        return true;
    }

    void AnimationLODSystem::Shutdown() {
        LOG_INFO("Shutting down AnimationLODSystem");
        
        m_instances.clear();
        m_nextInstanceId = 1;
        
        LOG_INFO("AnimationLODSystem shutdown complete");
    }

    uint32_t AnimationLODSystem::RegisterAnimationInstance(std::shared_ptr<AnimationController> controller,
                                                         const Math::Vec3& worldPosition,
                                                         float boundingRadius,
                                                         float importance) {
        if (!controller) {
            LOG_ERROR("Cannot register null animation controller");
            return 0;
        }
        
        uint32_t instanceId = m_nextInstanceId++;
        
        AnimationInstance instance;
        instance.controller = controller;
        instance.worldPosition = worldPosition;
        instance.boundingRadius = boundingRadius;
        instance.importance = importance;
        instance.instanceId = instanceId;
        instance.currentLOD = AnimationLODLevel::High;
        instance.targetLOD = AnimationLODLevel::High;
        instance.cullingReason = AnimationCullingReason::None;
        instance.isCulled = false;
        
        m_instances[instanceId] = instance;
        
        LOG_DEBUG("Registered animation instance at position");
        
        return instanceId;
    }

    void AnimationLODSystem::UnregisterAnimationInstance(uint32_t instanceId) {
        auto it = m_instances.find(instanceId);
        if (it != m_instances.end()) {
            LOG_DEBUG("Unregistered animation instance");
            m_instances.erase(it);
        } else {
            LOG_WARNING("Attempted to unregister non-existent animation instance");
        }
    }

    void AnimationLODSystem::UpdateInstancePosition(uint32_t instanceId, const Math::Vec3& worldPosition) {
        auto it = m_instances.find(instanceId);
        if (it != m_instances.end()) {
            it->second.worldPosition = worldPosition;
        }
    }

    void AnimationLODSystem::UpdateInstanceImportance(uint32_t instanceId, float importance) {
        auto it = m_instances.find(instanceId);
        if (it != m_instances.end()) {
            it->second.importance = std::max(0.0f, importance);
        }
    }

    void AnimationLODSystem::SetCamera(const CameraData& camera) {
        m_camera = camera;
        m_camera.UpdateFrustumPlanes();
    }

    void AnimationLODSystem::SetLODDistances(float highToMedium, float mediumToLow, float lowToDisabled) {
        m_lodDistanceHighToMedium = std::max(0.0f, highToMedium);
        m_lodDistanceMediumToLow = std::max(m_lodDistanceHighToMedium, mediumToLow);
        m_lodDistanceLowToDisabled = std::max(m_lodDistanceMediumToLow, lowToDisabled);
        
        LOG_DEBUG("LOD distances updated");
    }

    void AnimationLODSystem::SetScreenSizeThresholds(float highThreshold, float mediumThreshold, float lowThreshold) {
        m_screenSizeHighThreshold = std::clamp(highThreshold, 0.0f, 1.0f);
        m_screenSizeMediumThreshold = std::clamp(mediumThreshold, 0.0f, m_screenSizeHighThreshold);
        m_screenSizeLowThreshold = std::clamp(lowThreshold, 0.0f, m_screenSizeMediumThreshold);
    }

    void AnimationLODSystem::UpdatePerformanceMetrics(float frameTime, float cpuUsage, float memoryUsage) {
        m_performanceMetrics.frameTime = frameTime;
        m_performanceMetrics.cpuUsagePercent = cpuUsage;
        m_performanceMetrics.memoryUsageMB = memoryUsage;
        
        // Count active and culled animations
        m_performanceMetrics.activeAnimations = 0;
        m_performanceMetrics.culledAnimations = 0;
        
        for (const auto& pair : m_instances) {
            if (pair.second.isCulled) {
                m_performanceMetrics.culledAnimations++;
            } else {
                m_performanceMetrics.activeAnimations++;
            }
        }
    }

    void AnimationLODSystem::Update(float deltaTime) {
        // Update LOD transitions
        UpdateLODTransitions(deltaTime);
        
        // Evaluate LOD and culling
        EvaluateLOD();
        EvaluateCulling();
        
        // Apply performance scaling if enabled
        if (m_performanceMetrics.adaptiveScaling) {
            ApplyPerformanceScaling();
        }
    }

    void AnimationLODSystem::EvaluateLOD() {
        for (auto& pair : m_instances) {
            AnimationInstance& instance = pair.second;
            
            // Skip culled instances
            if (instance.isCulled) {
                continue;
            }
            
            // Calculate distance and screen size
            instance.distanceToCamera = CalculateDistanceToCamera(instance.worldPosition);
            instance.screenSize = CalculateScreenSize(instance.worldPosition, instance.boundingRadius);
            
            // Determine target LOD
            AnimationLODLevel targetLOD = CalculateLODFromDistance(
                instance.distanceToCamera, 
                instance.screenSize, 
                instance.importance
            );
            
            // Start transition if LOD changed
            if (targetLOD != instance.currentLOD && !instance.isTransitioning) {
                TransitionInstanceLOD(instance, targetLOD);
            }
        }
    }

    void AnimationLODSystem::EvaluateCulling() {
        for (auto& pair : m_instances) {
            AnimationInstance& instance = pair.second;
            
            bool shouldCull = ShouldCullInstance(instance);
            AnimationCullingReason newReason = AnimationCullingReason::None;
            
            if (shouldCull) {
                // Determine culling reason
                if (IsDistanceCulled(instance)) {
                    newReason = AnimationCullingReason::Distance;
                } else if (IsFrustumCulled(instance)) {
                    newReason = AnimationCullingReason::Frustum;
                } else if (IsOcclusionCulled(instance)) {
                    newReason = AnimationCullingReason::Occlusion;
                } else {
                    newReason = AnimationCullingReason::Performance;
                }
            }
            
            // Update culling state if changed
            if (instance.isCulled != shouldCull || instance.cullingReason != newReason) {
                instance.isCulled = shouldCull;
                instance.cullingReason = newReason;
                
                // Notify callback if set
                if (m_cullingChangeCallback) {
                    m_cullingChangeCallback(instance.instanceId, shouldCull, newReason);
                }
            }
        }
    }

    AnimationLODLevel AnimationLODSystem::GetInstanceLOD(uint32_t instanceId) const {
        auto it = m_instances.find(instanceId);
        return (it != m_instances.end()) ? it->second.currentLOD : AnimationLODLevel::High;
    }

    bool AnimationLODSystem::IsInstanceCulled(uint32_t instanceId) const {
        auto it = m_instances.find(instanceId);
        return (it != m_instances.end()) ? it->second.isCulled : false;
    }

    AnimationCullingReason AnimationLODSystem::GetInstanceCullingReason(uint32_t instanceId) const {
        auto it = m_instances.find(instanceId);
        return (it != m_instances.end()) ? it->second.cullingReason : AnimationCullingReason::None;
    }

    const AnimationInstance* AnimationLODSystem::GetInstance(uint32_t instanceId) const {
        auto it = m_instances.find(instanceId);
        return (it != m_instances.end()) ? &it->second : nullptr;
    }

    std::vector<uint32_t> AnimationLODSystem::GetActiveInstances() const {
        std::vector<uint32_t> activeInstances;
        for (const auto& pair : m_instances) {
            if (!pair.second.isCulled) {
                activeInstances.push_back(pair.first);
            }
        }
        return activeInstances;
    }

    std::vector<uint32_t> AnimationLODSystem::GetCulledInstances() const {
        std::vector<uint32_t> culledInstances;
        for (const auto& pair : m_instances) {
            if (pair.second.isCulled) {
                culledInstances.push_back(pair.first);
            }
        }
        return culledInstances;
    }

    // Private helper methods
    float AnimationLODSystem::CalculateDistanceToCamera(const Math::Vec3& position) const {
        Math::Vec3 diff = position - m_camera.position;
        return std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    }

    float AnimationLODSystem::CalculateScreenSize(const Math::Vec3& position, float boundingRadius) const {
        float distance = CalculateDistanceToCamera(position);
        if (distance <= 0.0f) return 1.0f;
        
        // Calculate approximate screen size based on distance and FOV
        float angularSize = 2.0f * std::atan(boundingRadius / distance);
        float screenSize = angularSize / (m_camera.fov * Math::PI / 180.0f);
        
        return std::clamp(screenSize, 0.0f, 1.0f);
    }

    AnimationLODLevel AnimationLODSystem::CalculateLODFromDistance(float distance, float screenSize, float importance) const {
        // Apply importance and LOD bias
        float adjustedDistance = distance / (importance * m_performanceMetrics.lodBias);
        
        // Use both distance and screen size for LOD calculation
        float screenSizeFactor = 1.0f - screenSize; // Smaller screen size = higher LOD level
        float combinedFactor = adjustedDistance * (1.0f + screenSizeFactor);
        
        if (combinedFactor >= m_lodDistanceLowToDisabled) {
            return AnimationLODLevel::Disabled;
        } else if (combinedFactor >= m_lodDistanceMediumToLow) {
            return AnimationLODLevel::Low;
        } else if (combinedFactor >= m_lodDistanceHighToMedium) {
            return AnimationLODLevel::Medium;
        } else {
            return AnimationLODLevel::High;
        }
    }

    bool AnimationLODSystem::ShouldCullInstance(const AnimationInstance& instance) const {
        return IsDistanceCulled(instance) || 
               IsFrustumCulled(instance) || 
               IsOcclusionCulled(instance);
    }

    void AnimationLODSystem::TransitionInstanceLOD(AnimationInstance& instance, AnimationLODLevel targetLOD) {
        if (instance.currentLOD == targetLOD) {
            return;
        }
        
        AnimationLODLevel oldLOD = instance.currentLOD;
        instance.targetLOD = targetLOD;
        instance.isTransitioning = true;
        instance.lodTransitionTime = 0.0f;
        
        // For immediate transitions (no blending needed), apply directly
        if (m_lodTransitionTime <= 0.0f) {
            instance.currentLOD = targetLOD;
            instance.isTransitioning = false;
        }
        
        // Notify callback if set
        if (m_lodChangeCallback) {
            m_lodChangeCallback(instance.instanceId, oldLOD, targetLOD);
        }
    }

    void AnimationLODSystem::UpdateLODTransitions(float deltaTime) {
        for (auto& pair : m_instances) {
            AnimationInstance& instance = pair.second;
            
            if (!instance.isTransitioning) {
                continue;
            }
            
            instance.lodTransitionTime += deltaTime;
            
            if (instance.lodTransitionTime >= m_lodTransitionTime) {
                // Transition complete
                instance.currentLOD = instance.targetLOD;
                instance.isTransitioning = false;
                instance.lodTransitionTime = 0.0f;
            }
        }
    }

    void AnimationLODSystem::ApplyPerformanceScaling() {
        if (m_performanceMetrics.frameTime <= m_performanceMetrics.targetFrameTime) {
            return; // Performance is good, no scaling needed
        }
        
        // Calculate performance pressure
        float performancePressure = m_performanceMetrics.frameTime / m_performanceMetrics.targetFrameTime;
        
        if (performancePressure > 1.5f) {
            // High pressure - increase LOD bias to reduce quality
            m_performanceMetrics.lodBias = std::min(2.0f, m_performanceMetrics.lodBias * 1.1f);
        } else if (performancePressure < 1.1f && m_performanceMetrics.lodBias > 1.0f) {
            // Low pressure - decrease LOD bias to improve quality
            m_performanceMetrics.lodBias = std::max(1.0f, m_performanceMetrics.lodBias * 0.95f);
        }
    }

    bool AnimationLODSystem::IsFrustumCulled(const AnimationInstance& instance) const {
        if (!m_frustumCullingEnabled) {
            return false;
        }
        
        return !m_camera.IsPointInFrustum(instance.worldPosition, instance.boundingRadius);
    }

    bool AnimationLODSystem::IsDistanceCulled(const AnimationInstance& instance) const {
        return instance.distanceToCamera > m_performanceMetrics.cullingDistance;
    }

    bool AnimationLODSystem::IsOcclusionCulled(const AnimationInstance& instance) const {
        // Occlusion culling requires additional systems (occlusion queries, etc.)
        // For now, return false as it's not implemented
        return false;
    }

    // AnimationCullingSystem implementation
    AnimationCullingSystem::AnimationCullingSystem() {
        LOG_INFO("AnimationCullingSystem created");
    }

    AnimationCullingSystem::~AnimationCullingSystem() {
        Shutdown();
        LOG_INFO("AnimationCullingSystem destroyed");
    }

    bool AnimationCullingSystem::Initialize() {
        LOG_INFO("Initializing AnimationCullingSystem");
        
        m_culledCount = 0;
        m_visibleCount = 0;
        
        LOG_INFO("AnimationCullingSystem initialized successfully");
        return true;
    }

    void AnimationCullingSystem::Shutdown() {
        LOG_INFO("Shutting down AnimationCullingSystem");
        LOG_INFO("AnimationCullingSystem shutdown complete");
    }

    void AnimationCullingSystem::EvaluateCulling(const std::vector<AnimationInstance*>& instances, const CameraData& camera) {
        m_culledCount = 0;
        m_visibleCount = 0;
        
        for (AnimationInstance* instance : instances) {
            if (!instance) continue;
            
            bool culled = false;
            
            // Distance culling
            if (PerformDistanceCulling(*instance, camera)) {
                culled = true;
                instance->cullingReason = AnimationCullingReason::Distance;
            }
            // Frustum culling
            else if (PerformFrustumCulling(*instance, camera)) {
                culled = true;
                instance->cullingReason = AnimationCullingReason::Frustum;
            }
            // Occlusion culling
            else if (PerformOcclusionCulling(*instance, camera)) {
                culled = true;
                instance->cullingReason = AnimationCullingReason::Occlusion;
            }
            else {
                instance->cullingReason = AnimationCullingReason::None;
            }
            
            instance->isCulled = culled;
            
            if (culled) {
                m_culledCount++;
            } else {
                m_visibleCount++;
            }
        }
    }

    bool AnimationCullingSystem::PerformFrustumCulling(const AnimationInstance& instance, const CameraData& camera) const {
        if (!m_frustumCullingEnabled) {
            return false;
        }
        
        return !camera.IsPointInFrustum(instance.worldPosition, instance.boundingRadius);
    }

    bool AnimationCullingSystem::PerformDistanceCulling(const AnimationInstance& instance, const CameraData& camera) const {
        Math::Vec3 diff = instance.worldPosition - camera.position;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        return distance > m_cullingDistance;
    }

    bool AnimationCullingSystem::PerformOcclusionCulling(const AnimationInstance& instance, const CameraData& camera) const {
        // Occlusion culling not implemented yet
        return false;
    }

} // namespace Animation
} // namespace GameEngine