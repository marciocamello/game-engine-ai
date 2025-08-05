#include "Core/AssetValidator.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>

namespace GameEngine {

    AssetValidator::AssetValidator() {
        InitializeDefaultFallbacks();
        RegisterRequiredAssets();
    }

    AssetValidator::~AssetValidator() = default;

    bool AssetValidator::ValidateAsset(const std::string& path) const {
        if (path.empty()) {
            return false;
        }
        
        return FileExists(path);
    }

    bool AssetValidator::ValidateAssets(const std::vector<std::string>& paths) const {
        bool allValid = true;
        
        for (const auto& path : paths) {
            if (!ValidateAsset(path)) {
                LOG_WARNING("Asset validation failed: " + path);
                allValid = false;
            }
        }
        
        return allValid;
    }

    std::string AssetValidator::GetFallbackPath(const std::string& originalPath, const std::string& assetType) const {
        if (assetType == "texture") {
            return m_defaultTextureFallback;
        } else if (assetType == "model" || assetType == "mesh") {
            return m_defaultModelFallback;
        } else if (assetType == "audio") {
            return m_defaultAudioFallback;
        }
        
        return "";
    }

    void AssetValidator::RegisterFallback(const std::string& assetType, const std::string& fallbackPath) {
        if (assetType == "texture") {
            m_defaultTextureFallback = fallbackPath;
        } else if (assetType == "model" || assetType == "mesh") {
            m_defaultModelFallback = fallbackPath;
        } else if (assetType == "audio") {
            m_defaultAudioFallback = fallbackPath;
        }
        
        m_fallbackTypes.insert(assetType);
    }

    void AssetValidator::RegisterAsset(const AssetInfo& asset) {
        m_registeredAssets.push_back(asset);
    }

    void AssetValidator::RegisterRequiredAssets() {
        // Register critical assets for the enhanced game experience
        
        // Character model (optional - has fallback)
        RegisterAsset({
            "assets/meshes/XBot.fbx",
            "model",
            false,
            "capsule_fallback"
        });
        
        // Environment texture (optional - has color fallback)
        RegisterAsset({
            "assets/textures/wall.jpg",
            "texture",
            false,
            "color_fallback"
        });
        
        // Audio files (optional - can run without audio)
        RegisterAsset({
            "assets/audio/file_example_WAV_5MG.wav",
            "audio",
            false,
            ""
        });
        
        RegisterAsset({
            "assets/audio/cartoon-jump.wav",
            "audio",
            false,
            ""
        });
        
        RegisterAsset({
            "assets/audio/concrete-footsteps.wav",
            "audio",
            false,
            ""
        });
        
        // Capsule mesh for debug visualization (optional)
        RegisterAsset({
            "assets/meshes/capsule.obj",
            "mesh",
            false,
            "primitive_fallback"
        });
    }

    std::vector<std::string> AssetValidator::GetMissingAssets() const {
        std::vector<std::string> missing;
        
        for (const auto& asset : m_registeredAssets) {
            if (!ValidateAsset(asset.path)) {
                missing.push_back(asset.path);
            }
        }
        
        return missing;
    }

    void AssetValidator::LogAssetStatus() const {
        LOG_INFO("========================================");
        LOG_INFO("ASSET VALIDATION REPORT");
        LOG_INFO("========================================");
        
        int availableCount = 0;
        int missingCount = 0;
        int requiredMissing = 0;
        
        for (const auto& asset : m_registeredAssets) {
            bool exists = ValidateAsset(asset.path);
            
            if (exists) {
                availableCount++;
                LOG_INFO("✓ " + asset.path + " (" + asset.type + ")");
            } else {
                missingCount++;
                if (asset.required) {
                    requiredMissing++;
                    LOG_ERROR("✗ " + asset.path + " (" + asset.type + ") - REQUIRED");
                } else {
                    LOG_WARNING("⚠ " + asset.path + " (" + asset.type + ") - Optional, fallback available");
                }
            }
        }
        
        LOG_INFO("----------------------------------------");
        LOG_INFO("Available Assets: " + std::to_string(availableCount));
        LOG_INFO("Missing Assets: " + std::to_string(missingCount));
        LOG_INFO("Missing Required: " + std::to_string(requiredMissing));
        LOG_INFO("System Status: " + std::string(requiredMissing == 0 ? "STABLE" : "UNSTABLE"));
        LOG_INFO("========================================");
    }

    bool AssetValidator::AllRequiredAssetsAvailable() const {
        for (const auto& asset : m_registeredAssets) {
            if (asset.required && !ValidateAsset(asset.path)) {
                return false;
            }
        }
        return true;
    }

    bool AssetValidator::FileExists(const std::string& path) const {
        try {
            return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
        } catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    }

    void AssetValidator::InitializeDefaultFallbacks() {
        // Initialize with reasonable defaults
        m_fallbackTypes.insert("texture");
        m_fallbackTypes.insert("model");
        m_fallbackTypes.insert("mesh");
        m_fallbackTypes.insert("audio");
    }

}