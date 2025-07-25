#pragma once

#include "Core/Math.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace GameEngine {

    /**
     * @brief Bone in a skeletal hierarchy
     */
    class Bone : public std::enable_shared_from_this<Bone> {
    public:
        Bone(const std::string& name = "", int32_t index = -1);
        ~Bone() = default;

        // Basic properties
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        void SetIndex(int32_t index) { m_index = index; }
        int32_t GetIndex() const { return m_index; }

        // Hierarchy management
        void SetParent(std::shared_ptr<Bone> parent);
        std::shared_ptr<Bone> GetParent() const { return m_parent.lock(); }
        
        void AddChild(std::shared_ptr<Bone> child);
        void RemoveChild(std::shared_ptr<Bone> child);
        const std::vector<std::shared_ptr<Bone>>& GetChildren() const { return m_children; }

        // Transform management
        void SetLocalTransform(const Math::Mat4& transform) { m_localTransform = transform; }
        const Math::Mat4& GetLocalTransform() const { return m_localTransform; }

        void SetWorldTransform(const Math::Mat4& transform) { m_worldTransform = transform; }
        const Math::Mat4& GetWorldTransform() const { return m_worldTransform; }

        void SetInverseBindMatrix(const Math::Mat4& matrix) { m_inverseBindMatrix = matrix; }
        const Math::Mat4& GetInverseBindMatrix() const { return m_inverseBindMatrix; }

        // Calculate final bone matrix for skinning
        Math::Mat4 GetSkinningMatrix() const;

        // Update transforms recursively
        void UpdateTransforms(const Math::Mat4& parentTransform = Math::Mat4(1.0f));

        // Utility methods
        bool IsRoot() const { return m_parent.expired(); }
        bool IsLeaf() const { return m_children.empty(); }
        size_t GetDepth() const;

    private:
        std::string m_name;
        int32_t m_index;

        // Hierarchy
        std::weak_ptr<Bone> m_parent;
        std::vector<std::shared_ptr<Bone>> m_children;

        // Transforms
        Math::Mat4 m_localTransform = Math::Mat4(1.0f);
        Math::Mat4 m_worldTransform = Math::Mat4(1.0f);
        Math::Mat4 m_inverseBindMatrix = Math::Mat4(1.0f);
    };

    /**
     * @brief Skeletal system for bone-based animation
     */
    class Skeleton {
    public:
        Skeleton() = default;
        ~Skeleton() = default;

        // Bone management
        void AddBone(std::shared_ptr<Bone> bone);
        void SetBones(const std::vector<std::shared_ptr<Bone>>& bones);
        const std::vector<std::shared_ptr<Bone>>& GetBones() const { return m_bones; }

        std::shared_ptr<Bone> GetBone(size_t index) const;
        std::shared_ptr<Bone> FindBone(const std::string& name) const;
        size_t GetBoneCount() const { return m_bones.size(); }

        // Root bone management
        void SetRootBone(std::shared_ptr<Bone> root) { m_rootBone = root; }
        std::shared_ptr<Bone> GetRootBone() const { return m_rootBone; }

        // Bone matrices for GPU skinning
        std::vector<Math::Mat4> GetBoneMatrices() const;
        void UpdateBoneMatrices();

        // Hierarchy operations
        void BuildHierarchy();
        void ValidateHierarchy() const;

        // Utility methods
        void PrintHierarchy() const;
        size_t GetMaxDepth() const;

        // Bind pose management
        void SetBindPose();
        void RestoreBindPose();

    private:
        std::vector<std::shared_ptr<Bone>> m_bones;
        std::shared_ptr<Bone> m_rootBone;
        std::unordered_map<std::string, std::shared_ptr<Bone>> m_boneMap;

        // Cached bone matrices for GPU upload
        mutable std::vector<Math::Mat4> m_boneMatrices;
        mutable bool m_matricesDirty = true;

        void BuildBoneMap();
        void PrintBoneHierarchy(std::shared_ptr<Bone> bone, size_t depth = 0) const;
    };

    /**
     * @brief Skin binding information for meshes
     */
    class Skin {
    public:
        Skin() = default;
        ~Skin() = default;

        void SetSkeleton(std::shared_ptr<Skeleton> skeleton) { m_skeleton = skeleton; }
        std::shared_ptr<Skeleton> GetSkeleton() const { return m_skeleton; }

        void SetInverseBindMatrices(const std::vector<Math::Mat4>& matrices) { m_inverseBindMatrices = matrices; }
        const std::vector<Math::Mat4>& GetInverseBindMatrices() const { return m_inverseBindMatrices; }

        void SetJoints(const std::vector<uint32_t>& joints) { m_joints = joints; }
        const std::vector<uint32_t>& GetJoints() const { return m_joints; }

        // Get final bone matrices for skinning
        std::vector<Math::Mat4> GetSkinningMatrices() const;

        // Validation
        bool IsValid() const;

    private:
        std::shared_ptr<Skeleton> m_skeleton;
        std::vector<Math::Mat4> m_inverseBindMatrices;
        std::vector<uint32_t> m_joints; // Indices into skeleton bones
    };

} // namespace GameEngine