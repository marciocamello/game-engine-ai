#pragma once

#include "Core/Math.h"
#include "Resource/ResourceManager.h"
#include "Graphics/BoundingVolumes.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

// Forward declarations
namespace GameEngine {
    class MorphTargetSet;
}

namespace GameEngine {
    class Material;
    
    // Vertex attribute enumeration for flexible layout system
    enum class VertexAttribute {
        Position = 0,
        Normal = 1,
        TexCoords = 2,
        Tangent = 3,
        Bitangent = 4,
        Color = 5,
        BoneIds = 6,
        BoneWeights = 7,
        TexCoords2 = 8,
        TexCoords3 = 9
    };
    
    // Vertex layout system for flexible attribute management
    struct VertexLayout {
        struct Attribute {
            VertexAttribute type;
            uint32_t offset;
            uint32_t size;
            uint32_t dataType; // GL_FLOAT, etc.
            bool normalized;
            bool enabled;
        };
        
        std::vector<Attribute> attributes;
        uint32_t stride;
        
        VertexLayout();
        void AddAttribute(VertexAttribute type, uint32_t size, uint32_t dataType, bool normalized = false);
        uint32_t GetAttributeOffset(VertexAttribute type) const;
        bool HasAttribute(VertexAttribute type) const;
        void EnableAttribute(VertexAttribute type);
        void DisableAttribute(VertexAttribute type);
        bool IsAttributeEnabled(VertexAttribute type) const;
        void CalculateStride();
    };
    
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
        
        // Equality operators for optimization
        bool operator==(const Vertex& other) const;
        bool IsNearlyEqual(const Vertex& other, float epsilon = 0.0001f) const;
    };
    
    // Mesh statistics for analysis and debugging
    struct MeshStats {
        uint32_t vertexCount = 0;
        uint32_t triangleCount = 0;
        uint32_t duplicateVertices = 0;
        uint32_t degenerateTriangles = 0;
        float averageTriangleArea = 0.0f;
        float minTriangleArea = 0.0f;
        float maxTriangleArea = 0.0f;
        bool hasNormals = false;
        bool hasTangents = false;
        bool hasTextureCoords = false;
        bool hasColors = false;
        bool hasBoneWeights = false;
        size_t memoryUsage = 0;
    };
    


    class Mesh : public Resource {
    public:
        enum class PrimitiveType { Triangles, Lines, Points, TriangleStrip, TriangleFan };
        
        Mesh(const std::string& path = "");
        ~Mesh();

        // Loading methods
        bool LoadFromFile(const std::string& filepath);
        void CreateDefault(); // Creates default cube mesh
        
        // Vertex data management
        void SetVertices(const std::vector<Vertex>& vertices);
        void SetIndices(const std::vector<uint32_t>& indices);
        
        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<uint32_t>& GetIndices() const { return m_indices; }
        
        // Vertex layout management
        void SetVertexLayout(const VertexLayout& layout);
        VertexLayout GetVertexLayout() const { return m_layout; }
        void EnableAttribute(VertexAttribute attribute);
        void DisableAttribute(VertexAttribute attribute);
        bool IsAttributeEnabled(VertexAttribute attribute) const;
        
        // Statistics methods
        uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_vertices.size()); }
        uint32_t GetTriangleCount() const { return static_cast<uint32_t>(m_indices.size() / 3); }
        MeshStats GetStats() const;
        
        // Name management
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }
        
        // Material management
        void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }
        std::shared_ptr<Material> GetMaterial() const { return m_material; }
        void SetMaterialIndex(uint32_t index) { m_materialIndex = index; }
        uint32_t GetMaterialIndex() const { return m_materialIndex; }
        
        // Primitive type management
        void SetPrimitiveType(PrimitiveType type) { m_primitiveType = type; }
        PrimitiveType GetPrimitiveType() const { return m_primitiveType; }
        
        // Morph target management
        void SetMorphTargets(std::shared_ptr<MorphTargetSet> morphTargets) { m_morphTargets = morphTargets; }
        std::shared_ptr<MorphTargetSet> GetMorphTargets() const { return m_morphTargets; }
        bool HasMorphTargets() const { return m_morphTargets != nullptr; }
        
        // Bounding volume management
        BoundingBox GetBoundingBox() const { return m_boundingBox; }
        BoundingSphere GetBoundingSphere() const { return m_boundingSphere; }
        void UpdateBounds();
        
        // Mesh optimization methods
        void OptimizeVertexCache();
        void OptimizeVertexFetch();
        void OptimizeOverdraw(float threshold = 1.05f);
        void RemoveDuplicateVertices(float epsilon = 0.0001f);
        void GenerateNormals(bool smooth = true);
        void GenerateTangents();
        
        // Validation methods
        bool Validate() const;
        std::vector<std::string> GetValidationErrors() const;
        bool HasValidUVCoordinates() const;
        void GenerateFallbackUVCoordinates();
        
        uint32_t GetVAO() const { return m_VAO; }
        uint32_t GetVBO() const { return m_VBO; }
        uint32_t GetEBO() const { return m_EBO; }
        
        void Bind() const;
        void Unbind() const;
        void Draw() const;
        void DrawInstanced(uint32_t instanceCount) const;
        
        // Cleanup methods
        void Cleanup(); // Explicit cleanup of OpenGL resources
        
        // Resource interface
        size_t GetMemoryUsage() const override;

    private:
        void SetupMesh() const;
        void EnsureGPUResourcesCreated() const; // Lazy initialization
        void CreateGPUResources() const;
        void CalculateBounds();
        void SetupVertexAttributes() const;
        
        // Helper methods for optimization
        float CalculateTriangleArea(uint32_t i0, uint32_t i1, uint32_t i2) const;
        bool IsTriangleDegenerate(uint32_t i0, uint32_t i1, uint32_t i2, float epsilon = 0.0001f) const;
        
        // CPU data (always available)
        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::string m_name;
        std::shared_ptr<Material> m_material;
        uint32_t m_materialIndex = 0;
        PrimitiveType m_primitiveType = PrimitiveType::Triangles;
        
        // Vertex layout system
        VertexLayout m_layout;
        
        // Morph targets
        std::shared_ptr<MorphTargetSet> m_morphTargets;
        
        // Bounding volumes
        BoundingBox m_boundingBox;
        BoundingSphere m_boundingSphere;
        
        // GPU resources (created lazily)
        mutable uint32_t m_VAO = 0;
        mutable uint32_t m_VBO = 0;
        mutable uint32_t m_EBO = 0;
        mutable bool m_gpuResourcesCreated = false;
    };
}