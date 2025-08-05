#pragma once

#include "Core/Math.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderVariant.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <chrono>
#include <filesystem>

namespace GameEngine {
    struct ShaderCacheEntry {
        std::string shaderName;
        std::string sourceHash;
        std::string variantHash;
        std::shared_ptr<Shader> shader;
        std::chrono::system_clock::time_point creationTime;
        std::chrono::system_clock::time_point lastAccessTime;
        size_t accessCount = 0;
        size_t memoryUsage = 0;
        bool isPersistent = false;
        
        ShaderCacheEntry() = default;
        ShaderCacheEntry(const std::string& name, const std::string& srcHash, 
                        const std::string& varHash, std::shared_ptr<Shader> shdr)
            : shaderName(name), sourceHash(srcHash), variantHash(varHash), shader(shdr),
              creationTime(std::chrono::system_clock::now()),
              lastAccessTime(std::chrono::system_clock::now()) {}
    };

    struct ShaderCacheStats {
        size_t totalEntries = 0;
        size_t hitCount = 0;
        size_t missCount = 0;
        size_t evictionCount = 0;
        size_t totalMemoryUsage = 0;
        size_t maxMemoryUsage = 0;
        float hitRatio = 0.0f;
        size_t persistentEntries = 0;
        size_t temporaryEntries = 0;
        
        void UpdateHitRatio() {
            size_t totalAccess = hitCount + missCount;
            hitRatio = totalAccess > 0 ? static_cast<float>(hitCount) / totalAccess : 0.0f;
        }
        
        void Reset() {
            totalEntries = 0;
            hitCount = 0;
            missCount = 0;
            evictionCount = 0;
            totalMemoryUsage = 0;
            persistentEntries = 0;
            temporaryEntries = 0;
            hitRatio = 0.0f;
        }
    };

    enum class CacheEvictionPolicy {
        LRU,        // Least Recently Used
        LFU,        // Least Frequently Used
        FIFO,       // First In, First Out
        TimeBasedLRU // LRU with time-based expiration
    };

    struct ShaderCacheConfig {
        size_t maxEntries = 1000;
        size_t maxMemoryUsage = 256 * 1024 * 1024; // 256MB
        CacheEvictionPolicy evictionPolicy = CacheEvictionPolicy::LRU;
        bool enablePersistentCache = true;
        std::string cacheDirectory = "cache/shaders";
        bool enablePrecompilation = true;
        bool enableVariantCaching = true;
        std::chrono::minutes entryExpirationTime = std::chrono::minutes(60);
        bool enableStatistics = true;
        bool enableCompression = false;
    };

    class ShaderCache {
    public:
        // Lifecycle
        ShaderCache();
        ~ShaderCache();
        
        bool Initialize(const ShaderCacheConfig& config = ShaderCacheConfig{});
        void Shutdown();

        // Cache operations
        std::shared_ptr<Shader> GetShader(const std::string& name, const std::string& sourceHash = "");
        std::shared_ptr<Shader> GetShaderVariant(const std::string& baseName, const ShaderVariant& variant, 
                                                 const std::string& sourceHash = "");
        
        void StoreShader(const std::string& name, std::shared_ptr<Shader> shader, 
                        const std::string& sourceHash = "", bool persistent = false);
        void StoreShaderVariant(const std::string& baseName, const ShaderVariant& variant,
                               std::shared_ptr<Shader> shader, const std::string& sourceHash = "",
                               bool persistent = false);
        
        bool HasShader(const std::string& name, const std::string& sourceHash = "") const;
        bool HasShaderVariant(const std::string& baseName, const ShaderVariant& variant,
                             const std::string& sourceHash = "") const;
        
        void RemoveShader(const std::string& name);
        void RemoveShaderVariant(const std::string& baseName, const ShaderVariant& variant);
        void RemoveAllVariants(const std::string& baseName);

        // Cache management
        void ClearCache();
        void ClearTemporaryEntries();
        void InvalidateShader(const std::string& name);
        void InvalidateAllShaders();
        
        // Eviction and cleanup
        void EvictLeastRecentlyUsed(size_t count = 1);
        void EvictLeastFrequentlyUsed(size_t count = 1);
        void EvictExpiredEntries();
        void PerformMaintenance();
        
        // Precompilation system
        void PrecompileShaders(const std::vector<std::string>& shaderNames);
        void PrecompileShaderVariants(const std::string& baseName, 
                                     const std::vector<ShaderVariant>& variants);
        void SetPrecompilationCallback(std::function<std::shared_ptr<Shader>(const std::string&)> callback);
        void SetVariantPrecompilationCallback(std::function<std::shared_ptr<Shader>(const std::string&, const ShaderVariant&)> callback);
        
        // Persistent cache operations
        bool SaveCacheToDisk();
        bool LoadCacheFromDisk();
        void SetPersistentCacheEnabled(bool enabled) { m_config.enablePersistentCache = enabled; }
        bool IsPersistentCacheEnabled() const { return m_config.enablePersistentCache; }
        
        // Statistics and monitoring
        ShaderCacheStats GetStats() const { return m_stats; }
        void ResetStats();
        std::vector<std::string> GetCachedShaderNames() const;
        std::vector<ShaderVariant> GetCachedVariants(const std::string& baseName) const;
        size_t GetEntryCount() const { return m_cache.size(); }
        size_t GetMemoryUsage() const { return m_stats.totalMemoryUsage; }
        
        // Configuration
        void SetConfig(const ShaderCacheConfig& config);
        ShaderCacheConfig GetConfig() const { return m_config; }
        void SetMaxEntries(size_t maxEntries) { m_config.maxEntries = maxEntries; }
        void SetMaxMemoryUsage(size_t maxMemory) { m_config.maxMemoryUsage = maxMemory; }
        void SetEvictionPolicy(CacheEvictionPolicy policy) { m_config.evictionPolicy = policy; }
        
        // Debug and diagnostics
        void SetDebugMode(bool enabled) { m_debugMode = enabled; }
        bool IsDebugMode() const { return m_debugMode; }
        void PrintCacheInfo() const;
        void ValidateCacheIntegrity() const;

    private:
        // Internal cache key generation
        std::string GenerateCacheKey(const std::string& name, const std::string& sourceHash = "") const;
        std::string GenerateVariantCacheKey(const std::string& baseName, const ShaderVariant& variant,
                                           const std::string& sourceHash = "") const;
        
        // Hash generation
        std::string GenerateSourceHash(const std::string& source) const;
        std::string GenerateVariantHash(const ShaderVariant& variant) const;
        
        // Eviction policy implementations
        void EvictByLRU(size_t count);
        void EvictByLFU(size_t count);
        void EvictByFIFO(size_t count);
        void EvictByTimeBasedLRU(size_t count);
        
        // Memory management
        void UpdateMemoryUsage();
        void EnforceMemoryLimit();
        void EnforceEntryLimit();
        size_t EstimateShaderMemoryUsage(std::shared_ptr<Shader> shader) const;
        
        // Persistent cache file operations
        std::string GetCacheFilePath(const std::string& cacheKey) const;
        bool SaveShaderToDisk(const std::string& cacheKey, const ShaderCacheEntry& entry);
        bool LoadShaderFromDisk(const std::string& cacheKey, ShaderCacheEntry& entry);
        void CleanupDiskCache();
        
        // Utility methods
        void UpdateAccessInfo(ShaderCacheEntry& entry);
        bool IsEntryExpired(const ShaderCacheEntry& entry) const;
        void LogCacheOperation(const std::string& operation, const std::string& key, bool hit = false) const;
        
        // Member variables
        bool m_initialized = false;
        bool m_debugMode = false;
        
        ShaderCacheConfig m_config;
        std::unordered_map<std::string, ShaderCacheEntry> m_cache;
        mutable ShaderCacheStats m_stats;
        
        // Precompilation callbacks
        std::function<std::shared_ptr<Shader>(const std::string&)> m_precompileCallback;
        std::function<std::shared_ptr<Shader>(const std::string&, const ShaderVariant&)> m_variantPrecompileCallback;
        
        // Maintenance timing
        std::chrono::system_clock::time_point m_lastMaintenanceTime;
        std::chrono::minutes m_maintenanceInterval = std::chrono::minutes(5);
    };
}