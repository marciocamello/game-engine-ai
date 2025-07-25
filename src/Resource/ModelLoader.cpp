#include "Resource/ModelLoader.h"
#include "Graphics/Model.h"
#include "Resource/GLTFLoader.h"
#include "Resource/FBXLoader.h"
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

ModelLoader::ModelLoader() 
    : m_loadingFlags(LoadingFlags::None)
    , m_importScale(1.0f)
    , m_initialized(false)
    , m_extensionsCached(false) {
}

ModelLoader::~ModelLoader() {
    Shutdown();
}

bool ModelLoader::Initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize specialized loaders
    m_gltfLoader = std::make_unique<GLTFLoader>();
    m_fbxLoader = std::make_unique<FBXLoader>();
    
    // Initialize FBX loader
    if (m_fbxLoader && !m_fbxLoader->Initialize()) {
        Logger::GetInstance().Warning("Failed to initialize FBX loader");
        m_fbxLoader.reset();
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    try {
        m_importer = std::make_unique<Assimp::Importer>();
        
        // Set default post-processing flags for better quality
        m_loadingFlags = LoadingFlags::Triangulate | 
                        LoadingFlags::GenerateNormals |
                        LoadingFlags::ValidateDataStructure |
                        LoadingFlags::ImproveCacheLocality |
                        LoadingFlags::RemoveDuplicateVertices;
        
        m_initialized = true;
        std::string loaderInfo = "ModelLoader initialized successfully with Assimp, GLTF";
        if (m_fbxLoader) {
            loaderInfo += ", and FBX";
        }
        loaderInfo += " support";
        Logger::GetInstance().Info(loaderInfo);
        
        // Log supported formats
        auto formats = GetSupportedFormats();
        Logger::GetInstance().Info("ModelLoader supports " + std::to_string(formats.size()) + " formats");
        
        return true;
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Failed to initialize ModelLoader: " + std::string(e.what()));
        return false;
    }
#else
    Logger::GetInstance().Info("ModelLoader initialized with GLTF support (Assimp not available)");
    m_initialized = true;
    return true;
#endif
}

void ModelLoader::Shutdown() {
    if (!m_initialized) {
        return;
    }

    m_gltfLoader.reset();
    
    if (m_fbxLoader) {
        m_fbxLoader->Shutdown();
        m_fbxLoader.reset();
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    m_importer.reset();
#endif
    
    m_initialized = false;
    m_extensionsCached = false;
    m_supportedExtensions.clear();
    
    Logger::GetInstance().Info("ModelLoader shutdown complete");
}

ModelLoader::LoadResult ModelLoader::LoadModel(const std::string& filepath) {
    LoadResult result;
    
    if (!m_initialized) {
        result.errorMessage = "ModelLoader not initialized";
        Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
        return result;
    }

    if (!std::filesystem::exists(filepath)) {
        result.errorMessage = "File not found: " + filepath;
        Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
        return result;
    }

    // Check if it's an FBX file and use specialized loader
    if (FBXLoader::IsFBXFile(filepath) && m_fbxLoader) {
        auto fbxResult = m_fbxLoader->LoadFBX(filepath);
        
        // Convert FBX result to ModelLoader result
        if (fbxResult.success) {
            result.success = true;
            result.meshes = fbxResult.meshes;
            result.totalVertices = fbxResult.totalVertices;
            result.totalTriangles = fbxResult.totalTriangles;
            result.loadingTimeMs = fbxResult.loadingTimeMs;
            result.formatUsed = "fbx";
            
            LogLoadingStats(result);
            return result;
        } else {
            result.errorMessage = "FBX loading failed: " + fbxResult.errorMessage;
            Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
            return result;
        }
    }
    
    // Check if it's a GLTF file and use specialized loader
    if (GLTFLoader::IsGLTFFile(filepath) || GLTFLoader::IsGLBFile(filepath)) {
        if (m_gltfLoader) {
            auto gltfResult = m_gltfLoader->LoadGLTF(filepath);
            
            // Convert GLTF result to ModelLoader result
            if (gltfResult.success && gltfResult.model) {
                result.success = true;
                result.meshes = gltfResult.model->GetMeshes();
                result.totalVertices = gltfResult.totalVertices;
                result.totalTriangles = gltfResult.totalTriangles;
                result.loadingTimeMs = gltfResult.loadingTimeMs;
                result.formatUsed = GLTFLoader::IsGLBFile(filepath) ? "glb" : "gltf";
                
                LogLoadingStats(result);
                return result;
            } else {
                result.errorMessage = "GLTF loading failed: " + gltfResult.errorMessage;
                Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
                return result;
            }
        } else {
            result.errorMessage = "GLTF loader not available";
            Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
            return result;
        }
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Load the scene
        const aiScene* scene = m_importer->ReadFile(filepath, GetAssimpPostProcessFlags());
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            result.errorMessage = "Assimp error: " + std::string(m_importer->GetErrorString());
            Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
            return result;
        }

        // Validate the scene
        ValidateScene(scene);
        
        // Process the scene
        result = ProcessScene(scene, filepath);
        
        // Calculate loading time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        // Detect format used
        result.formatUsed = DetectFormat(filepath);
        
        // Log statistics
        LogLoadingStats(result);
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception during model loading: " + std::string(e.what());
        Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
    }
#else
    result.errorMessage = "Assimp not available and file is not GLTF format";
    Logger::GetInstance().Error("ModelLoader::LoadModel: " + result.errorMessage);
#endif

    return result;
}

std::shared_ptr<Model> ModelLoader::LoadModelAsResource(const std::string& filepath) {
    if (!m_initialized) {
        LOG_ERROR("ModelLoader not initialized");
        return nullptr;
    }

    // Create a new Model resource
    auto model = std::make_shared<Model>(filepath);
    
    // Load the model data using the existing LoadModel method
    auto result = LoadModel(filepath);
    if (!result.success) {
        LOG_ERROR("Failed to load model as resource: " + filepath + " - " + result.errorMessage);
        return nullptr;
    }
    
    // Populate the Model with the loaded data
    model->SetMeshes(result.meshes);
    
    // Set model metadata - Graphics/Model doesn't have SetFormat method
    // The format information is stored in the LoadResult for now
    
    // Extract filename as model name
    std::filesystem::path path(filepath);
    model->SetName(path.stem().string());
    
    LOG_INFO("Successfully loaded model as resource: " + filepath + 
             " (" + std::to_string(result.meshes.size()) + " meshes)");
    
    return model;
}

ModelLoader::LoadResult ModelLoader::LoadModelFromMemory(const std::vector<uint8_t>& data, const std::string& format) {
    LoadResult result;
    
    if (!m_initialized) {
        result.errorMessage = "ModelLoader not initialized";
        Logger::GetInstance().Error("ModelLoader::LoadModelFromMemory: " + result.errorMessage);
        return result;
    }

    if (data.empty()) {
        result.errorMessage = "Empty data buffer";
        Logger::GetInstance().Error("ModelLoader::LoadModelFromMemory: " + result.errorMessage);
        return result;
    }

#ifdef GAMEENGINE_HAS_ASSIMP
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Load from memory
        const aiScene* scene = m_importer->ReadFileFromMemory(
            data.data(), 
            data.size(), 
            GetAssimpPostProcessFlags(),
            format.c_str()
        );
        
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            result.errorMessage = "Assimp error: " + std::string(m_importer->GetErrorString());
            Logger::GetInstance().Error("ModelLoader::LoadModelFromMemory: " + result.errorMessage);
            return result;
        }

        // Validate the scene
        ValidateScene(scene);
        
        // Process the scene
        result = ProcessScene(scene, "memory_buffer." + format);
        
        // Calculate loading time
        auto endTime = std::chrono::high_resolution_clock::now();
        result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        result.formatUsed = format;
        
        // Log statistics
        LogLoadingStats(result);
        
    } catch (const std::exception& e) {
        result.errorMessage = "Exception during model loading from memory: " + std::string(e.what());
        Logger::GetInstance().Error("ModelLoader::LoadModelFromMemory: " + result.errorMessage);
    }
#else
    result.errorMessage = "Assimp not available";
    Logger::GetInstance().Error("ModelLoader::LoadModelFromMemory: " + result.errorMessage);
#endif

    return result;
}

bool ModelLoader::IsFormatSupported(const std::string& extension) const {
    if (!m_extensionsCached) {
        CacheSupportedExtensions();
    }
    
    std::string normalizedExt = NormalizeExtension(extension);
    return m_supportedExtensions.find(normalizedExt) != m_supportedExtensions.end();
}

std::vector<std::string> ModelLoader::GetSupportedExtensions() const {
    if (!m_extensionsCached) {
        CacheSupportedExtensions();
    }
    
    std::vector<std::string> extensions;
    extensions.reserve(m_supportedExtensions.size());
    
    for (const auto& ext : m_supportedExtensions) {
        extensions.push_back(ext);
    }
    
    std::sort(extensions.begin(), extensions.end());
    return extensions;
}

std::vector<ModelLoader::FormatInfo> ModelLoader::GetSupportedFormats() const {
    std::vector<FormatInfo> formats;
    
#ifdef GAMEENGINE_HAS_ASSIMP
    if (m_importer) {
        // Get format descriptions from Assimp
        aiString formatList;
        m_importer->GetExtensionList(formatList);
        
        // Parse the format string (format: "*.ext1;*.ext2;...")
        std::string formatStr(formatList.C_Str());
        std::istringstream iss(formatStr);
        std::string token;
        
        while (std::getline(iss, token, ';')) {
            if (token.size() > 2 && token.substr(0, 2) == "*.") {
                FormatInfo info;
                info.extension = token.substr(2); // Remove "*."
                info.canRead = true;
                info.canWrite = false; // Assimp primarily for reading
                
                // Add descriptions for common formats
                if (info.extension == "obj") {
                    info.description = "Wavefront OBJ";
                } else if (info.extension == "fbx") {
                    info.description = "Autodesk FBX";
                } else if (info.extension == "gltf") {
                    info.description = "glTF 2.0";
                } else if (info.extension == "glb") {
                    info.description = "glTF 2.0 Binary";
                } else if (info.extension == "dae") {
                    info.description = "Collada DAE";
                } else if (info.extension == "3ds") {
                    info.description = "3D Studio Max";
                } else if (info.extension == "blend") {
                    info.description = "Blender";
                } else if (info.extension == "x") {
                    info.description = "DirectX X";
                } else if (info.extension == "ply") {
                    info.description = "Stanford PLY";
                } else if (info.extension == "stl") {
                    info.description = "Stereolithography STL";
                } else {
                    info.description = "3D Model (" + info.extension + ")";
                }
                
                formats.push_back(info);
            }
        }
    }
#endif
    
    return formats;
}

std::string ModelLoader::DetectFormat(const std::string& filepath) const {
    std::string extension = GetFileExtension(filepath);
    
    // Convert to lowercase for comparison
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return extension;
}

void ModelLoader::SetLoadingFlags(LoadingFlags flags) {
    m_loadingFlags = flags;
}

void ModelLoader::SetImportScale(float scale) {
    if (scale > 0.0f) {
        m_importScale = scale;
    } else {
        Logger::GetInstance().Warning("ModelLoader::SetImportScale: Invalid scale " + std::to_string(scale) + ", keeping current value " + std::to_string(m_importScale));
    }
}

bool ModelLoader::IsModelFile(const std::string& filepath) {
    std::string extension = GetFileExtension(filepath);
    
    // Common 3D model extensions
    static const std::unordered_set<std::string> modelExtensions = {
        "obj", "fbx", "gltf", "glb", "dae", "3ds", "blend", "x", "ply", "stl",
        "md2", "md3", "md5", "mdl", "ms3d", "lwo", "lws", "ac", "ase", "hmp"
    };
    
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return modelExtensions.find(extension) != modelExtensions.end();
}

std::string ModelLoader::GetFileExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos != std::string::npos && dotPos < filepath.length() - 1) {
        return filepath.substr(dotPos + 1);
    }
    return "";
}

#ifdef GAMEENGINE_HAS_ASSIMP

ModelLoader::LoadResult ModelLoader::ProcessScene(const aiScene* scene, const std::string& filepath) {
    LoadResult result;
    
    try {
        // Process all meshes in the scene
        ProcessNode(scene->mRootNode, scene, result.meshes);
        
        // Calculate statistics
        for (const auto& mesh : result.meshes) {
            if (mesh) {
                result.totalVertices += mesh->GetVertexCount();
                result.totalTriangles += mesh->GetTriangleCount();
            }
        }
        
        result.success = !result.meshes.empty();
        
        if (result.success) {
            Logger::GetInstance().Info("Successfully loaded model '" + filepath + "': " + 
                                     std::to_string(result.meshes.size()) + " meshes, " + 
                                     std::to_string(result.totalVertices) + " vertices, " + 
                                     std::to_string(result.totalTriangles) + " triangles");
        } else {
            result.errorMessage = "No valid meshes found in model";
            Logger::GetInstance().Warning("ModelLoader::ProcessScene: " + result.errorMessage);
        }
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Exception during scene processing: " + std::string(e.what());
        Logger::GetInstance().Error("ModelLoader::ProcessScene: " + result.errorMessage);
    }
    
    return result;
}

std::shared_ptr<Mesh> ModelLoader::ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
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
                vertex.position = ConvertVector3(mesh->mVertices[i]) * m_importScale;
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
            
            vertices.push_back(vertex);
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
                Logger::GetInstance().Warning("ModelLoader::ProcessMesh: Non-triangular face with " + 
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
        
        Logger::GetInstance().Debug("Processed mesh '" + engineMesh->GetName() + "': " + 
                                  std::to_string(vertices.size()) + " vertices, " + 
                                  std::to_string(indices.size() / 3) + " triangles");
        
        return engineMesh;
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("ModelLoader::ProcessMesh: Exception processing mesh: " + std::string(e.what()));
        return nullptr;
    }
}

void ModelLoader::ProcessNode(const aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Mesh>>& meshes) {
    if (!node) {
        return;
    }
    
    // Process all meshes in this node
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto engineMesh = ProcessMesh(mesh, scene);
        if (engineMesh) {
            meshes.push_back(engineMesh);
        }
    }
    
    // Recursively process child nodes
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, meshes);
    }
}

Math::Vec3 ModelLoader::ConvertVector3(const aiVector3D& vec) const {
    return Math::Vec3(vec.x, vec.y, vec.z);
}

Math::Vec2 ModelLoader::ConvertVector2(const aiVector3D& vec) const {
    return Math::Vec2(vec.x, vec.y);
}

uint32_t ModelLoader::GetAssimpPostProcessFlags() const {
    uint32_t flags = 0;
    
    if (HasFlag(m_loadingFlags, LoadingFlags::Triangulate)) {
        flags |= aiProcess_Triangulate;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::GenerateNormals)) {
        flags |= aiProcess_GenNormals;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::GenerateTangents)) {
        flags |= aiProcess_CalcTangentSpace;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::OptimizeMeshes)) {
        flags |= aiProcess_OptimizeMeshes;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::OptimizeGraph)) {
        flags |= aiProcess_OptimizeGraph;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::FlipWindingOrder)) {
        flags |= aiProcess_FlipWindingOrder;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::MakeLeftHanded)) {
        flags |= aiProcess_MakeLeftHanded;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::RemoveDuplicateVertices)) {
        flags |= aiProcess_JoinIdenticalVertices;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::SortByPrimitiveType)) {
        flags |= aiProcess_SortByPType;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::ImproveCacheLocality)) {
        flags |= aiProcess_ImproveCacheLocality;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::LimitBoneWeights)) {
        flags |= aiProcess_LimitBoneWeights;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::SplitLargeMeshes)) {
        flags |= aiProcess_SplitLargeMeshes;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::ValidateDataStructure)) {
        flags |= aiProcess_ValidateDataStructure;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::PreTransformVertices)) {
        flags |= aiProcess_PreTransformVertices;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::FixInfacingNormals)) {
        flags |= aiProcess_FixInfacingNormals;
    }
    if (HasFlag(m_loadingFlags, LoadingFlags::FlipUVs)) {
        flags |= aiProcess_FlipUVs;
    }
    
    return flags;
}

void ModelLoader::ValidateScene(const aiScene* scene) const {
    if (!scene) {
        throw std::runtime_error("Scene is null");
    }
    
    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        Logger::GetInstance().Warning("ModelLoader: Scene is incomplete");
    }
    
    if (!scene->mRootNode) {
        throw std::runtime_error("Scene has no root node");
    }
    
    if (scene->mNumMeshes == 0) {
        Logger::GetInstance().Warning("ModelLoader: Scene contains no meshes");
    }
    
    Logger::GetInstance().Debug("Scene validation: " + std::to_string(scene->mNumMeshes) + " meshes, " + 
                              std::to_string(scene->mNumMaterials) + " materials, " + 
                              std::to_string(scene->mNumTextures) + " textures");
}

#endif // GAMEENGINE_HAS_ASSIMP

void ModelLoader::CacheSupportedExtensions() const {
    m_supportedExtensions.clear();
    
#ifdef GAMEENGINE_HAS_ASSIMP
    if (m_importer) {
        aiString formatList;
        m_importer->GetExtensionList(formatList);
        
        // Parse the format string (format: "*.ext1;*.ext2;...")
        std::string formatStr(formatList.C_Str());
        std::istringstream iss(formatStr);
        std::string token;
        
        while (std::getline(iss, token, ';')) {
            if (token.size() > 2 && token.substr(0, 2) == "*.") {
                std::string ext = token.substr(2); // Remove "*."
                m_supportedExtensions.insert(NormalizeExtension(ext));
            }
        }
    }
#endif
    
    m_extensionsCached = true;
}

std::string ModelLoader::NormalizeExtension(const std::string& extension) const {
    std::string normalized = extension;
    
    // Remove leading dot if present
    if (!normalized.empty() && normalized[0] == '.') {
        normalized = normalized.substr(1);
    }
    
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    return normalized;
}

void ModelLoader::LogLoadingStats(const LoadResult& result) const {
    if (result.success) {
        Logger::GetInstance().Info("Model loading completed: " + std::to_string(result.meshes.size()) + " meshes, " + 
                                 std::to_string(result.totalVertices) + " vertices, " + 
                                 std::to_string(result.totalTriangles) + " triangles, " + 
                                 std::to_string(result.loadingTimeMs) + "ms, format: " + result.formatUsed);
    } else {
        Logger::GetInstance().Error("Model loading failed: " + result.errorMessage);
    }
}

} // namespace GameEngine