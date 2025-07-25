#include "Resource/MeshLoader.h"
#include "Resource/MTLLoader.h"
#include "Core/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>

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
    
    MeshLoader::OBJLoadResult MeshLoader::LoadOBJWithMaterials(const std::string& filepath) {
        Logger::GetInstance().Info("Loading OBJ file with materials: " + filepath);
        
        if (!IsOBJFile(filepath)) {
            OBJLoadResult result;
            result.errorMessage = "File is not a valid OBJ file: " + filepath;
            Logger::GetInstance().Error("MeshLoader::LoadOBJWithMaterials: " + result.errorMessage);
            return result;
        }
        
        return LoadOBJWithMaterialsImpl(filepath);
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
        
        // Set mesh properties
        if (!meshData.groupName.empty() || !meshData.objectName.empty()) {
            std::string name = meshData.objectName.empty() ? meshData.groupName : meshData.objectName;
            if (!meshData.groupName.empty() && !meshData.objectName.empty()) {
                name = meshData.objectName + "_" + meshData.groupName;
            }
            mesh->SetName(name);
        }
        
        // Set material if available
        if (meshData.material) {
            mesh->SetMaterial(meshData.material);
        }
        
        Logger::GetInstance().Log(LogLevel::Info, "Created mesh '" + mesh->GetName() + "' with " + 
                                 std::to_string(meshData.vertices.size()) + " vertices and " + 
                                 std::to_string(meshData.indices.size()) + " indices");
        
        return mesh;
    }
    
    std::vector<std::shared_ptr<Mesh>> MeshLoader::CreateMeshesFromResult(const OBJLoadResult& result) {
        std::vector<std::shared_ptr<Mesh>> meshes;
        
        if (!result.success) {
            Logger::GetInstance().Error("Cannot create meshes from failed OBJ load result");
            return meshes;
        }
        
        meshes.reserve(result.meshes.size());
        
        for (const auto& meshData : result.meshes) {
            auto mesh = CreateMeshFromData(meshData);
            if (mesh) {
                meshes.push_back(mesh);
            }
        }
        
        Logger::GetInstance().Info("Created " + std::to_string(meshes.size()) + " meshes from OBJ load result");
        
        return meshes;
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
    
    MeshLoader::OBJLoadResult MeshLoader::LoadOBJWithMaterialsImpl(const std::string& filepath) {
        OBJLoadResult result;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            result.errorMessage = "Could not open file: " + filepath;
            Logger::GetInstance().Error("MeshLoader::LoadOBJWithMaterialsImpl: " + result.errorMessage);
            return result;
        }
        
        std::string basePath = GetDirectoryPath(filepath);
        OBJParseState state;
        state.currentMesh.objectName = "default";
        state.currentMesh.groupName = "default";
        
        std::string line;
        int lineNumber = 0;
        
        while (std::getline(file, line)) {
            lineNumber++;
            line = TrimString(line);
            
            if (line.empty() || line[0] == '#') {
                continue; // Skip empty lines and comments
            }
            
            if (!ParseOBJLine(line, state, basePath)) {
                Logger::GetInstance().Warning("Warning: Could not parse line " + std::to_string(lineNumber) + 
                                             " in file " + filepath + ": " + line);
            }
        }
        
        file.close();
        
        // Finalize the last mesh if it has faces
        if (state.hasFaces) {
            FinalizeMesh(state);
        }
        
        // Calculate statistics
        for (const auto& meshData : state.meshes) {
            result.totalVertices += static_cast<uint32_t>(meshData.vertices.size());
            result.totalTriangles += static_cast<uint32_t>(meshData.indices.size() / 3);
        }
        
        // Calculate loading time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        result.meshes = std::move(state.meshes);
        result.materials = std::move(state.materials);
        result.success = !result.meshes.empty();
        
        if (result.success) {
            Logger::GetInstance().Info("Successfully loaded OBJ file with materials: " + filepath + 
                                     " (" + std::to_string(result.meshes.size()) + " meshes, " +
                                     std::to_string(result.materials.size()) + " materials, " +
                                     std::to_string(result.totalVertices) + " vertices, " +
                                     std::to_string(result.totalTriangles) + " triangles, " +
                                     std::to_string(result.loadingTimeMs) + "ms)");
        } else {
            result.errorMessage = "No valid meshes found in OBJ file: " + filepath;
            Logger::GetInstance().Warning("MeshLoader::LoadOBJWithMaterialsImpl: " + result.errorMessage);
        }
        
        return result;
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
    
    bool MeshLoader::ParseOBJLine(const std::string& line, OBJParseState& state, const std::string& basePath) {
        if (line.length() < 2) return false;
        
        if (line.substr(0, 2) == "v ") {
            // Vertex position
            Math::Vec3 vertex;
            if (ParseVertex(line, vertex)) {
                state.positions.push_back(vertex);
                return true;
            }
        }
        else if (line.substr(0, 3) == "vn ") {
            // Vertex normal
            Math::Vec3 normal;
            if (ParseNormal(line, normal)) {
                state.normals.push_back(normal);
                return true;
            }
        }
        else if (line.substr(0, 3) == "vt ") {
            // Texture coordinate
            Math::Vec2 texCoord;
            if (ParseTexCoord(line, texCoord)) {
                state.texCoords.push_back(texCoord);
                return true;
            }
        }
        else if (line.substr(0, 2) == "f ") {
            // Face
            return ParseFace(line, state);
        }
        else if (line.substr(0, 7) == "mtllib ") {
            // Material library
            return ParseMaterialLib(line, state, basePath);
        }
        else if (line.substr(0, 7) == "usemtl ") {
            // Use material
            return ParseUseMaterial(line, state);
        }
        else if (line.substr(0, 2) == "g ") {
            // Group
            return ParseGroup(line, state);
        }
        else if (line.substr(0, 2) == "o ") {
            // Object
            return ParseObject(line, state);
        }
        else if (line.substr(0, 2) == "s ") {
            // Smoothing group (ignored for now)
            return true;
        }
        
        return true; // Other lines are ignored but not considered errors
    }
    
    bool MeshLoader::ParseMaterialLib(const std::string& line, OBJParseState& state, const std::string& basePath) {
        std::string mtlFilename = line.substr(7); // Skip "mtllib "
        mtlFilename = TrimString(mtlFilename);
        
        std::string mtlPath = MTLLoader::FindMTLFile(basePath + "/dummy.obj", mtlFilename);
        if (mtlPath.empty()) {
            Logger::GetInstance().Warning("MTL file not found: " + mtlFilename);
            return false;
        }
        
        MTLLoader mtlLoader;
        mtlLoader.SetVerboseLogging(false); // Keep it quiet unless debugging
        auto mtlResult = mtlLoader.LoadMTL(mtlPath);
        
        if (mtlResult.success) {
            // Convert MTL materials to engine materials
            for (const auto& pair : mtlResult.materials) {
                auto engineMaterial = mtlLoader.ConvertToEngineMaterial(pair.second, basePath);
                if (engineMaterial) {
                    state.materials[pair.first] = engineMaterial;
                }
            }
            
            Logger::GetInstance().Debug("Loaded " + std::to_string(mtlResult.materialCount) + 
                                       " materials from " + mtlPath);
            return true;
        } else {
            Logger::GetInstance().Warning("Failed to load MTL file: " + mtlPath + " - " + mtlResult.errorMessage);
            return false;
        }
    }
    
    bool MeshLoader::ParseUseMaterial(const std::string& line, OBJParseState& state) {
        std::string materialName = line.substr(7); // Skip "usemtl "
        materialName = TrimString(materialName);
        
        // If we're switching materials and have faces, finalize current mesh
        if (state.hasFaces && state.currentMaterial != materialName) {
            FinalizeMesh(state);
            StartNewMesh(state);
        }
        
        state.currentMaterial = materialName;
        state.currentMesh.materialName = materialName;
        
        // Set material if we have it loaded
        auto it = state.materials.find(materialName);
        if (it != state.materials.end()) {
            state.currentMesh.material = it->second;
        }
        
        return true;
    }
    
    bool MeshLoader::ParseGroup(const std::string& line, OBJParseState& state) {
        std::string groupName = line.substr(2); // Skip "g "
        groupName = TrimString(groupName);
        
        if (groupName.empty()) {
            groupName = "default";
        }
        
        // If we're switching groups and have faces, finalize current mesh
        if (state.hasFaces && state.currentGroup != groupName) {
            FinalizeMesh(state);
            StartNewMesh(state);
        }
        
        state.currentGroup = groupName;
        state.currentMesh.groupName = groupName;
        
        return true;
    }
    
    bool MeshLoader::ParseObject(const std::string& line, OBJParseState& state) {
        std::string objectName = line.substr(2); // Skip "o "
        objectName = TrimString(objectName);
        
        if (objectName.empty()) {
            objectName = "default";
        }
        
        // If we're switching objects and have faces, finalize current mesh
        if (state.hasFaces && state.currentObject != objectName) {
            FinalizeMesh(state);
            StartNewMesh(state);
        }
        
        state.currentObject = objectName;
        state.currentMesh.objectName = objectName;
        
        return true;
    }
    
    bool MeshLoader::ParseFace(const std::string& line, OBJParseState& state) {
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
                if (posIndex >= 0 && posIndex < static_cast<int>(state.positions.size())) {
                    vertex.position = state.positions[posIndex];
                } else {
                    return false; // Invalid position index
                }
                
                // Texture coordinate index (optional)
                if (indices.size() > 1 && !indices[1].empty()) {
                    int texIndex = std::stoi(indices[1]) - 1;
                    if (texIndex >= 0 && texIndex < static_cast<int>(state.texCoords.size())) {
                        vertex.texCoords = state.texCoords[texIndex];
                    }
                }
                
                // Normal index (optional)
                if (indices.size() > 2 && !indices[2].empty()) {
                    int normalIndex = std::stoi(indices[2]) - 1;
                    if (normalIndex >= 0 && normalIndex < static_cast<int>(state.normals.size())) {
                        vertex.normal = state.normals[normalIndex];
                    }
                }
                
                // If no normal was provided, we'll calculate it later
                if (vertex.normal.x == 0.0f && vertex.normal.y == 0.0f && vertex.normal.z == 0.0f) {
                    vertex.normal = Math::Vec3(0.0f, 1.0f, 0.0f); // Default up normal
                }
                
                state.currentMesh.vertices.push_back(vertex);
                state.currentMesh.indices.push_back(static_cast<uint32_t>(state.currentMesh.vertices.size() - 1));
            }
        }
        
        state.hasFaces = true;
        return true;
    }
    
    void MeshLoader::FinalizeMesh(OBJParseState& state) {
        if (state.currentMesh.vertices.empty()) {
            return;
        }
        
        // Validate the mesh and collect any issues
        std::vector<std::string> validationErrors;
        bool isValid = ValidateOBJMesh(state.currentMesh, validationErrors);
        
        // Log validation results
        if (!validationErrors.empty()) {
            for (const auto& error : validationErrors) {
                if (error.find("degenerate") != std::string::npos || 
                    error.find("normals") != std::string::npos ||
                    error.find("UV coordinates") != std::string::npos ||
                    error.find("large") != std::string::npos ||
                    error.find("small") != std::string::npos) {
                    Logger::GetInstance().Debug("Mesh validation: " + error);
                } else {
                    Logger::GetInstance().Warning("Mesh validation: " + error);
                }
            }
        }
        
        // Only proceed if the mesh has basic validity (vertices and indices)
        if (!isValid) {
            Logger::GetInstance().Error("Mesh failed validation, skipping");
            return;
        }
        
        // Optimize the mesh (removes degenerate triangles, generates normals, etc.)
        OptimizeOBJMesh(state.currentMesh);
        
        // Generate fallback UV coordinates if needed
        bool hasValidUVs = false;
        for (const auto& vertex : state.currentMesh.vertices) {
            if (vertex.texCoords.x != 0.0f || vertex.texCoords.y != 0.0f) {
                hasValidUVs = true;
                break;
            }
        }
        
        if (!hasValidUVs) {
            Logger::GetInstance().Debug("Generating fallback UV coordinates");
            // Generate simple planar UV coordinates
            if (!state.currentMesh.vertices.empty()) {
                Math::Vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
                for (const auto& vertex : state.currentMesh.vertices) {
                    minPos = glm::min(minPos, vertex.position);
                    maxPos = glm::max(maxPos, vertex.position);
                }
                
                Math::Vec3 size = maxPos - minPos;
                float maxDimension = std::max({size.x, size.y, size.z});
                
                if (maxDimension > 0.0f) {
                    for (auto& vertex : state.currentMesh.vertices) {
                        vertex.texCoords.x = (vertex.position.x - minPos.x) / maxDimension;
                        vertex.texCoords.y = (vertex.position.z - minPos.z) / maxDimension;
                        vertex.texCoords.x = std::max(0.0f, std::min(1.0f, vertex.texCoords.x));
                        vertex.texCoords.y = std::max(0.0f, std::min(1.0f, vertex.texCoords.y));
                    }
                }
            }
        }
        
        state.currentMesh.isValid = true;
        state.meshes.push_back(state.currentMesh);
        
        Logger::GetInstance().Debug("Finalized mesh: object='" + state.currentMesh.objectName + 
                                   "', group='" + state.currentMesh.groupName + 
                                   "', material='" + state.currentMesh.materialName + 
                                   "', vertices=" + std::to_string(state.currentMesh.vertices.size()) +
                                   ", triangles=" + std::to_string(state.currentMesh.indices.size() / 3));
    }
    
    void MeshLoader::StartNewMesh(OBJParseState& state) {
        state.currentMesh = MeshData();
        state.currentMesh.objectName = state.currentObject;
        state.currentMesh.groupName = state.currentGroup;
        state.currentMesh.materialName = state.currentMaterial;
        
        // Set material if we have it loaded
        auto it = state.materials.find(state.currentMaterial);
        if (it != state.materials.end()) {
            state.currentMesh.material = it->second;
        }
        
        state.hasFaces = false;
    }
    
    std::string MeshLoader::GetDirectoryPath(const std::string& filepath) {
        size_t pos = filepath.find_last_of("/\\");
        if (pos != std::string::npos) {
            return filepath.substr(0, pos);
        }
        return "."; // Current directory
    }

    bool MeshLoader::ValidateOBJMesh(const MeshData& meshData, std::vector<std::string>& errors) {
        errors.clear();
        bool isValid = true;
        
        // Check if mesh has vertices
        if (meshData.vertices.empty()) {
            errors.push_back("Mesh has no vertices");
            isValid = false;
        }
        
        // Check if mesh has indices
        if (meshData.indices.empty()) {
            errors.push_back("Mesh has no indices");
            isValid = false;
        }
        
        // Check if indices are valid
        for (size_t i = 0; i < meshData.indices.size(); ++i) {
            if (meshData.indices[i] >= meshData.vertices.size()) {
                errors.push_back("Invalid index " + std::to_string(meshData.indices[i]) + 
                                " at position " + std::to_string(i) + 
                                " (vertex count: " + std::to_string(meshData.vertices.size()) + ")");
                isValid = false;
            }
        }
        
        // Check if indices form complete triangles
        if (meshData.indices.size() % 3 != 0) {
            errors.push_back("Index count (" + std::to_string(meshData.indices.size()) + 
                           ") is not divisible by 3 (incomplete triangles)");
            isValid = false;
        }
        
        // Check for degenerate triangles
        uint32_t degenerateCount = 0;
        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            if (i + 2 < meshData.indices.size()) {
                uint32_t i0 = meshData.indices[i];
                uint32_t i1 = meshData.indices[i + 1];
                uint32_t i2 = meshData.indices[i + 2];
                
                // Check for duplicate indices in triangle
                if (i0 == i1 || i1 == i2 || i0 == i2) {
                    degenerateCount++;
                    continue;
                }
                
                // Check for zero-area triangles
                if (i0 < meshData.vertices.size() && i1 < meshData.vertices.size() && i2 < meshData.vertices.size()) {
                    const Math::Vec3& v0 = meshData.vertices[i0].position;
                    const Math::Vec3& v1 = meshData.vertices[i1].position;
                    const Math::Vec3& v2 = meshData.vertices[i2].position;
                    
                    Math::Vec3 edge1 = v1 - v0;
                    Math::Vec3 edge2 = v2 - v0;
                    Math::Vec3 cross = glm::cross(edge1, edge2);
                    float area = glm::length(cross) * 0.5f;
                    
                    if (area < 1e-6f) {
                        degenerateCount++;
                    }
                }
            }
        }
        
        if (degenerateCount > 0) {
            errors.push_back("Found " + std::to_string(degenerateCount) + " degenerate triangles");
            // This is a warning, not a fatal error
        }
        
        // Check for valid normals
        bool hasValidNormals = false;
        for (const auto& vertex : meshData.vertices) {
            float normalLength = glm::length(vertex.normal);
            if (normalLength > 0.1f) { // Allow some tolerance
                hasValidNormals = true;
                break;
            }
        }
        
        if (!hasValidNormals) {
            errors.push_back("Mesh has no valid normals (will be generated automatically)");
            // This is a warning, not a fatal error
        }
        
        // Check for valid UV coordinates
        bool hasValidUVs = false;
        for (const auto& vertex : meshData.vertices) {
            if (vertex.texCoords.x != 0.0f || vertex.texCoords.y != 0.0f) {
                hasValidUVs = true;
                break;
            }
        }
        
        if (!hasValidUVs) {
            errors.push_back("Mesh has no valid UV coordinates (fallback UVs will be generated)");
            // This is a warning, not a fatal error
        }
        
        // Check for extreme vertex positions (potential scale issues)
        Math::Vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
        for (const auto& vertex : meshData.vertices) {
            minPos = glm::min(minPos, vertex.position);
            maxPos = glm::max(maxPos, vertex.position);
        }
        
        Math::Vec3 size = maxPos - minPos;
        float maxDimension = std::max({size.x, size.y, size.z});
        
        if (maxDimension > 1000.0f) {
            errors.push_back("Mesh is very large (max dimension: " + std::to_string(maxDimension) + 
                           ") - consider scaling down");
        } else if (maxDimension < 0.001f) {
            errors.push_back("Mesh is very small (max dimension: " + std::to_string(maxDimension) + 
                           ") - consider scaling up");
        }
        
        return isValid;
    }
    
    void MeshLoader::OptimizeOBJMesh(MeshData& meshData) {
        if (!meshData.isValid || meshData.vertices.empty()) {
            return;
        }
        
        Logger::GetInstance().Debug("Optimizing OBJ mesh with " + std::to_string(meshData.vertices.size()) + " vertices");
        
        // Remove degenerate triangles
        std::vector<uint32_t> validIndices;
        validIndices.reserve(meshData.indices.size());
        
        uint32_t removedTriangles = 0;
        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            if (i + 2 < meshData.indices.size()) {
                uint32_t i0 = meshData.indices[i];
                uint32_t i1 = meshData.indices[i + 1];
                uint32_t i2 = meshData.indices[i + 2];
                
                // Skip triangles with duplicate indices
                if (i0 == i1 || i1 == i2 || i0 == i2) {
                    removedTriangles++;
                    continue;
                }
                
                // Skip zero-area triangles
                if (i0 < meshData.vertices.size() && i1 < meshData.vertices.size() && i2 < meshData.vertices.size()) {
                    const Math::Vec3& v0 = meshData.vertices[i0].position;
                    const Math::Vec3& v1 = meshData.vertices[i1].position;
                    const Math::Vec3& v2 = meshData.vertices[i2].position;
                    
                    Math::Vec3 edge1 = v1 - v0;
                    Math::Vec3 edge2 = v2 - v0;
                    Math::Vec3 cross = glm::cross(edge1, edge2);
                    float area = glm::length(cross) * 0.5f;
                    
                    if (area < 1e-6f) {
                        removedTriangles++;
                        continue;
                    }
                }
                
                // Triangle is valid, keep it
                validIndices.push_back(i0);
                validIndices.push_back(i1);
                validIndices.push_back(i2);
            }
        }
        
        if (removedTriangles > 0) {
            meshData.indices = std::move(validIndices);
            Logger::GetInstance().Debug("Removed " + std::to_string(removedTriangles) + " degenerate triangles");
        }
        
        // Generate normals if needed
        bool hasValidNormals = false;
        for (const auto& vertex : meshData.vertices) {
            if (glm::length(vertex.normal) > 0.1f) {
                hasValidNormals = true;
                break;
            }
        }
        
        if (!hasValidNormals) {
            GenerateNormalsForOBJMesh(meshData);
        }
        
        // Calculate tangents
        if (!meshData.indices.empty()) {
            CalculateTangents(meshData.vertices, meshData.indices);
        }
        
        Logger::GetInstance().Debug("OBJ mesh optimization complete");
    }
    
    void MeshLoader::GenerateNormalsForOBJMesh(MeshData& meshData) {
        if (meshData.vertices.empty() || meshData.indices.empty()) {
            return;
        }
        
        Logger::GetInstance().Debug("Generating normals for OBJ mesh");
        
        // Initialize all normals to zero
        for (auto& vertex : meshData.vertices) {
            vertex.normal = Math::Vec3(0.0f);
        }
        
        // Calculate face normals and accumulate to vertices
        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            if (i + 2 < meshData.indices.size()) {
                uint32_t i0 = meshData.indices[i];
                uint32_t i1 = meshData.indices[i + 1];
                uint32_t i2 = meshData.indices[i + 2];
                
                if (i0 < meshData.vertices.size() && i1 < meshData.vertices.size() && i2 < meshData.vertices.size()) {
                    const Math::Vec3& v0 = meshData.vertices[i0].position;
                    const Math::Vec3& v1 = meshData.vertices[i1].position;
                    const Math::Vec3& v2 = meshData.vertices[i2].position;
                    
                    Math::Vec3 edge1 = v1 - v0;
                    Math::Vec3 edge2 = v2 - v0;
                    Math::Vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
                    
                    // Accumulate face normal to all vertices of the triangle
                    meshData.vertices[i0].normal += faceNormal;
                    meshData.vertices[i1].normal += faceNormal;
                    meshData.vertices[i2].normal += faceNormal;
                }
            }
        }
        
        // Normalize all vertex normals
        for (auto& vertex : meshData.vertices) {
            if (glm::length(vertex.normal) > 0.0f) {
                vertex.normal = glm::normalize(vertex.normal);
            } else {
                vertex.normal = Math::Vec3(0.0f, 1.0f, 0.0f); // Default up normal
            }
        }
        
        Logger::GetInstance().Debug("Generated normals for " + std::to_string(meshData.vertices.size()) + " vertices");
    }
    
    void MeshLoader::ConvertCoordinateSystem(MeshData& meshData, bool flipYZ, bool flipWinding) {
        if (meshData.vertices.empty()) {
            return;
        }
        
        Logger::GetInstance().Debug("Converting coordinate system for OBJ mesh (flipYZ=" + 
                                   std::string(flipYZ ? "true" : "false") + 
                                   ", flipWinding=" + std::string(flipWinding ? "true" : "false") + ")");
        
        // Convert vertex positions and normals
        for (auto& vertex : meshData.vertices) {
            if (flipYZ) {
                // Swap Y and Z coordinates (common when converting from right-handed to left-handed)
                std::swap(vertex.position.y, vertex.position.z);
                std::swap(vertex.normal.y, vertex.normal.z);
                std::swap(vertex.tangent.y, vertex.tangent.z);
                std::swap(vertex.bitangent.y, vertex.bitangent.z);
                
                // Flip Z to maintain handedness
                vertex.position.z = -vertex.position.z;
                vertex.normal.z = -vertex.normal.z;
                vertex.tangent.z = -vertex.tangent.z;
                vertex.bitangent.z = -vertex.bitangent.z;
            }
        }
        
        // Flip triangle winding order if requested
        if (flipWinding) {
            for (size_t i = 0; i < meshData.indices.size(); i += 3) {
                if (i + 2 < meshData.indices.size()) {
                    std::swap(meshData.indices[i + 1], meshData.indices[i + 2]);
                }
            }
        }
        
        Logger::GetInstance().Debug("Coordinate system conversion complete");
    }
    
    void MeshLoader::ScaleOBJMesh(MeshData& meshData, float scale) {
        ScaleOBJMesh(meshData, Math::Vec3(scale));
    }
    
    void MeshLoader::ScaleOBJMesh(MeshData& meshData, const Math::Vec3& scale) {
        if (meshData.vertices.empty()) {
            return;
        }
        
        Logger::GetInstance().Debug("Scaling OBJ mesh by (" + 
                                   std::to_string(scale.x) + ", " + 
                                   std::to_string(scale.y) + ", " + 
                                   std::to_string(scale.z) + ")");
        
        // Scale vertex positions
        for (auto& vertex : meshData.vertices) {
            vertex.position.x *= scale.x;
            vertex.position.y *= scale.y;
            vertex.position.z *= scale.z;
        }
        
        // If scaling is non-uniform, we need to recalculate normals
        if (std::abs(scale.x - scale.y) > 1e-6f || 
            std::abs(scale.y - scale.z) > 1e-6f || 
            std::abs(scale.x - scale.z) > 1e-6f) {
            Logger::GetInstance().Debug("Non-uniform scaling detected, regenerating normals");
            GenerateNormalsForOBJMesh(meshData);
        }
        
        Logger::GetInstance().Debug("Mesh scaling complete");
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