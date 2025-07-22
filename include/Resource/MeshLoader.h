#pragma once

#include "Graphics/Mesh.h"
#include <string>
#include <memory>
#include <vector>

namespace GameEngine {
    class MeshLoader {
    public:
        struct MeshData {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            bool isValid = false;
            std::string errorMessage;
        };

        // Main loading interface
        static MeshData LoadOBJ(const std::string& filepath);
        
        // Utility methods
        static bool IsOBJFile(const std::string& filepath);
        static std::shared_ptr<Mesh> CreateMeshFromData(const MeshData& meshData);
        static std::shared_ptr<Mesh> CreateDefaultCube();
        static MeshData CreateDefaultCubeData(); // Headless version for testing

    private:
        // OBJ parsing implementation
        static MeshData LoadOBJImpl(const std::string& filepath);
        static bool ParseOBJLine(const std::string& line, MeshData& meshData, 
                                std::vector<Math::Vec3>& positions,
                                std::vector<Math::Vec3>& normals,
                                std::vector<Math::Vec2>& texCoords);
        static bool ParseVertex(const std::string& line, Math::Vec3& vertex);
        static bool ParseNormal(const std::string& line, Math::Vec3& normal);
        static bool ParseTexCoord(const std::string& line, Math::Vec2& texCoord);
        static bool ParseFace(const std::string& line, MeshData& meshData,
                             const std::vector<Math::Vec3>& positions,
                             const std::vector<Math::Vec3>& normals,
                             const std::vector<Math::Vec2>& texCoords);
        
        // Utility methods
        static std::vector<std::string> SplitString(const std::string& str, char delimiter);
        static std::string TrimString(const std::string& str);
        static void CalculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    };
}