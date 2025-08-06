#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>

namespace GameEngine {
    class Texture;

    enum class CompressionFormat {
        None = 0,
        DXT1 = 0x83F1,      // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
        DXT3 = 0x83F2,      // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        DXT5 = 0x83F3,      // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        BC7 = 0x8E8C,       // GL_COMPRESSED_RGBA_BPTC_UNORM
        ETC2_RGB = 0x9274,  // GL_COMPRESSED_RGB8_ETC2
        ETC2_RGBA = 0x9278, // GL_COMPRESSED_RGBA8_ETC2_EAC
        ASTC_4x4 = 0x93B0,  // GL_COMPRESSED_RGBA_ASTC_4x4_KHR
        ASTC_8x8 = 0x93B7   // GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    };

    enum class CompressionQuality {
        Fast,
        Normal,
        High,
        Ultra
    };

    struct CompressionSettings {
        CompressionFormat format = CompressionFormat::None;
        CompressionQuality quality = CompressionQuality::Normal;
        bool generateMipmaps = true;
        bool preserveAlpha = true;
        float compressionRatio = 0.0f; // 0.0 = auto-detect
        bool enableMultithreading = true;
    };

    struct CompressionResult {
        bool success = false;
        CompressionFormat usedFormat = CompressionFormat::None;
        size_t originalSize = 0;
        size_t compressedSize = 0;
        float compressionRatio = 0.0f;
        float compressionTime = 0.0f;
        std::string errorMessage;
        std::vector<uint8_t> compressedData;
    };

    struct CompressionStats {
        size_t totalTexturesCompressed = 0;
        size_t totalOriginalSize = 0;
        size_t totalCompressedSize = 0;
        float averageCompressionRatio = 0.0f;
        float totalCompressionTime = 0.0f;
        size_t compressionErrors = 0;
        std::unordered_map<CompressionFormat, size_t> formatUsage;
    };

    using CompressionProgressCallback = std::function<void(const std::string& textureName, float progress)>;
    using CompressionCompleteCallback = std::function<void(const std::string& textureName, const CompressionResult& result)>;

    class TextureCompression {
    public:
        static TextureCompression& GetInstance();

        bool Initialize();
        void Shutdown();

        // Compression operations
        CompressionResult CompressTexture(const std::string& name, const void* data, 
                                        uint32_t width, uint32_t height, uint32_t channels,
                                        const CompressionSettings& settings);
        CompressionResult CompressTexture(const std::string& name, std::shared_ptr<Texture> texture,
                                        const CompressionSettings& settings);
        
        // Batch compression
        void CompressTexturesAsync(const std::vector<std::string>& textureNames,
                                 const CompressionSettings& settings,
                                 CompressionProgressCallback progressCallback = nullptr,
                                 CompressionCompleteCallback completeCallback = nullptr);
        
        // Format support detection
        bool IsFormatSupported(CompressionFormat format) const;
        std::vector<CompressionFormat> GetSupportedFormats() const;
        CompressionFormat GetBestFormat(uint32_t width, uint32_t height, uint32_t channels, 
                                      CompressionQuality quality) const;
        
        // Settings and configuration
        void SetDefaultSettings(const CompressionSettings& settings) { m_defaultSettings = settings; }
        CompressionSettings GetDefaultSettings() const { return m_defaultSettings; }
        void EnableAutoCompression(bool enable) { m_autoCompressionEnabled = enable; }
        bool IsAutoCompressionEnabled() const { return m_autoCompressionEnabled; }
        
        // Statistics
        CompressionStats GetStats() const;
        void ResetStats();
        
        // Utility functions
        size_t EstimateCompressedSize(uint32_t width, uint32_t height, CompressionFormat format) const;
        float EstimateCompressionRatio(uint32_t width, uint32_t height, uint32_t channels, 
                                     CompressionFormat format) const;
        std::string GetFormatName(CompressionFormat format) const;

    private:
        TextureCompression() = default;
        ~TextureCompression() = default;
        TextureCompression(const TextureCompression&) = delete;
        TextureCompression& operator=(const TextureCompression&) = delete;

        void DetectSupportedFormats();
        CompressionResult CompressWithDXT(const void* data, uint32_t width, uint32_t height, 
                                        uint32_t channels, CompressionFormat format, 
                                        CompressionQuality quality);
        CompressionResult CompressWithBC7(const void* data, uint32_t width, uint32_t height, 
                                        uint32_t channels, CompressionQuality quality);
        CompressionResult CompressWithETC2(const void* data, uint32_t width, uint32_t height, 
                                         uint32_t channels, CompressionFormat format, 
                                         CompressionQuality quality);
        CompressionResult CompressWithASTC(const void* data, uint32_t width, uint32_t height, 
                                         uint32_t channels, CompressionFormat format, 
                                         CompressionQuality quality);
        
        void UpdateStats(const CompressionResult& result);
        bool IsValidForCompression(uint32_t width, uint32_t height, CompressionFormat format) const;

        mutable std::mutex m_compressionMutex;
        std::vector<CompressionFormat> m_supportedFormats;
        CompressionSettings m_defaultSettings;
        bool m_autoCompressionEnabled = true;
        
        // Statistics
        mutable std::mutex m_statsMutex;
        CompressionStats m_stats;
        std::atomic<bool> m_initialized{false};
    };
}