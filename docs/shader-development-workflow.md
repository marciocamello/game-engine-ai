# Shader Development Workflow and Best Practices

## Overview

This guide provides comprehensive information on developing shaders for Game Engine Kiro, including workflow recommendations, best practices, debugging techniques, and optimization strategies. It covers the complete development cycle from initial shader creation to production deployment.

## Table of Contents

1. [Development Environment Setup](#development-environment-setup)
2. [Shader Development Workflow](#shader-development-workflow)
3. [Hot-Reload Development](#hot-reload-development)
4. [Debugging and Profiling](#debugging-and-profiling)
5. [Performance Optimization](#performance-optimization)
6. [Cross-Platform Considerations](#cross-platform-considerations)
7. [Testing and Validation](#testing-and-validation)
8. [Best Practices](#best-practices)

## Development Environment Setup

### Project Structure

Organize your shader files in a clear, maintainable structure:

```
assets/shaders/
├── common/                 # Shared shader code
│   ├── lighting.glsl      # Lighting functions
│   ├── pbr.glsl          # PBR calculations
│   └── utils.glsl        # Utility functions
├── vertex/               # Vertex shaders
│   ├── basic.vert
│   ├── pbr.vert
│   └── skinned.vert
├── fragment/             # Fragment shaders
│   ├── basic.frag
│   ├── pbr.frag
│   └── unlit.frag
├── compute/              # Compute shaders
│   ├── particles.glsl
│   └── culling.glsl
└── post_process/         # Post-processing shaders
    ├── tone_mapping.frag
    ├── bloom.frag
    └── fxaa.frag
```

### IDE Configuration

#### Visual Studio Code Setup

Create `.vscode/settings.json` for shader development:

```json
{
  "files.associations": {
    "*.vert": "glsl",
    "*.frag": "glsl",
    "*.geom": "glsl",
    "*.comp": "glsl",
    "*.glsl": "glsl"
  },
  "glsl-canvas.refreshOnChange": true,
  "glsl-canvas.uniforms": {
    "u_time": "time",
    "u_resolution": "resolution"
  }
}
```

#### Recommended Extensions

- **Shader languages support for VS Code**: GLSL syntax highlighting
- **GLSL Canvas**: Live shader preview
- **GLSL Lint**: Shader validation and error checking

### Shader Validation Tools

Set up external validation tools for comprehensive shader checking:

```cpp
// Shader validation utility
class ShaderValidator {
public:
    struct ValidationResult {
        bool isValid = false;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        std::vector<std::string> optimizations;
    };

    static ValidationResult ValidateShader(const std::string& source,
                                          Shader::Type type) {
        ValidationResult result;

        // Basic syntax validation
        if (!ValidateSyntax(source)) {
            result.errors.push_back("Syntax validation failed");
            return result;
        }

        // Performance analysis
        auto perfWarnings = AnalyzePerformance(source);
        result.warnings.insert(result.warnings.end(),
                              perfWarnings.begin(), perfWarnings.end());

        // Optimization suggestions
        auto optimizations = SuggestOptimizations(source);
        result.optimizations.insert(result.optimizations.end(),
                                   optimizations.begin(), optimizations.end());

        result.isValid = result.errors.empty();
        return result;
    }

private:
    static bool ValidateSyntax(const std::string& source);
    static std::vector<std::string> AnalyzePerformance(const std::string& source);
    static std::vector<std::string> SuggestOptimizations(const std::string& source);
};
```

## Shader Development Workflow

### 1. Planning and Design

Before writing shader code, plan your shader's functionality:

```cpp
// Shader specification document
/*
Shader Name: Advanced PBR Shader
Purpose: Physically based rendering with multiple light support
Features:
- Cook-Torrance BRDF
- Normal mapping
- Multiple point lights (up to 8)
- Directional light support
- HDR output
- Gamma correction

Uniforms:
- Material properties (albedo, metallic, roughness, ao)
- Texture samplers (albedo, normal, metallic, roughness, ao)
- Lighting data (light positions, colors, intensities)
- Camera data (view position, matrices)

Performance Target:
- 60 FPS at 1080p on mid-range hardware
- Maximum 2ms GPU time per frame
*/
```

### 2. Iterative Development

Use an iterative approach for shader development:

#### Phase 1: Basic Implementation

```glsl
// basic_pbr.frag - Initial implementation
#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 u_albedo;
uniform float u_metallic;
uniform float u_roughness;

void main() {
    // Simple Lambert shading for initial testing
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(normalize(Normal), lightDir), 0.0);

    vec3 color = u_albedo * NdotL;
    FragColor = vec4(color, 1.0);
}
```

#### Phase 2: Add Core PBR

```glsl
// pbr_core.frag - Add PBR calculations
#version 460 core

// ... inputs and uniforms ...

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265 * denom * denom;

    return num / denom;
}

// ... other PBR functions ...

void main() {
    // Implement Cook-Torrance BRDF
    // ... PBR calculations ...
}
```

#### Phase 3: Add Advanced Features

```glsl
// pbr_advanced.frag - Add textures, multiple lights, etc.
#version 460 core

// ... complete implementation with all features ...
```

### 3. Version Control Integration

Use meaningful commit messages for shader changes:

```bash
# Good commit messages
git commit -m "shader: Add normal mapping support to PBR shader"
git commit -m "shader: Optimize light loop in fragment shader"
git commit -m "shader: Fix metallic workflow F0 calculation"

# Include performance impact
git commit -m "shader: Reduce texture samples in PBR shader (-15% GPU time)"
```

## Hot-Reload Development

### Setting Up Hot-Reload

```cpp
// Development setup with hot-reload
class ShaderDevelopmentEnvironment {
private:
    ShaderManager m_shaderManager;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;

public:
    bool Initialize() {
        // Initialize shader manager with hot-reload
        if (!m_shaderManager.Initialize()) {
            return false;
        }

        m_shaderManager.EnableHotReload(true);
        m_shaderManager.SetHotReloadCallback([this](const std::string& shaderName) {
            OnShaderReloaded(shaderName);
        });

        return true;
    }

    void OnShaderReloaded(const std::string& shaderName) {
        LOG_INFO("Shader reloaded: " + shaderName);

        // Update all materials using this shader
        for (auto& [name, material] : m_materials) {
            if (material->GetShader() &&
                material->GetShader()->GetName() == shaderName) {

                // Reapply material properties
                material->ApplyToShader(material->GetShader());
                LOG_INFO("Updated material: " + name);
            }
        }

        // Trigger scene refresh
        RequestSceneRefresh();
    }

    void Update(float deltaTime) {
        m_shaderManager.Update(deltaTime);
    }
};
```

### Hot-Reload Best Practices

1. **Save Frequently**: Hot-reload triggers on file save
2. **Test Incrementally**: Make small changes and test immediately
3. **Use Fallbacks**: Ensure graceful fallback on compilation errors
4. **Monitor Performance**: Watch for performance regressions

```cpp
// Hot-reload with error handling
void SetupHotReloadWithErrorHandling() {
    shaderManager.SetHotReloadCallback([](const std::string& shaderName) {
        LOG_INFO("Attempting to reload shader: " + shaderName);
    });

    shaderManager.SetErrorCallback([](const ShaderCompilationError& error) {
        LOG_ERROR("Hot-reload failed: " + error.what());

        // Show error in development UI
        ShowDevelopmentError("Shader Compilation Error", error.what());

        // Keep using previous version
        LOG_INFO("Continuing with previous shader version");
    });
}
```

## Debugging and Profiling

### Shader Debugging Techniques

#### 1. Visual Debugging

```glsl
// Debug visualization techniques
void main() {
    // Debug normal vectors
    if (u_debugMode == 1) {
        FragColor = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);
        return;
    }

    // Debug UV coordinates
    if (u_debugMode == 2) {
        FragColor = vec4(TexCoord, 0.0, 1.0);
        return;
    }

    // Debug lighting
    if (u_debugMode == 3) {
        vec3 lightDir = normalize(u_lightPos - FragPos);
        float NdotL = max(dot(normalize(Normal), lightDir), 0.0);
        FragColor = vec4(vec3(NdotL), 1.0);
        return;
    }

    // Normal rendering
    // ... regular shader code ...
}
```

#### 2. Step-by-Step Validation

```glsl
// Validate each step of PBR calculation
void main() {
    vec3 N = normalize(Normal);
    vec3 V = normalize(u_viewPos - FragPos);
    vec3 L = normalize(u_lightPos - FragPos);
    vec3 H = normalize(V + L);

    // Debug individual components
    if (u_debugComponent == 0) {
        // Show distribution term
        float D = DistributionGGX(N, H, u_roughness);
        FragColor = vec4(vec3(D), 1.0);
        return;
    }

    if (u_debugComponent == 1) {
        // Show geometry term
        float G = GeometrySmith(N, V, L, u_roughness);
        FragColor = vec4(vec3(G), 1.0);
        return;
    }

    if (u_debugComponent == 2) {
        // Show fresnel term
        vec3 F0 = mix(vec3(0.04), u_albedo, u_metallic);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        FragColor = vec4(F, 1.0);
        return;
    }

    // ... continue with normal calculation
}
```

### Performance Profiling

#### GPU Timing

```cpp
// GPU timing for shader performance analysis
class ShaderProfiler {
private:
    std::unordered_map<std::string, GLuint> m_queries;
    std::unordered_map<std::string, float> m_timings;

public:
    void BeginTiming(const std::string& name) {
        if (m_queries.find(name) == m_queries.end()) {
            glGenQueries(1, &m_queries[name]);
        }

        glBeginQuery(GL_TIME_ELAPSED, m_queries[name]);
    }

    void EndTiming(const std::string& name) {
        glEndQuery(GL_TIME_ELAPSED);
    }

    void UpdateTimings() {
        for (auto& [name, query] : m_queries) {
            GLuint64 timeElapsed;
            glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);
            m_timings[name] = timeElapsed / 1000000.0f; // Convert to milliseconds
        }
    }

    float GetTiming(const std::string& name) const {
        auto it = m_timings.find(name);
        return (it != m_timings.end()) ? it->second : 0.0f;
    }

    void LogTimings() const {
        LOG_INFO("=== Shader Performance ===");
        for (const auto& [name, time] : m_timings) {
            LOG_INFO(name + ": " + std::to_string(time) + "ms");
        }
    }
};

// Usage
ShaderProfiler profiler;

// In render loop
profiler.BeginTiming("PBR_Shader");
pbrShader->Use();
RenderScene();
profiler.EndTiming("PBR_Shader");

profiler.UpdateTimings();
profiler.LogTimings();
```

#### Instruction Count Analysis

```cpp
// Analyze shader complexity
struct ShaderComplexity {
    int instructionCount = 0;
    int textureReads = 0;
    int mathOperations = 0;
    int branchingInstructions = 0;
};

ShaderComplexity AnalyzeShaderComplexity(const std::string& shaderSource) {
    ShaderComplexity complexity;

    // Count texture reads
    size_t pos = 0;
    while ((pos = shaderSource.find("texture(", pos)) != std::string::npos) {
        complexity.textureReads++;
        pos += 8;
    }

    // Count branching
    pos = 0;
    std::vector<std::string> branchKeywords = {"if", "for", "while"};
    for (const auto& keyword : branchKeywords) {
        pos = 0;
        while ((pos = shaderSource.find(keyword, pos)) != std::string::npos) {
            complexity.branchingInstructions++;
            pos += keyword.length();
        }
    }

    // Estimate total instruction count
    complexity.instructionCount = complexity.textureReads * 4 +
                                 complexity.mathOperations * 1 +
                                 complexity.branchingInstructions * 2;

    return complexity;
}
```

## Performance Optimization

### Optimization Strategies

#### 1. Reduce Texture Samples

```glsl
// Before: Multiple texture samples
vec3 albedo = texture(u_albedoMap, TexCoord).rgb;
float metallic = texture(u_metallicMap, TexCoord).r;
float roughness = texture(u_roughnessMap, TexCoord).r;
float ao = texture(u_aoMap, TexCoord).r;

// After: Pack textures into channels
vec4 materialData = texture(u_materialMap, TexCoord);
vec3 albedo = materialData.rgb;
float metallic = materialData.a;

vec2 roughnessAO = texture(u_roughnessAOMap, TexCoord).rg;
float roughness = roughnessAO.r;
float ao = roughnessAO.g;
```

#### 2. Optimize Mathematical Operations

```glsl
// Before: Expensive operations
float distance = length(lightPos - FragPos);
float attenuation = 1.0 / (distance * distance);

// After: Use squared distance directly
vec3 lightVector = lightPos - FragPos;
float distanceSquared = dot(lightVector, lightVector);
float attenuation = 1.0 / distanceSquared;
vec3 lightDir = lightVector * inversesqrt(distanceSquared);
```

#### 3. Early Exit Optimizations

```glsl
// Early exit for distant lights
void main() {
    vec3 lightVector = u_lightPos - FragPos;
    float distanceSquared = dot(lightVector, lightVector);

    // Early exit if light is too far
    if (distanceSquared > u_lightRange * u_lightRange) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Continue with expensive calculations
    // ...
}
```

#### 4. Loop Unrolling

```glsl
// Before: Dynamic loop
for (int i = 0; i < u_numLights; ++i) {
    // Light calculation
}

// After: Unrolled loop for known maximum
#define MAX_LIGHTS 8

vec3 totalLight = vec3(0.0);
if (u_numLights > 0) totalLight += calculateLight(0);
if (u_numLights > 1) totalLight += calculateLight(1);
if (u_numLights > 2) totalLight += calculateLight(2);
// ... continue for MAX_LIGHTS
```

### Shader Variants for Performance

```cpp
// Create performance-optimized variants
class ShaderVariantOptimizer {
public:
    static ShaderVariant CreateMobileVariant() {
        ShaderVariant variant;
        variant.name = "Mobile";
        variant.AddDefine("MAX_LIGHTS", "4");
        variant.AddDefine("USE_SIMPLIFIED_BRDF", "1");
        variant.AddDefine("DISABLE_NORMAL_MAPPING", "1");
        return variant;
    }

    static ShaderVariant CreateHighQualityVariant() {
        ShaderVariant variant;
        variant.name = "HighQuality";
        variant.AddDefine("MAX_LIGHTS", "16");
        variant.AddDefine("USE_ADVANCED_BRDF", "1");
        variant.AddDefine("ENABLE_PARALLAX_MAPPING", "1");
        variant.AddDefine("USE_AREA_LIGHTS", "1");
        return variant;
    }

    static ShaderVariant CreateVariantForHardware(const HardwareCapabilities& caps) {
        ShaderVariant variant;

        if (caps.maxTextureUnits >= 16) {
            variant.AddDefine("HIGH_TEXTURE_SUPPORT", "1");
        }

        if (caps.supportsComputeShaders) {
            variant.AddDefine("COMPUTE_CAPABLE", "1");
        }

        variant.AddDefine("MAX_LIGHTS", std::to_string(caps.maxLights));

        return variant;
    }
};
```

## Cross-Platform Considerations

### GLSL Version Compatibility

```glsl
// Use appropriate GLSL version for target platforms
#version 460 core  // Modern desktop
#version 330 core  // Older desktop/mobile
#version 300 es    // Mobile ES

// Conditional compilation for different versions
#if __VERSION__ >= 460
    #define MODERN_GLSL
#endif

#ifdef MODERN_GLSL
    // Use modern features
    layout(binding = 0) uniform sampler2D u_albedoMap;
#else
    // Fallback for older versions
    uniform sampler2D u_albedoMap;
#endif
```

### Precision Qualifiers

```glsl
// Mobile-specific precision qualifiers
#ifdef GL_ES
precision mediump float;
precision mediump int;
precision lowp sampler2D;
#endif

// Use appropriate precision for different variables
varying highp vec3 FragPos;    // Position needs high precision
varying mediump vec3 Normal;   // Normal can use medium precision
varying lowp vec2 TexCoord;    // UV coordinates can use low precision
```

### Hardware-Specific Optimizations

```cpp
// Detect and optimize for specific hardware
class HardwareOptimizer {
public:
    static ShaderVariant OptimizeForGPU(const std::string& gpuName) {
        ShaderVariant variant;

        if (gpuName.find("NVIDIA") != std::string::npos) {
            // NVIDIA-specific optimizations
            variant.AddDefine("NVIDIA_GPU", "1");
            variant.AddDefine("PREFER_MAD_INSTRUCTIONS", "1");
        } else if (gpuName.find("AMD") != std::string::npos) {
            // AMD-specific optimizations
            variant.AddDefine("AMD_GPU", "1");
            variant.AddDefine("OPTIMIZE_BRANCHING", "1");
        } else if (gpuName.find("Intel") != std::string::npos) {
            // Intel-specific optimizations
            variant.AddDefine("INTEL_GPU", "1");
            variant.AddDefine("REDUCE_TEXTURE_SAMPLES", "1");
        }

        return variant;
    }
};
```

## Testing and Validation

### Automated Shader Testing

```cpp
// Automated shader testing framework
class ShaderTestSuite {
private:
    struct TestCase {
        std::string name;
        std::string vertexShader;
        std::string fragmentShader;
        std::function<bool()> validator;
    };

    std::vector<TestCase> m_testCases;

public:
    void AddTest(const std::string& name,
                 const std::string& vertexPath,
                 const std::string& fragmentPath,
                 std::function<bool()> validator) {
        TestCase test;
        test.name = name;
        test.vertexShader = ReadFile(vertexPath);
        test.fragmentShader = ReadFile(fragmentPath);
        test.validator = validator;
        m_testCases.push_back(test);
    }

    bool RunAllTests() {
        bool allPassed = true;

        for (const auto& test : m_testCases) {
            LOG_INFO("Running test: " + test.name);

            // Compile shader
            auto shader = std::make_shared<Shader>();
            bool compiled = shader->LoadFromSource(test.vertexShader, test.fragmentShader);

            if (!compiled) {
                LOG_ERROR("Test failed - compilation error: " + test.name);
                LOG_ERROR("Compile log: " + shader->GetCompileLog());
                allPassed = false;
                continue;
            }

            // Run validation
            if (!test.validator()) {
                LOG_ERROR("Test failed - validation error: " + test.name);
                allPassed = false;
                continue;
            }

            LOG_INFO("Test passed: " + test.name);
        }

        return allPassed;
    }
};

// Usage
ShaderTestSuite testSuite;

testSuite.AddTest("PBR Basic",
                  "assets/shaders/pbr.vert",
                  "assets/shaders/pbr.frag",
                  []() {
                      // Validate PBR shader functionality
                      return ValidatePBRShader();
                  });

testSuite.AddTest("Compute Particles",
                  "",
                  "assets/shaders/particles.comp",
                  []() {
                      return ValidateParticleCompute();
                  });

bool allTestsPassed = testSuite.RunAllTests();
```

### Visual Regression Testing

```cpp
// Visual regression testing for shaders
class ShaderVisualTester {
private:
    struct ReferenceImage {
        std::string testName;
        std::vector<uint8_t> imageData;
        int width, height;
    };

    std::unordered_map<std::string, ReferenceImage> m_referenceImages;

public:
    void CaptureReference(const std::string& testName) {
        // Render test scene
        RenderTestScene();

        // Capture framebuffer
        ReferenceImage ref;
        ref.testName = testName;
        ref.width = GetFramebufferWidth();
        ref.height = GetFramebufferHeight();
        ref.imageData.resize(ref.width * ref.height * 4);

        glReadPixels(0, 0, ref.width, ref.height, GL_RGBA, GL_UNSIGNED_BYTE,
                     ref.imageData.data());

        m_referenceImages[testName] = ref;

        // Save reference image
        SaveImage("references/" + testName + ".png", ref);
    }

    bool CompareWithReference(const std::string& testName, float tolerance = 0.01f) {
        auto it = m_referenceImages.find(testName);
        if (it == m_referenceImages.end()) {
            LOG_ERROR("No reference image for test: " + testName);
            return false;
        }

        // Render current scene
        RenderTestScene();

        // Capture current image
        std::vector<uint8_t> currentImage(it->second.width * it->second.height * 4);
        glReadPixels(0, 0, it->second.width, it->second.height, GL_RGBA,
                     GL_UNSIGNED_BYTE, currentImage.data());

        // Compare images
        float difference = CalculateImageDifference(it->second.imageData, currentImage);

        if (difference > tolerance) {
            LOG_ERROR("Visual test failed: " + testName +
                     " (difference: " + std::to_string(difference) + ")");
            SaveImage("failures/" + testName + "_current.png",
                     {testName, currentImage, it->second.width, it->second.height});
            return false;
        }

        return true;
    }
};
```

## Best Practices

### Code Organization

1. **Use Include Files**: Share common code across shaders
2. **Consistent Naming**: Use clear, consistent naming conventions
3. **Document Uniforms**: Comment all uniform variables
4. **Version Control**: Track shader changes with meaningful commits

```glsl
// Good shader organization example
#version 460 core

// Include common functionality
#include "common/lighting.glsl"
#include "common/pbr.glsl"

// Input attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Uniforms - grouped by purpose
// Transform matrices
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat3 u_normalMatrix;

// Material properties
uniform vec3 u_albedo;          // Base color (0-1)
uniform float u_metallic;       // Metallic factor (0-1)
uniform float u_roughness;      // Surface roughness (0-1)
uniform float u_ao;             // Ambient occlusion (0-1)

// Lighting data
uniform vec3 u_lightPos;        // World space light position
uniform vec3 u_lightColor;      // Light color (HDR)
uniform float u_lightIntensity; // Light intensity multiplier

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    // Transform vertex to world space
    FragPos = vec3(u_model * vec4(aPos, 1.0));

    // Transform normal to world space
    Normal = u_normalMatrix * aNormal;

    // Pass through texture coordinates
    TexCoord = aTexCoord;

    // Transform to clip space
    gl_Position = u_projection * u_view * vec4(FragPos, 1.0);
}
```

### Performance Guidelines

1. **Minimize Texture Samples**: Pack data into texture channels
2. **Avoid Branching**: Use conditional moves instead of if statements
3. **Use Appropriate Precision**: Lower precision for mobile targets
4. **Profile Regularly**: Monitor performance impact of changes

### Error Handling

1. **Validate Inputs**: Check for invalid values
2. **Provide Fallbacks**: Handle missing textures gracefully
3. **Log Errors**: Comprehensive error logging for debugging
4. **Test Edge Cases**: Test with extreme values and edge conditions

### Documentation

1. **Comment Complex Code**: Explain non-obvious calculations
2. **Document Uniforms**: Describe purpose and valid ranges
3. **Include Examples**: Provide usage examples
4. **Version History**: Track changes and improvements

This comprehensive guide provides the foundation for effective shader development in Game Engine Kiro. Following these practices will result in maintainable, performant, and robust shaders that work across different platforms and hardware configurations.
