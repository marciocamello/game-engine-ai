#include "Resource/AssetManager.h"
#include "Core/Logger.h"
#include <filesystem>
#include <algorithm>
#include <regex>

namespace GameEngine {
namespace Resource {

AssetManager& AssetManager::GetInstance() {
    static AssetManager instance;
    return instance;
}

std::string AssetManager::ResolveAssetPath(const std::string& relativePath) const {
    // Search through all search paths in priority order
    for (const auto& searchPath : m_searchPaths) {
        std::filesystem::path fullPath = std::filesystem::path(searchPath.path) / relativePath;
        
        if (std::filesystem::exists(fullPath)) {
            return fullPath.string();
        }
    }

    // Asset not found in any search path
    LOG_WARNING("Asset not found: " + relativePath);
    return "";
}

bool AssetManager::AssetExists(const std::string& relativePath) const {
    return !ResolveAssetPath(relativePath).empty();
}

std::vector<std::string> AssetManager::FindAssetVariants(const std::string& basePath) const {
    std::vector<std::string> variants;
    
    for (const auto& searchPath : m_searchPaths) {
        std::filesystem::path searchDir = std::filesystem::path(searchPath.path) / basePath;
        
        if (std::filesystem::exists(searchDir) && std::filesystem::is_directory(searchDir)) {
            try {
                for (const auto& entry : std::filesystem::directory_iterator(searchDir)) {
                    if (entry.is_regular_file()) {
                        std::filesystem::path relativePath = std::filesystem::relative(entry.path(), searchPath.path);
                        variants.push_back(relativePath.string());
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                LOG_ERROR("Error scanning directory " + searchDir.string() + ": " + e.what());
            }
        }
    }

    // Remove duplicates while preserving order
    std::vector<std::string> uniqueVariants;
    for (const auto& variant : variants) {
        if (std::find(uniqueVariants.begin(), uniqueVariants.end(), variant) == uniqueVariants.end()) {
            uniqueVariants.push_back(variant);
        }
    }

    return uniqueVariants;
}

void AssetManager::AddSearchPath(const std::string& path, int priority) {
    // Check if path already exists
    auto it = std::find_if(m_searchPaths.begin(), m_searchPaths.end(),
        [&path](const SearchPath& sp) { return sp.path == path; });
    
    if (it != m_searchPaths.end()) {
        // Update priority if path already exists
        it->priority = priority;
    } else {
        // Add new search path
        SearchPath searchPath;
        searchPath.path = path;
        searchPath.priority = priority;
        searchPath.isShared = (path == m_sharedAssetPath);
        searchPath.isLegacy = (path == m_legacyAssetPath);
        
        m_searchPaths.push_back(searchPath);
    }
    
    SortSearchPaths();
    LOG_INFO("Added asset search path: " + path + " (priority: " + std::to_string(priority) + ")");
}

void AssetManager::RemoveSearchPath(const std::string& path) {
    auto it = std::remove_if(m_searchPaths.begin(), m_searchPaths.end(),
        [&path](const SearchPath& sp) { return sp.path == path; });
    
    if (it != m_searchPaths.end()) {
        m_searchPaths.erase(it, m_searchPaths.end());
        LOG_INFO("Removed asset search path: " + path);
    }
}

void AssetManager::ClearSearchPaths() {
    m_searchPaths.clear();
    LOG_INFO("Cleared all asset search paths");
}

std::vector<std::string> AssetManager::GetSearchPaths() const {
    std::vector<std::string> paths;
    for (const auto& searchPath : m_searchPaths) {
        paths.push_back(searchPath.path);
    }
    return paths;
}

void AssetManager::SetProjectAssetPath(const std::string& projectPath) {
    m_projectAssetPath = projectPath;
    AddSearchPath(projectPath, 100); // Highest priority
}

void AssetManager::SetSharedAssetPath(const std::string& sharedPath) {
    m_sharedAssetPath = sharedPath;
    AddSearchPath(sharedPath, 50); // Medium priority
}

void AssetManager::SetLegacyAssetPath(const std::string& legacyPath) {
    m_legacyAssetPath = legacyPath;
    AddSearchPath(legacyPath, 10); // Lowest priority
}

bool AssetManager::DeployAssets(const DeploymentConfig& config) const {
    try {
        std::filesystem::path targetPath(config.targetDirectory);
        
        // Create target directory if it doesn't exist
        if (!std::filesystem::exists(targetPath)) {
            std::filesystem::create_directories(targetPath);
        }

        // Copy project-specific assets
        if (!config.sourceProject.empty()) {
            std::string projectAssetPath = "projects/" + config.sourceProject + "/assets";
            if (std::filesystem::exists(projectAssetPath)) {
                CopyDirectoryRecursive(projectAssetPath, targetPath / "assets",
                                     config.includePatterns, config.excludePatterns);
                LOG_INFO("Deployed project assets from: " + projectAssetPath);
            }
        }

        // Copy shared assets if requested
        if (config.copySharedAssets && !m_sharedAssetPath.empty()) {
            if (std::filesystem::exists(m_sharedAssetPath)) {
                CopyDirectoryRecursive(m_sharedAssetPath, targetPath / "assets",
                                     config.includePatterns, config.excludePatterns);
                LOG_INFO("Deployed shared assets from: " + m_sharedAssetPath);
            }
        }

        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Asset deployment failed: " + std::string(e.what()));
        return false;
    }
}

bool AssetManager::CopyRelevantAssets(const std::string& projectName, const std::string& targetDir) const {
    DeploymentConfig config;
    config.sourceProject = projectName;
    config.targetDirectory = targetDir;
    config.copySharedAssets = true;
    config.overwriteExisting = true;
    
    // Include common asset types
    config.includePatterns = {
        "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tga", // Textures
        "*.obj", "*.fbx", "*.gltf", "*.glb", "*.dae", // Models
        "*.wav", "*.ogg", "*.mp3", // Audio
        "*.glsl", "*.vert", "*.frag", "*.comp", // Shaders
        "*.json", "*.xml", "*.txt" // Data files
    };
    
    return DeployAssets(config);
}

AssetManager::AssetInfo AssetManager::GetAssetInfo(const std::string& relativePath) const {
    AssetInfo info;
    info.relativePath = relativePath;
    
    for (const auto& searchPath : m_searchPaths) {
        std::filesystem::path fullPath = std::filesystem::path(searchPath.path) / relativePath;
        
        if (std::filesystem::exists(fullPath)) {
            info.fullPath = fullPath.string();
            info.searchPath = searchPath.path;
            info.isShared = searchPath.isShared;
            
            try {
                info.fileSize = std::filesystem::file_size(fullPath);
                info.lastModified = std::filesystem::last_write_time(fullPath);
            } catch (const std::filesystem::filesystem_error& e) {
                LOG_WARNING("Could not get file info for " + fullPath.string() + ": " + e.what());
                info.fileSize = 0;
            }
            
            break;
        }
    }
    
    return info;
}

std::vector<AssetManager::AssetInfo> AssetManager::ListAssets(const std::string& directory) const {
    std::vector<AssetInfo> assets;
    
    for (const auto& searchPath : m_searchPaths) {
        std::filesystem::path searchDir = std::filesystem::path(searchPath.path) / directory;
        
        if (std::filesystem::exists(searchDir) && std::filesystem::is_directory(searchDir)) {
            try {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(searchDir)) {
                    if (entry.is_regular_file()) {
                        AssetInfo info;
                        info.fullPath = entry.path().string();
                        info.relativePath = std::filesystem::relative(entry.path(), searchPath.path).string();
                        info.searchPath = searchPath.path;
                        info.isShared = searchPath.isShared;
                        info.fileSize = entry.file_size();
                        info.lastModified = entry.last_write_time();
                        
                        assets.push_back(info);
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                LOG_ERROR("Error listing assets in " + searchDir.string() + ": " + e.what());
            }
        }
    }
    
    return assets;
}

void AssetManager::SortSearchPaths() {
    std::sort(m_searchPaths.begin(), m_searchPaths.end(),
        [](const SearchPath& a, const SearchPath& b) {
            return a.priority > b.priority; // Higher priority first
        });
}

bool AssetManager::MatchesPattern(const std::string& path, const std::string& pattern) const {
    // Convert glob pattern to regex
    std::string regexPattern = pattern;
    
    // Escape special regex characters except * and ?
    std::regex specialChars(R"([\.\+\^\$\(\)\[\]\{\}\|\\])");
    regexPattern = std::regex_replace(regexPattern, specialChars, R"(\$&)");
    
    // Convert glob wildcards to regex
    std::regex globStar(R"(\\\*)");
    regexPattern = std::regex_replace(regexPattern, globStar, ".*");
    
    std::regex globQuestion(R"(\\\?)");
    regexPattern = std::regex_replace(regexPattern, globQuestion, ".");
    
    try {
        std::regex regex(regexPattern, std::regex_constants::icase);
        return std::regex_match(path, regex);
    } catch (const std::regex_error& e) {
        LOG_WARNING("Invalid pattern: " + pattern + " (" + e.what() + ")");
        return false;
    }
}

void AssetManager::CopyDirectoryRecursive(const std::filesystem::path& source,
                                        const std::filesystem::path& destination,
                                        const std::vector<std::string>& includePatterns,
                                        const std::vector<std::string>& excludePatterns) const {
    try {
        if (!std::filesystem::exists(source) || !std::filesystem::is_directory(source)) {
            return;
        }

        // Create destination directory
        std::filesystem::create_directories(destination);

        for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
            if (entry.is_regular_file()) {
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), source);
                std::string relativePathStr = relativePath.string();
                
                // Check exclude patterns first
                bool excluded = false;
                for (const auto& pattern : excludePatterns) {
                    if (MatchesPattern(relativePathStr, pattern)) {
                        excluded = true;
                        break;
                    }
                }
                
                if (excluded) {
                    continue;
                }
                
                // Check include patterns (if any)
                bool included = includePatterns.empty(); // Include all if no patterns specified
                for (const auto& pattern : includePatterns) {
                    if (MatchesPattern(relativePathStr, pattern)) {
                        included = true;
                        break;
                    }
                }
                
                if (included) {
                    std::filesystem::path targetPath = destination / relativePath;
                    
                    // Create parent directories
                    std::filesystem::create_directories(targetPath.parent_path());
                    
                    // Copy file
                    std::filesystem::copy_file(entry.path(), targetPath,
                        std::filesystem::copy_options::overwrite_existing);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Error copying directory " + source.string() + " to " + destination.string() + ": " + e.what());
    }
}

} // namespace Resource
} // namespace GameEngine