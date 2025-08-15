#pragma once

#ifdef GAMEENGINE_HAS_JSON
#include <nlohmann/json.hpp>
#endif

#include "Animation/SkeletalAnimation.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/BlendTree.h"
#include "Animation/AnimationTransition.h"
#include "Animation/Keyframe.h"
#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>

namespace GameEngine {
namespace Animation {

    /**
     * Animation serialization system for caching and storage
     */
    class AnimationSerialization {
    public:
        // Skeletal Animation serialization
        static std::string SerializeSkeletalAnimation(const SkeletalAnimation& animation);
        static std::shared_ptr<SkeletalAnimation> DeserializeSkeletalAnimation(const std::string& jsonData);
        
        // State Machine serialization
        static std::string SerializeStateMachine(const AnimationStateMachine& stateMachine);
        static std::shared_ptr<AnimationStateMachine> DeserializeStateMachine(const std::string& jsonData);
        
        // Blend Tree serialization
        static std::string SerializeBlendTree(const BlendTree& blendTree);
        static std::shared_ptr<BlendTree> DeserializeBlendTree(const std::string& jsonData);
        
        // Animation Transition serialization
        static std::string SerializeTransition(const AnimationTransition& transition);
        static std::shared_ptr<AnimationTransition> DeserializeTransition(const std::string& jsonData);

        // File I/O operations
        static bool SaveAnimationToFile(const SkeletalAnimation& animation, const std::string& filepath);
        static std::shared_ptr<SkeletalAnimation> LoadAnimationFromFile(const std::string& filepath);
        
        static bool SaveStateMachineToFile(const AnimationStateMachine& stateMachine, const std::string& filepath);
        static std::shared_ptr<AnimationStateMachine> LoadStateMachineFromFile(const std::string& filepath);
        
        static bool SaveBlendTreeToFile(const BlendTree& blendTree, const std::string& filepath);
        static std::shared_ptr<BlendTree> LoadBlendTreeFromFile(const std::string& filepath);

        // Asset pipeline integration
        struct AnimationAsset {
            std::string name;
            std::string type; // "skeletal_animation", "state_machine", "blend_tree"
            std::string version;
            std::string sourceFile;
            std::string data; // Serialized JSON data
            uint64_t timestamp;
            size_t dataSize;
        };

        static std::string SerializeAnimationAsset(const AnimationAsset& asset);
        static AnimationAsset DeserializeAnimationAsset(const std::string& jsonData);
        
        static bool SaveAnimationAsset(const AnimationAsset& asset, const std::string& filepath);
        static AnimationAsset LoadAnimationAsset(const std::string& filepath);

        // Batch operations
        struct AnimationCollection {
            std::string name;
            std::string version;
            std::vector<AnimationAsset> animations;
            std::vector<AnimationAsset> stateMachines;
            std::vector<AnimationAsset> blendTrees;
        };

        static std::string SerializeAnimationCollection(const AnimationCollection& collection);
        static AnimationCollection DeserializeAnimationCollection(const std::string& jsonData);
        
        static bool SaveAnimationCollection(const AnimationCollection& collection, const std::string& filepath);
        static AnimationCollection LoadAnimationCollection(const std::string& filepath);

        // Validation and versioning
        static bool ValidateAnimationData(const std::string& jsonData, const std::string& expectedType);
        static std::string GetCurrentVersion();
        static bool IsVersionCompatible(const std::string& version);

    private:
#ifdef GAMEENGINE_HAS_JSON
        // JSON conversion helpers
        static nlohmann::json SkeletalAnimationToJson(const SkeletalAnimation& animation);
        static std::shared_ptr<SkeletalAnimation> JsonToSkeletalAnimation(const nlohmann::json& json);
        
        static nlohmann::json StateMachineToJson(const AnimationStateMachine& stateMachine);
        static std::shared_ptr<AnimationStateMachine> JsonToStateMachine(const nlohmann::json& json);
        
        static nlohmann::json BlendTreeToJson(const BlendTree& blendTree);
        static std::shared_ptr<BlendTree> JsonToBlendTree(const nlohmann::json& json);
        
        static nlohmann::json TransitionToJson(const AnimationTransition& transition);
        static std::shared_ptr<AnimationTransition> JsonToTransition(const nlohmann::json& json);

        // Keyframe serialization helpers
        static nlohmann::json PositionKeyframeToJson(const PositionKeyframe& keyframe);
        static PositionKeyframe JsonToPositionKeyframe(const nlohmann::json& json);
        
        static nlohmann::json RotationKeyframeToJson(const RotationKeyframe& keyframe);
        static RotationKeyframe JsonToRotationKeyframe(const nlohmann::json& json);
        
        static nlohmann::json ScaleKeyframeToJson(const ScaleKeyframe& keyframe);
        static ScaleKeyframe JsonToScaleKeyframe(const nlohmann::json& json);

        // Math type serialization helpers
        static nlohmann::json Vec3ToJson(const Math::Vec3& vec);
        static Math::Vec3 JsonToVec3(const nlohmann::json& json);
        
        static nlohmann::json QuatToJson(const Math::Quat& quat);
        static Math::Quat JsonToQuat(const nlohmann::json& json);

        // Utility functions
        static bool WriteJsonToFile(const nlohmann::json& json, const std::string& filepath);
        static nlohmann::json ReadJsonFromFile(const std::string& filepath);
        static uint64_t GetCurrentTimestamp();
#endif
    };

} // namespace Animation
} // namespace GameEngine