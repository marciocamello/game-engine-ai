# Advanced Shader System API Reference

## Overview

The Advanced Shader System in Game Engine Kiro provides comprehensive support for modern graphics programming including PBR materials, compute shaders, hot-reloading, post-processing effects, and shader variants. This document provides detailed API reference and usage examples.

## Table of Contents

1. [Shader Class](#shader-class)
2. [Material System](#material-system)
3. [ShaderManager](#shadermanager)
4. [Hot-Reload System](#hot-reload-system)
5. [Post-Processing Pipeline](#post-processing-pipeline)
6. [Compute Shaders](#compute-shaders)
7. [Shader Variants](#shader-variants)
8. [Usage Examples](#usage-examples)

## Shader Class

The `Shader` class is the core component for loading, compiling, and managing OpenGL shaders.

### Basic Usage

```cpp
#include "Graphics/Shader.h"

// Create and load a basic vertex/fragment shader
auto shader = std::make_shared<Shader>();
if (shader->LoadFromFiles("assets/shaders/basic.vert", "assets/shaders/basic.frag")) {
    shader->Use();
    shader->SetUniform("u_model", modelMatrix);
    shader->SetUniform("u_view", viewMatrix);
    shader->SetUniform("u_projection", projectionMatrix);
}
```

### Compute Shader Support

```cpp
// Create and compile a compute shader
auto computeShader = std::make_shared<Shader>();
if (computeShader->CompileFromFile("assets/shaders/particle_compute.glsl", Shader::Type::Compute)) {
    computeShader->Use();

    // Bind storage buffers
    computeShader->BindStorageBuffer("ParticleBuffer", particleBufferId, 0);
    computeShader->BindStorageBuffer("EmitterBuffer", emitterBufferId, 1);

    // Set uniforms
    computeShader->SetUniform("u_deltaTime", deltaTime);
    computeShader->SetUniform("u_time", currentTime);

    // Dispatch compute work
    computeShader->Dispatch(numParticles / 64, 1, 1);
    computeShader->MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
```

### Enhanced Uniform Management

```cpp
// Basic uniform setters
shader->SetUniform("u_albedo", Math::Vec3(0.8f, 0.2f, 0.2f));
shader->SetUniform("u_metallic", 0.0f);
shader->SetUniform("u_roughness", 0.5f);
shader->SetUniform("u_model", modelMatrix);

// Array uniforms
std::vector<Math::Vec3> lightPositions = {
    Math::Vec3(10.0f, 10.0f, 10.0f),
    Math::Vec3(-10.0f, 10.0f, 10.0f)
};
shader->SetUniformArray("u_lightPositions", lightPositions);

// Direct uniform setters (bypass state management)
shader->SetUniformDirect("u_time", currentTime);
```

### Texture Binding

```cpp
// Manual texture binding
shader->BindTexture("u_albedoMap", albedoTexture, 0);
shader->BindTexture("u_normalMap", normalTexture, 1);

// Automatic slot assignment
shader->BindTextureAuto("u_albedoMap", albedoTexture);
shader->BindTextureAuto("u_normalMap", normalTexture);

// Image texture binding for compute shaders
shader->BindImageTexture("u_outputImage", outputTextureId, 0, GL_WRITE_ONLY);
```

### State Management Optimization

```cpp
// Enable state optimization for better performance
shader->EnableStateOptimization(true);
shader->RegisterWithStateManager();

// Flush pending updates when needed
shader->FlushPendingUpdates();
```

### Error Handling

```cpp
// Set error callbacks
shader->SetErrorCallback([](const ShaderCompilationError& error) {
    LOG_ERROR("Shader compilation failed: " + error.what());
});

shader->SetWarningCallback([](const std::string& shader, const std::string& warning) {
    LOG_WARNING("Shader warning in " + shader + ": " + warning);
});

// Validate shader
if (!shader->ValidateShader()) {
    auto warnings = shader->GetValidationWarnings();
    for (const auto& warning : warnings) {
        LOG_WARNING("Validation: " + warning);
    }
}
```

## Material System

The Material system provides high-level abstraction for managing shader properties and textures.

### PBR Material Usage

```cpp
#include "Graphics/Material.h"

// Create PBR material from template
auto pbrMaterial = Material::CreateFromTemplate(Material::Type::PBR, "MyPBRMaterial");

// Set PBR properties
pbrMaterial->SetAlbedo(Math::Vec3(0.8f, 0.2f, 0.2f));
pbrMaterial->SetMetallic(0.0f);
pbrMaterial->SetRoughness(0.5f);
pbrMaterial->SetAO(1.0f);

// Set textures
auto albedoTexture = std::make_shared<Texture>();
albedoTexture->LoadFromFile("assets/textures/albedo.jpg");
pbrMaterial->SetTexture("u_albedoMap", albedoTexture);

// Apply material to shader
pbrMaterial->SetShader(shader);
pbrMaterial->Bind();
pbrMaterial->ApplyToShader(shader);
```

### Advanced Property System

```cpp
// Using the advanced property system
pbrMaterial->SetProperty("u_customFloat", MaterialProperty(2.5f));
pbrMaterial->SetProperty("u_customVec3", MaterialProperty(Math::Vec3(1.0f, 0.5f, 0.2f)));
pbrMaterial->SetProperty("u_customTexture", MaterialProperty(customTexture));

// Get properties
if (pbrMaterial->HasProperty("u_metallic")) {
    float metallic = pbrMaterial->GetProperty("u_metallic").AsFloat();
}
```

### Material Serialization

```cpp
// Save material to JSON file
pbrMaterial->SaveToFile("assets/materials/my_material.json");

// Load material from JSON file
auto loadedMaterial = std::make_shared<Material>();
if (loadedMaterial->LoadFromFile("assets/materials/my_material.json")) {
    // Material loaded successfully
}

// Manual serialization
nlohmann::json materialJson = pbrMaterial->Serialize();
std::string jsonString = materialJson.dump(4);

// Manual deserialization
nlohmann::json loadedJson = nlohmann::json::parse(jsonString);
pbrMaterial->Deserialize(loadedJson);
```

## ShaderManager

The ShaderManager provides centralized shader management with caching and hot-reload capabilities.

### Basic Usage

```cpp
#include "Graphics/ShaderManager.h"

// Initialize shader manager
ShaderManager shaderManager;
shaderManager.Initialize();

// Load shaders
ShaderDesc pbrDesc;
pbrDesc.name = "PBR";
pbrDesc.vertexPath = "assets/shaders/pbr.vert";
pbrDesc.fragmentPath = "assets/shaders/pbr.frag";
pbrDesc.enableHotReload = true;

auto pbrShader = shaderManager.LoadShader("PBR", pbrDesc);

// Get cached shader
auto cachedShader = shaderManager.GetShader("PBR");
```

### Hot-Reload System

```cpp
// Enable hot-reload
shaderManager.EnableHotReload(true);

// Set hot-reload callback
shaderManager.SetHotReloadCallback([](const std::string& shaderName) {
    LOG_INFO("Shader reloaded: " + shaderName);
    // Update materials using this shader
});

// Manual reload
shaderManager.ReloadShader("PBR");
shaderManager.ReloadAllShaders();

// Check for changes (called automatically in Update)
shaderManager.CheckForShaderChanges();
```

### Shader Variants

```cpp
// Create shader variant
ShaderVariant variant;
variant.AddDefine("USE_NORMAL_MAPPING", "1");
variant.AddDefine("MAX_LIGHTS", "8");
variant.AddFeature("PBR_METALLIC_WORKFLOW");

auto variantShader = shaderManager.CreateShaderVariant("PBR", variant);

// Variant selection based on runtime conditions
auto bestVariant = shaderManager.SelectBestVariant("PBR", renderContext);
```

### Performance and Debugging

```cpp
// Get shader statistics
ShaderStats stats = shaderManager.GetShaderStats();
LOG_INFO("Loaded shaders: " + std::to_string(stats.loadedShaders));
LOG_INFO("Compilation time: " + std::to_string(stats.totalCompilationTime) + "ms");

// Enable debug mode
shaderManager.SetDebugMode(true);

// Precompile shaders for faster startup
shaderManager.PrecompileShaders();

// Optimize shaders
shaderManager.OptimizeShaders();
```

## Hot-Reload System

The hot-reload system automatically detects shader file changes and recompiles them in real-time.

### Setup

```cpp
#include "Graphics/ShaderHotReloader.h"

ShaderHotReloader hotReloader;
hotReloader.Initialize();

// Watch shader directory
hotReloader.WatchShaderDirectory("assets/shaders/");

// Watch specific files
hotReloader.WatchShaderFile("assets/shaders/custom.frag");

// Set callbacks
hotReloader.SetReloadCallback([](const std::string& filepath) {
    LOG_INFO("Shader file changed: " + filepath);
});

hotReloader.SetErrorCallback([](const std::string& filepath, const std::string& error) {
    LOG_ERROR("Shader reload failed for " + filepath + ": " + error);
});
```

### Configuration

```cpp
// Enable/disable hot-reload
hotReloader.SetEnabled(true);

// Set check interval (in seconds)
hotReloader.SetCheckInterval(0.5f);

// Manual reload
hotReloader.ReloadShader("assets/shaders/pbr.frag");
hotReloader.ReloadAllShaders();
```

## Post-Processing Pipeline

The post-processing pipeline provides a flexible system for applying screen-space effects.

### Basic Setup

```cpp
#include "Graphics/PostProcessingPipeline.h"

PostProcessingPipeline pipeline;
pipeline.Initialize(1920, 1080);

// Enable built-in effects
pipeline.EnableToneMapping(true, ToneMappingType::ACES);
pipeline.EnableFXAA(true, 0.75f);
pipeline.EnableBloom(true, 1.0f, 0.5f);

// Set global parameters
pipeline.SetGlobalExposure(1.2f);
pipeline.SetGlobalGamma(2.2f);
```

### Custom Effects

```cpp
// Create custom post-processing effect
class CustomBlurEffect : public PostProcessEffect {
public:
    bool Initialize(int width, int height) override {
        // Initialize effect resources
        return true;
    }

    void Process(uint32_t inputTexture, uint32_t outputTexture) override {
        // Apply blur effect
    }

    const std::string& GetName() const override {
        static std::string name = "CustomBlur";
        return name;
    }
};

// Add custom effect to pipeline
auto blurEffect = std::make_shared<CustomBlurEffect>();
pipeline.AddEffect(blurEffect);

// Configure effect order
pipeline.SetEffectOrder({"ToneMapping", "CustomBlur", "FXAA"});
```

### Processing

```cpp
// Process frame
pipeline.Process(sceneTexture, outputTexture);

// Process to screen
pipeline.ProcessToScreen(sceneTexture);

// Get performance statistics
PostProcessStats stats = pipeline.GetStats();
LOG_INFO("Post-processing time: " + std::to_string(stats.processingTimeMs) + "ms");
```

## Compute Shaders

Compute shaders enable GPU-based parallel computing for advanced effects and simulations.

### Particle System Example

```cpp
// Create compute shader for particle simulation
auto particleCompute = std::make_shared<Shader>();
particleCompute->CompileFromFile("assets/shaders/particle_compute.glsl", Shader::Type::Compute);

// Create storage buffers
uint32_t particleBuffer, emitterBuffer;
glGenBuffers(1, &particleBuffer);
glGenBuffers(1, &emitterBuffer);

// Initialize particle data
std::vector<Particle> particles(1000);
glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle),
             particles.data(), GL_DYNAMIC_DRAW);

// Bind buffers to compute shader
particleCompute->Use();
particleCompute->BindStorageBuffer("ParticleBuffer", particleBuffer, 0);
particleCompute->BindStorageBuffer("EmitterBuffer", emitterBuffer, 1);

// Set compute uniforms
particleCompute->SetUniform("u_deltaTime", deltaTime);
particleCompute->SetUniform("u_gravity", Math::Vec3(0.0f, -9.81f, 0.0f));
particleCompute->SetUniform("u_maxParticles", static_cast<int>(particles.size()));

// Dispatch compute work
uint32_t workGroups = (particles.size() + 63) / 64; // Round up to nearest 64
particleCompute->Dispatch(workGroups, 1, 1);

// Wait for completion
particleCompute->MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
```

### Image Processing Example

```cpp
// Create compute shader for image processing
auto imageProcess = std::make_shared<Shader>();
imageProcess->CompileFromFile("assets/shaders/image_process.glsl", Shader::Type::Compute);

// Bind input and output textures
imageProcess->Use();
imageProcess->BindImageTexture("u_inputImage", inputTextureId, 0, GL_READ_ONLY);
imageProcess->BindImageTexture("u_outputImage", outputTextureId, 1, GL_WRITE_ONLY);

// Set processing parameters
imageProcess->SetUniform("u_brightness", 1.2f);
imageProcess->SetUniform("u_contrast", 1.1f);

// Dispatch work groups based on image size
uint32_t groupsX = (imageWidth + 15) / 16;
uint32_t groupsY = (imageHeight + 15) / 16;
imageProcess->Dispatch(groupsX, groupsY, 1);

// Ensure completion before using result
imageProcess->MemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
```

## Shader Variants

Shader variants allow conditional compilation for different hardware capabilities and rendering paths.

### Creating Variants

```cpp
#include "Graphics/ShaderVariant.h"

// Create base variant
ShaderVariant baseVariant;
baseVariant.name = "PBR_Base";

// Create high-quality variant
ShaderVariant highQualityVariant;
highQualityVariant.name = "PBR_HighQuality";
highQualityVariant.AddDefine("USE_NORMAL_MAPPING", "1");
highQualityVariant.AddDefine("USE_PARALLAX_MAPPING", "1");
highQualityVariant.AddDefine("MAX_LIGHTS", "16");
highQualityVariant.AddFeature("HIGH_QUALITY_LIGHTING");

// Create mobile variant
ShaderVariant mobileVariant;
mobileVariant.name = "PBR_Mobile";
mobileVariant.AddDefine("MAX_LIGHTS", "4");
mobileVariant.AddDefine("USE_SIMPLIFIED_BRDF", "1");
mobileVariant.AddFeature("MOBILE_OPTIMIZED");
```

### Variant Management

```cpp
#include "Graphics/ShaderVariantManager.h"

ShaderVariantManager variantManager;

// Create variants
auto highQualityShader = variantManager.CreateVariant("PBR", highQualityVariant);
auto mobileShader = variantManager.CreateVariant("PBR", mobileVariant);

// Automatic variant selection
variantManager.SetVariantSelectionCallback([](const RenderContext& context) {
    ShaderVariant variant;

    if (context.hardwareCapabilities.supportsComputeShaders) {
        variant.AddFeature("COMPUTE_CAPABLE");
    }

    if (context.qualitySettings.enableNormalMapping) {
        variant.AddDefine("USE_NORMAL_MAPPING", "1");
    }

    variant.AddDefine("MAX_LIGHTS", std::to_string(context.maxLights));

    return variant;
});

// Get best variant for current context
auto bestShader = variantManager.SelectBestVariant("PBR", currentContext);
```

## Usage Examples

### Complete PBR Material Setup

```cpp
// Create and configure a complete PBR material
void SetupPBRMaterial() {
    // Load PBR shader
    auto pbrShader = std::make_shared<Shader>();
    pbrShader->LoadFromFiles("assets/shaders/pbr_showcase.vert",
                            "assets/shaders/pbr_showcase.frag");

    // Create PBR material
    auto material = Material::CreateFromTemplate(Material::Type::PBR, "MetalMaterial");
    material->SetShader(pbrShader);

    // Set material properties
    material->SetAlbedo(Math::Vec3(0.7f, 0.7f, 0.7f)); // Silver
    material->SetMetallic(1.0f);
    material->SetRoughness(0.2f);
    material->SetAO(1.0f);

    // Load and set textures
    auto albedoTex = std::make_shared<Texture>();
    albedoTex->LoadFromFile("assets/textures/metal_albedo.jpg");
    material->SetTexture("u_albedoMap", albedoTex);

    auto normalTex = std::make_shared<Texture>();
    normalTex->LoadFromFile("assets/textures/metal_normal.jpg");
    material->SetTexture("u_normalMap", normalTex);

    auto roughnessTex = std::make_shared<Texture>();
    roughnessTex->LoadFromFile("assets/textures/metal_roughness.jpg");
    material->SetTexture("u_roughnessMap", roughnessTex);

    // Apply material
    material->Bind();

    // Render object
    RenderMesh(mesh);

    material->Unbind();
}
```

### Hot-Reload Development Workflow

```cpp
// Setup development environment with hot-reload
void SetupDevelopmentEnvironment() {
    // Initialize shader manager with hot-reload
    ShaderManager shaderManager;
    shaderManager.Initialize();
    shaderManager.EnableHotReload(true);

    // Set up hot-reload callback
    shaderManager.SetHotReloadCallback([&](const std::string& shaderName) {
        LOG_INFO("Shader reloaded: " + shaderName);

        // Update all materials using this shader
        for (auto& material : materials) {
            if (material->GetShader() && material->GetShader()->GetName() == shaderName) {
                // Reapply material properties to updated shader
                material->ApplyToShader(material->GetShader());
            }
        }
    });

    // Load shaders with hot-reload enabled
    ShaderDesc desc;
    desc.name = "PBR";
    desc.vertexPath = "assets/shaders/pbr_showcase.vert";
    desc.fragmentPath = "assets/shaders/pbr_showcase.frag";
    desc.enableHotReload = true;

    auto shader = shaderManager.LoadShader("PBR", desc);

    // In main loop
    while (running) {
        shaderManager.Update(deltaTime); // Checks for file changes

        // Render scene
        RenderScene();
    }
}
```

### Post-Processing Pipeline Setup

```cpp
// Complete post-processing pipeline setup
void SetupPostProcessing() {
    PostProcessingPipeline pipeline;
    pipeline.Initialize(screenWidth, screenHeight);

    // Configure tone mapping
    pipeline.EnableToneMapping(true, ToneMappingType::ACES);
    pipeline.SetGlobalExposure(1.0f);
    pipeline.SetGlobalGamma(2.2f);

    // Configure anti-aliasing
    pipeline.EnableFXAA(true, 0.75f);

    // Configure bloom
    pipeline.EnableBloom(true, 1.0f, 0.5f); // threshold, intensity

    // Set quality level
    pipeline.SetQualityLevel(QualityLevel::High);

    // In render loop
    while (rendering) {
        // Render scene to HDR framebuffer
        RenderSceneToHDR(hdrFramebuffer);

        // Apply post-processing
        pipeline.Process(hdrFramebuffer.colorTexture, finalFramebuffer.colorTexture);

        // Present final result
        PresentFramebuffer(finalFramebuffer);
    }
}
```

## Best Practices

### Performance Optimization

1. **Use State Management**: Enable shader state optimization for better performance
2. **Batch Uniform Updates**: Group uniform updates to minimize OpenGL calls
3. **Cache Shaders**: Use ShaderManager to avoid redundant compilation
4. **Optimize Variants**: Create variants for different quality levels and hardware

### Development Workflow

1. **Enable Hot-Reload**: Use hot-reload during development for faster iteration
2. **Validate Shaders**: Always validate shaders and handle compilation errors
3. **Use Templates**: Use material templates for consistent material setup
4. **Profile Performance**: Monitor shader compilation and rendering performance

### Error Handling

1. **Set Callbacks**: Always set error and warning callbacks for debugging
2. **Fallback Shaders**: Provide fallback shaders for unsupported features
3. **Graceful Degradation**: Handle missing textures and shader compilation failures
4. **Log Everything**: Use comprehensive logging for debugging shader issues

This API reference provides comprehensive coverage of the Advanced Shader System capabilities. For more specific examples and tutorials, see the shader system demonstration examples in the `examples/` directory.
