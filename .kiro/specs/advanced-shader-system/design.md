# Design Document - Advanced Shader System

## Overview

This design document outlines the implementation of an advanced shader system for Game Engine Kiro v1.1. The system provides hot-reloadable shader development, PBR materials, compute shader support, shader variants, and a comprehensive post-processing pipeline for modern graphics programming.

## Architecture

### Advanced Shader System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   Material System                          │
├─────────────────────────────────────────────────────────────┤
│  PBR Material │ Unlit Material │ Custom Material │ Effects  │
├─────────────────────────────────────────────────────────────┤
│                   Shader Management                         │
├─────────────────────────────────────────────────────────────┤
│ Shader Compiler │ Hot Reload │ Variants │ Cache │ Optimizer │
├─────────────────────────────────────────────────────────────┤
│                   Shader Pipeline                           │
├─────────────────────────────────────────────────────────────┤
│  Vertex  │ Fragment │ Geometry │ Compute │ Tessellation     │
├─────────────────────────────────────────────────────────────┤
│                Post-Processing Pipeline                     │
├─────────────────────────────────────────────────────────────┤
│ Tone Mapping │ FXAA │ Bloom │ SSAO │ Custom Effects        │
├─────────────────────────────────────────────────────────────┤
│                     OpenGL Backend                          │
└─────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### 1. Enhanced ShaderManager

```cpp
class ShaderManager {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);

    // Shader loading and management
    std::shared_ptr<Shader> LoadShader(const std::string& name, const ShaderDesc& desc);
    std::shared_ptr<Shader> CreateShaderVariant(const std::string& baseName, const ShaderVariant& variant);
    void UnloadShader(const std::string& name);

    // Hot-reloading system
    void EnableHotReload(bool enable);
    void SetHotReloadCallback(std::function<void(const std::string&)> callback);
    void CheckForShaderChanges();
    void ReloadShader(const std::string& name);
    void ReloadAllShaders();

    // Shader compilation and caching
    void PrecompileShaders();
    void OptimizeShaders();
    void ClearShaderCache();

    // Performance and debugging
    ShaderStats GetShaderStats() const;
    void SetDebugMode(bool enabled);

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unique_ptr<ShaderCompiler> m_compiler;
    std::unique_ptr<ShaderHotReloader> m_hotReloader;
    std::unique_ptr<ShaderCache> m_cache;
    std::unique_ptr<ShaderOptimizer> m_optimizer;

    bool m_hotReloadEnabled = false;
    bool m_debugMode = false;
};
```

### 2. Enhanced Shader Class

```cpp
class Shader {
public:
    enum class Type { Vertex, Fragment, Geometry, Compute, TessControl, TessEvaluation };
    enum class State { Uncompiled, Compiling, Compiled, Linked, Error };

    // Lifecycle
    Shader(const std::string& name);
    ~Shader();

    // Compilation and linking
    bool CompileFromSource(const std::string& source, Type type);
    bool CompileFromFile(const std::string& filepath, Type type);
    bool LinkProgram();
    bool IsValid() const;

    // Shader usage
    void Use() const;
    void Unuse() const;

    // Uniform management
    void SetUniform(const std::string& name, const Math::Mat4& value);
    void SetUniform(const std::string& name, const Math::Vec3& value);
    void SetUniform(const std::string& name, const Math::Vec4& value);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, bool value);
    void SetUniformArray(const std::string& name, const std::vector<Math::Mat4>& values);

    // Texture binding
    void BindTexture(const std::string& name, uint32_t textureId, uint32_t slot = 0);
    void BindTexture(const std::string& name, const Texture& texture, uint32_t slot = 0);
    void BindImageTexture(const std::string& name, uint32_t textureId, uint32_t slot, GLenum access);

    // Storage buffer binding (for compute shaders)
    void BindStorageBuffer(const std::string& name, uint32_t bufferId, uint32_t binding);
    void BindUniformBuffer(const std::string& name, uint32_t bufferId, uint32_t binding);

    // Compute shader dispatch
    void Dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1);
    void DispatchIndirect(uint32_t indirectBuffer);

    // Synchronization
    void MemoryBarrier(GLbitfield barriers);
    void WaitForCompletion();

    // Shader introspection
    std::vector<UniformInfo> GetUniforms() const;
    std::vector<AttributeInfo> GetAttributes() const;
    std::vector<StorageBufferInfo> GetStorageBuffers() const;

    // Performance and debugging
    std::string GetCompileLog() const;
    std::string GetLinkLog() const;
    ShaderPerformanceStats GetPerformanceStats() const;
    State GetState() const;

    // Properties
    const std::string& GetName() const { return m_name; }
    GLuint GetProgramId() const { return m_programId; }

private:
    std::string m_name;
    GLuint m_programId = 0;
    State m_state = State::Uncompiled;

    std::unordered_map<std::string, GLint> m_uniformLocations;
    std::unordered_map<std::string, UniformInfo> m_uniforms;
    std::unordered_map<std::string, AttributeInfo> m_attributes;

    std::string m_compileLog;
    std::string m_linkLog;
    ShaderPerformanceStats m_performanceStats;

    // Helper methods
    GLint GetUniformLocation(const std::string& name);
    void CacheUniformLocations();
    void CacheShaderInfo();
};
```

### 3. Hot-Reloading System

```cpp
class ShaderHotReloader {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();
    void Update();

    // File watching
    void WatchShaderDirectory(const std::string& directory);
    void WatchShaderFile(const std::string& filepath);
    void UnwatchShaderFile(const std::string& filepath);

    // Callbacks
    void SetReloadCallback(std::function<void(const std::string&)> callback);
    void SetErrorCallback(std::function<void(const std::string&, const std::string&)> callback);

    // Manual reload
    void ReloadShader(const std::string& filepath);
    void ReloadAllShaders();

    // Configuration
    void SetEnabled(bool enabled);
    void SetCheckInterval(float intervalSeconds);

private:
    struct WatchedFile {
        std::string filepath;
        std::filesystem::file_time_type lastWriteTime;
        bool needsReload = false;
    };

    std::unordered_map<std::string, WatchedFile> m_watchedFiles;
    std::function<void(const std::string&)> m_reloadCallback;
    std::function<void(const std::string&, const std::string&)> m_errorCallback;

    bool m_enabled = false;
    float m_checkInterval = 0.5f;
    float m_timeSinceLastCheck = 0.0f;

    void CheckFileChanges();
    bool HasFileChanged(const WatchedFile& file);
};
```

### 4. PBR Material System

```cpp
class Material {
public:
    enum class Type { PBR, Unlit, Custom, PostProcess };

    // Lifecycle
    Material(Type type = Type::PBR);
    virtual ~Material() = default;

    // Material properties
    virtual void SetProperty(const std::string& name, const MaterialProperty& value);
    virtual MaterialProperty GetProperty(const std::string& name) const;
    virtual bool HasProperty(const std::string& name) const;

    // Texture management
    void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture(const std::string& name) const;
    void RemoveTexture(const std::string& name);

    // Shader binding
    void SetShader(std::shared_ptr<Shader> shader);
    std::shared_ptr<Shader> GetShader() const;

    // Rendering
    virtual void Bind() const;
    virtual void Unbind() const;
    virtual void ApplyToShader(std::shared_ptr<Shader> shader) const;

    // Serialization
    void SaveToFile(const std::string& filepath) const;
    bool LoadFromFile(const std::string& filepath);
    nlohmann::json Serialize() const;
    bool Deserialize(const nlohmann::json& json);

    // Properties
    Type GetType() const { return m_type; }
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

protected:
    Type m_type;
    std::string m_name;
    std::shared_ptr<Shader> m_shader;
    std::unordered_map<std::string, MaterialProperty> m_properties;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};

class PBRMaterial : public Material {
public:
    struct Properties {
        Math::Vec3 albedo = Math::Vec3(1.0f);
        float metallic = 0.0f;
        float roughness = 0.5f;
        float ao = 1.0f;
        Math::Vec3 emission = Math::Vec3(0.0f);
        float emissionStrength = 1.0f;
        float normalStrength = 1.0f;
        float alphaCutoff = 0.5f;
        bool useAlphaCutoff = false;
    };

    PBRMaterial();

    // PBR-specific methods
    void SetProperties(const Properties& props);
    Properties GetProperties() const;

    // Convenience methods
    void SetAlbedo(const Math::Vec3& albedo);
    void SetMetallic(float metallic);
    void SetRoughness(float roughness);
    void SetEmission(const Math::Vec3& emission);
    void SetNormalStrength(float strength);

    // Texture convenience methods
    void SetAlbedoMap(std::shared_ptr<Texture> texture);
    void SetNormalMap(std::shared_ptr<Texture> texture);
    void SetMetallicRoughnessMap(std::shared_ptr<Texture> texture);
    void SetAOMap(std::shared_ptr<Texture> texture);
    void SetEmissionMap(std::shared_ptr<Texture> texture);

    void ApplyToShader(std::shared_ptr<Shader> shader) const override;

private:
    Properties m_properties;
};
```

### 5. Shader Variants System

```cpp
struct ShaderVariant {
    std::unordered_map<std::string, std::string> defines;
    std::vector<std::string> features;
    std::string name;

    // Helper methods
    void AddDefine(const std::string& name, const std::string& value = "1");
    void AddFeature(const std::string& feature);
    std::string GenerateHash() const;
    bool IsCompatibleWith(const ShaderVariant& other) const;
};

class ShaderVariantManager {
public:
    // Variant creation and management
    std::shared_ptr<Shader> CreateVariant(const std::string& baseName, const ShaderVariant& variant);
    std::shared_ptr<Shader> GetVariant(const std::string& baseName, const ShaderVariant& variant);
    void RemoveVariant(const std::string& baseName, const ShaderVariant& variant);

    // Variant selection
    std::shared_ptr<Shader> SelectBestVariant(const std::string& baseName, const RenderContext& context);
    void SetVariantSelectionCallback(std::function<ShaderVariant(const RenderContext&)> callback);

    // Cache management
    void ClearVariantCache();
    void OptimizeVariantCache();
    size_t GetVariantCount() const;

private:
    struct VariantKey {
        std::string baseName;
        std::string variantHash;

        bool operator==(const VariantKey& other) const;
    };

    struct VariantKeyHash {
        size_t operator()(const VariantKey& key) const;
    };

    std::unordered_map<VariantKey, std::shared_ptr<Shader>, VariantKeyHash> m_variants;
    std::function<ShaderVariant(const RenderContext&)> m_selectionCallback;
};
```

### 6. Post-Processing Pipeline

```cpp
class PostProcessingPipeline {
public:
    // Lifecycle
    bool Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    // Effect management
    void AddEffect(std::shared_ptr<PostProcessEffect> effect);
    void RemoveEffect(const std::string& name);
    void SetEffectEnabled(const std::string& name, bool enabled);
    void SetEffectOrder(const std::vector<std::string>& order);

    // Processing
    void Process(uint32_t inputTexture, uint32_t outputTexture);
    void ProcessToScreen(uint32_t inputTexture);

    // Built-in effects
    void EnableToneMapping(bool enable, ToneMappingType type = ToneMappingType::ACES);
    void EnableFXAA(bool enable, float quality = 0.75f);
    void EnableBloom(bool enable, float threshold = 1.0f, float intensity = 0.5f);
    void EnableSSAO(bool enable, float radius = 0.5f, float intensity = 1.0f);

    // Configuration
    void SetGlobalExposure(float exposure);
    void SetGlobalGamma(float gamma);

    // Performance
    PostProcessStats GetStats() const;
    void SetQualityLevel(QualityLevel level);

private:
    std::vector<std::shared_ptr<PostProcessEffect>> m_effects;
    std::unique_ptr<FramebufferManager> m_framebuffers;

    int m_width = 0;
    int m_height = 0;
    float m_globalExposure = 1.0f;
    float m_globalGamma = 2.2f;

    PostProcessStats m_stats;
    QualityLevel m_qualityLevel = QualityLevel::High;
};

class PostProcessEffect {
public:
    virtual ~PostProcessEffect() = default;

    // Effect interface
    virtual bool Initialize(int width, int height) = 0;
    virtual void Shutdown() = 0;
    virtual void Resize(int width, int height) = 0;
    virtual void Process(uint32_t inputTexture, uint32_t outputTexture) = 0;

    // Properties
    virtual const std::string& GetName() const = 0;
    virtual void SetEnabled(bool enabled) { m_enabled = enabled; }
    virtual bool IsEnabled() const { return m_enabled; }

    // Parameters
    virtual void SetParameter(const std::string& name, float value) {}
    virtual void SetParameter(const std::string& name, const Math::Vec3& value) {}
    virtual void SetParameter(const std::string& name, const Math::Vec4& value) {}

protected:
    bool m_enabled = true;
};
```

### 7. Compute Shader Support

```cpp
class ComputeShader : public Shader {
public:
    ComputeShader(const std::string& name);

    // Compute-specific compilation
    bool CompileFromSource(const std::string& source);
    bool CompileFromFile(const std::string& filepath);

    // Work group information
    Math::Vec3 GetWorkGroupSize() const;
    Math::Vec3 GetMaxWorkGroupCount() const;
    uint32_t GetMaxWorkGroupInvocations() const;

    // Dispatch methods
    void Dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1);
    void DispatchIndirect(uint32_t indirectBuffer);

    // Resource binding
    void BindStorageBuffer(uint32_t binding, uint32_t bufferId);
    void BindImageTexture(uint32_t binding, uint32_t textureId, GLenum access);
    void BindAtomicCounterBuffer(uint32_t binding, uint32_t bufferId);

    // Synchronization
    void MemoryBarrier(GLbitfield barriers = GL_ALL_BARRIER_BITS);
    void WaitForCompletion();

    // Performance monitoring
    ComputePerformanceStats GetComputeStats() const;

private:
    Math::Vec3 m_workGroupSize;
    ComputePerformanceStats m_computeStats;

    void QueryWorkGroupInfo();
};
```

## Data Models

### Shader Description

```cpp
struct ShaderDesc {
    std::string name;
    std::string vertexPath;
    std::string fragmentPath;
    std::string geometryPath;
    std::string computePath;
    std::string tessControlPath;
    std::string tessEvaluationPath;

    ShaderVariant variant;
    bool enableHotReload = true;
    bool enableOptimization = true;
};
```

### Material Property System

```cpp
class MaterialProperty {
public:
    enum class Type { Float, Vec2, Vec3, Vec4, Int, Bool, Texture };

    MaterialProperty() = default;
    MaterialProperty(float value);
    MaterialProperty(const Math::Vec3& value);
    MaterialProperty(const Math::Vec4& value);
    MaterialProperty(int value);
    MaterialProperty(bool value);
    MaterialProperty(std::shared_ptr<Texture> texture);

    Type GetType() const;

    float AsFloat() const;
    Math::Vec3 AsVec3() const;
    Math::Vec4 AsVec4() const;
    int AsInt() const;
    bool AsBool() const;
    std::shared_ptr<Texture> AsTexture() const;

    nlohmann::json Serialize() const;
    bool Deserialize(const nlohmann::json& json);

private:
    Type m_type;
    std::variant<float, Math::Vec3, Math::Vec4, int, bool, std::shared_ptr<Texture>> m_value;
};
```

## Error Handling

### Shader Compilation Error Handling

```cpp
class ShaderCompilationError : public std::runtime_error {
public:
    ShaderCompilationError(const std::string& shaderName, const std::string& error, int line = -1);

    const std::string& GetShaderName() const { return m_shaderName; }
    int GetLineNumber() const { return m_lineNumber; }

private:
    std::string m_shaderName;
    int m_lineNumber;
};

class ShaderErrorHandler {
public:
    static void HandleCompilationError(const std::string& shaderName, const std::string& log);
    static void HandleLinkingError(const std::string& shaderName, const std::string& log);
    static void HandleRuntimeError(const std::string& shaderName, const std::string& error);

    static void SetErrorCallback(std::function<void(const ShaderCompilationError&)> callback);

private:
    static std::function<void(const ShaderCompilationError&)> s_errorCallback;
    static void ParseErrorLog(const std::string& log, std::vector<ShaderError>& errors);
};
```

## Testing Strategy

### Unit Testing

```cpp
// Test shader compilation and linking
bool TestShaderCompilation() {
    TestOutput::PrintTestStart("shader compilation");

    ShaderManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test basic vertex/fragment shader
    ShaderDesc desc;
    desc.name = "test_shader";
    desc.vertexPath = "shaders/test_vertex.glsl";
    desc.fragmentPath = "shaders/test_fragment.glsl";

    auto shader = manager.LoadShader("test_shader", desc);
    EXPECT_NOT_NULL(shader);
    EXPECT_TRUE(shader->IsValid());

    TestOutput::PrintTestPass("shader compilation");
    return true;
}

// Test PBR material system
bool TestPBRMaterial() {
    TestOutput::PrintTestStart("pbr material");

    PBRMaterial material;
    material.SetAlbedo(Math::Vec3(0.8f, 0.2f, 0.2f));
    material.SetMetallic(0.0f);
    material.SetRoughness(0.3f);

    auto properties = material.GetProperties();
    EXPECT_NEAR_VEC3(properties.albedo, Math::Vec3(0.8f, 0.2f, 0.2f));
    EXPECT_NEARLY_EQUAL(properties.metallic, 0.0f);
    EXPECT_NEARLY_EQUAL(properties.roughness, 0.3f);

    TestOutput::PrintTestPass("pbr material");
    return true;
}

// Test compute shader dispatch
bool TestComputeShader() {
    TestOutput::PrintTestStart("compute shader");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("compute shader");
        return true;
    }

    ComputeShader computeShader("test_compute");
    std::string computeSource = R"(
        #version 460 core
        layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
        layout(std430, binding = 0) restrict buffer DataBuffer {
            float data[];
        };
        void main() {
            uint index = gl_GlobalInvocationID.x;
            if (index < data.length()) {
                data[index] *= 2.0;
            }
        }
    )";

    EXPECT_TRUE(computeShader.CompileFromSource(computeSource));
    EXPECT_TRUE(computeShader.IsValid());

    TestOutput::PrintTestPass("compute shader");
    return true;
}
```

### Integration Testing

```cpp
// Test hot-reload system
bool TestShaderHotReload() {
    TestOutput::PrintTestStart("shader hot reload");

    ShaderManager manager;
    manager.EnableHotReload(true);

    bool reloadCalled = false;
    manager.SetHotReloadCallback([&](const std::string& name) {
        reloadCalled = true;
    });

    // Simulate file change
    manager.ReloadShader("test_shader");
    EXPECT_TRUE(reloadCalled);

    TestOutput::PrintTestPass("shader hot reload");
    return true;
}

// Test post-processing pipeline
bool TestPostProcessingPipeline() {
    TestOutput::PrintTestStart("post processing pipeline");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("post processing pipeline");
        return true;
    }

    PostProcessingPipeline pipeline;
    EXPECT_TRUE(pipeline.Initialize(1920, 1080));

    pipeline.EnableToneMapping(true, ToneMappingType::ACES);
    pipeline.EnableFXAA(true, 0.75f);

    // Test would require actual framebuffers and textures
    TestOutput::PrintTestPass("post processing pipeline");
    return true;
}
```

## Implementation Phases

### Phase 1: Core Shader Enhancement

- Enhanced Shader class with compute support
- Improved ShaderManager with caching
- Basic hot-reload system implementation
- Shader variant system foundation

### Phase 2: Material System

- PBR Material implementation
- Material property system
- Texture binding and management
- Material serialization/deserialization

### Phase 3: Hot-Reload and Development Tools

- File watching system
- Automatic recompilation
- Error handling and reporting
- Developer debugging tools

### Phase 4: Post-Processing Pipeline

- Post-processing framework
- Built-in effects (tone mapping, FXAA, bloom)
- Framebuffer management
- Effect chaining and ordering

### Phase 5: Compute Shader Integration

- Compute shader compilation
- Resource binding for compute
- Synchronization primitives
- Performance monitoring

### Phase 6: Optimization and Polish

- Shader caching optimization
- Performance profiling
- Memory usage optimization
- Comprehensive testing

This design provides a comprehensive foundation for modern shader programming in Game Engine Kiro, enabling developers to create sophisticated visual effects and materials while maintaining ease of use and performance.
