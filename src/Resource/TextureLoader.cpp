#include "Resource/TextureLoader.h"
#include "Core/Logger.h"
#include <filesystem>
#include <algorithm>

#ifdef GAMEENGINE_HAS_STB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include <glad/glad.h>

namespace GameEngine {
    
    void TextureLoader::ImageData::FreeImageData() {
#ifdef GAMEENGINE_HAS_STB
        if (data) {
            stbi_image_free(data);
            data = nullptr;
        }
#endif
    }

    TextureLoader::TextureLoader() {
#ifdef GAMEENGINE_HAS_STB
        // Set STB to flip images vertically to match OpenGL coordinate system
        stbi_set_flip_vertically_on_load(true);
#endif
    }

    TextureLoader::~TextureLoader() {
    }

    TextureLoader::ImageData TextureLoader::LoadImageData(const std::string& filepath) {
        ImageData imageData;

#ifndef GAMEENGINE_HAS_STB
        LOG_ERROR("STB not available - cannot load image: " + filepath);
        return imageData;
#endif

        if (!std::filesystem::exists(filepath)) {
            LOG_ERROR("Image file not found: " + filepath);
            return imageData;
        }

        if (!IsSupportedFormat(filepath)) {
            LOG_ERROR("Unsupported image format: " + filepath);
            return imageData;
        }

#ifdef GAMEENGINE_HAS_STB
        // Load image using STB
        imageData.data = stbi_load(filepath.c_str(), &imageData.width, &imageData.height, &imageData.channels, 0);
        
        if (!imageData.data) {
            LOG_ERROR("Failed to load image: " + filepath + " - " + std::string(stbi_failure_reason()));
            return imageData;
        }

        imageData.isValid = true;
        LOG_INFO("Loaded image: " + filepath + " (" + std::to_string(imageData.width) + "x" + 
                 std::to_string(imageData.height) + ", " + std::to_string(imageData.channels) + " channels)");
#endif

        return imageData;
    }

    uint32_t TextureLoader::CreateOpenGLTexture(const ImageData& imageData) {
        if (!imageData.isValid || !imageData.data) {
            LOG_ERROR("Invalid image data provided to CreateOpenGLTexture");
            return 0;
        }

        uint32_t textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload texture data
        uint32_t internalFormat = GetOpenGLInternalFormat(imageData.channels);
        uint32_t format = GetOpenGLFormat(imageData.channels);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageData.width, imageData.height, 
                     0, format, GL_UNSIGNED_BYTE, imageData.data);
        
        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_ERROR("OpenGL error creating texture: " + std::to_string(error));
            glDeleteTextures(1, &textureID);
            return 0;
        }

        LOG_INFO("Created OpenGL texture with ID: " + std::to_string(textureID));
        return textureID;
    }

    std::shared_ptr<Texture> TextureLoader::LoadTexture(const std::string& filepath) {
        // Load image data
        ImageData imageData = LoadImageData(filepath);
        
        if (!imageData.isValid) {
            LOG_WARNING("Failed to load texture from file: " + filepath + ", using default texture");
            return CreateDefaultTexture();
        }

        // Create texture object
        auto texture = std::make_shared<Texture>();
        
        // Create OpenGL texture
        uint32_t textureID = CreateOpenGLTexture(imageData);
        if (textureID == 0) {
            LOG_WARNING("Failed to create OpenGL texture for: " + filepath + ", using default texture");
            return CreateDefaultTexture();
        }

        // Set texture properties (we'll need to modify Texture class to accept these)
        // For now, we'll create a basic texture and set the ID manually
        // This will be enhanced in task 6.2
        
        LOG_INFO("Successfully loaded texture: " + filepath);
        return texture;
    }

    std::shared_ptr<Texture> TextureLoader::CreateDefaultTexture() {
        // Create a 2x2 pink/magenta texture
        const int width = 2;
        const int height = 2;
        const int channels = 3; // RGB
        
        // Pink/magenta color data (RGB)
        unsigned char defaultData[] = {
            255, 0, 255,  // Magenta
            0, 0, 0,      // Black
            0, 0, 0,      // Black  
            255, 0, 255   // Magenta
        };

        uint32_t textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture parameters for pixel art (no filtering)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, defaultData);

        glBindTexture(GL_TEXTURE_2D, 0);

        // Create texture object
        auto texture = std::make_shared<Texture>();
        
        LOG_INFO("Created default pink/magenta texture with ID: " + std::to_string(textureID));
        return texture;
    }

    bool TextureLoader::IsSupportedFormat(const std::string& filepath) {
        std::string extension = std::filesystem::path(filepath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
               extension == ".tga" || extension == ".bmp";
    }

    TextureFormat TextureLoader::GetFormatFromFile(const std::string& filepath) {
        std::string extension = std::filesystem::path(filepath).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // Most image formats support RGBA, default to RGBA
        return TextureFormat::RGBA;
    }

    TextureFormat TextureLoader::GetTextureFormat(int channels) {
        switch (channels) {
            case 1: return TextureFormat::RGB; // Grayscale treated as RGB
            case 3: return TextureFormat::RGB;
            case 4: return TextureFormat::RGBA;
            default: 
                LOG_WARNING("Unsupported channel count: " + std::to_string(channels) + ", defaulting to RGBA");
                return TextureFormat::RGBA;
        }
    }

    uint32_t TextureLoader::GetOpenGLInternalFormat(int channels) {
        switch (channels) {
            case 1: return GL_RGB; // Grayscale as RGB
            case 3: return GL_RGB;
            case 4: return GL_RGBA;
            default: 
                LOG_WARNING("Unsupported channel count for internal format: " + std::to_string(channels));
                return GL_RGBA;
        }
    }

    uint32_t TextureLoader::GetOpenGLFormat(int channels) {
        switch (channels) {
            case 1: return GL_RED;
            case 3: return GL_RGB;
            case 4: return GL_RGBA;
            default:
                LOG_WARNING("Unsupported channel count for format: " + std::to_string(channels));
                return GL_RGBA;
        }
    }
}