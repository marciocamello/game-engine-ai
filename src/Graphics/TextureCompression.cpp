#include "Graphics/TextureCompression.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace GameEngine {
    TextureCompression& TextureCompression::GetInstance() {
        static TextureCompression instance;
        return instance;
    }

    bool TextureCompression::Initialize() {
        if (m_initialized.load()) {
            LOG_WARNING("TextureCompression already initialized");
            return true;
        }

        LOG_INFO("Initializing TextureCompression");

        std::lock_guard<std::mutex> lock(m_compressionMutex);
        
        // Detect supported compression formats
        DetectSupportedFormats();
        
        // Set default settings
        m_defaultSettings.format = CompressionFormat::None; // Auto-detect
        m_defaultSettings.quality = CompressionQuality::Normal;
        m_defaultSettings.generateMipmaps = true;
        m_defaultSettings.preserveAlpha = true;
        m_defaultSettings.enableMultithreading = true;

        // Initialize statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = CompressionStats{};
        }

        m_initialized.store(true);
        LOG_INFO("TextureCompression initialized with " + std::to_string(m_supportedFormats.size()) + " supported formats");
        return true;
    }

    void TextureCompression::Shutdown() {
        if (!m_initialized.load()) {
            return;
        }

        LOG_INFO("Shutting down TextureCompression");

        std::lock_guard<std::mutex> lock(m_compressionMutex);
        
        m_supportedFormats.clear();

        // Reset statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats = CompressionStats{};
        }

        m_initialized.store(false);
        LOG_INFO("TextureCompression shutdown complete");
    }

    CompressionResult TextureCompression::CompressTexture(const std::string& name, const void* data, 
                                                        uint32_t width, uint32_t height, uint32_t channels,
                                                        const CompressionSettings& settings) {
        if (!m_initialized.load()) {
            CompressionResult result;
            result.success = false;
            result.errorMessage = "TextureCompression not initialized";
            return result;
        }

        if (!data || width == 0 || height == 0 || channels == 0) {
            CompressionResult result;
            result.success = false;
            result.errorMessage = "Invalid texture data or dimensions";
            return result;
        }

        std::lock_guard<std::mutex> lock(m_compressionMutex);

        auto startTime = std::chrono::high_resolution_clock::now();

        CompressionResult result;
        result.originalSize = width * height * channels;

        // Determine compression format
        CompressionFormat format = settings.format;
        if (format == CompressionFormat::None) {
            format = GetBestFormat(width, height, channels, settings.quality);
        }

        // Check if format is supported
        if (!IsFormatSupported(format)) {
            result.success = false;
            result.errorMessage = "Compression format not supported: " + GetFormatName(format);
            UpdateStats(result);
            return result;
        }

        // Check if texture dimensions are valid for compression
        if (!IsValidForCompression(width, height, format)) {
            result.success = false;
            result.errorMessage = "Invalid texture dimensions for compression format";
            UpdateStats(result);
            return result;
        }

        // Perform compression based on format
        try {
            switch (format) {
                case CompressionFormat::DXT1:
                case CompressionFormat::DXT3:
                case CompressionFormat::DXT5:
                    result = CompressWithDXT(data, width, height, channels, format, settings.quality);
                    break;
                    
                case CompressionFormat::BC7:
                    result = CompressWithBC7(data, width, height, channels, settings.quality);
                    break;
                    
                case CompressionFormat::ETC2_RGB:
                case CompressionFormat::ETC2_RGBA:
                    result = CompressWithETC2(data, width, height, channels, format, settings.quality);
                    break;
                    
                case CompressionFormat::ASTC_4x4:
                case CompressionFormat::ASTC_8x8:
                    result = CompressWithASTC(data, width, height, channels, format, settings.quality);
                    break;
                    
                default:
                    result.success = false;
                    result.errorMessage = "Unsupported compression format";
                    break;
            }
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = "Compression failed: " + std::string(e.what());
        }

        // Calculate compression time
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        result.compressionTime = duration.count() / 1000.0f; // Convert to milliseconds

        // Calculate compression ratio
        if (result.success && result.compressedSize > 0) {
            result.compressionRatio = static_cast<float>(result.originalSize) / static_cast<float>(result.compressedSize);
            result.usedFormat = format;
        }

        UpdateStats(result);

        if (result.success) {
            LOG_INFO("Compressed texture: " + name + " (" + 
                    std::to_string(result.originalSize / 1024) + "KB -> " +
                    std::to_string(result.compressedSize / 1024) + "KB, ratio: " +
                    std::to_string(result.compressionRatio) + "x)");
        } else {
            LOG_ERROR("Failed to compress texture: " + name + " - " + result.errorMessage);
        }

        return result;
    }

    CompressionResult TextureCompression::CompressTexture(const std::string& name, std::shared_ptr<Texture> texture,
                                                        const CompressionSettings& settings) {
        CompressionResult result;
        result.success = false;
        result.errorMessage = "Compressing Texture objects not implemented yet (requires reading back from GPU)";
        return result;
    }

    void TextureCompression::CompressTexturesAsync(const std::vector<std::string>& textureNames,
                                                 const CompressionSettings& settings,
                                                 CompressionProgressCallback progressCallback,
                                                 CompressionCompleteCallback completeCallback) {
        // This is a simplified synchronous implementation
        // In a full implementation, this would use thread pools
        
        for (size_t i = 0; i < textureNames.size(); ++i) {
            const std::string& textureName = textureNames[i];
            
            if (progressCallback) {
                float progress = static_cast<float>(i) / static_cast<float>(textureNames.size());
                progressCallback(textureName, progress);
            }
            
            // Note: This would need actual texture data to compress
            // For now, just simulate the callback
            if (completeCallback) {
                CompressionResult result;
                result.success = false;
                result.errorMessage = "Async compression not fully implemented";
                completeCallback(textureName, result);
            }
        }
        
        if (progressCallback) {
            progressCallback("", 1.0f); // Signal completion
        }
    }

    bool TextureCompression::IsFormatSupported(CompressionFormat format) const {
        std::lock_guard<std::mutex> lock(m_compressionMutex);
        return std::find(m_supportedFormats.begin(), m_supportedFormats.end(), format) != m_supportedFormats.end();
    }

    std::vector<CompressionFormat> TextureCompression::GetSupportedFormats() const {
        std::lock_guard<std::mutex> lock(m_compressionMutex);
        return m_supportedFormats;
    }

    CompressionFormat TextureCompression::GetBestFormat(uint32_t width, uint32_t height, uint32_t channels, 
                                                      CompressionQuality quality) const {
        std::lock_guard<std::mutex> lock(m_compressionMutex);

        // Simple heuristic for format selection
        if (channels == 3) {
            // RGB textures
            if (IsFormatSupported(CompressionFormat::BC7)) {
                return CompressionFormat::BC7;
            } else if (IsFormatSupported(CompressionFormat::DXT1)) {
                return CompressionFormat::DXT1;
            } else if (IsFormatSupported(CompressionFormat::ETC2_RGB)) {
                return CompressionFormat::ETC2_RGB;
            }
        } else if (channels == 4) {
            // RGBA textures
            if (quality == CompressionQuality::Ultra && IsFormatSupported(CompressionFormat::BC7)) {
                return CompressionFormat::BC7;
            } else if (IsFormatSupported(CompressionFormat::DXT5)) {
                return CompressionFormat::DXT5;
            } else if (IsFormatSupported(CompressionFormat::ETC2_RGBA)) {
                return CompressionFormat::ETC2_RGBA;
            }
        }

        // Fallback to ASTC if available
        if (IsFormatSupported(CompressionFormat::ASTC_4x4)) {
            return CompressionFormat::ASTC_4x4;
        }

        return CompressionFormat::None;
    }

    CompressionStats TextureCompression::GetStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    void TextureCompression::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = CompressionStats{};
    }

    size_t TextureCompression::EstimateCompressedSize(uint32_t width, uint32_t height, CompressionFormat format) const {
        switch (format) {
            case CompressionFormat::DXT1:
                return (width * height) / 2; // 4 bits per pixel
            case CompressionFormat::DXT3:
            case CompressionFormat::DXT5:
                return width * height; // 8 bits per pixel
            case CompressionFormat::BC7:
                return width * height; // 8 bits per pixel
            case CompressionFormat::ETC2_RGB:
                return (width * height) / 2; // 4 bits per pixel
            case CompressionFormat::ETC2_RGBA:
                return width * height; // 8 bits per pixel
            case CompressionFormat::ASTC_4x4:
                return (width * height); // 8 bits per pixel
            case CompressionFormat::ASTC_8x8:
                return (width * height) / 4; // 2 bits per pixel
            default:
                return width * height * 4; // Uncompressed RGBA
        }
    }

    float TextureCompression::EstimateCompressionRatio(uint32_t width, uint32_t height, uint32_t channels, 
                                                     CompressionFormat format) const {
        size_t originalSize = width * height * channels;
        size_t compressedSize = EstimateCompressedSize(width, height, format);
        
        return compressedSize > 0 ? static_cast<float>(originalSize) / static_cast<float>(compressedSize) : 1.0f;
    }

    std::string TextureCompression::GetFormatName(CompressionFormat format) const {
        switch (format) {
            case CompressionFormat::None: return "None";
            case CompressionFormat::DXT1: return "DXT1";
            case CompressionFormat::DXT3: return "DXT3";
            case CompressionFormat::DXT5: return "DXT5";
            case CompressionFormat::BC7: return "BC7";
            case CompressionFormat::ETC2_RGB: return "ETC2_RGB";
            case CompressionFormat::ETC2_RGBA: return "ETC2_RGBA";
            case CompressionFormat::ASTC_4x4: return "ASTC_4x4";
            case CompressionFormat::ASTC_8x8: return "ASTC_8x8";
            default: return "Unknown";
        }
    }

    void TextureCompression::DetectSupportedFormats() {
        m_supportedFormats.clear();

        // Check for S3TC (DXT) support
        if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            // For now, assume basic formats are supported
            // In a real implementation, you'd check GL_EXTENSIONS
            m_supportedFormats.push_back(CompressionFormat::DXT1);
            m_supportedFormats.push_back(CompressionFormat::DXT3);
            m_supportedFormats.push_back(CompressionFormat::DXT5);
        }

        // Check for BPTC (BC7) support - requires OpenGL 4.2+
        GLint majorVersion, minorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
        
        if (majorVersion > 4 || (majorVersion == 4 && minorVersion >= 2)) {
            m_supportedFormats.push_back(CompressionFormat::BC7);
        }

        // Check for ETC2 support (OpenGL 4.3+)
        if (majorVersion > 4 || (majorVersion == 4 && minorVersion >= 3)) {
            m_supportedFormats.push_back(CompressionFormat::ETC2_RGB);
            m_supportedFormats.push_back(CompressionFormat::ETC2_RGBA);
        }

        // Check for ASTC support - simplified check
        // In a real implementation, you'd check for the specific extension
        if (majorVersion >= 4) {
            m_supportedFormats.push_back(CompressionFormat::ASTC_4x4);
            m_supportedFormats.push_back(CompressionFormat::ASTC_8x8);
        }

        LOG_INFO("Detected " + std::to_string(m_supportedFormats.size()) + " supported compression formats");
    }

    CompressionResult TextureCompression::CompressWithDXT(const void* data, uint32_t width, uint32_t height, 
                                                        uint32_t channels, CompressionFormat format, 
                                                        CompressionQuality quality) {
        CompressionResult result;
        
        // This is a placeholder implementation
        // Real DXT compression would require a library like squish or DirectXTex
        result.success = false;
        result.errorMessage = "DXT compression not implemented (requires external library)";
        
        return result;
    }

    CompressionResult TextureCompression::CompressWithBC7(const void* data, uint32_t width, uint32_t height, 
                                                        uint32_t channels, CompressionQuality quality) {
        CompressionResult result;
        
        // This is a placeholder implementation
        // Real BC7 compression would require a library like DirectXTex or Intel ISPC
        result.success = false;
        result.errorMessage = "BC7 compression not implemented (requires external library)";
        
        return result;
    }

    CompressionResult TextureCompression::CompressWithETC2(const void* data, uint32_t width, uint32_t height, 
                                                         uint32_t channels, CompressionFormat format, 
                                                         CompressionQuality quality) {
        CompressionResult result;
        
        // This is a placeholder implementation
        // Real ETC2 compression would require a library like etc2comp
        result.success = false;
        result.errorMessage = "ETC2 compression not implemented (requires external library)";
        
        return result;
    }

    CompressionResult TextureCompression::CompressWithASTC(const void* data, uint32_t width, uint32_t height, 
                                                         uint32_t channels, CompressionFormat format, 
                                                         CompressionQuality quality) {
        CompressionResult result;
        
        // This is a placeholder implementation
        // Real ASTC compression would require the ARM ASTC encoder
        result.success = false;
        result.errorMessage = "ASTC compression not implemented (requires external library)";
        
        return result;
    }

    void TextureCompression::UpdateStats(const CompressionResult& result) {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        
        if (result.success) {
            m_stats.totalTexturesCompressed++;
            m_stats.totalOriginalSize += result.originalSize;
            m_stats.totalCompressedSize += result.compressedSize;
            m_stats.totalCompressionTime += result.compressionTime;
            m_stats.formatUsage[result.usedFormat]++;
            
            // Update average compression ratio
            if (m_stats.totalOriginalSize > 0) {
                m_stats.averageCompressionRatio = static_cast<float>(m_stats.totalOriginalSize) / 
                                                static_cast<float>(m_stats.totalCompressedSize);
            }
        } else {
            m_stats.compressionErrors++;
        }
    }

    bool TextureCompression::IsValidForCompression(uint32_t width, uint32_t height, CompressionFormat format) const {
        switch (format) {
            case CompressionFormat::DXT1:
            case CompressionFormat::DXT3:
            case CompressionFormat::DXT5:
            case CompressionFormat::BC7:
                // DXT/BC formats require dimensions to be multiples of 4
                return (width % 4 == 0) && (height % 4 == 0);
                
            case CompressionFormat::ETC2_RGB:
            case CompressionFormat::ETC2_RGBA:
                // ETC2 requires dimensions to be multiples of 4
                return (width % 4 == 0) && (height % 4 == 0);
                
            case CompressionFormat::ASTC_4x4:
                // ASTC 4x4 requires dimensions to be multiples of 4
                return (width % 4 == 0) && (height % 4 == 0);
                
            case CompressionFormat::ASTC_8x8:
                // ASTC 8x8 requires dimensions to be multiples of 8
                return (width % 8 == 0) && (height % 8 == 0);
                
            default:
                return true;
        }
    }
}