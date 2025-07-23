#pragma once

#include "Core/Math.h"
#include "Resource/ResourceManager.h"
#include <vector>
#include <memory>
#include <string>

namespace GameEngine {
    struct Vertex {
        Math::Vec3 position;
        Math::Vec3 normal;
        Math::Vec2 texCoords;
        Math::Vec3 tangent;
        Math::Vec3 bitangent;
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
        
        // GPU resources (created lazily)
        mutable uint32_t m_VAO = 0;
        mutable uint32_t m_VBO = 0;
        mutable uint32_t m_EBO = 0;
        mutable bool m_gpuResourcesCreated = false;
    };
}