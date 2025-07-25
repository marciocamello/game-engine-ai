#include "Graphics/Model.h"
#include "Graphics/ModelNode.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Animation.h"
#include "Graphics/Skeleton.h"
#include "Core/Logger.h"
#include <algorithm>
#include <chrono>

namespace GameEngine {
    Model::Model(const std::string& filepath) 
        : Resource(filepath), m_name("Model") {
        m_rootNode = std::make_shared<ModelNode>("Root");
    }

    Model::~Model() {
        // Clear all collections to break potential circular references
        m_meshes.clear();
        m_materials.clear();
        m_animations.clear();
        m_skins.clear();
        m_lodLevels.clear();
        m_nodeMap.clear();
        m_meshMap.clear();
        m_materialMap.clear();
        m_animationMap.clear();
        m_rootNode.reset();
    }

    bool Model::LoadFromFile(const std::string& filepath) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        LOG_INFO("Loading model from file: " + filepath);
        
        // For now, create a default model since we haven't implemented Assimp integration yet
        // This will be replaced with actual file loading in future tasks
        CreateDefault();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        m_stats.loadingTimeMs = static_cast<float>(duration.count());
        
        LOG_INFO("Model loaded successfully in " + std::to_string(m_stats.loadingTimeMs) + "ms");
        return true;
    }

    void Model::CreateDefault() {
        LOG_INFO("Creating default model");
        
        // Clear existing data
        m_meshes.clear();
        m_materials.clear();
        m_nodeMap.clear();
        m_meshMap.clear();
        m_materialMap.clear();
        
        // Create default mesh (cube)
        auto defaultMesh = std::make_shared<Mesh>("default_cube");
        defaultMesh->CreateDefault();
        m_meshes.push_back(defaultMesh);
        
        // Create default material
        auto defaultMaterial = std::make_shared<Material>();
        defaultMaterial->SetAlbedo(Math::Vec3(0.8f, 0.8f, 0.8f));
        defaultMaterial->SetMetallic(0.0f);
        defaultMaterial->SetRoughness(0.5f);
        m_materials.push_back(defaultMaterial);
        
        // Associate mesh with material
        defaultMesh->SetMaterial(defaultMaterial);
        
        // Set up root node with the mesh
        m_rootNode = std::make_shared<ModelNode>("Root");
        m_rootNode->AddMeshIndex(0); // First (and only) mesh
        
        // Build lookup maps
        BuildNodeMap();
        BuildMeshMap();
        BuildMaterialMap();
        
        // Calculate bounds
        CalculateBounds();
        
        // Update stats
        m_stats.nodeCount = 1;
        m_stats.meshCount = 1;
        m_stats.materialCount = 1;
        m_stats.totalVertices = defaultMesh->GetVertexCount();
        m_stats.totalTriangles = defaultMesh->GetTriangleCount();
        m_stats.totalMemoryUsage = GetMemoryUsage();
        m_stats.formatUsed = "Default";
        
        LOG_INFO("Default model created with " + std::to_string(m_stats.totalVertices) + " vertices and " + 
                std::to_string(m_stats.totalTriangles) + " triangles");
    }

    std::shared_ptr<ModelNode> Model::GetRootNode() const {
        return m_rootNode;
    }

    std::shared_ptr<ModelNode> Model::FindNode(const std::string& name) const {
        auto it = m_nodeMap.find(name);
        if (it != m_nodeMap.end()) {
            return it->second;
        }
        
        // Fallback to recursive search if not in map
        if (m_rootNode) {
            return m_rootNode->FindChild(name);
        }
        
        return nullptr;
    }

    std::vector<std::shared_ptr<ModelNode>> Model::GetAllNodes() const {
        std::vector<std::shared_ptr<ModelNode>> nodes;
        if (m_rootNode) {
            CollectAllNodes(m_rootNode, nodes);
        }
        return nodes;
    }

    void Model::CollectAllNodes(std::shared_ptr<ModelNode> node, std::vector<std::shared_ptr<ModelNode>>& nodes) const {
        if (!node) return;
        
        nodes.push_back(node);
        for (auto& child : node->GetChildren()) {
            CollectAllNodes(child, nodes);
        }
    }

    std::vector<std::shared_ptr<Mesh>> Model::GetMeshes() const {
        return m_meshes;
    }

    std::shared_ptr<Mesh> Model::GetMesh(size_t index) const {
        if (index < m_meshes.size()) {
            return m_meshes[index];
        }
        return nullptr;
    }

    std::shared_ptr<Mesh> Model::FindMesh(const std::string& name) const {
        auto it = m_meshMap.find(name);
        if (it != m_meshMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    size_t Model::GetMeshCount() const {
        return m_meshes.size();
    }

    std::vector<std::shared_ptr<Material>> Model::GetMaterials() const {
        return m_materials;
    }

    std::shared_ptr<Material> Model::GetMaterial(size_t index) const {
        if (index < m_materials.size()) {
            return m_materials[index];
        }
        return nullptr;
    }

    std::shared_ptr<Material> Model::FindMaterial(const std::string& name) const {
        auto it = m_materialMap.find(name);
        if (it != m_materialMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    size_t Model::GetMaterialCount() const {
        return m_materials.size();
    }

    // Animation methods (placeholders)
    bool Model::HasAnimations() const {
        return !m_animations.empty();
    }

    std::vector<std::shared_ptr<Animation>> Model::GetAnimations() const {
        return m_animations;
    }

    std::shared_ptr<Animation> Model::GetAnimation(size_t index) const {
        if (index < m_animations.size()) {
            return m_animations[index];
        }
        return nullptr;
    }

    std::shared_ptr<Animation> Model::FindAnimation(const std::string& name) const {
        auto it = m_animationMap.find(name);
        if (it != m_animationMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    size_t Model::GetAnimationCount() const {
        return m_animations.size();
    }

    void Model::AddAnimation(std::shared_ptr<Animation> animation) {
        if (animation) {
            m_animations.push_back(animation);
            BuildAnimationMap();
        }
    }

    void Model::SetAnimations(const std::vector<std::shared_ptr<Animation>>& animations) {
        m_animations = animations;
        BuildAnimationMap();
    }

    // Skeleton methods
    std::shared_ptr<Skeleton> Model::GetSkeleton() const {
        return m_skeleton;
    }

    bool Model::HasSkeleton() const {
        return m_skeleton != nullptr;
    }

    void Model::SetSkeleton(std::shared_ptr<Skeleton> skeleton) {
        m_skeleton = skeleton;
    }

    // Skin methods
    std::vector<std::shared_ptr<Skin>> Model::GetSkins() const {
        return m_skins;
    }

    std::shared_ptr<Skin> Model::GetSkin(size_t index) const {
        if (index < m_skins.size()) {
            return m_skins[index];
        }
        return nullptr;
    }

    size_t Model::GetSkinCount() const {
        return m_skins.size();
    }

    void Model::AddSkin(std::shared_ptr<Skin> skin) {
        if (skin) {
            m_skins.push_back(skin);
        }
    }

    void Model::SetSkins(const std::vector<std::shared_ptr<Skin>>& skins) {
        m_skins = skins;
    }

    void Model::Render(const Math::Mat4& transform, std::shared_ptr<Shader> shader) {
        if (!m_rootNode || !shader) {
            return;
        }
        
        RenderNode(m_rootNode, transform, shader);
    }

    void Model::RenderNode(std::shared_ptr<ModelNode> node, const Math::Mat4& parentTransform, std::shared_ptr<Shader> shader) {
        if (!node || !node->IsVisible() || !shader) {
            return;
        }
        
        Math::Mat4 nodeTransform = parentTransform * node->GetLocalTransform();
        
        // Render meshes associated with this node
        for (uint32_t meshIndex : node->GetMeshIndices()) {
            if (meshIndex < m_meshes.size()) {
                auto mesh = m_meshes[meshIndex];
                if (mesh) {
                    // Set transform uniform
                    shader->SetMat4("u_model", nodeTransform);
                    
                    // Apply material if available
                    if (auto material = mesh->GetMaterial()) {
                        material->ApplyUniforms();
                    }
                    
                    // Render the mesh
                    mesh->Draw();
                }
            }
        }
        
        // Recursively render children
        for (auto& child : node->GetChildren()) {
            RenderNode(child, nodeTransform, shader);
        }
    }

    void Model::RenderInstanced(const std::vector<Math::Mat4>& transforms, std::shared_ptr<Shader> shader) {
        // Placeholder for instanced rendering - would require instanced rendering support in Mesh class
        LOG_WARNING("Instanced rendering not yet implemented for Model class");
        
        // Fallback to individual renders
        for (const auto& transform : transforms) {
            Render(transform, shader);
        }
    }

    BoundingBox Model::GetBoundingBox() const {
        return m_boundingBox;
    }

    BoundingSphere Model::GetBoundingSphere() const {
        return m_boundingSphere;
    }

    void Model::UpdateBounds() {
        CalculateBounds();
    }

    void Model::CalculateBounds() {
        m_boundingBox = BoundingBox();
        m_boundingSphere = BoundingSphere();
        
        // Calculate bounds from all meshes
        for (const auto& mesh : m_meshes) {
            if (!mesh) continue;
            
            const auto& vertices = mesh->GetVertices();
            for (const auto& vertex : vertices) {
                m_boundingBox.Expand(vertex.position);
                m_boundingSphere.Expand(vertex.position);
            }
        }
        
        // If we have a valid bounding box but invalid sphere, create sphere from box
        if (m_boundingBox.IsValid() && !m_boundingSphere.IsValid()) {
            Math::Vec3 center = m_boundingBox.GetCenter();
            Math::Vec3 size = m_boundingBox.GetSize();
            float radius = glm::length(size) * 0.5f;
            m_boundingSphere = BoundingSphere(center, radius);
        }
    }

    // LOD methods (placeholders)
    void Model::SetLODLevels(const std::vector<std::shared_ptr<Model>>& lodLevels) {
        m_lodLevels = lodLevels;
    }

    std::shared_ptr<Model> Model::GetLOD(float distance) const {
        // Placeholder implementation - would implement distance-based LOD selection
        return nullptr;
    }

    size_t Model::GetLODCount() const {
        return m_lodLevels.size();
    }

    ModelStats Model::GetStats() const {
        return m_stats;
    }

    void Model::PrintDebugInfo() const {
        LOG_INFO("=== Model Debug Info ===");
        LOG_INFO("Name: " + m_name);
        LOG_INFO("Path: " + GetPath());
        LOG_INFO("Nodes: " + std::to_string(m_stats.nodeCount));
        LOG_INFO("Meshes: " + std::to_string(m_stats.meshCount));
        LOG_INFO("Materials: " + std::to_string(m_stats.materialCount));
        LOG_INFO("Vertices: " + std::to_string(m_stats.totalVertices));
        LOG_INFO("Triangles: " + std::to_string(m_stats.totalTriangles));
        LOG_INFO("Memory Usage: " + std::to_string(m_stats.totalMemoryUsage / 1024) + " KB");
        LOG_INFO("Loading Time: " + std::to_string(m_stats.loadingTimeMs) + " ms");
        LOG_INFO("Format: " + m_stats.formatUsed);
        
        if (m_boundingBox.IsValid()) {
            Math::Vec3 center = m_boundingBox.GetCenter();
            Math::Vec3 size = m_boundingBox.GetSize();
            LOG_INFO("Bounding Box: Center(" + std::to_string(center.x) + ", " + 
                    std::to_string(center.y) + ", " + std::to_string(center.z) + 
                    ") Size(" + std::to_string(size.x) + ", " + 
                    std::to_string(size.y) + ", " + std::to_string(size.z) + ")");
        }
        
        if (m_boundingSphere.IsValid()) {
            LOG_INFO("Bounding Sphere: Center(" + std::to_string(m_boundingSphere.center.x) + ", " + 
                    std::to_string(m_boundingSphere.center.y) + ", " + 
                    std::to_string(m_boundingSphere.center.z) + ") Radius(" + 
                    std::to_string(m_boundingSphere.radius) + ")");
        }
        
        LOG_INFO("========================");
    }

    // Serialization methods (placeholders)
    bool Model::SaveToCache(const std::string& cachePath) const {
        LOG_WARNING("Model caching not yet implemented");
        return false;
    }

    bool Model::LoadFromCache(const std::string& cachePath) {
        LOG_WARNING("Model caching not yet implemented");
        return false;
    }

    size_t Model::GetMemoryUsage() const {
        size_t totalSize = sizeof(*this);
        
        // Add mesh memory usage
        for (const auto& mesh : m_meshes) {
            if (mesh) {
                totalSize += mesh->GetMemoryUsage();
            }
        }
        
        // Add material memory usage (rough estimate)
        totalSize += m_materials.size() * sizeof(Material);
        
        // Add node memory usage (rough estimate)
        totalSize += m_stats.nodeCount * sizeof(ModelNode);
        
        return totalSize;
    }

    void Model::OptimizeMeshes() {
        // Placeholder for mesh optimization
        LOG_INFO("Mesh optimization not yet implemented");
    }

    void Model::ValidateModel() {
        // Basic validation
        if (!m_rootNode) {
            LOG_ERROR("Model has no root node");
            return;
        }
        
        // Validate mesh indices in nodes
        auto allNodes = GetAllNodes();
        for (const auto& node : allNodes) {
            for (uint32_t meshIndex : node->GetMeshIndices()) {
                if (meshIndex >= m_meshes.size()) {
                    LOG_ERROR("Node '" + node->GetName() + "' references invalid mesh index: " + std::to_string(meshIndex));
                }
            }
        }
        
        LOG_INFO("Model validation completed");
    }

    void Model::BuildNodeMap() {
        m_nodeMap.clear();
        if (m_rootNode) {
            m_rootNode->Traverse([this](std::shared_ptr<ModelNode> node) {
                if (!node->GetName().empty()) {
                    m_nodeMap[node->GetName()] = node;
                }
            });
        }
    }

    void Model::BuildMeshMap() {
        m_meshMap.clear();
        for (const auto& mesh : m_meshes) {
            if (mesh && !mesh->GetName().empty()) {
                m_meshMap[mesh->GetName()] = mesh;
            }
        }
    }

    void Model::BuildMaterialMap() {
        m_materialMap.clear();
        // Materials don't have names in the current implementation
        // This would be implemented when materials get name support
    }

    void Model::BuildAnimationMap() {
        m_animationMap.clear();
        for (const auto& animation : m_animations) {
            if (animation && !animation->GetName().empty()) {
                m_animationMap[animation->GetName()] = animation;
            }
        }
    }

    void Model::AddMesh(std::shared_ptr<Mesh> mesh) {
        if (mesh) {
            m_meshes.push_back(mesh);
            BuildMeshMap();
            
            // Update stats
            m_stats.meshCount = static_cast<uint32_t>(m_meshes.size());
            m_stats.totalVertices += mesh->GetVertexCount();
            m_stats.totalTriangles += mesh->GetTriangleCount();
        }
    }

    void Model::AddMaterial(std::shared_ptr<Material> material) {
        if (material) {
            m_materials.push_back(material);
            BuildMaterialMap();
            
            // Update stats
            m_stats.materialCount = static_cast<uint32_t>(m_materials.size());
        }
    }

    void Model::SetMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) {
        m_meshes = meshes;
        BuildMeshMap();
        
        // Update stats
        m_stats.meshCount = static_cast<uint32_t>(m_meshes.size());
        m_stats.totalVertices = 0;
        m_stats.totalTriangles = 0;
        
        for (const auto& mesh : m_meshes) {
            if (mesh) {
                m_stats.totalVertices += mesh->GetVertexCount();
                m_stats.totalTriangles += mesh->GetTriangleCount();
            }
        }
    }

    void Model::SetMaterials(const std::vector<std::shared_ptr<Material>>& materials) {
        m_materials = materials;
        BuildMaterialMap();
        
        // Update stats
        m_stats.materialCount = static_cast<uint32_t>(m_materials.size());
    }
}