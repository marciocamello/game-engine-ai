# 3D Model Loading System

Game Engine Kiro v1.1 introduces comprehensive 3D model loading capabilities with support for industry-standard formats including FBX, GLTF, OBJ, and more, providing a complete asset pipeline for modern game development.

## 🎯 Overview

The 3D Model Loading System extends the basic resource management in v1.0 to handle complex 3D assets with materials, textures, animations, and hierarchical structures, enabling developers to import assets directly from content creation tools.

### Supported Formats

- **GLTF 2.0**: Modern, efficient format with PBR materials
- **FBX**: Industry standard from Autodesk Maya/3ds Max
- **OBJ**: Simple format with MTL material support
- **DAE (Collada)**: Open standard with animation support
- **3DS**: Legacy format support
- **PLY**: Point cloud and mesh data

### Key Features

- **Assimp Integration**: Robust format support via Open Asset Import Library
- **Material Import**: Automatic material and texture loading
- **Mesh Optimization**: Vertex cache optimization and LOD generation
- **Animation Support**: Skeletal and vertex animations
- **Scene Hierarchy**: Node-based scene graph import
- **Texture Embedding**: Support for embedded textures

## 🏗️ Architecture Overview

### Model Loading Pipeline

```
3D File → Assimp → Scene Graph → Mesh Processing → Material Loading → GPU Upload
   ↓         ↓          ↓             ↓               ↓              ↓
Format    Parse     Node Tree    Vertex Buffers   Texture Load   Render Ready
Detection  Scene     Creation     Index Buffers    Material       Model Object
```

### Core Components

**ModelLoader**

```cpp
class ModelLoader {
public:
    // Model loading
    std::shared_ptr<Model> LoadModel(const std::string& filepath);
    std::shared_ptr<Model> LoadModelAsync(const std::string& filepath);

    // Format support
    bool IsFormatSupported(const std::string& extension) const;
    std::vector<std::string> GetSupportedFormats() const;

    // Loading options
    void SetLoadingFlags(ModelLoadingFlags flags);
    void SetPostProcessingSteps(PostProcessingSteps steps);

private:
    std::unique_ptr<Assimp::Importer> m_importer;
    ModelLoadingFlags m_loadingFlags;
    PostProcessingSteps m_postProcessing;
};
```

**Model**

```cpp
class Model : public Resource {
public:
    // Rendering
    void Render(const Math::Mat4& transform, std::shared_ptr<Shader> shader);
    void RenderInstanced(const std::vector<Math::Mat4>& transforms, std::shared_ptr<Shader> shader);

    // Scene graph access
    std::shared_ptr<ModelNode> GetRootNode() const;
    std::vector<std::shared_ptr<Mesh>> GetMeshes() const;
    std::vector<std::shared_ptr<Material>> GetMaterials() const;

    // Animation
    bool HasAnimations() const;
    std::vector<std::shared_ptr<Animation>> GetAnimations() const;
    void PlayAnimation(const std::string& name, bool loop = true);

    // Bounding information
    BoundingBox GetBoundingBox() const;
    BoundingSphere GetBoundingSphere() const;

    // LOD support
    void SetLODLevels(const std::vector<std::shared_ptr<Model>>& lodLevels);
    std::shared_ptr<Model> GetLOD(float distance) const;

private:
    std::shared_ptr<ModelNode> m_rootNode;
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<std::shared_ptr<Material>> m_materials;
    std::vector<std::shared_ptr<Animation>> m_animations;
    BoundingBox m_boundingBox;
};
```

**ModelNode**

```cpp
class ModelNode {
public:
    // Hierarchy
    void AddChild(std::shared_ptr<ModelNode> child);
    std::vector<std::shared_ptr<ModelNode>> GetChildren() const;
    std::shared_ptr<ModelNode> GetParent() const;

    // Transform
    void SetTransform(const Math::Mat4& transform);
    Math::Mat4 GetTransform() const;
    Math::Mat4 GetWorldTransform() const;

    // Meshes
    void AddMesh(uint32_t meshIndex);
    std::vector<uint32_t> GetMeshIndices() const;

    // Properties
    void SetName(const std::string& name);
    const std::string& GetName() const;

private:
    std::string m_name;
    Math::Mat4 m_transform = Math::Mat4(1.0f);
    std::vector<uint32_t> m_meshIndices;
    std::vector<std::shared_ptr<ModelNode>> m_children;
    std::weak_ptr<ModelNode> m_parent;
};
```

## 📦 Enhanced Mesh System

### Advanced Mesh Features

```cpp
class Mesh : public Resource {
public:
    enum class PrimitiveType { Triangles, Lines, Points };

    // Vertex data
    void SetVertices(const std::vector<Vertex>& vertices);
    void SetIndices(const std::vector<uint32_t>& indices);
    void SetPrimitiveType(PrimitiveType type);

    // Vertex attributes
    void EnableAttribute(VertexAttribute attribute);
    void SetVertexLayout(const VertexLayout& layout);

    // Materials
    void SetMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial() const;

    // Optimization
    void OptimizeVertexCache();
    void GenerateTangents();
    void GenerateNormals();
    void Simplify(float ratio);

    // LOD generation
    std::shared_ptr<Mesh> GenerateLOD(float simplificationRatio);

    // Rendering
    void Render() const;
    void RenderInstanced(uint32_t instanceCount) const;

    // Statistics
    uint32_t GetVertexCount() const;
    uint32_t GetTriangleCount() const;
    size_t GetMemoryUsage() const;

private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::shared_ptr<Material> m_material;
    VertexLayout m_layout;
    PrimitiveType m_primitiveType = PrimitiveType::Triangles;

    // OpenGL resources
    GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
};
```

### Vertex Structure

```cpp
struct Vertex {
    Math::Vec3 position;
    Math::Vec3 normal;
    Math::Vec2 texCoords;
    Math::Vec3 tangent;
    Math::Vec3 bitangent;
    Math::Vec4 color = Math::Vec4(1.0f);

    // Skinning data (for animated models)
    Math::Vec4 boneIds = Math::Vec4(0.0f);
    Math::Vec4 boneWeights = Math::Vec4(0.0f);
};

struct VertexLayout {
    struct Attribute {
        VertexAttribute type;
        uint32_t offset;
        uint32_t size;
        GLenum dataType;
        bool normalized;
    };

    std::vector<Attribute> attributes;
    uint32_t stride;
};
```

## 🎨 Material Import System

### Material Loading

```cpp
class MaterialImporter {
public:
    // Import from model files
    std::vector<std::shared_ptr<Material>> ImportMaterials(const aiScene* scene, const std::string& modelPath);
    std::shared_ptr<Material> ImportMaterial(const aiMaterial* aiMat, const std::string& modelPath);

    // Texture loading
    std::shared_ptr<Texture> LoadEmbeddedTexture(const aiScene* scene, const std::string& texturePath);
    std::shared_ptr<Texture> LoadExternalTexture(const std::string& texturePath, const std::string& modelPath);

    // Material conversion
    std::shared_ptr<PBRMaterial> ConvertToPBR(const aiMaterial* aiMat, const std::string& modelPath);
    std::shared_ptr<UnlitMaterial> ConvertToUnlit(const aiMaterial* aiMat, const std::string& modelPath);

private:
    std::shared_ptr<ResourceManager> m_resourceManager;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
};
```

### Material Property Mapping

```cpp
// Assimp to Engine material property mapping
struct MaterialPropertyMapping {
    // PBR properties
    aiTextureType aiAlbedo = aiTextureType_DIFFUSE;
    aiTextureType aiNormal = aiTextureType_NORMALS;
    aiTextureType aiMetallic = aiTextureType_METALNESS;
    aiTextureType aiRoughness = aiTextureType_DIFFUSE_ROUGHNESS;
    aiTextureType aiAO = aiTextureType_AMBIENT_OCCLUSION;
    aiTextureType aiEmission = aiTextureType_EMISSIVE;

    // Legacy properties
    aiTextureType aiSpecular = aiTextureType_SPECULAR;
    aiTextureType aiGloss = aiTextureType_SHININESS;
    aiTextureType aiHeight = aiTextureType_HEIGHT;
};
```

## 🎬 Animation System Integration

### Animation Data Structures

```cpp
class Animation {
public:
    struct Channel {
        std::string nodeName;
        std::vector<VectorKey> positionKeys;
        std::vector<QuaternionKey> rotationKeys;
        std::vector<VectorKey> scalingKeys;
    };

    // Animation properties
    void SetName(const std::string& name);
    void SetDuration(float duration);
    void SetTicksPerSecond(float tps);

    // Channel management
    void AddChannel(const Channel& channel);
    std::vector<Channel> GetChannels() const;

    // Playback
    Math::Mat4 GetNodeTransform(const std::string& nodeName, float time) const;
    void Sample(float time, std::unordered_map<std::string, Math::Mat4>& transforms) const;

private:
    std::string m_name;
    float m_duration = 0.0f;
    float m_ticksPerSecond = 25.0f;
    std::vector<Channel> m_channels;
};

struct VectorKey {
    float time;
    Math::Vec3 value;
};

struct QuaternionKey {
    float time;
    Math::Quat value;
};
```

### Skeletal Animation Support

```cpp
class Skeleton {
public:
    struct Bone {
        std::string name;
        Math::Mat4 offsetMatrix;
        int parentIndex = -1;
        std::vector<int> childIndices;
    };

    // Bone management
    void AddBone(const Bone& bone);
    const Bone& GetBone(int index) const;
    int GetBoneIndex(const std::string& name) const;

    // Animation
    void UpdateBoneMatrices(const std::unordered_map<std::string, Math::Mat4>& transforms);
    std::vector<Math::Mat4> GetBoneMatrices() const;

private:
    std::vector<Bone> m_bones;
    std::vector<Math::Mat4> m_boneMatrices;
    std::unordered_map<std::string, int> m_boneNameToIndex;
};
```

## ⚙️ Loading Configuration

### Loading Flags

```cpp
enum class ModelLoadingFlags : uint32_t {
    None = 0,
    FlipUVs = 1 << 0,
    GenerateNormals = 1 << 1,
    GenerateTangents = 1 << 2,
    OptimizeMeshes = 1 << 3,
    OptimizeGraph = 1 << 4,
    FlipWindingOrder = 1 << 5,
    MakeLeftHanded = 1 << 6,
    RemoveDuplicateVertices = 1 << 7,
    SortByPrimitiveType = 1 << 8,
    ImproveCacheLocality = 1 << 9,
    LimitBoneWeights = 1 << 10,
    SplitLargeMeshes = 1 << 11,
    Triangulate = 1 << 12,
    ValidateDataStructure = 1 << 13
};
```

### Post-Processing Steps

```cpp
enum class PostProcessingSteps : uint32_t {
    None = 0,
    CalcTangentSpace = aiProcess_CalcTangentSpace,
    JoinIdenticalVertices = aiProcess_JoinIdenticalVertices,
    MakeLeftHanded = aiProcess_MakeLeftHanded,
    Triangulate = aiProcess_Triangulate,
    RemoveComponent = aiProcess_RemoveComponent,
    GenNormals = aiProcess_GenNormals,
    GenSmoothNormals = aiProcess_GenSmoothNormals,
    SplitLargeMeshes = aiProcess_SplitLargeMeshes,
    PreTransformVertices = aiProcess_PreTransformVertices,
    LimitBoneWeights = aiProcess_LimitBoneWeights,
    ValidateDataStructure = aiProcess_ValidateDataStructure,
    ImproveCacheLocality = aiProcess_ImproveCacheLocality,
    RemoveRedundantMaterials = aiProcess_RemoveRedundantMaterials,
    FixInfacingNormals = aiProcess_FixInfacingNormals,
    SortByPType = aiProcess_SortByPType,
    FindDegenerates = aiProcess_FindDegenerates,
    FindInvalidData = aiProcess_FindInvalidData,
    GenUVCoords = aiProcess_GenUVCoords,
    TransformUVCoords = aiProcess_TransformUVCoords,
    FindInstances = aiProcess_FindInstances,
    OptimizeMeshes = aiProcess_OptimizeMeshes,
    OptimizeGraph = aiProcess_OptimizeGraph,
    FlipUVs = aiProcess_FlipUVs,
    FlipWindingOrder = aiProcess_FlipWindingOrder
};
```

## 🚀 Performance Optimization

### Mesh Optimization

```cpp
class MeshOptimizer {
public:
    // Vertex cache optimization
    static void OptimizeVertexCache(Mesh& mesh);
    static void OptimizeVertexFetch(Mesh& mesh);

    // Overdraw optimization
    static void OptimizeOverdraw(Mesh& mesh, float threshold = 1.05f);

    // Simplification
    static std::shared_ptr<Mesh> Simplify(const Mesh& mesh, float ratio);
    static std::shared_ptr<Mesh> SimplifyToTargetError(const Mesh& mesh, float maxError);

    // LOD generation
    static std::vector<std::shared_ptr<Mesh>> GenerateLODChain(const Mesh& mesh, const std::vector<float>& ratios);

    // Statistics
    static MeshStats AnalyzeMesh(const Mesh& mesh);

private:
    static void ReorderIndices(std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices);
    static void ReorderVertices(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
};
```

### Asynchronous Loading

```cpp
class AsyncModelLoader {
public:
    // Async loading
    std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath);
    std::future<std::vector<std::shared_ptr<Model>>> LoadModelsAsync(const std::vector<std::string>& filepaths);

    // Progress tracking
    void SetProgressCallback(std::function<void(float)> callback);
    float GetLoadingProgress(const std::string& filepath) const;

    // Thread management
    void SetWorkerThreadCount(uint32_t count);
    uint32_t GetWorkerThreadCount() const;

private:
    std::unique_ptr<ThreadPool> m_threadPool;
    std::function<void(float)> m_progressCallback;
    std::unordered_map<std::string, float> m_loadingProgress;
};
```

## 🎮 Usage Examples

### Basic Model Loading

```cpp
// Load a model
auto modelLoader = std::make_unique<ModelLoader>();
modelLoader->SetLoadingFlags(
    ModelLoadingFlags::Triangulate |
    ModelLoadingFlags::GenerateNormals |
    ModelLoadingFlags::OptimizeMeshes
);

auto model = modelLoader->LoadModel("models/character.fbx");
if (model) {
    LOG_INFO("Model loaded successfully: " + std::to_string(model->GetMeshes().size()) + " meshes");
}

// Render the model
Math::Mat4 transform = Math::CreateTransformMatrix(position, rotation, scale);
model->Render(transform, shader);
```

### Advanced Model Loading with Materials

```cpp
// Configure loading for PBR materials
ModelLoadingFlags flags =
    ModelLoadingFlags::Triangulate |
    ModelLoadingFlags::GenerateNormals |
    ModelLoadingFlags::GenerateTangents |
    ModelLoadingFlags::OptimizeMeshes |
    ModelLoadingFlags::ImproveCacheLocality;

modelLoader->SetLoadingFlags(flags);

// Load model with materials
auto model = modelLoader->LoadModel("models/pbr_asset.gltf");

// Access materials
auto materials = model->GetMaterials();
for (auto& material : materials) {
    if (auto pbrMaterial = std::dynamic_pointer_cast<PBRMaterial>(material)) {
        LOG_INFO("PBR Material: " + pbrMaterial->GetName());
        LOG_INFO("  Albedo: " + std::to_string(pbrMaterial->GetAlbedo().r));
        LOG_INFO("  Metallic: " + std::to_string(pbrMaterial->GetMetallic()));
        LOG_INFO("  Roughness: " + std::to_string(pbrMaterial->GetRoughness()));
    }
}
```

### Scene Graph Traversal

```cpp
void TraverseNode(std::shared_ptr<ModelNode> node, const Math::Mat4& parentTransform) {
    Math::Mat4 nodeTransform = parentTransform * node->GetTransform();

    // Render meshes for this node
    for (uint32_t meshIndex : node->GetMeshIndices()) {
        auto mesh = model->GetMeshes()[meshIndex];
        mesh->Render(nodeTransform, shader);
    }

    // Recursively traverse children
    for (auto child : node->GetChildren()) {
        TraverseNode(child, nodeTransform);
    }
}

// Start traversal from root
TraverseNode(model->GetRootNode(), Math::Mat4(1.0f));
```

### Animation Playback

```cpp
// Load animated model
auto animatedModel = modelLoader->LoadModel("models/animated_character.fbx");

if (animatedModel->HasAnimations()) {
    auto animations = animatedModel->GetAnimations();
    LOG_INFO("Found " + std::to_string(animations.size()) + " animations");

    // Play first animation
    animatedModel->PlayAnimation(animations[0]->GetName(), true);
}

// Update animation in game loop
void Update(float deltaTime) {
    animatedModel->UpdateAnimation(deltaTime);

    // Render with bone matrices
    auto boneMatrices = animatedModel->GetBoneMatrices();
    shader->SetUniform("u_boneMatrices", boneMatrices);
    animatedModel->Render(transform, shader);
}
```

### LOD System

```cpp
// Generate LOD levels
auto highLOD = model;  // Original model
auto mediumLOD = MeshOptimizer::Simplify(*highLOD, 0.5f);  // 50% vertices
auto lowLOD = MeshOptimizer::Simplify(*highLOD, 0.25f);    // 25% vertices

model->SetLODLevels({highLOD, mediumLOD, lowLOD});

// Automatic LOD selection based on distance
float distanceToCamera = Math::Length(cameraPos - modelPos);
auto lodModel = model->GetLOD(distanceToCamera);
lodModel->Render(transform, shader);
```

## 🔧 Integration with Engine Systems

### Resource Manager Integration

```cpp
// Register model loader with resource manager
resourceManager->RegisterLoader<Model>(std::make_unique<ModelLoader>());

// Load models through resource manager
auto model = resourceManager->Load<Model>("models/building.fbx");
auto character = resourceManager->Load<Model>("models/character.gltf");

// Automatic caching and memory management
auto sameBuildingModel = resourceManager->Load<Model>("models/building.fbx");  // Returns cached version
```

### Graphics System Integration

```cpp
// Enhanced primitive renderer with model support
class PrimitiveRenderer {
public:
    // Model rendering
    void DrawModel(std::shared_ptr<Model> model, const Math::Mat4& transform);
    void DrawModelInstanced(std::shared_ptr<Model> model, const std::vector<Math::Mat4>& transforms);

    // Batch rendering
    void BeginBatch();
    void AddModelToBatch(std::shared_ptr<Model> model, const Math::Mat4& transform);
    void EndBatch();

private:
    std::vector<ModelRenderCommand> m_renderQueue;
};
```

## 🔮 Future Enhancements

### Planned Features (v1.2+)

- **Streaming LOD**: Dynamic level-of-detail based on performance
- **Instanced Rendering**: Efficient rendering of multiple instances
- **GPU Skinning**: Hardware-accelerated skeletal animation
- **Morph Targets**: Facial animation and blend shapes
- **Procedural Generation**: Runtime mesh generation
- **Asset Compression**: Reduced file sizes and loading times

### Advanced Features

- **Mesh Shaders**: Next-generation geometry pipeline
- **Variable Rate Shading**: Performance optimization
- **Nanite-style Virtualized Geometry**: Unlimited detail
- **Real-time Global Illumination**: Advanced lighting
- **Physics Mesh Generation**: Automatic collision mesh creation

## 📚 Best Practices

### Model Optimization

1. **Use Appropriate LODs**: Generate multiple detail levels
2. **Optimize Vertex Count**: Remove unnecessary vertices
3. **Texture Atlas**: Combine multiple textures
4. **Material Batching**: Reduce draw calls
5. **Bone Limit**: Keep bone count reasonable for skinning

### Performance Considerations

1. **Async Loading**: Load models on background threads
2. **Memory Management**: Unload unused models
3. **Culling**: Don't render off-screen models
4. **Instancing**: Use for repeated objects
5. **Cache Optimization**: Optimize vertex cache usage

---

The 3D Model Loading System in Game Engine Kiro v1.1 provides comprehensive support for modern 3D assets, enabling developers to create rich, detailed game worlds with professional-quality models and animations.

**Game Engine Kiro v1.1** - Bringing your 3D assets to life.
