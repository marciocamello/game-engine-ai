#include "Graphics/TextureAtlas.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <algorithm>
#include <fstream>
#include <cstring>

namespace GameEngine {
    TextureAtlas::TextureAtlas(uint32_t width, uint32_t height, uint32_t format)
        : m_width(width), m_height(height), m_format(format) {
        
        // Initialize root node
        m_rootNode = std::make_unique<AtlasNode>();
        m_rootNode->x = 0;
        m_rootNode->y = 0;
        m_rootNode->width = width;
        m_rootNode->height = height;
        m_rootNode->occupied = false;

        // Initialize atlas data
        size_t dataSize = width * height * 4; // Assume RGBA for now
        m_atlasData.resize(dataSize, 0);

        // Create OpenGL texture
        glGenTextures(1, &m_atlasTexture);
        glBindTexture(GL_TEXTURE_2D, m_atlasTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, m_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_atlasData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        LOG_INFO("Created texture atlas: " + std::to_string(width) + "x" + std::to_string(height));
    }

    TextureAtlas::~TextureAtlas() {
        if (m_atlasTexture != 0) {
            glDeleteTextures(1, &m_atlasTexture);
        }
    }

    bool TextureAtlas::AddTexture(const std::string& name, const void* data, uint32_t width, uint32_t height, uint32_t channels) {
        std::lock_guard<std::mutex> lock(m_atlasMutex);

        // Check if texture already exists
        if (m_regions.find(name) != m_regions.end()) {
            LOG_WARNING("Texture already exists in atlas: " + name);
            return false;
        }

        // Find space in atlas
        AtlasNode* node = FindNode(m_rootNode.get(), width, height);
        if (!node) {
            LOG_WARNING("No space available in atlas for texture: " + name + " (" + 
                       std::to_string(width) + "x" + std::to_string(height) + ")");
            return false;
        }

        // Split node if necessary
        node = SplitNode(node, width, height);
        if (!node) {
            LOG_ERROR("Failed to split node for texture: " + name);
            return false;
        }

        // Create region
        AtlasRegion region;
        region.name = name;
        region.x = node->x;
        region.y = node->y;
        region.width = width;
        region.height = height;
        region.occupied = true;
        CalculateUVCoordinates(region);

        // Copy texture data to atlas
        if (data) {
            const uint8_t* srcData = static_cast<const uint8_t*>(data);
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    uint32_t atlasIndex = ((region.y + y) * m_width + (region.x + x)) * 4;
                    uint32_t srcIndex = (y * width + x) * channels;
                    
                    if (atlasIndex + 3 < m_atlasData.size() && srcIndex + channels - 1 < width * height * channels) {
                        // Copy RGB channels
                        for (uint32_t c = 0; c < std::min(channels, 3u); ++c) {
                            m_atlasData[atlasIndex + c] = srcData[srcIndex + c];
                        }
                        
                        // Set alpha channel
                        if (channels >= 4) {
                            m_atlasData[atlasIndex + 3] = srcData[srcIndex + 3];
                        } else {
                            m_atlasData[atlasIndex + 3] = 255; // Opaque
                        }
                    }
                }
            }
        }

        // Mark node as occupied
        node->occupied = true;

        // Store region
        m_regions[name] = region;
        m_needsUpdate = true;

        UpdateStats();
        LOG_INFO("Added texture to atlas: " + name + " at (" + 
                std::to_string(region.x) + ", " + std::to_string(region.y) + ")");
        return true;
    }

    bool TextureAtlas::AddTexture(const std::string& name, std::shared_ptr<Texture> texture) {
        if (!texture) {
            LOG_ERROR("Cannot add null texture to atlas: " + name);
            return false;
        }

        // For now, we'll need to read the texture data from OpenGL
        // This is a simplified implementation - in practice, you'd want to cache the original data
        LOG_WARNING("Adding Texture object to atlas requires reading back from GPU - not implemented yet");
        return false;
    }

    AtlasRegion TextureAtlas::GetRegion(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        
        auto it = m_regions.find(name);
        if (it != m_regions.end()) {
            return it->second;
        }
        
        return AtlasRegion{}; // Return empty region if not found
    }

    bool TextureAtlas::HasTexture(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        return m_regions.find(name) != m_regions.end();
    }

    void TextureAtlas::RemoveTexture(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        
        auto it = m_regions.find(name);
        if (it != m_regions.end()) {
            // Clear the region in atlas data
            const AtlasRegion& region = it->second;
            for (uint32_t y = region.y; y < region.y + region.height; ++y) {
                for (uint32_t x = region.x; x < region.x + region.width; ++x) {
                    uint32_t index = (y * m_width + x) * 4;
                    if (index + 3 < m_atlasData.size()) {
                        m_atlasData[index] = 0;     // R
                        m_atlasData[index + 1] = 0; // G
                        m_atlasData[index + 2] = 0; // B
                        m_atlasData[index + 3] = 0; // A
                    }
                }
            }
            
            m_regions.erase(it);
            m_needsUpdate = true;
            UpdateStats();
            
            LOG_INFO("Removed texture from atlas: " + name);
        }
    }

    TextureAtlasStats TextureAtlas::GetStats() const {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        return m_stats;
    }

    float TextureAtlas::GetPackingEfficiency() const {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        
        size_t usedSpace = GetUsedSpace();
        size_t totalSpace = m_width * m_height;
        
        return totalSpace > 0 ? static_cast<float>(usedSpace) / static_cast<float>(totalSpace) : 0.0f;
    }

    size_t TextureAtlas::GetUsedSpace() const {
        size_t usedSpace = 0;
        for (const auto& pair : m_regions) {
            const AtlasRegion& region = pair.second;
            usedSpace += region.width * region.height;
        }
        return usedSpace;
    }

    size_t TextureAtlas::GetWastedSpace() const {
        size_t totalSpace = m_width * m_height;
        size_t usedSpace = GetUsedSpace();
        return totalSpace - usedSpace;
    }

    void TextureAtlas::Optimize() {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        
        // Simple optimization: rebuild the atlas with better packing
        // Store current textures
        std::vector<std::pair<std::string, AtlasRegion>> currentTextures;
        for (const auto& pair : m_regions) {
            currentTextures.push_back(pair);
        }
        
        // Clear current state
        Clear();
        
        // Sort textures by size (largest first) for better packing
        std::sort(currentTextures.begin(), currentTextures.end(),
            [](const auto& a, const auto& b) {
                return (a.second.width * a.second.height) > (b.second.width * b.second.height);
            });
        
        // Re-add textures
        for (const auto& pair : currentTextures) {
            const std::string& name = pair.first;
            const AtlasRegion& oldRegion = pair.second;
            
            // Extract texture data from old position
            std::vector<uint8_t> textureData(oldRegion.width * oldRegion.height * 4);
            for (uint32_t y = 0; y < oldRegion.height; ++y) {
                for (uint32_t x = 0; x < oldRegion.width; ++x) {
                    uint32_t srcIndex = ((oldRegion.y + y) * m_width + (oldRegion.x + x)) * 4;
                    uint32_t dstIndex = (y * oldRegion.width + x) * 4;
                    
                    if (srcIndex + 3 < m_atlasData.size() && dstIndex + 3 < textureData.size()) {
                        textureData[dstIndex] = m_atlasData[srcIndex];
                        textureData[dstIndex + 1] = m_atlasData[srcIndex + 1];
                        textureData[dstIndex + 2] = m_atlasData[srcIndex + 2];
                        textureData[dstIndex + 3] = m_atlasData[srcIndex + 3];
                    }
                }
            }
            
            // Re-add texture
            AddTexture(name, textureData.data(), oldRegion.width, oldRegion.height, 4);
        }
        
        UpdateAtlasTexture();
        LOG_INFO("Atlas optimization completed. Packing efficiency: " + 
                std::to_string(GetPackingEfficiency() * 100.0f) + "%");
    }

    void TextureAtlas::Clear() {
        m_regions.clear();
        
        // Reset root node
        m_rootNode = std::make_unique<AtlasNode>();
        m_rootNode->x = 0;
        m_rootNode->y = 0;
        m_rootNode->width = m_width;
        m_rootNode->height = m_height;
        m_rootNode->occupied = false;
        
        // Clear atlas data
        std::fill(m_atlasData.begin(), m_atlasData.end(), 0);
        m_needsUpdate = true;
        
        UpdateStats();
    }

    bool TextureAtlas::IsFull() const {
        std::lock_guard<std::mutex> lock(m_atlasMutex);
        return GetPackingEfficiency() > 0.95f; // Consider full if >95% packed
    }

    AtlasNode* TextureAtlas::FindNode(AtlasNode* node, uint32_t width, uint32_t height) {
        if (!node) {
            return nullptr;
        }
        
        if (node->occupied) {
            // Try children
            AtlasNode* result = FindNode(node->left.get(), width, height);
            if (result) {
                return result;
            }
            return FindNode(node->right.get(), width, height);
        } else {
            // Check if this node can fit the texture
            if (width <= node->width && height <= node->height) {
                return node;
            }
        }
        
        return nullptr;
    }

    AtlasNode* TextureAtlas::SplitNode(AtlasNode* node, uint32_t width, uint32_t height) {
        if (!node || node->occupied) {
            return nullptr;
        }
        
        // If perfect fit, use this node
        if (node->width == width && node->height == height) {
            return node;
        }
        
        // Split the node
        node->left = std::make_unique<AtlasNode>();
        node->right = std::make_unique<AtlasNode>();
        
        // Decide how to split (horizontal or vertical)
        uint32_t dw = node->width - width;
        uint32_t dh = node->height - height;
        
        if (dw > dh) {
            // Split vertically
            node->left->x = node->x;
            node->left->y = node->y;
            node->left->width = width;
            node->left->height = node->height;
            
            node->right->x = node->x + width;
            node->right->y = node->y;
            node->right->width = node->width - width;
            node->right->height = node->height;
        } else {
            // Split horizontally
            node->left->x = node->x;
            node->left->y = node->y;
            node->left->width = node->width;
            node->left->height = height;
            
            node->right->x = node->x;
            node->right->y = node->y + height;
            node->right->width = node->width;
            node->right->height = node->height - height;
        }
        
        return SplitNode(node->left.get(), width, height);
    }

    void TextureAtlas::UpdateAtlasTexture() {
        if (m_needsUpdate && m_atlasTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, m_atlasTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_atlasData.data());
            glBindTexture(GL_TEXTURE_2D, 0);
            m_needsUpdate = false;
        }
    }

    void TextureAtlas::CalculateUVCoordinates(AtlasRegion& region) const {
        region.uvMin.x = static_cast<float>(region.x) / static_cast<float>(m_width);
        region.uvMin.y = static_cast<float>(region.y) / static_cast<float>(m_height);
        region.uvMax.x = static_cast<float>(region.x + region.width) / static_cast<float>(m_width);
        region.uvMax.y = static_cast<float>(region.y + region.height) / static_cast<float>(m_height);
    }

    void TextureAtlas::UpdateStats() const {
        m_stats.totalTextures = m_regions.size();
        m_stats.atlasCount = 1;
        m_stats.totalMemoryUsage = m_width * m_height * 4; // RGBA
        m_stats.wastedSpace = GetWastedSpace();
        m_stats.packingEfficiency = GetPackingEfficiency();
        m_stats.maxAtlasSize = m_width * m_height;
        
        if (m_stats.totalTextures > 0) {
            m_stats.averageTextureSize = GetUsedSpace() / m_stats.totalTextures;
        }
    }

    // TextureAtlasManager implementation
    TextureAtlasManager& TextureAtlasManager::GetInstance() {
        static TextureAtlasManager instance;
        return instance;
    }

    bool TextureAtlasManager::Initialize(uint32_t defaultAtlasSize, size_t maxAtlases) {
        if (m_initialized.load()) {
            LOG_WARNING("TextureAtlasManager already initialized");
            return true;
        }

        LOG_INFO("Initializing TextureAtlasManager");

        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        m_defaultAtlasSize = defaultAtlasSize;
        m_maxAtlases = maxAtlases;

        m_initialized.store(true);
        LOG_INFO("TextureAtlasManager initialized with default atlas size: " + std::to_string(defaultAtlasSize));
        return true;
    }

    void TextureAtlasManager::Shutdown() {
        if (!m_initialized.load()) {
            return;
        }

        LOG_INFO("Shutting down TextureAtlasManager");

        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        m_atlases.clear();
        m_textureToAtlasMap.clear();

        m_initialized.store(false);
        LOG_INFO("TextureAtlasManager shutdown complete");
    }

    std::shared_ptr<TextureAtlas> TextureAtlasManager::CreateAtlas(const std::string& name, uint32_t width, uint32_t height) {
        if (!m_initialized.load()) {
            LOG_ERROR("TextureAtlasManager not initialized");
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(m_managerMutex);

        if (m_atlases.find(name) != m_atlases.end()) {
            LOG_WARNING("Atlas already exists: " + name);
            return m_atlases[name];
        }

        if (m_atlases.size() >= m_maxAtlases) {
            LOG_ERROR("Maximum number of atlases reached: " + std::to_string(m_maxAtlases));
            return nullptr;
        }

        auto atlas = std::make_shared<TextureAtlas>(width, height);
        m_atlases[name] = atlas;

        LOG_INFO("Created texture atlas: " + name + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
        return atlas;
    }

    std::shared_ptr<TextureAtlas> TextureAtlasManager::GetAtlas(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        auto it = m_atlases.find(name);
        if (it != m_atlases.end()) {
            return it->second;
        }
        
        return nullptr;
    }

    void TextureAtlasManager::RemoveAtlas(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        auto it = m_atlases.find(name);
        if (it != m_atlases.end()) {
            // Remove all texture mappings for this atlas
            for (auto mapIt = m_textureToAtlasMap.begin(); mapIt != m_textureToAtlasMap.end();) {
                if (mapIt->second == name) {
                    mapIt = m_textureToAtlasMap.erase(mapIt);
                } else {
                    ++mapIt;
                }
            }
            
            m_atlases.erase(it);
            LOG_INFO("Removed texture atlas: " + name);
        }
    }

    std::vector<std::string> TextureAtlasManager::GetAtlasNames() const {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        std::vector<std::string> names;
        names.reserve(m_atlases.size());
        
        for (const auto& pair : m_atlases) {
            names.push_back(pair.first);
        }
        
        return names;
    }

    bool TextureAtlasManager::AddTextureToAtlas(const std::string& textureName, const void* data, uint32_t width, uint32_t height, uint32_t channels) {
        if (!m_initialized.load()) {
            LOG_ERROR("TextureAtlasManager not initialized");
            return false;
        }

        std::lock_guard<std::mutex> lock(m_managerMutex);

        // Check if texture already exists
        if (m_textureToAtlasMap.find(textureName) != m_textureToAtlasMap.end()) {
            LOG_WARNING("Texture already exists in atlas system: " + textureName);
            return false;
        }

        // Find best atlas for this texture
        auto atlas = FindBestAtlasForTexture(width, height);
        if (!atlas) {
            // Create new atlas if possible
            atlas = CreateNewAtlasForTexture(width, height);
            if (!atlas) {
                LOG_ERROR("Failed to create atlas for texture: " + textureName);
                return false;
            }
        }

        // Add texture to atlas
        std::string atlasName;
        for (const auto& pair : m_atlases) {
            if (pair.second == atlas) {
                atlasName = pair.first;
                break;
            }
        }

        if (atlas->AddTexture(textureName, data, width, height, channels)) {
            m_textureToAtlasMap[textureName] = atlasName;
            LOG_INFO("Added texture to atlas: " + textureName + " -> " + atlasName);
            return true;
        }

        return false;
    }

    bool TextureAtlasManager::AddTextureToAtlas(const std::string& textureName, std::shared_ptr<Texture> texture) {
        // This would require reading texture data back from GPU
        LOG_WARNING("Adding Texture object to atlas not implemented yet");
        return false;
    }

    AtlasRegion TextureAtlasManager::FindTexture(const std::string& textureName, std::string& atlasName) const {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        auto it = m_textureToAtlasMap.find(textureName);
        if (it != m_textureToAtlasMap.end()) {
            atlasName = it->second;
            auto atlasIt = m_atlases.find(atlasName);
            if (atlasIt != m_atlases.end()) {
                return atlasIt->second->GetRegion(textureName);
            }
        }
        
        atlasName.clear();
        return AtlasRegion{};
    }

    void TextureAtlasManager::OptimizeAllAtlases() {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        for (auto& pair : m_atlases) {
            pair.second->Optimize();
        }
        
        LOG_INFO("Optimized all texture atlases");
    }

    void TextureAtlasManager::CleanupEmptyAtlases() {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        std::vector<std::string> atlasesToRemove;
        
        for (const auto& pair : m_atlases) {
            if (pair.second->GetStats().totalTextures == 0) {
                atlasesToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& atlasName : atlasesToRemove) {
            m_atlases.erase(atlasName);
            LOG_INFO("Removed empty atlas: " + atlasName);
        }
    }

    TextureAtlasStats TextureAtlasManager::GetGlobalStats() const {
        std::lock_guard<std::mutex> lock(m_managerMutex);
        
        TextureAtlasStats globalStats{};
        
        for (const auto& pair : m_atlases) {
            auto atlasStats = pair.second->GetStats();
            globalStats.totalTextures += atlasStats.totalTextures;
            globalStats.atlasCount += atlasStats.atlasCount;
            globalStats.totalMemoryUsage += atlasStats.totalMemoryUsage;
            globalStats.wastedSpace += atlasStats.wastedSpace;
            
            if (atlasStats.maxAtlasSize > globalStats.maxAtlasSize) {
                globalStats.maxAtlasSize = atlasStats.maxAtlasSize;
            }
        }
        
        if (globalStats.atlasCount > 0) {
            globalStats.packingEfficiency = 1.0f - (static_cast<float>(globalStats.wastedSpace) / static_cast<float>(globalStats.totalMemoryUsage));
        }
        
        if (globalStats.totalTextures > 0) {
            globalStats.averageTextureSize = (globalStats.totalMemoryUsage - globalStats.wastedSpace) / globalStats.totalTextures;
        }
        
        return globalStats;
    }

    std::shared_ptr<TextureAtlas> TextureAtlasManager::FindBestAtlasForTexture(uint32_t width, uint32_t height) {
        std::shared_ptr<TextureAtlas> bestAtlas = nullptr;
        float bestEfficiency = 0.0f;
        
        for (const auto& pair : m_atlases) {
            auto atlas = pair.second;
            
            // Check if atlas can fit the texture
            if (atlas->IsFull()) {
                continue;
            }
            
            // Simple heuristic: prefer atlas with better packing efficiency
            float efficiency = atlas->GetPackingEfficiency();
            if (efficiency > bestEfficiency) {
                bestEfficiency = efficiency;
                bestAtlas = atlas;
            }
        }
        
        return bestAtlas;
    }

    std::shared_ptr<TextureAtlas> TextureAtlasManager::CreateNewAtlasForTexture(uint32_t width, uint32_t height) {
        if (m_atlases.size() >= m_maxAtlases) {
            return nullptr;
        }
        
        // Determine atlas size based on texture size
        uint32_t atlasSize = m_defaultAtlasSize;
        while (atlasSize < width || atlasSize < height) {
            atlasSize *= 2;
            if (atlasSize > 8192) { // Reasonable maximum
                return nullptr;
            }
        }
        
        // Generate unique atlas name
        std::string atlasName = "atlas_" + std::to_string(m_atlases.size());
        
        return CreateAtlas(atlasName, atlasSize, atlasSize);
    }
}