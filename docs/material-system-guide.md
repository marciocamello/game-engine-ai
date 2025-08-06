# Material System Usage Guide

## Overview

The Material System in Game Engine Kiro provides a high-level abstraction for managing shader properties, textures, and rendering states. It supports PBR (Physically Based Rendering) materials, custom materials, and advanced property management with serialization capabilities.

## Table of Contents

1. [Getting Started](#getting-started)
2. [PBR Materials](#pbr-materials)
3. [Material Properties](#material-properties)
4. [Texture Management](#texture-management)
5. [Material Templates](#material-templates)
6. [Serialization](#serialization)
7. [Advanced Usage](#advanced-usage)
8. [Best Practices](#best-practices)

## Getting Started

### Basic Material Creation

```cpp
#include "Graphics/Material.h"
#include "Graphics/Shader.h"

// Create a basic PBR material
auto material = std::make_shared<Material>("MyMaterial", Material::Type::PBR);

// Load and assign a shader
auto shader = std::make_shared<Shader>();
shader->LoadFromFiles("assets/shaders/pbr.vert", "assets/shaders/pbr.frag");
material->SetShader(shader);

// Set basic properties
material->SetAlbedo(Math::Vec3(0.8f, 0.2f, 0.2f)); // Red albedo
material->SetMetallic(0.0f);                        // Non-metallic
material->SetRoughness(0.5f);                       // Medium roughness
material->SetAO(1.0f);                              // Full ambient occlusion
```

### Using Materials in Rendering

```cpp
// Apply material before rendering
material->Bind();

// Render your geometry
RenderMesh(mesh);

// Unbind material after rendering
material->Unbind();
```

## PBR Materials

### Understanding PBR Properties

PBR (Physically Based Rendering) materials use a metallic workflow with the following key properties:

- **Albedo**: Base color of the material (diffuse color for non-metals, reflectance for metals)
- **Metallic**: Whether the material is metallic (0.0 = dielectric, 1.0 = metallic)
- **Roughness**: Surface roughness (0.0 = mirror-like, 1.0 = completely rough)
- **AO (Ambient Occlusion)**: Self-shadowing factor (0.0 = fully occluded, 1.0 = no occlusion)
- **Emission**: Self-illumination color and intensity
- **Normal**: Surface normal perturbation for detail

### Creating Different Material Types

#### Metallic Materials

```cpp
// Gold material
auto goldMaterial = Material::CreateFromTemplate(Material::Type::PBR, "Gold");
goldMaterial->SetAlbedo(Math::Vec3(1.0f, 0.86f, 0.57f)); // Gold color
goldMaterial->SetMetallic(1.0f);                          // Fully metallic
goldMaterial->SetRoughness(0.1f);                         // Very smooth
goldMaterial->SetAO(1.0f);

// Iron material
auto ironMaterial = Material::CreateFromTemplate(Material::Type::PBR, "Iron");
ironMaterial->SetAlbedo(Math::Vec3(0.56f, 0.57f, 0.58f)); // Iron color
ironMaterial->SetMetallic(1.0f);                           // Fully metallic
ironMaterial->SetRoughness(0.8f);                          // Rough surface
ironMaterial->SetAO(1.0f);
```

#### Dielectric Materials

```cpp
// Plastic material
auto plasticMaterial = Material::CreateFromTemplate(Material::Type::PBR, "Plastic");
plasticMaterial->SetAlbedo(Math::Vec3(0.8f, 0.2f, 0.2f)); // Red plastic
plasticMaterial->SetMetallic(0.0f);                        // Non-metallic
plasticMaterial->SetRoughness(0.3f);                       // Smooth plastic
plasticMaterial->SetAO(1.0f);

// Rubber material
auto rubberMaterial = Material::CreateFromTemplate(Material::Type::PBR, "Rubber");
rubberMaterial->SetAlbedo(Math::Vec3(0.1f, 0.1f, 0.1f)); // Dark rubber
rubberMaterial->SetMetallic(0.0f);                        // Non-metallic
rubberMaterial->SetRoughness(0.9f);                       // Very rough
rubberMaterial->SetAO(0.8f);                              // Slight occlusion
```

#### Emissive Materials

```cpp
// Emissive material for light sources
auto emissiveMaterial = Material::CreateFromTemplate(Material::Type::PBR, "Emissive");
emissiveMaterial->SetAlbedo(Math::Vec3(0.1f, 0.1f, 0.1f)); // Dark base
emissiveMaterial->SetMetallic(0.0f);
emissiveMaterial->SetRoughness(1.0f);

// Set emission properties using advanced property system
emissiveMaterial->SetProperty("u_emission", MaterialProperty(Math::Vec3(5.0f, 2.0f, 0.5f)));
emissiveMaterial->SetProperty("u_emissionStrength", MaterialProperty(3.0f));
```

## Material Properties

### Basic Property Management

```cpp
// Set properties using convenience methods
material->SetFloat("u_customFloat", 2.5f);
material->SetVec3("u_customColor", Math::Vec3(1.0f, 0.5f, 0.2f));
material->SetBool("u_enableFeature", true);

// Get properties
float roughness = material->GetFloat("u_roughness");
Math::Vec3 albedo = material->GetVec3("u_albedo");
bool hasFeature = material->GetBool("u_enableFeature");
```

### Advanced Property System

```cpp
// Using the advanced MaterialProperty system
material->SetProperty("u_complexProperty", MaterialProperty(Math::Vec4(1.0f, 2.0f, 3.0f, 4.0f)));
material->SetProperty("u_transformMatrix", MaterialProperty(transformMatrix));

// Check if property exists
if (material->HasProperty("u_customProperty")) {
    auto property = material->GetProperty("u_customProperty");

    switch (property.GetType()) {
        case MaterialProperty::Type::Float:
            float value = property.AsFloat();
            break;
        case MaterialProperty::Type::Vec3:
            Math::Vec3 vector = property.AsVec3();
            break;
        case MaterialProperty::Type::Texture:
            auto texture = property.AsTexture();
            break;
    }
}

// Remove properties
material->RemoveProperty("u_obsoleteProperty");
```

### Property Validation and Defaults

```cpp
// Set up material with validation
void SetupValidatedMaterial(std::shared_ptr<Material> material) {
    // Set properties with validation
    float roughness = std::clamp(userRoughness, 0.0f, 1.0f);
    material->SetRoughness(roughness);

    float metallic = std::clamp(userMetallic, 0.0f, 1.0f);
    material->SetMetallic(metallic);

    // Ensure albedo is in valid range
    Math::Vec3 albedo = glm::clamp(userAlbedo, Math::Vec3(0.0f), Math::Vec3(1.0f));
    material->SetAlbedo(albedo);

    // Set default values for optional properties
    if (!material->HasProperty("u_normalStrength")) {
        material->SetFloat("u_normalStrength", 1.0f);
    }
}
```

## Texture Management

### Loading and Assigning Textures

```cpp
#include "Graphics/Texture.h"

// Load textures
auto albedoTexture = std::make_shared<Texture>();
if (albedoTexture->LoadFromFile("assets/textures/brick_albedo.jpg")) {
    material->SetTexture("u_albedoMap", albedoTexture);
}

auto normalTexture = std::make_shared<Texture>();
if (normalTexture->LoadFromFile("assets/textures/brick_normal.jpg")) {
    material->SetTexture("u_normalMap", normalTexture);
}

auto roughnessTexture = std::make_shared<Texture>();
if (roughnessTexture->LoadFromFile("assets/textures/brick_roughness.jpg")) {
    material->SetTexture("u_roughnessMap", roughnessTexture);
}
```

### Texture Fallbacks

```cpp
// Set up material with texture fallbacks
void SetupMaterialWithFallbacks(std::shared_ptr<Material> material) {
    // Try to load albedo texture
    auto albedoTexture = std::make_shared<Texture>();
    if (albedoTexture->LoadFromFile("assets/textures/albedo.jpg")) {
        material->SetTexture("u_albedoMap", albedoTexture);
        material->SetBool("u_useAlbedoMap", true);
    } else {
        // Use solid color fallback
        material->SetAlbedo(Math::Vec3(0.8f, 0.8f, 0.8f));
        material->SetBool("u_useAlbedoMap", false);
        LOG_WARNING("Failed to load albedo texture, using solid color");
    }

    // Try to load normal map
    auto normalTexture = std::make_shared<Texture>();
    if (normalTexture->LoadFromFile("assets/textures/normal.jpg")) {
        material->SetTexture("u_normalMap", normalTexture);
        material->SetBool("u_useNormalMap", true);
    } else {
        // Disable normal mapping
        material->SetBool("u_useNormalMap", false);
        LOG_INFO("No normal map available, using flat normals");
    }
}
```

### Texture Management Best Practices

```cpp
// Efficient texture management
class MaterialManager {
private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;

public:
    std::shared_ptr<Texture> GetOrLoadTexture(const std::string& path) {
        auto it = m_textureCache.find(path);
        if (it != m_textureCache.end()) {
            return it->second; // Return cached texture
        }

        // Load new texture
        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromFile(path)) {
            m_textureCache[path] = texture;
            return texture;
        }

        return nullptr;
    }

    void CreateMaterialWithCachedTextures(std::shared_ptr<Material> material,
                                         const std::string& albedoPath,
                                         const std::string& normalPath) {
        auto albedoTex = GetOrLoadTexture(albedoPath);
        if (albedoTex) {
            material->SetTexture("u_albedoMap", albedoTex);
        }

        auto normalTex = GetOrLoadTexture(normalPath);
        if (normalTex) {
            material->SetTexture("u_normalMap", normalTex);
        }
    }
};
```

## Material Templates

### Using Built-in Templates

```cpp
// Create materials from templates
auto pbrMaterial = Material::CreateFromTemplate(Material::Type::PBR, "MyPBR");
auto unlitMaterial = Material::CreateFromTemplate(Material::Type::Unlit, "MyUnlit");
auto customMaterial = Material::CreateFromTemplate(Material::Type::Custom, "MyCustom");
```

### Creating Custom Templates

```cpp
// Custom material template class
class CustomMaterialTemplate {
public:
    static std::shared_ptr<Material> CreateMetalTemplate(const std::string& name) {
        auto material = Material::CreateFromTemplate(Material::Type::PBR, name);

        // Set metallic defaults
        material->SetMetallic(1.0f);
        material->SetRoughness(0.2f);
        material->SetAO(1.0f);

        // Set common metal properties
        material->SetProperty("u_normalStrength", MaterialProperty(1.0f));
        material->SetProperty("u_emissionStrength", MaterialProperty(0.0f));

        return material;
    }

    static std::shared_ptr<Material> CreateFabricTemplate(const std::string& name) {
        auto material = Material::CreateFromTemplate(Material::Type::PBR, name);

        // Set fabric defaults
        material->SetMetallic(0.0f);
        material->SetRoughness(0.8f);
        material->SetAO(0.9f);

        // Fabric-specific properties
        material->SetProperty("u_subsurfaceScattering", MaterialProperty(0.3f));
        material->SetProperty("u_fuzziness", MaterialProperty(0.5f));

        return material;
    }
};

// Usage
auto metalMaterial = CustomMaterialTemplate::CreateMetalTemplate("Steel");
auto fabricMaterial = CustomMaterialTemplate::CreateFabricTemplate("Cotton");
```

## Serialization

### Saving Materials to JSON

```cpp
// Save material to file
material->SaveToFile("assets/materials/my_material.json");

// Manual serialization
nlohmann::json materialJson = material->Serialize();

// Pretty-print JSON
std::string jsonString = materialJson.dump(4);
std::cout << jsonString << std::endl;
```

### Loading Materials from JSON

```cpp
// Load material from file
auto loadedMaterial = std::make_shared<Material>();
if (loadedMaterial->LoadFromFile("assets/materials/my_material.json")) {
    LOG_INFO("Material loaded successfully");
} else {
    LOG_ERROR("Failed to load material");
}

// Manual deserialization
std::string jsonString = ReadFileToString("assets/materials/my_material.json");
nlohmann::json materialJson = nlohmann::json::parse(jsonString);

auto material = std::make_shared<Material>();
if (material->Deserialize(materialJson)) {
    LOG_INFO("Material deserialized successfully");
}
```

### JSON Format Example

```json
{
  "name": "BrickMaterial",
  "type": "PBR",
  "properties": {
    "u_albedo": {
      "type": "Vec3",
      "value": [0.8, 0.4, 0.2]
    },
    "u_metallic": {
      "type": "Float",
      "value": 0.0
    },
    "u_roughness": {
      "type": "Float",
      "value": 0.7
    },
    "u_ao": {
      "type": "Float",
      "value": 1.0
    }
  },
  "textures": {
    "u_albedoMap": "assets/textures/brick_albedo.jpg",
    "u_normalMap": "assets/textures/brick_normal.jpg",
    "u_roughnessMap": "assets/textures/brick_roughness.jpg"
  },
  "shader": "PBR"
}
```

## Advanced Usage

### Material Variants

```cpp
// Create material variants for different quality levels
class MaterialVariantManager {
private:
    std::shared_ptr<Material> m_baseMaterial;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_variants;

public:
    void SetBaseMaterial(std::shared_ptr<Material> material) {
        m_baseMaterial = material;
    }

    std::shared_ptr<Material> CreateVariant(const std::string& variantName) {
        // Clone base material
        auto variant = std::make_shared<Material>(*m_baseMaterial);
        variant->SetName(m_baseMaterial->GetName() + "_" + variantName);

        m_variants[variantName] = variant;
        return variant;
    }

    std::shared_ptr<Material> GetVariant(const std::string& variantName) {
        auto it = m_variants.find(variantName);
        return (it != m_variants.end()) ? it->second : m_baseMaterial;
    }
};

// Usage
MaterialVariantManager variantManager;
variantManager.SetBaseMaterial(baseMaterial);

// Create high-quality variant
auto highQuality = variantManager.CreateVariant("HighQuality");
highQuality->SetProperty("u_normalStrength", MaterialProperty(2.0f));
highQuality->SetProperty("u_detailScale", MaterialProperty(4.0f));

// Create mobile variant
auto mobile = variantManager.CreateVariant("Mobile");
mobile->SetProperty("u_normalStrength", MaterialProperty(0.5f));
mobile->RemoveTexture("u_aoMap"); // Remove AO map for performance
```

### Dynamic Material Modification

```cpp
// Animate material properties
class MaterialAnimator {
private:
    std::shared_ptr<Material> m_material;
    float m_time = 0.0f;

public:
    MaterialAnimator(std::shared_ptr<Material> material) : m_material(material) {}

    void Update(float deltaTime) {
        m_time += deltaTime;

        // Animate roughness
        float roughness = 0.5f + 0.3f * sinf(m_time * 2.0f);
        m_material->SetRoughness(roughness);

        // Animate emission
        float emissionStrength = std::max(0.0f, sinf(m_time * 3.0f));
        m_material->SetProperty("u_emissionStrength", MaterialProperty(emissionStrength));

        // Animate color
        Math::Vec3 color = Math::Vec3(
            0.5f + 0.5f * sinf(m_time),
            0.5f + 0.5f * sinf(m_time + 2.0f),
            0.5f + 0.5f * sinf(m_time + 4.0f)
        );
        m_material->SetAlbedo(color);
    }
};
```

### Material Blending

```cpp
// Blend between two materials
std::shared_ptr<Material> BlendMaterials(std::shared_ptr<Material> materialA,
                                        std::shared_ptr<Material> materialB,
                                        float blendFactor) {
    auto blendedMaterial = Material::CreateFromTemplate(Material::Type::PBR, "Blended");

    // Blend albedo
    Math::Vec3 albedoA = materialA->GetAlbedo();
    Math::Vec3 albedoB = materialB->GetAlbedo();
    Math::Vec3 blendedAlbedo = glm::mix(albedoA, albedoB, blendFactor);
    blendedMaterial->SetAlbedo(blendedAlbedo);

    // Blend metallic
    float metallicA = materialA->GetMetallic();
    float metallicB = materialB->GetMetallic();
    float blendedMetallic = glm::mix(metallicA, metallicB, blendFactor);
    blendedMaterial->SetMetallic(blendedMetallic);

    // Blend roughness
    float roughnessA = materialA->GetRoughness();
    float roughnessB = materialB->GetRoughness();
    float blendedRoughness = glm::mix(roughnessA, roughnessB, blendFactor);
    blendedMaterial->SetRoughness(blendedRoughness);

    return blendedMaterial;
}
```

## Best Practices

### Performance Optimization

1. **Cache Materials**: Reuse materials across multiple objects
2. **Batch by Material**: Group objects by material to minimize state changes
3. **Use Templates**: Use material templates for consistent setup
4. **Optimize Textures**: Use appropriate texture formats and sizes

```cpp
// Efficient material batching
class MaterialBatcher {
private:
    std::unordered_map<std::shared_ptr<Material>, std::vector<RenderObject>> m_batches;

public:
    void AddObject(const RenderObject& object, std::shared_ptr<Material> material) {
        m_batches[material].push_back(object);
    }

    void Render() {
        for (const auto& batch : m_batches) {
            auto material = batch.first;
            const auto& objects = batch.second;

            // Bind material once
            material->Bind();

            // Render all objects with this material
            for (const auto& object : objects) {
                RenderObject(object);
            }

            material->Unbind();
        }
    }

    void Clear() {
        m_batches.clear();
    }
};
```

### Memory Management

1. **Use Smart Pointers**: Always use `std::shared_ptr` for materials
2. **Clean Up Textures**: Remove unused textures from materials
3. **Monitor Memory Usage**: Use `GetMemoryUsage()` to track material memory

```cpp
// Memory-efficient material management
class MaterialPool {
private:
    std::vector<std::shared_ptr<Material>> m_materials;
    std::queue<size_t> m_availableIndices;

public:
    std::shared_ptr<Material> AcquireMaterial() {
        if (!m_availableIndices.empty()) {
            size_t index = m_availableIndices.front();
            m_availableIndices.pop();
            return m_materials[index];
        }

        // Create new material
        auto material = std::make_shared<Material>();
        m_materials.push_back(material);
        return material;
    }

    void ReleaseMaterial(std::shared_ptr<Material> material) {
        // Find material index and mark as available
        auto it = std::find(m_materials.begin(), m_materials.end(), material);
        if (it != m_materials.end()) {
            size_t index = std::distance(m_materials.begin(), it);
            m_availableIndices.push(index);

            // Reset material to default state
            material->CreateDefault();
        }
    }

    size_t GetTotalMemoryUsage() const {
        size_t total = 0;
        for (const auto& material : m_materials) {
            total += material->GetMemoryUsage();
        }
        return total;
    }
};
```

### Error Handling

1. **Validate Properties**: Always validate material properties
2. **Handle Missing Textures**: Provide fallbacks for missing textures
3. **Check Shader Compatibility**: Ensure materials are compatible with shaders

```cpp
// Robust material validation
bool ValidateMaterial(std::shared_ptr<Material> material) {
    // Check if material has a valid shader
    if (!material->GetShader() || !material->GetShader()->IsValid()) {
        LOG_ERROR("Material has invalid shader");
        return false;
    }

    // Validate PBR properties
    if (material->GetType() == Material::Type::PBR) {
        float metallic = material->GetMetallic();
        if (metallic < 0.0f || metallic > 1.0f) {
            LOG_WARNING("Metallic value out of range: " + std::to_string(metallic));
            material->SetMetallic(std::clamp(metallic, 0.0f, 1.0f));
        }

        float roughness = material->GetRoughness();
        if (roughness < 0.0f || roughness > 1.0f) {
            LOG_WARNING("Roughness value out of range: " + std::to_string(roughness));
            material->SetRoughness(std::clamp(roughness, 0.0f, 1.0f));
        }
    }

    // Check texture validity
    auto albedoTexture = material->GetTexture("u_albedoMap");
    if (albedoTexture && !albedoTexture->IsValid()) {
        LOG_WARNING("Invalid albedo texture, removing");
        material->RemoveTexture("u_albedoMap");
    }

    return true;
}
```

This comprehensive guide covers all aspects of the Material System in Game Engine Kiro. For more specific examples and advanced techniques, refer to the shader system demonstration examples and API reference documentation.
