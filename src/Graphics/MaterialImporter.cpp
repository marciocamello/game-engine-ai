#include "Graphics/MaterialImporter.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <filesystem>
#include <algorithm>
#include <cmath>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/texture.h>
#endif

namespace GameEngine {

MaterialImporter::MaterialImporter() {
    // Initialize default settings
    m_settings.textureSearchPaths = { "assets/textures/", "assets/materials/", "textures/", "materials/" };
}

MaterialImporter::~MaterialImporter() {
    Shutdown();
}

bool MaterialImporter::Initialize(std::shared_ptr<ResourceManager> resourceManager) {
    if (m_initialized) {
        LOG_WARNING("MaterialImporter already initialized");
        return true;
    }

    if (!resourceManager) {
        LOG_ERROR("ResourceManager is null, cannot initialize MaterialImporter");
        return false;
    }

    m_resourceManager = resourceManager;
    
    // Create default textures
    CreateDefaultTextures();
    
    m_initialized = true;
    LOG_INFO("MaterialImporter initialized successfully");
    return true;
}

void MaterialImporter::Shutdown() {
    if (!m_initialized) {
        return;
    }

    ClearCache();
    m_resourceManager.reset();
    m_initialized = false;
    LOG_INFO("MaterialImporter shut down");
}

void MaterialImporter::SetImportSettings(const MaterialImportSettings& settings) {
    m_settings = settings;
    LOG_INFO("MaterialImporter settings updated");
}

void MaterialImporter::SetDefaultTextures(const DefaultTextures& textures) {
    m_defaultTextures = textures;
    LOG_INFO("Default textures updated");
}

void MaterialImporter::SetProgressCallback(std::function<void(const std::string&, float)> callback) {
    m_progressCallback = callback;
}

#ifdef GAMEENGINE_HAS_ASSIMP

std::vector<std::shared_ptr<Material>> MaterialImporter::ImportMaterials(const aiScene* scene, const std::string& modelPath) {
    if (!m_initialized) {
        LOG_ERROR("MaterialImporter not initialized");
        return {};
    }

    if (!scene) {
        LOG_ERROR("Scene is null");
        return {};
    }

    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve(scene->mNumMaterials);

    ReportProgress("Importing materials", 0.0f);

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* aiMat = scene->mMaterials[i];
        if (!aiMat) {
            LOG_WARNING("Material " + std::to_string(i) + " is null, skipping");
            continue;
        }

        auto material = ImportMaterial(aiMat, modelPath);
        if (material) {
            materials.push_back(material);
            m_importedMaterials.push_back(material);
        }

        float progress = static_cast<float>(i + 1) / static_cast<float>(scene->mNumMaterials);
        ReportProgress("Importing materials", progress);
    }

    LOG_INFO("Imported " + std::to_string(materials.size()) + " materials from " + modelPath);
    return materials;
}

std::shared_ptr<Material> MaterialImporter::ImportMaterial(const aiMaterial* aiMat, const std::string& modelPath) {
    if (!aiMat) {
        LOG_ERROR("aiMaterial is null");
        return nullptr;
    }

    // Get material name
    aiString materialName;
    std::string matName = "UnnamedMaterial";
    if (aiMat->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS) {
        matName = std::string(materialName.C_Str());
    }

    LOG_DEBUG("Importing material: " + matName);

    // Determine conversion mode
    MaterialConversionMode mode = m_settings.conversionMode;
    if (mode == MaterialConversionMode::Auto) {
        mode = DetermineConversionMode(aiMat);
    }

    std::shared_ptr<Material> material;
    switch (mode) {
        case MaterialConversionMode::ForcePBR:
        case MaterialConversionMode::Auto:
            material = ConvertToPBR(aiMat, modelPath);
            break;
        case MaterialConversionMode::ForceUnlit:
            material = ConvertToUnlit(aiMat, modelPath);
            break;
        case MaterialConversionMode::Preserve:
            // For now, default to PBR for preserve mode
            material = ConvertToPBR(aiMat, modelPath);
            break;
    }

    if (material) {
        LOG_INFO("Successfully imported material: " + matName);
    } else {
        LOG_ERROR("Failed to import material: " + matName);
    }

    return material;
}

std::shared_ptr<Material> MaterialImporter::ConvertToPBR(const aiMaterial* aiMat, const std::string& modelPath) {
    auto material = std::make_shared<Material>();

    // Set default PBR values
    material->SetAlbedo(m_settings.defaultAlbedo);
    material->SetMetallic(m_settings.defaultMetallic);
    material->SetRoughness(m_settings.defaultRoughness);
    material->SetAO(m_settings.defaultAO);

    // Extract diffuse/albedo color
    Math::Vec3 diffuseColor;
    if (GetMaterialProperty(aiMat, AI_MATKEY_COLOR_DIFFUSE, diffuseColor)) {
        material->SetAlbedo(diffuseColor);
    }

    // Extract metallic value (if available)
    float metallic;
    if (GetMaterialProperty(aiMat, AI_MATKEY_METALLIC_FACTOR, metallic)) {
        material->SetMetallic(metallic);
    }

    // Extract roughness value (if available)
    float roughness;
    if (GetMaterialProperty(aiMat, AI_MATKEY_ROUGHNESS_FACTOR, roughness)) {
        material->SetRoughness(roughness);
    } else {
        // Try to derive roughness from shininess
        float shininess;
        if (GetMaterialProperty(aiMat, AI_MATKEY_SHININESS, shininess)) {
            // Convert shininess to roughness (approximate conversion)
            roughness = std::sqrt(2.0f / (shininess + 2.0f));
            material->SetRoughness(roughness);
        }
    }

    // Load textures
    auto diffuseTexture = ProcessTexture(aiMat, aiTextureType_DIFFUSE, modelPath);
    if (!diffuseTexture) {
        diffuseTexture = ProcessTexture(aiMat, aiTextureType_BASE_COLOR, modelPath);
    }
    if (diffuseTexture) {
        material->SetTexture("u_albedoMap", diffuseTexture);
    }

    auto normalTexture = ProcessTexture(aiMat, aiTextureType_NORMALS, modelPath);
    if (!normalTexture) {
        normalTexture = ProcessTexture(aiMat, aiTextureType_HEIGHT, modelPath);
    }
    if (normalTexture) {
        material->SetTexture("u_normalMap", normalTexture);
    }

    auto metallicTexture = ProcessTexture(aiMat, aiTextureType_METALNESS, modelPath);
    if (metallicTexture) {
        material->SetTexture("u_metallicMap", metallicTexture);
    }

    auto roughnessTexture = ProcessTexture(aiMat, aiTextureType_DIFFUSE_ROUGHNESS, modelPath);
    if (roughnessTexture) {
        material->SetTexture("u_roughnessMap", roughnessTexture);
    }

    auto aoTexture = ProcessTexture(aiMat, aiTextureType_AMBIENT_OCCLUSION, modelPath);
    if (aoTexture) {
        material->SetTexture("u_aoMap", aoTexture);
    }

    auto emissiveTexture = ProcessTexture(aiMat, aiTextureType_EMISSIVE, modelPath);
    if (emissiveTexture) {
        material->SetTexture("u_emissiveMap", emissiveTexture);
    }

    return material;
}

std::shared_ptr<Material> MaterialImporter::ConvertToUnlit(const aiMaterial* aiMat, const std::string& modelPath) {
    auto material = std::make_shared<Material>();

    // Extract diffuse color
    Math::Vec3 diffuseColor = m_settings.defaultAlbedo;
    GetMaterialProperty(aiMat, AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->SetVec3("u_color", diffuseColor);

    // Load diffuse texture
    auto diffuseTexture = ProcessTexture(aiMat, aiTextureType_DIFFUSE, modelPath);
    if (diffuseTexture) {
        material->SetTexture("u_mainTexture", diffuseTexture);
    }

    return material;
}

std::shared_ptr<Texture> MaterialImporter::LoadEmbeddedTexture(const aiScene* scene, const std::string& texturePath) {
    if (!scene || texturePath.empty()) {
        return nullptr;
    }

    // Check if it's an embedded texture (starts with '*')
    if (texturePath[0] != '*') {
        return nullptr;
    }

    // Extract texture index
    int textureIndex = std::atoi(texturePath.c_str() + 1);
    if (textureIndex < 0 || textureIndex >= static_cast<int>(scene->mNumTextures)) {
        LOG_ERROR("Invalid embedded texture index: " + std::to_string(textureIndex));
        return nullptr;
    }

    const aiTexture* aiTex = scene->mTextures[textureIndex];
    if (!aiTex) {
        LOG_ERROR("Embedded texture is null at index: " + std::to_string(textureIndex));
        return nullptr;
    }

    // Create texture from embedded data
    auto texture = std::make_shared<Texture>("embedded_" + std::to_string(textureIndex));
    
    // TODO: Implement embedded texture loading from aiTexture data
    // This would require extending the Texture class to support loading from memory
    LOG_WARNING("Embedded texture loading not yet implemented for index: " + std::to_string(textureIndex));
    
    return CreateDefaultTexture(TextureType::Diffuse);
}

std::shared_ptr<Texture> MaterialImporter::LoadExternalTexture(const std::string& texturePath, const std::string& modelPath) {
    if (texturePath.empty()) {
        return nullptr;
    }

    // Check cache first
    std::string cacheKey = GetTextureKey(texturePath, modelPath);
    auto it = m_textureCache.find(cacheKey);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // Use enhanced texture finding system
    auto texture = FindTexture(texturePath, modelPath);
    if (texture) {
        m_textureCache[cacheKey] = texture;
        return texture;
    }

    // If not found, handle missing texture
    ++m_missingTextureCount;
    LOG_WARNING("Texture not found: " + texturePath);
    
    if (m_settings.generateMissingTextures) {
        auto fallbackTex = CreateFallbackTexture(TextureType::Diffuse, texturePath);
        m_textureCache[cacheKey] = fallbackTex;
        return fallbackTex;
    }

    return nullptr;
}

std::shared_ptr<Texture> MaterialImporter::ProcessTexture(const aiMaterial* material, aiTextureType type, const std::string& modelPath) {
    if (!material) {
        return nullptr;
    }

    unsigned int textureCount = material->GetTextureCount(type);
    if (textureCount == 0) {
        return nullptr;
    }

    // Get the first texture of this type
    aiString texturePath;
    if (material->GetTexture(type, 0, &texturePath) != AI_SUCCESS) {
        return nullptr;
    }

    std::string pathStr(texturePath.C_Str());
    if (pathStr.empty()) {
        return nullptr;
    }

    // Check if it's an embedded texture
    if (pathStr[0] == '*') {
        // TODO: Get scene reference for embedded texture loading
        LOG_WARNING("Embedded texture processing requires scene reference: " + pathStr);
        return CreateDefaultTexture(DetermineTextureType(type));
    }

    return LoadExternalTexture(pathStr, modelPath);
}

bool MaterialImporter::IsPBRMaterial(const aiMaterial* material) {
    if (!material) return false;

    // Check for PBR-specific properties
    float metallic, roughness;
    bool hasMetallic = GetMaterialProperty(material, AI_MATKEY_METALLIC_FACTOR, metallic);
    bool hasRoughness = GetMaterialProperty(material, AI_MATKEY_ROUGHNESS_FACTOR, roughness);

    // Check for PBR texture types
    bool hasBaseColor = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0;
    bool hasMetallicTex = material->GetTextureCount(aiTextureType_METALNESS) > 0;
    bool hasRoughnessTex = material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0;

    return hasMetallic || hasRoughness || hasBaseColor || hasMetallicTex || hasRoughnessTex;
}

bool MaterialImporter::IsUnlitMaterial(const aiMaterial* material) {
    if (!material) return false;

    // Check shading model
    int shadingModel;
    if (material->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == AI_SUCCESS) {
        return shadingModel == aiShadingMode_NoShading;
    }

    return false;
}

MaterialConversionMode MaterialImporter::DetermineConversionMode(const aiMaterial* material) {
    if (IsUnlitMaterial(material)) {
        return MaterialConversionMode::ForceUnlit;
    }
    
    if (IsPBRMaterial(material)) {
        return MaterialConversionMode::ForcePBR;
    }

    // Default to PBR for modern rendering
    return MaterialConversionMode::ForcePBR;
}

#endif // GAMEENGINE_HAS_ASSIMP

void MaterialImporter::AddTextureSearchPath(const std::string& path) {
    auto it = std::find(m_settings.textureSearchPaths.begin(), m_settings.textureSearchPaths.end(), path);
    if (it == m_settings.textureSearchPaths.end()) {
        m_settings.textureSearchPaths.push_back(path);
        LOG_INFO("Added texture search path: " + path);
    }
}

void MaterialImporter::ClearTextureSearchPaths() {
    m_settings.textureSearchPaths.clear();
    LOG_INFO("Cleared all texture search paths");
}

std::shared_ptr<Texture> MaterialImporter::CreateDefaultTexture(TextureType type) {
    switch (type) {
        case TextureType::Diffuse:
        case TextureType::BaseColor:
            return m_defaultTextures.defaultDiffuse ? m_defaultTextures.defaultDiffuse : CreateSolidColorTexture(Math::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
        
        case TextureType::Normal:
            return m_defaultTextures.normal ? m_defaultTextures.normal : CreateNormalMapTexture();
        
        case TextureType::Metallic:
            return m_defaultTextures.defaultMetallic ? m_defaultTextures.defaultMetallic : CreateSolidColorTexture(Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        
        case TextureType::Roughness:
            return m_defaultTextures.defaultRoughness ? m_defaultTextures.defaultRoughness : CreateSolidColorTexture(Math::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
        
        case TextureType::AO:
            return m_defaultTextures.defaultAO ? m_defaultTextures.defaultAO : CreateSolidColorTexture(Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        case TextureType::Specular:
            return m_defaultTextures.defaultSpecular ? m_defaultTextures.defaultSpecular : CreateSolidColorTexture(Math::Vec4(0.04f, 0.04f, 0.04f, 1.0f));
        
        default:
            return m_defaultTextures.white ? m_defaultTextures.white : CreateSolidColorTexture(Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
}

std::shared_ptr<Texture> MaterialImporter::FindTexture(const std::string& texturePath, const std::string& modelPath) {
    // Try multiple strategies to find the texture
    
    // 1. Try the original path first
    if (ValidateTexture(texturePath)) {
        auto texture = m_resourceManager->Load<Texture>(texturePath);
        if (texture) {
            LOG_DEBUG("Found texture at original path: " + texturePath);
            return texture;
        }
    }

    // 2. Try relative to model directory
    std::string relativePath = FindTextureRelativeToModel(texturePath, modelPath);
    if (!relativePath.empty() && ValidateTexture(relativePath)) {
        auto texture = m_resourceManager->Load<Texture>(relativePath);
        if (texture) {
            LOG_DEBUG("Found texture relative to model: " + relativePath);
            return texture;
        }
    }

    // 3. Try in search paths
    std::string filename = std::filesystem::path(texturePath).filename().string();
    std::string searchPath = FindTextureInSearchPaths(filename);
    if (!searchPath.empty()) {
        auto texture = m_resourceManager->Load<Texture>(searchPath);
        if (texture) {
            LOG_DEBUG("Found texture in search paths: " + searchPath);
            return texture;
        }
    }

    // 4. Try path variants (different extensions, case variations)
    auto pathVariants = GenerateTexturePathVariants(texturePath);
    for (const auto& variant : pathVariants) {
        if (ValidateTexture(variant)) {
            auto texture = m_resourceManager->Load<Texture>(variant);
            if (texture) {
                LOG_DEBUG("Found texture variant: " + variant);
                return texture;
            }
        }
    }

    return nullptr;
}

bool MaterialImporter::ValidateTexture(const std::string& texturePath) {
    if (texturePath.empty()) {
        return false;
    }

    // Check if file exists
    if (!std::filesystem::exists(texturePath)) {
        return false;
    }

    // Check if it's a valid texture file
    return IsValidTextureFile(texturePath);
}

std::shared_ptr<Texture> MaterialImporter::CreateFallbackTexture(TextureType type, const std::string& originalPath) {
    ++m_fallbackTextureCount;
    
    LOG_INFO("Creating fallback texture for: " + originalPath + " (type: " + std::to_string(static_cast<int>(type)) + ")");
    
    auto texture = CreateDefaultTexture(type);
    if (texture) {
        LOG_DEBUG("Successfully created fallback texture");
    } else {
        LOG_ERROR("Failed to create fallback texture");
    }
    
    return texture;
}

bool MaterialImporter::ConvertTextureFormat(const std::string& inputPath, const std::string& outputPath, TextureFormat targetFormat) {
    if (!m_settings.enableTextureConversion) {
        LOG_INFO("Texture format conversion is disabled in settings");
        return false;
    }
    
    // Check if input file exists
    if (!std::filesystem::exists(inputPath)) {
        LOG_ERROR("Input texture file does not exist: " + inputPath);
        return false;
    }
    
    // Check if conversion is supported
    std::string inputExt = GetTextureFileExtension(inputPath);
    std::string outputExt = GetTextureFileExtension(outputPath);
    
    if (!CanConvertTextureFormat(inputExt, outputExt)) {
        LOG_ERROR("Texture format conversion not supported: " + inputExt + " -> " + outputExt);
        return false;
    }
    
    // For now, we'll use a simple approach: load the texture and save it in the target format
    // In a full implementation, this would use image processing libraries like SOIL2, DevIL, or STB
    try {
        // Load the source texture
        auto sourceTexture = m_resourceManager->Load<Texture>(inputPath);
        if (!sourceTexture || !sourceTexture->IsValid()) {
            LOG_ERROR("Failed to load source texture for conversion: " + inputPath);
            return false;
        }
        
        // For now, just copy the file if formats are compatible
        if (inputExt == outputExt) {
            std::filesystem::copy_file(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);
            LOG_INFO("Copied texture file (same format): " + inputPath + " -> " + outputPath);
            return true;
        }
        
        // TODO: Implement actual format conversion using image processing library
        LOG_WARNING("Advanced texture format conversion not yet implemented: " + inputPath + " -> " + outputPath);
        LOG_INFO("Consider using external tools to convert texture formats");
        return false;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during texture format conversion: " + std::string(e.what()));
        return false;
    }
}

bool MaterialImporter::IsTextureFormatSupported(const std::string& extension) {
    static const std::vector<std::string> supportedFormats = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".hdr", ".pic", ".ppm", ".pgm"
    };
    
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    return std::find(supportedFormats.begin(), supportedFormats.end(), lowerExt) != supportedFormats.end();
}

std::vector<std::string> MaterialImporter::GetSupportedTextureFormats() {
    return {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".hdr", ".pic", ".ppm", ".pgm"};
}

void MaterialImporter::ClearCache() {
    m_textureCache.clear();
    m_importedMaterials.clear();
    m_fallbackTextureCount = 0;
    m_missingTextureCount = 0;
    LOG_INFO("MaterialImporter cache cleared");
}

#ifdef GAMEENGINE_HAS_ASSIMP

Math::Vec3 MaterialImporter::ConvertColor(const aiColor3D& color) {
    return Math::Vec3(color.r, color.g, color.b);
}

Math::Vec4 MaterialImporter::ConvertColor(const aiColor4D& color) {
    return Math::Vec4(color.r, color.g, color.b, color.a);
}

std::string MaterialImporter::FindTexturePath(const std::string& texturePath, const std::string& modelPath) {
    // Try the original path first
    if (std::filesystem::exists(texturePath)) {
        return texturePath;
    }

    // Extract model directory
    std::string modelDir = std::filesystem::path(modelPath).parent_path().string();
    if (!modelDir.empty() && modelDir.back() != '/' && modelDir.back() != '\\') {
        modelDir += "/";
    }

    // Try relative to model directory
    std::string relativePath = modelDir + texturePath;
    if (std::filesystem::exists(relativePath)) {
        return relativePath;
    }

    // Try each search path
    for (const auto& searchPath : m_settings.textureSearchPaths) {
        std::string fullPath = searchPath + texturePath;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }

        // Also try just the filename in each search path
        std::string filename = std::filesystem::path(texturePath).filename().string();
        std::string filenamePath = searchPath + filename;
        if (std::filesystem::exists(filenamePath)) {
            return filenamePath;
        }
    }

    return ""; // Not found
}

TextureType MaterialImporter::DetermineTextureType(aiTextureType aiType) {
    switch (aiType) {
        case aiTextureType_DIFFUSE:
            return TextureType::Diffuse;
        case aiTextureType_SPECULAR:
            return TextureType::Specular;
        case aiTextureType_NORMALS:
            return TextureType::Normal;
        case aiTextureType_HEIGHT:
            return TextureType::Height;
        case aiTextureType_AMBIENT:
            return TextureType::Ambient;
        case aiTextureType_EMISSIVE:
            return TextureType::Emissive;
        case aiTextureType_SHININESS:
            return TextureType::Shininess;
        case aiTextureType_OPACITY:
            return TextureType::Opacity;
        case aiTextureType_DISPLACEMENT:
            return TextureType::Displacement;
        case aiTextureType_LIGHTMAP:
            return TextureType::Lightmap;
        case aiTextureType_REFLECTION:
            return TextureType::Reflection;
        case aiTextureType_BASE_COLOR:
            return TextureType::BaseColor;
        case aiTextureType_METALNESS:
            return TextureType::Metallic;
        case aiTextureType_DIFFUSE_ROUGHNESS:
            return TextureType::Roughness;
        case aiTextureType_AMBIENT_OCCLUSION:
            return TextureType::AO;
        default:
            return TextureType::Unknown;
    }
}

std::string MaterialImporter::GetTextureKey(const std::string& path, const std::string& modelPath) {
    return modelPath + ":" + path;
}

bool MaterialImporter::GetMaterialProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, float& value) {
    return material->Get(key, type, index, value) == AI_SUCCESS;
}

bool MaterialImporter::GetMaterialProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, Math::Vec3& value) {
    aiColor3D color;
    if (material->Get(key, type, index, color) == AI_SUCCESS) {
        value = ConvertColor(color);
        return true;
    }
    return false;
}

bool MaterialImporter::GetMaterialProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, std::string& value) {
    aiString str;
    if (material->Get(key, type, index, str) == AI_SUCCESS) {
        value = std::string(str.C_Str());
        return true;
    }
    return false;
}

#endif // GAMEENGINE_HAS_ASSIMP

void MaterialImporter::CreateDefaultTextures() {
    try {
        LOG_DEBUG("Creating default textures...");
        
        // Create basic default textures
        m_defaultTextures.white = CreateSolidColorTexture(Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_defaultTextures.black = CreateSolidColorTexture(Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        m_defaultTextures.normal = CreateNormalMapTexture();
        m_defaultTextures.defaultDiffuse = CreateSolidColorTexture(Math::Vec4(0.8f, 0.8f, 0.8f, 1.0f));
        m_defaultTextures.defaultSpecular = CreateSolidColorTexture(Math::Vec4(0.04f, 0.04f, 0.04f, 1.0f));
        m_defaultTextures.defaultMetallic = CreateSolidColorTexture(Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        m_defaultTextures.defaultRoughness = CreateSolidColorTexture(Math::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
        m_defaultTextures.defaultAO = CreateSolidColorTexture(Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        LOG_INFO("Default textures created successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Exception while creating default textures: " + std::string(e.what()));
        // Continue without default textures - they will be created on demand
    } catch (...) {
        LOG_ERROR("Unknown exception while creating default textures");
        // Continue without default textures - they will be created on demand
    }
}

std::shared_ptr<Texture> MaterialImporter::CreateSolidColorTexture(const Math::Vec4& color, int width, int height) {
    try {
        auto texture = std::make_shared<Texture>("solid_color_texture");
        
        // Validate parameters
        if (width <= 0 || height <= 0) {
            LOG_WARNING("Invalid texture dimensions, using 1x1");
            width = 1;
            height = 1;
        }
        
        // Create texture data with solid color
        std::vector<unsigned char> textureData(width * height * 4);
        unsigned char r = static_cast<unsigned char>(std::clamp(color.x, 0.0f, 1.0f) * 255.0f);
        unsigned char g = static_cast<unsigned char>(std::clamp(color.y, 0.0f, 1.0f) * 255.0f);
        unsigned char b = static_cast<unsigned char>(std::clamp(color.z, 0.0f, 1.0f) * 255.0f);
        unsigned char a = static_cast<unsigned char>(std::clamp(color.w, 0.0f, 1.0f) * 255.0f);
        
        for (int i = 0; i < width * height; ++i) {
            textureData[i * 4 + 0] = r;
            textureData[i * 4 + 1] = g;
            textureData[i * 4 + 2] = b;
            textureData[i * 4 + 3] = a;
        }
        
        // Set texture properties and data
        texture->m_width = width;
        texture->m_height = height;
        texture->m_channels = 4;
        texture->m_format = TextureFormat::RGBA;
        texture->m_filepath = "[SOLID_COLOR]";
        texture->m_imageData = std::move(textureData);
        
        LOG_DEBUG("Created solid color texture (" + std::to_string(width) + "x" + std::to_string(height) + ")");
        
        return texture;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception creating solid color texture: " + std::string(e.what()));
        return nullptr;
    } catch (...) {
        LOG_ERROR("Unknown exception creating solid color texture");
        return nullptr;
    }
}

std::shared_ptr<Texture> MaterialImporter::CreateNormalMapTexture(int width, int height) {
    try {
        auto texture = std::make_shared<Texture>("default_normal_map");
        
        // Validate parameters
        if (width <= 0 || height <= 0) {
            LOG_WARNING("Invalid normal map dimensions, using 1x1");
            width = 1;
            height = 1;
        }
        
        // Create normal map texture data (default normal pointing up: 0.5, 0.5, 1.0 -> 128, 128, 255)
        std::vector<unsigned char> textureData(width * height * 3);
        
        for (int i = 0; i < width * height; ++i) {
            textureData[i * 3 + 0] = 128; // X component (0.5 * 255)
            textureData[i * 3 + 1] = 128; // Y component (0.5 * 255)
            textureData[i * 3 + 2] = 255; // Z component (1.0 * 255) - pointing up
        }
        
        // Set texture properties and data
        texture->m_width = width;
        texture->m_height = height;
        texture->m_channels = 3;
        texture->m_format = TextureFormat::RGB;
        texture->m_filepath = "[DEFAULT_NORMAL]";
        texture->m_imageData = std::move(textureData);
        
        LOG_DEBUG("Created default normal map texture (" + std::to_string(width) + "x" + std::to_string(height) + ")");
        
        return texture;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception creating normal map texture: " + std::string(e.what()));
        return nullptr;
    } catch (...) {
        LOG_ERROR("Unknown exception creating normal map texture");
        return nullptr;
    }
}

std::string MaterialImporter::FindTextureInSearchPaths(const std::string& filename) {
    for (const auto& searchPath : m_settings.textureSearchPaths) {
        std::string fullPath = searchPath + filename;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }
    }
    return "";
}

std::string MaterialImporter::FindTextureRelativeToModel(const std::string& texturePath, const std::string& modelPath) {
    if (modelPath.empty()) {
        return "";
    }

    std::string modelDir = std::filesystem::path(modelPath).parent_path().string();
    if (modelDir.empty()) {
        return "";
    }

    if (modelDir.back() != '/' && modelDir.back() != '\\') {
        modelDir += "/";
    }

    std::string relativePath = modelDir + texturePath;
    return std::filesystem::exists(relativePath) ? relativePath : "";
}

std::vector<std::string> MaterialImporter::GenerateTexturePathVariants(const std::string& originalPath) {
    std::vector<std::string> variants;
    
    std::filesystem::path pathObj(originalPath);
    std::string directory = pathObj.parent_path().string();
    std::string stem = pathObj.stem().string();
    std::string extension = pathObj.extension().string();
    
    if (!directory.empty() && directory.back() != '/' && directory.back() != '\\') {
        directory += "/";
    }

    // Try different extensions
    std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds"};
    for (const auto& ext : extensions) {
        if (ext != extension) {
            variants.push_back(directory + stem + ext);
        }
    }

    // Try case variations
    std::string upperStem = stem;
    std::string lowerStem = stem;
    std::transform(upperStem.begin(), upperStem.end(), upperStem.begin(), ::toupper);
    std::transform(lowerStem.begin(), lowerStem.end(), lowerStem.begin(), ::tolower);
    
    if (upperStem != stem) {
        variants.push_back(directory + upperStem + extension);
    }
    if (lowerStem != stem) {
        variants.push_back(directory + lowerStem + extension);
    }

    // Try with different directory separators
    std::string normalizedPath = originalPath;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
    if (normalizedPath != originalPath) {
        variants.push_back(normalizedPath);
    }
    
    normalizedPath = originalPath;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '/', '\\');
    if (normalizedPath != originalPath) {
        variants.push_back(normalizedPath);
    }

    return variants;
}

bool MaterialImporter::IsValidTextureFile(const std::string& path) {
    std::string extension = GetTextureFileExtension(path);
    return IsTextureFormatSupported(extension);
}

std::string MaterialImporter::GetTextureFileExtension(const std::string& path) {
    std::filesystem::path pathObj(path);
    return pathObj.extension().string();
}

bool MaterialImporter::CanConvertTextureFormat(const std::string& fromExt, const std::string& toExt) {
    return IsTextureFormatSupported(fromExt) && IsTextureFormatSupported(toExt);
}

void MaterialImporter::ReportProgress(const std::string& operation, float progress) {
    if (m_progressCallback) {
        m_progressCallback(operation, progress);
    }
}

} // namespace GameEngine