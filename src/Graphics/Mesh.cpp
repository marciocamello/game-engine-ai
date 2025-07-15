#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace GameEngine {
    Mesh::Mesh() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
    }

    Mesh::~Mesh() {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);
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
        if (m_vertices.empty()) return;

        glBindVertexArray(m_VAO);

        // Vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

        // Index buffer
        if (!m_indices.empty()) {
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

        glBindVertexArray(0);
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
}