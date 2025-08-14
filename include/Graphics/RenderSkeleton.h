#pragma once

#include "Core/Math.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace GameEngine {
namespace Graphics {

    /**
     * @brief Bone in a skeletal hierarchy for rendering
     */
    class RenderBone : public std::enable_shared_from_this<RenderBone> {
    public:
        RenderBone(const std::string& name = "", int32_t index = -1);
        ~RenderBone() = default;

        // Basic properties
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        void SetIndex(int32_t index) { m_index = index; }
        int32_t GetIndex() const { return m_index; }

        // Hierarchy management
        void SetParent(std::shared_ptr<RenderBone> parent);
        std::shared_ptr<RenderBone> GetParent() const { return m_parent.lock(); }
        
        void AddChild(std::shared_ptr<RenderBone> child);
        void RemoveChild(std::shared_ptr<RenderBone> child);
        const std::vector<std::shared_ptr<RenderBone>>& GetChildren() const { return m_children; }

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
        std::weak_ptr<RenderBone> m_parent;
        std::vector<std::shared_ptr<RenderBone>> m_children;

        // Transforms
        Math::Mat4 m_localTransform = Math::Mat4(1.0f);
        Math::Mat4 m_worldTransform = Math::Mat4(1.0f);
        Math::Mat4 m_inverseBindMatrix = Math::Mat4(1.0f);
    };

    /**
     * @brief Skeletal system for bone-based rendering
     */
    class RenderSkeleton {
    public:
        RenderSkeleton() = default;
        ~RenderSkeleton() = default;

        // Bone management
        void AddBone(std::shared_ptr<RenderBone> bone);
        void SetBones(const std::vector<std::shared_ptr<RenderBone>>& bones);
        const std::vector<std::shared_ptr<RenderBone>>& GetBones() const { return m_bones; }

        std::shared_ptr<RenderBone> GetBone(size_t index) const;
        std::shared_ptr<RenderBone> FindBone(const std::string& name) const;
        size_t GetBoneCount() const { return m_bones.size(); }

        // Root bone management
        void SetRootBone(std::shared_ptr<RenderBone> root) { m_rootBone = root; }
        std::shared_ptr<RenderBone> GetRootBone() const { return m_rootBone; }

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
        std::vector<std::shared_ptr<RenderBone>> m_bones;
        std::shared_ptr<RenderBone> m_rootBone;
        std::unordered_map<std::string, std::shared_ptr<RenderBone>> m_boneMap;

        // Cached bone matrices for GPU upload
        mutable std::vector<Math::Mat4> m_boneMatrices;
        mutable bool m_matricesDirty = true;

        void BuildBoneMap();
        void PrintBoneHierarchy(std::shared_ptr<RenderBone> bone, size_t depth = 0) const;
    };

    /**
     * @brief Skin binding information for meshes
     */
    class RenderSkin {
    public:
        RenderSkin() = default;
        ~RenderSkin() = default;

        void SetSkeleton(std::shared_ptr<RenderSkeleton> skeleton) { m_skeleton = skeleton; }
        std::shared_ptr<RenderSkeleton> GetSkeleton() const { return m_skeleton; }

        void SetInverseBindMatrices(const std::vector<Math::Mat4>& matrices) { m_inverseBindMatrices = matrices; }
        const std::vector<Math::Mat4>& GetInverseBindMatrices() const { return m_inverseBindMatrices; }

        void SetJoints(const std::vector<uint32_t>& joints) { m_joints = joints; }
        const std::vector<uint32_t>& GetJoints() const { return m_joints; }

        // Get final bone matrices for skinning
        std::vector<Math::Mat4> GetSkinningMatrices() const;

        // Validation
        bool IsValid() const;

    private:
        std::shared_ptr<RenderSkeleton> m_skeleton;
        std::vector<Math::Mat4> m_inverseBindMatrices;
        std::vector<uint32_t> m_joints; // Indices into skeleton bones
    };

} // namespace Graphics
} // namespace GameEngine