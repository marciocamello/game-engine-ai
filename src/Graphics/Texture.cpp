#include "Graphics/Texture.h"
#include "Resource/TextureLoader.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace GameEngine {

    Texture::Texture() {
    }

    Texture::~Texture() {
        if (m_textureID != 0) {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
            LOG_INFO("Deleted texture with ID: " + std::to_string(m_textureID));
        }
    }

    bool Texture::LoadFromFile(const std::string& filepath) {
        // Clean up existing texture if any
        if (m_textureID != 0) {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        }

        // Use TextureLoader to load the image
        TextureLoader loader;
        TextureLoader::ImageData imageData = loader.LoadImageData(filepath);

        if (!imageData.isValid) {
            LOG_ERROR("Failed to load texture from file: " + filepath);
            // Create default pink/magenta texture
            CreateDefaultTexture();
            return false;
        }

        // Create OpenGL texture
        m_textureID = loader.CreateOpenGLTexture(imageData);
        if (m_textureID == 0) {
            LOG_ERROR("Failed to create OpenGL texture for: " + filepath);
            CreateDefaultTexture();
            return false;
        }

        // Store texture properties
        m_width = imageData.width;
        m_height = imageData.height;
        m_channels = imageData.channels;
        m_format = GetTextureFormatFromChannels(imageData.channels);
        m_filepath = filepath;

        LOG_INFO("Successfully loaded texture: " + filepath + " (ID: " + std::to_string(m_textureID) + ")");
        return true;
    }

    bool Texture::CreateEmpty(int width, int height, TextureFormat format) {
        // Clean up existing texture if any
        if (m_textureID != 0) {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        }

        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Create empty texture
        uint32_t glFormat = GetGLFormat(format);
        uint32_t glInternalFormat = GetGLInternalFormat(format);
        uint32_t glType = (format == TextureFormat::Depth || format == TextureFormat::DepthStencil) ? GL_FLOAT : GL_UNSIGNED_BYTE;

        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glFormat, glType, nullptr);

        glBindTexture(GL_TEXTURE_2D, 0);

        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_ERROR("OpenGL error creating empty texture: " + std::to_string(error));
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
            return false;
        }

        // Store texture properties
        m_width = width;
        m_height = height;
        m_channels = GetChannelsFromFormat(format);
        m_format = format;
        m_filepath = "";

        LOG_INFO("Created empty texture (ID: " + std::to_string(m_textureID) + ", " + 
                 std::to_string(width) + "x" + std::to_string(height) + ")");
        return true;
    }

    void Texture::Bind(uint32_t slot) const {
        if (m_textureID == 0) {
            return; // Silently ignore invalid textures
        }
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    }

    void Texture::Unbind() const {
        // Only unbind if we have a valid OpenGL context
        // In unit tests without OpenGL context, this should be safe to call
        try {
            glBindTexture(GL_TEXTURE_2D, 0);
        } catch (...) {
            // Silently ignore OpenGL errors in unit tests
        }
    }

    void Texture::SetFilter(TextureFilter minFilter, TextureFilter magFilter) {
        if (m_textureID == 0) {
            return; // Silently ignore invalid textures
        }

        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLFilter(minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFilter(magFilter));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::SetWrap(TextureWrap wrapS, TextureWrap wrapT) {
        if (m_textureID == 0) {
            return; // Silently ignore invalid textures
        }

        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrap(wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrap(wrapT));
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::GenerateMipmaps() {
        if (m_textureID == 0) {
            return; // Silently ignore invalid textures
        }

        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::CreateDefaultTexture() {
        // Create a 2x2 pink/magenta texture for missing files
        const int width = 2;
        const int height = 2;
        
        // Pink/magenta checkerboard pattern (RGBA)
        unsigned char defaultData[] = {
            255, 0, 255, 255,  // Magenta
            0, 0, 0, 255,      // Black
            0, 0, 0, 255,      // Black  
            255, 0, 255, 255   // Magenta
        };

        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // Set texture parameters for pixel art (no filtering)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultData);

        glBindTexture(GL_TEXTURE_2D, 0);

        // Store texture properties
        m_width = width;
        m_height = height;
        m_channels = 4;
        m_format = TextureFormat::RGBA;
        m_filepath = "[DEFAULT_TEXTURE]";

        LOG_INFO("Created default pink/magenta texture (ID: " + std::to_string(m_textureID) + ")");
    }

    uint32_t Texture::GetGLFormat(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB: return GL_RGB;
            case TextureFormat::RGBA: return GL_RGBA;
            case TextureFormat::Depth: return GL_DEPTH_COMPONENT;
            case TextureFormat::DepthStencil: return GL_DEPTH_STENCIL;
            default: return GL_RGBA;
        }
    }

    uint32_t Texture::GetGLInternalFormat(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB: return GL_RGB8;
            case TextureFormat::RGBA: return GL_RGBA8;
            case TextureFormat::Depth: return GL_DEPTH_COMPONENT24;
            case TextureFormat::DepthStencil: return GL_DEPTH24_STENCIL8;
            default: return GL_RGBA8;
        }
    }

    uint32_t Texture::GetGLFilter(TextureFilter filter) {
        switch (filter) {
            case TextureFilter::Nearest: return GL_NEAREST;
            case TextureFilter::Linear: return GL_LINEAR;
            case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
            case TextureFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
            case TextureFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
            case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
            default: return GL_LINEAR;
        }
    }

    uint32_t Texture::GetGLWrap(TextureWrap wrap) {
        switch (wrap) {
            case TextureWrap::Repeat: return GL_REPEAT;
            case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
            case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
            case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
            default: return GL_REPEAT;
        }
    }

    TextureFormat Texture::GetTextureFormatFromChannels(int channels) {
        switch (channels) {
            case 1: return TextureFormat::RGB; // Grayscale treated as RGB
            case 3: return TextureFormat::RGB;
            case 4: return TextureFormat::RGBA;
            default: 
                LOG_WARNING("Unsupported channel count: " + std::to_string(channels) + ", defaulting to RGBA");
                return TextureFormat::RGBA;
        }
    }

    int Texture::GetChannelsFromFormat(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB: return 3;
            case TextureFormat::RGBA: return 4;
            case TextureFormat::Depth: return 1;
            case TextureFormat::DepthStencil: return 2;
            default: return 4;
        }
    }
}