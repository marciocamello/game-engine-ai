#pragma once

#include "Core/Math.h"
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

    class Mesh {
    public:
        Mesh();
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

    private:
        void SetupMesh();
        
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        
        uint32_t m_VAO = 0;
        uint32_t m_VBO = 0;
        uint32_t m_EBO = 0;
    };
}