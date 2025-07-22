#pragma once

#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include <string>
#include <memory>

namespace GameEngine {
    /**
     * TextureLoader handles loading image files using STB image library
     * Supports PNG, JPG, TGA formats with automatic format detection
     */
    class TextureLoader {
    public:
        struct ImageData {
            unsigned char* data = nullptr;
            int width = 0;
            int height = 0;
            int channels = 0;
            bool isValid = false;
            
            // Default constructor
            ImageData() = default;
            
            ~ImageData() {
                if (data) {
                    FreeImageData();
                }
            }
            
            // Move constructor
            ImageData(ImageData&& other) noexcept 
                : data(other.data), width(other.width), height(other.height), 
                  channels(other.channels), isValid(other.isValid) {
                other.data = nullptr;
                other.isValid = false;
            }
            
            // Move assignment
            ImageData& operator=(ImageData&& other) noexcept {
                if (this != &other) {
                    if (data) {
                        FreeImageData();
                    }
                    data = other.data;
                    width = other.width;
                    height = other.height;
                    channels = other.channels;
                    isValid = other.isValid;
                    
                    other.data = nullptr;
                    other.isValid = false;
                }
                return *this;
            }
            
            // Delete copy constructor and assignment
            ImageData(const ImageData&) = delete;
            ImageData& operator=(const ImageData&) = delete;
            
        private:
            void FreeImageData();
        };

        TextureLoader();
        ~TextureLoader();

        /**
         * Load image data from file using STB
         * @param filepath Path to image file
         * @return ImageData structure with loaded data
         */
        ImageData LoadImageData(const std::string& filepath);

        /**
         * Create OpenGL texture from image data
         * @param imageData Loaded image data
         * @return OpenGL texture ID (0 if failed)
         */
        uint32_t CreateOpenGLTexture(const ImageData& imageData);

        /**
         * Load texture directly from file
         * @param filepath Path to image file
         * @return Shared pointer to loaded texture
         */
        std::shared_ptr<Texture> LoadTexture(const std::string& filepath);

        /**
         * Create default pink/magenta texture for missing files
         * @return Shared pointer to default texture
         */
        std::shared_ptr<Texture> CreateDefaultTexture();

        /**
         * Check if file format is supported
         * @param filepath Path to check
         * @return True if format is supported
         */
        static bool IsSupportedFormat(const std::string& filepath);

        /**
         * Get format from file extension
         * @param filepath Path to analyze
         * @return TextureFormat enum value
         */
        static TextureFormat GetFormatFromFile(const std::string& filepath);

    private:
        /**
         * Convert STB channels to TextureFormat
         * @param channels Number of channels from STB
         * @return Corresponding TextureFormat
         */
        TextureFormat GetTextureFormat(int channels);

        /**
         * Get OpenGL internal format from channels
         * @param channels Number of channels
         * @return OpenGL internal format constant
         */
        uint32_t GetOpenGLInternalFormat(int channels);

        /**
         * Get OpenGL format from channels
         * @param channels Number of channels
         * @return OpenGL format constant
         */
        uint32_t GetOpenGLFormat(int channels);
    };
}