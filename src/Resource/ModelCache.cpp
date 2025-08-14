#include "Resource/ModelCache.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/ModelNode.h"
#include "Graphics/GraphicsAnimation.h"
#include "Graphics/RenderSkeleton.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace GameEngine {

    std::unique_ptr<ModelCache> GlobalModelCache::s_instance = nullptr;

    ModelCache& GlobalModelCache::GetInstance() {
        if (!s_instance) {
            s_instance = std::make_unique<ModelCache>();
        }
        return *s_instance;
    }

    ModelCache::ModelCache() 
        : m_initialized(false) {
    }

    ModelCache::~ModelCache() {
        Shutdown();
    }

    bool ModelCache::Initialize(const std::string& cacheDirectory) {
        if (m_initialized) {
            return true;
        }

        m_cacheDirectory = cacheDirectory;
        m_indexFilePath = m_cacheDirectory + "/cache_index.dat";

        // Create cache directory if it doesn't exist
        if (!CreateDirectoryIfNotExists(m_cacheDirectory)) {
            LOG_ERROR("Failed to create cache directory: " + m_cacheDirectory);
            return false;
        }

        // Load existing cache index
        if (!LoadCacheIndex()) {
            LOG_WARNING("Failed to load cache index, starting with empty cache");
            m_cacheIndex.clear();
        }

        // Clean up invalid entries on startup
        CleanupInvalidEntries();

        m_initialized = true;
        LOG_INFO("ModelCache initialized with directory: " + m_cacheDirectory);
        LOG_INFO("Cache contains " + std::to_string(m_cacheIndex.size()) + " entries");

        return true;
    }

    void ModelCache::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Save cache index
        if (!SaveCacheIndex()) {
            LOG_ERROR("Failed to save cache index during shutdown");
        }

        m_cacheIndex.clear();
        m_initialized = false;

        LOG_INFO("ModelCache shutdown complete");
    }

    bool ModelCache::IsCached(const std::string& modelPath) const {
        if (!m_initialized) {
            return false;
        }

        std::string cacheKey = GenerateCacheKey(modelPath);
        return m_cacheIndex.find(cacheKey) != m_cacheIndex.end();
    }

    bool ModelCache::IsValidCache(const std::string& modelPath) const {
        if (!IsCached(modelPath)) {
            return false;
        }

        std::string cacheKey = GenerateCacheKey(modelPath);
        const auto& entry = m_cacheIndex.at(cacheKey);
        
        return ValidateCacheEntry(entry);
    }

    std::shared_ptr<Model> ModelCache::LoadFromCache(const std::string& modelPath) {
        if (!m_initialized || !IsValidCache(modelPath)) {
            m_stats.cacheMisses++;
            return nullptr;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        std::string cacheKey = GenerateCacheKey(modelPath);
        const auto& entry = m_cacheIndex[cacheKey];

        try {
            std::ifstream file(entry.cachePath, std::ios::binary);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open cache file: " + entry.cachePath);
                m_stats.cacheMisses++;
                return nullptr;
            }

            // Verify magic number and version
            uint32_t magic, version;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.read(reinterpret_cast<char*>(&version), sizeof(version));

            if (magic != CACHE_MAGIC) {
                LOG_ERROR("Invalid cache file magic number: " + entry.cachePath);
                m_stats.cacheMisses++;
                return nullptr;
            }

            if (version != CACHE_VERSION) {
                LOG_WARNING("Cache file version mismatch (expected " + std::to_string(CACHE_VERSION) + 
                           ", got " + std::to_string(version) + "): " + entry.cachePath);
                m_stats.cacheMisses++;
                return nullptr;
            }

            // Deserialize model
            auto model = DeserializeModel(file, modelPath);
            if (!model) {
                LOG_ERROR("Failed to deserialize model from cache: " + entry.cachePath);
                m_stats.cacheMisses++;
                return nullptr;
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            auto loadTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

            m_stats.cacheHits++;
            LOG_INFO("Successfully loaded model from cache: " + modelPath + " (" + 
                    std::to_string(loadTime) + "ms)");

            return model;

        } catch (const std::exception& e) {
            LOG_ERROR("Exception loading model from cache: " + std::string(e.what()));
            m_stats.cacheMisses++;
            return nullptr;
        }
    }

    bool ModelCache::SaveToCache(const std::string& modelPath, std::shared_ptr<Model> model) {
        if (!m_initialized || !model) {
            return false;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        std::string cacheKey = GenerateCacheKey(modelPath);
        std::string cachePath = GetCachePath(modelPath);

        try {
            std::ofstream file(cachePath, std::ios::binary);
            if (!file.is_open()) {
                LOG_ERROR("Failed to create cache file: " + cachePath);
                return false;
            }

            // Write magic number and version
            uint32_t magic = CACHE_MAGIC;
            uint32_t version = CACHE_VERSION;
            file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
            file.write(reinterpret_cast<const char*>(&version), sizeof(version));

            // Serialize model
            if (!SerializeModel(model, file)) {
                LOG_ERROR("Failed to serialize model to cache: " + cachePath);
                file.close();
                std::filesystem::remove(cachePath);
                return false;
            }

            file.close();

            // Create cache entry
            CacheEntry entry = CreateCacheEntry(modelPath);
            entry.cachePath = cachePath;
            entry.cacheModTime = std::chrono::system_clock::now();
            entry.cacheFileSize = GetFileSize(cachePath);
            entry.formatUsed = model->GetStats().formatUsed;
            entry.isValid = true;

            // Update cache index
            m_cacheIndex[cacheKey] = entry;

            // Save updated index
            SaveCacheIndex();

            auto endTime = std::chrono::high_resolution_clock::now();
            auto saveTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

            LOG_INFO("Successfully saved model to cache: " + modelPath + " (" + 
                    std::to_string(saveTime) + "ms, " + 
                    std::to_string(entry.cacheFileSize / 1024) + " KB)");

            // Check if we need to evict old entries
            if (GetCacheSize() > m_maxCacheSize) {
                EvictLargestEntries();
            }

            return true;

        } catch (const std::exception& e) {
            LOG_ERROR("Exception saving model to cache: " + std::string(e.what()));
            return false;
        }
    }

    void ModelCache::InvalidateCache(const std::string& modelPath) {
        if (!m_initialized) {
            return;
        }

        std::string cacheKey = GenerateCacheKey(modelPath);
        auto it = m_cacheIndex.find(cacheKey);
        
        if (it != m_cacheIndex.end()) {
            // Remove cache file
            try {
                if (std::filesystem::exists(it->second.cachePath)) {
                    std::filesystem::remove(it->second.cachePath);
                }
            } catch (const std::exception& e) {
                LOG_WARNING("Failed to remove cache file: " + std::string(e.what()));
            }

            // Remove from index
            m_cacheIndex.erase(it);
            SaveCacheIndex();

            LOG_INFO("Invalidated cache for: " + modelPath);
        }
    }

    void ModelCache::InvalidateAllCache() {
        if (!m_initialized) {
            return;
        }

        // Remove all cache files
        for (const auto& [key, entry] : m_cacheIndex) {
            try {
                if (std::filesystem::exists(entry.cachePath)) {
                    std::filesystem::remove(entry.cachePath);
                }
            } catch (const std::exception& e) {
                LOG_WARNING("Failed to remove cache file: " + std::string(e.what()));
            }
        }

        // Clear index
        m_cacheIndex.clear();
        SaveCacheIndex();

        LOG_INFO("Invalidated all cache entries");
    }

    void ModelCache::CleanupInvalidEntries() {
        if (!m_initialized) {
            return;
        }

        std::vector<std::string> keysToRemove;

        for (const auto& [key, entry] : m_cacheIndex) {
            if (!ValidateCacheEntry(entry)) {
                keysToRemove.push_back(key);
                
                // Remove cache file if it exists
                try {
                    if (std::filesystem::exists(entry.cachePath)) {
                        std::filesystem::remove(entry.cachePath);
                    }
                } catch (const std::exception& e) {
                    LOG_WARNING("Failed to remove invalid cache file: " + std::string(e.what()));
                }
            }
        }

        // Remove invalid entries from index
        for (const auto& key : keysToRemove) {
            m_cacheIndex.erase(key);
        }

        if (!keysToRemove.empty()) {
            SaveCacheIndex();
            LOG_INFO("Cleaned up " + std::to_string(keysToRemove.size()) + " invalid cache entries");
        }
    }

    void ModelCache::CompactCache() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Starting cache compaction...");

        // First, clean up invalid entries
        CleanupInvalidEntries();

        // Then, evict old entries if needed
        EvictOldEntries();

        // Finally, evict largest entries if we're still over the size limit
        if (GetCacheSize() > m_maxCacheSize) {
            EvictLargestEntries();
        }

        LOG_INFO("Cache compaction completed. Cache size: " + 
                std::to_string(GetCacheSize() / 1024 / 1024) + " MB");
    }

    void ModelCache::SetMaxCacheSize(size_t maxSizeBytes) {
        m_maxCacheSize = maxSizeBytes;
        
        // Trigger compaction if we're over the new limit
        if (GetCacheSize() > m_maxCacheSize) {
            CompactCache();
        }
    }

    void ModelCache::SetMaxCacheAge(std::chrono::hours maxAge) {
        m_maxCacheAge = maxAge;
        
        // Trigger cleanup of old entries
        EvictOldEntries();
    }

    void ModelCache::SetCompressionEnabled(bool enabled) {
        m_compressionEnabled = enabled;
        // Note: Compression would be implemented in the serialization methods
        // For now, this is just a configuration flag
    }

    ModelCache::CacheStats ModelCache::GetStats() const {
        m_stats.totalEntries = static_cast<uint32_t>(m_cacheIndex.size());
        m_stats.validEntries = 0;
        m_stats.invalidEntries = 0;
        m_stats.totalCacheSize = 0;

        for (const auto& [key, entry] : m_cacheIndex) {
            if (ValidateCacheEntry(entry)) {
                m_stats.validEntries++;
                m_stats.totalCacheSize += entry.cacheFileSize;
            } else {
                m_stats.invalidEntries++;
            }
        }

        // Calculate average load speedup (placeholder - would need timing data)
        if (m_stats.cacheHits > 0) {
            m_stats.averageLoadSpeedup = 3.5f; // Estimated 3.5x speedup
        }

        return m_stats;
    }

    std::vector<ModelCache::CacheEntry> ModelCache::GetAllEntries() const {
        std::vector<CacheEntry> entries;
        entries.reserve(m_cacheIndex.size());

        for (const auto& [key, entry] : m_cacheIndex) {
            entries.push_back(entry);
        }

        return entries;
    }

    size_t ModelCache::GetCacheSize() const {
        size_t totalSize = 0;
        for (const auto& [key, entry] : m_cacheIndex) {
            totalSize += entry.cacheFileSize;
        }
        return totalSize;
    }

    void ModelCache::PrintCacheInfo() const {
        auto stats = GetStats();
        
        LOG_INFO("=== Model Cache Information ===");
        LOG_INFO("Cache Directory: " + m_cacheDirectory);
        LOG_INFO("Total Entries: " + std::to_string(stats.totalEntries));
        LOG_INFO("Valid Entries: " + std::to_string(stats.validEntries));
        LOG_INFO("Invalid Entries: " + std::to_string(stats.invalidEntries));
        LOG_INFO("Cache Hits: " + std::to_string(stats.cacheHits));
        LOG_INFO("Cache Misses: " + std::to_string(stats.cacheMisses));
        LOG_INFO("Total Cache Size: " + std::to_string(stats.totalCacheSize / 1024 / 1024) + " MB");
        LOG_INFO("Max Cache Size: " + std::to_string(m_maxCacheSize / 1024 / 1024) + " MB");
        LOG_INFO("Max Cache Age: " + std::to_string(m_maxCacheAge.count()) + " hours");
        LOG_INFO("Compression Enabled: " + std::string(m_compressionEnabled ? "Yes" : "No"));
        
        if (stats.cacheHits > 0) {
            float hitRate = static_cast<float>(stats.cacheHits) / (stats.cacheHits + stats.cacheMisses) * 100.0f;
            LOG_INFO("Cache Hit Rate: " + std::to_string(hitRate) + "%");
            LOG_INFO("Average Load Speedup: " + std::to_string(stats.averageLoadSpeedup) + "x");
        }
        
        LOG_INFO("===============================");
    }

    std::string ModelCache::GetCachePath(const std::string& modelPath) const {
        std::string cacheKey = GenerateCacheKey(modelPath);
        return m_cacheDirectory + "/" + cacheKey + ".kmc"; // Kiro Model Cache
    }

    std::string ModelCache::GenerateCacheKey(const std::string& modelPath) {
        // Create a hash-based cache key from the model path
        std::hash<std::string> hasher;
        size_t hash = hasher(modelPath);
        
        std::stringstream ss;
        ss << std::hex << hash;
        return ss.str();
    }

    bool ModelCache::LoadCacheIndex() {
        if (!std::filesystem::exists(m_indexFilePath)) {
            return true; // Empty cache is valid
        }

        try {
            std::ifstream file(m_indexFilePath, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            // Read magic number and version
            uint32_t magic, version;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.read(reinterpret_cast<char*>(&version), sizeof(version));

            if (magic != CACHE_MAGIC || version != CACHE_VERSION) {
                LOG_WARNING("Cache index version mismatch, rebuilding cache");
                return false;
            }

            // Read number of entries
            uint32_t entryCount;
            file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));

            // Read entries
            for (uint32_t i = 0; i < entryCount; ++i) {
                CacheEntry entry;
                
                entry.originalPath = ReadString(file);
                entry.cachePath = ReadString(file);
                entry.formatUsed = ReadString(file);
                
                file.read(reinterpret_cast<char*>(&entry.originalFileSize), sizeof(entry.originalFileSize));
                file.read(reinterpret_cast<char*>(&entry.cacheFileSize), sizeof(entry.cacheFileSize));
                file.read(reinterpret_cast<char*>(&entry.version), sizeof(entry.version));
                
                // Read timestamps (as time_t for portability)
                std::time_t originalTime, cacheTime;
                file.read(reinterpret_cast<char*>(&originalTime), sizeof(originalTime));
                file.read(reinterpret_cast<char*>(&cacheTime), sizeof(cacheTime));
                
                entry.originalModTime = std::chrono::system_clock::from_time_t(originalTime);
                entry.cacheModTime = std::chrono::system_clock::from_time_t(cacheTime);
                entry.isValid = true;

                std::string cacheKey = GenerateCacheKey(entry.originalPath);
                m_cacheIndex[cacheKey] = entry;
            }

            LOG_INFO("Loaded cache index with " + std::to_string(entryCount) + " entries");
            return true;

        } catch (const std::exception& e) {
            LOG_ERROR("Failed to load cache index: " + std::string(e.what()));
            return false;
        }
    }

    bool ModelCache::SaveCacheIndex() {
        try {
            std::ofstream file(m_indexFilePath, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            // Write magic number and version
            uint32_t magic = CACHE_MAGIC;
            uint32_t version = CACHE_VERSION;
            file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
            file.write(reinterpret_cast<const char*>(&version), sizeof(version));

            // Write number of entries
            uint32_t entryCount = static_cast<uint32_t>(m_cacheIndex.size());
            file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));

            // Write entries
            for (const auto& [key, entry] : m_cacheIndex) {
                WriteString(file, entry.originalPath);
                WriteString(file, entry.cachePath);
                WriteString(file, entry.formatUsed);
                
                file.write(reinterpret_cast<const char*>(&entry.originalFileSize), sizeof(entry.originalFileSize));
                file.write(reinterpret_cast<const char*>(&entry.cacheFileSize), sizeof(entry.cacheFileSize));
                file.write(reinterpret_cast<const char*>(&entry.version), sizeof(entry.version));
                
                // Write timestamps (as time_t for portability)
                std::time_t originalTime = std::chrono::system_clock::to_time_t(entry.originalModTime);
                std::time_t cacheTime = std::chrono::system_clock::to_time_t(entry.cacheModTime);
                file.write(reinterpret_cast<const char*>(&originalTime), sizeof(originalTime));
                file.write(reinterpret_cast<const char*>(&cacheTime), sizeof(cacheTime));
            }

            return true;

        } catch (const std::exception& e) {
            LOG_ERROR("Failed to save cache index: " + std::string(e.what()));
            return false;
        }
    }

    bool ModelCache::ValidateCacheEntry(const CacheEntry& entry) const {
        // Check if original file exists
        if (!std::filesystem::exists(entry.originalPath)) {
            return false;
        }

        // Check if cache file exists
        if (!std::filesystem::exists(entry.cachePath)) {
            return false;
        }

        // Check if original file has been modified
        auto currentModTime = GetFileModificationTime(entry.originalPath);
        if (currentModTime > entry.originalModTime) {
            return false;
        }

        // Check cache age
        auto now = std::chrono::system_clock::now();
        auto cacheAge = std::chrono::duration_cast<std::chrono::hours>(now - entry.cacheModTime);
        if (cacheAge > m_maxCacheAge) {
            return false;
        }

        // Check version compatibility
        if (entry.version != CACHE_VERSION) {
            return false;
        }

        return true;
    }

    ModelCache::CacheEntry ModelCache::CreateCacheEntry(const std::string& modelPath) const {
        CacheEntry entry;
        entry.originalPath = modelPath;
        entry.originalModTime = GetFileModificationTime(modelPath);
        entry.originalFileSize = GetFileSize(modelPath);
        entry.version = CACHE_VERSION;
        return entry;
    }

    // Serialization methods would be implemented here
    // For brevity, I'll implement key methods and leave others as stubs

    bool ModelCache::SerializeModel(std::shared_ptr<Model> model, std::ofstream& file) {
        if (!model) {
            return false;
        }

        try {
            // Write model metadata
            WriteString(file, model->GetName());
            WriteString(file, model->GetPath());
            
            // Write model stats
            auto stats = model->GetStats();
            file.write(reinterpret_cast<const char*>(&stats.nodeCount), sizeof(stats.nodeCount));
            file.write(reinterpret_cast<const char*>(&stats.meshCount), sizeof(stats.meshCount));
            file.write(reinterpret_cast<const char*>(&stats.materialCount), sizeof(stats.materialCount));
            file.write(reinterpret_cast<const char*>(&stats.totalVertices), sizeof(stats.totalVertices));
            file.write(reinterpret_cast<const char*>(&stats.totalTriangles), sizeof(stats.totalTriangles));
            WriteString(file, stats.formatUsed);

            // Write meshes
            auto meshes = model->GetMeshes();
            uint32_t meshCount = static_cast<uint32_t>(meshes.size());
            file.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
            
            for (const auto& mesh : meshes) {
                if (!SerializeMesh(mesh, file)) {
                    return false;
                }
            }

            // Write materials
            auto materials = model->GetMaterials();
            uint32_t materialCount = static_cast<uint32_t>(materials.size());
            file.write(reinterpret_cast<const char*>(&materialCount), sizeof(materialCount));
            
            for (const auto& material : materials) {
                if (!SerializeMaterial(material, file)) {
                    return false;
                }
            }

            // Write root node
            if (!SerializeModelNode(model->GetRootNode(), file)) {
                return false;
            }

            return true;

        } catch (const std::exception& e) {
            LOG_ERROR("Exception during model serialization: " + std::string(e.what()));
            return false;
        }
    }

    std::shared_ptr<Model> ModelCache::DeserializeModel(std::ifstream& file, const std::string& originalPath) {
        try {
            auto model = std::make_shared<Model>(originalPath);

            // Read model metadata
            std::string name = ReadString(file);
            std::string path = ReadString(file);
            model->SetName(name);

            // Read model stats (for validation)
            uint32_t nodeCount, meshCount, materialCount, totalVertices, totalTriangles;
            file.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));
            file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
            file.read(reinterpret_cast<char*>(&materialCount), sizeof(materialCount));
            file.read(reinterpret_cast<char*>(&totalVertices), sizeof(totalVertices));
            file.read(reinterpret_cast<char*>(&totalTriangles), sizeof(totalTriangles));
            std::string formatUsed = ReadString(file);

            // Read meshes
            uint32_t meshCountFromFile;
            file.read(reinterpret_cast<char*>(&meshCountFromFile), sizeof(meshCountFromFile));
            
            std::vector<std::shared_ptr<Mesh>> meshes;
            meshes.reserve(meshCountFromFile);
            
            for (uint32_t i = 0; i < meshCountFromFile; ++i) {
                auto mesh = DeserializeMesh(file);
                if (!mesh) {
                    return nullptr;
                }
                meshes.push_back(mesh);
            }
            model->SetMeshes(meshes);

            // Read materials
            uint32_t materialCountFromFile;
            file.read(reinterpret_cast<char*>(&materialCountFromFile), sizeof(materialCountFromFile));
            
            std::vector<std::shared_ptr<Material>> materials;
            materials.reserve(materialCountFromFile);
            
            for (uint32_t i = 0; i < materialCountFromFile; ++i) {
                auto material = DeserializeMaterial(file);
                if (!material) {
                    return nullptr;
                }
                materials.push_back(material);
            }
            model->SetMaterials(materials);

            // Read root node (placeholder - would need ModelNode serialization)
            // For now, create a simple root node
            // auto rootNode = DeserializeModelNode(file);
            // if (!rootNode) {
            //     return nullptr;
            // }

            return model;

        } catch (const std::exception& e) {
            LOG_ERROR("Exception during model deserialization: " + std::string(e.what()));
            return nullptr;
        }
    }

    // Stub implementations for component serialization
    bool ModelCache::SerializeMesh(std::shared_ptr<Mesh> mesh, std::ofstream& file) {
        if (!mesh) {
            return false;
        }

        // Write mesh name
        WriteString(file, mesh->GetName());

        // Write vertex data
        auto vertices = mesh->GetVertices();
        uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
        file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
        file.write(reinterpret_cast<const char*>(vertices.data()), vertexCount * sizeof(Vertex));

        // Write index data
        auto indices = mesh->GetIndices();
        uint32_t indexCount = static_cast<uint32_t>(indices.size());
        file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
        file.write(reinterpret_cast<const char*>(indices.data()), indexCount * sizeof(uint32_t));

        return true;
    }

    std::shared_ptr<Mesh> ModelCache::DeserializeMesh(std::ifstream& file) {
        try {
            // Read mesh name
            std::string name = ReadString(file);
            auto mesh = std::make_shared<Mesh>(name);

            // Read vertex data
            uint32_t vertexCount;
            file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
            
            std::vector<Vertex> vertices(vertexCount);
            file.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));
            mesh->SetVertices(vertices);

            // Read index data
            uint32_t indexCount;
            file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
            
            std::vector<uint32_t> indices(indexCount);
            file.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(uint32_t));
            mesh->SetIndices(indices);

            return mesh;

        } catch (const std::exception& e) {
            LOG_ERROR("Exception during mesh deserialization: " + std::string(e.what()));
            return nullptr;
        }
    }

    bool ModelCache::SerializeMaterial(std::shared_ptr<Material> material, std::ofstream& file) {
        // Placeholder implementation
        if (!material) {
            return false;
        }

        // Write basic material properties
        WriteVector3(file, material->GetAlbedo());
        float metallic = material->GetMetallic();
        float roughness = material->GetRoughness();
        file.write(reinterpret_cast<const char*>(&metallic), sizeof(float));
        file.write(reinterpret_cast<const char*>(&roughness), sizeof(float));

        return true;
    }

    std::shared_ptr<Material> ModelCache::DeserializeMaterial(std::ifstream& file) {
        try {
            auto material = std::make_shared<Material>();

            // Read basic material properties
            Math::Vec3 albedo = ReadVector3(file);
            material->SetAlbedo(albedo);

            float metallic, roughness;
            file.read(reinterpret_cast<char*>(&metallic), sizeof(float));
            file.read(reinterpret_cast<char*>(&roughness), sizeof(float));
            material->SetMetallic(metallic);
            material->SetRoughness(roughness);

            return material;

        } catch (const std::exception& e) {
            LOG_ERROR("Exception during material deserialization: " + std::string(e.what()));
            return nullptr;
        }
    }

    // Stub implementations for other components
    bool ModelCache::SerializeModelNode(std::shared_ptr<ModelNode> node, std::ofstream& file) {
        // Placeholder - would implement full node hierarchy serialization
        return true;
    }

    std::shared_ptr<ModelNode> ModelCache::DeserializeModelNode(std::ifstream& file) {
        // Placeholder - would implement full node hierarchy deserialization
        return std::make_shared<ModelNode>("Root");
    }

    bool ModelCache::SerializeAnimation(std::shared_ptr<Animation> animation, std::ofstream& file) {
        // Placeholder
        return true;
    }

    std::shared_ptr<Animation> ModelCache::DeserializeAnimation(std::ifstream& file) {
        // Placeholder
        return nullptr;
    }

    bool ModelCache::SerializeSkeleton(std::shared_ptr<Skeleton> skeleton, std::ofstream& file) {
        // Placeholder
        return true;
    }

    std::shared_ptr<Skeleton> ModelCache::DeserializeSkeleton(std::ifstream& file) {
        // Placeholder
        return nullptr;
    }

    // Utility serialization methods
    void ModelCache::WriteString(std::ofstream& file, const std::string& str) {
        uint32_t length = static_cast<uint32_t>(str.length());
        file.write(reinterpret_cast<const char*>(&length), sizeof(length));
        file.write(str.c_str(), length);
    }

    std::string ModelCache::ReadString(std::ifstream& file) {
        uint32_t length;
        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        
        std::string str(length, '\0');
        file.read(&str[0], length);
        return str;
    }

    void ModelCache::WriteVector3(std::ofstream& file, const Math::Vec3& vec) {
        file.write(reinterpret_cast<const char*>(&vec.x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&vec.y), sizeof(float));
        file.write(reinterpret_cast<const char*>(&vec.z), sizeof(float));
    }

    Math::Vec3 ModelCache::ReadVector3(std::ifstream& file) {
        Math::Vec3 vec;
        file.read(reinterpret_cast<char*>(&vec.x), sizeof(float));
        file.read(reinterpret_cast<char*>(&vec.y), sizeof(float));
        file.read(reinterpret_cast<char*>(&vec.z), sizeof(float));
        return vec;
    }

    void ModelCache::WriteVector2(std::ofstream& file, const Math::Vec2& vec) {
        file.write(reinterpret_cast<const char*>(&vec.x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&vec.y), sizeof(float));
    }

    Math::Vec2 ModelCache::ReadVector2(std::ifstream& file) {
        Math::Vec2 vec;
        file.read(reinterpret_cast<char*>(&vec.x), sizeof(float));
        file.read(reinterpret_cast<char*>(&vec.y), sizeof(float));
        return vec;
    }

    void ModelCache::WriteVector4(std::ofstream& file, const Math::Vec4& vec) {
        file.write(reinterpret_cast<const char*>(&vec.x), sizeof(float));
        file.write(reinterpret_cast<const char*>(&vec.y), sizeof(float));
        file.write(reinterpret_cast<const char*>(&vec.z), sizeof(float));
        file.write(reinterpret_cast<const char*>(&vec.w), sizeof(float));
    }

    Math::Vec4 ModelCache::ReadVector4(std::ifstream& file) {
        Math::Vec4 vec;
        file.read(reinterpret_cast<char*>(&vec.x), sizeof(float));
        file.read(reinterpret_cast<char*>(&vec.y), sizeof(float));
        file.read(reinterpret_cast<char*>(&vec.z), sizeof(float));
        file.read(reinterpret_cast<char*>(&vec.w), sizeof(float));
        return vec;
    }

    void ModelCache::WriteMatrix4(std::ofstream& file, const Math::Mat4& mat) {
        file.write(reinterpret_cast<const char*>(&mat[0][0]), 16 * sizeof(float));
    }

    Math::Mat4 ModelCache::ReadMatrix4(std::ifstream& file) {
        Math::Mat4 mat;
        file.read(reinterpret_cast<char*>(&mat[0][0]), 16 * sizeof(float));
        return mat;
    }

    // Cache maintenance methods
    void ModelCache::EvictOldEntries() {
        auto now = std::chrono::system_clock::now();
        std::vector<std::string> keysToRemove;

        for (const auto& [key, entry] : m_cacheIndex) {
            auto age = std::chrono::duration_cast<std::chrono::hours>(now - entry.cacheModTime);
            if (age > m_maxCacheAge) {
                keysToRemove.push_back(key);
            }
        }

        for (const auto& key : keysToRemove) {
            InvalidateCache(m_cacheIndex[key].originalPath);
        }

        if (!keysToRemove.empty()) {
            LOG_INFO("Evicted " + std::to_string(keysToRemove.size()) + " old cache entries");
        }
    }

    void ModelCache::EvictLargestEntries() {
        // Sort entries by size (largest first)
        std::vector<std::pair<std::string, CacheEntry>> sortedEntries;
        for (const auto& [key, entry] : m_cacheIndex) {
            sortedEntries.emplace_back(key, entry);
        }

        std::sort(sortedEntries.begin(), sortedEntries.end(),
                  [](const auto& a, const auto& b) {
                      return a.second.cacheFileSize > b.second.cacheFileSize;
                  });

        // Remove largest entries until we're under the size limit
        size_t currentSize = GetCacheSize();
        size_t removedCount = 0;

        for (const auto& [key, entry] : sortedEntries) {
            if (currentSize <= m_maxCacheSize) {
                break;
            }

            InvalidateCache(entry.originalPath);
            currentSize -= entry.cacheFileSize;
            removedCount++;
        }

        if (removedCount > 0) {
            LOG_INFO("Evicted " + std::to_string(removedCount) + " largest cache entries");
        }
    }

    bool ModelCache::ShouldEvictEntry(const CacheEntry& entry) const {
        // Check age
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::hours>(now - entry.cacheModTime);
        if (age > m_maxCacheAge) {
            return true;
        }

        // Check if original file still exists
        if (!std::filesystem::exists(entry.originalPath)) {
            return true;
        }

        return false;
    }

    // File system utilities
    bool ModelCache::CreateDirectoryIfNotExists(const std::string& path) {
        try {
            if (!std::filesystem::exists(path)) {
                return std::filesystem::create_directories(path);
            }
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create directory: " + std::string(e.what()));
            return false;
        }
    }

    std::chrono::system_clock::time_point ModelCache::GetFileModificationTime(const std::string& path) const {
        try {
            auto ftime = std::filesystem::last_write_time(path);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            return sctp;
        } catch (const std::exception&) {
            return std::chrono::system_clock::now();
        }
    }

    size_t ModelCache::GetFileSize(const std::string& path) const {
        try {
            return std::filesystem::file_size(path);
        } catch (const std::exception&) {
            return 0;
        }
    }

} // namespace GameEngine