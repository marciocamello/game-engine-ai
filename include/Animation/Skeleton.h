#pragma once

#include "Animation/Bone.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace GameEngine {
namespace Animation {

    /**
     * Represents a complete skeletal hierarchy for character animation
     * Manages bones, their relationships, and provides utilities for animation
     */
    class Skeleton {
    public:
        Skeleton(const std::string& name = "Skeleton");
        ~Skeleton() = default;

        // Basic properties
        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

        // Bone management
        std::shared_ptr<Bone> CreateBone(const std::string& name, const Math::Mat4& bindPose = Math::Mat4(1.0f));
        std::shared_ptr<Bone> GetBone(const std::string& name) const;
        std::shared_ptr<Bone> GetBone(int32_t id) const;
        std::shared_ptr<Bone> GetRootBone() const { return m_rootBone; }
        void SetRootBone(std::shared_ptr<Bone> bone) { m_rootBone = bone; }

        // Bone hierarchy utilities
        bool AddBone(std::shared_ptr<Bone> bone, const std::string& parentName = "");
        bool RemoveBone(const std::string& name);
        void SetBoneParent(const std::string& boneName, const std::string& parentName);

        // Bone access and iteration
        const std::vector<std::shared_ptr<Bone>>& GetAllBones() const { return m_bones; }
        std::vector<std::shared_ptr<Bone>> GetRootBones() const;
        size_t GetBoneCount() const { return m_bones.size(); }

        // Transform calculations
        void UpdateBoneTransforms();
        void UpdateBoneTransforms(std::shared_ptr<Bone> bone);
        void UpdateBoneTransformsOptimized(); // Optimized version using bone indices
        std::vector<Math::Mat4> GetSkinningMatrices() const;
        void GetSkinningMatrices(std::vector<Math::Mat4>& outMatrices) const; // Avoid allocation
        
        // Efficient transform updates for animation
        void SetBoneLocalTransform(int32_t boneId, const Math::Mat4& transform);
        void SetBoneLocalTransform(const std::string& boneName, const Math::Mat4& transform);
        void SetBoneLocalTransforms(const std::vector<Math::Mat4>& transforms); // Batch update

        // Bind pose management
        void SetBindPose();
        void RestoreBindPose();
        bool HasValidBindPose() const { return m_hasValidBindPose; }

        // Bone lookup optimization
        void RebuildBoneMaps();
        std::vector<std::string> GetBoneNames() const;

        // Validation and debugging
        bool ValidateHierarchy() const;
        void PrintHierarchy() const;
        int32_t GetMaxDepth() const;

        // Serialization support
        struct SkeletonData {
            std::string name;
            std::vector<std::string> boneNames;
            std::vector<int32_t> boneParents; // -1 for root bones
            std::vector<Math::Mat4> bindPoses;
        };
        
        SkeletonData Serialize() const;
        bool Deserialize(const SkeletonData& data);

    private:
        std::string m_name;
        std::shared_ptr<Bone> m_rootBone;
        std::vector<std::shared_ptr<Bone>> m_bones;
        
        // Lookup maps for performance
        std::unordered_map<std::string, std::shared_ptr<Bone>> m_bonesByName;
        std::unordered_map<int32_t, std::shared_ptr<Bone>> m_bonesById;
        
        bool m_hasValidBindPose = false;
        int32_t m_nextBoneId = 0;

        // Helper methods
        void UpdateBoneTransformsRecursive(std::shared_ptr<Bone> bone, const Math::Mat4& parentTransform);
        void PrintHierarchyRecursive(std::shared_ptr<Bone> bone, int32_t depth) const;
        bool ValidateHierarchyRecursive(std::shared_ptr<Bone> bone, std::unordered_set<int32_t>& visitedIds) const;
    };

} // namespace Animation
} // namespace GameEngine