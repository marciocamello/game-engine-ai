#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include <filesystem>

namespace GameEngine {
    ResourceManager::ResourceManager() {
    }

    ResourceManager::~ResourceManager() {
        Shutdown();
    }

    bool ResourceManager::Initialize() {
        // Create assets directory if it doesn't exist
        if (!std::filesystem::exists(m_assetDirectory)) {
            std::filesystem::create_directories(m_assetDirectory);
            LOG_INFO("Created assets directory: " + m_assetDirectory);
        }

        LOG_INFO("Resource Manager initialized");
        return true;
    }

    void ResourceManager::Shutdown() {
        UnloadAll();
        LOG_INFO("Resource Manager shutdown");
    }

    void ResourceManager::UnloadAll() {
        m_resources.clear();
        LOG_INFO("All resources unloaded");
    }

    bool ResourceManager::ImportAsset(const std::string& sourcePath, const std::string& targetPath) {
        try {
            std::filesystem::copy_file(sourcePath, m_assetDirectory + targetPath, 
                std::filesystem::copy_options::overwrite_existing);
            LOG_INFO("Asset imported: " + sourcePath + " -> " + targetPath);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to import asset: " + std::string(e.what()));
            return false;
        }
    }

    bool ResourceManager::ExportAsset(const std::string& assetPath, const std::string& exportPath) {
        try {
            std::filesystem::copy_file(m_assetDirectory + assetPath, exportPath,
                std::filesystem::copy_options::overwrite_existing);
            LOG_INFO("Asset exported: " + assetPath + " -> " + exportPath);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to export asset: " + std::string(e.what()));
            return false;
        }
    }
}