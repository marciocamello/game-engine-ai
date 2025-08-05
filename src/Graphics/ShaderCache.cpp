#include "Graphics/ShaderCache.h"
#include "Core/Logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <functional>

namespace GameEngine {
    ShaderCache::ShaderCache() {
        m_lastMaintenanceTime = std::chrono::system_clock::now();
    }

    ShaderCache::~ShaderCache() {
        Shutdown();
    }

    bool ShaderCache::Initialize(const ShaderCacheConfig& config) {
        if (m_initialized) {
            LOG_WARNING("ShaderCache already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderCache");
        
        m_config = config;
        m_cache.clear();
        m_stats.Reset();
        
        // Create cache directory if it doesn't exist
        if (m_config.enablePersistentCache) {
            try {
                std::filesystem::create_directories(m_config.cacheDirectory);
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to create cache directory: " + std::string(e.what()));
                m_config.enablePersistentCache = false;
            }
        }
        
        // Load existing cache from disk if enabled
        if (m_config.enablePersistentCache) {
            LoadCacheFromDisk();
        }
        
        m_initialized = true;
        
        if (m_debugMode) {
            LOG_INFO("ShaderCache initialized with max entries: " + std::to_string(m_config.maxEntries) +
                    ", max memory: " + std::to_string(m_config.maxMemoryUsage / (1024 * 1024)) + "MB");
        }
        
        return true;
    }

    void ShaderCache::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down ShaderCache");
        
        // Save cache to disk if enabled
        if (m_config.enablePersistentCache) {
            SaveCacheToDisk();
        }
        
        m_cache.clear();
        m_stats.Reset();
        m_precompileCallback = nullptr;
        m_variantPrecompileCallback = nullptr;
        
        m_initialized = false;
    }

    std::shared_ptr<Shader> ShaderCache::GetShader(const std::string& name, const std::string& sourceHash) {
        if (!m_initialized) {
            LOG_ERROR("ShaderCache not initialized");
            return nullptr;
        }

        std::string cacheKey = GenerateCacheKey(name, sourceHash);
        
        auto it = m_cache.find(cacheKey);
        if (it != m_cache.end()) {
            // Cache hit
            UpdateAccessInfo(it->second);
            m_stats.hitCount++;
            m_stats.UpdateHitRatio();
            
            LogCacheOperation("GET", cacheKey, true);
            
            return it->second.shader;
        }
        
        // Cache miss
        m_stats.missCount++;
        m_stats.UpdateHitRatio();
        
        LogCacheOperation("GET", cacheKey, false);
        
        return nullptr;
    }

    std::shared_ptr<Shader> ShaderCache::GetShaderVariant(const std::string& baseName, const ShaderVariant& variant,
                                                          const std::string& sourceHash) {
        if (!m_initialized || !m_config.enableVariantCaching) {
            return nullptr;
        }

        std::string cacheKey = GenerateVariantCacheKey(baseName, variant, sourceHash);
        
        auto it = m_cache.find(cacheKey);
        if (it != m_cache.end()) {
            // Cache hit
            UpdateAccessInfo(it->second);
            m_stats.hitCount++;
            m_stats.UpdateHitRatio();
            
            LogCacheOperation("GET_VARIANT", cacheKey, true);
            
            return it->second.shader;
        }
        
        // Cache miss
        m_stats.missCount++;
        m_stats.UpdateHitRatio();
        
        LogCacheOperation("GET_VARIANT", cacheKey, false);
        
        return nullptr;
    }

    void ShaderCache::StoreShader(const std::string& name, std::shared_ptr<Shader> shader,
                                 const std::string& sourceHash, bool persistent) {
        if (!m_initialized || !shader) {
            return;
        }

        std::string cacheKey = GenerateCacheKey(name, sourceHash);
        
        // Check if entry already exists
        auto it = m_cache.find(cacheKey);
        if (it != m_cache.end()) {
            // Update existing entry
            it->second.shader = shader;
            it->second.lastAccessTime = std::chrono::system_clock::now();
            it->second.accessCount++;
            it->second.isPersistent = persistent;
            it->second.memoryUsage = EstimateShaderMemoryUsage(shader);
            
            LogCacheOperation("UPDATE", cacheKey);
        } else {
            // Create new entry
            ShaderCacheEntry entry(name, sourceHash, "", shader);
            entry.isPersistent = persistent;
            entry.memoryUsage = EstimateShaderMemoryUsage(shader);
            
            m_cache[cacheKey] = entry;
            m_stats.totalEntries++;
            
            if (persistent) {
                m_stats.persistentEntries++;
            } else {
                m_stats.temporaryEntries++;
            }
            
            LogCacheOperation("STORE", cacheKey);
        }
        
        UpdateMemoryUsage();
        
        // Enforce limits
        EnforceMemoryLimit();
        EnforceEntryLimit();
        
        // Save to disk if persistent and enabled
        if (persistent && m_config.enablePersistentCache) {
            SaveShaderToDisk(cacheKey, m_cache[cacheKey]);
        }
    }

    void ShaderCache::StoreShaderVariant(const std::string& baseName, const ShaderVariant& variant,
                                        std::shared_ptr<Shader> shader, const std::string& sourceHash,
                                        bool persistent) {
        if (!m_initialized || !shader || !m_config.enableVariantCaching) {
            return;
        }

        std::string cacheKey = GenerateVariantCacheKey(baseName, variant, sourceHash);
        std::string variantHash = GenerateVariantHash(variant);
        
        // Check if entry already exists
        auto it = m_cache.find(cacheKey);
        if (it != m_cache.end()) {
            // Update existing entry
            it->second.shader = shader;
            it->second.lastAccessTime = std::chrono::system_clock::now();
            it->second.accessCount++;
            it->second.isPersistent = persistent;
            it->second.memoryUsage = EstimateShaderMemoryUsage(shader);
            
            LogCacheOperation("UPDATE_VARIANT", cacheKey);
        } else {
            // Create new entry
            ShaderCacheEntry entry(baseName, sourceHash, variantHash, shader);
            entry.isPersistent = persistent;
            entry.memoryUsage = EstimateShaderMemoryUsage(shader);
            
            m_cache[cacheKey] = entry;
            m_stats.totalEntries++;
            
            if (persistent) {
                m_stats.persistentEntries++;
            } else {
                m_stats.temporaryEntries++;
            }
            
            LogCacheOperation("STORE_VARIANT", cacheKey);
        }
        
        UpdateMemoryUsage();
        
        // Enforce limits
        EnforceMemoryLimit();
        EnforceEntryLimit();
        
        // Save to disk if persistent and enabled
        if (persistent && m_config.enablePersistentCache) {
            SaveShaderToDisk(cacheKey, m_cache[cacheKey]);
        }
    }

    bool ShaderCache::HasShader(const std::string& name, const std::string& sourceHash) const {
        if (!m_initialized) {
            return false;
        }

        std::string cacheKey = GenerateCacheKey(name, sourceHash);
        return m_cache.find(cacheKey) != m_cache.end();
    }

    bool ShaderCache::HasShaderVariant(const std::string& baseName, const ShaderVariant& variant,
                                      const std::string& sourceHash) const {
        if (!m_initialized || !m_config.enableVariantCaching) {
            return false;
        }

        std::string cacheKey = GenerateVariantCacheKey(baseName, variant, sourceHash);
        return m_cache.find(cacheKey) != m_cache.end();
    }

    void ShaderCache::RemoveShader(const std::string& name) {
        if (!m_initialized) {
            return;
        }

        // Remove all entries that match the shader name (different source hashes)
        std::vector<std::string> keysToRemove;
        
        for (const auto& pair : m_cache) {
            if (pair.second.shaderName == name) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& key : keysToRemove) {
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                if (it->second.isPersistent) {
                    m_stats.persistentEntries--;
                } else {
                    m_stats.temporaryEntries--;
                }
                
                m_cache.erase(it);
                m_stats.totalEntries--;
                
                LogCacheOperation("REMOVE", key);
            }
        }
        
        UpdateMemoryUsage();
    }

    void ShaderCache::RemoveShaderVariant(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_initialized) {
            return;
        }

        std::string cacheKey = GenerateVariantCacheKey(baseName, variant, "");
        
        // Find and remove entries that match the base name and variant (any source hash)
        std::vector<std::string> keysToRemove;
        std::string variantHash = GenerateVariantHash(variant);
        
        for (const auto& pair : m_cache) {
            if (pair.second.shaderName == baseName && pair.second.variantHash == variantHash) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& key : keysToRemove) {
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                if (it->second.isPersistent) {
                    m_stats.persistentEntries--;
                } else {
                    m_stats.temporaryEntries--;
                }
                
                m_cache.erase(it);
                m_stats.totalEntries--;
                
                LogCacheOperation("REMOVE_VARIANT", key);
            }
        }
        
        UpdateMemoryUsage();
    }

    void ShaderCache::RemoveAllVariants(const std::string& baseName) {
        if (!m_initialized) {
            return;
        }

        std::vector<std::string> keysToRemove;
        
        for (const auto& pair : m_cache) {
            if (pair.second.shaderName == baseName && !pair.second.variantHash.empty()) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& key : keysToRemove) {
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                if (it->second.isPersistent) {
                    m_stats.persistentEntries--;
                } else {
                    m_stats.temporaryEntries--;
                }
                
                m_cache.erase(it);
                m_stats.totalEntries--;
                
                LogCacheOperation("REMOVE_ALL_VARIANTS", key);
            }
        }
        
        UpdateMemoryUsage();
    }   
 void ShaderCache::ClearCache() {
        if (!m_initialized) {
            return;
        }

        if (m_debugMode) {
            LOG_INFO("Clearing shader cache (" + std::to_string(m_cache.size()) + " entries)");
        }
        
        m_cache.clear();
        m_stats.Reset();
        
        LogCacheOperation("CLEAR_ALL", "");
    }

    void ShaderCache::ClearTemporaryEntries() {
        if (!m_initialized) {
            return;
        }

        std::vector<std::string> keysToRemove;
        
        for (const auto& pair : m_cache) {
            if (!pair.second.isPersistent) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& key : keysToRemove) {
            m_cache.erase(key);
            m_stats.totalEntries--;
            m_stats.temporaryEntries--;
        }
        
        UpdateMemoryUsage();
        
        if (m_debugMode) {
            LOG_INFO("Cleared " + std::to_string(keysToRemove.size()) + " temporary cache entries");
        }
    }

    void ShaderCache::InvalidateShader(const std::string& name) {
        RemoveShader(name);
        
        if (m_debugMode) {
            LOG_INFO("Invalidated shader: " + name);
        }
    }

    void ShaderCache::InvalidateAllShaders() {
        ClearCache();
        
        if (m_debugMode) {
            LOG_INFO("Invalidated all shaders");
        }
    }

    void ShaderCache::EvictLeastRecentlyUsed(size_t count) {
        if (!m_initialized || count == 0) {
            return;
        }

        EvictByLRU(count);
    }

    void ShaderCache::EvictLeastFrequentlyUsed(size_t count) {
        if (!m_initialized || count == 0) {
            return;
        }

        EvictByLFU(count);
    }

    void ShaderCache::EvictExpiredEntries() {
        if (!m_initialized) {
            return;
        }

        std::vector<std::string> keysToRemove;
        
        for (const auto& pair : m_cache) {
            if (IsEntryExpired(pair.second)) {
                keysToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& key : keysToRemove) {
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                if (it->second.isPersistent) {
                    m_stats.persistentEntries--;
                } else {
                    m_stats.temporaryEntries--;
                }
                
                m_cache.erase(it);
                m_stats.totalEntries--;
                m_stats.evictionCount++;
            }
        }
        
        UpdateMemoryUsage();
        
        if (m_debugMode && !keysToRemove.empty()) {
            LOG_INFO("Evicted " + std::to_string(keysToRemove.size()) + " expired cache entries");
        }
    }

    void ShaderCache::PerformMaintenance() {
        if (!m_initialized) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        if (now - m_lastMaintenanceTime < m_maintenanceInterval) {
            return;
        }
        
        if (m_debugMode) {
            LOG_INFO("Performing shader cache maintenance");
        }
        
        // Evict expired entries
        EvictExpiredEntries();
        
        // Enforce memory and entry limits
        EnforceMemoryLimit();
        EnforceEntryLimit();
        
        // Update memory usage statistics
        UpdateMemoryUsage();
        
        // Clean up disk cache if enabled
        if (m_config.enablePersistentCache) {
            CleanupDiskCache();
        }
        
        m_lastMaintenanceTime = now;
    }

    void ShaderCache::PrecompileShaders(const std::vector<std::string>& shaderNames) {
        if (!m_initialized || !m_config.enablePrecompilation || !m_precompileCallback) {
            return;
        }

        if (m_debugMode) {
            LOG_INFO("Precompiling " + std::to_string(shaderNames.size()) + " shaders");
        }
        
        for (const std::string& shaderName : shaderNames) {
            // Check if already cached
            if (HasShader(shaderName)) {
                continue;
            }
            
            // Compile shader using callback
            auto shader = m_precompileCallback(shaderName);
            if (shader) {
                StoreShader(shaderName, shader, "", true); // Store as persistent
                
                if (m_debugMode) {
                    LOG_INFO("Precompiled shader: " + shaderName);
                }
            } else {
                LOG_WARNING("Failed to precompile shader: " + shaderName);
            }
        }
    }

    void ShaderCache::PrecompileShaderVariants(const std::string& baseName,
                                              const std::vector<ShaderVariant>& variants) {
        if (!m_initialized || !m_config.enablePrecompilation || !m_variantPrecompileCallback) {
            return;
        }

        if (m_debugMode) {
            LOG_INFO("Precompiling " + std::to_string(variants.size()) + " variants for shader: " + baseName);
        }
        
        for (const ShaderVariant& variant : variants) {
            // Check if already cached
            if (HasShaderVariant(baseName, variant)) {
                continue;
            }
            
            // Compile variant using callback
            auto shader = m_variantPrecompileCallback(baseName, variant);
            if (shader) {
                StoreShaderVariant(baseName, variant, shader, "", true); // Store as persistent
                
                if (m_debugMode) {
                    LOG_INFO("Precompiled variant for shader: " + baseName);
                }
            } else {
                LOG_WARNING("Failed to precompile variant for shader: " + baseName);
            }
        }
    }

    void ShaderCache::SetPrecompilationCallback(std::function<std::shared_ptr<Shader>(const std::string&)> callback) {
        m_precompileCallback = callback;
    }

    void ShaderCache::SetVariantPrecompilationCallback(std::function<std::shared_ptr<Shader>(const std::string&, const ShaderVariant&)> callback) {
        m_variantPrecompileCallback = callback;
    }

    bool ShaderCache::SaveCacheToDisk() {
        if (!m_initialized || !m_config.enablePersistentCache) {
            return false;
        }

        if (m_debugMode) {
            LOG_INFO("Saving shader cache to disk");
        }
        
        size_t savedCount = 0;
        
        for (const auto& pair : m_cache) {
            if (pair.second.isPersistent) {
                if (SaveShaderToDisk(pair.first, pair.second)) {
                    savedCount++;
                }
            }
        }
        
        if (m_debugMode) {
            LOG_INFO("Saved " + std::to_string(savedCount) + " persistent cache entries to disk");
        }
        
        return savedCount > 0;
    }

    bool ShaderCache::LoadCacheFromDisk() {
        if (!m_initialized || !m_config.enablePersistentCache) {
            return false;
        }

        if (!std::filesystem::exists(m_config.cacheDirectory)) {
            return false;
        }
        
        if (m_debugMode) {
            LOG_INFO("Loading shader cache from disk");
        }
        
        size_t loadedCount = 0;
        
        try {
            for (const auto& entry : std::filesystem::directory_iterator(m_config.cacheDirectory)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cache") {
                    std::string cacheKey = entry.path().stem().string();
                    ShaderCacheEntry cacheEntry;
                    
                    if (LoadShaderFromDisk(cacheKey, cacheEntry)) {
                        m_cache[cacheKey] = cacheEntry;
                        m_stats.totalEntries++;
                        m_stats.persistentEntries++;
                        loadedCount++;
                    }
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error loading cache from disk: " + std::string(e.what()));
            return false;
        }
        
        UpdateMemoryUsage();
        
        if (m_debugMode) {
            LOG_INFO("Loaded " + std::to_string(loadedCount) + " cache entries from disk");
        }
        
        return loadedCount > 0;
    }

    void ShaderCache::ResetStats() {
        m_stats.Reset();
    }

    std::vector<std::string> ShaderCache::GetCachedShaderNames() const {
        std::vector<std::string> names;
        
        for (const auto& pair : m_cache) {
            const std::string& shaderName = pair.second.shaderName;
            if (std::find(names.begin(), names.end(), shaderName) == names.end()) {
                names.push_back(shaderName);
            }
        }
        
        return names;
    }

    std::vector<ShaderVariant> ShaderCache::GetCachedVariants(const std::string& baseName) const {
        std::vector<ShaderVariant> variants;
        
        for (const auto& pair : m_cache) {
            if (pair.second.shaderName == baseName && !pair.second.variantHash.empty()) {
                // Note: We can't reconstruct the full ShaderVariant from just the hash
                // This would require storing the variant data separately
                // For now, we'll return empty variants as a placeholder
                variants.push_back(ShaderVariant{});
            }
        }
        
        return variants;
    }

    void ShaderCache::SetConfig(const ShaderCacheConfig& config) {
        m_config = config;
        
        // Enforce new limits immediately
        if (m_initialized) {
            EnforceMemoryLimit();
            EnforceEntryLimit();
        }
    }

    void ShaderCache::PrintCacheInfo() const {
        if (!m_initialized) {
            LOG_INFO("ShaderCache not initialized");
            return;
        }

        LOG_INFO("=== Shader Cache Information ===");
        LOG_INFO("Total entries: " + std::to_string(m_stats.totalEntries));
        LOG_INFO("Persistent entries: " + std::to_string(m_stats.persistentEntries));
        LOG_INFO("Temporary entries: " + std::to_string(m_stats.temporaryEntries));
        LOG_INFO("Memory usage: " + std::to_string(m_stats.totalMemoryUsage / 1024) + " KB");
        LOG_INFO("Hit ratio: " + std::to_string(m_stats.hitRatio * 100.0f) + "%");
        LOG_INFO("Cache hits: " + std::to_string(m_stats.hitCount));
        LOG_INFO("Cache misses: " + std::to_string(m_stats.missCount));
        LOG_INFO("Evictions: " + std::to_string(m_stats.evictionCount));
        LOG_INFO("Max entries: " + std::to_string(m_config.maxEntries));
        LOG_INFO("Max memory: " + std::to_string(m_config.maxMemoryUsage / (1024 * 1024)) + " MB");
        LOG_INFO("================================");
    }

    void ShaderCache::ValidateCacheIntegrity() const {
        if (!m_initialized) {
            return;
        }

        size_t actualEntries = m_cache.size();
        size_t actualPersistent = 0;
        size_t actualTemporary = 0;
        size_t actualMemory = 0;
        
        for (const auto& pair : m_cache) {
            if (pair.second.isPersistent) {
                actualPersistent++;
            } else {
                actualTemporary++;
            }
            actualMemory += pair.second.memoryUsage;
        }
        
        bool valid = true;
        
        if (actualEntries != m_stats.totalEntries) {
            LOG_ERROR("Cache integrity error: entry count mismatch");
            valid = false;
        }
        
        if (actualPersistent != m_stats.persistentEntries) {
            LOG_ERROR("Cache integrity error: persistent entry count mismatch");
            valid = false;
        }
        
        if (actualTemporary != m_stats.temporaryEntries) {
            LOG_ERROR("Cache integrity error: temporary entry count mismatch");
            valid = false;
        }
        
        if (actualMemory != m_stats.totalMemoryUsage) {
            LOG_ERROR("Cache integrity error: memory usage mismatch");
            valid = false;
        }
        
        if (valid && m_debugMode) {
            LOG_INFO("Cache integrity validation passed");
        }
    }

    // Private implementation methods
    std::string ShaderCache::GenerateCacheKey(const std::string& name, const std::string& sourceHash) const {
        if (sourceHash.empty()) {
            return name;
        }
        return name + "_" + sourceHash;
    }

    std::string ShaderCache::GenerateVariantCacheKey(const std::string& baseName, const ShaderVariant& variant,
                                                    const std::string& sourceHash) const {
        std::string variantHash = GenerateVariantHash(variant);
        std::string key = baseName + "_variant_" + variantHash;
        
        if (!sourceHash.empty()) {
            key += "_" + sourceHash;
        }
        
        return key;
    }

    std::string ShaderCache::GenerateSourceHash(const std::string& source) const {
        // Simple hash implementation using std::hash
        std::hash<std::string> hasher;
        size_t hashValue = hasher(source);
        
        std::stringstream ss;
        ss << std::hex << hashValue;
        return ss.str();
    }

    std::string ShaderCache::GenerateVariantHash(const ShaderVariant& variant) const {
        // Generate hash from variant defines and features
        std::string variantString;
        
        // Add defines
        for (const auto& define : variant.defines) {
            variantString += define.first + "=" + define.second + ";";
        }
        
        // Add features
        for (const auto& feature : variant.features) {
            variantString += "feature:" + feature + ";";
        }
        
        return GenerateSourceHash(variantString);
    }

    void ShaderCache::EvictByLRU(size_t count) {
        if (m_cache.empty() || count == 0) {
            return;
        }

        // Create vector of cache entries sorted by last access time
        std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> entries;
        
        for (const auto& pair : m_cache) {
            if (!pair.second.isPersistent) { // Don't evict persistent entries
                entries.emplace_back(pair.first, pair.second.lastAccessTime);
            }
        }
        
        // Sort by access time (oldest first)
        std::sort(entries.begin(), entries.end(),
                 [](const auto& a, const auto& b) {
                     return a.second < b.second;
                 });
        
        // Evict the oldest entries
        size_t evictCount = std::min(count, entries.size());
        for (size_t i = 0; i < evictCount; i++) {
            auto it = m_cache.find(entries[i].first);
            if (it != m_cache.end()) {
                m_stats.temporaryEntries--;
                m_cache.erase(it);
                m_stats.totalEntries--;
                m_stats.evictionCount++;
                
                LogCacheOperation("EVICT_LRU", entries[i].first);
            }
        }
        
        UpdateMemoryUsage();
    }

    void ShaderCache::EvictByLFU(size_t count) {
        if (m_cache.empty() || count == 0) {
            return;
        }

        // Create vector of cache entries sorted by access count
        std::vector<std::pair<std::string, size_t>> entries;
        
        for (const auto& pair : m_cache) {
            if (!pair.second.isPersistent) { // Don't evict persistent entries
                entries.emplace_back(pair.first, pair.second.accessCount);
            }
        }
        
        // Sort by access count (least frequent first)
        std::sort(entries.begin(), entries.end(),
                 [](const auto& a, const auto& b) {
                     return a.second < b.second;
                 });
        
        // Evict the least frequently used entries
        size_t evictCount = std::min(count, entries.size());
        for (size_t i = 0; i < evictCount; i++) {
            auto it = m_cache.find(entries[i].first);
            if (it != m_cache.end()) {
                m_stats.temporaryEntries--;
                m_cache.erase(it);
                m_stats.totalEntries--;
                m_stats.evictionCount++;
                
                LogCacheOperation("EVICT_LFU", entries[i].first);
            }
        }
        
        UpdateMemoryUsage();
    }

    void ShaderCache::EvictByFIFO(size_t count) {
        if (m_cache.empty() || count == 0) {
            return;
        }

        // Create vector of cache entries sorted by creation time
        std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> entries;
        
        for (const auto& pair : m_cache) {
            if (!pair.second.isPersistent) { // Don't evict persistent entries
                entries.emplace_back(pair.first, pair.second.creationTime);
            }
        }
        
        // Sort by creation time (oldest first)
        std::sort(entries.begin(), entries.end(),
                 [](const auto& a, const auto& b) {
                     return a.second < b.second;
                 });
        
        // Evict the oldest entries
        size_t evictCount = std::min(count, entries.size());
        for (size_t i = 0; i < evictCount; i++) {
            auto it = m_cache.find(entries[i].first);
            if (it != m_cache.end()) {
                m_stats.temporaryEntries--;
                m_cache.erase(it);
                m_stats.totalEntries--;
                m_stats.evictionCount++;
                
                LogCacheOperation("EVICT_FIFO", entries[i].first);
            }
        }
        
        UpdateMemoryUsage();
    }

    void ShaderCache::EvictByTimeBasedLRU(size_t count) {
        if (m_cache.empty() || count == 0) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        
        // Create vector of cache entries that are expired or least recently used
        std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> entries;
        
        for (const auto& pair : m_cache) {
            if (!pair.second.isPersistent) { // Don't evict persistent entries
                // Check if entry is expired
                if (IsEntryExpired(pair.second)) {
                    entries.emplace_back(pair.first, pair.second.lastAccessTime);
                }
            }
        }
        
        // If we don't have enough expired entries, add LRU entries
        if (entries.size() < count) {
            for (const auto& pair : m_cache) {
                if (!pair.second.isPersistent && !IsEntryExpired(pair.second)) {
                    entries.emplace_back(pair.first, pair.second.lastAccessTime);
                }
            }
        }
        
        // Sort by access time (oldest first)
        std::sort(entries.begin(), entries.end(),
                 [](const auto& a, const auto& b) {
                     return a.second < b.second;
                 });
        
        // Evict entries
        size_t evictCount = std::min(count, entries.size());
        for (size_t i = 0; i < evictCount; i++) {
            auto it = m_cache.find(entries[i].first);
            if (it != m_cache.end()) {
                m_stats.temporaryEntries--;
                m_cache.erase(it);
                m_stats.totalEntries--;
                m_stats.evictionCount++;
                
                LogCacheOperation("EVICT_TIME_LRU", entries[i].first);
            }
        }
        
        UpdateMemoryUsage();
    }

    void ShaderCache::UpdateMemoryUsage() {
        size_t totalMemory = 0;
        
        for (const auto& pair : m_cache) {
            totalMemory += pair.second.memoryUsage;
        }
        
        m_stats.totalMemoryUsage = totalMemory;
        m_stats.maxMemoryUsage = std::max(m_stats.maxMemoryUsage, totalMemory);
    }

    void ShaderCache::EnforceMemoryLimit() {
        if (m_stats.totalMemoryUsage <= m_config.maxMemoryUsage) {
            return;
        }

        // Calculate how much memory we need to free
        size_t memoryToFree = m_stats.totalMemoryUsage - m_config.maxMemoryUsage;
        size_t freedMemory = 0;
        
        // Evict entries based on configured policy until we're under the limit
        while (freedMemory < memoryToFree && !m_cache.empty()) {
            size_t entriesBefore = m_cache.size();
            
            switch (m_config.evictionPolicy) {
                case CacheEvictionPolicy::LRU:
                    EvictByLRU(1);
                    break;
                case CacheEvictionPolicy::LFU:
                    EvictByLFU(1);
                    break;
                case CacheEvictionPolicy::FIFO:
                    EvictByFIFO(1);
                    break;
                case CacheEvictionPolicy::TimeBasedLRU:
                    EvictByTimeBasedLRU(1);
                    break;
            }
            
            // Check if we actually evicted something to avoid infinite loop
            if (m_cache.size() == entriesBefore) {
                break; // No more entries to evict (all are persistent)
            }
            
            UpdateMemoryUsage();
            freedMemory = (m_stats.totalMemoryUsage <= m_config.maxMemoryUsage) ? memoryToFree : 0;
        }
        
        if (m_debugMode && freedMemory > 0) {
            LOG_INFO("Enforced memory limit, freed " + std::to_string(freedMemory / 1024) + " KB");
        }
    }

    void ShaderCache::EnforceEntryLimit() {
        if (m_stats.totalEntries <= m_config.maxEntries) {
            return;
        }

        size_t entriesToEvict = m_stats.totalEntries - m_config.maxEntries;
        
        switch (m_config.evictionPolicy) {
            case CacheEvictionPolicy::LRU:
                EvictByLRU(entriesToEvict);
                break;
            case CacheEvictionPolicy::LFU:
                EvictByLFU(entriesToEvict);
                break;
            case CacheEvictionPolicy::FIFO:
                EvictByFIFO(entriesToEvict);
                break;
            case CacheEvictionPolicy::TimeBasedLRU:
                EvictByTimeBasedLRU(entriesToEvict);
                break;
        }
        
        if (m_debugMode) {
            LOG_INFO("Enforced entry limit, evicted " + std::to_string(entriesToEvict) + " entries");
        }
    }

    size_t ShaderCache::EstimateShaderMemoryUsage(std::shared_ptr<Shader> shader) const {
        if (!shader) {
            return 0;
        }

        // Rough estimate of shader memory usage
        // This is a simplified calculation - actual GPU memory usage would be more complex
        size_t baseSize = sizeof(Shader);
        size_t programSize = 1024; // Estimate for compiled program
        size_t uniformCacheSize = 256; // Estimate for uniform cache
        
        return baseSize + programSize + uniformCacheSize;
    }

    std::string ShaderCache::GetCacheFilePath(const std::string& cacheKey) const {
        return m_config.cacheDirectory + "/" + cacheKey + ".cache";
    }

    bool ShaderCache::SaveShaderToDisk(const std::string& cacheKey, const ShaderCacheEntry& entry) {
        if (!m_config.enablePersistentCache) {
            return false;
        }

        try {
            std::string filepath = GetCacheFilePath(cacheKey);
            std::ofstream file(filepath, std::ios::binary);
            
            if (!file.is_open()) {
                LOG_ERROR("Failed to open cache file for writing: " + filepath);
                return false;
            }
            
            // Write cache entry metadata (simplified serialization)
            file << entry.shaderName << "\n";
            file << entry.sourceHash << "\n";
            file << entry.variantHash << "\n";
            file << entry.accessCount << "\n";
            file << entry.memoryUsage << "\n";
            file << (entry.isPersistent ? 1 : 0) << "\n";
            
            // Note: We can't serialize the actual Shader object easily
            // In a full implementation, we would serialize the compiled bytecode
            
            file.close();
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Error saving cache entry to disk: " + std::string(e.what()));
            return false;
        }
    }

    bool ShaderCache::LoadShaderFromDisk(const std::string& cacheKey, ShaderCacheEntry& entry) {
        if (!m_config.enablePersistentCache) {
            return false;
        }

        try {
            std::string filepath = GetCacheFilePath(cacheKey);
            std::ifstream file(filepath);
            
            if (!file.is_open()) {
                return false;
            }
            
            // Read cache entry metadata
            std::getline(file, entry.shaderName);
            std::getline(file, entry.sourceHash);
            std::getline(file, entry.variantHash);
            
            std::string line;
            std::getline(file, line);
            entry.accessCount = std::stoull(line);
            
            std::getline(file, line);
            entry.memoryUsage = std::stoull(line);
            
            std::getline(file, line);
            entry.isPersistent = (std::stoi(line) == 1);
            
            entry.creationTime = std::chrono::system_clock::now();
            entry.lastAccessTime = std::chrono::system_clock::now();
            
            // Note: We can't deserialize the actual Shader object easily
            // In a full implementation, we would deserialize and recompile from bytecode
            // For now, we'll leave the shader as nullptr and rely on lazy loading
            entry.shader = nullptr;
            
            file.close();
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Error loading cache entry from disk: " + std::string(e.what()));
            return false;
        }
    }

    void ShaderCache::CleanupDiskCache() {
        if (!m_config.enablePersistentCache) {
            return;
        }

        try {
            // Remove cache files that are no longer in memory
            for (const auto& entry : std::filesystem::directory_iterator(m_config.cacheDirectory)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cache") {
                    std::string cacheKey = entry.path().stem().string();
                    
                    if (m_cache.find(cacheKey) == m_cache.end()) {
                        std::filesystem::remove(entry.path());
                        
                        if (m_debugMode) {
                            LOG_INFO("Cleaned up orphaned cache file: " + entry.path().string());
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Error cleaning up disk cache: " + std::string(e.what()));
        }
    }

    void ShaderCache::UpdateAccessInfo(ShaderCacheEntry& entry) {
        entry.lastAccessTime = std::chrono::system_clock::now();
        entry.accessCount++;
    }

    bool ShaderCache::IsEntryExpired(const ShaderCacheEntry& entry) const {
        auto now = std::chrono::system_clock::now();
        auto timeSinceAccess = std::chrono::duration_cast<std::chrono::minutes>(now - entry.lastAccessTime);
        
        return timeSinceAccess >= m_config.entryExpirationTime;
    }

    void ShaderCache::LogCacheOperation(const std::string& operation, const std::string& key, bool hit) const {
        if (!m_debugMode) {
            return;
        }

        std::string message = "Cache " + operation + ": " + key;
        if (operation == "GET" || operation == "GET_VARIANT") {
            message += hit ? " (HIT)" : " (MISS)";
        }
        
        LOG_INFO(message);
    }
}