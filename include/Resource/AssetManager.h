#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace GameEngine {
namespace Resource {

/**
 * Asset path resolution and management system
 * Handles asset loading from multiple search paths with priority order
 */
class AssetManager {
public:
    static AssetManager& GetInstance();

    // Asset path resolution
    std::string ResolveAssetPath(const std::string& relativePath) const;
    bool AssetExists(const std::string& relativePath) const;
    std::vector<std::string> FindAssetVariants(const std::string& basePath) const;

    // Search path management
    void AddSearchPath(const std::string& path, int priority = 0);
    void RemoveSearchPath(const std::string& path);
    void ClearSearchPaths();
    std::vector<std::string> GetSearchPaths() const;

    // Project-specific configuration
    void SetProjectAssetPath(const std::string& projectPath);
    void SetSharedAssetPath(const std::string& sharedPath);
    void SetLegacyAssetPath(const std::string& legacyPath);

    // Asset deployment
    struct DeploymentConfig {
        std::string sourceProject;
        std::string targetDirectory;
        std::vector<std::string> includePatterns;
        std::vector<std::string> excludePatterns;
        bool copySharedAssets = true;
        bool overwriteExisting = true;
    };

    bool DeployAssets(const DeploymentConfig& config) const;
    bool CopyRelevantAssets(const std::string& projectName, const std::string& targetDir) const;

    // Asset information
    struct AssetInfo {
        std::string fullPath;
        std::string relativePath;
        std::string searchPath;
        size_t fileSize;
        std::filesystem::file_time_type lastModified;
        bool isShared;
    };

    AssetInfo GetAssetInfo(const std::string& relativePath) const;
    std::vector<AssetInfo> ListAssets(const std::string& directory = "") const;

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    struct SearchPath {
        std::string path;
        int priority;
        bool isShared;
        bool isLegacy;
    };

    std::vector<SearchPath> m_searchPaths;
    std::string m_projectAssetPath;
    std::string m_sharedAssetPath;
    std::string m_legacyAssetPath;

    void SortSearchPaths();
    bool MatchesPattern(const std::string& path, const std::string& pattern) const;
    void CopyDirectoryRecursive(const std::filesystem::path& source, 
                               const std::filesystem::path& destination,
                               const std::vector<std::string>& includePatterns,
                               const std::vector<std::string>& excludePatterns) const;
};

} // namespace Resource
} // namespace GameEngine