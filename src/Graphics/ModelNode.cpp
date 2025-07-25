#include "Graphics/ModelNode.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <algorithm>
#include <queue>

namespace GameEngine {
    ModelNode::ModelNode(const std::string& name) 
        : m_name(name) {
    }

    ModelNode::~ModelNode() {
        // Clear children to break circular references
        m_children.clear();
    }

    void ModelNode::AddChild(std::shared_ptr<ModelNode> child) {
        if (!child) {
            LOG_WARNING("Attempted to add null child to ModelNode: " + m_name);
            return;
        }

        // Check if child is already in our children list
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end()) {
            LOG_WARNING("Child '" + child->GetName() + "' is already a child of '" + m_name + "'");
            return;
        }

        // Remove child from its current parent if it has one
        if (auto currentParent = child->GetParent()) {
            currentParent->RemoveChild(child);
        }

        // Add child and set parent relationship
        m_children.push_back(child);
        child->m_parent = shared_from_this();

        // Update child's world transform
        child->UpdateWorldTransform(m_worldTransform);

        LOG_DEBUG("Added child '" + child->GetName() + "' to node '" + m_name + "'");
    }

    void ModelNode::RemoveChild(std::shared_ptr<ModelNode> child) {
        if (!child) {
            return;
        }

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end()) {
            // Clear parent relationship
            child->m_parent.reset();
            
            // Remove from children list
            m_children.erase(it);
            
            LOG_DEBUG("Removed child '" + child->GetName() + "' from node '" + m_name + "'");
        }
    }

    std::vector<std::shared_ptr<ModelNode>> ModelNode::GetChildren() const {
        return m_children;
    }

    std::shared_ptr<ModelNode> ModelNode::GetParent() const {
        return m_parent.lock();
    }

    std::shared_ptr<ModelNode> ModelNode::FindChild(const std::string& name) const {
        // Search direct children first
        for (const auto& child : m_children) {
            if (child->GetName() == name) {
                return child;
            }
        }

        // Search recursively in grandchildren
        for (const auto& child : m_children) {
            if (auto found = child->FindChild(name)) {
                return found;
            }
        }

        return nullptr;
    }

    void ModelNode::SetLocalTransform(const Math::Mat4& transform) {
        m_localTransform = transform;
        
        // Update world transform based on parent
        if (auto parent = GetParent()) {
            UpdateWorldTransform(parent->GetWorldTransform());
        } else {
            UpdateWorldTransform();
        }
    }

    Math::Mat4 ModelNode::GetLocalTransform() const {
        return m_localTransform;
    }

    Math::Mat4 ModelNode::GetWorldTransform() const {
        return m_worldTransform;
    }

    void ModelNode::UpdateWorldTransform(const Math::Mat4& parentTransform) {
        m_worldTransform = parentTransform * m_localTransform;
        UpdateChildTransforms();
    }

    void ModelNode::UpdateChildTransforms() {
        for (auto& child : m_children) {
            child->UpdateWorldTransform(m_worldTransform);
        }
    }

    void ModelNode::AddMeshIndex(uint32_t meshIndex) {
        // Check if mesh index is already added
        auto it = std::find(m_meshIndices.begin(), m_meshIndices.end(), meshIndex);
        if (it == m_meshIndices.end()) {
            m_meshIndices.push_back(meshIndex);
            LOG_DEBUG("Added mesh index " + std::to_string(meshIndex) + " to node '" + m_name + "'");
        }
    }

    void ModelNode::RemoveMeshIndex(uint32_t meshIndex) {
        auto it = std::find(m_meshIndices.begin(), m_meshIndices.end(), meshIndex);
        if (it != m_meshIndices.end()) {
            m_meshIndices.erase(it);
            LOG_DEBUG("Removed mesh index " + std::to_string(meshIndex) + " from node '" + m_name + "'");
        }
    }

    std::vector<uint32_t> ModelNode::GetMeshIndices() const {
        return m_meshIndices;
    }

    bool ModelNode::HasMeshes() const {
        return !m_meshIndices.empty();
    }

    void ModelNode::SetName(const std::string& name) {
        m_name = name;
    }

    const std::string& ModelNode::GetName() const {
        return m_name;
    }

    void ModelNode::SetVisible(bool visible) {
        m_visible = visible;
    }

    bool ModelNode::IsVisible() const {
        return m_visible;
    }

    void ModelNode::Traverse(std::function<void(std::shared_ptr<ModelNode>)> callback) {
        TraverseDepthFirst(callback);
    }

    void ModelNode::TraverseDepthFirst(std::function<void(std::shared_ptr<ModelNode>)> callback) {
        if (!callback) {
            return;
        }

        // Visit this node
        callback(shared_from_this());

        // Visit children recursively
        for (auto& child : m_children) {
            child->TraverseDepthFirst(callback);
        }
    }

    void ModelNode::TraverseBreadthFirst(std::function<void(std::shared_ptr<ModelNode>)> callback) {
        if (!callback) {
            return;
        }

        std::queue<std::shared_ptr<ModelNode>> nodeQueue;
        nodeQueue.push(shared_from_this());

        while (!nodeQueue.empty()) {
            auto currentNode = nodeQueue.front();
            nodeQueue.pop();

            // Visit current node
            callback(currentNode);

            // Add children to queue
            for (auto& child : currentNode->m_children) {
                nodeQueue.push(child);
            }
        }
    }

    BoundingBox ModelNode::GetLocalBounds() const {
        return m_localBounds;
    }

    BoundingBox ModelNode::GetWorldBounds() const {
        return m_localBounds.Transform(m_worldTransform);
    }

    BoundingSphere ModelNode::GetLocalBoundingSphere() const {
        return m_localBoundingSphere;
    }

    BoundingSphere ModelNode::GetWorldBoundingSphere() const {
        // Transform sphere center to world space
        Math::Vec4 worldCenter = m_worldTransform * Math::Vec4(m_localBoundingSphere.center, 1.0f);
        
        // Calculate scale factor from transform matrix
        Math::Vec3 scale = Math::Vec3(
            glm::length(Math::Vec3(m_worldTransform[0])),
            glm::length(Math::Vec3(m_worldTransform[1])),
            glm::length(Math::Vec3(m_worldTransform[2]))
        );
        float maxScale = std::max({scale.x, scale.y, scale.z});
        
        return BoundingSphere(Math::Vec3(worldCenter), m_localBoundingSphere.radius * maxScale);
    }

    void ModelNode::SetLocalBounds(const BoundingBox& bounds) {
        m_localBounds = bounds;
    }

    void ModelNode::SetLocalBoundingSphere(const BoundingSphere& sphere) {
        m_localBoundingSphere = sphere;
    }

    void ModelNode::CalculateHierarchicalBounds(const std::vector<std::shared_ptr<Mesh>>& meshes) {
        // Calculate bounds from meshes associated with this node
        m_localBounds = CalculateCombinedBounds(meshes);
        m_localBoundingSphere = CalculateCombinedBoundingSphere(meshes);
        
        // Expand bounds to include all child nodes
        for (auto& child : m_children) {
            child->CalculateHierarchicalBounds(meshes);
            
            // Transform child bounds to this node's local space
            Math::Mat4 childToLocal = glm::inverse(m_localTransform) * child->GetLocalTransform();
            BoundingBox childBounds = child->GetLocalBounds().Transform(childToLocal);
            m_localBounds.Expand(childBounds);
            
            // For sphere, we need to transform and expand
            BoundingSphere childSphere = child->GetLocalBoundingSphere();
            if (childSphere.IsValid()) {
                Math::Vec4 transformedCenter = childToLocal * Math::Vec4(childSphere.center, 1.0f);
                
                // Calculate scale factor
                Math::Vec3 scale = Math::Vec3(
                    glm::length(Math::Vec3(childToLocal[0])),
                    glm::length(Math::Vec3(childToLocal[1])),
                    glm::length(Math::Vec3(childToLocal[2]))
                );
                float maxScale = std::max({scale.x, scale.y, scale.z});
                
                BoundingSphere transformedSphere(Math::Vec3(transformedCenter), childSphere.radius * maxScale);
                m_localBoundingSphere.Expand(transformedSphere);
            }
        }
    }

    BoundingBox ModelNode::CalculateCombinedBounds(const std::vector<std::shared_ptr<Mesh>>& meshes) const {
        BoundingBox combinedBounds;
        
        // Combine bounds from all meshes associated with this node
        for (uint32_t meshIndex : m_meshIndices) {
            if (meshIndex < meshes.size() && meshes[meshIndex]) {
                combinedBounds.Expand(meshes[meshIndex]->GetBoundingBox());
            }
        }
        
        return combinedBounds;
    }

    BoundingSphere ModelNode::CalculateCombinedBoundingSphere(const std::vector<std::shared_ptr<Mesh>>& meshes) const {
        BoundingSphere combinedSphere;
        
        // Combine bounding spheres from all meshes associated with this node
        for (uint32_t meshIndex : m_meshIndices) {
            if (meshIndex < meshes.size() && meshes[meshIndex]) {
                combinedSphere.Expand(meshes[meshIndex]->GetBoundingSphere());
            }
        }
        
        return combinedSphere;
    }

    void ModelNode::UpdateAnimatedBounds(const std::vector<std::shared_ptr<Mesh>>& meshes, float animationTime) {
        // For now, we'll use the static bounds as animated bounds
        // In a full implementation, this would calculate bounds based on animated vertex positions
        m_lastAnimationTime = animationTime;
        m_cachedAnimatedBounds = CalculateCombinedBounds(meshes);
        m_cachedAnimatedSphere = CalculateCombinedBoundingSphere(meshes);
        
        // Update child nodes
        for (auto& child : m_children) {
            child->UpdateAnimatedBounds(meshes, animationTime);
        }
    }

    BoundingBox ModelNode::GetAnimatedBounds(float animationTime) const {
        // Check if we have cached bounds for this time
        if (std::abs(m_lastAnimationTime - animationTime) < 0.001f) {
            return m_cachedAnimatedBounds;
        }
        
        // Check animated bounds cache
        for (const auto& entry : m_animatedBoundsCache) {
            if (std::abs(entry.first - animationTime) < 0.001f) {
                return entry.second;
            }
        }
        
        // Interpolate between cached values if available
        if (m_animatedBoundsCache.size() >= 2) {
            // Find the two closest time entries
            float t1 = -1.0f, t2 = -1.0f;
            BoundingBox b1, b2;
            
            for (size_t i = 0; i < m_animatedBoundsCache.size() - 1; ++i) {
                if (m_animatedBoundsCache[i].first <= animationTime && 
                    m_animatedBoundsCache[i + 1].first >= animationTime) {
                    t1 = m_animatedBoundsCache[i].first;
                    t2 = m_animatedBoundsCache[i + 1].first;
                    b1 = m_animatedBoundsCache[i].second;
                    b2 = m_animatedBoundsCache[i + 1].second;
                    break;
                }
            }
            
            if (t1 >= 0.0f && t2 >= 0.0f && t2 > t1) {
                // Linear interpolation between bounds
                float alpha = (animationTime - t1) / (t2 - t1);
                BoundingBox interpolated;
                interpolated.min = b1.min + alpha * (b2.min - b1.min);
                interpolated.max = b1.max + alpha * (b2.max - b1.max);
                return interpolated;
            }
        }
        
        // Fallback to static bounds
        return m_localBounds;
    }

    BoundingSphere ModelNode::GetAnimatedBoundingSphere(float animationTime) const {
        // Check if we have cached sphere for this time
        if (std::abs(m_lastAnimationTime - animationTime) < 0.001f) {
            return m_cachedAnimatedSphere;
        }
        
        // Check animated sphere cache
        for (const auto& entry : m_animatedSphereCache) {
            if (std::abs(entry.first - animationTime) < 0.001f) {
                return entry.second;
            }
        }
        
        // Interpolate between cached values if available
        if (m_animatedSphereCache.size() >= 2) {
            // Find the two closest time entries
            float t1 = -1.0f, t2 = -1.0f;
            BoundingSphere s1, s2;
            
            for (size_t i = 0; i < m_animatedSphereCache.size() - 1; ++i) {
                if (m_animatedSphereCache[i].first <= animationTime && 
                    m_animatedSphereCache[i + 1].first >= animationTime) {
                    t1 = m_animatedSphereCache[i].first;
                    t2 = m_animatedSphereCache[i + 1].first;
                    s1 = m_animatedSphereCache[i].second;
                    s2 = m_animatedSphereCache[i + 1].second;
                    break;
                }
            }
            
            if (t1 >= 0.0f && t2 >= 0.0f && t2 > t1) {
                // Linear interpolation between spheres
                float alpha = (animationTime - t1) / (t2 - t1);
                BoundingSphere interpolated;
                interpolated.center = s1.center + alpha * (s2.center - s1.center);
                interpolated.radius = s1.radius + alpha * (s2.radius - s1.radius);
                return interpolated;
            }
        }
        
        // Fallback to static sphere
        return m_localBoundingSphere;
    }

    void ModelNode::SetAnimatedBoundsCache(const std::vector<std::pair<float, BoundingBox>>& boundsCache) {
        m_animatedBoundsCache = boundsCache;
        // Sort by time for efficient lookup
        std::sort(m_animatedBoundsCache.begin(), m_animatedBoundsCache.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
    }

    void ModelNode::SetAnimatedSphereCache(const std::vector<std::pair<float, BoundingSphere>>& sphereCache) {
        m_animatedSphereCache = sphereCache;
        // Sort by time for efficient lookup
        std::sort(m_animatedSphereCache.begin(), m_animatedSphereCache.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
    }
}