#pragma once

#include "Graphics/Material.h"
#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace GameEngine {

    /**
     * @brief MTL (Material Template Library) file loader for OBJ materials
     * 
     * Supports standard MTL properties including:
     * - Ambient, diffuse, and specular colors
     * - Texture maps (diffuse, normal, specular, etc.)
     * - Material properties (shininess, transparency, etc.)
     * - PBR extensions where available
     */
    class MTLLoader {
    public:
        /**
         * @brief Material data parsed from MTL file
         */
        struct MTLMaterial {
            std::string name;
            
            // Basic material properties
            Math::Vec3 ambient = Math::Vec3(0.2f);      // Ka
            Math::Vec3 diffuse = Math::Vec3(0.8f);      // Kd
            Math::Vec3 specular = Math::Vec3(1.0f);     // Ks
            Math::Vec3 emissive = Math::Vec3(0.0f);     // Ke
            float shininess = 32.0f;                    // Ns
            float transparency = 1.0f;                  // d or Tr
            float indexOfRefraction = 1.0f;             // Ni
            int illuminationModel = 2;                  // illum
            
            // Texture maps
            std::string diffuseMap;                     // map_Kd
            std::string ambientMap;                     // map_Ka
            std::string specularMap;                    // map_Ks
            std::string normalMap;                      // map_Bump or bump
            std::string heightMap;                      // map_Disp
            std::string alphaMap;                       // map_d
            std::string reflectionMap;                  // refl
            
            // PBR extensions (if available)
            float metallic = 0.0f;                      // Pm
            float roughness = 0.5f;                     // Pr
            std::string metallicMap;                    // map_Pm
            std::string roughnessMap;                   // map_Pr
            std::string aoMap;                          // map_Ka (when used for AO)
            
            // Validation
            bool isValid = true;
            std::string errorMessage;
        };

        /**
         * @brief Result of MTL loading operation
         */
        struct LoadResult {
            std::unordered_map<std::string, MTLMaterial> materials;
            bool success = false;
            std::string errorMessage;
            uint32_t materialCount = 0;
            float loadingTimeMs = 0.0f;
        };

    public:
        MTLLoader() = default;
        ~MTLLoader() = default;

        // Main loading interface
        LoadResult LoadMTL(const std::string& filepath);
        LoadResult LoadMTLFromString(const std::string& mtlContent, const std::string& basePath = "");

        // Material conversion
        std::shared_ptr<Material> ConvertToEngineMaterial(const MTLMaterial& mtlMaterial, const std::string& basePath = "");
        std::vector<std::shared_ptr<Material>> ConvertAllMaterials(const LoadResult& result, const std::string& basePath = "");

        // Utility methods
        static bool IsMTLFile(const std::string& filepath);
        static std::string FindMTLFile(const std::string& objFilepath, const std::string& mtlFilename);

        // Configuration
        void SetTextureSearchPaths(const std::vector<std::string>& paths) { m_textureSearchPaths = paths; }
        void SetCreateDefaultTextures(bool create) { m_createDefaultTextures = create; }
        void SetVerboseLogging(bool verbose) { m_verboseLogging = verbose; }

    private:
        // Parsing methods
        bool ParseMTLLine(const std::string& line, MTLMaterial& currentMaterial, const std::string& basePath);
        bool ParseNewMaterial(const std::string& line, std::string& materialName);
        bool ParseColor(const std::string& line, Math::Vec3& color);
        bool ParseFloat(const std::string& line, float& value);
        bool ParseInt(const std::string& line, int& value);
        bool ParseTexture(const std::string& line, std::string& texturePath, const std::string& basePath);
        
        // Texture loading helpers
        std::shared_ptr<Texture> LoadTexture(const std::string& texturePath, const std::string& basePath);
        std::string ResolveTexturePath(const std::string& texturePath, const std::string& basePath);
        std::shared_ptr<Texture> CreateDefaultTexture(const Math::Vec3& color);
        
        // Utility methods
        std::vector<std::string> SplitString(const std::string& str, char delimiter);
        std::string TrimString(const std::string& str);
        static std::string GetDirectoryPath(const std::string& filepath);
        
        // Configuration
        std::vector<std::string> m_textureSearchPaths;
        bool m_createDefaultTextures = true;
        bool m_verboseLogging = false;
        
        // Texture cache to avoid loading duplicates
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
    };

} // namespace GameEngine