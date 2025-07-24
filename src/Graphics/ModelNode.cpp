#include "Graphics/ModelNode.h"
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

    void ModelNode::SetLocalBounds(const BoundingBox& bounds) {
        m_localBounds = bounds;
    }
}