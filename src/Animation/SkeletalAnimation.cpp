#include "Animation/SkeletalAnimation.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Animation {

    SkeletalAnimation::SkeletalAnimation(const std::string& name)
        : m_name(name), m_eventManager(std::make_unique<AnimationEventManager>()) {
        // Duration will be updated as keyframes are added
    }

    BoneAnimation* SkeletalAnimation::GetBoneAnimation(const std::string& boneName) {
        auto it = m_boneAnimations.find(boneName);
        return (it != m_boneAnimations.end()) ? it->second.get() : nullptr;
    }

    const BoneAnimation* SkeletalAnimation::GetBoneAnimation(const std::string& boneName) const {
        auto it = m_boneAnimations.find(boneName);
        return (it != m_boneAnimations.end()) ? it->second.get() : nullptr;
    }

    BoneAnimation* SkeletalAnimation::CreateBoneAnimation(const std::string& boneName) {
        if (GetBoneAnimation(boneName)) {
            LOG_WARNING("Bone animation for '" + boneName + "' already exists");
            return GetBoneAnimation(boneName);
        }

        auto boneAnim = std::make_unique<BoneAnimation>(boneName);
        BoneAnimation* ptr = boneAnim.get();
        m_boneAnimations[boneName] = std::move(boneAnim);
        
        LOG_INFO("Created bone animation for '" + boneName + "'");
        return ptr;
    }

    bool SkeletalAnimation::RemoveBoneAnimation(const std::string& boneName) {
        auto it = m_boneAnimations.find(boneName);
        if (it != m_boneAnimations.end()) {
            m_boneAnimations.erase(it);
            return true;
        }
        return false;
    }

    PositionTrack* SkeletalAnimation::GetPositionTrack(const std::string& boneName) {
        auto* boneAnim = GetBoneAnimation(boneName);
        return boneAnim ? boneAnim->positionTrack.get() : nullptr;
    }

    RotationTrack* SkeletalAnimation::GetRotationTrack(const std::string& boneName) {
        auto* boneAnim = GetBoneAnimation(boneName);
        return boneAnim ? boneAnim->rotationTrack.get() : nullptr;
    }

    ScaleTrack* SkeletalAnimation::GetScaleTrack(const std::string& boneName) {
        auto* boneAnim = GetBoneAnimation(boneName);
        return boneAnim ? boneAnim->scaleTrack.get() : nullptr;
    }

    PositionTrack* SkeletalAnimation::CreatePositionTrack(const std::string& boneName) {
        auto* boneAnim = GetOrCreateBoneAnimation(boneName);
        if (!boneAnim->positionTrack) {
            boneAnim->positionTrack = std::make_unique<PositionTrack>(boneName, "position");
        }
        return boneAnim->positionTrack.get();
    }

    RotationTrack* SkeletalAnimation::CreateRotationTrack(const std::string& boneName) {
        auto* boneAnim = GetOrCreateBoneAnimation(boneName);
        if (!boneAnim->rotationTrack) {
            boneAnim->rotationTrack = std::make_unique<RotationTrack>(boneName, "rotation");
        }
        return boneAnim->rotationTrack.get();
    }

    ScaleTrack* SkeletalAnimation::CreateScaleTrack(const std::string& boneName) {
        auto* boneAnim = GetOrCreateBoneAnimation(boneName);
        if (!boneAnim->scaleTrack) {
            boneAnim->scaleTrack = std::make_unique<ScaleTrack>(boneName, "scale");
        }
        return boneAnim->scaleTrack.get();
    }

    void SkeletalAnimation::AddPositionKeyframe(const std::string& boneName, float time, const Math::Vec3& position) {
        auto* track = CreatePositionTrack(boneName);
        track->AddKeyframe(time, position);
        
        // Update duration if necessary
        if (time > m_duration) {
            m_duration = time;
        }
    }

    void SkeletalAnimation::AddRotationKeyframe(const std::string& boneName, float time, const Math::Quat& rotation) {
        auto* track = CreateRotationTrack(boneName);
        track->AddKeyframe(time, rotation);
        
        // Update duration if necessary
        if (time > m_duration) {
            m_duration = time;
        }
    }

    void SkeletalAnimation::AddScaleKeyframe(const std::string& boneName, float time, const Math::Vec3& scale) {
        auto* track = CreateScaleTrack(boneName);
        track->AddKeyframe(time, scale);
        
        // Update duration if necessary
        if (time > m_duration) {
            m_duration = time;
        }
    }

    SkeletalAnimation::BonePose SkeletalAnimation::SampleBone(const std::string& boneName, float time) const {
        BonePose pose;
        
        auto* boneAnim = GetBoneAnimation(boneName);
        if (!boneAnim) {
            return pose;
        }

        // Wrap time according to loop mode
        float wrappedTime = WrapTime(time);

        // Sample position
        if (boneAnim->HasPositionTrack()) {
            pose.position = boneAnim->positionTrack->SampleAt(wrappedTime);
            pose.hasPosition = true;
        }

        // Sample rotation
        if (boneAnim->HasRotationTrack()) {
            pose.rotation = boneAnim->rotationTrack->SampleAt(wrappedTime);
            pose.hasRotation = true;
        }

        // Sample scale
        if (boneAnim->HasScaleTrack()) {
            pose.scale = boneAnim->scaleTrack->SampleAt(wrappedTime);
            pose.hasScale = true;
        }

        return pose;
    }

    std::unordered_map<std::string, SkeletalAnimation::BonePose> SkeletalAnimation::SampleAllBones(float time) const {
        std::unordered_map<std::string, BonePose> poses;
        SampleAllBones(time, poses);
        return poses;
    }

    void SkeletalAnimation::SampleAllBones(float time, std::unordered_map<std::string, BonePose>& outPoses) const {
        outPoses.clear();
        
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->HasAnyTracks()) {
                outPoses[boneName] = SampleBone(boneName, time);
            }
        }
    }

    float SkeletalAnimation::NormalizeTime(float time) const {
        if (m_duration <= 0.0f) {
            return 0.0f;
        }
        return time / m_duration;
    }

    float SkeletalAnimation::WrapTime(float time) const {
        if (m_duration <= 0.0f) {
            return time; // Allow time to pass through if duration is not set
        }

        switch (m_loopMode) {
            case LoopMode::Once:
            case LoopMode::Clamp:
                return std::clamp(time, 0.0f, m_duration);

            case LoopMode::Loop:
                if (time < 0.0f) {
                    return m_duration + std::fmod(time, m_duration);
                }
                // Special case: if time equals duration exactly, return duration
                if (time == m_duration) {
                    return m_duration;
                }
                return std::fmod(time, m_duration);

            case LoopMode::PingPong: {
                float cycleDuration = m_duration * 2.0f;
                float wrappedTime = std::fmod(std::abs(time), cycleDuration);
                
                if (wrappedTime <= m_duration) {
                    return wrappedTime;
                } else {
                    return cycleDuration - wrappedTime;
                }
            }

            default:
                return std::clamp(time, 0.0f, m_duration);
        }
    }

    std::vector<std::string> SkeletalAnimation::GetAnimatedBoneNames() const {
        std::vector<std::string> names;
        names.reserve(m_boneAnimations.size());
        
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->HasAnyTracks()) {
                names.push_back(boneName);
            }
        }
        
        return names;
    }

    bool SkeletalAnimation::HasBone(const std::string& boneName) const {
        auto it = m_boneAnimations.find(boneName);
        return it != m_boneAnimations.end() && it->second->HasAnyTracks();
    }

    void SkeletalAnimation::OptimizeKeyframes(float tolerance) {
        for (auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->positionTrack) {
                boneAnim->positionTrack->OptimizeKeyframes(tolerance);
            }
            if (boneAnim->rotationTrack) {
                boneAnim->rotationTrack->OptimizeKeyframes(tolerance);
            }
            if (boneAnim->scaleTrack) {
                boneAnim->scaleTrack->OptimizeKeyframes(tolerance);
            }
        }
        
        LOG_INFO("Optimized keyframes for skeletal animation '" + m_name + "'");
    }

    void SkeletalAnimation::RecalculateDuration() {
        m_duration = CalculateDurationFromTracks();
        LOG_INFO("Recalculated duration for skeletal animation '" + m_name + "': " + std::to_string(m_duration) + "s");
    }

    bool SkeletalAnimation::ValidateAnimation() const {
        if (m_boneAnimations.empty()) {
            LOG_WARNING("Skeletal animation '" + m_name + "' has no bone animations");
            return false;
        }

        bool hasValidTracks = false;
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->HasAnyTracks()) {
                hasValidTracks = true;
                break;
            }
        }

        if (!hasValidTracks) {
            LOG_WARNING("Skeletal animation '" + m_name + "' has no valid tracks");
            return false;
        }

        if (m_duration <= 0.0f) {
            LOG_WARNING("Skeletal animation '" + m_name + "' has invalid duration: " + std::to_string(m_duration));
            return false;
        }

        return true;
    }

    SkeletalAnimation::AnimationData SkeletalAnimation::Serialize() const {
        AnimationData data;
        data.name = m_name;
        data.duration = m_duration;
        data.frameRate = m_frameRate;
        data.loopMode = m_loopMode;

        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (!boneAnim->HasAnyTracks()) {
                continue;
            }

            AnimationData::BoneData boneData;
            boneData.boneName = boneName;

            // Serialize position keyframes
            if (boneAnim->HasPositionTrack()) {
                const auto& keyframes = boneAnim->positionTrack->GetKeyframes();
                boneData.positionKeyframes.assign(keyframes.begin(), keyframes.end());
            }

            // Serialize rotation keyframes
            if (boneAnim->HasRotationTrack()) {
                const auto& keyframes = boneAnim->rotationTrack->GetKeyframes();
                boneData.rotationKeyframes.assign(keyframes.begin(), keyframes.end());
            }

            // Serialize scale keyframes
            if (boneAnim->HasScaleTrack()) {
                const auto& keyframes = boneAnim->scaleTrack->GetKeyframes();
                boneData.scaleKeyframes.assign(keyframes.begin(), keyframes.end());
            }

            data.bones.push_back(std::move(boneData));
        }

        return data;
    }

    bool SkeletalAnimation::Deserialize(const AnimationData& data) {
        // Clear existing data
        m_boneAnimations.clear();

        // Set basic properties
        m_name = data.name;
        m_duration = data.duration;
        m_frameRate = data.frameRate;
        m_loopMode = data.loopMode;

        // Deserialize bone animations
        for (const auto& boneData : data.bones) {
            auto* boneAnim = CreateBoneAnimation(boneData.boneName);

            // Deserialize position keyframes
            if (!boneData.positionKeyframes.empty()) {
                auto* posTrack = CreatePositionTrack(boneData.boneName);
                for (const auto& keyframe : boneData.positionKeyframes) {
                    posTrack->AddKeyframe(keyframe);
                }
            }

            // Deserialize rotation keyframes
            if (!boneData.rotationKeyframes.empty()) {
                auto* rotTrack = CreateRotationTrack(boneData.boneName);
                for (const auto& keyframe : boneData.rotationKeyframes) {
                    rotTrack->AddKeyframe(keyframe);
                }
            }

            // Deserialize scale keyframes
            if (!boneData.scaleKeyframes.empty()) {
                auto* scaleTrack = CreateScaleTrack(boneData.boneName);
                for (const auto& keyframe : boneData.scaleKeyframes) {
                    scaleTrack->AddKeyframe(keyframe);
                }
            }
        }

        return ValidateAnimation();
    }

    void SkeletalAnimation::CompressAnimation(float tolerance) {
        LOG_INFO("Compressing skeletal animation '" + m_name + "' with tolerance: " + std::to_string(tolerance));
        
        size_t originalKeyframes = GetKeyframeCount();
        
        for (auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->positionTrack) {
                boneAnim->positionTrack->OptimizeKeyframes(tolerance);
            }
            if (boneAnim->rotationTrack) {
                boneAnim->rotationTrack->OptimizeKeyframes(tolerance);
            }
            if (boneAnim->scaleTrack) {
                boneAnim->scaleTrack->OptimizeKeyframes(tolerance);
            }
        }
        
        size_t compressedKeyframes = GetKeyframeCount();
        float compressionRatio = originalKeyframes > 0 ? 
            static_cast<float>(compressedKeyframes) / static_cast<float>(originalKeyframes) : 1.0f;
        
        LOG_INFO("Skeletal animation compression completed:");
        LOG_INFO("  Original keyframes: " + std::to_string(originalKeyframes));
        LOG_INFO("  Compressed keyframes: " + std::to_string(compressedKeyframes));
        LOG_INFO("  Compression ratio: " + std::to_string(compressionRatio));
    }

    void SkeletalAnimation::RemoveRedundantKeyframes(float tolerance) {
        LOG_INFO("Removing redundant keyframes from skeletal animation '" + m_name + "'");
        
        size_t originalKeyframes = GetKeyframeCount();
        
        for (auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->positionTrack) {
                boneAnim->positionTrack->OptimizeKeyframes(tolerance);
            }
            if (boneAnim->rotationTrack) {
                boneAnim->rotationTrack->OptimizeKeyframes(tolerance);
            }
            if (boneAnim->scaleTrack) {
                boneAnim->scaleTrack->OptimizeKeyframes(tolerance);
            }
        }
        
        size_t optimizedKeyframes = GetKeyframeCount();
        LOG_INFO("Removed " + std::to_string(originalKeyframes - optimizedKeyframes) + " redundant keyframes");
    }

    std::shared_ptr<SkeletalAnimation> SkeletalAnimation::CreateCompressedCopy(float tolerance) const {
        auto compressed = std::make_shared<SkeletalAnimation>(m_name + "_compressed");
        compressed->SetDuration(m_duration);
        compressed->SetFrameRate(m_frameRate);
        compressed->SetLoopMode(m_loopMode);
        
        // Copy and compress all bone animations
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (!boneAnim->HasAnyTracks()) {
                continue;
            }
            
            auto* compressedBoneAnim = compressed->CreateBoneAnimation(boneName);
            
            // Copy and optimize position track
            if (boneAnim->HasPositionTrack()) {
                auto* posTrack = compressed->CreatePositionTrack(boneName);
                const auto& keyframes = boneAnim->positionTrack->GetKeyframes();
                for (const auto& keyframe : keyframes) {
                    posTrack->AddKeyframe(keyframe);
                }
                posTrack->OptimizeKeyframes(tolerance);
            }
            
            // Copy and optimize rotation track
            if (boneAnim->HasRotationTrack()) {
                auto* rotTrack = compressed->CreateRotationTrack(boneName);
                const auto& keyframes = boneAnim->rotationTrack->GetKeyframes();
                for (const auto& keyframe : keyframes) {
                    rotTrack->AddKeyframe(keyframe);
                }
                rotTrack->OptimizeKeyframes(tolerance);
            }
            
            // Copy and optimize scale track
            if (boneAnim->HasScaleTrack()) {
                auto* scaleTrack = compressed->CreateScaleTrack(boneName);
                const auto& keyframes = boneAnim->scaleTrack->GetKeyframes();
                for (const auto& keyframe : keyframes) {
                    scaleTrack->AddKeyframe(keyframe);
                }
                scaleTrack->OptimizeKeyframes(tolerance);
            }
        }
        
        // Copy events
        if (m_eventManager) {
            auto events = m_eventManager->GetEvents();
            for (const auto& event : events) {
                compressed->AddEvent(event);
            }
        }
        
        return compressed;
    }

    size_t SkeletalAnimation::GetMemoryUsage() const {
        size_t totalSize = sizeof(SkeletalAnimation);
        totalSize += m_name.size();
        
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            totalSize += sizeof(BoneAnimation);
            totalSize += boneName.size();
            
            if (boneAnim->HasPositionTrack()) {
                totalSize += sizeof(PositionTrack);
                totalSize += sizeof(PositionKeyframe) * boneAnim->positionTrack->GetKeyframeCount();
            }
            if (boneAnim->HasRotationTrack()) {
                totalSize += sizeof(RotationTrack);
                totalSize += sizeof(RotationKeyframe) * boneAnim->rotationTrack->GetKeyframeCount();
            }
            if (boneAnim->HasScaleTrack()) {
                totalSize += sizeof(ScaleTrack);
                totalSize += sizeof(ScaleKeyframe) * boneAnim->scaleTrack->GetKeyframeCount();
            }
        }
        
        // Add event manager memory usage
        if (m_eventManager) {
            totalSize += sizeof(AnimationEventManager);
            totalSize += sizeof(AnimationEvent) * GetEventCount();
        }
        
        return totalSize;
    }

    size_t SkeletalAnimation::GetKeyframeCount() const {
        size_t totalKeyframes = 0;
        
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->HasPositionTrack()) {
                totalKeyframes += boneAnim->positionTrack->GetKeyframeCount();
            }
            if (boneAnim->HasRotationTrack()) {
                totalKeyframes += boneAnim->rotationTrack->GetKeyframeCount();
            }
            if (boneAnim->HasScaleTrack()) {
                totalKeyframes += boneAnim->scaleTrack->GetKeyframeCount();
            }
        }
        
        return totalKeyframes;
    }

    void SkeletalAnimation::PrintAnimationInfo() const {
        LOG_INFO("Skeletal Animation '" + m_name + "':");
        LOG_INFO("  Duration: " + std::to_string(m_duration) + "s");
        LOG_INFO("  Frame Rate: " + std::to_string(m_frameRate) + " fps");
        LOG_INFO("  Loop Mode: " + std::to_string(static_cast<int>(m_loopMode)));
        LOG_INFO("  Bone Count: " + std::to_string(GetBoneCount()));
        LOG_INFO("  Total Keyframes: " + std::to_string(GetKeyframeCount()));
        LOG_INFO("  Memory Usage: " + std::to_string(GetMemoryUsage()) + " bytes");
        
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->HasAnyTracks()) {
                LOG_INFO("  Bone '" + boneName + "':");
                if (boneAnim->HasPositionTrack()) {
                    LOG_INFO("    Position keyframes: " + std::to_string(boneAnim->positionTrack->GetKeyframeCount()));
                }
                if (boneAnim->HasRotationTrack()) {
                    LOG_INFO("    Rotation keyframes: " + std::to_string(boneAnim->rotationTrack->GetKeyframeCount()));
                }
                if (boneAnim->HasScaleTrack()) {
                    LOG_INFO("    Scale keyframes: " + std::to_string(boneAnim->scaleTrack->GetKeyframeCount()));
                }
            }
        }
    }

    BoneAnimation* SkeletalAnimation::GetOrCreateBoneAnimation(const std::string& boneName) {
        auto* existing = GetBoneAnimation(boneName);
        return existing ? existing : CreateBoneAnimation(boneName);
    }

    float SkeletalAnimation::CalculateDurationFromTracks() const {
        float maxDuration = 0.0f;
        
        for (const auto& [boneName, boneAnim] : m_boneAnimations) {
            if (boneAnim->HasPositionTrack()) {
                maxDuration = std::max(maxDuration, boneAnim->positionTrack->GetEndTime());
            }
            if (boneAnim->HasRotationTrack()) {
                maxDuration = std::max(maxDuration, boneAnim->rotationTrack->GetEndTime());
            }
            if (boneAnim->HasScaleTrack()) {
                maxDuration = std::max(maxDuration, boneAnim->scaleTrack->GetEndTime());
            }
        }
        
        return maxDuration;
    }

    // Event system implementation
    void SkeletalAnimation::AddEvent(const AnimationEvent& event) {
        if (m_eventManager) {
            m_eventManager->AddEvent(event);
        }
    }

    void SkeletalAnimation::RemoveEvent(const std::string& eventName, float time) {
        if (m_eventManager) {
            // Convert absolute time to normalized time
            float normalizedTime = NormalizeTime(time);
            m_eventManager->RemoveEvent(eventName, normalizedTime);
        }
    }

    void SkeletalAnimation::RemoveAllEvents(const std::string& eventName) {
        if (m_eventManager) {
            m_eventManager->RemoveAllEvents(eventName);
        }
    }

    void SkeletalAnimation::ClearAllEvents() {
        if (m_eventManager) {
            m_eventManager->ClearAllEvents();
        }
    }

    std::vector<AnimationEvent> SkeletalAnimation::GetEvents() const {
        if (m_eventManager) {
            return m_eventManager->GetEvents();
        }
        return {};
    }

    std::vector<AnimationEvent> SkeletalAnimation::GetEventsInTimeRange(float startTime, float endTime) const {
        if (m_eventManager) {
            // Convert absolute times to normalized times
            float normalizedStart = NormalizeTime(startTime);
            float normalizedEnd = NormalizeTime(endTime);
            return m_eventManager->GetEventsInTimeRange(normalizedStart, normalizedEnd);
        }
        return {};
    }

    std::vector<AnimationEvent> SkeletalAnimation::GetEventsByName(const std::string& eventName) const {
        if (m_eventManager) {
            return m_eventManager->GetEventsByName(eventName);
        }
        return {};
    }

    std::vector<AnimationEvent> SkeletalAnimation::GetEventsByType(AnimationEventType type) const {
        if (m_eventManager) {
            return m_eventManager->GetEventsByType(type);
        }
        return {};
    }

    bool SkeletalAnimation::HasEvent(const std::string& eventName, float time) const {
        if (m_eventManager) {
            float normalizedTime = NormalizeTime(time);
            return m_eventManager->HasEvent(eventName, normalizedTime);
        }
        return false;
    }

    bool SkeletalAnimation::HasEventsInRange(float startTime, float endTime) const {
        if (m_eventManager) {
            float normalizedStart = NormalizeTime(startTime);
            float normalizedEnd = NormalizeTime(endTime);
            return m_eventManager->HasEventsInRange(normalizedStart, normalizedEnd);
        }
        return false;
    }

    size_t SkeletalAnimation::GetEventCount() const {
        if (m_eventManager) {
            return m_eventManager->GetEventCount();
        }
        return 0;
    }

    std::vector<AnimationEvent> SkeletalAnimation::GetTriggeredEvents(float previousTime, float currentTime, bool looping) const {
        if (m_eventManager) {
            float normalizedPrevious = NormalizeTime(previousTime);
            float normalizedCurrent = NormalizeTime(currentTime);
            return m_eventManager->GetTriggeredEvents(normalizedPrevious, normalizedCurrent, looping);
        }
        return {};
    }

    void SkeletalAnimation::ProcessEvents(float previousTime, float currentTime, const AnimationEventCallback& callback, bool looping) const {
        if (m_eventManager && callback) {
            float normalizedPrevious = NormalizeTime(previousTime);
            float normalizedCurrent = NormalizeTime(currentTime);
            m_eventManager->ProcessEvents(normalizedPrevious, normalizedCurrent, callback, looping);
        }
    }

} // namespace Animation
} // namespace GameEngine