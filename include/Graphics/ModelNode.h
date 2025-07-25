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
        BoundingSphere GetLocalBoundingSphere() const;
        BoundingSphere GetWorldBoundingSphere() const;
        void SetLocalBounds(const BoundingBox& bounds);
        void SetLocalBoundingSphere(const BoundingSphere& sphere);
        
        // Hierarchical bounding volume calculation
        void CalculateHierarchicalBounds(const std::vector<std::shared_ptr<class Mesh>>& meshes);
        BoundingBox CalculateCombinedBounds(const std::vector<std::shared_ptr<class Mesh>>& meshes) const;
        BoundingSphere CalculateCombinedBoundingSphere(const std::vector<std::shared_ptr<class Mesh>>& meshes) const;
        
        // Animated bounding volume support
        void UpdateAnimatedBounds(const std::vector<std::shared_ptr<class Mesh>>& meshes, float animationTime);
        BoundingBox GetAnimatedBounds(float animationTime) const;
        BoundingSphere GetAnimatedBoundingSphere(float animationTime) const;
        void SetAnimatedBoundsCache(const std::vector<std::pair<float, BoundingBox>>& boundsCache);
        void SetAnimatedSphereCache(const std::vector<std::pair<float, BoundingSphere>>& sphereCache);

    private:
        std::string m_name;
        Math::Mat4 m_localTransform = Math::Mat4(1.0f);
        Math::Mat4 m_worldTransform = Math::Mat4(1.0f);

        std::vector<uint32_t> m_meshIndices;
        std::vector<std::shared_ptr<ModelNode>> m_children;
        std::weak_ptr<ModelNode> m_parent;

        bool m_visible = true;
        BoundingBox m_localBounds;
        BoundingSphere m_localBoundingSphere;
        
        // Animated bounding volume caches
        std::vector<std::pair<float, BoundingBox>> m_animatedBoundsCache;
        std::vector<std::pair<float, BoundingSphere>> m_animatedSphereCache;
        mutable float m_lastAnimationTime = -1.0f;
        mutable BoundingBox m_cachedAnimatedBounds;
        mutable BoundingSphere m_cachedAnimatedSphere;

        void UpdateChildTransforms();
    };
}