#include "Resource/FBXLoader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <functional>

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
    m_config.importTextures = false; // Disable texture loading for now to focus on basic mesh import
    m_config.importSkeleton = true;
    m_config.importAnimations = true;
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
        Logger::GetInstance().Debug("FBXLoader: Starting to load file: " + filepath);
        
        // Load the FBX scene with specific post-processing
        Logger::GetInstance().Debug("FBXLoader: Calling Assimp ReadFile...");
        const aiScene* scene = m_importer->ReadFile(filepath, GetFBXPostProcessFlags());
        Logger::GetInstance().Debug("FBXLoader: Assimp ReadFile completed");
        
        if (!scene) {
            result.errorMessage = "Assimp FBX error (scene is null): " + std::string(m_importer->GetErrorString());
            Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
            return result;
        }
        
        if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            Logger::GetInstance().Warning("FBXLoader: Scene has incomplete flag set");
        }
        
        if (!scene->mRootNode) {
            result.errorMessage = "Assimp FBX error: Scene has no root node";
            Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
            return result;
        }

        Logger::GetInstance().Debug("FBXLoader: Scene loaded successfully, validating...");
        
        // Validate the FBX scene
        try {
            ValidateFBXScene(scene);
            Logger::GetInstance().Debug("FBXLoader: Scene validation completed");
        } catch (const std::exception& e) {
            result.errorMessage = "FBX scene validation failed: " + std::string(e.what());
            Logger::GetInstance().Error("FBXLoader::LoadFBX: " + result.errorMessage);
            return result;
        }
        
        // Process the FBX scene
        Logger::GetInstance().Debug("FBXLoader: Starting scene processing...");
        result = ProcessFBXScene(scene, filepath);
        Logger::GetInstance().Debug("FBXLoader: Scene processing completed");
        
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
        // Process skeleton first if enabled
        if (m_config.importSkeleton && scene->mNumMeshes > 0) {
            result.skeleton = ProcessFBXSkeleton(scene);
            if (result.skeleton) {
                result.boneCount = static_cast<uint32_t>(result.skeleton->GetBoneCount());
                result.hasSkeleton = true;
            }
        }
        
        // Process materials if enabled
        if (m_config.importMaterials) {
            Logger::GetInstance().Info("FBXLoader: Starting material processing...");
            result.materials = ProcessFBXMaterials(scene, filepath);
            result.materialCount = static_cast<uint32_t>(result.materials.size());
            Logger::GetInstance().Info("FBXLoader: Finished material processing.");
        }
        
        // Process all meshes in the scene
        Logger::GetInstance().Info("FBXLoader: Starting mesh processing...");
        try {
            ProcessFBXNode(scene->mRootNode, scene, result.meshes);
            Logger::GetInstance().Info("FBXLoader: Finished mesh processing, found " + std::to_string(result.meshes.size()) + " meshes");
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("FBXLoader: Exception during mesh processing: " + std::string(e.what()));
            throw; // Re-throw to be caught by outer try-catch
        }
        
        // Process bone weights for meshes if we have a skeleton
        if (result.skeleton && m_config.importSkeleton) {
            Logger::GetInstance().Info("FBXLoader: Starting skeleton binding...");
            for (auto& mesh : result.meshes) {
                if (mesh) {
                    // Find the corresponding aiMesh and process bone weights
                    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                        const aiMesh* aiMesh = scene->mMeshes[i];
                        if (aiMesh->mName.length > 0 && mesh->GetName() == std::string(aiMesh->mName.C_Str())) {
                            Logger::GetInstance().Info("FBXLoader: Processing bone weights for mesh: " + mesh->GetName());
                            auto vertices = mesh->GetVertices();
                            ProcessBoneWeights(aiMesh, scene, const_cast<std::vector<Vertex>&>(vertices));
                            mesh->SetVertices(vertices);
                            break;
                        }
                    }
                }
            }
            Logger::GetInstance().Info("FBXLoader: Finished skeleton binding.");
        }
        
        // Process animations if enabled
        if (m_config.importAnimations && scene->mNumAnimations > 0) {
            Logger::GetInstance().Info("FBXLoader: Starting animation import...");
            result.animations = ProcessFBXAnimations(scene);
            result.animationCount = static_cast<uint32_t>(result.animations.size());
            result.hasAnimations = !result.animations.empty();
            Logger::GetInstance().Info("FBXLoader: Finished animation import.");
        }
        
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
            std::string logMessage = "Successfully loaded FBX model '" + filepath + "': " + 
                                   std::to_string(result.meshes.size()) + " meshes, " + 
                                   std::to_string(result.totalVertices) + " vertices, " + 
                                   std::to_string(result.totalTriangles) + " triangles, " +
                                   std::to_string(result.materialCount) + " materials";
            
            if (result.hasSkeleton) {
                logMessage += ", " + std::to_string(result.boneCount) + " bones";
            }
            
            if (result.hasAnimations) {
                logMessage += ", " + std::to_string(result.animationCount) + " animations";
            }
            
            Logger::GetInstance().Info(logMessage);
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
        Logger::GetInstance().Error("FBXLoader::ProcessFBXMesh: mesh is null");
        return nullptr;
    }
    
    std::string meshName = mesh->mName.length > 0 ? std::string(mesh->mName.C_Str()) : "unnamed_mesh";
    Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Starting to process mesh '" + meshName + "'");
    Logger::GetInstance().Info("  Vertices: " + std::to_string(mesh->mNumVertices));
    Logger::GetInstance().Info("  Faces: " + std::to_string(mesh->mNumFaces));
    Logger::GetInstance().Info("  Has bones: " + std::string(mesh->HasBones() ? "Yes" : "No"));
    
    try {
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Creating engine mesh...");
        auto engineMesh = std::make_shared<Mesh>();
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Engine mesh created successfully");
        
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Initializing vertex and index arrays...");
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Arrays initialized");
        
        // Process vertices
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Starting vertex processing...");
        vertices.reserve(mesh->mNumVertices);
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Reserved space for " + std::to_string(mesh->mNumVertices) + " vertices");
        
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            if (i % 1000 == 0) { // Log every 1000 vertices to avoid spam
                Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Processing vertex " + std::to_string(i) + "/" + std::to_string(mesh->mNumVertices));
            }
            
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
        
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Finished processing " + std::to_string(vertices.size()) + " vertices");
        
        // Apply coordinate system conversion if enabled
        if (m_config.convertToOpenGLCoordinates) {
            Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Applying coordinate system conversion...");
            ApplyCoordinateSystemConversion(vertices);
            Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Coordinate system conversion completed");
        }
        
        // Process indices
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Starting index processing...");
        indices.reserve(mesh->mNumFaces * 3);
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Reserved space for " + std::to_string(mesh->mNumFaces * 3) + " indices");
        
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            if (i % 1000 == 0) { // Log every 1000 faces to avoid spam
                Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Processing face " + std::to_string(i) + "/" + std::to_string(mesh->mNumFaces));
            }
            
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
        
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Finished processing " + std::to_string(indices.size()) + " indices");
        
        // Set mesh data
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Setting vertices on engine mesh...");
        engineMesh->SetVertices(vertices);
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Setting indices on engine mesh...");
        engineMesh->SetIndices(indices);
        Logger::GetInstance().Info("FBXLoader::ProcessFBXMesh: Mesh data set successfully");
        
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
        Logger::GetInstance().Warning("FBXLoader::ProcessFBXMaterial: aiMaterial is null");
        return nullptr;
    }
    
    try {
        auto material = std::make_shared<Material>();
        
        // Get material name
        aiString name;
        std::string materialName = "Unnamed Material";
        if (aiMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            materialName = std::string(name.C_Str());
            Logger::GetInstance().Debug("Processing FBX material: " + materialName);
        }
        
        // Process diffuse properties
        aiColor3D diffuse(1.0f, 1.0f, 1.0f);
        if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material->SetAlbedo(ConvertFBXColor(diffuse));
            Logger::GetInstance().Debug("Material '" + materialName + "' diffuse: (" + 
                                      std::to_string(diffuse.r) + ", " + 
                                      std::to_string(diffuse.g) + ", " + 
                                      std::to_string(diffuse.b) + ")");
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
            Logger::GetInstance().Debug("Processing textures for material: " + materialName);
            
            // Diffuse texture
            aiString texturePath;
            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                Logger::GetInstance().Debug("Found diffuse texture: " + std::string(texturePath.C_Str()));
                auto texture = LoadFBXTexture(std::string(texturePath.C_Str()), filepath);
                if (texture) {
                    material->SetTexture("u_diffuseTexture", texture);
                    Logger::GetInstance().Debug("Successfully set diffuse texture for material: " + materialName);
                }
            }
            
            // Normal texture
            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
                Logger::GetInstance().Debug("Found normal texture: " + std::string(texturePath.C_Str()));
                auto texture = LoadFBXTexture(std::string(texturePath.C_Str()), filepath);
                if (texture) {
                    material->SetTexture("u_normalTexture", texture);
                }
            }
            
            // Specular texture
            if (aiMat->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
                Logger::GetInstance().Debug("Found specular texture: " + std::string(texturePath.C_Str()));
                auto texture = LoadFBXTexture(std::string(texturePath.C_Str()), filepath);
                if (texture) {
                    material->SetTexture("u_specularTexture", texture);
                }
            }
        }
        
        Logger::GetInstance().Debug("Successfully processed FBX material: " + materialName);
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
        Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Node is null, returning");
        return;
    }
    
    std::string nodeName = node->mName.length > 0 ? std::string(node->mName.C_Str()) : "unnamed_node";
    Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Processing node '" + nodeName + "' with " + 
                              std::to_string(node->mNumMeshes) + " meshes and " + 
                              std::to_string(node->mNumChildren) + " children");
    
    // Process all meshes in this node
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        try {
            uint32_t meshIndex = node->mMeshes[i];
            Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Processing mesh " + std::to_string(i) + 
                                      " (scene index: " + std::to_string(meshIndex) + ")");
            
            if (meshIndex >= scene->mNumMeshes) {
                Logger::GetInstance().Error("FBXLoader::ProcessFBXNode: Invalid mesh index " + std::to_string(meshIndex) + 
                                          " (scene has " + std::to_string(scene->mNumMeshes) + " meshes)");
                continue;
            }
            
            aiMesh* mesh = scene->mMeshes[meshIndex];
            if (!mesh) {
                Logger::GetInstance().Error("FBXLoader::ProcessFBXNode: Mesh at index " + std::to_string(meshIndex) + " is null");
                continue;
            }
            
            Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: About to call ProcessFBXMesh for mesh '" + 
                                      (mesh->mName.length > 0 ? std::string(mesh->mName.C_Str()) : "unnamed_mesh") + "'");
            
            auto engineMesh = ProcessFBXMesh(mesh, scene);
            
            Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: ProcessFBXMesh completed");
            
            if (engineMesh) {
                meshes.push_back(engineMesh);
                Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Added mesh to result list (total: " + std::to_string(meshes.size()) + ")");
            } else {
                Logger::GetInstance().Warning("FBXLoader::ProcessFBXNode: ProcessFBXMesh returned null for mesh " + std::to_string(i));
            }
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("FBXLoader::ProcessFBXNode: Exception processing mesh " + std::to_string(i) + ": " + std::string(e.what()));
            // Continue processing other meshes
        }
    }
    
    Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Finished processing meshes for node '" + nodeName + 
                              "', now processing " + std::to_string(node->mNumChildren) + " child nodes");
    
    // Recursively process child nodes
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        try {
            Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Processing child node " + std::to_string(i));
            ProcessFBXNode(node->mChildren[i], scene, meshes);
            Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Completed child node " + std::to_string(i));
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("FBXLoader::ProcessFBXNode: Exception processing child node " + std::to_string(i) + ": " + std::string(e.what()));
            // Continue processing other child nodes
        }
    }
    
    Logger::GetInstance().Debug("FBXLoader::ProcessFBXNode: Completed processing node '" + nodeName + "'");
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
        Logger::GetInstance().Warning("FBXLoader: Texture not found: " + texturePath + ", creating default texture");
        
        // Create a default texture instead of returning nullptr
        try {
            auto texture = std::make_shared<Texture>();
            texture->CreateDefault();
            m_textureCache[texturePath] = texture;
            return texture;
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("Exception creating default texture: " + std::string(e.what()));
            return nullptr;
        }
    }
    
    try {
        // Load texture using the engine's texture system
        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromFile(actualPath)) {
            m_textureCache[texturePath] = texture;
            Logger::GetInstance().Debug("Loaded FBX texture: " + actualPath);
            return texture;
        } else {
            Logger::GetInstance().Warning("Failed to load FBX texture: " + actualPath + ", creating default texture");
            
            // Create default texture as fallback
            texture->CreateDefault();
            m_textureCache[texturePath] = texture;
            return texture;
        }
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Exception loading FBX texture '" + actualPath + "': " + std::string(e.what()));
        
        // Try to create default texture as last resort
        try {
            auto texture = std::make_shared<Texture>();
            texture->CreateDefault();
            m_textureCache[texturePath] = texture;
            return texture;
        } catch (const std::exception& e2) {
            Logger::GetInstance().Error("Exception creating fallback texture: " + std::string(e2.what()));
            return nullptr;
        }
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
        std::string logMessage = "FBX loading completed: " + 
                               std::to_string(result.meshes.size()) + " meshes, " + 
                               std::to_string(result.totalVertices) + " vertices, " + 
                               std::to_string(result.totalTriangles) + " triangles, " + 
                               std::to_string(result.materialCount) + " materials, " +
                               std::to_string(result.loadingTimeMs) + "ms, " +
                               "source: " + result.sourceApplication;
        
        if (result.hasSkeleton) {
            logMessage += ", skeleton: " + std::to_string(result.boneCount) + " bones";
        }
        
        if (result.hasAnimations) {
            logMessage += ", animations: " + std::to_string(result.animationCount);
        }
        
        Logger::GetInstance().Info(logMessage);
    } else {
        Logger::GetInstance().Error("FBX loading failed: " + result.errorMessage);
    }
}

std::shared_ptr<Graphics::RenderSkeleton> FBXLoader::ProcessFBXSkeleton(const aiScene* scene) {
    if (!scene || scene->mNumMeshes == 0) {
        return nullptr;
    }
    
    auto skeleton = std::make_shared<Graphics::RenderSkeleton>();
    std::vector<std::shared_ptr<Graphics::RenderBone>> bones;
    std::unordered_map<std::string, std::shared_ptr<Graphics::RenderBone>> boneMap;
    
    Logger::GetInstance().Debug("FBXLoader: Processing skeleton...");
    
    // First pass: collect all bones from all meshes
    int32_t boneIndex = 0;
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (!mesh->HasBones()) {
            continue;
        }
        
        Logger::GetInstance().Debug("Processing bones from mesh: " + std::string(mesh->mName.C_Str()));
        
        for (uint32_t i = 0; i < mesh->mNumBones; i++) {
            const aiBone* aiBone = mesh->mBones[i];
            std::string boneName(aiBone->mName.C_Str());
            
            // Skip if bone already exists
            if (boneMap.find(boneName) != boneMap.end()) {
                continue;
            }
            
            // Create new bone
            auto bone = std::make_shared<Graphics::RenderBone>(boneName, boneIndex++);
            
            // Set inverse bind matrix
            Math::Mat4 inverseBindMatrix;
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    inverseBindMatrix[col][row] = aiBone->mOffsetMatrix[row][col];
                }
            }
            bone->SetInverseBindMatrix(inverseBindMatrix);
            
            bones.push_back(bone);
            boneMap[boneName] = bone;
            
            Logger::GetInstance().Debug("Added bone: " + boneName + " (Index: " + std::to_string(bone->GetIndex()) + ")");
        }
    }
    
    // Second pass: establish hierarchy using scene node structure
    std::function<void(const aiNode*, std::shared_ptr<Graphics::RenderBone>)> processNode = [&](const aiNode* node, std::shared_ptr<Graphics::RenderBone> parentBone) -> void {
        if (!node) return;
        
        std::string nodeName(node->mName.C_Str());
        auto it = boneMap.find(nodeName);
        
        std::shared_ptr<Graphics::RenderBone> currentBone = nullptr;
        if (it != boneMap.end()) {
            currentBone = it->second;
            
            // Set parent relationship
            if (parentBone) {
                currentBone->SetParent(parentBone);
                parentBone->AddChild(currentBone);
                Logger::GetInstance().Debug("Set parent for bone " + nodeName + " to " + parentBone->GetName());
            } else {
                // This is a root bone
                skeleton->SetRootBone(currentBone);
            }
            
            // Set local transform from node
            Math::Mat4 localTransform;
            for (int row = 0; row < 4; row++) {
                for (int col = 0; col < 4; col++) {
                    localTransform[col][row] = node->mTransformation[row][col];
                }
            }
            currentBone->SetLocalTransform(localTransform);
        }
        
        // Process children
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], currentBone ? currentBone : parentBone);
        }
    };
    
    // Start processing from root node
    processNode(scene->mRootNode, nullptr);
    
    // Set bones in skeleton
    skeleton->SetBones(bones);
    skeleton->BuildHierarchy();
    
    Logger::GetInstance().Info("FBXLoader: Successfully processed skeleton with " + 
                             std::to_string(skeleton->GetBoneCount()) + " bones");
    
    return skeleton;
}

std::vector<std::shared_ptr<Graphics::GraphicsAnimation>> FBXLoader::ProcessFBXAnimations(const aiScene* scene) {
    std::vector<std::shared_ptr<Graphics::GraphicsAnimation>> animations;
    
    if (!scene || scene->mNumAnimations == 0) {
        return animations;
    }
    
    Logger::GetInstance().Debug("FBXLoader: Processing " + std::to_string(scene->mNumAnimations) + " animations...");
    
    for (uint32_t i = 0; i < scene->mNumAnimations; i++) {
        auto animation = ProcessFBXAnimation(scene->mAnimations[i]);
        if (animation && animation->GetChannelCount() > 0) {
            animations.push_back(animation);
            Logger::GetInstance().Debug("Processed animation: " + animation->GetName());
        }
    }
    
    Logger::GetInstance().Info("FBXLoader: Successfully processed " + std::to_string(animations.size()) + " animations");
    
    return animations;
}

std::shared_ptr<Graphics::GraphicsAnimation> FBXLoader::ProcessFBXAnimation(const aiAnimation* aiAnim) {
    if (!aiAnim) {
        return nullptr;
    }
    
    auto animation = std::make_shared<Graphics::GraphicsAnimation>(std::string(aiAnim->mName.C_Str()));
    
    Logger::GetInstance().Debug("Processing animation: " + animation->GetName() + 
                              " (duration: " + std::to_string(aiAnim->mDuration) + 
                              ", ticks/sec: " + std::to_string(aiAnim->mTicksPerSecond) + ")");
    
    // TODO: Implement FBX animation processing with new Graphics::AnimationChannel system
    // This is a placeholder implementation for now
    
    return animation;
}

void FBXLoader::ProcessBoneWeights(const aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices) {
    if (!mesh || !mesh->HasBones() || vertices.size() != mesh->mNumVertices) {
        return;
    }
    
    Logger::GetInstance().Debug("Processing bone weights for mesh: " + std::string(mesh->mName.C_Str()));
    
    // Initialize bone data for all vertices
    for (auto& vertex : vertices) {
        vertex.boneIds = Math::Vec4(0.0f);
        vertex.boneWeights = Math::Vec4(0.0f);
    }
    
    // Process each bone
    for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
        const aiBone* bone = mesh->mBones[boneIndex];
        
        // Process each vertex weight for this bone
        for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
            const aiVertexWeight& weight = bone->mWeights[weightIndex];
            uint32_t vertexId = weight.mVertexId;
            
            if (vertexId >= vertices.size()) {
                continue;
            }
            
            Vertex& vertex = vertices[vertexId];
            
            // Find an empty slot for this bone weight
            for (int i = 0; i < 4; i++) {
                if (vertex.boneWeights[i] == 0.0f) {
                    vertex.boneIds[i] = static_cast<float>(boneIndex);
                    vertex.boneWeights[i] = weight.mWeight;
                    break;
                }
            }
        }
    }
    
    // Normalize bone weights to ensure they sum to 1.0
    for (auto& vertex : vertices) {
        float totalWeight = vertex.boneWeights.x + vertex.boneWeights.y + vertex.boneWeights.z + vertex.boneWeights.w;
        if (totalWeight > 0.0f) {
            vertex.boneWeights /= totalWeight;
        }
    }
    
    Logger::GetInstance().Debug("Processed bone weights for " + std::to_string(vertices.size()) + " vertices");
}

void FBXLoader::ExtractBoneData(const aiMesh* mesh, std::shared_ptr<Graphics::RenderSkeleton> skeleton) {
    if (!mesh || !mesh->HasBones() || !skeleton) {
        return;
    }
    
    // This function is used to extract additional bone data if needed
    // Currently, most bone processing is done in ProcessFBXSkeleton
    Logger::GetInstance().Debug("Extracting additional bone data from mesh: " + std::string(mesh->mName.C_Str()));
}

void FBXLoader::ClearTextureCache() {
    m_textureCache.clear();
}

} // namespace GameEngine