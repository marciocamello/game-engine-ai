#include "Resource/GLTFLoader.h"
#include "Graphics/ModelNode.h"
#include "Graphics/Texture.h"
#include "Graphics/Animation.h"
#include "Graphics/Skeleton.h"
#include "Animation/MorphTarget.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <cstdint>

namespace GameEngine {

// GLTF constants
constexpr uint32_t GLTF_MAGIC = 0x46546C67; // "glTF"
constexpr uint32_t GLTF_VERSION = 2;
constexpr uint32_t GLTF_CHUNK_JSON = 0x4E4F534A; // "JSON"
constexpr uint32_t GLTF_CHUNK_BIN = 0x004E4942;  // "BIN\0"

// Component types
constexpr uint32_t COMPONENT_TYPE_BYTE = 5120;
constexpr uint32_t COMPONENT_TYPE_UNSIGNED_BYTE = 5121;
constexpr uint32_t COMPONENT_TYPE_SHORT = 5122;
constexpr uint32_t COMPONENT_TYPE_UNSIGNED_SHORT = 5123;
constexpr uint32_t COMPONENT_TYPE_UNSIGNED_INT = 5125;
constexpr uint32_t COMPONENT_TYPE_FLOAT = 5126;

GLTFLoader::GLTFLoader() {
}

GLTFLoader::~GLTFLoader() {
}

GLTFLoader::LoadResult GLTFLoader::LoadGLTF(const std::string& filepath) {
    LoadResult result;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    LogInfo("Loading GLTF file: " + filepath);
    
    if (!std::filesystem::exists(filepath)) {
        result.errorMessage = "File not found: " + filepath;
        LogError(result.errorMessage);
        return result;
    }
    
    // Set base directory for relative paths
    m_baseDirectory = std::filesystem::path(filepath).parent_path().string();
    
    // Clear previous data
    m_buffers.clear();
    m_bufferViews.clear();
    m_accessors.clear();
    m_materials.clear();
    m_meshes.clear();
    m_animations.clear();
    m_skeletons.clear();
    m_skins.clear();
    
    bool loadSuccess = false;
    
    // Determine file format and load accordingly
    if (IsGLBFile(filepath)) {
        loadSuccess = LoadGLBBinary(filepath);
    } else if (IsGLTFFile(filepath)) {
        loadSuccess = LoadGLTFJson(filepath);
    } else {
        result.errorMessage = "Unsupported file format: " + filepath;
        LogError(result.errorMessage);
        return result;
    }
    
    if (!loadSuccess) {
        result.errorMessage = "Failed to load GLTF file";
        LogError(result.errorMessage);
        return result;
    }
    
    // Parse GLTF components
    if (!ParseBuffers() || !ParseBufferViews() || !ParseAccessors() || 
        !ParseMaterials() || !ParseMeshes() || !ParseAnimations() || !ParseSkins()) {
        result.errorMessage = "Failed to parse GLTF components";
        LogError(result.errorMessage);
        return result;
    }
    
    // Parse scene
    result.model = ParseScene();
    if (!result.model) {
        result.errorMessage = "Failed to parse GLTF scene";
        LogError(result.errorMessage);
        return result;
    }
    
    // Calculate statistics
    result.nodeCount = static_cast<uint32_t>(result.model->GetAllNodes().size());
    result.meshCount = static_cast<uint32_t>(m_meshes.size());
    result.materialCount = static_cast<uint32_t>(m_materials.size());
    
    for (const auto& mesh : m_meshes) {
        if (mesh) {
            result.totalVertices += mesh->GetVertexCount();
            result.totalTriangles += mesh->GetTriangleCount();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    result.success = true;
    LogInfo("GLTF loaded successfully: " + std::to_string(result.meshCount) + " meshes, " +
            std::to_string(result.totalVertices) + " vertices, " +
            std::to_string(result.totalTriangles) + " triangles in " +
            std::to_string(result.loadingTimeMs) + "ms");
    
    return result;
}

GLTFLoader::LoadResult GLTFLoader::LoadGLTFFromMemory(const std::vector<uint8_t>& data, const std::string& baseDir) {
    LoadResult result;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    LogInfo("Loading GLTF from memory buffer");
    
    m_baseDirectory = baseDir;
    
    // Clear previous data
    m_buffers.clear();
    m_bufferViews.clear();
    m_accessors.clear();
    m_materials.clear();
    m_meshes.clear();
    m_animations.clear();
    m_skeletons.clear();
    m_skins.clear();
    
    // Check if it's GLB format (starts with magic number)
    if (data.size() >= 12) {
        uint32_t magic = *reinterpret_cast<const uint32_t*>(data.data());
        if (magic == GLTF_MAGIC) {
            // Parse GLB format
            std::string jsonChunk;
            std::vector<uint8_t> binaryChunk;
            
            if (!ExtractGLBChunks(data, jsonChunk, binaryChunk)) {
                result.errorMessage = "Failed to extract GLB chunks";
                LogError(result.errorMessage);
                return result;
            }
            
            try {
                m_gltfJson = nlohmann::json::parse(jsonChunk);
                
                // Add binary chunk as first buffer if present
                if (!binaryChunk.empty()) {
                    BufferInfo binaryBuffer;
                    binaryBuffer.data = std::move(binaryChunk);
                    binaryBuffer.byteLength = static_cast<uint32_t>(binaryBuffer.data.size());
                    m_buffers.push_back(std::move(binaryBuffer));
                }
                
            } catch (const nlohmann::json::exception& e) {
                result.errorMessage = "Failed to parse GLB JSON: " + std::string(e.what());
                LogError(result.errorMessage);
                return result;
            }
        } else {
            // Assume JSON format
            try {
                std::string jsonStr(data.begin(), data.end());
                m_gltfJson = nlohmann::json::parse(jsonStr);
            } catch (const nlohmann::json::exception& e) {
                result.errorMessage = "Failed to parse GLTF JSON: " + std::string(e.what());
                LogError(result.errorMessage);
                return result;
            }
        }
    } else {
        result.errorMessage = "Invalid GLTF data size";
        LogError(result.errorMessage);
        return result;
    }
    
    // Parse GLTF components
    if (!ParseBuffers() || !ParseBufferViews() || !ParseAccessors() || 
        !ParseMaterials() || !ParseMeshes() || !ParseAnimations() || !ParseSkins()) {
        result.errorMessage = "Failed to parse GLTF components";
        LogError(result.errorMessage);
        return result;
    }
    
    // Parse scene
    result.model = ParseScene();
    if (!result.model) {
        result.errorMessage = "Failed to parse GLTF scene";
        LogError(result.errorMessage);
        return result;
    }
    
    // Calculate statistics
    result.nodeCount = static_cast<uint32_t>(result.model->GetAllNodes().size());
    result.meshCount = static_cast<uint32_t>(m_meshes.size());
    result.materialCount = static_cast<uint32_t>(m_materials.size());
    
    for (const auto& mesh : m_meshes) {
        if (mesh) {
            result.totalVertices += mesh->GetVertexCount();
            result.totalTriangles += mesh->GetTriangleCount();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.loadingTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    result.success = true;
    LogInfo("GLTF loaded from memory successfully: " + std::to_string(result.meshCount) + " meshes, " +
            std::to_string(result.totalVertices) + " vertices, " +
            std::to_string(result.totalTriangles) + " triangles in " +
            std::to_string(result.loadingTimeMs) + "ms");
    
    return result;
}

bool GLTFLoader::IsGLTFFile(const std::string& filepath) {
    std::string extension = std::filesystem::path(filepath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".gltf";
}

bool GLTFLoader::IsGLBFile(const std::string& filepath) {
    std::string extension = std::filesystem::path(filepath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".glb";
}

bool GLTFLoader::LoadGLTFJson(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LogError("Failed to open GLTF file: " + filepath);
            return false;
        }
        
        file >> m_gltfJson;
        return ParseGLTFJson(m_gltfJson);
        
    } catch (const nlohmann::json::exception& e) {
        LogError("Failed to parse GLTF JSON: " + std::string(e.what()));
        return false;
    }
}

bool GLTFLoader::LoadGLBBinary(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        LogError("Failed to open GLB file: " + filepath);
        return false;
    }
    
    // Read entire file
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    // Extract JSON and binary chunks
    std::string jsonChunk;
    std::vector<uint8_t> binaryChunk;
    
    if (!ExtractGLBChunks(data, jsonChunk, binaryChunk)) {
        return false;
    }
    
    try {
        m_gltfJson = nlohmann::json::parse(jsonChunk);
        
        // Add binary chunk as first buffer if present
        if (!binaryChunk.empty()) {
            BufferInfo binaryBuffer;
            binaryBuffer.data = std::move(binaryChunk);
            binaryBuffer.byteLength = static_cast<uint32_t>(binaryBuffer.data.size());
            m_buffers.push_back(std::move(binaryBuffer));
        }
        
        return ParseGLTFJson(m_gltfJson);
        
    } catch (const nlohmann::json::exception& e) {
        LogError("Failed to parse GLB JSON: " + std::string(e.what()));
        return false;
    }
}

bool GLTFLoader::ParseGLTFJson(const nlohmann::json& json) {
    // Validate GLTF version
    if (!json.contains("asset") || !json["asset"].contains("version")) {
        LogError("Invalid GLTF: missing asset version");
        return false;
    }
    
    std::string version = json["asset"]["version"];
    if (version != "2.0") {
        LogError("Unsupported GLTF version: " + version);
        return false;
    }
    
    LogInfo("GLTF version: " + version);
    return true;
}

bool GLTFLoader::ExtractGLBChunks(const std::vector<uint8_t>& data, std::string& jsonChunk, std::vector<uint8_t>& binaryChunk) {
    if (data.size() < 12) {
        LogError("GLB file too small");
        return false;
    }
    
    // Parse GLB header
    uint32_t magic = *reinterpret_cast<const uint32_t*>(data.data());
    uint32_t version = *reinterpret_cast<const uint32_t*>(data.data() + 4);
    uint32_t length = *reinterpret_cast<const uint32_t*>(data.data() + 8);
    
    if (magic != GLTF_MAGIC) {
        LogError("Invalid GLB magic number");
        return false;
    }
    
    if (version != GLTF_VERSION) {
        LogError("Unsupported GLB version: " + std::to_string(version));
        return false;
    }
    
    if (length > data.size()) {
        LogError("GLB length exceeds file size");
        return false;
    }
    
    size_t offset = 12; // Skip header
    
    // Parse JSON chunk
    if (offset + 8 > data.size()) {
        LogError("GLB file truncated at JSON chunk header");
        return false;
    }
    
    uint32_t jsonLength = *reinterpret_cast<const uint32_t*>(data.data() + offset);
    uint32_t jsonType = *reinterpret_cast<const uint32_t*>(data.data() + offset + 4);
    offset += 8;
    
    if (jsonType != GLTF_CHUNK_JSON) {
        LogError("Expected JSON chunk, got: " + std::to_string(jsonType));
        return false;
    }
    
    if (offset + jsonLength > data.size()) {
        LogError("GLB file truncated at JSON chunk data");
        return false;
    }
    
    jsonChunk = std::string(data.begin() + offset, data.begin() + offset + jsonLength);
    offset += jsonLength;
    
    // Parse binary chunk (optional)
    if (offset + 8 <= data.size()) {
        uint32_t binaryLength = *reinterpret_cast<const uint32_t*>(data.data() + offset);
        uint32_t binaryType = *reinterpret_cast<const uint32_t*>(data.data() + offset + 4);
        offset += 8;
        
        if (binaryType == GLTF_CHUNK_BIN && offset + binaryLength <= data.size()) {
            binaryChunk = std::vector<uint8_t>(data.begin() + offset, data.begin() + offset + binaryLength);
        }
    }
    
    return true;
}

bool GLTFLoader::ParseBuffers() {
    if (!m_gltfJson.contains("buffers")) {
        LogInfo("No buffers found in GLTF");
        return true;
    }
    
    const auto& buffersJson = m_gltfJson["buffers"];
    
    // Skip first buffer if we already loaded it from GLB
    size_t startIndex = m_buffers.empty() ? 0 : 1;
    
    for (size_t i = startIndex; i < buffersJson.size(); ++i) {
        const auto& bufferJson = buffersJson[i];
        
        BufferInfo buffer;
        buffer.byteLength = bufferJson["byteLength"];
        
        if (bufferJson.contains("uri")) {
            buffer.uri = bufferJson["uri"];
            
            if (!LoadExternalBuffer(buffer.uri, buffer.data)) {
                LogError("Failed to load buffer: " + buffer.uri);
                return false;
            }
        }
        
        if (i < m_buffers.size()) {
            // Replace placeholder buffer
            m_buffers[i] = std::move(buffer);
        } else {
            m_buffers.push_back(std::move(buffer));
        }
    }
    
    LogInfo("Loaded " + std::to_string(m_buffers.size()) + " buffers");
    return true;
}

bool GLTFLoader::ParseBufferViews() {
    if (!m_gltfJson.contains("bufferViews")) {
        LogInfo("No buffer views found in GLTF");
        return true;
    }
    
    const auto& bufferViewsJson = m_gltfJson["bufferViews"];
    m_bufferViews.reserve(bufferViewsJson.size());
    
    for (const auto& bufferViewJson : bufferViewsJson) {
        BufferViewInfo bufferView;
        bufferView.buffer = bufferViewJson["buffer"];
        bufferView.byteOffset = bufferViewJson.value("byteOffset", 0);
        bufferView.byteLength = bufferViewJson["byteLength"];
        bufferView.byteStride = bufferViewJson.value("byteStride", 0);
        bufferView.target = bufferViewJson.value("target", 0);
        
        m_bufferViews.push_back(bufferView);
    }
    
    LogInfo("Loaded " + std::to_string(m_bufferViews.size()) + " buffer views");
    return true;
}

bool GLTFLoader::ParseAccessors() {
    if (!m_gltfJson.contains("accessors")) {
        LogInfo("No accessors found in GLTF");
        return true;
    }
    
    const auto& accessorsJson = m_gltfJson["accessors"];
    m_accessors.reserve(accessorsJson.size());
    
    for (const auto& accessorJson : accessorsJson) {
        AccessorInfo accessor;
        accessor.bufferView = accessorJson.value("bufferView", 0);
        accessor.byteOffset = accessorJson.value("byteOffset", 0);
        accessor.componentType = accessorJson["componentType"];
        accessor.count = accessorJson["count"];
        accessor.type = accessorJson["type"];
        accessor.normalized = accessorJson.value("normalized", false);
        
        m_accessors.push_back(accessor);
    }
    
    LogInfo("Loaded " + std::to_string(m_accessors.size()) + " accessors");
    return true;
}

bool GLTFLoader::ParseMaterials() {
    if (!m_gltfJson.contains("materials")) {
        LogInfo("No materials found in GLTF, creating default material");
        
        // Create default material
        auto defaultMaterial = std::make_shared<Material>();
        defaultMaterial->SetAlbedo(Math::Vec3(0.8f, 0.8f, 0.8f));
        defaultMaterial->SetMetallic(0.0f);
        defaultMaterial->SetRoughness(0.5f);
        m_materials.push_back(defaultMaterial);
        
        return true;
    }
    
    const auto& materialsJson = m_gltfJson["materials"];
    m_materials.reserve(materialsJson.size());
    
    for (size_t i = 0; i < materialsJson.size(); ++i) {
        auto material = ParseMaterial(materialsJson[i], static_cast<uint32_t>(i));
        if (material) {
            m_materials.push_back(material);
        } else {
            LogWarning("Failed to parse material " + std::to_string(i) + ", using default");
            
            auto defaultMaterial = std::make_shared<Material>();
            defaultMaterial->SetAlbedo(Math::Vec3(0.8f, 0.8f, 0.8f));
            defaultMaterial->SetMetallic(0.0f);
            defaultMaterial->SetRoughness(0.5f);
            m_materials.push_back(defaultMaterial);
        }
    }
    
    LogInfo("Loaded " + std::to_string(m_materials.size()) + " materials");
    return true;
}

bool GLTFLoader::ParseMeshes() {
    if (!m_gltfJson.contains("meshes")) {
        LogInfo("No meshes found in GLTF");
        return true; // Empty scenes are valid
    }
    
    const auto& meshesJson = m_gltfJson["meshes"];
    m_meshes.reserve(meshesJson.size());
    
    LogInfo("Starting to parse " + std::to_string(meshesJson.size()) + " meshes");
    
    for (size_t i = 0; i < meshesJson.size(); ++i) {
        LogInfo("Parsing mesh " + std::to_string(i + 1) + "/" + std::to_string(meshesJson.size()));
        
        auto mesh = ParseMesh(meshesJson[i], static_cast<uint32_t>(i));
        if (mesh) {
            m_meshes.push_back(mesh);
            LogInfo("Successfully parsed mesh " + std::to_string(i) + " with " + 
                   std::to_string(mesh->GetVertexCount()) + " vertices");
        } else {
            LogError("Failed to parse mesh " + std::to_string(i));
            return false;
        }
    }
    
    LogInfo("Loaded " + std::to_string(m_meshes.size()) + " meshes");
    return true;
}

std::shared_ptr<Model> GLTFLoader::ParseScene(uint32_t sceneIndex) {
    if (!m_gltfJson.contains("scenes")) {
        LogError("No scenes found in GLTF");
        return nullptr;
    }
    
    const auto& scenesJson = m_gltfJson["scenes"];
    if (sceneIndex >= scenesJson.size()) {
        LogError("Scene index out of range: " + std::to_string(sceneIndex));
        return nullptr;
    }
    
    const auto& sceneJson = scenesJson[sceneIndex];
    
    // Create model
    auto model = std::make_shared<Model>("gltf_model");
    model->SetName("GLTF Model");
    
    // Set meshes and materials
    model->SetMeshes(m_meshes);
    model->SetMaterials(m_materials);
    
    // Set animations and skeletons
    model->SetAnimations(m_animations);
    model->SetSkins(m_skins);
    if (!m_skeletons.empty()) {
        model->SetSkeleton(m_skeletons[0]); // Use first skeleton as primary
    }
    
    // Parse root nodes
    if (sceneJson.contains("nodes")) {
        auto rootNode = model->GetRootNode();
        
        for (uint32_t nodeIndex : sceneJson["nodes"]) {
            if (nodeIndex < m_gltfJson["nodes"].size()) {
                auto childNode = ParseNode(m_gltfJson["nodes"][nodeIndex], nodeIndex);
                if (childNode) {
                    rootNode->AddChild(childNode);
                }
            }
        }
    }
    
    // Update model bounds
    model->UpdateBounds();
    
    return model;
}

std::shared_ptr<ModelNode> GLTFLoader::ParseNode(const nlohmann::json& nodeJson, uint32_t nodeIndex) {
    std::string nodeName = nodeJson.value("name", "Node_" + std::to_string(nodeIndex));
    auto node = std::make_shared<ModelNode>(nodeName);
    
    // Parse transform
    if (nodeJson.contains("matrix")) {
        // Matrix transform
        Math::Mat4 transform = ParseMatrix(nodeJson["matrix"]);
        node->SetLocalTransform(transform);
    } else {
        // TRS transform
        Math::Mat4 transform = Math::Mat4(1.0f);
        
        if (nodeJson.contains("translation")) {
            Math::Vec3 translation = ParseVec3(nodeJson["translation"]);
            transform = glm::translate(transform, translation);
        }
        
        if (nodeJson.contains("rotation")) {
            // Quaternion rotation
            auto rotJson = nodeJson["rotation"];
            if (rotJson.size() == 4) {
                Math::Quat rotation(rotJson[3], rotJson[0], rotJson[1], rotJson[2]); // w, x, y, z
                transform = transform * glm::mat4_cast(rotation);
            }
        }
        
        if (nodeJson.contains("scale")) {
            Math::Vec3 scale = ParseVec3(nodeJson["scale"], Math::Vec3(1.0f));
            transform = glm::scale(transform, scale);
        }
        
        node->SetLocalTransform(transform);
    }
    
    // Parse mesh references
    if (nodeJson.contains("mesh")) {
        uint32_t meshIndex = nodeJson["mesh"];
        if (meshIndex < m_meshes.size()) {
            node->AddMeshIndex(meshIndex);
        }
    }
    
    // Parse children
    if (nodeJson.contains("children")) {
        for (uint32_t childIndex : nodeJson["children"]) {
            if (childIndex < m_gltfJson["nodes"].size()) {
                auto childNode = ParseNode(m_gltfJson["nodes"][childIndex], childIndex);
                if (childNode) {
                    node->AddChild(childNode);
                }
            }
        }
    }
    
    return node;
}

std::shared_ptr<Mesh> GLTFLoader::ParseMesh(const nlohmann::json& meshJson, uint32_t meshIndex) {
    std::string meshName = meshJson.value("name", "Mesh_" + std::to_string(meshIndex));
    auto mesh = std::make_shared<Mesh>();
    mesh->SetName(meshName);
    
    // GLTF meshes contain primitives - we'll combine them into a single mesh for now
    if (!meshJson.contains("primitives")) {
        LogError("Mesh has no primitives: " + meshName);
        return nullptr;
    }
    
    const auto& primitivesJson = meshJson["primitives"];
    
    // For now, just parse the first primitive
    // TODO: Handle multiple primitives properly
    if (!primitivesJson.empty()) {
        if (!ParseMeshPrimitive(primitivesJson[0], mesh)) {
            LogError("Failed to parse mesh primitive: " + meshName);
            return nullptr;
        }
    }
    
    return mesh;
}

bool GLTFLoader::ParseMeshPrimitive(const nlohmann::json& primitiveJson, std::shared_ptr<Mesh> mesh) {
    if (!primitiveJson.contains("attributes")) {
        LogError("Primitive has no attributes");
        return false;
    }
    
    const auto& attributesJson = primitiveJson["attributes"];
    
    // Parse vertex attributes
    std::vector<Vertex> vertices;
    
    // Position is required
    if (!attributesJson.contains("POSITION")) {
        LogError("Primitive missing POSITION attribute");
        return false;
    }
    
    uint32_t positionAccessor = attributesJson["POSITION"];
    auto positions = GetVec3AccessorData(positionAccessor);
    
    if (positions.empty()) {
        LogError("Failed to get position data");
        return false;
    }
    
    vertices.resize(positions.size());
    for (size_t i = 0; i < positions.size(); ++i) {
        vertices[i].position = positions[i];
    }
    
    // Normal (optional)
    if (attributesJson.contains("NORMAL")) {
        uint32_t normalAccessor = attributesJson["NORMAL"];
        auto normals = GetVec3AccessorData(normalAccessor);
        
        if (normals.size() == vertices.size()) {
            for (size_t i = 0; i < normals.size(); ++i) {
                vertices[i].normal = normals[i];
            }
        }
    }
    
    // Texture coordinates (optional)
    if (attributesJson.contains("TEXCOORD_0")) {
        uint32_t texCoordAccessor = attributesJson["TEXCOORD_0"];
        auto texCoords = GetVec2AccessorData(texCoordAccessor);
        
        if (texCoords.size() == vertices.size()) {
            for (size_t i = 0; i < texCoords.size(); ++i) {
                vertices[i].texCoords = texCoords[i];
            }
        }
    }
    
    // Tangent (optional)
    if (attributesJson.contains("TANGENT")) {
        uint32_t tangentAccessor = attributesJson["TANGENT"];
        // Tangents are Vec4 in GLTF (w component for handedness)
        // For now, we'll just use the xyz components
        auto tangentData = GetAccessorData<Math::Vec4>(tangentAccessor);
        
        if (tangentData.size() == vertices.size()) {
            for (size_t i = 0; i < tangentData.size(); ++i) {
                vertices[i].tangent = Math::Vec3(tangentData[i].x, tangentData[i].y, tangentData[i].z);
            }
        }
    }
    
    // Color (optional)
    if (attributesJson.contains("COLOR_0")) {
        uint32_t colorAccessor = attributesJson["COLOR_0"];
        auto colors = GetAccessorData<Math::Vec4>(colorAccessor);
        
        if (colors.size() == vertices.size()) {
            for (size_t i = 0; i < colors.size(); ++i) {
                vertices[i].color = colors[i];
            }
        }
    }
    
    // Bone IDs (for skinning)
    if (attributesJson.contains("JOINTS_0")) {
        uint32_t jointsAccessor = attributesJson["JOINTS_0"];
        auto joints = GetAccessorData<Math::Vec4>(jointsAccessor);
        
        if (joints.size() == vertices.size()) {
            for (size_t i = 0; i < joints.size(); ++i) {
                vertices[i].boneIds = joints[i];
            }
        }
    }
    
    // Bone weights (for skinning)
    if (attributesJson.contains("WEIGHTS_0")) {
        uint32_t weightsAccessor = attributesJson["WEIGHTS_0"];
        auto weights = GetAccessorData<Math::Vec4>(weightsAccessor);
        
        if (weights.size() == vertices.size()) {
            for (size_t i = 0; i < weights.size(); ++i) {
                vertices[i].boneWeights = weights[i];
            }
        }
    }
    
    mesh->SetVertices(vertices);
    
    // Parse indices
    if (primitiveJson.contains("indices")) {
        uint32_t indicesAccessor = primitiveJson["indices"];
        auto indices = GetScalarAccessorData(indicesAccessor);
        mesh->SetIndices(indices);
    }
    
    // Parse material
    if (primitiveJson.contains("material")) {
        uint32_t materialIndex = primitiveJson["material"];
        if (materialIndex < m_materials.size()) {
            mesh->SetMaterial(m_materials[materialIndex]);
            mesh->SetMaterialIndex(materialIndex);
        }
    } else if (!m_materials.empty()) {
        // Use default material
        mesh->SetMaterial(m_materials[0]);
        mesh->SetMaterialIndex(0);
    }
    
    // Parse morph targets
    if (primitiveJson.contains("targets")) {
        auto morphTargets = ParseMorphTargets(primitiveJson["targets"]);
        if (morphTargets) {
            mesh->SetMorphTargets(morphTargets);
        }
    }
    
    return true;
}

std::shared_ptr<Material> GLTFLoader::ParseMaterial(const nlohmann::json& materialJson, uint32_t materialIndex) {
    auto material = std::make_shared<Material>();
    
    // Parse PBR metallic-roughness
    if (materialJson.contains("pbrMetallicRoughness")) {
        ParsePBRMetallicRoughness(materialJson["pbrMetallicRoughness"], material);
    }
    
    // Parse other material properties
    if (materialJson.contains("normalTexture")) {
        // TODO: Handle normal texture
    }
    
    if (materialJson.contains("occlusionTexture")) {
        // TODO: Handle occlusion texture
    }
    
    if (materialJson.contains("emissiveTexture")) {
        // TODO: Handle emissive texture
    }
    
    if (materialJson.contains("emissiveFactor")) {
        Math::Vec3 emissive = ParseVec3(materialJson["emissiveFactor"]);
        material->SetVec3("u_emissive", emissive);
    }
    
    if (materialJson.contains("alphaMode")) {
        std::string alphaMode = materialJson["alphaMode"];
        // TODO: Handle alpha mode (OPAQUE, MASK, BLEND)
    }
    
    if (materialJson.contains("alphaCutoff")) {
        float alphaCutoff = materialJson["alphaCutoff"];
        material->SetFloat("u_alphaCutoff", alphaCutoff);
    }
    
    if (materialJson.contains("doubleSided")) {
        bool doubleSided = materialJson["doubleSided"];
        material->SetBool("u_doubleSided", doubleSided);
    }
    
    return material;
}

void GLTFLoader::ParsePBRMetallicRoughness(const nlohmann::json& pbrJson, std::shared_ptr<Material> material) {
    // Base color factor
    if (pbrJson.contains("baseColorFactor")) {
        Math::Vec4 baseColor = ParseVec4(pbrJson["baseColorFactor"], Math::Vec4(1.0f));
        material->SetAlbedo(Math::Vec3(baseColor.x, baseColor.y, baseColor.z));
        material->SetFloat("u_alpha", baseColor.w);
    }
    
    // Metallic factor
    if (pbrJson.contains("metallicFactor")) {
        float metallic = pbrJson["metallicFactor"];
        material->SetMetallic(metallic);
    }
    
    // Roughness factor
    if (pbrJson.contains("roughnessFactor")) {
        float roughness = pbrJson["roughnessFactor"];
        material->SetRoughness(roughness);
    }
    
    // Base color texture
    if (pbrJson.contains("baseColorTexture")) {
        // TODO: Load and set base color texture
        LogInfo("Base color texture found but texture loading not implemented yet");
    }
    
    // Metallic-roughness texture
    if (pbrJson.contains("metallicRoughnessTexture")) {
        // TODO: Load and set metallic-roughness texture
        LogInfo("Metallic-roughness texture found but texture loading not implemented yet");
    }
}

template<typename T>
std::vector<T> GLTFLoader::GetAccessorData(uint32_t accessorIndex) {
    if (accessorIndex >= m_accessors.size()) {
        LogError("Accessor index out of range: " + std::to_string(accessorIndex));
        return {};
    }
    
    const auto& accessor = m_accessors[accessorIndex];
    
    if (accessor.bufferView >= m_bufferViews.size()) {
        LogError("Buffer view index out of range: " + std::to_string(accessor.bufferView));
        return {};
    }
    
    const auto& bufferView = m_bufferViews[accessor.bufferView];
    
    if (bufferView.buffer >= m_buffers.size()) {
        LogError("Buffer index out of range: " + std::to_string(bufferView.buffer));
        return {};
    }
    
    const auto& buffer = m_buffers[bufferView.buffer];
    
    // Calculate data offset and size
    size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
    size_t componentSize = GetComponentSize(accessor.componentType);
    size_t componentCount = GetTypeComponentCount(accessor.type);
    size_t elementSize = componentSize * componentCount;
    
    if (dataOffset + accessor.count * elementSize > buffer.data.size()) {
        LogError("Accessor data exceeds buffer size");
        return {};
    }
    
    std::vector<T> result;
    result.reserve(accessor.count);
    
    const uint8_t* dataPtr = buffer.data.data() + dataOffset;
    
    for (uint32_t i = 0; i < accessor.count; ++i) {
        T element;
        std::memcpy(&element, dataPtr + i * elementSize, sizeof(T));
        result.push_back(element);
    }
    
    return result;
}

std::vector<Math::Vec3> GLTFLoader::GetVec3AccessorData(uint32_t accessorIndex) {
    return GetAccessorData<Math::Vec3>(accessorIndex);
}

std::vector<Math::Vec2> GLTFLoader::GetVec2AccessorData(uint32_t accessorIndex) {
    return GetAccessorData<Math::Vec2>(accessorIndex);
}

std::vector<uint32_t> GLTFLoader::GetScalarAccessorData(uint32_t accessorIndex) {
    if (accessorIndex >= m_accessors.size()) {
        LogError("Accessor index out of range: " + std::to_string(accessorIndex));
        return {};
    }
    
    const auto& accessor = m_accessors[accessorIndex];
    
    // Handle different component types for indices
    switch (accessor.componentType) {
        case COMPONENT_TYPE_UNSIGNED_BYTE: {
            auto data = GetAccessorData<uint8_t>(accessorIndex);
            std::vector<uint32_t> result;
            result.reserve(data.size());
            for (uint8_t value : data) {
                result.push_back(static_cast<uint32_t>(value));
            }
            return result;
        }
        case COMPONENT_TYPE_UNSIGNED_SHORT: {
            auto data = GetAccessorData<uint16_t>(accessorIndex);
            std::vector<uint32_t> result;
            result.reserve(data.size());
            for (uint16_t value : data) {
                result.push_back(static_cast<uint32_t>(value));
            }
            return result;
        }
        case COMPONENT_TYPE_UNSIGNED_INT: {
            return GetAccessorData<uint32_t>(accessorIndex);
        }
        default:
            LogError("Unsupported component type for indices: " + std::to_string(accessor.componentType));
            return {};
    }
}

bool GLTFLoader::LoadExternalBuffer(const std::string& uri, std::vector<uint8_t>& data) {
    if (IsDataURI(uri)) {
        return DecodeDataURI(uri, data);
    }
    
    // Load from file
    std::filesystem::path bufferPath = std::filesystem::path(m_baseDirectory) / uri;
    
    std::ifstream file(bufferPath, std::ios::binary);
    if (!file.is_open()) {
        LogError("Failed to open buffer file: " + bufferPath.string());
        return false;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    data.resize(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    return true;
}

bool GLTFLoader::IsDataURI(const std::string& uri) {
    return uri.substr(0, 5) == "data:";
}

bool GLTFLoader::DecodeDataURI(const std::string& uri, std::vector<uint8_t>& data) {
    // Simple base64 decoding for data URIs
    // This is a basic implementation - a full implementation would handle all data URI formats
    LogWarning("Data URI decoding not fully implemented: " + uri.substr(0, 50) + "...");
    return false;
}

Math::Mat4 GLTFLoader::ParseMatrix(const nlohmann::json& matrixJson) {
    if (matrixJson.size() != 16) {
        LogError("Invalid matrix size: " + std::to_string(matrixJson.size()));
        return Math::Mat4(1.0f);
    }
    
    Math::Mat4 matrix;
    for (int i = 0; i < 16; ++i) {
        matrix[i / 4][i % 4] = matrixJson[i];
    }
    
    return matrix;
}

Math::Vec3 GLTFLoader::ParseVec3(const nlohmann::json& vecJson, const Math::Vec3& defaultValue) {
    if (vecJson.size() != 3) {
        return defaultValue;
    }
    
    return Math::Vec3(vecJson[0], vecJson[1], vecJson[2]);
}

Math::Vec4 GLTFLoader::ParseVec4(const nlohmann::json& vecJson, const Math::Vec4& defaultValue) {
    if (vecJson.size() != 4) {
        return defaultValue;
    }
    
    return Math::Vec4(vecJson[0], vecJson[1], vecJson[2], vecJson[3]);
}

uint32_t GLTFLoader::GetComponentSize(uint32_t componentType) {
    switch (componentType) {
        case COMPONENT_TYPE_BYTE:
        case COMPONENT_TYPE_UNSIGNED_BYTE:
            return 1;
        case COMPONENT_TYPE_SHORT:
        case COMPONENT_TYPE_UNSIGNED_SHORT:
            return 2;
        case COMPONENT_TYPE_UNSIGNED_INT:
        case COMPONENT_TYPE_FLOAT:
            return 4;
        default:
            LogError("Unknown component type: " + std::to_string(componentType));
            return 0;
    }
}

uint32_t GLTFLoader::GetTypeComponentCount(const std::string& type) {
    if (type == "SCALAR") return 1;
    if (type == "VEC2") return 2;
    if (type == "VEC3") return 3;
    if (type == "VEC4") return 4;
    if (type == "MAT2") return 4;
    if (type == "MAT3") return 9;
    if (type == "MAT4") return 16;
    
    LogError("Unknown accessor type: " + type);
    return 0;
}

void GLTFLoader::LogError(const std::string& message) {
    Logger::GetInstance().Error("GLTFLoader: " + message);
}

void GLTFLoader::LogWarning(const std::string& message) {
    Logger::GetInstance().Warning("GLTFLoader: " + message);
}

void GLTFLoader::LogInfo(const std::string& message) {
    Logger::GetInstance().Info("GLTFLoader: " + message);
}

bool GLTFLoader::ParseAnimations() {
    if (!m_gltfJson.contains("animations")) {
        LogInfo("No animations found in GLTF");
        return true;
    }
    
    const auto& animationsJson = m_gltfJson["animations"];
    m_animations.reserve(animationsJson.size());
    
    for (size_t i = 0; i < animationsJson.size(); ++i) {
        auto animation = ParseAnimation(animationsJson[i], static_cast<uint32_t>(i));
        if (animation) {
            m_animations.push_back(animation);
        } else {
            LogWarning("Failed to parse animation " + std::to_string(i));
        }
    }
    
    LogInfo("Loaded " + std::to_string(m_animations.size()) + " animations");
    return true;
}

bool GLTFLoader::ParseSkins() {
    if (!m_gltfJson.contains("skins")) {
        LogInfo("No skins found in GLTF");
        return true;
    }
    
    const auto& skinsJson = m_gltfJson["skins"];
    m_skins.reserve(skinsJson.size());
    m_skeletons.reserve(skinsJson.size());
    
    for (size_t i = 0; i < skinsJson.size(); ++i) {
        auto skin = ParseSkin(skinsJson[i], static_cast<uint32_t>(i));
        if (skin) {
            m_skins.push_back(skin);
            
            // Create skeleton from skin
            auto skeleton = CreateSkeletonFromSkin(skinsJson[i]);
            if (skeleton) {
                skin->SetSkeleton(skeleton);
                m_skeletons.push_back(skeleton);
            }
        } else {
            LogWarning("Failed to parse skin " + std::to_string(i));
        }
    }
    
    LogInfo("Loaded " + std::to_string(m_skins.size()) + " skins and " + 
            std::to_string(m_skeletons.size()) + " skeletons");
    return true;
}

std::shared_ptr<Animation> GLTFLoader::ParseAnimation(const nlohmann::json& animationJson, uint32_t animationIndex) {
    std::string animationName = animationJson.value("name", "Animation_" + std::to_string(animationIndex));
    auto animation = std::make_shared<Animation>(animationName);
    
    if (!animationJson.contains("channels") || !animationJson.contains("samplers")) {
        LogError("Animation missing channels or samplers: " + animationName);
        return nullptr;
    }
    
    const auto& channelsJson = animationJson["channels"];
    const auto& samplersJson = animationJson["samplers"];
    
    // Parse channels
    for (const auto& channelJson : channelsJson) {
        auto channel = ParseAnimationChannel(channelJson);
        if (channel) {
            // Get sampler index
            if (!channelJson.contains("sampler")) {
                LogError("Animation channel missing sampler reference");
                continue;
            }
            
            uint32_t samplerIndex = channelJson["sampler"];
            if (samplerIndex >= samplersJson.size()) {
                LogError("Animation channel sampler index out of range");
                continue;
            }
            
            const auto& samplerJson = samplersJson[samplerIndex];
            
            // Parse target property
            if (!channelJson.contains("target") || !channelJson["target"].contains("path")) {
                LogError("Animation channel missing target path");
                continue;
            }
            
            std::string targetPath = channelJson["target"]["path"];
            
            if (targetPath == "translation") {
                auto sampler = ParseAnimationSampler<Math::Vec3>(samplerJson);
                channel->SetTranslationSampler(sampler);
                channel->SetTargetProperty(AnimationTarget::Translation);
            } else if (targetPath == "rotation") {
                auto sampler = ParseAnimationSampler<Math::Quat>(samplerJson);
                channel->SetRotationSampler(sampler);
                channel->SetTargetProperty(AnimationTarget::Rotation);
            } else if (targetPath == "scale") {
                auto sampler = ParseAnimationSampler<Math::Vec3>(samplerJson);
                channel->SetScaleSampler(sampler);
                channel->SetTargetProperty(AnimationTarget::Scale);
            } else if (targetPath == "weights") {
                auto sampler = ParseAnimationSampler<std::vector<float>>(samplerJson);
                channel->SetWeightsSampler(sampler);
                channel->SetTargetProperty(AnimationTarget::Weights);
            }
            
            animation->AddChannel(channel);
        }
    }
    
    return animation;
}

std::shared_ptr<AnimationChannel> GLTFLoader::ParseAnimationChannel(const nlohmann::json& channelJson) {
    auto channel = std::make_shared<AnimationChannel>();
    
    if (!channelJson.contains("target") || !channelJson["target"].contains("node")) {
        LogError("Animation channel missing target node");
        return nullptr;
    }
    
    uint32_t targetNode = channelJson["target"]["node"];
    channel->SetTargetNode(targetNode);
    
    return channel;
}

template<typename T>
std::shared_ptr<AnimationSampler<T>> GLTFLoader::ParseAnimationSampler(const nlohmann::json& samplerJson) {
    auto sampler = std::make_shared<AnimationSampler<T>>();
    
    if (!samplerJson.contains("input") || !samplerJson.contains("output")) {
        LogError("Animation sampler missing input or output");
        return nullptr;
    }
    
    uint32_t inputAccessor = samplerJson["input"];
    uint32_t outputAccessor = samplerJson["output"];
    
    // Parse interpolation type
    std::string interpolation = samplerJson.value("interpolation", "LINEAR");
    sampler->SetInterpolationType(ParseInterpolationType(interpolation));
    
    // Get time values
    auto timeValues = GetAccessorData<float>(inputAccessor);
    
    // Get output values
    std::vector<T> outputValues;
    if constexpr (std::is_same_v<T, Math::Vec3>) {
        outputValues = GetVec3AccessorData(outputAccessor);
    } else if constexpr (std::is_same_v<T, Math::Quat>) {
        auto quatData = GetAccessorData<Math::Vec4>(outputAccessor);
        outputValues.reserve(quatData.size());
        for (const auto& q : quatData) {
            outputValues.emplace_back(q.w, q.x, q.y, q.z); // GLTF uses (x,y,z,w), GLM uses (w,x,y,z)
        }
    } else if constexpr (std::is_same_v<T, std::vector<float>>) {
        // For morph target weights, we need to handle variable-length arrays
        auto floatData = GetAccessorData<float>(outputAccessor);
        // Group floats into vectors based on the number of morph targets
        // This is a simplified approach - in practice, we'd need to know the target count
        outputValues.push_back(floatData);
    }
    
    // Create keyframes
    std::vector<Keyframe<T>> keyframes;
    size_t keyframeCount = std::min(timeValues.size(), outputValues.size());
    
    for (size_t i = 0; i < keyframeCount; ++i) {
        keyframes.emplace_back(timeValues[i], outputValues[i]);
    }
    
    sampler->SetKeyframes(keyframes);
    return sampler;
}

InterpolationType GLTFLoader::ParseInterpolationType(const std::string& interpolation) {
    if (interpolation == "LINEAR") {
        return InterpolationType::Linear;
    } else if (interpolation == "STEP") {
        return InterpolationType::Step;
    } else if (interpolation == "CUBICSPLINE") {
        return InterpolationType::CubicSpline;
    } else {
        LogWarning("Unknown interpolation type: " + interpolation + ", using LINEAR");
        return InterpolationType::Linear;
    }
}

std::shared_ptr<Skin> GLTFLoader::ParseSkin(const nlohmann::json& skinJson, uint32_t skinIndex) {
    auto skin = std::make_shared<Skin>();
    
    if (!skinJson.contains("joints")) {
        LogError("Skin missing joints array");
        return nullptr;
    }
    
    // Parse joint indices
    std::vector<uint32_t> joints;
    for (uint32_t jointIndex : skinJson["joints"]) {
        joints.push_back(jointIndex);
    }
    skin->SetJoints(joints);
    
    // Parse inverse bind matrices
    if (skinJson.contains("inverseBindMatrices")) {
        uint32_t accessorIndex = skinJson["inverseBindMatrices"];
        auto matrices = GetAccessorData<Math::Mat4>(accessorIndex);
        skin->SetInverseBindMatrices(matrices);
    }
    
    return skin;
}

std::shared_ptr<Skeleton> GLTFLoader::CreateSkeletonFromSkin(const nlohmann::json& skinJson) {
    if (!skinJson.contains("joints") || !m_gltfJson.contains("nodes")) {
        return nullptr;
    }
    
    auto skeleton = std::make_shared<Skeleton>();
    std::vector<std::shared_ptr<Bone>> bones;
    
    const auto& nodesJson = m_gltfJson["nodes"];
    const auto& jointIndices = skinJson["joints"];
    
    // Create bones for each joint
    for (size_t i = 0; i < jointIndices.size(); ++i) {
        uint32_t nodeIndex = jointIndices[i];
        
        if (nodeIndex >= nodesJson.size()) {
            LogError("Joint node index out of range: " + std::to_string(nodeIndex));
            continue;
        }
        
        const auto& nodeJson = nodesJson[nodeIndex];
        std::string boneName = nodeJson.value("name", "Bone_" + std::to_string(nodeIndex));
        
        auto bone = std::make_shared<Bone>(boneName, static_cast<int32_t>(i));
        
        // Parse transform
        Math::Mat4 transform = Math::Mat4(1.0f);
        if (nodeJson.contains("matrix")) {
            transform = ParseMatrix(nodeJson["matrix"]);
        } else {
            // TRS transform
            if (nodeJson.contains("translation")) {
                Math::Vec3 translation = ParseVec3(nodeJson["translation"]);
                transform = glm::translate(transform, translation);
            }
            
            if (nodeJson.contains("rotation")) {
                auto rotJson = nodeJson["rotation"];
                if (rotJson.size() == 4) {
                    Math::Quat rotation(rotJson[3], rotJson[0], rotJson[1], rotJson[2]); // w, x, y, z
                    transform = transform * glm::mat4_cast(rotation);
                }
            }
            
            if (nodeJson.contains("scale")) {
                Math::Vec3 scale = ParseVec3(nodeJson["scale"], Math::Vec3(1.0f));
                transform = glm::scale(transform, scale);
            }
        }
        
        bone->SetLocalTransform(transform);
        bones.push_back(bone);
    }
    
    skeleton->SetBones(bones);
    
    // Build hierarchy by examining node children
    for (size_t i = 0; i < jointIndices.size(); ++i) {
        uint32_t nodeIndex = jointIndices[i];
        const auto& nodeJson = nodesJson[nodeIndex];
        
        if (nodeJson.contains("children")) {
            for (uint32_t childNodeIndex : nodeJson["children"]) {
                // Find if this child is also a joint
                auto childJointIt = std::find(jointIndices.begin(), jointIndices.end(), childNodeIndex);
                if (childJointIt != jointIndices.end()) {
                    size_t childJointIndex = std::distance(jointIndices.begin(), childJointIt);
                    bones[i]->AddChild(bones[childJointIndex]);
                }
            }
        }
    }
    
    skeleton->BuildHierarchy();
    return skeleton;
}

std::shared_ptr<MorphTargetSet> GLTFLoader::ParseMorphTargets(const nlohmann::json& targetsJson) {
    auto morphTargetSet = std::make_shared<MorphTargetSet>();
    
    for (size_t i = 0; i < targetsJson.size(); ++i) {
        const auto& targetJson = targetsJson[i];
        
        std::string targetName = "MorphTarget_" + std::to_string(i);
        auto morphTarget = std::make_shared<MorphTarget>(targetName);
        
        // Parse position deltas
        if (targetJson.contains("POSITION")) {
            uint32_t accessorIndex = targetJson["POSITION"];
            auto positionDeltas = GetVec3AccessorData(accessorIndex);
            morphTarget->SetVertexDeltas(positionDeltas);
        }
        
        // Parse normal deltas
        if (targetJson.contains("NORMAL")) {
            uint32_t accessorIndex = targetJson["NORMAL"];
            auto normalDeltas = GetVec3AccessorData(accessorIndex);
            morphTarget->SetNormalDeltas(normalDeltas);
        }
        
        // Parse tangent deltas
        if (targetJson.contains("TANGENT")) {
            uint32_t accessorIndex = targetJson["TANGENT"];
            auto tangentData = GetAccessorData<Math::Vec4>(accessorIndex);
            
            std::vector<Math::Vec3> tangentDeltas;
            tangentDeltas.reserve(tangentData.size());
            for (const auto& t : tangentData) {
                tangentDeltas.emplace_back(t.x, t.y, t.z);
            }
            morphTarget->SetTangentDeltas(tangentDeltas);
        }
        
        morphTargetSet->AddMorphTarget(morphTarget);
    }
    
    return morphTargetSet;
}

// Explicit template instantiations for animation samplers
template std::shared_ptr<AnimationSampler<Math::Vec3>> GLTFLoader::ParseAnimationSampler<Math::Vec3>(const nlohmann::json& samplerJson);
template std::shared_ptr<AnimationSampler<Math::Quat>> GLTFLoader::ParseAnimationSampler<Math::Quat>(const nlohmann::json& samplerJson);
template std::shared_ptr<AnimationSampler<std::vector<float>>> GLTFLoader::ParseAnimationSampler<std::vector<float>>(const nlohmann::json& samplerJson);

} // namespace GameEngine