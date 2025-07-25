#pragma once

#include "Resource/ResourceManager.h"
#include "Graphics/BoundingVolumes.h"
#include "Core/Math.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace GameEngine {
    class Shader;
    class Animation;
    class Skeleton;
    class Skin;
    class ModelNode;
    class Mesh;
    class Material;

    struct ModelStats {
        uint32_t nodeCount = 0;
        uint32_t meshCount = 0;
        uint32_t materialCount = 0;
        uint32_t textureCount = 0;
        uint32_t animationCount = 0;
        uint32_t totalVertices = 0;
        uint32_t totalTriangles = 0;
        size_t totalMemoryUsage = 0;
        float loadingTimeMs = 0.0f;
        std::string formatUsed;
    };

    class Model : public Resource {
    public:
        // Lifecycle
        Model(const std::string& filepath);
        ~Model();

        // Loading methods
        bool LoadFromFile(const std::string& filepath) override;
        void CreateDefault() override; // Creates default model with a cube

        // Scene graph access
        std::shared_ptr<ModelNode> GetRootNode() const;
        std::shared_ptr<ModelNode> FindNode(const std::string& name) const;
        std::vector<std::shared_ptr<ModelNode>> GetAllNodes() const;

        // Mesh access
        std::vector<std::shared_ptr<Mesh>> GetMeshes() const;
        std::shared_ptr<Mesh> GetMesh(size_t index) const;
        std::shared_ptr<Mesh> FindMesh(const std::string& name) const;
        size_t GetMeshCount() const;

        // Material access
        std::vector<std::shared_ptr<Material>> GetMaterials() const;
        std::shared_ptr<Material> GetMaterial(size_t index) const;
        std::shared_ptr<Material> FindMaterial(const std::string& name) const;
        size_t GetMaterialCount() const;

        // Animation access
        bool HasAnimations() const;
        std::vector<std::shared_ptr<Animation>> GetAnimations() const;
        std::shared_ptr<Animation> GetAnimation(size_t index) const;
        std::shared_ptr<Animation> FindAnimation(const std::string& name) const;
        size_t GetAnimationCount() const;
        void AddAnimation(std::shared_ptr<Animation> animation);
        void SetAnimations(const std::vector<std::shared_ptr<Animation>>& animations);

        // Skeleton access
        std::shared_ptr<Skeleton> GetSkeleton() const;
        bool HasSkeleton() const;
        void SetSkeleton(std::shared_ptr<Skeleton> skeleton);
        
        // Skin access
        std::vector<std::shared_ptr<Skin>> GetSkins() const;
        std::shared_ptr<Skin> GetSkin(size_t index) const;
        size_t GetSkinCount() const;
        void AddSkin(std::shared_ptr<Skin> skin);
        void SetSkins(const std::vector<std::shared_ptr<Skin>>& skins);

        // Rendering
        void Render(const Math::Mat4& transform, std::shared_ptr<Shader> shader);
        void RenderNode(std::shared_ptr<ModelNode> node, const Math::Mat4& parentTransform, std::shared_ptr<Shader> shader);
        void RenderInstanced(const std::vector<Math::Mat4>& transforms, std::shared_ptr<Shader> shader);

        // Bounding information
        BoundingBox GetBoundingBox() const;
        BoundingSphere GetBoundingSphere() const;
        void UpdateBounds();
        
        // Animated bounding volume support
        BoundingBox GetAnimatedBoundingBox(float animationTime) const;
        BoundingSphere GetAnimatedBoundingSphere(float animationTime) const;
        void UpdateAnimatedBounds(float animationTime);
        void PrecomputeAnimatedBounds(float startTime, float endTime, float timeStep);

        // LOD support (placeholder for future implementation)
        void SetLODLevels(const std::vector<std::shared_ptr<Model>>& lodLevels);
        std::shared_ptr<Model> GetLOD(float distance) const;
        size_t GetLODCount() const;

        // Statistics and debugging
        ModelStats GetStats() const;
        void PrintDebugInfo() const;

        // Serialization (placeholder for future implementation)
        bool SaveToCache(const std::string& cachePath) const;
        bool LoadFromCache(const std::string& cachePath);

        // Resource interface
        size_t GetMemoryUsage() const override;

        // Name management
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        // Model building methods (for loaders)
        void AddMesh(std::shared_ptr<Mesh> mesh);
        void AddMaterial(std::shared_ptr<Material> material);
        void SetMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes);
        void SetMaterials(const std::vector<std::shared_ptr<Material>>& materials);

    private:
        std::string m_name;
        std::shared_ptr<ModelNode> m_rootNode;
        std::vector<std::shared_ptr<Mesh>> m_meshes;
        std::vector<std::shared_ptr<Material>> m_materials;
        std::vector<std::shared_ptr<Animation>> m_animations;
        std::shared_ptr<Skeleton> m_skeleton;
        std::vector<std::shared_ptr<Skin>> m_skins;
        std::vector<std::shared_ptr<Model>> m_lodLevels; // Placeholder

        BoundingBox m_boundingBox;
        BoundingSphere m_boundingSphere;
        ModelStats m_stats;
        
        // Animated bounding volume cache
        mutable float m_lastAnimationTime = -1.0f;
        mutable BoundingBox m_cachedAnimatedBoundingBox;
        mutable BoundingSphere m_cachedAnimatedBoundingSphere;

        // Name-based lookup maps for performance
        std::unordered_map<std::string, std::shared_ptr<ModelNode>> m_nodeMap;
        std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshMap;
        std::unordered_map<std::string, std::shared_ptr<Material>> m_materialMap;
        std::unordered_map<std::string, std::shared_ptr<Animation>> m_animationMap;

        // Helper methods
        void CalculateBounds();
        void OptimizeMeshes();
        void ValidateModel();
        void BuildNodeMap();
        void BuildMeshMap();
        void BuildMaterialMap();
        void BuildAnimationMap();
        void CollectAllNodes(std::shared_ptr<ModelNode> node, std::vector<std::shared_ptr<ModelNode>>& nodes) const;
    };
}