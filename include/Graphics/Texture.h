#pragma once

#include <string>

namespace GameEngine {
    enum class TextureFormat {
        RGB,
        RGBA,
        Depth,
        DepthStencil
    };

    enum class TextureFilter {
        Nearest,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear
    };

    enum class TextureWrap {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };

    class Texture {
    public:
        Texture();
        ~Texture();

        bool LoadFromFile(const std::string& filepath);
        bool CreateEmpty(int width, int height, TextureFormat format = TextureFormat::RGBA);
        
        void Bind(uint32_t slot = 0) const;
        void Unbind() const;
        
        void SetFilter(TextureFilter minFilter, TextureFilter magFilter);
        void SetWrap(TextureWrap wrapS, TextureWrap wrapT);
        void GenerateMipmaps();
        
        uint32_t GetID() const { return m_textureID; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        int GetChannels() const { return m_channels; }
        TextureFormat GetFormat() const { return m_format; }
        
        bool IsValid() const { return m_textureID != 0; }

    private:
        void CreateDefaultTexture();
        
        uint32_t GetGLFormat(TextureFormat format);
        uint32_t GetGLInternalFormat(TextureFormat format);
        uint32_t GetGLFilter(TextureFilter filter);
        uint32_t GetGLWrap(TextureWrap wrap);
        
        TextureFormat GetTextureFormatFromChannels(int channels);
        int GetChannelsFromFormat(TextureFormat format);
        
        uint32_t m_textureID = 0;
        int m_width = 0;
        int m_height = 0;
        int m_channels = 0;
        TextureFormat m_format = TextureFormat::RGBA;
        std::string m_filepath;
    };
}