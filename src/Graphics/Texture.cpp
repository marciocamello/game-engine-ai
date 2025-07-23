#include "Graphics/Texture.h"
#include "Resource/TextureLoader.h"
#include "Core/Logger.h"
#include "Core/OpenGLContext.h"
#include <glad/glad.h>

namespace GameEngine {

    Texture::Texture(const std::string& path) : Resource(path) {
    }

    Texture::~Texture() {
        if (m_textureID != 0 && OpenGLContext::HasActiveContext()) {
            glDeleteTextures(1, &m_textureID);
            LOG_INFO("Deleted texture with ID: " + std::to_string(m_textureID));
        }
        m_textureID = 0;
        m_gpuResourcesCreated = false;
    }

    bool Texture::LoadFromFile(const std::string& filepath) {
        // Clean up existing GPU resources if any
        if (m_textureID != 0 && OpenGLContext::HasActiveContext()) {
            glDeleteTextures(1, &m_textureID);
        }
        m_textureID = 0;
        m_gpuResourcesCreated = false;

        // Use TextureLoader to load the image data (CPU only)
        TextureLoader loader;
        TextureLoader::ImageData imageData = loader.LoadImageData(filepath);

        if (!imageData.isValid) {
            LOG_ERROR("Failed to load texture from file: " + filepath);
            // Set default properties for fallback
            m_width = 2;
            m_height = 2;
            m_channels = 4;
            m_format = TextureFormat::RGBA;
            m_filepath = "[DEFAULT_TEXTURE]";
            
            // Create default image data (pink/magenta checkerboard)
            m_imageData = {
                255, 0, 255, 255,  // Magenta
                0, 0, 0, 255,      // Black
                0, 0, 0, 255,      // Black  
                255, 0, 255, 255   // Magenta
            };
            return false;
        }

        // Store texture properties (CPU data)
        m_width = imageData.width;
        m_height = imageData.height;
        m_channels = imageData.channels;
        m_format = GetTextureFormatFromChannels(imageData.channels);
        m_filepath = filepath;
        
        // Store image data for lazy GPU resource creation
        if (imageData.data && imageData.width > 0 && imageData.height > 0) {
            size_t dataSize = static_cast<size_t>(imageData.width) * imageData.height * imageData.channels;
            m_imageData.assign(imageData.data, imageData.data + dataSize);
        }

        LOG_INFO("Successfully loaded texture data: " + filepath + " (" + 
                 std::to_string(m_width) + "x" + std::to_string(m_height) + ")");
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
        EnsureGPUResourcesCreated();
        if (m_textureID == 0) {
            return; // Silently ignore invalid textures or no OpenGL context
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
        EnsureGPUResourcesCreated();
        if (m_textureID == 0) {
            return; // Silently ignore invalid textures or no OpenGL context
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

    uint32_t Texture::GetGLFormat(TextureFormat format) const {
        switch (format) {
            case TextureFormat::RGB: return GL_RGB;
            case TextureFormat::RGBA: return GL_RGBA;
            case TextureFormat::Depth: return GL_DEPTH_COMPONENT;
            case TextureFormat::DepthStencil: return GL_DEPTH_STENCIL;
            default: return GL_RGBA;
        }
    }

    uint32_t Texture::GetGLInternalFormat(TextureFormat format) const {
        switch (format) {
            case TextureFormat::RGB: return GL_RGB8;
            case TextureFormat::RGBA: return GL_RGBA8;
            case TextureFormat::Depth: return GL_DEPTH_COMPONENT24;
            case TextureFormat::DepthStencil: return GL_DEPTH24_STENCIL8;
            default: return GL_RGBA8;
        }
    }

    uint32_t Texture::GetGLFilter(TextureFilter filter) const {
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

    uint32_t Texture::GetGLWrap(TextureWrap wrap) const {
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

    size_t Texture::GetMemoryUsage() const {
        // Base resource memory usage
        size_t baseSize = Resource::GetMemoryUsage();
        
        // Calculate texture memory usage based on dimensions and format
        size_t textureSize = 0;
        if (m_textureID != 0) {
            // Calculate bytes per pixel based on format
            size_t bytesPerPixel = 0;
            switch (m_format) {
                case TextureFormat::RGB: bytesPerPixel = 3; break;
                case TextureFormat::RGBA: bytesPerPixel = 4; break;
                case TextureFormat::Depth: bytesPerPixel = 4; break; // 32-bit depth
                case TextureFormat::DepthStencil: bytesPerPixel = 4; break; // 24-bit depth + 8-bit stencil
                default: bytesPerPixel = 4; break;
            }
            
            textureSize = static_cast<size_t>(m_width) * static_cast<size_t>(m_height) * bytesPerPixel;
            
            // Account for mipmaps (approximately 1/3 additional memory)
            textureSize = textureSize + (textureSize / 3);
        }
        
        // Add object overhead
        size_t objectSize = sizeof(*this);
        
        return baseSize + textureSize + objectSize;
    }
    
    void Texture::EnsureGPUResourcesCreated() const {
        if (m_gpuResourcesCreated || !OpenGLContext::HasActiveContext()) {
            return;
        }
        
        CreateGPUResources();
        m_gpuResourcesCreated = true;
    }
    
    void Texture::CreateGPUResources() const {
        if (!OpenGLContext::HasActiveContext()) {
            LOG_WARNING("Cannot create texture GPU resources: No OpenGL context available");
            return;
        }
        
        // Generate OpenGL texture
        glGenTextures(1, &m_textureID);
        if (m_textureID == 0) {
            LOG_ERROR("Failed to generate OpenGL texture ID");
            return;
        }
        
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        
        // Set default texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Upload texture data if available
        if (!m_imageData.empty()) {
            uint32_t glFormat = GetGLFormat(m_format);
            uint32_t glInternalFormat = GetGLInternalFormat(m_format);
            
            glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, m_width, m_height, 0, 
                        glFormat, GL_UNSIGNED_BYTE, m_imageData.data());
        } else {
            // Create empty texture
            uint32_t glFormat = GetGLFormat(m_format);
            uint32_t glInternalFormat = GetGLInternalFormat(m_format);
            
            glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, m_width, m_height, 0, 
                        glFormat, GL_UNSIGNED_BYTE, nullptr);
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_ERROR("OpenGL error creating texture GPU resources: " + std::to_string(error));
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        } else {
            LOG_INFO("Created GPU resources for texture: " + m_filepath + " (ID: " + std::to_string(m_textureID) + ")");
        }
    }

}