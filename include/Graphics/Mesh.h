#pragma once

#include "Core/Math.h"
#include "Resource/ResourceManager.h"
#include <vector>
#include <memory>
#include <string>

namespace GameEngine {
    class Material;
    struct Vertex {
        Math::Vec3 position;
        Math::Vec3 normal;
        Math::Vec2 texCoords;
        Math::Vec3 tangent;
        Math::Vec3 bitangent;
        Math::Vec4 color = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white color
        
        // Skinning data (for future animation support)
        Math::Vec4 boneIds = Math::Vec4(0.0f);
        Math::Vec4 boneWeights = Math::Vec4(0.0f);
        
        // Additional texture coordinates (for future use)
        Math::Vec2 texCoords2 = Math::Vec2(0.0f);
        Math::Vec2 texCoords3 = Math::Vec2(0.0f);
    };

    class Mesh : public Resource {
    public:
        Mesh(const std::string& path = "");
        ~Mesh();

        // Loading methods
        bool LoadFromFile(const std::string& filepath);
        void CreateDefault(); // Creates default cube mesh
        
        void SetVertices(const std::vector<Vertex>& vertices);
        void SetIndices(const std::vector<uint32_t>& indices);
        
        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_indices; }
        
        // Statistics methods
        uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_vertices.size()); }
        uint32_t GetTriangleCount() const { return static_cast<uint32_t>(m_indices.size() / 3); }
        
        // Name management
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }
        
        // Material management
        void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }
        std::shared_ptr<Material> GetMaterial() const { return m_material; }
        
        uint32_t GetVAO() const { return m_VAO; }
        uint32_t GetVBO() const { return m_VBO; }
        uint32_t GetEBO() const { return m_EBO; }
        
        void Bind() const;
        void Unbind() const;
        void Draw() const;
        
        // Cleanup methods
        void Cleanup(); // Explicit cleanup of OpenGL resources
        
        // Resource interface
        size_t GetMemoryUsage() const override;

    private:
        void SetupMesh() const;
        void EnsureGPUResourcesCreated() const; // Lazy initialization
        void CreateGPUResources() const;
        
        // CPU data (always available)
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::string m_name;
        std::shared_ptr<Material> m_material;
        
        // GPU resources (created lazily)
        mutable uint32_t m_VAO = 0;
        mutable uint32_t m_VBO = 0;
        mutable uint32_t m_EBO = 0;
        mutable bool m_gpuResourcesCreated = false;
    };
}