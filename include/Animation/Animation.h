#pragma once

#include "Animation/Keyframe.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimationEvent.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace GameEngine {
namespace Animation {

    /**
     * Animation loop modes
     */
    enum class LoopMode {
        Once,       // Play once and stop
        Loop,       // Loop continuously
        PingPong,   // Play forward, then backward, repeat
        Clamp       // Play once and hold last frame
    };

    /**
     * Bone animation data containing all tracks for a single bone
     */
    struct BoneAnimation {
        std::string boneName;
        std::unique_ptr<PositionTrack> positionTrack;
        std::unique_ptr<RotationTrack> rotationTrack;
        std::unique_ptr<ScaleTrack> scaleTrack;

        BoneAnimation(const std::string& name) : boneName(name) {}
        
        bool HasPositionTrack() const { return positionTrack && !positionTrack->IsEmpty(); }
        bool HasRotationTrack() const { return rotationTrack && !rotationTrack->IsEmpty(); }
        bool HasScaleTrack() const { return scaleTrack && !scaleTrack->IsEmpty(); }
        bool HasAnyTracks() const { return HasPositionTrack() || HasRotationTrack() || HasScaleTrack(); }
    };

    /**
     * Complete animation containing all bone tracks and metadata
     */
    class Animation {
    public:
        Animation(const std::string& name = "Animation");
        ~Animation() = default;

        // Basic properties
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        float GetDuration() const { return m_duration; }
        void SetDuration(float duration) { m_duration = duration; }

        float GetFrameRate() const { return m_frameRate; }
        void SetFrameRate(float frameRate) { m_frameRate = frameRate; }

        LoopMode GetLoopMode() const { return m_loopMode; }
        void SetLoopMode(LoopMode mode) { m_loopMode = mode; }

        // Bone animation management
        BoneAnimation* GetBoneAnimation(const std::string& boneName);
        const BoneAnimation* GetBoneAnimation(const std::string& boneName) const;
        BoneAnimation* CreateBoneAnimation(const std::string& boneName);
        bool RemoveBoneAnimation(const std::string& boneName);
        
        const std::unordered_map<std::string, std::unique_ptr<BoneAnimation>>& GetBoneAnimations() const { 
            return m_boneAnimations; 
        }

        // Track management
        PositionTrack* GetPositionTrack(const std::string& boneName);
        RotationTrack* GetRotationTrack(const std::string& boneName);
        ScaleTrack* GetScaleTrack(const std::string& boneName);

        PositionTrack* CreatePositionTrack(const std::string& boneName);
        RotationTrack* CreateRotationTrack(const std::string& boneName);
        ScaleTrack* CreateScaleTrack(const std::string& boneName);

        // Keyframe utilities
        void AddPositionKeyframe(const std::string& boneName, float time, const Math::Vec3& position);
        void AddRotationKeyframe(const std::string& boneName, float time, const Math::Quat& rotation);
        void AddScaleKeyframe(const std::string& boneName, float time, const Math::Vec3& scale);

        // Animation sampling
        struct BonePose {
            Math::Vec3 position{0.0f};
            Math::Quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
            Math::Vec3 scale{1.0f};
            bool hasPosition = false;
            bool hasRotation = false;
            bool hasScale = false;
        };

        BonePose SampleBone(const std::string& boneName, float time) const;
        std::unordered_map<std::string, BonePose> SampleAllBones(float time) const;
        void SampleAllBones(float time, std::unordered_map<std::string, BonePose>& outPoses) const;

        // Time utilities
        float NormalizeTime(float time) const;
        float WrapTime(float time) const;
        bool IsTimeInRange(float time) const { return time >= 0.0f && time <= m_duration; }

        // Animation information
        std::vector<std::string> GetAnimatedBoneNames() const;
        size_t GetBoneCount() const { return m_boneAnimations.size(); }
        bool IsEmpty() const { return m_boneAnimations.empty(); }
        bool HasBone(const std::string& boneName) const;

        // Event system
        void AddEvent(const AnimationEvent& event);
        void RemoveEvent(const std::string& eventName, float time);
        void RemoveAllEvents(const std::string& eventName);
        void ClearAllEvents();
        
        std::vector<AnimationEvent> GetEvents() const;
        std::vector<AnimationEvent> GetEventsInTimeRange(float startTime, float endTime) const;
        std::vector<AnimationEvent> GetEventsByName(const std::string& eventName) const;
        std::vector<AnimationEvent> GetEventsByType(AnimationEventType type) const;
        
        bool HasEvent(const std::string& eventName, float time) const;
        bool HasEventsInRange(float startTime, float endTime) const;
        size_t GetEventCount() const;
        
        // Event processing during animation playback
        std::vector<AnimationEvent> GetTriggeredEvents(float previousTime, float currentTime, bool looping = false) const;
        void ProcessEvents(float previousTime, float currentTime, const AnimationEventCallback& callback, bool looping = false) const;

        // Optimization and validation
        void OptimizeKeyframes(float tolerance = 0.001f);
        void RecalculateDuration();
        bool ValidateAnimation() const;

        // Compression and optimization
        void CompressAnimation(float tolerance = 0.001f);
        void RemoveRedundantKeyframes(float tolerance = 0.001f);
        std::shared_ptr<Animation> CreateCompressedCopy(float tolerance = 0.001f) const;
        
        // Memory usage analysis
        size_t GetMemoryUsage() const;
        size_t GetKeyframeCount() const;

        // Serialization support
        struct AnimationData {
            std::string name;
            float duration;
            float frameRate;
            LoopMode loopMode;
            
            struct BoneData {
                std::string boneName;
                std::vector<PositionKeyframe> positionKeyframes;
                std::vector<RotationKeyframe> rotationKeyframes;
                std::vector<ScaleKeyframe> scaleKeyframes;
            };
            
            std::vector<BoneData> bones;
        };

        AnimationData Serialize() const;
        bool Deserialize(const AnimationData& data);

        // Debugging
        void PrintAnimationInfo() const;

    private:
        std::string m_name;
        float m_duration = 0.0f;
        float m_frameRate = 30.0f;
        LoopMode m_loopMode = LoopMode::Loop;

        std::unordered_map<std::string, std::unique_ptr<BoneAnimation>> m_boneAnimations;
        
        // Event management
        std::unique_ptr<AnimationEventManager> m_eventManager;

        // Helper methods
        BoneAnimation* GetOrCreateBoneAnimation(const std::string& boneName);
        float CalculateDurationFromTracks() const;
    };

} // namespace Animation
} // namespace GameEngine