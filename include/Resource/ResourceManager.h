#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <mutex>
#include <chrono>
#include "Core/Logger.h"

namespace GameEngine {
    class ResourceMemoryPool;
    class GPUUploadOptimizer;
    template<typename T> class LRUResourceCache;
    
    class Resource {
    public:
        Resource(const std::string& path) : m_path(path), m_loadTime(std::chrono::steady_clock::now()) {}
        virtual ~Resource() = default;
        
        const std::string& GetPath() const { return m_path; }
        virtual size_t GetMemoryUsage() const { return sizeof(*this); }
        virtual bool LoadFromFile(const std::string& filepath) { return true; } // Default implementation
        
        std::chrono::steady_clock::time_point GetLoadTime() const { return m_loadTime; }
        std::chrono::steady_clock::time_point GetLastAccessTime() const { return m_lastAccessTime; }
        void UpdateLastAccessTime() const { m_lastAccessTime = std::chrono::steady_clock::now(); }
        
    protected:
        std::string m_path;
        std::chrono::steady_clock::time_point m_loadTime;
        mutable std::chrono::steady_clock::time_point m_lastAccessTime;
    };

    struct ResourceStats {
        size_t totalResources = 0;
        size_t totalMemoryUsage = 0;
        size_t expiredReferences = 0;
        std::unordered_map<std::string, size_t> resourcesByType;
        std::unordered_map<std::string, size_t> memoryByType;
    };

    class ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        bool Initialize();
        void Shutdown();

        // Error handling and recovery
        void SetFallbackResourcesEnabled(bool enabled) { m_fallbackResourcesEnabled = enabled; }
        bool IsFallbackResourcesEnabled() const { return m_fallbackResourcesEnabled; }
        void HandleMemoryPressure();
        void HandleResourceLoadFailure(const std::string& path, const std::string& error);

        template<typename T>
        std::shared_ptr<T> Load(const std::string& path);

        template<typename T>
        void Unload(const std::string& path);

        void UnloadAll();
        void UnloadUnused();
        
        // Memory management
        void UnloadLeastRecentlyUsed(size_t targetMemoryReduction = 0);
        void SetMemoryPressureThreshold(size_t thresholdBytes);
        void CheckMemoryPressure();
        
        // Statistics and debugging
        size_t GetMemoryUsage() const;
        size_t GetResourceCount() const;
        ResourceStats GetResourceStats() const;
        void LogResourceUsage() const;
        void LogDetailedResourceInfo() const;
        
        // Asset pipeline functions
        bool ImportAsset(const std::string& sourcePath, const std::string& targetPath);
        bool ExportAsset(const std::string& assetPath, const std::string& exportPath);
        
        // Performance optimization controls
        void EnableMemoryPooling(bool enabled);
        void EnableLRUCache(bool enabled);
        void EnableGPUUploadOptimization(bool enabled);
        void SetMemoryPoolSize(size_t poolSize);
        void SetLRUCacheSize(size_t maxSize, size_t maxMemory);
        void SetGPUUploadBandwidth(size_t bytesPerSecond);
        
        // Performance statistics
        float GetLRUCacheHitRatio() const;
        float GetMemoryPoolUtilization() const;
        size_t GetGPUUploadQueueSize() const;

    private:
        template<typename T>
        std::string GetResourceKey(const std::string& path);
        
        template<typename T>
        std::shared_ptr<T> CreateResource(const std::string& path);

        mutable std::mutex m_resourcesMutex;
        std::unordered_map<std::string, std::weak_ptr<Resource>> m_resources;
        std::string m_assetDirectory = "assets/";
        
        // Performance optimization components
        std::unique_ptr<ResourceMemoryPool> m_memoryPool;
        std::unique_ptr<LRUResourceCache<Resource>> m_lruCache;
        std::unique_ptr<GPUUploadOptimizer> m_gpuUploadOptimizer;
        
        // Performance settings
        bool m_memoryPoolingEnabled = true;
        bool m_lruCacheEnabled = true;
        bool m_gpuUploadOptimizationEnabled = true;
        
        // Memory management
        size_t m_memoryPressureThreshold = 512 * 1024 * 1024; // 512 MB default
        bool m_autoMemoryManagement = true;
        
        // Error handling
        bool m_fallbackResourcesEnabled = true;
        size_t m_loadFailureCount = 0;
        size_t m_memoryPressureEvents = 0;
        std::chrono::steady_clock::time_point m_lastMemoryPressureCheck;
        
        // Statistics tracking
        mutable size_t m_totalLoads = 0;
        mutable size_t m_cacheHits = 0;
        mutable size_t m_cacheMisses = 0;
        mutable size_t m_lruCleanups = 0;
        mutable size_t m_fallbackResourcesCreated = 0;
    };

    template<typename T>
    std::shared_ptr<T> ResourceManager::Load(const std::string& path) {
        // Check memory pressure before acquiring lock (every 10 loads)
        if (m_autoMemoryManagement && (m_totalLoads % 10 == 0)) {
            CheckMemoryPressure();
        }
        
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        std::string key = GetResourceKey<T>(path);
        
        ++m_totalLoads;
        
        // Check if resource exists and is still valid
        auto it = m_resources.find(key);
        if (it != m_resources.end()) {
            if (auto resource = it->second.lock()) {
                ++m_cacheHits;
                resource->UpdateLastAccessTime();
                LOG_INFO("Resource cache hit: " + path + " (" + std::to_string(resource->GetMemoryUsage() / 1024) + " KB)");
                return std::static_pointer_cast<T>(resource);
            } else {
                // Weak pointer expired, remove it
                m_resources.erase(it);
            }
        }

        ++m_cacheMisses;
        
        // Create new resource
        auto resource = CreateResource<T>(path);
        if (resource) {
            m_resources[key] = resource;
            LOG_INFO("Resource loaded: " + path + " (" + std::to_string(resource->GetMemoryUsage() / 1024) + " KB)");
        }
        
        return resource;
    }

    template<typename T>
    void ResourceManager::Unload(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_resourcesMutex);
        std::string key = GetResourceKey<T>(path);
        
        auto it = m_resources.find(key);
        if (it != m_resources.end()) {
            if (auto resource = it->second.lock()) {
                LOG_INFO("Resource unloaded: " + path + " (" + std::to_string(resource->GetMemoryUsage() / 1024) + " KB)");
            }
            m_resources.erase(it);
        }
    }

    template<typename T>
    std::string ResourceManager::GetResourceKey(const std::string& path) {
        return std::string(typeid(T).name()) + ":" + path;
    }
    
    template<typename T>
    std::shared_ptr<T> ResourceManager::CreateResource(const std::string& path) {
        // Avoid duplicating the assets/ prefix if it's already in the path
        std::string fullPath;
        if (path.find(m_assetDirectory) == 0) {
            // Path already starts with assets/, use it as-is
            fullPath = path;
        } else {
            // Path doesn't start with assets/, prepend it
            fullPath = m_assetDirectory + path;
        }
        LOG_DEBUG("Creating resource: " + path + " (full path: " + fullPath + ")");
        
        try {
            auto resource = std::make_shared<T>(fullPath);
            
            // Try to load the resource if it has a LoadFromFile method
            if constexpr (requires { resource->LoadFromFile(std::string{}); }) {
                bool loadSuccess = false;
                
                try {
                    loadSuccess = resource->LoadFromFile(fullPath);
                } catch (const std::exception& e) {
                    LOG_ERROR("Exception while loading resource '" + path + "': " + e.what());
                    HandleResourceLoadFailure(path, "Exception: " + std::string(e.what()));
                    loadSuccess = false;
                } catch (...) {
                    LOG_ERROR("Unknown exception while loading resource: " + path);
                    HandleResourceLoadFailure(path, "Unknown exception");
                    loadSuccess = false;
                }
                
                if (!loadSuccess) {
                    LOG_WARNING("Failed to load resource from file: " + path);
                    
                    // If loading fails, try to create a default/fallback resource
                    if (m_fallbackResourcesEnabled) {
                        if constexpr (requires { resource->CreateDefault(); }) {
                            try {
                                LOG_INFO("Creating fallback resource for: " + path);
                                resource->CreateDefault();
                                ++m_fallbackResourcesCreated;
                                LOG_INFO("Successfully created fallback resource for: " + path);
                            } catch (const std::exception& e) {
                                LOG_ERROR("Exception while creating fallback resource for '" + path + "': " + e.what());
                                return nullptr;
                            } catch (...) {
                                LOG_ERROR("Unknown exception while creating fallback resource for: " + path);
                                return nullptr;
                            }
                        } else {
                            LOG_ERROR("Resource type does not support fallback creation: " + path);
                            return nullptr;
                        }
                    } else {
                        LOG_ERROR("Fallback resources disabled, returning null for: " + path);
                        return nullptr;
                    }
                }
            }
            
            LOG_INFO("Successfully created resource: " + path + " (memory: " + 
                    std::to_string(resource->GetMemoryUsage() / 1024) + " KB)");
            return resource;
            
        } catch (const std::bad_alloc& e) {
            LOG_ERROR("Memory allocation failed while creating resource '" + path + "': " + e.what());
            HandleMemoryPressure();
            return nullptr;
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while creating resource '" + path + "': " + e.what());
            HandleResourceLoadFailure(path, "Creation exception: " + std::string(e.what()));
            return nullptr;
        } catch (...) {
            LOG_ERROR("Unknown exception while creating resource: " + path);
            HandleResourceLoadFailure(path, "Unknown creation exception");
            return nullptr;
        }
    }
}