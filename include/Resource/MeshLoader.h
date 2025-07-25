#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace GameEngine {
    class MTLLoader;
    
    class MeshLoader {
    public:
        struct MeshData {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            std::shared_ptr<Material> material;
            std::string materialName;
            std::string groupName;
            std::string objectName;
            bool isValid = false;
            std::string errorMessage;
        };
        
        struct OBJLoadResult {
            std::vector<MeshData> meshes;
            std::unordered_map<std::string, std::shared_ptr<Material>> materials;
            bool success = false;
            std::string errorMessage;
            uint32_t totalVertices = 0;
            uint32_t totalTriangles = 0;
            float loadingTimeMs = 0.0f;
        };

        // Main loading interface
        static MeshData LoadOBJ(const std::string& filepath); // Legacy single mesh
        static OBJLoadResult LoadOBJWithMaterials(const std::string& filepath); // Enhanced multi-mesh with materials
        
        // Utility methods
        static bool IsOBJFile(const std::string& filepath);
        static std::shared_ptr<Mesh> CreateMeshFromData(const MeshData& meshData);
        static std::vector<std::shared_ptr<Mesh>> CreateMeshesFromResult(const OBJLoadResult& result);
        static std::shared_ptr<Mesh> CreateDefaultCube();
        static MeshData CreateDefaultCubeData(); // Headless version for testing
        
        // Validation and optimization
        static bool ValidateOBJMesh(const MeshData& meshData, std::vector<std::string>& errors);
        static void OptimizeOBJMesh(MeshData& meshData);
        static void GenerateNormalsForOBJMesh(MeshData& meshData);
        static void ConvertCoordinateSystem(MeshData& meshData, bool flipYZ = false, bool flipWinding = false);
        static void ScaleOBJMesh(MeshData& meshData, float scale);
        static void ScaleOBJMesh(MeshData& meshData, const Math::Vec3& scale);

    private:
        // Enhanced OBJ parsing with material support
        struct OBJParseState {
            std::vector<Math::Vec3> positions;
            std::vector<Math::Vec3> normals;
            std::vector<Math::Vec2> texCoords;
            std::unordered_map<std::string, std::shared_ptr<Material>> materials;
            std::string currentMaterial;
            std::string currentGroup;
            std::string currentObject;
            std::vector<MeshData> meshes;
            MeshData currentMesh;
            bool hasFaces = false;
        };
        
        // OBJ parsing implementation
        static MeshData LoadOBJImpl(const std::string& filepath); // Legacy
        static OBJLoadResult LoadOBJWithMaterialsImpl(const std::string& filepath);
        static bool ParseOBJLine(const std::string& line, OBJParseState& state, const std::string& basePath);
        static bool ParseVertex(const std::string& line, Math::Vec3& vertex);
        static bool ParseNormal(const std::string& line, Math::Vec3& normal);
        static bool ParseTexCoord(const std::string& line, Math::Vec2& texCoord);
        static bool ParseFace(const std::string& line, OBJParseState& state);
        static bool ParseMaterialLib(const std::string& line, OBJParseState& state, const std::string& basePath);
        static bool ParseUseMaterial(const std::string& line, OBJParseState& state);
        static bool ParseGroup(const std::string& line, OBJParseState& state);
        static bool ParseObject(const std::string& line, OBJParseState& state);
        
        // Legacy parsing for backward compatibility
        static bool ParseOBJLine(const std::string& line, MeshData& meshData, 
                                std::vector<Math::Vec3>& positions,
                                std::vector<Math::Vec3>& normals,
                                std::vector<Math::Vec2>& texCoords);
        static bool ParseFace(const std::string& line, MeshData& meshData,
                             const std::vector<Math::Vec3>& positions,
                             const std::vector<Math::Vec3>& normals,
                             const std::vector<Math::Vec2>& texCoords);
        
        // Helper methods
        static void FinalizeMesh(OBJParseState& state);
        static void StartNewMesh(OBJParseState& state);
        static std::string GetDirectoryPath(const std::string& filepath);
        
        // Utility methods
        static std::vector<std::string> SplitString(const std::string& str, char delimiter);
        static std::string TrimString(const std::string& str);
        static void CalculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    };
}