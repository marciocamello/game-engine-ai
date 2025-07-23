#include "Graphics/Mesh.h"
#include "Resource/MeshLoader.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace GameEngine {
    Mesh::Mesh(const std::string& path) : Resource(path), m_VAO(0), m_VBO(0), m_EBO(0) {
        // Generate OpenGL objects
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
        
        Logger::GetInstance().Log(LogLevel::Debug, "Mesh created with VAO: " + std::to_string(m_VAO) + 
                                 ", VBO: " + std::to_string(m_VBO) + ", EBO: " + std::to_string(m_EBO));
    }

    Mesh::~Mesh() {
        // Ensure proper cleanup of OpenGL resources
        if (m_VAO != 0) {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
        
        if (m_VBO != 0) {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        
        if (m_EBO != 0) {
            glDeleteBuffers(1, &m_EBO);
            m_EBO = 0;
        }
        
        // Clear vertex and index data
        m_vertices.clear();
        m_indices.clear();
        
        Logger::GetInstance().Log(LogLevel::Debug, "Mesh destroyed and OpenGL buffers cleaned up");
    }

    bool Mesh::LoadFromFile(const std::string& filepath) {
        Logger::GetInstance().Log(LogLevel::Info, "Loading mesh from file: " + filepath);
        
        // Use MeshLoader to load the mesh data
        MeshLoader::MeshData meshData = MeshLoader::LoadOBJ(filepath);
        
        if (!meshData.isValid) {
            Logger::GetInstance().Log(LogLevel::Error, "Failed to load mesh from file: " + filepath + 
                                     " - " + meshData.errorMessage);
            Logger::GetInstance().Log(LogLevel::Info, "Creating default cube mesh as fallback");
            CreateDefault();
            return false;
        }
        
        // Set the loaded data
        SetVertices(meshData.vertices);
        SetIndices(meshData.indices);
        
        Logger::GetInstance().Log(LogLevel::Info, "Successfully loaded mesh from file: " + filepath);
        return true;
    }
    
    void Mesh::CreateDefault() {
        Logger::GetInstance().Log(LogLevel::Info, "Creating default cube mesh");
        
        // Use MeshLoader to create default cube
        auto defaultMesh = MeshLoader::CreateDefaultCube();
        
        // Copy the data from the default mesh
        SetVertices(defaultMesh->GetVertices());
        SetIndices(defaultMesh->GetIndices());
        
        Logger::GetInstance().Log(LogLevel::Info, "Default cube mesh created successfully");
    }

    void Mesh::SetVertices(const std::vector<Vertex>& vertices) {
        m_vertices = vertices;
        SetupMesh();
    }

    void Mesh::SetIndices(const std::vector<uint32_t>& indices) {
        m_indices = indices;
        SetupMesh();
    }

    void Mesh::SetupMesh() {
        if (m_vertices.empty()) {
            Logger::GetInstance().Log(LogLevel::Warning, "Attempting to setup mesh with no vertices");
            return;
        }

        if (m_VAO == 0 || m_VBO == 0) {
            Logger::GetInstance().Log(LogLevel::Error, "Invalid OpenGL buffer IDs in SetupMesh");
            return;
        }

        glBindVertexArray(m_VAO);

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

        // Index buffer
        if (!m_indices.empty()) {
            if (m_EBO == 0) {
                Logger::GetInstance().Log(LogLevel::Error, "Invalid EBO ID when setting up indices");
                return;
            }
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);
        }

        // Vertex attributes
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // Texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        // Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

        // Bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

        // Unbind VAO
        glBindVertexArray(0);
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            Logger::GetInstance().Log(LogLevel::Error, "OpenGL error in SetupMesh: " + std::to_string(error));
        } else {
            Logger::GetInstance().Log(LogLevel::Debug, "Mesh setup completed successfully with " + 
                                     std::to_string(m_vertices.size()) + " vertices and " + 
                                     std::to_string(m_indices.size()) + " indices");
        }
    }

    void Mesh::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void Mesh::Unbind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        Bind();
        
        if (!m_indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
        }
        
        Unbind();
    }
    
    void Mesh::Cleanup() {
        Logger::GetInstance().Log(LogLevel::Debug, "Explicitly cleaning up mesh resources");
        
        // Unbind any currently bound VAO to avoid issues
        glBindVertexArray(0);
        
        // Delete OpenGL resources
        if (m_VAO != 0) {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
        
        if (m_VBO != 0) {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        
        if (m_EBO != 0) {
            glDeleteBuffers(1, &m_EBO);
            m_EBO = 0;
        }
        
        // Clear CPU-side data
        m_vertices.clear();
        m_indices.clear();
        
        Logger::GetInstance().Log(LogLevel::Debug, "Mesh cleanup completed");
    }
    
    size_t Mesh::GetMemoryUsage() const {
        // Base resource memory usage
        size_t baseSize = Resource::GetMemoryUsage();
        
        // Calculate mesh memory usage
        size_t vertexMemory = m_vertices.size() * sizeof(Vertex);
        size_t indexMemory = m_indices.size() * sizeof(uint32_t);
        
        // Add estimated GPU memory usage (VAO, VBO, EBO are relatively small)
        size_t gpuMemory = vertexMemory + indexMemory;
        
        return baseSize + vertexMemory + indexMemory + gpuMemory;
    }
}