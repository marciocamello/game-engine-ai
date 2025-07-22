# Advanced Shader System

Game Engine Kiro v1.1 introduces a comprehensive advanced shader system that provides powerful rendering capabilities, hot-reloading, shader compilation optimization, and support for modern graphics techniques.

## 🎯 Overview

The Advanced Shader System builds upon the basic shader foundation in v1.0, adding sophisticated features for modern game development including PBR (Physically Based Rendering), post-processing effects, compute shaders, and a flexible material system.

### Key Features

- **Hot-Reloading**: Real-time shader editing and recompilation
- **PBR Materials**: Industry-standard physically based rendering
- **Compute Shaders**: GPU compute for advanced effects
- **Shader Variants**: Conditional compilation and optimization
- **Material System**: High-level material abstraction
- **Post-Processing**: Screen-space effects pipeline

## 🏗️ Architecture Overview

### Shader System Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                    Material System                          │
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
│                     OpenGL Backend                          │
└─────────────────────────────────────────────────────────────┘
```

### Core Components

**ShaderManager (Enhanced)**

```cpp
class ShaderManager {
public:
    // Shader compilation and management
    std::shared_ptr<Shader> LoadShader(const std::string& name, const ShaderDesc& desc);
    std::shared_ptr<Shader> CreateShaderVariant(const std::string& baseName, const ShaderVariant& variant);

    // Hot-reloading system
    void EnableHotReload(bool enable);
    void CheckForShaderChanges();
    void ReloadShader(const std::string& name);

    // Shader compilation optimization
    void PrecompileShaders();
    void OptimizeShaders();

private:
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unique_ptr<ShaderCompiler> m_compiler;
    std::unique_ptr<ShaderHotReloader> m_hotReloader;
    std::unique_ptr<ShaderCache> m_cache;
};
```

**Shader (Enhanced)**

```cpp
class Shader {
public:
    enum class Type { Vertex, Fragment, Geometry, Compute, TessControl, TessEvaluation };

    // Compilation and linking
    bool CompileFromSource(const std::string& source, Type type);
    bool CompileFromFile(const std::string& filepath, Type type);
    bool LinkProgram();

    // Uniform management
    void SetUniform(const std::string& name, const Math::Mat4& value);
    void SetUniform(const std::string& name, const Math::Vec3& value);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, bool value);

    // Texture binding
    void BindTexture(const std::string& name, uint32_t textureId, uint32_t slot = 0);
    void BindTexture(const std::string& name, const Texture& texture, uint32_t slot = 0);

    // Shader introspection
    std::vector<UniformInfo> GetUniforms() const;
    std::vector<AttributeInfo> GetAttributes() const;

    // Performance and debugging
    bool IsValid() const;
    std::string GetCompileLog() const;
    ShaderStats GetPerformanceStats() const;

private:
    GLuint m_programId = 0;
    std::unordered_map<std::string, GLint> m_uniformLocations;
    std::unordered_map<std::string, UniformInfo> m_uniforms;
    std::string m_compileLog;
};
```

## 🎨 Material System

### Material Architecture

```cpp
class Material {
public:
    enum class Type { PBR, Unlit, Custom, PostProcess };

    // Material properties
    void SetAlbedo(const Math::Vec3& color);
    void SetAlbedoTexture(std::shared_ptr<Texture> texture);
    void SetMetallic(float metallic);
    void SetRoughness(float roughness);
    void SetNormalTexture(std::shared_ptr<Texture> texture);
    void SetEmission(const Math::Vec3& emission);

    // Rendering
    void Bind() const;
    void Unbind() const;
    void ApplyToShader(std::shared_ptr<Shader> shader) const;

    // Serialization
    void SaveToFile(const std::string& filepath) const;
    bool LoadFromFile(const std::string& filepath);

private:
    Type m_type = Type::PBR;
    std::shared_ptr<Shader> m_shader;
    MaterialProperties m_properties;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
```

### PBR Material Implementation

```cpp
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
    };

    // PBR-specific methods
    void SetProperties(const Properties& props);
    void SetAlbedoMap(std::shared_ptr<Texture> texture);
    void SetNormalMap(std::shared_ptr<Texture> texture);
    void SetMetallicRoughnessMap(std::shared_ptr<Texture> texture);
    void SetAOMap(std::shared_ptr<Texture> texture);
    void SetEmissionMap(std::shared_ptr<Texture> texture);

private:
    Properties m_properties;
};
```

## 🔥 Hot-Reloading System

### Real-Time Shader Development

```cpp
class ShaderHotReloader {
public:
    // File watching
    void WatchShaderDirectory(const std::string& directory);
    void WatchShaderFile(const std::string& filepath);

    // Hot-reload callbacks
    void SetReloadCallback(std::function<void(const std::string&)> callback);
    void Update();  // Call every frame to check for changes

    // Manual reload
    void ReloadShader(const std::string& name);
    void ReloadAllShaders();

private:
    std::unique_ptr<FileWatcher> m_fileWatcher;
    std::function<void(const std::string&)> m_reloadCallback;
    std::unordered_set<std::string> m_watchedFiles;
};
```

### Usage Example

```cpp
// Enable hot-reloading in development
#ifdef _DEBUG
shaderManager.EnableHotReload(true);
shaderManager.SetReloadCallback([](const std::string& shaderName) {
    LOG_INFO("Shader reloaded: " + shaderName);
});
#endif

// In main loop
void Update(float deltaTime) {
    #ifdef _DEBUG
    shaderManager.CheckForShaderChanges();
    #endif

    // Rest of update logic...
}
```

## ⚡ Shader Variants and Optimization

### Conditional Compilation

```cpp
struct ShaderVariant {
    std::unordered_map<std::string, std::string> defines;
    std::vector<std::string> features;
};

// Create shader variants
ShaderVariant pbrVariant;
pbrVariant.defines["USE_NORMAL_MAP"] = "1";
pbrVariant.defines["USE_METALLIC_ROUGHNESS_MAP"] = "1";
pbrVariant.features.push_back("LIGHTING");

auto pbrShader = shaderManager.CreateShaderVariant("pbr_base", pbrVariant);
```

### Shader Preprocessing

```glsl
// pbr_fragment.glsl
#version 460 core

// Conditional compilation
#ifdef USE_NORMAL_MAP
uniform sampler2D u_normalMap;
#endif

#ifdef USE_METALLIC_ROUGHNESS_MAP
uniform sampler2D u_metallicRoughnessMap;
#endif

// Feature toggles
#ifdef LIGHTING
vec3 calculateLighting(vec3 albedo, float metallic, float roughness, vec3 normal) {
    // PBR lighting calculations
    return computePBR(albedo, metallic, roughness, normal);
}
#else
vec3 calculateLighting(vec3 albedo, float metallic, float roughness, vec3 normal) {
    return albedo;  // Unlit
}
#endif

void main() {
    // Shader implementation using conditional features
}
```

## 🖥️ Compute Shader Support

### Compute Shader Integration

```cpp
class ComputeShader : public Shader {
public:
    // Compute-specific methods
    void Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ);
    void DispatchIndirect(uint32_t indirectBuffer);

    // Buffer binding
    void BindStorageBuffer(uint32_t binding, uint32_t bufferId);
    void BindImageTexture(uint32_t binding, uint32_t textureId, GLenum access);

    // Synchronization
    void MemoryBarrier(GLbitfield barriers);
    void WaitForCompletion();

private:
    Math::Vec3 m_workGroupSize;
};
```

### Compute Shader Examples

```cpp
// Particle system compute shader
auto particleShader = shaderManager.LoadComputeShader("particle_update");
particleShader->BindStorageBuffer(0, particleBuffer);
particleShader->BindStorageBuffer(1, velocityBuffer);
particleShader->SetUniform("u_deltaTime", deltaTime);
particleShader->Dispatch(particleCount / 64, 1, 1);
particleShader->MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

// Image processing compute shader
auto blurShader = shaderManager.LoadComputeShader("gaussian_blur");
blurShader->BindImageTexture(0, inputTexture, GL_READ_ONLY);
blurShader->BindImageTexture(1, outputTexture, GL_WRITE_ONLY);
blurShader->Dispatch(width / 16, height / 16, 1);
```

## 🌟 Post-Processing Pipeline

### Post-Processing Framework

```cpp
class PostProcessingPipeline {
public:
    // Effect management
    void AddEffect(std::shared_ptr<PostProcessEffect> effect);
    void RemoveEffect(const std::string& name);
    void SetEffectEnabled(const std::string& name, bool enabled);

    // Rendering
    void Process(uint32_t inputTexture, uint32_t outputTexture);
    void Resize(int width, int height);

    // Built-in effects
    void EnableToneMapping(bool enable);
    void EnableFXAA(bool enable);
    void EnableBloom(bool enable);
    void EnableSSAO(bool enable);

private:
    std::vector<std::shared_ptr<PostProcessEffect>> m_effects;
    std::unique_ptr<FramebufferManager> m_framebuffers;
};
```

### Built-in Post-Processing Effects

```cpp
// Tone mapping
class ToneMappingEffect : public PostProcessEffect {
public:
    enum class Type { Reinhard, ACES, Filmic };

    void SetType(Type type);
    void SetExposure(float exposure);
    void SetGamma(float gamma);
};

// FXAA Anti-aliasing
class FXAAEffect : public PostProcessEffect {
public:
    void SetQuality(float quality);  // 0.0 - 1.0
    void SetThreshold(float threshold);
};

// Bloom effect
class BloomEffect : public PostProcessEffect {
public:
    void SetThreshold(float threshold);
    void SetIntensity(float intensity);
    void SetRadius(float radius);
};
```

## 🔧 Shader Compilation and Caching

### Shader Compiler

```cpp
class ShaderCompiler {
public:
    struct CompileResult {
        bool success = false;
        std::string log;
        std::vector<uint8_t> spirv;  // For Vulkan future support
        ShaderReflection reflection;
    };

    // Compilation
    CompileResult CompileGLSL(const std::string& source, Shader::Type type);
    CompileResult CompileFromFile(const std::string& filepath);

    // Optimization
    void SetOptimizationLevel(int level);  // 0-3
    void EnableDebugInfo(bool enable);

    // Preprocessing
    std::string PreprocessShader(const std::string& source, const ShaderVariant& variant);

private:
    std::unique_ptr<GLSLCompiler> m_glslCompiler;
    int m_optimizationLevel = 2;
    bool m_debugInfo = false;
};
```

### Shader Caching System

```cpp
class ShaderCache {
public:
    // Cache management
    bool HasCachedShader(const std::string& name, const ShaderVariant& variant);
    std::shared_ptr<Shader> LoadFromCache(const std::string& name, const ShaderVariant& variant);
    void SaveToCache(const std::string& name, const ShaderVariant& variant, std::shared_ptr<Shader> shader);

    // Cache optimization
    void PrecompileShaders(const std::vector<std::string>& shaderNames);
    void ClearCache();
    size_t GetCacheSize() const;

private:
    std::string m_cacheDirectory = "cache/shaders/";
    std::unordered_map<std::string, CachedShader> m_cache;
};
```

## 📊 Performance and Debugging

### Shader Performance Monitoring

```cpp
struct ShaderStats {
    float compileTimeMs = 0.0f;
    float linkTimeMs = 0.0f;
    size_t memoryUsage = 0;
    uint32_t uniformCount = 0;
    uint32_t textureBindings = 0;
    bool isOptimized = false;
};

// Usage
ShaderStats stats = shader->GetPerformanceStats();
LOG_INFO("Shader compile time: " + std::to_string(stats.compileTimeMs) + "ms");
```

### Shader Debugging Tools

```cpp
class ShaderDebugger {
public:
    // Debug output
    void EnableDebugOutput(bool enable);
    void SetDebugCallback(std::function<void(const std::string&)> callback);

    // Shader validation
    bool ValidateShader(std::shared_ptr<Shader> shader);
    std::vector<ShaderIssue> AnalyzeShader(std::shared_ptr<Shader> shader);

    // Performance analysis
    ShaderPerformanceReport ProfileShader(std::shared_ptr<Shader> shader);

private:
    bool m_debugEnabled = false;
    std::function<void(const std::string&)> m_debugCallback;
};
```

## 🎮 Usage Examples

### Basic PBR Material Setup

```cpp
// Create PBR material
auto material = std::make_shared<PBRMaterial>();
material->SetAlbedo(Math::Vec3(0.8f, 0.2f, 0.2f));  // Red
material->SetMetallic(0.0f);
material->SetRoughness(0.3f);

// Load textures
auto albedoTexture = resourceManager->Load<Texture>("textures/metal_albedo.png");
auto normalTexture = resourceManager->Load<Texture>("textures/metal_normal.png");
auto metallicRoughnessTexture = resourceManager->Load<Texture>("textures/metal_metallic_roughness.png");

material->SetAlbedoMap(albedoTexture);
material->SetNormalMap(normalTexture);
material->SetMetallicRoughnessMap(metallicRoughnessTexture);

// Use in rendering
material->Bind();
renderer->DrawMesh(mesh, transform);
material->Unbind();
```

### Custom Shader Creation

```cpp
// Create custom shader with variants
ShaderDesc shaderDesc;
shaderDesc.vertexPath = "shaders/custom_vertex.glsl";
shaderDesc.fragmentPath = "shaders/custom_fragment.glsl";

ShaderVariant variant;
variant.defines["USE_VERTEX_COLORS"] = "1";
variant.defines["MAX_LIGHTS"] = "8";
variant.features.push_back("SHADOWS");

auto customShader = shaderManager.CreateShaderVariant("custom_shader", variant);

// Use custom shader
customShader->Use();
customShader->SetUniform("u_viewProjection", camera->GetViewProjectionMatrix());
customShader->SetUniform("u_model", transform);
customShader->SetUniform("u_lightCount", lightCount);
```

### Post-Processing Setup

```cpp
// Create post-processing pipeline
auto postProcess = std::make_unique<PostProcessingPipeline>();

// Add effects
postProcess->EnableToneMapping(true);
postProcess->EnableFXAA(true);
postProcess->EnableBloom(true);

// Configure bloom
auto bloomEffect = postProcess->GetEffect<BloomEffect>("bloom");
bloomEffect->SetThreshold(1.0f);
bloomEffect->SetIntensity(0.5f);
bloomEffect->SetRadius(2.0f);

// Apply post-processing
postProcess->Process(sceneTexture, finalTexture);
```

## 🔮 Future Enhancements

### Planned Features (v1.2+)

- **Vulkan Backend**: Modern graphics API support
- **Ray Tracing Shaders**: Hardware-accelerated ray tracing
- **Mesh Shaders**: Next-generation geometry pipeline
- **Variable Rate Shading**: Performance optimization
- **Shader Graph Editor**: Visual shader creation tool

### Advanced Techniques

- **Temporal Anti-Aliasing (TAA)**: High-quality anti-aliasing
- **Screen Space Reflections (SSR)**: Realistic reflections
- **Volumetric Lighting**: Atmospheric effects
- **Clustered Deferred Rendering**: Efficient lighting
- **GPU-Driven Rendering**: Reduce CPU overhead

## 📚 Best Practices

### Shader Development

1. **Use Hot-Reloading**: Enable for faster iteration
2. **Optimize Early**: Profile shaders during development
3. **Cache Aggressively**: Pre-compile shaders for release
4. **Minimize Variants**: Reduce compilation overhead
5. **Document Uniforms**: Clear parameter documentation

### Performance Optimization

1. **Batch State Changes**: Minimize shader switches
2. **Use Uniform Buffers**: Efficient uniform updates
3. **Optimize Texture Access**: Minimize texture bindings
4. **Profile Regularly**: Monitor GPU performance
5. **Use Compute Wisely**: Leverage GPU compute power

---

The Advanced Shader System in Game Engine Kiro v1.1 provides developers with professional-grade rendering capabilities, enabling the creation of visually stunning games with modern graphics techniques.

**Game Engine Kiro v1.1** - Unleashing the power of advanced graphics programming.
