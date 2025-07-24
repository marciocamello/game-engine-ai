#pragma once

#include "Core/Math.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace GameEngine {
    struct BoundingBox {
        Math::Vec3 min = Math::Vec3(0.0f);
        Math::Vec3 max = Math::Vec3(0.0f);
        
        BoundingBox() = default;
        BoundingBox(const Math::Vec3& minPoint, const Math::Vec3& maxPoint) 
            : min(minPoint), max(maxPoint) {}
        
        Math::Vec3 GetCenter() const { return (min + max) * 0.5f; }
        Math::Vec3 GetSize() const { return max - min; }
        bool IsValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }
        
        void Expand(const Math::Vec3& point) {
            if (!IsValid()) {
                min = max = point;
            } else {
                min = Math::Vec3(std::min(min.x, point.x), std::min(min.y, point.y), std::min(min.z, point.z));
                max = Math::Vec3(std::max(max.x, point.x), std::max(max.y, point.y), std::max(max.z, point.z));
            }
        }
        
        void Expand(const BoundingBox& other) {
            if (other.IsValid()) {
                Expand(other.min);
                Expand(other.max);
            }
        }
        
        BoundingBox Transform(const Math::Mat4& transform) const {
            if (!IsValid()) return *this;
            
            BoundingBox result;
            
            // Transform all 8 corners of the bounding box
            Math::Vec3 corners[8] = {
                Math::Vec3(min.x, min.y, min.z),
                Math::Vec3(max.x, min.y, min.z),
                Math::Vec3(min.x, max.y, min.z),
                Math::Vec3(max.x, max.y, min.z),
                Math::Vec3(min.x, min.y, max.z),
                Math::Vec3(max.x, min.y, max.z),
                Math::Vec3(min.x, max.y, max.z),
                Math::Vec3(max.x, max.y, max.z)
            };
            
            for (int i = 0; i < 8; ++i) {
                Math::Vec4 transformedCorner = transform * Math::Vec4(corners[i], 1.0f);
                result.Expand(Math::Vec3(transformedCorner));
            }
            
            return result;
        }
    };

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