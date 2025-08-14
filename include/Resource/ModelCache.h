#pragma once

#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <fstream>

namespace GameEngine {
    class Model;
    class Mesh;
    class Material;
    class ModelNode;
    
    namespace Graphics {
        class GraphicsAnimation;
        class RenderSkeleton;
    }

    /**
     * @brief Binary model cache for faster subsequent loading
     * 
     * Provides efficient serialization and deserialization of Model objects
     * with version compatibility and cache invalidation management.
     */
    class ModelCache {
    public:
        /**
         * @brief Cache file format version for compatibility checking
         */
        static constexpr uint32_t CACHE_VERSION = 1;
        
        /**
         * @brief Magic number for cache file identification
         */
        static constexpr uint32_t CACHE_MAGIC = 0x4B4D4443; // "KMDC" (Kiro Model Data Cache)

        /**
         * @brief Cache entry metadata
         */
        struct CacheEntry {
            std::string originalPath;
            std::string cachePath;
            std::chrono::system_clock::time_point originalModTime;
            std::chrono::system_clock::time_point cacheModTime;
            size_t originalFileSize = 0;
            size_t cacheFileSize = 0;
            uint32_t version = CACHE_VERSION;
            std::string formatUsed;
            bool isValid = false;
        };

        /**
         * @brief Cache statistics for monitoring and debugging
         */
        struct CacheStats {
            uint32_t totalEntries = 0;
            uint32_t validEntries = 0;
            uint32_t invalidEntries = 0;
            uint32_t cacheHits = 0;
            uint32_t cacheMisses = 0;
            size_t totalCacheSize = 0;
            float averageLoadSpeedup = 0.0f;
        };

    public:
        ModelCache();
        ~ModelCache();

        // Lifecycle
        bool Initialize(const std::string& cacheDirectory = "cache/models");
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // Cache operations
        bool IsCached(const std::string& modelPath) const;
        bool IsValidCache(const std::string& modelPath) const;
        std::shared_ptr<Model> LoadFromCache(const std::string& modelPath);
        bool SaveToCache(const std::string& modelPath, std::shared_ptr<Model> model);

        // Cache management
        void InvalidateCache(const std::string& modelPath);
        void InvalidateAllCache();
        void CleanupInvalidEntries();
        void CompactCache();

        // Configuration
        void SetMaxCacheSize(size_t maxSizeBytes);
        void SetMaxCacheAge(std::chrono::hours maxAge);
        void SetCompressionEnabled(bool enabled);

        // Statistics and monitoring
        CacheStats GetStats() const;
        std::vector<CacheEntry> GetAllEntries() const;
        size_t GetCacheSize() const;
        void PrintCacheInfo() const;

        // Utility methods
        std::string GetCachePath(const std::string& modelPath) const;
        static std::string GenerateCacheKey(const std::string& modelPath);

    private:
        // Cache directory and file management
        std::string m_cacheDirectory;
        std::string m_indexFilePath;
        std::unordered_map<std::string, CacheEntry> m_cacheIndex;
        
        // Configuration
        size_t m_maxCacheSize = 1024 * 1024 * 1024; // 1GB default
        std::chrono::hours m_maxCacheAge = std::chrono::hours(24 * 7); // 1 week default
        bool m_compressionEnabled = true;
        bool m_initialized = false;

        // Statistics
        mutable CacheStats m_stats;

        // Internal methods
        bool LoadCacheIndex();
        bool SaveCacheIndex();
        bool ValidateCacheEntry(const CacheEntry& entry) const;
        CacheEntry CreateCacheEntry(const std::string& modelPath) const;
        
        // Serialization methods
        bool SerializeModel(std::shared_ptr<Model> model, std::ofstream& file);
        std::shared_ptr<Model> DeserializeModel(std::ifstream& file, const std::string& originalPath);
        
        // Component serialization
        bool SerializeMesh(std::shared_ptr<Mesh> mesh, std::ofstream& file);
        std::shared_ptr<Mesh> DeserializeMesh(std::ifstream& file);
        bool SerializeMaterial(std::shared_ptr<Material> material, std::ofstream& file);
        std::shared_ptr<Material> DeserializeMaterial(std::ifstream& file);
        bool SerializeModelNode(std::shared_ptr<ModelNode> node, std::ofstream& file);
        std::shared_ptr<ModelNode> DeserializeModelNode(std::ifstream& file);
        bool SerializeAnimation(std::shared_ptr<Graphics::GraphicsAnimation> animation, std::ofstream& file);
        std::shared_ptr<Graphics::GraphicsAnimation> DeserializeAnimation(std::ifstream& file);
        bool SerializeSkeleton(std::shared_ptr<Graphics::RenderSkeleton> skeleton, std::ofstream& file);
        std::shared_ptr<Graphics::RenderSkeleton> DeserializeSkeleton(std::ifstream& file);

        // Utility serialization methods
        void WriteString(std::ofstream& file, const std::string& str);
        std::string ReadString(std::ifstream& file);
        void WriteVector3(std::ofstream& file, const Math::Vec3& vec);
        Math::Vec3 ReadVector3(std::ifstream& file);
        void WriteVector2(std::ofstream& file, const Math::Vec2& vec);
        Math::Vec2 ReadVector2(std::ifstream& file);
        void WriteVector4(std::ofstream& file, const Math::Vec4& vec);
        Math::Vec4 ReadVector4(std::ifstream& file);
        void WriteMatrix4(std::ofstream& file, const Math::Mat4& mat);
        Math::Mat4 ReadMatrix4(std::ifstream& file);

        // Cache maintenance
        void EvictOldEntries();
        void EvictLargestEntries();
        bool ShouldEvictEntry(const CacheEntry& entry) const;
        
        // File system utilities
        bool CreateDirectoryIfNotExists(const std::string& path);
        std::chrono::system_clock::time_point GetFileModificationTime(const std::string& path) const;
        size_t GetFileSize(const std::string& path) const;
    };

    /**
     * @brief Global model cache instance for engine-wide use
     */
    class GlobalModelCache {
    public:
        static ModelCache& GetInstance();
        
    private:
        static std::unique_ptr<ModelCache> s_instance;
    };
}