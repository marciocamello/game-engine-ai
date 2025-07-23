# Design Document - 3D Model Loading System

## Overview

This design document outlines the implementation of a comprehensive 3D model loading system for Game Engine Kiro v1.1. The system provides robust support for industry-standard formats (GLTF, FBX, OBJ, DAE) with automatic material import, mesh optimization, animation support, and seamless integration with the engine's resource management system.

## Architecture

### 3D Model Loading System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Model Loading API                        │
├─────────────────────────────────────────────────────────────┤
│  ModelLoader │ AsyncLoader │ Progress │ Cache │ Validation  │
├─────────────────────────────────────────────────────────────┤
│                   Format Processors                        │
├─────────────────────────────────────────────────────────────┤
│  GLTF Loader │ FBX Loader │ OBJ Loader │ DAE Loader        │
├─────────────────────────────────────────────────────────────┤
│                   Assimp Integration                        │
├─────────────────────────────────────────────────────────────┤
│ Scene Import │ Mesh Process │ Material │ Animation │ Texture │
├─────────────────────────────────────────────────────────────┤
│                   Model Processing                          │
├─────────────────────────────────────────────────────────────┤
│ Mesh Optimize │ LOD Generate │ Bounds │ Validation │ Convert │
├─────────────────────────────────────────────────────────────┤
│                   Engine Integration                        │
├─────────────────────────────────────────────────────────────┤
│ Resource Mgr │ Graphics │ Animation │ Physics │ Audio       │
└─────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. ModelLoader Core System

```cpp
class ModelLoader {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();

    // Synchronous loading
    std::shared_ptr<Model> LoadModel(const std::string& filepath);
    std::shared_ptr<Model> LoadModelFromMemory(const std::vector<uint8_t>& data, const std::string& format);

    // Asynchronous loading
    std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath);
    std::future<std::vector<std::shared_ptr<Model>>> LoadModelsAsync(const std::vector<std::string>& filepaths);

    // Format support
    bool IsFormatSupported(const std::string& extension) const;
    std::vector<std::string> GetSupportedFormats() const;

    // Loading configuration
    void SetLoadingFlags(ModelLoadingFlags flags);
    void SetPostProcessingSteps(PostProcessingSteps steps);
    void SetImportScale(float scale);
    void SetCoordinateSystem(CoordinateSystem system);

    // Progress tracking
    void SetProgressCallback(std::function<void(float)> callback);
    float GetLoadingProgress(const std::string& filepath) const;

    // Performance and debugging
    ModelLoadingStats GetLoadingStats() const;
    void SetVerboseLogging(bool enabled);

private:
    std::unique_ptr<Assimp::Importer> m_importer;
    std::unique_ptr<AsyncModelLoader> m_asyncLoader;
    std::unique_ptr<ModelCache> m_cache;
    std::unique_ptr<MaterialImporter> m_materialImporter;

    ModelLoadingFlags m_loadingFlags;
    PostProcessingSteps m_postProcessing;
    float m_importScale = 1.0f;
    CoordinateSystem m_coordinateSystem = CoordinateSystem::RightHanded;

    std::function<void(float)> m_progressCallback;
    bool m_verboseLogging = false;

    // Helper methods
    std::shared_ptr<Model> ProcessScene(const aiScene* scene, const std::string& filepath);
    void ValidateScene(const aiScene* scene);
    void OptimizeScene(const aiScene* scene);
};
```

### 2. Enhanced Model Class

```cpp
class Model : public Resource {
public:
    // Lifecycle
    Model(const std::string& filepath);
    ~Model();

    // Scene graph access
    std::shared_ptr<ModelNode> GetRootNode() const;
    std::shared_ptr<ModelNode> FindNode(const std::string& name) const;
    std::vector<std::shared_ptr<ModelNode>> GetAllNodes() const;

    // Mesh access
    std::vector<std::shared_ptr<Mesh>> GetMeshes() const;
    std::shared_ptr<Mesh> GetMesh(size_t index) const;
    std::shared_ptr<Mesh> FindMesh(const std::string& name) const;

    // Material access
    std::vector<std::shared_ptr<Material>> GetMaterials() const;
    std::shared_ptr<Material> GetMaterial(size_t index) const;
    std::shared_ptr<Material> FindMaterial(const std::string& name) const;

    // Animation access
    bool HasAnimations() const;
    std::vector<std::shared_ptr<Animation>> GetAnimations() const;
    std::shared_ptr<Animation> GetAnimation(size_t index) const;
    std::shared_ptr<Animation> FindAnimation(const std::string& name) const;

    // Skeleton access
    std::shared_ptr<Skeleton> GetSkeleton() const;
    bool HasSkeleton() const;

    // Rendering
    void Render(const Math::Mat4& transform, std::shared_ptr<Shader> shader);
    void RenderNode(std::shared_ptr<ModelNode> node, const Math::Mat4& parentTransform, std::shared_ptr<Shader> shader);
    void RenderInstanced(const std::vector<Math::Mat4>& transforms, std::shared_ptr<Shader> shader);

    // Bounding information
    BoundingBox GetBoundingBox() const;
    BoundingSphere GetBoundingSphere() const;
    void UpdateBounds();

    // LOD support
    void SetLODLevels(const std::vector<std::shared_ptr<Model>>& lodLevels);
    std::shared_ptr<Model> GetLOD(float distance) const;
    size_t GetLODCount() const;

    // Statistics and debugging
    ModelStats GetStats() const;
    void PrintDebugInfo() const;

    // Serialization (for caching)
    bool SaveToCache(const std::string& cachePath) const;
    bool LoadFromCache(const std::string& cachePath);

private:
    std::shared_ptr<ModelNode> m_rootNode;
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<std::shared_ptr<Material>> m_materials;
    std::vector<std::shared_ptr<Animation>> m_animations;
    std::shared_ptr<Skeleton> m_skeleton;
    std::vector<std::shared_ptr<Model>> m_lodLevels;

    BoundingBox m_boundingBox;
    BoundingSphere m_boundingSphere;
    ModelStats m_stats;

    // Helper methods
    void CalculateBounds();
    void OptimizeMeshes();
    void ValidateModel();
};
```

### 3. ModelNode Hierarchy System

```cpp
class ModelNode {
public:
    // Lifecycle
    ModelNode(const std::string& name = "");
    ~ModelNode();

    // Hierarchy management
    void AddChild(std::shared_ptr<ModelNode> child);
    void RemoveChild(std::shared_ptr<ModelNode> child);
    std::vector<std::shared_ptr<ModelNode>> GetChildren() const;
    std::shared_ptr<ModelNode> GetParent() const;
    std::shared_ptr<ModelNode> FindChild(const std::string& name) const;

    // Transform management
    void SetLocalTransform(const Math::Mat4& transform);
    Math::Mat4 GetLocalTransform() const;
    Math::Mat4 GetWorldTransform() const;
    void UpdateWorldTransform(const Math::Mat4& parentTransform = Math::Mat4(1.0f));

    // Mesh association
    void AddMeshIndex(uint32_t meshIndex);
    void RemoveMeshIndex(uint32_t meshIndex);
    std::vector<uint32_t> GetMeshIndices() const;
    bool HasMeshes() const;

    // Properties
    void SetName(const std::string& name);
    const std::string& GetName() const;
    void SetVisible(bool visible);
    bool IsVisible() const;

    // Traversal
    void Traverse(std::function<void(std::shared_ptr<ModelNode>)> callback);
    void TraverseDepthFirst(std::function<void(std::shared_ptr<ModelNode>)> callback);
    void TraverseBreadthFirst(std::function<void(std::shared_ptr<ModelNode>)> callback);

    // Bounding information
    BoundingBox GetLocalBounds() const;
    BoundingBox GetWorldBounds() const;

private:
    std::string m_name;
    Math::Mat4 m_localTransform = Math::Mat4(1.0f);
    Math::Mat4 m_worldTransform = Math::Mat4(1.0f);

    std::vector<uint32_t> m_meshIndices;
    std::vector<std::shared_ptr<ModelNode>> m_children;
    std::weak_ptr<ModelNode> m_parent;

    bool m_visible = true;
    BoundingBox m_localBounds;

    void UpdateChildTransforms();
};
```

### 4. Enhanced Mesh System

```cpp
class Mesh : public Resource {
public:
    enum class PrimitiveType { Triangles, Lines, Points, TriangleStrip, TriangleFan };

    // Lifecycle
    Mesh(const std::string& name = "");
    ~Mesh();

    // Vertex data management
    void SetVertices(const std::vector<Vertex>& vertices);
    void SetIndices(const std::vector<uint32_t>& indices);
    const std::vector<Vertex>& GetVertices() const;
    const std::vector<uint32_t>& GetIndices() const;

    // Vertex attributes
    void SetVertexLayout(const VertexLayout& layout);
    VertexLayout GetVertexLayout() const;
    void EnableAttribute(VertexAttribute attribute);
    void DisableAttribute(VertexAttribute attribute);

    // Material association
    void SetMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial() const;
    void SetMaterialIndex(uint32_t index);
    uint32_t GetMaterialIndex() const;

    // Primitive type
    void SetPrimitiveType(PrimitiveType type);
    PrimitiveType GetPrimitiveType() const;

    // Mesh optimization
    void OptimizeVertexCache();
    void OptimizeVertexFetch();
    void OptimizeOverdraw(float threshold = 1.05f);
    void RemoveDuplicateVertices();
    void GenerateNormals(bool smooth = true);
    void GenerateTangents();

    // LOD generation
    std::shared_ptr<Mesh> GenerateLOD(float simplificationRatio);
    std::shared_ptr<Mesh> SimplifyToTargetError(float maxError);
    std::vector<std::shared_ptr<Mesh>> GenerateLODChain(const std::vector<float>& ratios);

    // Bounding information
    BoundingBox GetBoundingBox() const;
    BoundingSphere GetBoundingSphere() const;
    void UpdateBounds();

    // Rendering
    void Render() const;
    void RenderInstanced(uint32_t instanceCount) const;
    void RenderWithMaterial(std::shared_ptr<Shader> shader) const;

    // Statistics
    uint32_t GetVertexCount() const;
    uint32_t GetTriangleCount() const;
    size_t GetMemoryUsage() const override;
    MeshStats GetStats() const;

    // Validation
    bool Validate() const;
    std::vector<std::string> GetValidationErrors() const;

private:
    std::string m_name;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    VertexLayout m_layout;
    PrimitiveType m_primitiveType = PrimitiveType::Triangles;

    std::shared_ptr<Material> m_material;
    uint32_t m_materialIndex = 0;

    BoundingBox m_boundingBox;
    BoundingSphere m_boundingSphere;

    // OpenGL resources
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_EBO = 0;

    // Helper methods
    void CreateGLBuffers();
    void UpdateGLBuffers();
    void DestroyGLBuffers();
    void CalculateBounds();
};
```

### 5. Material Import System

```cpp
class MaterialImporter {
public:
    // Material import from Assimp
    std::vector<std::shared_ptr<Material>> ImportMaterials(const aiScene* scene, const std::string& modelPath);
    std::shared_ptr<Material> ImportMaterial(const aiMaterial* aiMat, const std::string& modelPath);

    // Texture loading
    std::shared_ptr<Texture> LoadEmbeddedTexture(const aiScene* scene, const std::string& texturePath);
    std::shared_ptr<Texture> LoadExternalTexture(const std::string& texturePath, const std::string& modelPath);

    // Material conversion
    std::shared_ptr<PBRMaterial> ConvertToPBR(const aiMaterial* aiMat, const std::string& modelPath);
    std::shared_ptr<UnlitMaterial> ConvertToUnlit(const aiMaterial* aiMat, const std::string& modelPath);

    // Configuration
    void SetTextureSearchPaths(const std::vector<std::string>& paths);
    void SetDefaultTextures(const DefaultTextures& textures);
    void SetMaterialConversionMode(MaterialConversionMode mode);

private:
    std::shared_ptr<ResourceManager> m_resourceManager;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
    std::vector<std::string> m_textureSearchPaths;
    DefaultTextures m_defaultTextures;
    MaterialConversionMode m_conversionMode = MaterialConversionMode::Auto;

    // Helper methods
    Math::Vec3 ConvertColor(const aiColor3D& color);
    std::string FindTexturePath(const std::string& texturePath, const std::string& modelPath);
    TextureType DetermineTextureType(aiTextureType aiType);
    std::shared_ptr<Texture> CreateDefaultTexture(TextureType type);
};
```

### 6. Asynchronous Loading System

```cpp
class AsyncModelLoader {
public:
    // Lifecycle
    bool Initialize(uint32_t workerThreadCount = 0);
    void Shutdown();

    // Async loading
    std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath);
    std::future<std::vector<std::shared_ptr<Model>>> LoadModelsAsync(const std::vector<std::string>& filepaths);

    // Progress tracking
    void SetProgressCallback(std::function<void(const std::string&, float)> callback);
    float GetLoadingProgress(const std::string& filepath) const;
    std::vector<std::string> GetActiveLoads() const;

    // Load management
    void CancelLoad(const std::string& filepath);
    void CancelAllLoads();
    void SetMaxConcurrentLoads(uint32_t maxLoads);

    // Thread management
    void SetWorkerThreadCount(uint32_t count);
    uint32_t GetWorkerThreadCount() const;

private:
    struct LoadTask {
        std::string filepath;
        std::promise<std::shared_ptr<Model>> promise;
        std::atomic<float> progress{0.0f};
        std::atomic<bool> cancelled{false};
    };

    std::unique_ptr<ThreadPool> m_threadPool;
    std::unordered_map<std::string, std::shared_ptr<LoadTask>> m_activeTasks;
    std::function<void(const std::string&, float)> m_progressCallback;

    uint32_t m_maxConcurrentLoads = 4;
    std::mutex m_tasksMutex;

    void ProcessLoadTask(std::shared_ptr<LoadTask> task);
    void UpdateProgress(const std::string& filepath, float progress);
};
```

### 7. Mesh Optimization System

```cpp
class MeshOptimizer {
public:
    // Vertex cache optimization
    static void OptimizeVertexCache(Mesh& mesh);
    static void OptimizeVertexFetch(Mesh& mesh);
    static std::vector<uint32_t> OptimizeIndices(const std::vector<uint32_t>& indices, size_t vertexCount);

    // Overdraw optimization
    static void OptimizeOverdraw(Mesh& mesh, float threshold = 1.05f);
    static std::vector<uint32_t> OptimizeOverdrawIndices(const std::vector<uint32_t>& indices,
                                                         const std::vector<Vertex>& vertices,
                                                         float threshold);

    // Mesh simplification
    static std::shared_ptr<Mesh> Simplify(const Mesh& mesh, float ratio);
    static std::shared_ptr<Mesh> SimplifyToTargetError(const Mesh& mesh, float maxError);
    static std::shared_ptr<Mesh> SimplifyToTriangleCount(const Mesh& mesh, uint32_t targetTriangles);

    // LOD generation
    static std::vector<std::shared_ptr<Mesh>> GenerateLODChain(const Mesh& mesh, const std::vector<float>& ratios);
    static std::vector<std::shared_ptr<Mesh>> GenerateAutomaticLODs(const Mesh& mesh, uint32_t lodCount);

    // Vertex processing
    static void RemoveDuplicateVertices(Mesh& mesh, float epsilon = 0.0001f);
    static void GenerateNormals(Mesh& mesh, bool smooth = true);
    static void GenerateTangents(Mesh& mesh);
    static void FlipNormals(Mesh& mesh);

    // Validation and analysis
    static MeshAnalysis AnalyzeMesh(const Mesh& mesh);
    static bool ValidateMesh(const Mesh& mesh);
    static std::vector<std::string> GetMeshIssues(const Mesh& mesh);

    // Statistics
    static MeshOptimizationStats GetOptimizationStats(const Mesh& originalMesh, const Mesh& optimizedMesh);

private:
    // Helper methods for optimization algorithms
    static void ReorderIndicesForCache(std::vector<uint32_t>& indices, size_t vertexCount);
    static void ReorderVerticesForFetch(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    static float CalculateACMR(const std::vector<uint32_t>& indices, size_t cacheSize = 32);
    static float CalculateATVR(const std::vector<uint32_t>& indices, size_t vertexCount);
};
```

## Data Models

### Loading Configuration

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
    ValidateDataStructure = 1 << 13,
    PreTransformVertices = 1 << 14,
    FixInfacingNormals = 1 << 15
};

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

enum class CoordinateSystem {
    RightHanded,
    LeftHanded
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

    // Additional texture coordinates
    Math::Vec2 texCoords2;
    Math::Vec2 texCoords3;

    bool operator==(const Vertex& other) const;
    bool IsNearlyEqual(const Vertex& other, float epsilon = 0.0001f) const;
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

    void AddAttribute(VertexAttribute type, uint32_t size, GLenum dataType, bool normalized = false);
    uint32_t GetAttributeOffset(VertexAttribute type) const;
    bool HasAttribute(VertexAttribute type) const;
};
```

### Statistics and Analysis

```cpp
struct ModelStats {
    uint32_t nodeCount = 0;
    uint32_t meshCount = 0;
    uint32_t materialCount = 0;
    uint32_t textureCount = 0;
    uint32_t animationCount = 0;
    uint32_t totalVertices = 0;
    uint32_t totalTriangles = 0;
    size_t totalMemoryUsage = 0;
    float loadingTimeMs = 0.0f;
    std::string formatUsed;
};

struct MeshAnalysis {
    uint32_t vertexCount;
    uint32_t triangleCount;
    uint32_t duplicateVertices;
    uint32_t degenerateTriangles;
    float averageTriangleArea;
    float minTriangleArea;
    float maxTriangleArea;
    BoundingBox bounds;
    bool hasNormals;
    bool hasTangents;
    bool hasTextureCoords;
    bool hasColors;
    bool hasBoneWeights;
    float cacheEfficiency;  // ACMR score
};
```

## Error Handling

### Model Loading Exceptions

```cpp
class ModelLoadingException : public std::runtime_error {
public:
    enum class Type {
        FileNotFound,
        UnsupportedFormat,
        CorruptedFile,
        OutOfMemory,
        InvalidData,
        ImporterError
    };

    ModelLoadingException(Type type, const std::string& message, const std::string& filepath = "");

    Type GetType() const { return m_type; }
    const std::string& GetFilePath() const { return m_filepath; }

private:
    Type m_type;
    std::string m_filepath;
};

class ModelValidationError : public std::runtime_error {
public:
    ModelValidationError(const std::string& message, const std::string& component = "");

    const std::string& GetComponent() const { return m_component; }

private:
    std::string m_component;
};
```

## Testing Strategy

### Unit Testing

```cpp
// Test basic model loading
bool TestModelLoading() {
    TestOutput::PrintTestStart("model loading");

    ModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test format support
    EXPECT_TRUE(loader.IsFormatSupported("obj"));
    EXPECT_TRUE(loader.IsFormatSupported("gltf"));
    EXPECT_TRUE(loader.IsFormatSupported("fbx"));

    auto formats = loader.GetSupportedFormats();
    EXPECT_TRUE(formats.size() > 0);

    TestOutput::PrintTestPass("model loading");
    return true;
}

// Test mesh optimization
bool TestMeshOptimization() {
    TestOutput::PrintTestStart("mesh optimization");

    // Create test mesh with duplicate vertices
    Mesh testMesh("test_mesh");
    std::vector<Vertex> vertices = CreateTestVertices();
    std::vector<uint32_t> indices = CreateTestIndices();

    testMesh.SetVertices(vertices);
    testMesh.SetIndices(indices);

    uint32_t originalVertexCount = testMesh.GetVertexCount();

    // Optimize mesh
    testMesh.RemoveDuplicateVertices();
    testMesh.OptimizeVertexCache();

    EXPECT_TRUE(testMesh.GetVertexCount() <= originalVertexCount);
    EXPECT_TRUE(testMesh.Validate());

    TestOutput::PrintTestPass("mesh optimization");
    return true;
}

// Test scene graph hierarchy
bool TestSceneGraphHierarchy() {
    TestOutput::PrintTestStart("scene graph hierarchy");

    auto rootNode = std::make_shared<ModelNode>("root");
    auto childNode1 = std::make_shared<ModelNode>("child1");
    auto childNode2 = std::make_shared<ModelNode>("child2");
    auto grandChild = std::make_shared<ModelNode>("grandchild");

    rootNode->AddChild(childNode1);
    rootNode->AddChild(childNode2);
    childNode1->AddChild(grandChild);

    EXPECT_EQUAL(rootNode->GetChildren().size(), static_cast<size_t>(2));
    EXPECT_EQUAL(childNode1->GetChildren().size(), static_cast<size_t>(1));
    EXPECT_NOT_NULL(grandChild->GetParent());

    // Test traversal
    int nodeCount = 0;
    rootNode->Traverse([&](std::shared_ptr<ModelNode> node) {
        nodeCount++;
    });
    EXPECT_EQUAL(nodeCount, 4);  // root + 2 children + 1 grandchild

    TestOutput::PrintTestPass("scene graph hierarchy");
    return true;
}
```

### Integration Testing

```cpp
// Test model loading with materials
bool TestModelLoadingWithMaterials() {
    TestOutput::PrintTestStart("model loading with materials");

    if (!std::filesystem::exists("test_assets/test_model.obj")) {
        TestOutput::PrintInfo("Skipping test - test model not found");
        TestOutput::PrintTestPass("model loading with materials");
        return true;
    }

    ModelLoader loader;
    loader.Initialize();

    auto model = loader.LoadModel("test_assets/test_model.obj");
    EXPECT_NOT_NULL(model);

    auto materials = model->GetMaterials();
    EXPECT_TRUE(materials.size() > 0);

    auto meshes = model->GetMeshes();
    EXPECT_TRUE(meshes.size() > 0);

    // Verify mesh-material associations
    for (auto& mesh : meshes) {
        EXPECT_NOT_NULL(mesh->GetMaterial());
    }

    TestOutput::PrintTestPass("model loading with materials");
    return true;
}

// Test asynchronous loading
bool TestAsynchronousLoading() {
    TestOutput::PrintTestStart("asynchronous loading");

    ModelLoader loader;
    loader.Initialize();

    bool progressCalled = false;
    loader.SetProgressCallback([&](float progress) {
        progressCalled = true;
        EXPECT_IN_RANGE(progress, 0.0f, 1.0f);
    });

    auto future = loader.LoadModelAsync("test_assets/test_model.obj");

    // Wait for completion with timeout
    auto status = future.wait_for(std::chrono::seconds(10));
    EXPECT_TRUE(status == std::future_status::ready);

    if (status == std::future_status::ready) {
        auto model = future.get();
        EXPECT_NOT_NULL(model);
    }

    TestOutput::PrintTestPass("asynchronous loading");
    return true;
}
```

## Implementation Phases

### Phase 1: Core Infrastructure

- ModelLoader class with Assimp integration
- Basic Model and ModelNode classes
- Enhanced Mesh class with optimization
- Material import system foundation

### Phase 2: Format Support

- GLTF 2.0 loader implementation
- FBX loader with material support
- OBJ/MTL loader enhancement
- DAE (Collada) basic support

### Phase 3: Optimization and Processing

- Mesh optimization algorithms
- LOD generation system
- Bounding volume calculation
- Vertex cache optimization

### Phase 4: Asynchronous Loading

- Thread pool implementation
- Progress tracking system
- Concurrent loading support
- Load cancellation and management

### Phase 5: Animation Integration

- Skeleton import from models
- Animation data processing
- Bone weight validation
- Animation timeline support

### Phase 6: Advanced Features

- Model caching system
- Hot-reloading support
- Performance profiling
- Comprehensive error handling

This design provides a robust foundation for 3D model loading in Game Engine Kiro, supporting modern asset pipelines while maintaining performance and ease of use.
