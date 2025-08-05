#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace GameEngine {

    /**
     * @brief Asset validation and fallback system
     * 
     * Provides comprehensive error handling for missing assets with
     * graceful fallbacks to ensure stable operation.
     */
    class AssetValidator {
    public:
        struct AssetInfo {
            std::string path;
            std::string type;
            bool required;
            std::string fallbackPath;
        };

        AssetValidator();
        ~AssetValidator();

        // Asset validation
        bool ValidateAsset(const std::string& path) const;
        bool ValidateAssets(const std::vector<std::string>& paths) const;
        
        // Fallback management
        std::string GetFallbackPath(const std::string& originalPath, const std::string& assetType) const;
        void RegisterFallback(const std::string& assetType, const std::string& fallbackPath);
        
        // Asset registration
        void RegisterAsset(const AssetInfo& asset);
        void RegisterRequiredAssets();
        
        // Validation reporting
        std::vector<std::string> GetMissingAssets() const;
        void LogAssetStatus() const;
        bool AllRequiredAssetsAvailable() const;

    private:
        bool FileExists(const std::string& path) const;
        void InitializeDefaultFallbacks();

        std::vector<AssetInfo> m_registeredAssets;
        std::unordered_set<std::string> m_fallbackTypes;
        
        // Default fallback paths
        std::string m_defaultTextureFallback = "assets/textures/missing_texture.png";
        std::string m_defaultModelFallback = "assets/meshes/cube.obj";
        std::string m_defaultAudioFallback = "";
    };

}