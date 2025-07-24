#include "Resource/MeshLoader.h"
#include "Core/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace GameEngine {
    
    MeshLoader::MeshData MeshLoader::LoadOBJ(const std::string& filepath) {
        Logger::GetInstance().Log(LogLevel::Info, "Loading OBJ file: " + filepath);
        
        if (!IsOBJFile(filepath)) {
            MeshData data;
            data.isValid = false;
            data.errorMessage = "File is not a valid OBJ file: " + filepath;
            Logger::GetInstance().Log(LogLevel::Error, data.errorMessage);
            return data;
        }
        
        return LoadOBJImpl(filepath);
    }
    
    bool MeshLoader::IsOBJFile(const std::string& filepath) {
        if (filepath.length() < 4) return false;
        
        std::string extension = filepath.substr(filepath.length() - 4);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension == ".obj";
    }
    
    std::shared_ptr<Mesh> MeshLoader::CreateMeshFromData(const MeshData& meshData) {
        if (!meshData.isValid) {
            Logger::GetInstance().Log(LogLevel::Error, "Cannot create mesh from invalid data");
            return CreateDefaultCube();
        }
        
        auto mesh = std::make_shared<Mesh>();
        mesh->SetVertices(meshData.vertices);
        mesh->SetIndices(meshData.indices);
        
        Logger::GetInstance().Log(LogLevel::Info, "Created mesh with " + std::to_string(meshData.vertices.size()) + 
                                 " vertices and " + std::to_string(meshData.indices.size()) + " indices");
        
        return mesh;
    }
    
    MeshLoader::MeshData MeshLoader::CreateDefaultCubeData() {
        MeshData cubeData;
        
        // Default cube vertices with positions, normals, and texture coordinates
        cubeData.vertices = {
            // Front face
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            
            // Back face
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            
            // Left face
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            
            // Right face
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            
            // Bottom face
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            
            // Top face
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}
        };
        
        // Cube indices
        cubeData.indices = {
            0,  1,  2,   2,  3,  0,   // Front face
            4,  5,  6,   6,  7,  4,   // Back face
            8,  9,  10,  10, 11, 8,   // Left face
            12, 13, 14,  14, 15, 12,  // Right face
            16, 17, 18,  18, 19, 16,  // Bottom face
            20, 21, 22,  22, 23, 20   // Top face
        };
        
        // Calculate tangents for the cube
        CalculateTangents(cubeData.vertices, cubeData.indices);
        
        cubeData.isValid = true;
        cubeData.errorMessage = "";
        
        return cubeData;
    }
    
    std::shared_ptr<Mesh> MeshLoader::CreateDefaultCube() {
        Logger::GetInstance().Log(LogLevel::Info, "Creating default cube mesh");
        
        // Get the cube data without OpenGL calls
        MeshData cubeData = CreateDefaultCubeData();
        
        // Create mesh from data (this will involve OpenGL calls)
        auto mesh = std::make_shared<Mesh>();
        mesh->SetVertices(cubeData.vertices);
        mesh->SetIndices(cubeData.indices);
        
        return mesh;
    }
    
    MeshLoader::MeshData MeshLoader::LoadOBJImpl(const std::string& filepath) {
        MeshData meshData;
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            meshData.isValid = false;
            meshData.errorMessage = "Could not open file: " + filepath;
            Logger::GetInstance().Log(LogLevel::Error, meshData.errorMessage);
            return meshData;
        }
        
        std::vector<Math::Vec3> positions;
        std::vector<Math::Vec3> normals;
        std::vector<Math::Vec2> texCoords;
        
        std::string line;
        int lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            line = TrimString(line);
            
            if (line.empty() || line[0] == '#') {
                continue; // Skip empty lines and comments
            }
            
            if (!ParseOBJLine(line, meshData, positions, normals, texCoords)) {
                Logger::GetInstance().Log(LogLevel::Warning, "Warning: Could not parse line " + std::to_string(lineNumber) + 
                                         " in file " + filepath + ": " + line);
            }
        }
        
        file.close();
        
        if (meshData.vertices.empty()) {
            meshData.isValid = false;
            meshData.errorMessage = "No valid vertices found in OBJ file: " + filepath;
            Logger::GetInstance().Log(LogLevel::Error, meshData.errorMessage);
            return meshData;
        }
        
        // Validate and fix UV coordinates
        bool hasValidUVs = false;
        for (const auto& vertex : meshData.vertices) {
            if (vertex.texCoords.x != 0.0f || vertex.texCoords.y != 0.0f) {
                hasValidUVs = true;
                break;
            }
        }
        
        if (!hasValidUVs) {
            Logger::GetInstance().Log(LogLevel::Warning, "OBJ file has no valid UV coordinates, generating fallback UVs");
            // Generate simple planar UV coordinates
            if (!meshData.vertices.empty()) {
                Math::Vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
                for (const auto& vertex : meshData.vertices) {
                    minPos = glm::min(minPos, vertex.position);
                    maxPos = glm::max(maxPos, vertex.position);
                }
                
                Math::Vec3 size = maxPos - minPos;
                float maxDimension = std::max({size.x, size.y, size.z});
                
                if (maxDimension > 0.0f) {
                    for (auto& vertex : meshData.vertices) {
                        vertex.texCoords.x = (vertex.position.x - minPos.x) / maxDimension;
                        vertex.texCoords.y = (vertex.position.z - minPos.z) / maxDimension;
                        vertex.texCoords.x = std::max(0.0f, std::min(1.0f, vertex.texCoords.x));
                        vertex.texCoords.y = std::max(0.0f, std::min(1.0f, vertex.texCoords.y));
                    }
                }
            }
        }
        
        // Calculate tangents if we have texture coordinates
        if (!meshData.indices.empty()) {
            CalculateTangents(meshData.vertices, meshData.indices);
        }
        
        meshData.isValid = true;
        Logger::GetInstance().Log(LogLevel::Info, "Successfully loaded OBJ file: " + filepath + 
                                 " (" + std::to_string(meshData.vertices.size()) + " vertices, " +
                                 std::to_string(meshData.indices.size()) + " indices)" +
                                 (hasValidUVs ? " with UV coordinates" : " with generated UV coordinates"));
        
        return meshData;
    }    

    bool MeshLoader::ParseOBJLine(const std::string& line, MeshData& meshData,
                                 std::vector<Math::Vec3>& positions,
                                 std::vector<Math::Vec3>& normals,
                                 std::vector<Math::Vec2>& texCoords) {
        if (line.length() < 2) return false;
        
        if (line.substr(0, 2) == "v ") {
            // Vertex position
            Math::Vec3 vertex;
            if (ParseVertex(line, vertex)) {
                positions.push_back(vertex);
                return true;
            }
        }
        else if (line.substr(0, 3) == "vn ") {
            // Vertex normal
            Math::Vec3 normal;
            if (ParseNormal(line, normal)) {
                normals.push_back(normal);
                return true;
            }
        }
        else if (line.substr(0, 3) == "vt ") {
            // Texture coordinate
            Math::Vec2 texCoord;
            if (ParseTexCoord(line, texCoord)) {
                texCoords.push_back(texCoord);
                return true;
            }
        }
        else if (line.substr(0, 2) == "f ") {
            // Face
            return ParseFace(line, meshData, positions, normals, texCoords);
        }
        
        return true; // Other lines are ignored but not considered errors
    }
    
    bool MeshLoader::ParseVertex(const std::string& line, Math::Vec3& vertex) {
        std::istringstream iss(line.substr(2)); // Skip "v "
        return !!(iss >> vertex.x >> vertex.y >> vertex.z);
    }
    
    bool MeshLoader::ParseNormal(const std::string& line, Math::Vec3& normal) {
        std::istringstream iss(line.substr(3)); // Skip "vn "
        return !!(iss >> normal.x >> normal.y >> normal.z);
    }
    
    bool MeshLoader::ParseTexCoord(const std::string& line, Math::Vec2& texCoord) {
        std::istringstream iss(line.substr(3)); // Skip "vt "
        bool success = !!(iss >> texCoord.x >> texCoord.y);
        
        if (success) {
            // Flip V coordinate for OpenGL (OBJ uses bottom-left origin, OpenGL uses top-left)
            texCoord.y = 1.0f - texCoord.y;
            
            // Clamp to valid range
            texCoord.x = std::max(0.0f, std::min(1.0f, texCoord.x));
            texCoord.y = std::max(0.0f, std::min(1.0f, texCoord.y));
        }
        
        return success;
    }
    
    bool MeshLoader::ParseFace(const std::string& line, MeshData& meshData,
                              const std::vector<Math::Vec3>& positions,
                              const std::vector<Math::Vec3>& normals,
                              const std::vector<Math::Vec2>& texCoords) {
        std::string faceData = line.substr(2); // Skip "f "
        std::vector<std::string> vertices = SplitString(faceData, ' ');
        
        if (vertices.size() < 3) {
            return false; // Need at least 3 vertices for a triangle
        }
        
        // Convert face to triangles (fan triangulation for polygons)
        for (size_t i = 1; i < vertices.size() - 1; ++i) {
            // Create triangle: vertex[0], vertex[i], vertex[i+1]
            std::vector<std::string> triangleVertices = {vertices[0], vertices[i], vertices[i + 1]};
            
            for (const std::string& vertexStr : triangleVertices) {
                Vertex vertex = {};
                
                // Parse vertex indices (format: v/vt/vn or v//vn or v/vt or v)
                std::vector<std::string> indices = SplitString(vertexStr, '/');
                
                if (indices.empty()) continue;
                
                // Position index (required)
                int posIndex = std::stoi(indices[0]) - 1; // OBJ indices are 1-based
                if (posIndex >= 0 && posIndex < static_cast<int>(positions.size())) {
                    vertex.position = positions[posIndex];
                } else {
                    return false; // Invalid position index
                }
                
                // Texture coordinate index (optional)
                if (indices.size() > 1 && !indices[1].empty()) {
                    int texIndex = std::stoi(indices[1]) - 1;
                    if (texIndex >= 0 && texIndex < static_cast<int>(texCoords.size())) {
                        vertex.texCoords = texCoords[texIndex];
                    }
                }
                
                // Normal index (optional)
                if (indices.size() > 2 && !indices[2].empty()) {
                    int normalIndex = std::stoi(indices[2]) - 1;
                    if (normalIndex >= 0 && normalIndex < static_cast<int>(normals.size())) {
                        vertex.normal = normals[normalIndex];
                    }
                }
                
                // If no normal was provided, we'll calculate it later
                if (vertex.normal.x == 0.0f && vertex.normal.y == 0.0f && vertex.normal.z == 0.0f) {
                    vertex.normal = Math::Vec3(0.0f, 1.0f, 0.0f); // Default up normal
                }
                
                meshData.vertices.push_back(vertex);
                meshData.indices.push_back(static_cast<uint32_t>(meshData.vertices.size() - 1));
            }
        }
        
        return true;
    }
    
    std::vector<std::string> MeshLoader::SplitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, delimiter)) {
            token = TrimString(token);
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        
        return tokens;
    }
    
    std::string MeshLoader::TrimString(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
    
    void MeshLoader::CalculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        // Initialize tangents and bitangents to zero
        for (auto& vertex : vertices) {
            vertex.tangent = Math::Vec3(0.0f);
            vertex.bitangent = Math::Vec3(0.0f);
        }
        
        // Calculate tangents for each triangle
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 >= indices.size()) break;
            
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];
            
            if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) continue;
            
            Vertex& v0 = vertices[i0];
            Vertex& v1 = vertices[i1];
            Vertex& v2 = vertices[i2];
            
            Math::Vec3 edge1 = v1.position - v0.position;
            Math::Vec3 edge2 = v2.position - v0.position;
            
            Math::Vec2 deltaUV1 = v1.texCoords - v0.texCoords;
            Math::Vec2 deltaUV2 = v2.texCoords - v0.texCoords;
            
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            
            if (std::isfinite(f)) {
                Math::Vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
                Math::Vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
                
                v0.tangent = v0.tangent + tangent;
                v1.tangent = v1.tangent + tangent;
                v2.tangent = v2.tangent + tangent;
                
                v0.bitangent = v0.bitangent + bitangent;
                v1.bitangent = v1.bitangent + bitangent;
                v2.bitangent = v2.bitangent + bitangent;
            }
        }
        
        // Normalize tangents and bitangents
        for (auto& vertex : vertices) {
            if (glm::length(vertex.tangent) > 0.0f) {
                vertex.tangent = glm::normalize(vertex.tangent);
            } else {
                vertex.tangent = Math::Vec3(1.0f, 0.0f, 0.0f); // Default tangent
            }
            
            if (glm::length(vertex.bitangent) > 0.0f) {
                vertex.bitangent = glm::normalize(vertex.bitangent);
            } else {
                vertex.bitangent = Math::Vec3(0.0f, 0.0f, 1.0f); // Default bitangent
            }
        }
    }
}