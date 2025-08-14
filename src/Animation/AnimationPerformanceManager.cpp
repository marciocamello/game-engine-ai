#include "Animation/AnimationPerformanceManager.h"
#include "Animation/AnimationSkeleton.h"
#include "Core/Logger.h"
#include <algorithm>
#include <numeric>
#include <chrono>

namespace GameEngine {
namespace Animation {

    // AnimationPerformanceManager implementation
    AnimationPerformanceManager::AnimationPerformanceManager() {
        LOG_INFO("AnimationPerformanceManager created");
    }

    AnimationPerformanceManager::~AnimationPerformanceManager() {
        Shutdown();
        LOG_INFO("AnimationPerformanceManager destroyed");
    }

    bool AnimationPerformanceManager::Initialize(const AnimationPerformanceSettings& settings) {
        LOG_INFO("Initializing AnimationPerformanceManager");
        
        m_settings = settings;
        
        // Initialize LOD system
        m_lodSystem = std::make_unique<AnimationLODSystem>();
        if (!m_lodSystem->Initialize()) {
            LOG_ERROR("Failed to initialize AnimationLODSystem");
            return false;
        }
        
        // Configure LOD system with settings
        m_lodSystem->SetLODDistances(
            m_settings.lodDistanceHigh,
            m_settings.lodDistanceMedium,
            m_settings.lodDistanceLow
        );
        m_lodSystem->SetLODTransitionTime(m_settings.lodTransitionTime);
        m_lodSystem->SetCullingDistance(m_settings.cullingDistance);
        m_lodSystem->SetFrustumCullingEnabled(m_settings.enableFrustumCulling);
        m_lodSystem->SetOcclusionCullingEnabled(m_settings.enableOcclusionCulling);
        m_lodSystem->SetPerformanceScalingEnabled(m_settings.enableAdaptiveScaling);
        m_lodSystem->SetTargetFrameTime(m_settings.targetFrameTime);
        
        // Initialize performance tracking
        m_frameTimeHistory.reserve(m_maxHistorySize);
        m_lastStatsUpdate = std::chrono::high_resolution_clock::now();
        
        // Initialize stats
        m_stats = AnimationSystemStats{};
        
        LOG_INFO("AnimationPerformanceManager initialized successfully");
        return true;
    }

    void AnimationPerformanceManager::Shutdown() {
        LOG_INFO("Shutting down AnimationPerformanceManager");
        
        if (m_lodSystem) {
            m_lodSystem->Shutdown();
            m_lodSystem.reset();
        }
        
        m_managedInstances.clear();
        m_frameTimeHistory.clear();
        m_nextInstanceId = 1;
        
        LOG_INFO("AnimationPerformanceManager shutdown complete");
    }

    uint32_t AnimationPerformanceManager::RegisterAnimationController(std::shared_ptr<AnimationController> controller,
                                                                    const Math::Vec3& worldPosition,
                                                                    float boundingRadius,
                                                                    float importance) {
        if (!controller || !m_lodSystem) {
            LOG_ERROR("Cannot register animation controller: invalid controller or LOD system");
            return 0;
        }
        
        uint32_t instanceId = m_nextInstanceId++;
        
        // Register with LOD system
        uint32_t lodInstanceId = m_lodSystem->RegisterAnimationInstance(
            controller, worldPosition, boundingRadius, importance
        );
        
        if (lodInstanceId == 0) {
            LOG_ERROR("Failed to register instance with LOD system");
            return 0;
        }
        
        // Create managed instance
        ManagedAnimationInstance managedInstance(controller);
        managedInstance.lodInstanceId = lodInstanceId;
        managedInstance.lastUpdateTime = 0.0f;
        managedInstance.updateAccumulator = 0.0f;
        managedInstance.needsUpdate = true;
        
        // Initialize bone sets if bone reduction is enabled
        if (m_settings.enableBoneReduction && controller->HasValidSkeleton()) {
            auto skeleton = controller->GetSkeleton();
            size_t boneCount = skeleton->GetBoneCount();
            
            // Initialize with all bones active
            managedInstance.activeBones.reserve(boneCount);
            for (size_t i = 0; i < boneCount; ++i) {
                managedInstance.activeBones.push_back(static_cast<int>(i));
            }
            
            // Calculate reduced bone sets for different LOD levels
            CalculateReducedBoneSet(managedInstance, m_settings.mediumLODBoneRatio);
        }
        
        m_managedInstances[instanceId] = managedInstance;
        
        LOG_DEBUG("Registered animation controller with LOD instance");
        return instanceId;
    }

    void AnimationPerformanceManager::UnregisterAnimationController(uint32_t instanceId) {
        auto it = m_managedInstances.find(instanceId);
        if (it != m_managedInstances.end()) {
            // Unregister from LOD system
            if (m_lodSystem) {
                m_lodSystem->UnregisterAnimationInstance(it->second.lodInstanceId);
            }
            
            m_managedInstances.erase(it);
            LOG_DEBUG("Unregistered animation controller");
        } else {
            LOG_WARNING("Attempted to unregister non-existent animation controller");
        }
    }

    void AnimationPerformanceManager::UpdateInstanceTransform(uint32_t instanceId, const Math::Vec3& worldPosition) {
        auto it = m_managedInstances.find(instanceId);
        if (it != m_managedInstances.end() && m_lodSystem) {
            m_lodSystem->UpdateInstancePosition(it->second.lodInstanceId, worldPosition);
        }
    }

    void AnimationPerformanceManager::UpdateInstanceImportance(uint32_t instanceId, float importance) {
        auto it = m_managedInstances.find(instanceId);
        if (it != m_managedInstances.end() && m_lodSystem) {
            m_lodSystem->UpdateInstanceImportance(it->second.lodInstanceId, importance);
        }
    }

    void AnimationPerformanceManager::SetCamera(const CameraData& camera) {
        if (m_lodSystem) {
            m_lodSystem->SetCamera(camera);
        }
    }

    void AnimationPerformanceManager::SetSceneBounds(const Math::Vec3& min, const Math::Vec3& max) {
        // Calculate scene diagonal for automatic culling distance adjustment
        Math::Vec3 diagonal = max - min;
        float sceneSize = std::sqrt(diagonal.x * diagonal.x + diagonal.y * diagonal.y + diagonal.z * diagonal.z);
        
        // Adjust culling distance based on scene size
        if (m_settings.enableAdaptiveScaling) {
            float adaptiveCullingDistance = std::max(m_settings.cullingDistance, sceneSize * 0.5f);
            if (m_lodSystem) {
                m_lodSystem->SetCullingDistance(adaptiveCullingDistance);
            }
        }
    }

    void AnimationPerformanceManager::UpdateSettings(const AnimationPerformanceSettings& settings) {
        m_settings = settings;
        
        if (m_lodSystem) {
            m_lodSystem->SetLODDistances(
                m_settings.lodDistanceHigh,
                m_settings.lodDistanceMedium,
                m_settings.lodDistanceLow
            );
            m_lodSystem->SetLODTransitionTime(m_settings.lodTransitionTime);
            m_lodSystem->SetCullingDistance(m_settings.cullingDistance);
            m_lodSystem->SetFrustumCullingEnabled(m_settings.enableFrustumCulling);
            m_lodSystem->SetOcclusionCullingEnabled(m_settings.enableOcclusionCulling);
            m_lodSystem->SetPerformanceScalingEnabled(m_settings.enableAdaptiveScaling);
            m_lodSystem->SetTargetFrameTime(m_settings.targetFrameTime);
        }
        
        LOG_DEBUG("Animation performance settings updated");
    }

    void AnimationPerformanceManager::Update(float deltaTime) {
        if (!m_lodSystem) return;
        
        // ANIMATION_PROFILE_FUNCTION(); // Disabled for now
        
        // Update LOD system
        m_lodSystem->Update(deltaTime);
        
        // Update managed instances
        for (auto& pair : m_managedInstances) {
            UpdateInstanceLOD(pair.first, pair.second);
            UpdateInstanceFrequency(pair.second, deltaTime);
        }
        
        // Apply adaptive scaling if enabled
        if (m_settings.enableAdaptiveScaling) {
            ApplyAdaptiveScaling();
        }
        
        // Update performance statistics
        UpdatePerformanceStats();
    }

    void AnimationPerformanceManager::UpdateAnimations(float deltaTime) {
        // ANIMATION_PROFILE_SCOPE("UpdateAnimations"); // Disabled for now
        
        m_stats.updatesSkipped = 0;
        
        for (auto& pair : m_managedInstances) {
            ManagedAnimationInstance& instance = pair.second;
            
            // Skip culled instances
            if (m_lodSystem && m_lodSystem->IsInstanceCulled(instance.lodInstanceId)) {
                continue;
            }
            
            // Check if instance needs update based on frequency scaling
            if (!ShouldUpdateInstance(instance, deltaTime)) {
                m_stats.updatesSkipped++;
                continue;
            }
            
            // Update animation controller
            if (instance.controller) {
                auto updateStart = std::chrono::high_resolution_clock::now();
                
                instance.controller->Update(deltaTime);
                
                auto updateEnd = std::chrono::high_resolution_clock::now();
                float updateTime = std::chrono::duration<float, std::milli>(updateEnd - updateStart).count();
                
                // Update performance tracking
                instance.averageUpdateTime = (instance.averageUpdateTime * instance.updateCount + updateTime) / (instance.updateCount + 1);
                instance.updateCount++;
                instance.lastUpdateTime = updateTime;
            }
        }
    }

    void AnimationPerformanceManager::BeginPerformanceFrame() {
        m_frameStartTime = std::chrono::high_resolution_clock::now();
    }

    void AnimationPerformanceManager::EndPerformanceFrame() {
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::milli>(frameEndTime - m_frameStartTime).count();
        
        // Update frame time history
        m_frameTimeHistory.push_back(frameTime);
        if (m_frameTimeHistory.size() > m_maxHistorySize) {
            m_frameTimeHistory.erase(m_frameTimeHistory.begin());
        }
        
        // Update performance metrics
        if (m_lodSystem) {
            // Calculate CPU usage (simplified estimation)
            float cpuUsage = std::min(100.0f, (frameTime / m_settings.targetFrameTime) * 100.0f);
            
            // Memory usage would need to be calculated by the actual memory system
            float memoryUsage = 0.0f; // Placeholder
            
            m_lodSystem->UpdatePerformanceMetrics(frameTime, cpuUsage, memoryUsage);
        }
        
        m_stats.frameTime = frameTime;
    }

    AnimationLODLevel AnimationPerformanceManager::GetInstanceLOD(uint32_t instanceId) const {
        auto it = m_managedInstances.find(instanceId);
        if (it != m_managedInstances.end() && m_lodSystem) {
            return m_lodSystem->GetInstanceLOD(it->second.lodInstanceId);
        }
        return AnimationLODLevel::High;
    }

    bool AnimationPerformanceManager::IsInstanceCulled(uint32_t instanceId) const {
        auto it = m_managedInstances.find(instanceId);
        if (it != m_managedInstances.end() && m_lodSystem) {
            return m_lodSystem->IsInstanceCulled(it->second.lodInstanceId);
        }
        return false;
    }

    const ManagedAnimationInstance* AnimationPerformanceManager::GetManagedInstance(uint32_t instanceId) const {
        auto it = m_managedInstances.find(instanceId);
        return (it != m_managedInstances.end()) ? &it->second : nullptr;
    }

    void AnimationPerformanceManager::SetPerformanceMode(bool highPerformance) {
        if (highPerformance) {
            // Aggressive performance settings
            m_settings.lodDistanceHigh = 15.0f;
            m_settings.lodDistanceMedium = 30.0f;
            m_settings.lodDistanceLow = 60.0f;
            m_settings.cullingDistance = 100.0f;
            m_settings.mediumLODUpdateFrequency = 0.33f; // Every 3 frames
            m_settings.lowLODUpdateFrequency = 0.2f;     // Every 5 frames
        } else {
            // Quality-focused settings
            m_settings.lodDistanceHigh = 40.0f;
            m_settings.lodDistanceMedium = 80.0f;
            m_settings.lodDistanceLow = 150.0f;
            m_settings.cullingDistance = 200.0f;
            m_settings.mediumLODUpdateFrequency = 0.67f; // Every 1.5 frames
            m_settings.lowLODUpdateFrequency = 0.5f;     // Every 2 frames
        }
        
        UpdateSettings(m_settings);
        LOG_INFO("Performance mode updated");
    }

    void AnimationPerformanceManager::ForceUpdateAllInstances() {
        for (auto& pair : m_managedInstances) {
            pair.second.needsUpdate = true;
            pair.second.updateAccumulator = 0.0f;
        }
        LOG_DEBUG("Forced update for all animation instances");
    }

    void AnimationPerformanceManager::OptimizeBoneHierarchies() {
        // ANIMATION_PROFILE_SCOPE("OptimizeBoneHierarchies"); // Disabled for now
        
        for (auto& pair : m_managedInstances) {
            ManagedAnimationInstance& instance = pair.second;
            
            if (m_settings.enableBoneReduction && instance.controller && instance.controller->HasValidSkeleton()) {
                // Recalculate reduced bone sets
                CalculateReducedBoneSet(instance, m_settings.mediumLODBoneRatio);
                
                // Apply current LOD
                AnimationLODLevel currentLOD = GetInstanceLOD(pair.first);
                ApplyBoneReduction(instance, currentLOD);
            }
        }
        
        LOG_DEBUG("Optimized bone hierarchies for all instances");
    }

    void AnimationPerformanceManager::SetDebugVisualization(bool enabled) {
        m_debugVisualization = enabled;
        if (m_lodSystem) {
            m_lodSystem->SetDebugVisualization(enabled);
        }
    }

    bool AnimationPerformanceManager::IsDebugVisualizationEnabled() const {
        return m_debugVisualization;
    }

    std::vector<uint32_t> AnimationPerformanceManager::GetInstancesByLOD(AnimationLODLevel lod) const {
        std::vector<uint32_t> instances;
        
        for (const auto& pair : m_managedInstances) {
            if (GetInstanceLOD(pair.first) == lod) {
                instances.push_back(pair.first);
            }
        }
        
        return instances;
    }

    // Private helper methods
    void AnimationPerformanceManager::UpdateInstanceLOD(uint32_t instanceId, ManagedAnimationInstance& instance) {
        if (!m_lodSystem) return;
        
        AnimationLODLevel currentLOD = m_lodSystem->GetInstanceLOD(instance.lodInstanceId);
        ApplyLODToInstance(instance, currentLOD);
    }

    void AnimationPerformanceManager::ApplyLODToInstance(ManagedAnimationInstance& instance, AnimationLODLevel lod) {
        if (!instance.controller) return;
        
        // Apply update frequency scaling
        float targetFrequency = 1.0f;
        switch (lod) {
            case AnimationLODLevel::High:
                targetFrequency = m_settings.highLODUpdateFrequency;
                break;
            case AnimationLODLevel::Medium:
                targetFrequency = m_settings.mediumLODUpdateFrequency;
                break;
            case AnimationLODLevel::Low:
                targetFrequency = m_settings.lowLODUpdateFrequency;
                break;
            case AnimationLODLevel::Disabled:
                targetFrequency = 0.0f;
                break;
        }
        
        // Apply bone reduction if enabled
        if (m_settings.enableBoneReduction) {
            ApplyBoneReduction(instance, lod);
        }
        
        // Adjust animation controller settings based on LOD
        if (lod == AnimationLODLevel::Disabled) {
            instance.controller->Pause();
        } else {
            if (instance.controller->IsPaused()) {
                instance.controller->Resume();
            }
            
            // Adjust playback speed for lower LOD levels
            float speedMultiplier = 1.0f;
            if (lod == AnimationLODLevel::Low) {
                speedMultiplier = 0.8f; // Slightly slower for less noticeable quality reduction
            }
            
            instance.controller->SetPlaybackSpeed(speedMultiplier);
        }
    }

    void AnimationPerformanceManager::UpdateInstanceFrequency(ManagedAnimationInstance& instance, float deltaTime) {
        if (!m_settings.enableUpdateFrequencyScaling) {
            instance.needsUpdate = true;
            return;
        }
        
        instance.updateAccumulator += deltaTime;
        
        // Determine update frequency based on current LOD
        AnimationLODLevel lod = m_lodSystem ? m_lodSystem->GetInstanceLOD(instance.lodInstanceId) : AnimationLODLevel::High;
        
        float updateInterval = 0.0f;
        switch (lod) {
            case AnimationLODLevel::High:
                updateInterval = 1.0f / (60.0f * m_settings.highLODUpdateFrequency);
                break;
            case AnimationLODLevel::Medium:
                updateInterval = 1.0f / (60.0f * m_settings.mediumLODUpdateFrequency);
                break;
            case AnimationLODLevel::Low:
                updateInterval = 1.0f / (60.0f * m_settings.lowLODUpdateFrequency);
                break;
            case AnimationLODLevel::Disabled:
                instance.needsUpdate = false;
                return;
        }
        
        if (instance.updateAccumulator >= updateInterval) {
            instance.needsUpdate = true;
            instance.updateAccumulator = 0.0f;
        } else {
            instance.needsUpdate = false;
        }
    }

    bool AnimationPerformanceManager::ShouldUpdateInstance(const ManagedAnimationInstance& instance, float deltaTime) const {
        return instance.needsUpdate;
    }

    void AnimationPerformanceManager::CalculateReducedBoneSet(ManagedAnimationInstance& instance, float boneRatio) {
        if (!instance.controller || !instance.controller->HasValidSkeleton()) {
            return;
        }
        
        auto skeleton = instance.controller->GetSkeleton();
        size_t totalBones = skeleton->GetBoneCount();
        size_t targetBoneCount = static_cast<size_t>(totalBones * boneRatio);
        
        // Simple bone reduction: keep root bones and important bones
        instance.reducedBoneSet.clear();
        instance.reducedBoneSet.reserve(targetBoneCount);
        
        // Always include root bones
        auto rootBones = skeleton->GetRootBones();
        for (const auto& rootBone : rootBones) {
            if (instance.reducedBoneSet.size() < targetBoneCount) {
                instance.reducedBoneSet.push_back(rootBone->GetId());
            }
        }
        
        // Add other important bones (simplified heuristic)
        const auto& allBones = skeleton->GetAllBones();
        for (size_t i = 0; i < allBones.size() && instance.reducedBoneSet.size() < targetBoneCount; ++i) {
            int boneId = allBones[i]->GetId();
            if (std::find(instance.reducedBoneSet.begin(), instance.reducedBoneSet.end(), boneId) == instance.reducedBoneSet.end()) {
                // Add bones that have children (important for hierarchy)
                if (allBones[i]->HasChildren()) {
                    instance.reducedBoneSet.push_back(boneId);
                }
            }
        }
        
        // Fill remaining slots with any bones
        for (size_t i = 0; i < allBones.size() && instance.reducedBoneSet.size() < targetBoneCount; ++i) {
            int boneId = allBones[i]->GetId();
            if (std::find(instance.reducedBoneSet.begin(), instance.reducedBoneSet.end(), boneId) == instance.reducedBoneSet.end()) {
                instance.reducedBoneSet.push_back(boneId);
            }
        }
        
        std::sort(instance.reducedBoneSet.begin(), instance.reducedBoneSet.end());
    }

    void AnimationPerformanceManager::ApplyBoneReduction(ManagedAnimationInstance& instance, AnimationLODLevel lod) {
        if (!m_settings.enableBoneReduction || !instance.controller || !instance.controller->HasValidSkeleton()) {
            return;
        }
        
        // Determine which bone set to use based on LOD
        switch (lod) {
            case AnimationLODLevel::High:
                instance.activeBones = instance.activeBones; // Use all bones
                break;
            case AnimationLODLevel::Medium:
            case AnimationLODLevel::Low:
                // Use reduced bone set (this would require integration with the animation system)
                // For now, just track which bones should be active
                break;
            case AnimationLODLevel::Disabled:
                instance.activeBones.clear();
                break;
        }
    }

    void AnimationPerformanceManager::UpdatePerformanceStats() {
        if (!m_lodSystem) return;
        
        // Get LOD system metrics
        const auto& lodMetrics = m_lodSystem->GetPerformanceMetrics();
        
        // Update basic stats
        m_stats.totalInstances = m_managedInstances.size();
        m_stats.activeInstances = lodMetrics.activeAnimations;
        m_stats.culledInstances = lodMetrics.culledAnimations;
        
        // Count LOD distribution
        m_stats.highLODInstances = 0;
        m_stats.mediumLODInstances = 0;
        m_stats.lowLODInstances = 0;
        m_stats.disabledInstances = 0;
        
        for (const auto& pair : m_managedInstances) {
            AnimationLODLevel lod = m_lodSystem->GetInstanceLOD(pair.second.lodInstanceId);
            switch (lod) {
                case AnimationLODLevel::High: m_stats.highLODInstances++; break;
                case AnimationLODLevel::Medium: m_stats.mediumLODInstances++; break;
                case AnimationLODLevel::Low: m_stats.lowLODInstances++; break;
                case AnimationLODLevel::Disabled: m_stats.disabledInstances++; break;
            }
        }
        
        // Calculate performance metrics
        m_stats.frameTime = lodMetrics.frameTime;
        m_stats.cpuUsage = lodMetrics.cpuUsagePercent;
        m_stats.memoryUsage = lodMetrics.memoryUsageMB;
        
        // Calculate total update time
        m_stats.totalUpdateTime = 0.0f;
        int updateCount = 0;
        for (const auto& pair : m_managedInstances) {
            m_stats.totalUpdateTime += pair.second.lastUpdateTime;
            if (pair.second.updateCount > 0) {
                updateCount++;
            }
        }
        
        m_stats.averageUpdateTime = (updateCount > 0) ? m_stats.totalUpdateTime / updateCount : 0.0f;
        
        // Calculate performance gain (simplified)
        float baselineTime = m_stats.totalInstances * 1.0f; // Assume 1ms per instance baseline
        m_stats.performanceGain = (baselineTime > 0.0f) ? 
            ((baselineTime - m_stats.totalUpdateTime) / baselineTime) * 100.0f : 0.0f;
    }

    void AnimationPerformanceManager::AnalyzePerformanceTrends() {
        if (m_frameTimeHistory.size() < 10) return; // Need enough data
        
        // Calculate average frame time over recent history
        float recentAverage = std::accumulate(
            m_frameTimeHistory.end() - 10, m_frameTimeHistory.end(), 0.0f
        ) / 10.0f;
        
        // Trigger events based on performance trends
        if (recentAverage > m_settings.targetFrameTime * m_settings.performanceThreshold) {
            TriggerPerformanceEvent("performance_degradation");
        } else if (recentAverage < m_settings.targetFrameTime * 0.8f) {
            TriggerPerformanceEvent("performance_improved");
        }
    }

    void AnimationPerformanceManager::TriggerPerformanceEvent(const std::string& event) {
        if (m_performanceCallback) {
            m_performanceCallback(event, m_stats);
        }
    }

    void AnimationPerformanceManager::ApplyAdaptiveScaling() {
        float pressure = CalculatePerformancePressure();
        
        if (pressure > 1.2f) { // 20% over target
            AdjustLODDistances(pressure);
            AdjustUpdateFrequencies(pressure);
        }
    }

    float AnimationPerformanceManager::CalculatePerformancePressure() const {
        if (m_frameTimeHistory.empty()) return 1.0f;
        
        float recentAverage = std::accumulate(
            m_frameTimeHistory.end() - std::min(10, static_cast<int>(m_frameTimeHistory.size())),
            m_frameTimeHistory.end(), 0.0f
        ) / std::min(10, static_cast<int>(m_frameTimeHistory.size()));
        
        return recentAverage / m_settings.targetFrameTime;
    }

    void AnimationPerformanceManager::AdjustLODDistances(float pressure) {
        // Reduce LOD distances under pressure
        float scaleFactor = 1.0f / pressure;
        
        AnimationPerformanceSettings adjustedSettings = m_settings;
        adjustedSettings.lodDistanceHigh *= scaleFactor;
        adjustedSettings.lodDistanceMedium *= scaleFactor;
        adjustedSettings.lodDistanceLow *= scaleFactor;
        
        if (m_lodSystem) {
            m_lodSystem->SetLODDistances(
                adjustedSettings.lodDistanceHigh,
                adjustedSettings.lodDistanceMedium,
                adjustedSettings.lodDistanceLow
            );
        }
    }

    void AnimationPerformanceManager::AdjustUpdateFrequencies(float pressure) {
        // Reduce update frequencies under pressure
        float scaleFactor = 1.0f / pressure;
        
        m_settings.mediumLODUpdateFrequency *= scaleFactor;
        m_settings.lowLODUpdateFrequency *= scaleFactor;
        
        // Clamp to reasonable values
        m_settings.mediumLODUpdateFrequency = std::max(0.1f, m_settings.mediumLODUpdateFrequency);
        m_settings.lowLODUpdateFrequency = std::max(0.05f, m_settings.lowLODUpdateFrequency);
    }

    // AnimationProfiler implementation
    AnimationProfiler& AnimationProfiler::GetInstance() {
        static AnimationProfiler instance;
        return instance;
    }

    void AnimationProfiler::BeginProfile(const std::string& name) {
        if (!m_enabled) return;
        
        m_startTimes[name] = std::chrono::high_resolution_clock::now();
    }

    void AnimationProfiler::EndProfile(const std::string& name) {
        if (!m_enabled) return;
        
        auto it = m_startTimes.find(name);
        if (it == m_startTimes.end()) return;
        
        auto endTime = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration<float, std::milli>(endTime - it->second).count();
        
        // Update profile data
        ProfileData& data = m_profileData[name];
        data.name = name;
        data.totalTime += duration;
        data.callCount++;
        data.averageTime = data.totalTime / data.callCount;
        data.minTime = std::min(data.minTime, duration);
        data.maxTime = std::max(data.maxTime, duration);
        
        m_startTimes.erase(it);
    }

    const AnimationProfiler::ProfileData* AnimationProfiler::GetProfileData(const std::string& name) const {
        auto it = m_profileData.find(name);
        return (it != m_profileData.end()) ? &it->second : nullptr;
    }

    std::vector<AnimationProfiler::ProfileData> AnimationProfiler::GetAllProfileData() const {
        std::vector<ProfileData> allData;
        allData.reserve(m_profileData.size());
        
        for (const auto& pair : m_profileData) {
            allData.push_back(pair.second);
        }
        
        return allData;
    }

    void AnimationProfiler::ClearProfileData() {
        m_profileData.clear();
        m_startTimes.clear();
    }

} // namespace Animation
} // namespace GameEngine