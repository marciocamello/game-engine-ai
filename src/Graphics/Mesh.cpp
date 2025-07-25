#include "Graphics/Mesh.h"
#include "Resource/MeshLoader.h"
#include "Core/Logger.h"
#include "Core/OpenGLContext.h"
#include <glad/glad.h>
#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <cfloat>

namespace GameEngine {
    // VertexLayout implementation
    VertexLayout::VertexLayout() : stride(0) {
        // Initialize default vertex layout with all standard attributes
        AddAttribute(VertexAttribute::Position, 3, GL_FLOAT);
        AddAttribute(VertexAttribute::Normal, 3, GL_FLOAT);
        AddAttribute(VertexAttribute::TexCoords, 2, GL_FLOAT);
        AddAttribute(VertexAttribute::Tangent, 3, GL_FLOAT);
        AddAttribute(VertexAttribute::Bitangent, 3, GL_FLOAT);
        AddAttribute(VertexAttribute::Color, 4, GL_FLOAT);
        AddAttribute(VertexAttribute::BoneIds, 4, GL_FLOAT);
        AddAttribute(VertexAttribute::BoneWeights, 4, GL_FLOAT);
        AddAttribute(VertexAttribute::TexCoords2, 2, GL_FLOAT);
        AddAttribute(VertexAttribute::TexCoords3, 2, GL_FLOAT);
        
        CalculateStride();
    }
    
    void VertexLayout::CalculateStride() {
        stride = 0;
        for (auto& attr : attributes) {
            attr.offset = stride;
            if (attr.enabled) {
                uint32_t attributeSize = 0;
                switch (attr.dataType) {
                    case GL_FLOAT:
                        attributeSize = attr.size * sizeof(float);
                        break;
                    case GL_INT:
                        attributeSize = attr.size * sizeof(int);
                        break;
                    case GL_UNSIGNED_INT:
                        attributeSize = attr.size * sizeof(unsigned int);
                        break;
                    case GL_BYTE:
                        attributeSize = attr.size * sizeof(char);
                        break;
                    case GL_UNSIGNED_BYTE:
                        attributeSize = attr.size * sizeof(unsigned char);
                        break;
                    default:
                        attributeSize = attr.size * sizeof(float); // Default to float
                        break;
                }
                stride += attributeSize;
            }
        }
    }
    
    void VertexLayout::AddAttribute(VertexAttribute type, uint32_t size, uint32_t dataType, bool normalized) {
        Attribute attr;
        attr.type = type;
        attr.size = size;
        attr.dataType = dataType;
        attr.normalized = normalized;
        attr.enabled = true; // Enable by default
        attr.offset = 0; // Will be calculated in CalculateStride
        
        // Remove existing attribute of same type if present
        attributes.erase(std::remove_if(attributes.begin(), attributes.end(),
            [type](const Attribute& a) { return a.type == type; }), attributes.end());
        
        attributes.push_back(attr);
        CalculateStride();
    }
    
    uint32_t VertexLayout::GetAttributeOffset(VertexAttribute type) const {
        for (const auto& attr : attributes) {
            if (attr.type == type) {
                return attr.offset;
            }
        }
        return 0;
    }
    
    bool VertexLayout::HasAttribute(VertexAttribute type) const {
        return std::any_of(attributes.begin(), attributes.end(),
            [type](const Attribute& attr) { return attr.type == type; });
    }
    
    void VertexLayout::EnableAttribute(VertexAttribute type) {
        for (auto& attr : attributes) {
            if (attr.type == type) {
                attr.enabled = true;
                break;
            }
        }
    }
    
    void VertexLayout::DisableAttribute(VertexAttribute type) {
        for (auto& attr : attributes) {
            if (attr.type == type) {
                attr.enabled = false;
                break;
            }
        }
    }
    
    bool VertexLayout::IsAttributeEnabled(VertexAttribute type) const {
        for (const auto& attr : attributes) {
            if (attr.type == type) {
                return attr.enabled;
            }
        }
        return false;
    }
    
    // Vertex implementation
    bool Vertex::operator==(const Vertex& other) const {
        return position == other.position &&
               normal == other.normal &&
               texCoords == other.texCoords &&
               tangent == other.tangent &&
               bitangent == other.bitangent &&
               color == other.color &&
               boneIds == other.boneIds &&
               boneWeights == other.boneWeights &&
               texCoords2 == other.texCoords2 &&
               texCoords3 == other.texCoords3;
    }
    
    bool Vertex::IsNearlyEqual(const Vertex& other, float epsilon) const {
        return glm::length(position - other.position) < epsilon &&
               glm::length(normal - other.normal) < epsilon &&
               glm::length(texCoords - other.texCoords) < epsilon &&
               glm::length(tangent - other.tangent) < epsilon &&
               glm::length(bitangent - other.bitangent) < epsilon &&
               glm::length(color - other.color) < epsilon;
    }
    

    
    Mesh::Mesh(const std::string& path) : Resource(path), m_VAO(0), m_VBO(0), m_EBO(0) {
        // Only create OpenGL resources if context is available
        if (OpenGLContext::HasActiveContext()) {
            glGenVertexArrays(1, &m_VAO);
            glGenBuffers(1, &m_VBO);
            glGenBuffers(1, &m_EBO);
            m_gpuResourcesCreated = true;
            Logger::GetInstance().Log(LogLevel::Debug, "Mesh created with VAO: " + std::to_string(m_VAO) + 
                                     ", VBO: " + std::to_string(m_VBO) + ", EBO: " + std::to_string(m_EBO));
        } else {
            Logger::GetInstance().Log(LogLevel::Debug, "Mesh created (no OpenGL context - will use lazy initialization)");
        }
    }

    Mesh::~Mesh() {
        // Only cleanup OpenGL resources if context is available
        if (OpenGLContext::HasActiveContext()) {
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
            
            Logger::GetInstance().Log(LogLevel::Debug, "Mesh destroyed and OpenGL buffers cleaned up");
        } else {
            Logger::GetInstance().Log(LogLevel::Debug, "Mesh destroyed (no OpenGL context for cleanup)");
        }
        
        // Clear CPU data
        m_vertices.clear();
        m_indices.clear();
        m_gpuResourcesCreated = false;
    }

    bool Mesh::LoadFromFile(const std::string& filepath) {
        Logger::GetInstance().Log(LogLevel::Info, "Loading mesh from file: " + filepath);
        
        // Use MeshLoader to load the mesh data
        MeshLoader::MeshData meshData = MeshLoader::LoadOBJ(filepath);
        
        if (!meshData.isValid) {
            Logger::GetInstance().Log(LogLevel::Error, "Failed to load mesh from file: " + filepath + 
                                     " - " + meshData.errorMessage);
            Logger::GetInstance().Log(LogLevel::Info, "Mesh loading failed, but CreateDefault() can be called to create fallback mesh");
            return false;
        }
        
        // Set the loaded data
        SetVertices(meshData.vertices);
        SetIndices(meshData.indices);
        
        Logger::GetInstance().Log(LogLevel::Info, "Successfully loaded mesh from file: " + filepath);
        return true;
    }
    
    void Mesh::CreateDefault() {
        Logger::GetInstance().Log(LogLevel::Info, "Creating default fallback cube mesh");
        
        try {
            // Use MeshLoader to create default cube
            auto defaultMesh = MeshLoader::CreateDefaultCube();
            
            if (!defaultMesh) {
                LOG_ERROR("MeshLoader::CreateDefaultCube() returned null");
                throw std::runtime_error("Failed to create default cube mesh");
            }
            
            // Copy the data from the default mesh
            SetVertices(defaultMesh->GetVertices());
            SetIndices(defaultMesh->GetIndices());
            
            Logger::GetInstance().Log(LogLevel::Info, "Default fallback cube mesh created successfully (" + 
                                     std::to_string(m_vertices.size()) + " vertices, " + 
                                     std::to_string(m_indices.size()) + " indices)");
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while creating default mesh: " + std::string(e.what()));
            
            // Create a minimal fallback triangle if cube creation fails
            LOG_INFO("Creating minimal triangle fallback");
            
            std::vector<Vertex> triangleVertices = {
                {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}
            };
            
            std::vector<uint32_t> triangleIndices = {0, 1, 2};
            
            SetVertices(triangleVertices);
            SetIndices(triangleIndices);
            
            LOG_INFO("Created minimal triangle fallback mesh");
        } catch (...) {
            LOG_ERROR("Unknown exception while creating default mesh");
            throw;
        }
    }

    void Mesh::SetVertices(const std::vector<Vertex>& vertices) {
        m_vertices = vertices;
        CalculateBounds();
        SetupMesh();
    }

    void Mesh::SetIndices(const std::vector<uint32_t>& indices) {
        m_indices = indices;
        SetupMesh();
    }
    
    void Mesh::SetVertexLayout(const VertexLayout& layout) {
        m_layout = layout;
        // Force recreation of GPU resources with new layout
        if (m_gpuResourcesCreated) {
            m_gpuResourcesCreated = false;
            if (m_VAO != 0 && OpenGLContext::HasActiveContext()) {
                glDeleteVertexArrays(1, &m_VAO);
                glDeleteBuffers(1, &m_VBO);
                glDeleteBuffers(1, &m_EBO);
                m_VAO = m_VBO = m_EBO = 0;
            }
        }
    }
    
    void Mesh::EnableAttribute(VertexAttribute attribute) {
        m_layout.EnableAttribute(attribute);
        // Force recreation of GPU resources
        if (m_gpuResourcesCreated) {
            m_gpuResourcesCreated = false;
        }
    }
    
    void Mesh::DisableAttribute(VertexAttribute attribute) {
        m_layout.DisableAttribute(attribute);
        // Force recreation of GPU resources
        if (m_gpuResourcesCreated) {
            m_gpuResourcesCreated = false;
        }
    }
    
    bool Mesh::IsAttributeEnabled(VertexAttribute attribute) const {
        return m_layout.IsAttributeEnabled(attribute);
    }
    
    MeshStats Mesh::GetStats() const {
        MeshStats stats;
        stats.vertexCount = GetVertexCount();
        stats.triangleCount = GetTriangleCount();
        stats.memoryUsage = GetMemoryUsage();
        
        // Analyze vertex attributes
        if (!m_vertices.empty()) {
            stats.hasNormals = std::any_of(m_vertices.begin(), m_vertices.end(),
                [](const Vertex& v) { return glm::length(v.normal) > 0.001f; });
            stats.hasTangents = std::any_of(m_vertices.begin(), m_vertices.end(),
                [](const Vertex& v) { return glm::length(v.tangent) > 0.001f; });
            stats.hasTextureCoords = std::any_of(m_vertices.begin(), m_vertices.end(),
                [](const Vertex& v) { return glm::length(v.texCoords) > 0.001f; });
            stats.hasColors = std::any_of(m_vertices.begin(), m_vertices.end(),
                [](const Vertex& v) { return v.color != Math::Vec4(1.0f); });
            stats.hasBoneWeights = std::any_of(m_vertices.begin(), m_vertices.end(),
                [](const Vertex& v) { return glm::length(v.boneWeights) > 0.001f; });
        }
        
        // Analyze triangles
        if (!m_indices.empty() && m_indices.size() >= 3) {
            float totalArea = 0.0f;
            stats.minTriangleArea = FLT_MAX;
            stats.maxTriangleArea = 0.0f;
            uint32_t validTriangles = 0;
            
            for (size_t i = 0; i < m_indices.size(); i += 3) {
                if (i + 2 < m_indices.size()) {
                    float area = CalculateTriangleArea(m_indices[i], m_indices[i + 1], m_indices[i + 2]);
                    if (area > 0.0001f) { // Valid triangle
                        totalArea += area;
                        stats.minTriangleArea = std::min(stats.minTriangleArea, area);
                        stats.maxTriangleArea = std::max(stats.maxTriangleArea, area);
                        validTriangles++;
                    } else {
                        stats.degenerateTriangles++;
                    }
                }
            }
            
            if (validTriangles > 0) {
                stats.averageTriangleArea = totalArea / validTriangles;
            }
        }
        
        // Count duplicate vertices
        std::unordered_set<size_t> uniqueVertices;
        for (const auto& vertex : m_vertices) {
            // Simple hash based on position
            size_t hash = std::hash<float>{}(vertex.position.x) ^
                         (std::hash<float>{}(vertex.position.y) << 1) ^
                         (std::hash<float>{}(vertex.position.z) << 2);
            if (uniqueVertices.find(hash) != uniqueVertices.end()) {
                stats.duplicateVertices++;
            } else {
                uniqueVertices.insert(hash);
            }
        }
        
        return stats;
    }
    
    void Mesh::UpdateBounds() {
        CalculateBounds();
    }

    void Mesh::SetupMesh() const {
        if (m_vertices.empty()) {
            Logger::GetInstance().Log(LogLevel::Warning, "Attempting to setup mesh with no vertices");
            return;
        }

        // Ensure GPU resources are created before setting up
        EnsureGPUResourcesCreated();
        
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

        // Setup vertex attributes using the flexible layout system
        SetupVertexAttributes();

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
    
    void Mesh::SetupVertexAttributes() const {
        // Setup vertex attributes based on the layout
        for (const auto& attr : m_layout.attributes) {
            if (!attr.enabled) continue;
            
            uint32_t location = static_cast<uint32_t>(attr.type);
            glEnableVertexAttribArray(location);
            
            // Get the actual offset in the Vertex struct
            void* offset = nullptr;
            switch (attr.type) {
                case VertexAttribute::Position:
                    offset = (void*)offsetof(Vertex, position);
                    break;
                case VertexAttribute::Normal:
                    offset = (void*)offsetof(Vertex, normal);
                    break;
                case VertexAttribute::TexCoords:
                    offset = (void*)offsetof(Vertex, texCoords);
                    break;
                case VertexAttribute::Tangent:
                    offset = (void*)offsetof(Vertex, tangent);
                    break;
                case VertexAttribute::Bitangent:
                    offset = (void*)offsetof(Vertex, bitangent);
                    break;
                case VertexAttribute::Color:
                    offset = (void*)offsetof(Vertex, color);
                    break;
                case VertexAttribute::BoneIds:
                    offset = (void*)offsetof(Vertex, boneIds);
                    break;
                case VertexAttribute::BoneWeights:
                    offset = (void*)offsetof(Vertex, boneWeights);
                    break;
                case VertexAttribute::TexCoords2:
                    offset = (void*)offsetof(Vertex, texCoords2);
                    break;
                case VertexAttribute::TexCoords3:
                    offset = (void*)offsetof(Vertex, texCoords3);
                    break;
            }
            
            glVertexAttribPointer(location, attr.size, attr.dataType, 
                                attr.normalized ? GL_TRUE : GL_FALSE, 
                                sizeof(Vertex), offset);
        }
    }

    void Mesh::Bind() const {
        EnsureGPUResourcesCreated();
        if (m_VAO != 0) {
            glBindVertexArray(m_VAO);
        }
    }

    void Mesh::Unbind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        Bind();
        
        GLenum primitiveMode = GL_TRIANGLES;
        switch (m_primitiveType) {
            case PrimitiveType::Triangles: primitiveMode = GL_TRIANGLES; break;
            case PrimitiveType::Lines: primitiveMode = GL_LINES; break;
            case PrimitiveType::Points: primitiveMode = GL_POINTS; break;
            case PrimitiveType::TriangleStrip: primitiveMode = GL_TRIANGLE_STRIP; break;
            case PrimitiveType::TriangleFan: primitiveMode = GL_TRIANGLE_FAN; break;
        }
        
        if (!m_indices.empty()) {
            glDrawElements(primitiveMode, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(primitiveMode, 0, static_cast<GLsizei>(m_vertices.size()));
        }
        
        Unbind();
    }
    
    void Mesh::DrawInstanced(uint32_t instanceCount) const {
        Bind();
        
        GLenum primitiveMode = GL_TRIANGLES;
        switch (m_primitiveType) {
            case PrimitiveType::Triangles: primitiveMode = GL_TRIANGLES; break;
            case PrimitiveType::Lines: primitiveMode = GL_LINES; break;
            case PrimitiveType::Points: primitiveMode = GL_POINTS; break;
            case PrimitiveType::TriangleStrip: primitiveMode = GL_TRIANGLE_STRIP; break;
            case PrimitiveType::TriangleFan: primitiveMode = GL_TRIANGLE_FAN; break;
        }
        
        if (!m_indices.empty()) {
            glDrawElementsInstanced(primitiveMode, static_cast<GLsizei>(m_indices.size()), 
                                  GL_UNSIGNED_INT, 0, instanceCount);
        } else {
            glDrawArraysInstanced(primitiveMode, 0, static_cast<GLsizei>(m_vertices.size()), instanceCount);
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
    
    void Mesh::EnsureGPUResourcesCreated() const {
        if (m_gpuResourcesCreated || !OpenGLContext::HasActiveContext()) {
            return;
        }
        
        CreateGPUResources();
        m_gpuResourcesCreated = true;
    }
    
    void Mesh::CreateGPUResources() const {
        if (!OpenGLContext::HasActiveContext()) {
            LOG_WARNING("Cannot create mesh GPU resources: No OpenGL context available");
            return;
        }
        
        // Generate OpenGL objects
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
        
        if (m_VAO == 0 || m_VBO == 0 || m_EBO == 0) {
            LOG_ERROR("Failed to generate OpenGL mesh resources");
            return;
        }
        
        Logger::GetInstance().Log(LogLevel::Debug, "Created GPU resources for mesh with VAO: " + std::to_string(m_VAO) + 
                                 ", VBO: " + std::to_string(m_VBO) + ", EBO: " + std::to_string(m_EBO));
        
        // Setup mesh data if available
        if (!m_vertices.empty()) {
            SetupMesh();
        }
    }
    
    // Mesh optimization methods
    void Mesh::OptimizeVertexCache() {
        if (m_indices.empty() || m_indices.size() < 3) {
            LOG_WARNING("Cannot optimize vertex cache: insufficient indices");
            return;
        }
        
        LOG_INFO("Optimizing vertex cache using Tom Forsyth's algorithm...");
        
        // Tom Forsyth's vertex cache optimization algorithm
        const uint32_t CACHE_SIZE = 32; // Typical GPU vertex cache size
        const float CACHE_DECAY_POWER = 1.5f;
        const float LAST_TRI_SCORE = 0.75f;
        const float VALENCE_BOOST_SCALE = 2.0f;
        const float VALENCE_BOOST_POWER = 0.5f;
        
        uint32_t numVertices = static_cast<uint32_t>(m_vertices.size());
        uint32_t numTriangles = static_cast<uint32_t>(m_indices.size() / 3);
        
        // Build adjacency information
        std::vector<std::vector<uint32_t>> vertexTriangles(numVertices);
        std::vector<bool> triangleAdded(numTriangles, false);
        
        for (uint32_t i = 0; i < numTriangles; ++i) {
            uint32_t i0 = m_indices[i * 3];
            uint32_t i1 = m_indices[i * 3 + 1];
            uint32_t i2 = m_indices[i * 3 + 2];
            
            if (i0 < numVertices) vertexTriangles[i0].push_back(i);
            if (i1 < numVertices) vertexTriangles[i1].push_back(i);
            if (i2 < numVertices) vertexTriangles[i2].push_back(i);
        }
        
        // Vertex cache simulation
        std::vector<uint32_t> cache;
        std::vector<uint32_t> cacheTimestamp(numVertices, 0);
        std::vector<float> vertexScore(numVertices, 0.0f);
        std::vector<uint32_t> vertexValence(numVertices);
        
        // Initialize vertex valence
        for (uint32_t i = 0; i < numVertices; ++i) {
            vertexValence[i] = static_cast<uint32_t>(vertexTriangles[i].size());
        }
        
        // Calculate initial vertex scores
        auto calculateVertexScore = [&](uint32_t vertex) -> float {
            if (vertexValence[vertex] == 0) return -1.0f;
            
            float score = 0.0f;
            
            // Cache position score
            uint32_t cachePos = 0;
            bool inCache = false;
            for (uint32_t i = 0; i < cache.size() && i < CACHE_SIZE; ++i) {
                if (cache[i] == vertex) {
                    cachePos = i;
                    inCache = true;
                    break;
                }
            }
            
            if (inCache) {
                if (cachePos < 3) {
                    score = LAST_TRI_SCORE;
                } else {
                    const float scaler = 1.0f / (CACHE_SIZE - 3);
                    score = 1.0f - (cachePos - 3) * scaler;
                    score = std::pow(score, CACHE_DECAY_POWER);
                }
            }
            
            // Valence score
            float valenceScore = std::pow(static_cast<float>(vertexValence[vertex]), -VALENCE_BOOST_POWER);
            score += VALENCE_BOOST_SCALE * valenceScore;
            
            return score;
        };
        
        // Initialize scores
        for (uint32_t i = 0; i < numVertices; ++i) {
            vertexScore[i] = calculateVertexScore(i);
        }
        
        // Optimize triangle order
        std::vector<uint32_t> newIndices;
        newIndices.reserve(m_indices.size());
        
        for (uint32_t addedTriangles = 0; addedTriangles < numTriangles; ++addedTriangles) {
            // Find best triangle to add next
            uint32_t bestTriangle = 0;
            float bestScore = -1.0f;
            
            for (uint32_t i = 0; i < numTriangles; ++i) {
                if (triangleAdded[i]) continue;
                
                uint32_t i0 = m_indices[i * 3];
                uint32_t i1 = m_indices[i * 3 + 1];
                uint32_t i2 = m_indices[i * 3 + 2];
                
                float score = 0.0f;
                if (i0 < numVertices) score += vertexScore[i0];
                if (i1 < numVertices) score += vertexScore[i1];
                if (i2 < numVertices) score += vertexScore[i2];
                
                if (score > bestScore) {
                    bestScore = score;
                    bestTriangle = i;
                }
            }
            
            // Add best triangle
            triangleAdded[bestTriangle] = true;
            uint32_t i0 = m_indices[bestTriangle * 3];
            uint32_t i1 = m_indices[bestTriangle * 3 + 1];
            uint32_t i2 = m_indices[bestTriangle * 3 + 2];
            
            newIndices.push_back(i0);
            newIndices.push_back(i1);
            newIndices.push_back(i2);
            
            // Update cache
            auto updateCache = [&](uint32_t vertex) {
                // Remove vertex from cache if present
                cache.erase(std::remove(cache.begin(), cache.end(), vertex), cache.end());
                // Add to front
                cache.insert(cache.begin(), vertex);
                // Limit cache size
                if (cache.size() > CACHE_SIZE) {
                    cache.resize(CACHE_SIZE);
                }
            };
            
            if (i0 < numVertices) updateCache(i0);
            if (i1 < numVertices) updateCache(i1);
            if (i2 < numVertices) updateCache(i2);
            
            // Update vertex valences and scores
            auto updateVertex = [&](uint32_t vertex) {
                if (vertex >= numVertices) return;
                
                // Remove this triangle from vertex's triangle list
                auto& triangles = vertexTriangles[vertex];
                triangles.erase(std::remove(triangles.begin(), triangles.end(), bestTriangle), triangles.end());
                vertexValence[vertex] = static_cast<uint32_t>(triangles.size());
                
                // Recalculate score
                vertexScore[vertex] = calculateVertexScore(vertex);
                
                // Update scores of adjacent vertices
                for (uint32_t adjTri : triangles) {
                    if (!triangleAdded[adjTri]) {
                        uint32_t ai0 = m_indices[adjTri * 3];
                        uint32_t ai1 = m_indices[adjTri * 3 + 1];
                        uint32_t ai2 = m_indices[adjTri * 3 + 2];
                        
                        if (ai0 < numVertices) vertexScore[ai0] = calculateVertexScore(ai0);
                        if (ai1 < numVertices) vertexScore[ai1] = calculateVertexScore(ai1);
                        if (ai2 < numVertices) vertexScore[ai2] = calculateVertexScore(ai2);
                    }
                }
            };
            
            updateVertex(i0);
            updateVertex(i1);
            updateVertex(i2);
        }
        
        // Replace indices with optimized version
        m_indices = newIndices;
        
        LOG_INFO("Vertex cache optimization completed");
        
        // Update GPU resources
        if (m_gpuResourcesCreated) {
            SetupMesh();
        }
    }
    
    void Mesh::OptimizeVertexFetch() {
        if (m_vertices.empty() || m_indices.empty()) {
            LOG_WARNING("Cannot optimize vertex fetch: insufficient data");
            return;
        }
        
        LOG_INFO("Optimizing vertex fetch order...");
        
        // Create mapping from old to new vertex indices
        std::vector<uint32_t> vertexRemap(m_vertices.size());
        std::vector<bool> vertexUsed(m_vertices.size(), false);
        std::vector<Vertex> newVertices;
        
        uint32_t newIndex = 0;
        
        // Process vertices in the order they appear in indices
        for (uint32_t oldIndex : m_indices) {
            if (oldIndex < m_vertices.size() && !vertexUsed[oldIndex]) {
                vertexRemap[oldIndex] = newIndex++;
                newVertices.push_back(m_vertices[oldIndex]);
                vertexUsed[oldIndex] = true;
            }
        }
        
        // Add any unused vertices at the end
        for (uint32_t i = 0; i < m_vertices.size(); ++i) {
            if (!vertexUsed[i]) {
                vertexRemap[i] = newIndex++;
                newVertices.push_back(m_vertices[i]);
            }
        }
        
        // Update indices to use new vertex order
        for (uint32_t& index : m_indices) {
            if (index < m_vertices.size()) {
                index = vertexRemap[index];
            }
        }
        
        // Replace vertices with reordered version
        m_vertices = newVertices;
        
        LOG_INFO("Vertex fetch optimization completed");
        
        // Update GPU resources
        if (m_gpuResourcesCreated) {
            SetupMesh();
        }
    }
    
    void Mesh::OptimizeOverdraw(float threshold) {
        if (m_indices.empty() || m_indices.size() < 3) {
            LOG_WARNING("Cannot optimize overdraw: insufficient indices");
            return;
        }
        
        LOG_INFO("Optimizing overdraw with threshold: " + std::to_string(threshold));
        
        // Simple overdraw optimization: sort triangles by depth (back-to-front)
        // This is a basic implementation - more sophisticated methods exist
        
        struct Triangle {
            uint32_t indices[3];
            float depth;
        };
        
        std::vector<Triangle> triangles;
        
        // Calculate triangle depths (using centroid Z coordinate)
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            if (i + 2 < m_indices.size()) {
                Triangle tri;
                tri.indices[0] = m_indices[i];
                tri.indices[1] = m_indices[i + 1];
                tri.indices[2] = m_indices[i + 2];
                
                // Calculate centroid depth
                float depth = 0.0f;
                if (tri.indices[0] < m_vertices.size()) depth += m_vertices[tri.indices[0]].position.z;
                if (tri.indices[1] < m_vertices.size()) depth += m_vertices[tri.indices[1]].position.z;
                if (tri.indices[2] < m_vertices.size()) depth += m_vertices[tri.indices[2]].position.z;
                tri.depth = depth / 3.0f;
                
                triangles.push_back(tri);
            }
        }
        
        // Sort triangles by depth (back-to-front for better overdraw)
        std::sort(triangles.begin(), triangles.end(),
            [](const Triangle& a, const Triangle& b) {
                return a.depth > b.depth; // Back-to-front
            });
        
        // Rebuild indices
        m_indices.clear();
        m_indices.reserve(triangles.size() * 3);
        
        for (const auto& tri : triangles) {
            m_indices.push_back(tri.indices[0]);
            m_indices.push_back(tri.indices[1]);
            m_indices.push_back(tri.indices[2]);
        }
        
        LOG_INFO("Overdraw optimization completed");
        
        // Update GPU resources
        if (m_gpuResourcesCreated) {
            SetupMesh();
        }
    }
    
    void Mesh::RemoveDuplicateVertices(float epsilon) {
        if (m_vertices.empty()) return;
        
        std::vector<Vertex> uniqueVertices;
        std::vector<uint32_t> newIndices;
        std::unordered_map<size_t, uint32_t> vertexMap;
        
        for (size_t i = 0; i < m_vertices.size(); ++i) {
            // Simple hash for vertex comparison
            const Vertex& v = m_vertices[i];
            size_t hash = std::hash<float>{}(v.position.x) ^
                         (std::hash<float>{}(v.position.y) << 1) ^
                         (std::hash<float>{}(v.position.z) << 2);
            
            // Check if we already have a similar vertex
            bool found = false;
            for (size_t j = 0; j < uniqueVertices.size(); ++j) {
                if (v.IsNearlyEqual(uniqueVertices[j], epsilon)) {
                    // Update indices to point to the existing vertex
                    for (auto& index : m_indices) {
                        if (index == i) {
                            index = static_cast<uint32_t>(j);
                        }
                    }
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                uniqueVertices.push_back(v);
                // Update indices to point to the new unique vertex
                for (auto& index : m_indices) {
                    if (index == i) {
                        index = static_cast<uint32_t>(uniqueVertices.size() - 1);
                    }
                }
            }
        }
        
        size_t originalCount = m_vertices.size();
        m_vertices = uniqueVertices;
        
        LOG_INFO("Removed duplicate vertices: " + std::to_string(originalCount) + 
                " -> " + std::to_string(m_vertices.size()) + 
                " (removed " + std::to_string(originalCount - m_vertices.size()) + ")");
        
        // Update GPU resources
        if (m_gpuResourcesCreated) {
            SetupMesh();
        }
    }
    
    void Mesh::GenerateNormals(bool smooth) {
        if (m_vertices.empty() || m_indices.empty()) return;
        
        // Reset all normals
        for (auto& vertex : m_vertices) {
            vertex.normal = Math::Vec3(0.0f);
        }
        
        // Calculate face normals and accumulate
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            if (i + 2 >= m_indices.size()) break;
            
            uint32_t i0 = m_indices[i];
            uint32_t i1 = m_indices[i + 1];
            uint32_t i2 = m_indices[i + 2];
            
            if (i0 >= m_vertices.size() || i1 >= m_vertices.size() || i2 >= m_vertices.size()) continue;
            
            Math::Vec3 v0 = m_vertices[i0].position;
            Math::Vec3 v1 = m_vertices[i1].position;
            Math::Vec3 v2 = m_vertices[i2].position;
            
            Math::Vec3 edge1 = v1 - v0;
            Math::Vec3 edge2 = v2 - v0;
            Math::Vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
            
            if (smooth) {
                // Accumulate normals for smooth shading
                m_vertices[i0].normal += faceNormal;
                m_vertices[i1].normal += faceNormal;
                m_vertices[i2].normal += faceNormal;
            } else {
                // Set same normal for flat shading
                m_vertices[i0].normal = faceNormal;
                m_vertices[i1].normal = faceNormal;
                m_vertices[i2].normal = faceNormal;
            }
        }
        
        if (smooth) {
            // Normalize accumulated normals
            for (auto& vertex : m_vertices) {
                if (glm::length(vertex.normal) > 0.001f) {
                    vertex.normal = glm::normalize(vertex.normal);
                }
            }
        }
        
        LOG_INFO("Generated " + std::string(smooth ? "smooth" : "flat") + " normals for mesh");
        
        // Update GPU resources
        if (m_gpuResourcesCreated) {
            SetupMesh();
        }
    }
    
    void Mesh::GenerateTangents() {
        if (m_vertices.empty() || m_indices.empty()) return;
        
        // Reset tangents and bitangents
        for (auto& vertex : m_vertices) {
            vertex.tangent = Math::Vec3(0.0f);
            vertex.bitangent = Math::Vec3(0.0f);
        }
        
        // Calculate tangents for each triangle
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            if (i + 2 >= m_indices.size()) break;
            
            uint32_t i0 = m_indices[i];
            uint32_t i1 = m_indices[i + 1];
            uint32_t i2 = m_indices[i + 2];
            
            if (i0 >= m_vertices.size() || i1 >= m_vertices.size() || i2 >= m_vertices.size()) continue;
            
            Vertex& v0 = m_vertices[i0];
            Vertex& v1 = m_vertices[i1];
            Vertex& v2 = m_vertices[i2];
            
            Math::Vec3 edge1 = v1.position - v0.position;
            Math::Vec3 edge2 = v2.position - v0.position;
            
            Math::Vec2 deltaUV1 = v1.texCoords - v0.texCoords;
            Math::Vec2 deltaUV2 = v2.texCoords - v0.texCoords;
            
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            
            if (std::isfinite(f)) {
                Math::Vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
                Math::Vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
                
                v0.tangent += tangent;
                v1.tangent += tangent;
                v2.tangent += tangent;
                
                v0.bitangent += bitangent;
                v1.bitangent += bitangent;
                v2.bitangent += bitangent;
            }
        }
        
        // Normalize tangents and bitangents
        for (auto& vertex : m_vertices) {
            if (glm::length(vertex.tangent) > 0.0f) {
                vertex.tangent = glm::normalize(vertex.tangent);
            } else {
                vertex.tangent = Math::Vec3(1.0f, 0.0f, 0.0f);
            }
            
            if (glm::length(vertex.bitangent) > 0.0f) {
                vertex.bitangent = glm::normalize(vertex.bitangent);
            } else {
                vertex.bitangent = Math::Vec3(0.0f, 0.0f, 1.0f);
            }
        }
        
        LOG_INFO("Generated tangents and bitangents for mesh");
        
        // Update GPU resources
        if (m_gpuResourcesCreated) {
            SetupMesh();
        }
    }
    
    // Validation methods
    bool Mesh::Validate() const {
        return GetValidationErrors().empty();
    }
    
    std::vector<std::string> Mesh::GetValidationErrors() const {
        std::vector<std::string> errors;
        
        if (m_vertices.empty()) {
            errors.push_back("Mesh has no vertices");
        }
        
        if (m_indices.empty() && m_vertices.size() % 3 != 0) {
            errors.push_back("Mesh has no indices and vertex count is not divisible by 3");
        }
        
        // Check for out-of-bounds indices
        for (size_t i = 0; i < m_indices.size(); ++i) {
            if (m_indices[i] >= m_vertices.size()) {
                errors.push_back("Index " + std::to_string(i) + " is out of bounds (" + 
                                std::to_string(m_indices[i]) + " >= " + std::to_string(m_vertices.size()) + ")");
            }
        }
        
        // Check for degenerate triangles
        uint32_t degenerateCount = 0;
        for (size_t i = 0; i < m_indices.size(); i += 3) {
            if (i + 2 < m_indices.size()) {
                if (IsTriangleDegenerate(m_indices[i], m_indices[i + 1], m_indices[i + 2])) {
                    degenerateCount++;
                }
            }
        }
        
        if (degenerateCount > 0) {
            errors.push_back("Mesh has " + std::to_string(degenerateCount) + " degenerate triangles");
        }
        
        return errors;
    }
    
    bool Mesh::HasValidUVCoordinates() const {
        if (m_vertices.empty()) return false;
        
        // Check if any vertex has non-zero UV coordinates
        return std::any_of(m_vertices.begin(), m_vertices.end(),
            [](const Vertex& v) { 
                return v.texCoords.x != 0.0f || v.texCoords.y != 0.0f ||
                       (v.texCoords.x >= 0.0f && v.texCoords.x <= 1.0f && 
                        v.texCoords.y >= 0.0f && v.texCoords.y <= 1.0f);
            });
    }
    
    void Mesh::GenerateFallbackUVCoordinates() {
        if (m_vertices.empty()) return;
        
        // Generate simple planar UV coordinates based on position
        BoundingBox bounds = m_boundingBox;
        if (!bounds.IsValid()) {
            CalculateBounds();
            bounds = m_boundingBox;
        }
        
        Math::Vec3 size = bounds.GetSize();
        float maxDimension = std::max({size.x, size.y, size.z});
        
        if (maxDimension > 0.0f) {
            for (auto& vertex : m_vertices) {
                // Project onto XY plane and normalize
                vertex.texCoords.x = (vertex.position.x - bounds.min.x) / maxDimension;
                vertex.texCoords.y = (vertex.position.y - bounds.min.y) / maxDimension;
                
                // Clamp to [0, 1] range
                vertex.texCoords.x = std::max(0.0f, std::min(1.0f, vertex.texCoords.x));
                vertex.texCoords.y = std::max(0.0f, std::min(1.0f, vertex.texCoords.y));
            }
            
            LOG_INFO("Generated fallback UV coordinates for mesh");
            
            // Update GPU resources
            if (m_gpuResourcesCreated) {
                SetupMesh();
            }
        }
    }
    
    // Helper methods
    void Mesh::CalculateBounds() {
        m_boundingBox = BoundingBox();
        
        if (m_vertices.empty()) {
            return;
        }
        
        // Initialize bounding box with first vertex
        m_boundingBox.min = m_vertices[0].position;
        m_boundingBox.max = m_vertices[0].position;
        
        // Calculate bounding box
        for (const auto& vertex : m_vertices) {
            m_boundingBox.Expand(vertex.position);
        }
        
        // Calculate bounding sphere
        if (m_boundingBox.IsValid()) {
            m_boundingSphere.center = m_boundingBox.GetCenter();
            m_boundingSphere.radius = 0.0f;
            
            for (const auto& vertex : m_vertices) {
                float distance = glm::length(vertex.position - m_boundingSphere.center);
                m_boundingSphere.radius = std::max(m_boundingSphere.radius, distance);
            }
        }
    }
    
    float Mesh::CalculateTriangleArea(uint32_t i0, uint32_t i1, uint32_t i2) const {
        if (i0 >= m_vertices.size() || i1 >= m_vertices.size() || i2 >= m_vertices.size()) {
            return 0.0f;
        }
        
        Math::Vec3 v0 = m_vertices[i0].position;
        Math::Vec3 v1 = m_vertices[i1].position;
        Math::Vec3 v2 = m_vertices[i2].position;
        
        Math::Vec3 edge1 = v1 - v0;
        Math::Vec3 edge2 = v2 - v0;
        
        return 0.5f * glm::length(glm::cross(edge1, edge2));
    }
    
    bool Mesh::IsTriangleDegenerate(uint32_t i0, uint32_t i1, uint32_t i2, float epsilon) const {
        if (i0 >= m_vertices.size() || i1 >= m_vertices.size() || i2 >= m_vertices.size()) {
            return true;
        }
        
        // Check if indices are the same
        if (i0 == i1 || i1 == i2 || i0 == i2) {
            return true;
        }
        
        // Check if triangle area is too small
        return CalculateTriangleArea(i0, i1, i2) < epsilon;
    }
}