#include "Resource/FBXLoader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

namespace GameEngine {

FBXLoader::FBXLoader() 
    : m_initialized(false) {
    // Set default configuration
    m_config.convertToOpenGLCoordinates = true;
    m_config.importMaterials = true;
    m_config.importTextures = true;
    m_config.optimizeMeshes = true;
    m_config.generateMissingNormals = true;
    m_config.generateTangents = true;
    m_config.importScale = 1.0f;
}

FBXLoader::~FBXLoader() {
    Shutdown();
}

bool FBXLoader::Initialize() {
    if (m_initialized) {
        return true;
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    try {
        m_importer = std::make_unique<Assimp::Importer>();
        
        m_initialized = true;
        Logger::GetInstance().Info("FBXLoader initialized successfully");
        
        return true;
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to initialize FBXLoader: " + std::string(e.what()));
        return false;
    }
#else
    Logger::GetInstance().Error("FBXLoader: Assimp not available");
    return false;
#endif
}

void FBXLoader::Shutdown() {
    if (!m_initialized) {
        return;
    }

    ClearTextureCache();

#ifdef GAMEENGINE_HAS_ASSIMP
    m_importer.reset();
#endif
    
    m_initialized = false;
    Logger::GetInstance().Info("FBXLoader shutdown complete");
}

FBXLoader::FBXLoadResult FBXLoader::LoadFBX(const std::string& filepath) {
    FBXLoadResult result;
    
    if (!m_initialized) {
        result.errorMessage = "FBXLoader not initialized";
        Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
        return result;
    }

    if (!std::filesystem::exists(filepath)) {
        result.errorMessage = "File not found: " + filepath;
        Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
        return result;
    }

    if (!IsFBXFile(filepath)) {
        result.errorMessage = "File is not an FBX file: " + filepath;
        Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
        return result;
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Load the FBX scene with specific post-processing
        const aiScene* scene = m_importer->ReadFile(filepath, GetFBXPostProcessFlags());
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            result.errorMessage = "Assimp FBX error: " + std::string(m_importer->GetErrorString());
            Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
            return result;
        }

        // Validate the FBX scene
        ValidateFBXScene(scene);
        
        // Process the FBX scene
        result = ProcessFBXScene(scene, filepath);
        
        // Calculate loading time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        // Detect source application
        result.sourceApplication = DetectSourceApplicationFromScene(scene);
        
        // Check for skeleton and animations
        result.hasSkeleton = (scene->mNumMeshes > 0 && scene->mMeshes[0]->HasBones());
        result.hasAnimations = (scene->mNumAnimations > 0);
        
        // Log statistics
        LogFBXLoadingStats(result);
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception during FBX loading: " + std::string(e.what());
        Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
    }
#else
    result.errorMessage = "Assimp not available";
    Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
#endif

    return result;
}

FBXLoader::FBXLoadResult FBXLoader::LoadFBXFromMemory(const std::vector<uint8_t>& data) {
    FBXLoadResult result;
    
    if (!m_initialized) {
        result.errorMessage = "FBXLoader not initialized";
        Logger::GetInstance().Error("FBXLoader::LoadFBXFromMemory: " + result.errorMessage);
        return result;
    }

    if (data.empty()) {
        result.errorMessage = "Empty data buffer";
        Logger::GetInstance().Error("FBXLoader::LoadFBXFromMemory: " + result.errorMessage);
        return result;
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Load FBX from memory
        const aiScene* scene = m_importer->ReadFileFromMemory(
            data.data(), 
            data.size(), 
            GetFBXPostProcessFlags(),
            "fbx"
        );
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            result.errorMessage = "Assimp FBX error: " + std::string(m_importer->GetErrorString());
            Logger::GetInstance().Error("FBXLoader::LoadFBXFromMemory: " + result.errorMessage);
            return result;
        }

        // Validate the FBX scene
        ValidateFBXScene(scene);
        
        // Process the FBX scene
        result = ProcessFBXScene(scene, "memory_buffer.fbx");
        
        // Calculate loading time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        // Detect source application
        result.sourceApplication = DetectSourceApplicationFromScene(scene);
        
        // Check for skeleton and animations
        result.hasSkeleton = (scene->mNumMeshes > 0 && scene->mMeshes[0]->HasBones());
        result.hasAnimations = (scene->mNumAnimations > 0);
        
        // Log statistics
        LogFBXLoadingStats(result);
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception during FBX loading from memory: " + std::string(e.what());
        Logger::GetInstance().Error("FBXLoader::LoadFBXFromMemory: " + result.errorMessage);
    }
#else
    result.errorMessage = "Assimp not available";
    Logger::GetInstance().Error("FBXLoader::LoadFBXFromMemory: " + result.errorMessage);
#endif

    return result;
}

void FBXLoader::SetLoadingConfig(const FBXLoadingConfig& config) {
    m_config = config;
}

bool FBXLoader::IsFBXFile(const std::string& filepath) {
    std::string extension = filepath.substr(filepath.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == "fbx";
}

std::string FBXLoader::DetectSourceApplication(const std::string& filepath) {
    // This is a simplified version - in practice, you'd need to parse the FBX file
    // For now, we'll return a generic message
    return "Unknown";
}

#ifdef GAMEENGINE_HAS_ASSIMP

FBXLoader::FBXLoadResult FBXLoader::ProcessFBXScene(const aiScene* scene, const std::string& filepath) {
    FBXLoadResult result;
    
    try {
        // Process materials first if enabled
        if (m_config.importMaterials) {
            result.materials = ProcessFBXMaterials(scene, filepath);
            result.materialCount = static_cast<uint32_t>(result.materials.size());
        }
        
        // Process all meshes in the scene
        ProcessFBXNode(scene->mRootNode, scene, result.meshes);
        
        // Associate materials with meshes
        if (m_config.importMaterials && !result.materials.empty()) {
            for (auto& mesh : result.meshes) {
                if (mesh && mesh->GetMaterialIndex() < result.materials.size()) {
                    mesh->SetMaterial(result.materials[mesh->GetMaterialIndex()]);
                }
            }
        }
        
        // Calculate statistics
        for (const auto& mesh : result.meshes) {
            if (mesh) {
                result.totalVertices += mesh->GetVertexCount();
                result.totalTriangles += mesh->GetTriangleCount();
            }
        }
        
        result.success = !result.meshes.empty();
        
        if (result.success) {
            Logger::GetInstance().Info("Successfully loaded FBX model '" + filepath + "': " + 
                                     std::to_string(result.meshes.size()) + " meshes, " + 
                                     std::to_string(result.totalVertices) + " vertices, " + 
                                     std::to_string(result.totalTriangles) + " triangles, " +
                                     std::to_string(result.materialCount) + " materials");
        } else {
            result.errorMessage = "No valid meshes found in FBX model";
            Logger::GetInstance().Warning("FBXLoader::ProcessFBXScene: " + result.errorMessage);
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Exception during FBX scene processing: " + std::string(e.what());
        Logger::GetInstance().Error("FBXLoader::ProcessFBXScene: " + result.errorMessage);
    }
    
    return result;
}

std::shared_ptr<Mesh> FBXLoader::ProcessFBXMesh(const aiMesh* mesh, const aiScene* scene) {
    if (!mesh) {
        return nullptr;
    }
    
    try {
        auto engineMesh = std::make_shared<Mesh>();
        
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        
        // Process vertices
        vertices.reserve(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            
            // Position (required)
            if (mesh->mVertices) {
                vertex.position = ConvertVector3(mesh->mVertices[i]) * m_config.importScale;
            }
            
            // Normal
            if (mesh->mNormals) {
                vertex.normal = ConvertVector3(mesh->mNormals[i]);
            }
            
            // Texture coordinates (use first set if available)
            if (mesh->mTextureCoords[0]) {
                vertex.texCoords = ConvertVector2(mesh->mTextureCoords[0][i]);
            }
            
            // Tangent
            if (mesh->mTangents) {
                vertex.tangent = ConvertVector3(mesh->mTangents[i]);
            }
            
            // Bitangent
            if (mesh->mBitangents) {
                vertex.bitangent = ConvertVector3(mesh->mBitangents[i]);
            }
            
            // Vertex colors (use first set if available)
            if (mesh->mColors[0]) {
                const aiColor4D& color = mesh->mColors[0][i];
                vertex.color = Math::Vec4(color.r, color.g, color.b, color.a);
            }
            
            // Bone weights and IDs (for skeletal animation)
            if (mesh->HasBones()) {
                // Initialize bone data
                vertex.boneIds = Math::Vec4(0.0f);
                vertex.boneWeights = Math::Vec4(0.0f);
                
                // This will be filled in when processing bones
                // For now, we'll leave it as default values
            }
            
            vertices.push_back(vertex);
        }
        
        // Apply coordinate system conversion if enabled
        if (m_config.convertToOpenGLCoordinates) {
            ApplyCoordinateSystemConversion(vertices);
        }
        
        // Process indices
        indices.reserve(mesh->mNumFaces * 3);
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            
            // We triangulate during post-processing, so faces should have 3 indices
            if (face.mNumIndices == 3) {
                indices.push_back(face.mIndices[0]);
                indices.push_back(face.mIndices[1]);
                indices.push_back(face.mIndices[2]);
            } else {
                Logger::GetInstance().Warning("FBXLoader::ProcessFBXMesh: Non-triangular face with " + 
                                            std::to_string(face.mNumIndices) + " indices found");
            }
        }
        
        // Set mesh data
        engineMesh->SetVertices(vertices);
        engineMesh->SetIndices(indices);
        
        // Set mesh name if available
        if (mesh->mName.length > 0) {
            engineMesh->SetName(std::string(mesh->mName.C_Str()));
        }
        
        // Set material index
        engineMesh->SetMaterialIndex(mesh->mMaterialIndex);
        
        // Apply mesh optimizations if enabled
        if (m_config.optimizeMeshes) {
            if (m_config.generateMissingNormals && !mesh->mNormals) {
                engineMesh->GenerateNormals(true);
            }
            if (m_config.generateTangents && !mesh->mTangents) {
                engineMesh->GenerateTangents();
            }
            engineMesh->OptimizeVertexCache();
        }
        
        Logger::GetInstance().Debug("Processed FBX mesh '" + engineMesh->GetName() + "': " + 
                                  std::to_string(vertices.size()) + " vertices, " + 
                                  std::to_string(indices.size() / 3) + " triangles");
        
        return engineMesh;
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("FBXLoader::ProcessFBXMesh: Exception processing mesh: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<Material>> FBXLoader::ProcessFBXMaterials(const aiScene* scene, const std::string& filepath) {
    std::vector<std::shared_ptr<Material>> materials;
    
    if (!scene || !m_config.importMaterials) {
        return materials;
    }
    
    materials.reserve(scene->mNumMaterials);
    
    for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
        auto material = ProcessFBXMaterial(scene->mMaterials[i], filepath);
        if (material) {
            materials.push_back(material);
        }
    }
    
    Logger::GetInstance().Info("Processed " + std::to_string(materials.size()) + " FBX materials");
    
    return materials;
}

std::shared_ptr<Material> FBXLoader::ProcessFBXMaterial(const aiMaterial* aiMat, const std::string& filepath) {
    if (!aiMat) {
        return nullptr;
    }
    
    try {
        auto material = std::make_shared<Material>();
        
        // Get material name
        aiString name;
        if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            Logger::GetInstance().Debug("Processing FBX material: " + std::string(name.C_Str()));
        }
        
        // Process diffuse properties
        aiColor3D diffuse(1.0f, 1.0f, 1.0f);
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material->SetAlbedo(ConvertFBXColor(diffuse));
        }
        
        // Process specular properties
        aiColor3D specular(1.0f, 1.0f, 1.0f);
        if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS) {
            // Convert specular to roughness (simplified)
            float specularStrength = (specular.r + specular.g + specular.b) / 3.0f;
            material->SetRoughness(1.0f - specularStrength);
        }
        
        // Process metallic properties (if available)
        float metallic = 0.0f;
        if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
            material->SetMetallic(metallic);
        }
        
        // Process roughness properties (if available)
        float roughness = 0.5f;
        if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
            material->SetRoughness(roughness);
        }
        
        // Process textures if enabled
        if (m_config.importTextures) {
            // Diffuse texture
            aiString texturePath;
            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                auto texture = LoadFBXTexture(std::string(texturePath.C_Str()), filepath);
                if (texture) {
                    material->SetTexture("u_diffuseTexture", texture);
                }
            }
            
            // Normal texture
            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
                auto texture = LoadFBXTexture(std::string(texturePath.C_Str()), filepath);
                if (texture) {
                    material->SetTexture("u_normalTexture", texture);
                }
            }
            
            // Specular texture
            if (aiMat->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
                auto texture = LoadFBXTexture(std::string(texturePath.C_Str()), filepath);
                if (texture) {
                    material->SetTexture("u_specularTexture", texture);
                }
            }
        }
        
        return material;
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("FBXLoader::ProcessFBXMaterial: Exception processing material: " + std::string(e.what()));
        return nullptr;
    }
}

void FBXLoader::ApplyCoordinateSystemConversion(std::vector<Vertex>& vertices) const {
    // FBX typically uses a right-handed coordinate system with Y-up
    // OpenGL uses a right-handed coordinate system with Y-up as well
    // However, different applications may export with different conventions
    
    // For Maya/3ds Max FBX files, we typically need to:
    // - No coordinate system change needed for basic cases
    // - May need to flip Z-axis depending on export settings
    
    // This is a simplified conversion - in practice, you might need to
    // check the FBX file's coordinate system metadata
    
    for (auto& vertex : vertices) {
        // For now, we'll apply a simple transformation that works for most cases
        // This can be expanded based on specific requirements
        
        // Example: If we need to flip Z-axis (uncomment if needed)
        // vertex.position.z = -vertex.position.z;
        // vertex.normal.z = -vertex.normal.z;
        
        // The current implementation assumes the coordinate system is already correct
        // or that the FBX exporter handled the conversion properly
    }
}

Math::Mat4 FBXLoader::GetFBXToOpenGLTransform() const {
    // Identity matrix for now - can be customized based on specific needs
    return Math::Mat4(1.0f);
}

void FBXLoader::ProcessFBXNode(const aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Mesh>>& meshes) {
    if (!node) {
        return;
    }
    
    // Process all meshes in this node
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto engineMesh = ProcessFBXMesh(mesh, scene);
        if (engineMesh) {
            meshes.push_back(engineMesh);
        }
    }
    
    // Recursively process child nodes
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        ProcessFBXNode(node->mChildren[i], scene, meshes);
    }
}

std::shared_ptr<Texture> FBXLoader::LoadFBXTexture(const std::string& texturePath, const std::string& modelPath) {
    // Check cache first
    auto it = m_textureCache.find(texturePath);
    if (it != m_textureCache.end()) {
        return it->second;
    }
    
    // Find the actual texture file path
    std::string actualPath = FindTexturePath(texturePath, modelPath);
    
    if (actualPath.empty() || !std::filesystem::exists(actualPath)) {
        Logger::GetInstance().Warning("FBXLoader: Texture not found: " + texturePath);
        return nullptr;
    }
    
    try {
        // Load texture using the engine's texture system
        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromFile(actualPath)) {
            m_textureCache[texturePath] = texture;
            Logger::GetInstance().Debug("Loaded FBX texture: " + actualPath);
            return texture;
        } else {
            Logger::GetInstance().Warning("Failed to load FBX texture: " + actualPath);
            return nullptr;
        }
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Exception loading FBX texture '" + actualPath + "': " + std::string(e.what()));
        return nullptr;
    }
}

std::string FBXLoader::FindTexturePath(const std::string& texturePath, const std::string& modelPath) const {
    // Try the path as-is first
    if (std::filesystem::exists(texturePath)) {
        return texturePath;
    }
    
    // Try relative to the model file
    std::filesystem::path modelDir = std::filesystem::path(modelPath).parent_path();
    std::filesystem::path relativePath = modelDir / texturePath;
    if (std::filesystem::exists(relativePath)) {
        return relativePath.string();
    }
    
    // Try just the filename in the model directory
    std::filesystem::path filename = std::filesystem::path(texturePath).filename();
    std::filesystem::path filenameInModelDir = modelDir / filename;
    if (std::filesystem::exists(filenameInModelDir)) {
        return filenameInModelDir.string();
    }
    
    // Try additional search paths
    for (const auto& searchPath : m_config.textureSearchPaths) {
        std::filesystem::path searchFilePath = std::filesystem::path(searchPath) / filename;
        if (std::filesystem::exists(searchFilePath)) {
            return searchFilePath.string();
        }
    }
    
    return ""; // Not found
}

Math::Vec3 FBXLoader::ConvertFBXColor(const aiColor3D& color) const {
    return Math::Vec3(color.r, color.g, color.b);
}

Math::Vec3 FBXLoader::ConvertVector3(const aiVector3D& vec) const {
    return Math::Vec3(vec.x, vec.y, vec.z);
}

Math::Vec2 FBXLoader::ConvertVector2(const aiVector3D& vec) const {
    return Math::Vec2(vec.x, vec.y);
}

uint32_t FBXLoader::GetFBXPostProcessFlags() const {
    uint32_t flags = 0;
    
    // Always triangulate for consistency
    flags |= aiProcess_Triangulate;
    
    // Always validate data structure
    flags |= aiProcess_ValidateDataStructure;
    
    // Generate normals if missing and enabled
    if (m_config.generateMissingNormals) {
        flags |= aiProcess_GenNormals;
    }
    
    // Generate tangents if enabled
    if (m_config.generateTangents) {
        flags |= aiProcess_CalcTangentSpace;
    }
    
    // Apply optimizations if enabled
    if (m_config.optimizeMeshes) {
        flags |= aiProcess_JoinIdenticalVertices;
        flags |= aiProcess_ImproveCacheLocality;
        flags |= aiProcess_OptimizeMeshes;
        flags |= aiProcess_RemoveRedundantMaterials;
    }
    
    // FBX-specific flags
    flags |= aiProcess_LimitBoneWeights; // Limit to 4 bones per vertex
    flags |= aiProcess_SplitLargeMeshes; // Split large meshes for better performance
    
    return flags;
}

void FBXLoader::ValidateFBXScene(const aiScene* scene) const {
    if (!scene) {
        throw std::runtime_error("FBX Scene is null");
    }
    
    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        Logger::GetInstance().Warning("FBXLoader: FBX Scene is incomplete");
    }
    
    if (!scene->mRootNode) {
        throw std::runtime_error("FBX Scene has no root node");
    }
    
    if (scene->mNumMeshes == 0) {
        Logger::GetInstance().Warning("FBXLoader: FBX Scene contains no meshes");
    }
    
    Logger::GetInstance().Debug("FBX Scene validation: " + std::to_string(scene->mNumMeshes) + " meshes, " + 
                              std::to_string(scene->mNumMaterials) + " materials, " + 
                              std::to_string(scene->mNumTextures) + " textures, " +
                              std::to_string(scene->mNumAnimations) + " animations");
}

std::string FBXLoader::DetectSourceApplicationFromScene(const aiScene* scene) const {
    // Try to detect the source application from metadata
    // This is a simplified implementation
    
    if (!scene) {
        return "Unknown";
    }
    
    // Check for metadata that might indicate the source application
    if (scene->mMetaData) {
        // Look for common metadata keys
        for (unsigned int i = 0; i < scene->mMetaData->mNumProperties; i++) {
            const aiString& key = scene->mMetaData->mKeys[i];
            std::string keyStr(key.C_Str());
            
            if (keyStr.find("Maya") != std::string::npos) {
                return "Maya";
            } else if (keyStr.find("3ds Max") != std::string::npos || keyStr.find("3dsmax") != std::string::npos) {
                return "3ds Max";
            } else if (keyStr.find("Blender") != std::string::npos) {
                return "Blender";
            }
        }
    }
    
    // Fallback: try to infer from mesh characteristics or naming conventions
    // This is very basic and could be expanded
    return "Unknown";
}

#endif // GAMEENGINE_HAS_ASSIMP

void FBXLoader::LogFBXLoadingStats(const FBXLoadResult& result) const {
    if (result.success) {
        Logger::GetInstance().Info("FBX loading completed: " + 
                                 std::to_string(result.meshes.size()) + " meshes, " + 
                                 std::to_string(result.totalVertices) + " vertices, " + 
                                 std::to_string(result.totalTriangles) + " triangles, " + 
                                 std::to_string(result.materialCount) + " materials, " +
                                 std::to_string(result.loadingTimeMs) + "ms, " +
                                 "source: " + result.sourceApplication +
                                 (result.hasSkeleton ? ", has skeleton" : "") +
                                 (result.hasAnimations ? ", has animations" : ""));
    } else {
        Logger::GetInstance().Error("FBX loading failed: " + result.errorMessage);
    }
}

void FBXLoader::ClearTextureCache() {
    m_textureCache.clear();
}

} // namespace GameEngine