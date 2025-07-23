#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include <filesystem>
#include <sstream>

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
        LOG_INFO("Resource Manager shutdown - Total loads: " + std::to_string(m_totalLoads) + 
                 ", Cache hits: " + std::to_string(m_cacheHits) + 
                 ", Cache misses: " + std::to_string(m_cacheMisses));
    }

    void ResourceManager::UnloadAll() {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        size_t resourceCount = GetResourceCount();
        m_resources.clear();
        LOG_INFO("All resources unloaded (" + std::to_string(resourceCount) + " resources)");
    }

    void ResourceManager::UnloadUnused() {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        size_t removedCount = 0;
        
        for (auto it = m_resources.begin(); it != m_resources.end();) {
            if (it->second.expired()) {
                it = m_resources.erase(it);
                ++removedCount;
            } else {
                ++it;
            }
        }
        
        if (removedCount > 0) {
            LOG_INFO("Cleaned up " + std::to_string(removedCount) + " expired resource references");
        }
    }

    size_t ResourceManager::GetMemoryUsage() const {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        size_t totalMemory = 0;
        
        for (const auto& pair : m_resources) {
            if (auto resource = pair.second.lock()) {
                totalMemory += resource->GetMemoryUsage();
            }
        }
        
        return totalMemory;
    }

    size_t ResourceManager::GetResourceCount() const {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        size_t activeCount = 0;
        
        for (const auto& pair : m_resources) {
            if (!pair.second.expired()) {
                ++activeCount;
            }
        }
        
        return activeCount;
    }

    ResourceStats ResourceManager::GetResourceStats() const {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        ResourceStats stats;
        
        for (const auto& pair : m_resources) {
            if (auto resource = pair.second.lock()) {
                ++stats.totalResources;
                size_t memUsage = resource->GetMemoryUsage();
                stats.totalMemoryUsage += memUsage;
                
                // Extract type name from key (format: "typename:path")
                std::string key = pair.first;
                size_t colonPos = key.find(':');
                if (colonPos != std::string::npos) {
                    std::string typeName = key.substr(0, colonPos);
                    stats.resourcesByType[typeName]++;
                    stats.memoryByType[typeName] += memUsage;
                }
            } else {
                ++stats.expiredReferences;
            }
        }
        
        return stats;
    }

    void ResourceManager::UnloadLeastRecentlyUsed(size_t targetMemoryReduction) {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        
        // Collect all active resources with their last access times
        std::vector<std::pair<std::string, std::shared_ptr<Resource>>> activeResources;
        
        for (const auto& pair : m_resources) {
            if (auto resource = pair.second.lock()) {
                activeResources.emplace_back(pair.first, resource);
            }
        }
        
        if (activeResources.empty()) {
            LOG_INFO("No resources to unload for LRU cleanup");
            return;
        }
        
        // Sort by last access time (oldest first)
        std::sort(activeResources.begin(), activeResources.end(),
            [](const auto& a, const auto& b) {
                return a.second->GetLastAccessTime() < b.second->GetLastAccessTime();
            });
        
        size_t memoryFreed = 0;
        size_t resourcesUnloaded = 0;
        
        // Unload resources starting from least recently used
        for (const auto& pair : activeResources) {
            size_t resourceMemory = pair.second->GetMemoryUsage();
            
            // Remove from cache (this will allow the resource to be destroyed if no other references exist)
            m_resources.erase(pair.first);
            
            memoryFreed += resourceMemory;
            ++resourcesUnloaded;
            ++m_lruCleanups;
            
            LOG_INFO("LRU cleanup: Unloaded resource '" + pair.second->GetPath() + 
                     "' (" + std::to_string(resourceMemory / 1024) + " KB)");
            
            // Stop if we've reached the target memory reduction
            if (targetMemoryReduction > 0 && memoryFreed >= targetMemoryReduction) {
                break;
            }
            
            // Don't unload more than 50% of resources in one cleanup
            if (resourcesUnloaded >= activeResources.size() / 2) {
                break;
            }
        }
        
        LOG_INFO("LRU cleanup completed: Unloaded " + std::to_string(resourcesUnloaded) + 
                 " resources, freed ~" + std::to_string(memoryFreed / 1024 / 1024) + " MB");
    }
    
    void ResourceManager::SetMemoryPressureThreshold(size_t thresholdBytes) {
        m_memoryPressureThreshold = thresholdBytes;
        LOG_INFO("Memory pressure threshold set to " + std::to_string(thresholdBytes / 1024 / 1024) + " MB");
    }
    
    void ResourceManager::CheckMemoryPressure() {
        if (!m_autoMemoryManagement) {
            return;
        }
        
        size_t currentMemoryUsage = GetMemoryUsage();
        
        if (currentMemoryUsage > m_memoryPressureThreshold) {
            LOG_WARNING("Memory pressure detected: " + std::to_string(currentMemoryUsage / 1024 / 1024) + 
                       " MB > " + std::to_string(m_memoryPressureThreshold / 1024 / 1024) + " MB threshold");
            
            // Try to free 25% of current memory usage
            size_t targetReduction = currentMemoryUsage / 4;
            UnloadLeastRecentlyUsed(targetReduction);
        }
    }

    void ResourceManager::LogResourceUsage() const {
        ResourceStats stats = GetResourceStats();
        
        std::stringstream ss;
        ss << "Resource Manager Statistics:\n";
        ss << "  Total Resources: " << stats.totalResources << "\n";
        ss << "  Total Memory Usage: " << (stats.totalMemoryUsage / 1024.0 / 1024.0) << " MB\n";
        ss << "  Expired References: " << stats.expiredReferences << "\n";
        ss << "  Cache Hit Rate: " << (m_totalLoads > 0 ? (m_cacheHits * 100.0 / m_totalLoads) : 0.0) << "%\n";
        ss << "  LRU Cleanups: " << m_lruCleanups << "\n";
        ss << "  Memory Threshold: " << (m_memoryPressureThreshold / 1024.0 / 1024.0) << " MB\n";
        
        if (!stats.resourcesByType.empty()) {
            ss << "  Resources by Type:\n";
            for (const auto& pair : stats.resourcesByType) {
                double memMB = stats.memoryByType.at(pair.first) / 1024.0 / 1024.0;
                ss << "    " << pair.first << ": " << pair.second << " resources, " 
                   << memMB << " MB\n";
            }
        }
        
        LOG_INFO(ss.str());
    }
    
    void ResourceManager::LogDetailedResourceInfo() const {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        
        std::stringstream ss;
        ss << "Detailed Resource Information:\n";
        
        // Collect active resources with timing info
        std::vector<std::pair<std::string, std::shared_ptr<Resource>>> activeResources;
        
        for (const auto& pair : m_resources) {
            if (auto resource = pair.second.lock()) {
                activeResources.emplace_back(pair.first, resource);
            }
        }
        
        if (activeResources.empty()) {
            ss << "  No active resources\n";
        } else {
            // Sort by memory usage (largest first)
            std::sort(activeResources.begin(), activeResources.end(),
                [](const auto& a, const auto& b) {
                    return a.second->GetMemoryUsage() > b.second->GetMemoryUsage();
                });
            
            auto now = std::chrono::steady_clock::now();
            
            for (const auto& pair : activeResources) {
                auto resource = pair.second;
                auto loadTime = std::chrono::duration_cast<std::chrono::seconds>(
                    now - resource->GetLoadTime()).count();
                auto lastAccessTime = std::chrono::duration_cast<std::chrono::seconds>(
                    now - resource->GetLastAccessTime()).count();
                
                ss << "  " << resource->GetPath() << ":\n";
                ss << "    Memory: " << (resource->GetMemoryUsage() / 1024.0) << " KB\n";
                ss << "    Loaded: " << loadTime << "s ago\n";
                ss << "    Last Access: " << lastAccessTime << "s ago\n";
            }
        }
        
        LOG_INFO(ss.str());
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