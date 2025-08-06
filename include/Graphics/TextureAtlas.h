#pragma once

#include "Core/Math.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>

namespace GameEngine {
    class Texture;

    struct AtlasRegion {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        Math::Vec2 uvMin{0.0f, 0.0f};
        Math::Vec2 uvMax{1.0f, 1.0f};
        std::string name;
        bool occupied = false;
    };

    struct AtlasNode {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        bool occupied = false;
        std::unique_ptr<AtlasNode> left;
        std::unique_ptr<AtlasNode> right;
    };

    struct TextureAtlasStats {
        size_t totalTextures = 0;
        size_t atlasCount = 0;
        size_t totalMemoryUsage = 0;
        size_t wastedSpace = 0;
        float packingEfficiency = 0.0f;
        size_t maxAtlasSize = 0;
        size_t averageTextureSize = 0;
    };

    class TextureAtlas {
    public:
        TextureAtlas(uint32_t width = 2048, uint32_t height = 2048, uint32_t format = 0x8058); // GL_RGBA8
        ~TextureAtlas();

        // Texture management
        bool AddTexture(const std::string& name, const void* data, uint32_t width, uint32_t height, uint32_t channels = 4);
        bool AddTexture(const std::string& name, std::shared_ptr<Texture> texture);
        AtlasRegion GetRegion(const std::string& name) const;
        bool HasTexture(const std::string& name) const;
        void RemoveTexture(const std::string& name);

        // Atlas properties
        uint32_t GetAtlasTexture() const { return m_atlasTexture; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
        uint32_t GetFormat() const { return m_format; }
        
        // Statistics
        TextureAtlasStats GetStats() const;
        float GetPackingEfficiency() const;
        size_t GetUsedSpace() const;
        size_t GetWastedSpace() const;

        // Optimization
        void Optimize(); // Repack textures for better efficiency
        void Clear();
        bool IsFull() const;

        // Serialization
        bool SaveToFile(const std::string& filepath) const;
        bool LoadFromFile(const std::string& filepath);

    private:
        AtlasNode* FindNode(AtlasNode* node, uint32_t width, uint32_t height);
        AtlasNode* SplitNode(AtlasNode* node, uint32_t width, uint32_t height);
        void UpdateAtlasTexture();
        void CalculateUVCoordinates(AtlasRegion& region) const;
        void UpdateStats() const;

        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_format;
        uint32_t m_atlasTexture = 0;
        
        std::unique_ptr<AtlasNode> m_rootNode;
        std::unordered_map<std::string, AtlasRegion> m_regions;
        std::vector<uint8_t> m_atlasData;
        
        mutable std::mutex m_atlasMutex;
        mutable TextureAtlasStats m_stats;
        bool m_needsUpdate = false;
    };

    class TextureAtlasManager {
    public:
        static TextureAtlasManager& GetInstance();

        bool Initialize(uint32_t defaultAtlasSize = 2048, size_t maxAtlases = 16);
        void Shutdown();

        // Atlas management
        std::shared_ptr<TextureAtlas> CreateAtlas(const std::string& name, uint32_t width = 2048, uint32_t height = 2048);
        std::shared_ptr<TextureAtlas> GetAtlas(const std::string& name);
        void RemoveAtlas(const std::string& name);
        std::vector<std::string> GetAtlasNames() const;

        // Automatic texture packing
        bool AddTextureToAtlas(const std::string& textureName, const void* data, uint32_t width, uint32_t height, uint32_t channels = 4);
        bool AddTextureToAtlas(const std::string& textureName, std::shared_ptr<Texture> texture);
        AtlasRegion FindTexture(const std::string& textureName, std::string& atlasName) const;
        
        // Optimization
        void OptimizeAllAtlases();
        void CleanupEmptyAtlases();
        
        // Statistics
        TextureAtlasStats GetGlobalStats() const;
        void SetAutoOptimization(bool enable) { m_autoOptimizationEnabled = enable; }
        bool IsAutoOptimizationEnabled() const { return m_autoOptimizationEnabled; }

        // Configuration
        void SetDefaultAtlasSize(uint32_t size) { m_defaultAtlasSize = size; }
        uint32_t GetDefaultAtlasSize() const { return m_defaultAtlasSize; }
        void SetMaxAtlases(size_t count) { m_maxAtlases = count; }
        size_t GetMaxAtlases() const { return m_maxAtlases; }

    private:
        TextureAtlasManager() = default;
        ~TextureAtlasManager() = default;
        TextureAtlasManager(const TextureAtlasManager&) = delete;
        TextureAtlasManager& operator=(const TextureAtlasManager&) = delete;

        std::shared_ptr<TextureAtlas> FindBestAtlasForTexture(uint32_t width, uint32_t height);
        std::shared_ptr<TextureAtlas> CreateNewAtlasForTexture(uint32_t width, uint32_t height);

        mutable std::mutex m_managerMutex;
        std::unordered_map<std::string, std::shared_ptr<TextureAtlas>> m_atlases;
        std::unordered_map<std::string, std::string> m_textureToAtlasMap; // Maps texture name to atlas name

        uint32_t m_defaultAtlasSize = 2048;
        size_t m_maxAtlases = 16;
        bool m_autoOptimizationEnabled = true;
        std::atomic<bool> m_initialized{false};
    };
}