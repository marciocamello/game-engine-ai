#pragma once

#include "Core/Math.h"
#include "Graphics/BoundingVolumes.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace GameEngine {

    class ModelNode : public std::enable_shared_from_this<ModelNode> {
    public:
        // Lifecycle
        ModelNode(const std::string& name = "");
        ~ModelNode();

        // Hierarchy management
        void AddChild(std::shared_ptr<ModelNode> child);
        void RemoveChild(std::shared_ptr<ModelNode> child);
        std::vector<std::shared_ptr<ModelNode>> GetChildren() const;
        std::shared_ptr<ModelNode> GetParent() const;
        std::shared_ptr<ModelNode> FindChild(const std::string& name) const;

        // Transform management
        void SetLocalTransform(const Math::Mat4& transform);
        Math::Mat4 GetLocalTransform() const;
        Math::Mat4 GetWorldTransform() const;
        void UpdateWorldTransform(const Math::Mat4& parentTransform = Math::Mat4(1.0f));

        // Mesh association
        void AddMeshIndex(uint32_t meshIndex);
        void RemoveMeshIndex(uint32_t meshIndex);
        std::vector<uint32_t> GetMeshIndices() const;
        bool HasMeshes() const;

        // Properties
        void SetName(const std::string& name);
        const std::string& GetName() const;
        void SetVisible(bool visible);
        bool IsVisible() const;

        // Traversal
        void Traverse(std::function<void(std::shared_ptr<ModelNode>)> callback);
        void TraverseDepthFirst(std::function<void(std::shared_ptr<ModelNode>)> callback);
        void TraverseBreadthFirst(std::function<void(std::shared_ptr<ModelNode>)> callback);

        // Bounding information
        BoundingBox GetLocalBounds() const;
        BoundingBox GetWorldBounds() const;
        void SetLocalBounds(const BoundingBox& bounds);

    private:
        std::string m_name;
        Math::Mat4 m_localTransform = Math::Mat4(1.0f);
        Math::Mat4 m_worldTransform = Math::Mat4(1.0f);

        std::vector<uint32_t> m_meshIndices;
        std::vector<std::shared_ptr<ModelNode>> m_children;
        std::weak_ptr<ModelNode> m_parent;

        bool m_visible = true;
        BoundingBox m_localBounds;

        void UpdateChildTransforms();
    };
}