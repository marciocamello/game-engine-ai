#pragma once

#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>

namespace GameEngine {
namespace Animation {

    /**
     * Represents a single bone in a skeletal hierarchy
     * Each bone has a transform, parent-child relationships, and bind pose information
     */
    class Bone {
    public:
        Bone(const std::string& name, int32_t id);
        ~Bone() = default;

        // Basic properties
        const std::string& GetName() const { return m_name; }
        int32_t GetId() const { return m_id; }

        // Hierarchy management
        void SetParent(std::shared_ptr<Bone> parent);
        std::shared_ptr<Bone> GetParent() const { return m_parent.lock(); }
        void AddChild(std::shared_ptr<Bone> child);
        void RemoveChild(std::shared_ptr<Bone> child);
        const std::vector<std::shared_ptr<Bone>>& GetChildren() const { return m_children; }
        bool HasChildren() const { return !m_children.empty(); }

        // Transform management
        void SetLocalTransform(const Math::Mat4& transform) { m_localTransform = transform; }
        const Math::Mat4& GetLocalTransform() const { return m_localTransform; }
        
        void SetWorldTransform(const Math::Mat4& transform) { m_worldTransform = transform; }
        const Math::Mat4& GetWorldTransform() const { return m_worldTransform; }

        // Component-based transform setters
        void SetLocalPosition(const Math::Vec3& position);
        void SetLocalRotation(const Math::Quat& rotation);
        void SetLocalScale(const Math::Vec3& scale);
        void SetLocalTransform(const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale);

        // Component-based transform getters
        Math::Vec3 GetLocalPosition() const;
        Math::Quat GetLocalRotation() const;
        Math::Vec3 GetLocalScale() const;
        Math::Vec3 GetWorldPosition() const;
        Math::Quat GetWorldRotation() const;

        // Bind pose (rest pose) management
        void SetBindPose(const Math::Mat4& bindPose) { m_bindPose = bindPose; }
        const Math::Mat4& GetBindPose() const { return m_bindPose; }
        
        void SetInverseBindPose(const Math::Mat4& inverseBindPose) { m_inverseBindPose = inverseBindPose; }
        const Math::Mat4& GetInverseBindPose() const { return m_inverseBindPose; }

        // Skinning matrix calculation
        Math::Mat4 GetSkinningMatrix() const;
        
        // Transform calculation utilities
        void CalculateWorldTransform();
        void CalculateWorldTransform(const Math::Mat4& parentWorldTransform);
        
        // Transform decomposition utilities
        static void DecomposeTransform(const Math::Mat4& transform, Math::Vec3& position, Math::Quat& rotation, Math::Vec3& scale);
        static Math::Mat4 ComposeTransform(const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale);

        // Utility methods
        bool IsRoot() const { return m_parent.expired(); }
        int32_t GetDepth() const;
        std::vector<std::shared_ptr<Bone>> GetDescendants() const;

    private:
        std::string m_name;
        int32_t m_id;

        // Hierarchy
        std::weak_ptr<Bone> m_parent;
        std::vector<std::shared_ptr<Bone>> m_children;

        // Transforms
        Math::Mat4 m_localTransform{1.0f};  // Transform relative to parent
        Math::Mat4 m_worldTransform{1.0f};  // Transform in world space
        Math::Mat4 m_bindPose{1.0f};        // Rest pose transform
        Math::Mat4 m_inverseBindPose{1.0f}; // Inverse of bind pose for skinning
    };

} // namespace Animation
} // namespace GameEngine