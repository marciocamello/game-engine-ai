#include "Graphics/PostProcessEffects.h"
#include "Graphics/Shader.h"
#include "Core/Logger.h"
#include <glad/glad.h>

namespace GameEngine {

    // ToneMappingEffect Implementation
    ToneMappingEffect::ToneMappingEffect() = default;

    ToneMappingEffect::~ToneMappingEffect() {
        Shutdown();
    }

    bool ToneMappingEffect::Initialize(int width, int height) {
        m_width = width;
        m_height = height;

        CreateShaders();

        if (!m_shader || !m_shader->IsValid()) {
            LOG_ERROR("Failed to create tone mapping shader");
            return false;
        }

        LOG_INFO("ToneMappingEffect initialized");
        return true;
    }

    void ToneMappingEffect::Shutdown() {
        m_shader.reset();
        LOG_INFO("ToneMappingEffect shutdown");
    }

    void ToneMappingEffect::Resize(int width, int height) {
        m_width = width;
        m_height = height;
        // No additional resize logic needed for tone mapping
    }

    void ToneMappingEffect::Process(uint32_t inputTexture, uint32_t outputTexture) {
        if (!m_shader || !m_shader->IsValid() || !m_enabled) {
            return;
        }

        // Bind output framebuffer if not rendering to screen
        if (outputTexture != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, outputTexture);
        }

        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->Use();
        UpdateShaderUniforms();

        // Bind input texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);
        m_shader->SetUniform("u_inputTexture", 0);

        // Render fullscreen quad (this would need to be implemented)
        // For now, we'll use a simple approach
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(2.0f, 0.0f); glVertex2f(3.0f, -1.0f);
        glTexCoord2f(0.0f, 2.0f); glVertex2f(-1.0f, 3.0f);
        glEnd();

        m_shader->Unuse();
        glBindTexture(GL_TEXTURE_2D, 0);
        
        if (outputTexture != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    const std::string& ToneMappingEffect::GetName() const {
        return m_name;
    }

    void ToneMappingEffect::SetParameter(const std::string& name, float value) {
        if (name == "exposure") {
            SetExposure(value);
        } else if (name == "gamma") {
            SetGamma(value);
        } else if (name == "type") {
            SetToneMappingType(static_cast<ToneMappingType>(static_cast<int>(value)));
        }
    }

    void ToneMappingEffect::CreateShaders() {
        m_shader = std::make_shared<Shader>();

        std::string vertexSource = R"(
            #version 460 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            
            out vec2 TexCoord;
            
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        std::string fragmentSource = R"(
            #version 460 core
            in vec2 TexCoord;
            out vec4 FragColor;
            
            uniform sampler2D u_inputTexture;
            uniform float u_exposure;
            uniform float u_gamma;
            uniform int u_toneMappingType;
            
            // Reinhard tone mapping
            vec3 reinhard(vec3 color) {
                return color / (color + vec3(1.0));
            }
            
            // ACES tone mapping
            vec3 aces(vec3 color) {
                const float a = 2.51;
                const float b = 0.03;
                const float c = 2.43;
                const float d = 0.59;
                const float e = 0.14;
                return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
            }
            
            // Filmic tone mapping (Uncharted 2)
            vec3 filmic(vec3 color) {
                const float A = 0.15;
                const float B = 0.50;
                const float C = 0.10;
                const float D = 0.20;
                const float E = 0.02;
                const float F = 0.30;
                
                vec3 x = color;
                return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
            }
            
            void main() {
                vec3 color = texture(u_inputTexture, TexCoord).rgb;
                
                // Apply exposure
                color *= u_exposure;
                
                // Apply tone mapping
                if (u_toneMappingType == 1) {
                    color = reinhard(color);
                } else if (u_toneMappingType == 2) {
                    color = aces(color);
                } else if (u_toneMappingType == 3) {
                    color = filmic(color);
                }
                
                // Apply gamma correction
                color = pow(color, vec3(1.0 / u_gamma));
                
                FragColor = vec4(color, 1.0);
            }
        )";

        if (!m_shader->LoadFromSource(vertexSource, fragmentSource)) {
            LOG_ERROR("Failed to compile tone mapping shader");
        }
    }

    void ToneMappingEffect::UpdateShaderUniforms() {
        if (m_shader && m_shader->IsValid()) {
            m_shader->SetUniform("u_exposure", m_exposure);
            m_shader->SetUniform("u_gamma", m_gamma);
            m_shader->SetUniform("u_toneMappingType", static_cast<int>(m_toneMappingType));
        }
    }

    // FXAAEffect Implementation
    FXAAEffect::FXAAEffect() = default;

    FXAAEffect::~FXAAEffect() {
        Shutdown();
    }

    bool FXAAEffect::Initialize(int width, int height) {
        m_width = width;
        m_height = height;

        CreateShaders();

        if (!m_shader || !m_shader->IsValid()) {
            LOG_ERROR("Failed to create FXAA shader");
            return false;
        }

        LOG_INFO("FXAAEffect initialized");
        return true;
    }

    void FXAAEffect::Shutdown() {
        m_shader.reset();
        LOG_INFO("FXAAEffect shutdown");
    }

    void FXAAEffect::Resize(int width, int height) {
        m_width = width;
        m_height = height;
    }

    void FXAAEffect::Process(uint32_t inputTexture, uint32_t outputTexture) {
        if (!m_shader || !m_shader->IsValid() || !m_enabled) {
            return;
        }

        // Bind output framebuffer if not rendering to screen
        if (outputTexture != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, outputTexture);
        }

        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->Use();
        UpdateShaderUniforms();

        // Bind input texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);
        m_shader->SetUniform("u_inputTexture", 0);

        // Render fullscreen quad
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(2.0f, 0.0f); glVertex2f(3.0f, -1.0f);
        glTexCoord2f(0.0f, 2.0f); glVertex2f(-1.0f, 3.0f);
        glEnd();

        m_shader->Unuse();
        glBindTexture(GL_TEXTURE_2D, 0);
        
        if (outputTexture != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    const std::string& FXAAEffect::GetName() const {
        return m_name;
    }

    void FXAAEffect::SetParameter(const std::string& name, float value) {
        if (name == "quality") {
            SetQuality(value);
        } else if (name == "subPixelShift") {
            SetSubPixelShift(value);
        } else if (name == "edgeThreshold") {
            SetEdgeThreshold(value);
        } else if (name == "edgeThresholdMin") {
            SetEdgeThresholdMin(value);
        }
    }

    void FXAAEffect::CreateShaders() {
        m_shader = std::make_shared<Shader>();

        std::string vertexSource = R"(
            #version 460 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            
            out vec2 TexCoord;
            
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        std::string fragmentSource = R"(
            #version 460 core
            in vec2 TexCoord;
            out vec4 FragColor;
            
            uniform sampler2D u_inputTexture;
            uniform vec2 u_texelSize;
            uniform float u_quality;
            uniform float u_subPixelShift;
            uniform float u_edgeThreshold;
            uniform float u_edgeThresholdMin;
            
            void main() {
                vec2 texelSize = u_texelSize;
                vec3 rgbNW = texture(u_inputTexture, TexCoord + vec2(-1.0, -1.0) * texelSize).rgb;
                vec3 rgbNE = texture(u_inputTexture, TexCoord + vec2(1.0, -1.0) * texelSize).rgb;
                vec3 rgbSW = texture(u_inputTexture, TexCoord + vec2(-1.0, 1.0) * texelSize).rgb;
                vec3 rgbSE = texture(u_inputTexture, TexCoord + vec2(1.0, 1.0) * texelSize).rgb;
                vec3 rgbM = texture(u_inputTexture, TexCoord).rgb;
                
                vec3 luma = vec3(0.299, 0.587, 0.114);
                float lumaNW = dot(rgbNW, luma);
                float lumaNE = dot(rgbNE, luma);
                float lumaSW = dot(rgbSW, luma);
                float lumaSE = dot(rgbSE, luma);
                float lumaM = dot(rgbM, luma);
                
                float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
                float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
                
                vec2 dir;
                dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
                dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));
                
                float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * u_subPixelShift), u_edgeThresholdMin);
                float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
                
                dir = min(vec2(8.0, 8.0), max(vec2(-8.0, -8.0), dir * rcpDirMin)) * texelSize;
                
                vec3 rgbA = 0.5 * (
                    texture(u_inputTexture, TexCoord + dir * (1.0/3.0 - 0.5)).rgb +
                    texture(u_inputTexture, TexCoord + dir * (2.0/3.0 - 0.5)).rgb);
                vec3 rgbB = rgbA * 0.5 + 0.25 * (
                    texture(u_inputTexture, TexCoord + dir * -0.5).rgb +
                    texture(u_inputTexture, TexCoord + dir * 0.5).rgb);
                
                float lumaB = dot(rgbB, luma);
                
                if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
                    FragColor = vec4(rgbA, 1.0);
                } else {
                    FragColor = vec4(rgbB, 1.0);
                }
            }
        )";

        if (!m_shader->LoadFromSource(vertexSource, fragmentSource)) {
            LOG_ERROR("Failed to compile FXAA shader");
        }
    }

    void FXAAEffect::UpdateShaderUniforms() {
        if (m_shader && m_shader->IsValid()) {
            m_shader->SetUniform("u_texelSize", Math::Vec2(1.0f / m_width, 1.0f / m_height));
            m_shader->SetUniform("u_quality", m_quality);
            m_shader->SetUniform("u_subPixelShift", m_subPixelShift);
            m_shader->SetUniform("u_edgeThreshold", m_edgeThreshold);
            m_shader->SetUniform("u_edgeThresholdMin", m_edgeThresholdMin);
        }
    }

    // BloomEffect Implementation
    BloomEffect::BloomEffect() = default;

    BloomEffect::~BloomEffect() {
        Shutdown();
    }

    bool BloomEffect::Initialize(int width, int height) {
        m_width = width;
        m_height = height;

        CreateShaders();
        CreateFramebuffers();

        if (!m_brightPassShader || !m_brightPassShader->IsValid() ||
            !m_blurShader || !m_blurShader->IsValid() ||
            !m_combineShader || !m_combineShader->IsValid()) {
            LOG_ERROR("Failed to create bloom shaders");
            return false;
        }

        LOG_INFO("BloomEffect initialized");
        return true;
    }

    void BloomEffect::Shutdown() {
        m_brightPassShader.reset();
        m_blurShader.reset();
        m_combineShader.reset();

        if (m_brightPassFBO != 0) {
            glDeleteFramebuffers(1, &m_brightPassFBO);
            m_brightPassFBO = 0;
        }
        if (m_brightPassTexture != 0) {
            glDeleteTextures(1, &m_brightPassTexture);
            m_brightPassTexture = 0;
        }
        
        for (int i = 0; i < 2; ++i) {
            if (m_blurFBO[i] != 0) {
                glDeleteFramebuffers(1, &m_blurFBO[i]);
                m_blurFBO[i] = 0;
            }
            if (m_blurTexture[i] != 0) {
                glDeleteTextures(1, &m_blurTexture[i]);
                m_blurTexture[i] = 0;
            }
        }

        LOG_INFO("BloomEffect shutdown");
    }

    void BloomEffect::Resize(int width, int height) {
        m_width = width;
        m_height = height;
        CreateFramebuffers(); // Recreate framebuffers with new size
    }

    void BloomEffect::Process(uint32_t inputTexture, uint32_t outputTexture) {
        if (!m_enabled || !m_brightPassShader || !m_blurShader || !m_combineShader) {
            return;
        }

        // Step 1: Bright pass - extract bright pixels
        glBindFramebuffer(GL_FRAMEBUFFER, m_brightPassFBO);
        glViewport(0, 0, m_width / 2, m_height / 2); // Half resolution for performance
        glClear(GL_COLOR_BUFFER_BIT);

        m_brightPassShader->Use();
        m_brightPassShader->SetUniform("u_threshold", m_threshold);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);
        m_brightPassShader->SetUniform("u_inputTexture", 0);
        RenderFullscreenQuad();
        m_brightPassShader->Unuse();

        // Step 2: Blur passes (ping-pong between two framebuffers)
        bool horizontal = true;
        bool firstIteration = true;
        int amount = m_blurPasses;

        m_blurShader->Use();
        for (int i = 0; i < amount; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO[horizontal]);
            m_blurShader->SetUniform("u_horizontal", horizontal);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, firstIteration ? m_brightPassTexture : m_blurTexture[!horizontal]);
            m_blurShader->SetUniform("u_inputTexture", 0);
            
            RenderFullscreenQuad();
            horizontal = !horizontal;
            if (firstIteration) firstIteration = false;
        }
        m_blurShader->Unuse();

        // Step 3: Combine original with bloom
        if (outputTexture != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, outputTexture);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT);

        m_combineShader->Use();
        m_combineShader->SetUniform("u_intensity", m_intensity);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);
        m_combineShader->SetUniform("u_originalTexture", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_blurTexture[!horizontal]);
        m_combineShader->SetUniform("u_bloomTexture", 1);
        
        RenderFullscreenQuad();
        m_combineShader->Unuse();

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const std::string& BloomEffect::GetName() const {
        return m_name;
    }

    void BloomEffect::SetParameter(const std::string& name, float value) {
        if (name == "threshold") {
            SetThreshold(value);
        } else if (name == "intensity") {
            SetIntensity(value);
        } else if (name == "radius") {
            SetRadius(value);
        } else if (name == "blurPasses") {
            SetBlurPasses(static_cast<int>(value));
        }
    }

    void BloomEffect::CreateShaders() {
        // Bright pass shader
        m_brightPassShader = std::make_shared<Shader>();
        std::string brightPassVert = R"(
            #version 460 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            out vec2 TexCoord;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";
        
        std::string brightPassFrag = R"(
            #version 460 core
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D u_inputTexture;
            uniform float u_threshold;
            void main() {
                vec3 color = texture(u_inputTexture, TexCoord).rgb;
                float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
                if (brightness > u_threshold) {
                    FragColor = vec4(color, 1.0);
                } else {
                    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                }
            }
        )";
        
        m_brightPassShader->LoadFromSource(brightPassVert, brightPassFrag);

        // Blur shader
        m_blurShader = std::make_shared<Shader>();
        std::string blurFrag = R"(
            #version 460 core
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D u_inputTexture;
            uniform bool u_horizontal;
            uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
            void main() {
                vec2 tex_offset = 1.0 / textureSize(u_inputTexture, 0);
                vec3 result = texture(u_inputTexture, TexCoord).rgb * weight[0];
                if (u_horizontal) {
                    for (int i = 1; i < 5; ++i) {
                        result += texture(u_inputTexture, TexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                        result += texture(u_inputTexture, TexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                    }
                } else {
                    for (int i = 1; i < 5; ++i) {
                        result += texture(u_inputTexture, TexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                        result += texture(u_inputTexture, TexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                    }
                }
                FragColor = vec4(result, 1.0);
            }
        )";
        
        m_blurShader->LoadFromSource(brightPassVert, blurFrag);

        // Combine shader
        m_combineShader = std::make_shared<Shader>();
        std::string combineFrag = R"(
            #version 460 core
            in vec2 TexCoord;
            out vec4 FragColor;
            uniform sampler2D u_originalTexture;
            uniform sampler2D u_bloomTexture;
            uniform float u_intensity;
            void main() {
                vec3 original = texture(u_originalTexture, TexCoord).rgb;
                vec3 bloom = texture(u_bloomTexture, TexCoord).rgb;
                FragColor = vec4(original + bloom * u_intensity, 1.0);
            }
        )";
        
        m_combineShader->LoadFromSource(brightPassVert, combineFrag);
    }

    void BloomEffect::CreateFramebuffers() {
        // Clean up existing framebuffers
        if (m_brightPassFBO != 0) {
            glDeleteFramebuffers(1, &m_brightPassFBO);
            glDeleteTextures(1, &m_brightPassTexture);
        }
        
        for (int i = 0; i < 2; ++i) {
            if (m_blurFBO[i] != 0) {
                glDeleteFramebuffers(1, &m_blurFBO[i]);
                glDeleteTextures(1, &m_blurTexture[i]);
            }
        }

        // Create bright pass framebuffer
        glGenFramebuffers(1, &m_brightPassFBO);
        glGenTextures(1, &m_brightPassTexture);
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_brightPassFBO);
        glBindTexture(GL_TEXTURE_2D, m_brightPassTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width / 2, m_height / 2, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brightPassTexture, 0);

        // Create blur framebuffers
        for (int i = 0; i < 2; ++i) {
            glGenFramebuffers(1, &m_blurFBO[i]);
            glGenTextures(1, &m_blurTexture[i]);
            
            glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO[i]);
            glBindTexture(GL_TEXTURE_2D, m_blurTexture[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width / 2, m_height / 2, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blurTexture[i], 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void BloomEffect::UpdateShaderUniforms() {
        // Uniforms are set directly in Process method
    }

    void BloomEffect::RenderFullscreenQuad() {
        // Simple fullscreen quad rendering
        glBegin(GL_TRIANGLES);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(2.0f, 0.0f); glVertex2f(3.0f, -1.0f);
        glTexCoord2f(0.0f, 2.0f); glVertex2f(-1.0f, 3.0f);
        glEnd();
    }

} // namespace GameEngine