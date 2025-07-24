#pragma once

#include "Core/Math.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Resource/ResourceManager.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

#ifdef GAMEENGINE_HAS_ASSIMP
#include <assimp/scene.h>
#include <assimp/material.h>
#endif

namespace GameEngine {
    class Shader;

    enum class MaterialConversionMode {
        Auto,           // Automatically detect best material type
        ForcePBR,       // Force conversion to PBR materials
        ForceUnlit,     // Force conversion to unlit materials
        Preserve        // Preserve original material properties as much as possible
    };

    enum class TextureType {
        Diffuse,
        Specular,
        Normal,
        Height,
        Ambient,
        Emissive,
        Shininess,
        Opacity,
        Displacement,
        Lightmap,
        Reflection,
        BaseColor,      // PBR
        Metallic,       // PBR
        Roughness,      // PBR
        AO,             // PBR (Ambient Occlusion)
        Unknown
    };

    struct DefaultTextures {
        std::shared_ptr<Texture> white;
        std::shared_ptr<Texture> black;
        std::shared_ptr<Texture> normal;
        std::shared_ptr<Texture> defaultDiffuse;
        std::shared_ptr<Texture> defaultSpecular;
        std::shared_ptr<Texture> defaultMetallic;
        std::shared_ptr<Texture> defaultRoughness;
        std::shared_ptr<Texture> defaultAO;
    };

    struct MaterialImportSettings {
        MaterialConversionMode conversionMode = MaterialConversionMode::Auto;
        std::vector<std::string> textureSearchPaths;
        bool generateMissingTextures = true;
        bool enableTextureConversion = true;
        bool preserveOriginalPaths = false;
        float defaultMetallic = 0.0f;
        float defaultRoughness = 0.5f;
        float defaultAO = 1.0f;
        Math::Vec3 defaultAlbedo = Math::Vec3(0.8f, 0.8f, 0.8f);
    };

    class MaterialImporter {
    public:
        MaterialImporter();
        ~MaterialImporter();

        // Initialization
        bool Initialize(std::shared_ptr<ResourceManager> resourceManager);
        void Shutdown();

        // Settings
        void SetImportSettings(const MaterialImportSettings& settings);
        const MaterialImportSettings& GetImportSettings() const { return m_settings; }
        void SetDefaultTextures(const DefaultTextures& textures);
        void SetProgressCallback(std::function<void(const std::string&, float)> callback);

#ifdef GAMEENGINE_HAS_ASSIMP
        // Material import from Assimp
        std::vector<std::shared_ptr<Material>> ImportMaterials(const aiScene* scene, const std::string& modelPath);
        std::shared_ptr<Material> ImportMaterial(const aiMaterial* aiMat, const std::string& modelPath);

        // Texture loading
        std::shared_ptr<Texture> LoadEmbeddedTexture(const aiScene* scene, const std::string& texturePath);
        std::shared_ptr<Texture> LoadExternalTexture(const std::string& texturePath, const std::string& modelPath);

        // Material conversion
        std::shared_ptr<Material> ConvertToPBR(const aiMaterial* aiMat, const std::string& modelPath);
        std::shared_ptr<Material> ConvertToUnlit(const aiMaterial* aiMat, const std::string& modelPath);
#endif

        // Texture management
        void AddTextureSearchPath(const std::string& path);
        void ClearTextureSearchPaths();
        std::shared_ptr<Texture> CreateDefaultTexture(TextureType type);
        
        // Texture search and fallback system
        std::shared_ptr<Texture> FindTexture(const std::string& texturePath, const std::string& modelPath);
        std::vector<std::string> GetTextureSearchPaths() const { return m_settings.textureSearchPaths; }
        bool ValidateTexture(const std::string& texturePath);
        std::shared_ptr<Texture> CreateFallbackTexture(TextureType type, const std::string& originalPath);
        
        // Texture format conversion and validation
        bool ConvertTextureFormat(const std::string& inputPath, const std::string& outputPath, TextureFormat targetFormat);
        bool IsTextureFormatSupported(const std::string& extension);
        std::vector<std::string> GetSupportedTextureFormats();
        bool CanConvertTextureFormat(const std::string& fromExt, const std::string& toExt);

        // Statistics and debugging
        size_t GetImportedMaterialCount() const { return m_importedMaterials.size(); }
        size_t GetImportedTextureCount() const { return m_textureCache.size(); }
        size_t GetFallbackTextureCount() const { return m_fallbackTextureCount; }
        size_t GetMissingTextureCount() const { return m_missingTextureCount; }
        void ClearCache();

    private:
        std::shared_ptr<ResourceManager> m_resourceManager;
        MaterialImportSettings m_settings;
        DefaultTextures m_defaultTextures;
        std::function<void(const std::string&, float)> m_progressCallback;

        // Caching
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
        std::vector<std::shared_ptr<Material>> m_importedMaterials;
        
        // Statistics
        mutable size_t m_fallbackTextureCount = 0;
        mutable size_t m_missingTextureCount = 0;

        // Initialization state
        bool m_initialized = false;

#ifdef GAMEENGINE_HAS_ASSIMP
        // Helper methods
        Math::Vec3 ConvertColor(const aiColor3D& color);
        Math::Vec4 ConvertColor(const aiColor4D& color);
        std::string FindTexturePath(const std::string& texturePath, const std::string& modelPath);
        TextureType DetermineTextureType(aiTextureType aiType);
        std::string GetTextureKey(const std::string& path, const std::string& modelPath);
        
        // Material property extraction
        bool GetMaterialProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, float& value);
        bool GetMaterialProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, Math::Vec3& value);
        bool GetMaterialProperty(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, std::string& value);
        
        // Texture processing
        std::shared_ptr<Texture> ProcessTexture(const aiMaterial* material, aiTextureType type, const std::string& modelPath);
        std::vector<std::shared_ptr<Texture>> ProcessAllTextures(const aiMaterial* material, const std::string& modelPath);
        
        // Material type detection
        bool IsPBRMaterial(const aiMaterial* material);
        bool IsUnlitMaterial(const aiMaterial* material);
        MaterialConversionMode DetermineConversionMode(const aiMaterial* material);
#endif

        // Default texture creation
        void CreateDefaultTextures();
        std::shared_ptr<Texture> CreateSolidColorTexture(const Math::Vec4& color, int width = 1, int height = 1);
        std::shared_ptr<Texture> CreateNormalMapTexture(int width = 1, int height = 1);
        
        // Advanced texture search
        std::string FindTextureInSearchPaths(const std::string& filename);
        std::string FindTextureRelativeToModel(const std::string& texturePath, const std::string& modelPath);
        std::vector<std::string> GenerateTexturePathVariants(const std::string& originalPath);
        
        // Texture validation and conversion
        bool IsValidTextureFile(const std::string& path);
        std::string GetTextureFileExtension(const std::string& path);

        // Progress reporting
        void ReportProgress(const std::string& operation, float progress);
    };
}