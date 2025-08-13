#include "Animation/Bone.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
namespace Animation {

    Bone::Bone(const std::string& name, int32_t id)
        : m_name(name), m_id(id) {
        // Initialize with identity matrices
        m_localTransform = Math::Mat4(1.0f);
        m_worldTransform = Math::Mat4(1.0f);
        m_bindPose = Math::Mat4(1.0f);
        m_inverseBindPose = Math::Mat4(1.0f);
    }

    void Bone::SetParent(std::shared_ptr<Bone> parent) {
        // Remove from old parent if exists
        if (auto oldParent = m_parent.lock()) {
            oldParent->RemoveChild(shared_from_this());
        }

        // Set new parent
        m_parent = parent;

        // Add to new parent's children
        if (parent) {
            parent->AddChild(shared_from_this());
        }
    }

    void Bone::AddChild(std::shared_ptr<Bone> child) {
        if (!child) {
            LOG_WARNING("Attempted to add null child to bone: " + m_name);
            return;
        }

        // Check if child is already in the list
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it == m_children.end()) {
            m_children.push_back(child);
            child->m_parent = shared_from_this();
        }
    }

    void Bone::RemoveChild(std::shared_ptr<Bone> child) {
        if (!child) {
            return;
        }

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end()) {
            m_children.erase(it);
            child->m_parent.reset();
        }
    }

    void Bone::SetLocalPosition(const Math::Vec3& position) {
        Math::Vec3 currentPos, currentScale;
        Math::Quat currentRot;
        DecomposeTransform(m_localTransform, currentPos, currentRot, currentScale);
        m_localTransform = ComposeTransform(position, currentRot, currentScale);
    }

    void Bone::SetLocalRotation(const Math::Quat& rotation) {
        Math::Vec3 currentPos, currentScale;
        Math::Quat currentRot;
        DecomposeTransform(m_localTransform, currentPos, currentRot, currentScale);
        m_localTransform = ComposeTransform(currentPos, rotation, currentScale);
    }

    void Bone::SetLocalScale(const Math::Vec3& scale) {
        Math::Vec3 currentPos, currentScale;
        Math::Quat currentRot;
        DecomposeTransform(m_localTransform, currentPos, currentRot, currentScale);
        m_localTransform = ComposeTransform(currentPos, currentRot, scale);
    }

    void Bone::SetLocalTransform(const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale) {
        m_localTransform = ComposeTransform(position, rotation, scale);
    }

    Math::Vec3 Bone::GetLocalPosition() const {
        Math::Vec3 position, scale;
        Math::Quat rotation;
        DecomposeTransform(m_localTransform, position, rotation, scale);
        return position;
    }

    Math::Quat Bone::GetLocalRotation() const {
        Math::Vec3 position, scale;
        Math::Quat rotation;
        DecomposeTransform(m_localTransform, position, rotation, scale);
        return rotation;
    }

    Math::Vec3 Bone::GetLocalScale() const {
        Math::Vec3 position, scale;
        Math::Quat rotation;
        DecomposeTransform(m_localTransform, position, rotation, scale);
        return scale;
    }

    Math::Vec3 Bone::GetWorldPosition() const {
        return Math::Vec3(m_worldTransform[3]);
    }

    Math::Quat Bone::GetWorldRotation() const {
        Math::Vec3 position, scale;
        Math::Quat rotation;
        DecomposeTransform(m_worldTransform, position, rotation, scale);
        return rotation;
    }

    Math::Mat4 Bone::GetSkinningMatrix() const {
        // Skinning matrix = current world transform * inverse bind pose
        return m_worldTransform * m_inverseBindPose;
    }

    void Bone::CalculateWorldTransform() {
        Math::Mat4 parentTransform(1.0f);
        if (auto parent = m_parent.lock()) {
            parentTransform = parent->GetWorldTransform();
        }
        CalculateWorldTransform(parentTransform);
    }

    void Bone::CalculateWorldTransform(const Math::Mat4& parentWorldTransform) {
        m_worldTransform = parentWorldTransform * m_localTransform;
    }

    void Bone::DecomposeTransform(const Math::Mat4& transform, Math::Vec3& position, Math::Quat& rotation, Math::Vec3& scale) {
        // Extract position
        position = Math::Vec3(transform[3]);

        // Extract scale
        Math::Vec3 col0(transform[0]);
        Math::Vec3 col1(transform[1]);
        Math::Vec3 col2(transform[2]);
        
        scale.x = glm::length(col0);
        scale.y = glm::length(col1);
        scale.z = glm::length(col2);

        // Remove scaling from the matrix
        if (scale.x != 0) col0 /= scale.x;
        if (scale.y != 0) col1 /= scale.y;
        if (scale.z != 0) col2 /= scale.z;

        // Extract rotation
        Math::Mat3 rotationMatrix(col0, col1, col2);
        rotation = glm::quat_cast(rotationMatrix);
    }

    Math::Mat4 Bone::ComposeTransform(const Math::Vec3& position, const Math::Quat& rotation, const Math::Vec3& scale) {
        Math::Mat4 translationMatrix = glm::translate(Math::Mat4(1.0f), position);
        Math::Mat4 rotationMatrix = glm::mat4_cast(rotation);
        Math::Mat4 scaleMatrix = glm::scale(Math::Mat4(1.0f), scale);
        
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    int32_t Bone::GetDepth() const {
        int32_t depth = 0;
        auto parent = m_parent.lock();
        while (parent) {
            depth++;
            parent = parent->GetParent();
        }
        return depth;
    }

    std::vector<std::shared_ptr<Bone>> Bone::GetDescendants() const {
        std::vector<std::shared_ptr<Bone>> descendants;
        
        // Add all children and their descendants recursively
        for (const auto& child : m_children) {
            descendants.push_back(child);
            
            // Get child's descendants
            auto childDescendants = child->GetDescendants();
            descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
        }
        
        return descendants;
    }

} // namespace Animation
} // namespace GameEngine