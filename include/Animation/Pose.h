#pragma once

#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Core/Math.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace GameEngine {
namespace Animation {

    // Forward declarations
    class SkeletalAnimation;

    /**
     * Represents a single bone's transform at a specific time
     */
    struct BoneTransform {
        Math::Vec3 position{0.0f};
        Math::Quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        Math::Vec3 scale{1.0f};

        BoneTransform() = default;
        BoneTransform(const Math::Vec3& pos, const Math::Quat& rot, const Math::Vec3& scl)
            : position(pos), rotation(rot), scale(scl) {}

        // Convert to matrix
        Math::Mat4 ToMatrix() const;
        
        // Interpolation
        static BoneTransform Lerp(const BoneTransform& a, const BoneTransform& b, float t);
        static BoneTransform Slerp(const BoneTransform& a, const BoneTransform& b, float t);
        
        // Operators
        BoneTransform operator+(const BoneTransform& other) const;
        BoneTransform operator*(float weight) const;
        BoneTransform& operator+=(const BoneTransform& other);
        BoneTransform& operator*=(float weight);
    };

    /**
     * Complete pose containing transforms for all bones in a skeleton
     */
    class Pose {
    public:
        Pose() = default;
        explicit Pose(std::shared_ptr<AnimationSkeleton> skeleton);

        // Skeleton association
        void SetSkeleton(std::shared_ptr<AnimationSkeleton> skeleton);
        std::shared_ptr<AnimationSkeleton> GetSkeleton() const { return m_skeleton.lock(); }
        bool HasValidSkeleton() const { return !m_skeleton.expired(); }

        // Bone transform access
        void SetBoneTransform(const std::string& boneName, const BoneTransform& transform);
        void SetBoneTransform(int32_t boneId, const BoneTransform& transform);
        BoneTransform GetBoneTransform(const std::string& boneName) const;
        BoneTransform GetBoneTransform(int32_t boneId) const;
        bool HasBoneTransform(const std::string& boneName) const;

        // Local space transforms (relative to parent)
        void SetLocalTransform(const std::string& boneName, const BoneTransform& transform);
        BoneTransform GetLocalTransform(const std::string& boneName) const;

        // World space transforms (absolute)
        void SetWorldTransform(const std::string& boneName, const BoneTransform& transform);
        BoneTransform GetWorldTransform(const std::string& boneName) const;

        // Pose operations
        void Reset(); // Reset to bind pose
        void ResetToBindPose();
        void Clear(); // Clear all transforms

        // Pose blending
        static Pose Blend(const Pose& poseA, const Pose& poseB, float weight);
        static Pose BlendAdditive(const Pose& basePose, const Pose& additivePose, float weight);
        void BlendWith(const Pose& other, float weight);
        void BlendAdditiveWith(const Pose& additive, float weight);

        // Pose evaluation
        void EvaluateLocalToWorld(); // Convert local transforms to world transforms
        void EvaluateWorldToLocal(); // Convert world transforms to local transforms
        void ApplyToSkeleton(); // Apply pose transforms to skeleton
        void ExtractFromSkeleton(); // Extract current skeleton transforms

        // Skinning matrix generation
        std::vector<Math::Mat4> GetSkinningMatrices() const;
        void GetSkinningMatrices(std::vector<Math::Mat4>& outMatrices) const;

        // Pose information
        size_t GetBoneCount() const { return m_boneTransforms.size(); }
        std::vector<std::string> GetBoneNames() const;
        bool IsEmpty() const { return m_boneTransforms.empty(); }

        // Validation
        bool ValidatePose() const;
        bool IsCompatibleWith(const Pose& other) const;

        // Debugging
        void PrintPoseInfo() const;

    private:
        std::weak_ptr<AnimationSkeleton> m_skeleton;
        std::unordered_map<std::string, BoneTransform> m_boneTransforms;
        std::unordered_map<int32_t, BoneTransform> m_boneTransformsById;

        // Helper methods
        void UpdateBoneTransformMaps();
        BoneTransform GetBindPoseTransform(const std::string& boneName) const;
    };

    /**
     * Pose evaluation utilities
     */
    class PoseEvaluator {
    public:
        // Evaluate animation at specific time
        static Pose EvaluateAnimation(const SkeletalAnimation& animation, float time);
        static Pose EvaluateAnimation(const SkeletalAnimation& animation, float time, std::shared_ptr<AnimationSkeleton> skeleton);

        // Evaluate multiple animations with blending
        struct AnimationLayer {
            const SkeletalAnimation* animation;
            float time;
            float weight;
            bool additive = false;
        };

        static Pose EvaluateAnimationLayers(const std::vector<AnimationLayer>& layers, std::shared_ptr<AnimationSkeleton> skeleton);

        // Pose utilities
        static void ApplyPoseToSkeleton(const Pose& pose, std::shared_ptr<AnimationSkeleton> skeleton);
        static Pose ExtractPoseFromSkeleton(std::shared_ptr<AnimationSkeleton> skeleton);

        // Transform space conversion
        static void ConvertLocalToWorld(Pose& pose);
        static void ConvertWorldToLocal(Pose& pose);

    private:
        static BoneTransform EvaluateBoneAnimation(const SkeletalAnimation& animation, const std::string& boneName, float time);
    };

} // namespace Animation
} // namespace GameEngine