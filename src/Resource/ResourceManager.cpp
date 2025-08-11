#include "Resource/ResourceManager.h"
#include "Resource/ResourceMemoryPool.h"
#include "Resource/LRUResourceCache.h"
#include "Resource/GPUUploadOptimizer.h"
#include "../../engine/core/Logger.h"
#include <filesystem>
#include <sstream>

namespace GameEngine {
    ResourceManager::ResourceManager() : m_lastMemoryPressureCheck(std::chrono::steady_clock::now()) {
        // Initialize performance optimization components
        m_memoryPool = std::make_unique<ResourceMemoryPool>();
        m_lruCache = std::make_unique<LRUResourceCache<Resource>>();
        m_gpuUploadOptimizer = std::make_unique<GPUUploadOptimizer>();
        
        LOG_DEBUG("ResourceManager created with performance optimizations");
    }

    ResourceManager::~ResourceManager() {
        Shutdown();
    }

    bool ResourceManager::Initialize() {
        LOG_INFO("Initializing Resource Manager...");
        
        try {
            // Create assets directory if it doesn't exist
            if (!std::filesystem::exists(m_assetDirectory)) {
                std::filesystem::create_directories(m_assetDirectory);
                LOG_INFO("Created assets directory: " + m_assetDirectory);
            } else {
                LOG_INFO("Assets directory exists: " + m_assetDirectory);
            }
            
            // Verify directory is writable
            std::string testFile = m_assetDirectory + ".resource_manager_test";
            try {
                std::ofstream test(testFile);
                if (test.is_open()) {
                    test << "test";
                    test.close();
                    std::filesystem::remove(testFile);
                    LOG_DEBUG("Assets directory is writable");
                } else {
                    LOG_WARNING("Assets directory may not be writable: " + m_assetDirectory);
                }
            } catch (const std::exception& e) {
                LOG_WARNING("Could not verify assets directory writability: " + std::string(e.what()));
            }
            
            // Initialize performance optimization components
            if (m_memoryPoolingEnabled && m_memoryPool) {
                m_memoryPool->PreallocatePool();
            }
            
            if (m_gpuUploadOptimizationEnabled && m_gpuUploadOptimizer) {
                m_gpuUploadOptimizer->Initialize();
            }
            
            // Initialize error handling state
            m_loadFailureCount = 0;
            m_memoryPressureEvents = 0;
            m_fallbackResourcesCreated = 0;
            m_lastMemoryPressureCheck = std::chrono::steady_clock::now();
            
            LOG_INFO("Resource Manager initialized successfully");
            LOG_INFO("  Assets directory: " + m_assetDirectory);
            LOG_INFO("  Memory pressure threshold: " + std::to_string(m_memoryPressureThreshold / 1024 / 1024) + " MB");
            LOG_INFO("  Auto memory management: " + std::string(m_autoMemoryManagement ? "enabled" : "disabled"));
            LOG_INFO("  Fallback resources: " + std::string(m_fallbackResourcesEnabled ? "enabled" : "disabled"));
            LOG_INFO("  Memory pooling: " + std::string(m_memoryPoolingEnabled ? "enabled" : "disabled"));
            LOG_INFO("  LRU caching: " + std::string(m_lruCacheEnabled ? "enabled" : "disabled"));
            LOG_INFO("  GPU upload optimization: " + std::string(m_gpuUploadOptimizationEnabled ? "enabled" : "disabled"));
            
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize Resource Manager: " + std::string(e.what()));
            return false;
        } catch (...) {
            LOG_ERROR("Unknown exception while initializing Resource Manager");
            return false;
        }
    }

    void ResourceManager::Shutdown() {
        LOG_INFO("Shutting down Resource Manager...");
        
        try {
            // Shutdown performance optimization components
            if (m_gpuUploadOptimizer) {
                m_gpuUploadOptimizer->Shutdown();
            }
            
            if (m_memoryPool) {
                m_memoryPool->Clear();
            }
            
            if (m_lruCache) {
                m_lruCache->Clear();
            }
            
            UnloadAll();
            
            // Reset performance components
            m_memoryPool.reset();
            m_lruCache.reset();
            m_gpuUploadOptimizer.reset();
            
            // Log comprehensive statistics
            std::stringstream ss;
            ss << "Resource Manager shutdown statistics:\n";
            ss << "  Total loads: " << m_totalLoads << "\n";
            ss << "  Cache hits: " << m_cacheHits << " (" << (m_totalLoads > 0 ? (m_cacheHits * 100.0 / m_totalLoads) : 0.0) << "%)\n";
            ss << "  Cache misses: " << m_cacheMisses << "\n";
            ss << "  Load failures: " << m_loadFailureCount << "\n";
            ss << "  Fallback resources created: " << m_fallbackResourcesCreated << "\n";
            ss << "  LRU cleanups: " << m_lruCleanups << "\n";
            ss << "  Memory pressure events: " << m_memoryPressureEvents;
            
            LOG_INFO(ss.str());
            LOG_INFO("Resource Manager shutdown completed");
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during Resource Manager shutdown: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("Unknown exception during Resource Manager shutdown");
        }
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
        
        // Throttle memory pressure checks to avoid excessive overhead
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastCheck = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastMemoryPressureCheck);
        if (timeSinceLastCheck.count() < 5) { // Check at most every 5 seconds
            return;
        }
        m_lastMemoryPressureCheck = now;
        
        try {
            size_t currentMemoryUsage = GetMemoryUsage();
            
            if (currentMemoryUsage > m_memoryPressureThreshold) {
                LOG_WARNING("Memory pressure detected: " + std::to_string(currentMemoryUsage / 1024 / 1024) + 
                           " MB > " + std::to_string(m_memoryPressureThreshold / 1024 / 1024) + " MB threshold");
                
                HandleMemoryPressure();
            } else {
                LOG_DEBUG("Memory usage within limits: " + std::to_string(currentMemoryUsage / 1024 / 1024) + " MB");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while checking memory pressure: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("Unknown exception while checking memory pressure");
        }
    }

    void ResourceManager::HandleMemoryPressure() {
        ++m_memoryPressureEvents;
        LOG_WARNING("Handling memory pressure event #" + std::to_string(m_memoryPressureEvents));
        
        try {
            size_t currentMemoryUsage = GetMemoryUsage();
            
            // First, clean up expired references
            UnloadUnused();
            
            size_t memoryAfterCleanup = GetMemoryUsage();
            size_t freedByCleanup = currentMemoryUsage - memoryAfterCleanup;
            
            if (freedByCleanup > 0) {
                LOG_INFO("Freed " + std::to_string(freedByCleanup / 1024 / 1024) + " MB by cleaning up expired references");
            }
            
            // If still over threshold, use LRU cleanup
            if (memoryAfterCleanup > m_memoryPressureThreshold) {
                LOG_INFO("Still over threshold after cleanup, initiating LRU cleanup");
                
                // Try to free 30% of current memory usage
                size_t targetReduction = memoryAfterCleanup * 30 / 100;
                UnloadLeastRecentlyUsed(targetReduction);
                
                size_t finalMemoryUsage = GetMemoryUsage();
                size_t totalFreed = currentMemoryUsage - finalMemoryUsage;
                
                LOG_INFO("Memory pressure handling completed. Freed " + 
                        std::to_string(totalFreed / 1024 / 1024) + " MB total");
                
                if (finalMemoryUsage > m_memoryPressureThreshold) {
                    LOG_WARNING("Memory usage still above threshold after cleanup: " + 
                               std::to_string(finalMemoryUsage / 1024 / 1024) + " MB");
                    
                    // Consider more aggressive measures if this happens frequently
                    if (m_memoryPressureEvents > 10) {
                        LOG_WARNING("Frequent memory pressure events detected. Consider increasing threshold or reducing resource usage.");
                    }
                }
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while handling memory pressure: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("Unknown exception while handling memory pressure");
        }
    }

    void ResourceManager::HandleResourceLoadFailure(const std::string& path, const std::string& error) {
        ++m_loadFailureCount;
        
        LOG_ERROR("Resource load failure #" + std::to_string(m_loadFailureCount) + 
                 " for '" + path + "': " + error);
        
        // Log additional context for debugging
        LOG_DEBUG("Resource load failure context:");
        LOG_DEBUG("  Full path attempted: " + m_assetDirectory + path);
        LOG_DEBUG("  Assets directory exists: " + std::string(std::filesystem::exists(m_assetDirectory) ? "yes" : "no"));
        LOG_DEBUG("  Current memory usage: " + std::to_string(GetMemoryUsage() / 1024 / 1024) + " MB");
        LOG_DEBUG("  Total resources loaded: " + std::to_string(GetResourceCount()));
        
        // Check if file exists and provide helpful error messages
        std::string fullPath = m_assetDirectory + path;
        if (!std::filesystem::exists(fullPath)) {
            LOG_ERROR("File does not exist: " + fullPath);
            
            // Suggest similar files if any exist
            try {
                std::string directory = std::filesystem::path(fullPath).parent_path().string();
                std::string filename = std::filesystem::path(fullPath).filename().string();
                
                if (std::filesystem::exists(directory)) {
                    LOG_INFO("Files in directory " + directory + ":");
                    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                        if (entry.is_regular_file()) {
                            LOG_INFO("  - " + entry.path().filename().string());
                        }
                    }
                }
            } catch (const std::exception& e) {
                LOG_DEBUG("Could not list directory contents: " + std::string(e.what()));
            }
        } else {
            LOG_ERROR("File exists but could not be loaded: " + fullPath);
            
            // Check file permissions and size
            try {
                auto fileSize = std::filesystem::file_size(fullPath);
                LOG_DEBUG("File size: " + std::to_string(fileSize) + " bytes");
                
                if (fileSize == 0) {
                    LOG_ERROR("File is empty: " + fullPath);
                } else if (fileSize > 100 * 1024 * 1024) { // 100MB
                    LOG_WARNING("File is very large: " + std::to_string(fileSize / 1024 / 1024) + " MB");
                }
            } catch (const std::exception& e) {
                LOG_DEBUG("Could not get file information: " + std::string(e.what()));
            }
        }
        
        // If we have too many failures, suggest checking the asset directory
        if (m_loadFailureCount > 5 && m_loadFailureCount % 5 == 0) {
            LOG_WARNING("Multiple resource load failures detected (" + std::to_string(m_loadFailureCount) + 
                       " total). Please verify:");
            LOG_WARNING("  1. Asset files exist in: " + m_assetDirectory);
            LOG_WARNING("  2. File paths are correct and case-sensitive");
            LOG_WARNING("  3. File formats are supported");
            LOG_WARNING("  4. Sufficient disk space and memory available");
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

    // Performance optimization methods
    void ResourceManager::EnableMemoryPooling(bool enabled) {
        m_memoryPoolingEnabled = enabled;
        LOG_INFO("ResourceManager memory pooling " + std::string(enabled ? "enabled" : "disabled"));
        
        if (enabled && m_memoryPool) {
            m_memoryPool->EnablePooling(true);
        } else if (m_memoryPool) {
            m_memoryPool->EnablePooling(false);
        }
    }

    void ResourceManager::EnableLRUCache(bool enabled) {
        m_lruCacheEnabled = enabled;
        LOG_INFO("ResourceManager LRU caching " + std::string(enabled ? "enabled" : "disabled"));
        
        if (!enabled && m_lruCache) {
            m_lruCache->Clear();
        }
    }

    void ResourceManager::EnableGPUUploadOptimization(bool enabled) {
        m_gpuUploadOptimizationEnabled = enabled;
        LOG_INFO("ResourceManager GPU upload optimization " + std::string(enabled ? "enabled" : "disabled"));
        
        if (enabled && m_gpuUploadOptimizer) {
            m_gpuUploadOptimizer->EnableAsyncUploads(true);
        } else if (m_gpuUploadOptimizer) {
            m_gpuUploadOptimizer->EnableAsyncUploads(false);
        }
    }

    void ResourceManager::SetMemoryPoolSize(size_t poolSize) {
        if (m_memoryPool) {
            m_memoryPool->SetPoolSize(poolSize);
            LOG_INFO("ResourceManager memory pool size set to " + std::to_string(poolSize / 1024 / 1024) + " MB");
        }
    }

    void ResourceManager::SetLRUCacheSize(size_t maxSize, size_t maxMemory) {
        if (m_lruCache) {
            m_lruCache->SetMaxSize(maxSize);
            m_lruCache->SetMaxMemory(maxMemory);
            LOG_INFO("ResourceManager LRU cache size set to " + std::to_string(maxSize) + 
                    " resources, " + std::to_string(maxMemory / 1024 / 1024) + " MB");
        }
    }

    void ResourceManager::SetGPUUploadBandwidth(size_t bytesPerSecond) {
        if (m_gpuUploadOptimizer) {
            m_gpuUploadOptimizer->SetMaxUploadBandwidth(bytesPerSecond);
            LOG_INFO("ResourceManager GPU upload bandwidth set to " + 
                    std::to_string(bytesPerSecond / 1024 / 1024) + " MB/s");
        }
    }

    float ResourceManager::GetLRUCacheHitRatio() const {
        if (m_lruCache) {
            return m_lruCache->GetHitRatio();
        }
        return 0.0f;
    }

    float ResourceManager::GetMemoryPoolUtilization() const {
        if (m_memoryPool) {
            return m_memoryPool->GetUtilization();
        }
        return 0.0f;
    }

    size_t ResourceManager::GetGPUUploadQueueSize() const {
        if (m_gpuUploadOptimizer) {
            return m_gpuUploadOptimizer->GetPendingUploadCount();
        }
        return 0;
    }
}