#include "Graphics/OpenGLRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/ShaderStateManager.h"
#include "Graphics/Shader.h"
#include "Graphics/PostProcessingPipeline.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/PBRMaterial.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace GameEngine {
    OpenGLRenderer::OpenGLRenderer() {
    }

    OpenGLRenderer::~OpenGLRenderer() {
        Shutdown();
    }

    bool OpenGLRenderer::Initialize(const RenderSettings& settings) {
        m_settings = settings;

        // Configure GLFW for OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);

        // Create window
        m_window = glfwCreateWindow(
            settings.windowWidth, 
            settings.windowHeight, 
            "Game Engine Kiro", 
            settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, 
            nullptr
        );

        if (!m_window) {
            LOG_ERROR("Failed to create GLFW window");
            return false;
        }

        glfwMakeContextCurrent(m_window);
        
        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG_ERROR("Failed to initialize GLAD");
            return false;
        }
        
        if (settings.vsync) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }

        if (!InitializeOpenGL()) {
            LOG_ERROR("Failed to initialize OpenGL");
            return false;
        }

        // Initialize ShaderManager
        if (!ShaderManager::GetInstance().Initialize()) {
            LOG_ERROR("Failed to initialize ShaderManager");
            return false;
        }

        // Initialize ShaderStateManager
        if (!ShaderStateManager::GetInstance().Initialize()) {
            LOG_ERROR("Failed to initialize ShaderStateManager");
            return false;
        }

        // Initialize post-processing pipeline
        if (!InitializePostProcessing()) {
            LOG_ERROR("Failed to initialize post-processing pipeline");
            return false;
        }
        
        // Temporarily disable post-processing to debug rendering issue
        m_postProcessingEnabled = false;

        SetViewport(0, 0, settings.windowWidth, settings.windowHeight);
        SetupPBRLighting();
        
        LOG_INFO("OpenGL Renderer initialized successfully");
        return true;
    }

    bool OpenGLRenderer::InitializeOpenGL() {
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Enable face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable MSAA if requested
        if (m_settings.msaaSamples > 1) {
            glEnable(GL_MULTISAMPLE);
        }

        SetupDebugCallback();
        
        return true;
    }

    void OpenGLRenderer::SetupDebugCallback() {
        // OpenGL debug callback would be implemented here
        // This requires loading OpenGL extensions which we'll skip for now
    }

    void OpenGLRenderer::Shutdown() {
        // Shutdown post-processing pipeline
        if (m_postProcessingPipeline) {
            m_postProcessingPipeline->Shutdown();
            m_postProcessingPipeline.reset();
        }

        // Clean up main framebuffer
        if (m_mainFramebuffer != 0) {
            glDeleteFramebuffers(1, &m_mainFramebuffer);
            m_mainFramebuffer = 0;
        }
        if (m_mainColorTexture != 0) {
            glDeleteTextures(1, &m_mainColorTexture);
            m_mainColorTexture = 0;
        }
        if (m_mainDepthTexture != 0) {
            glDeleteTextures(1, &m_mainDepthTexture);
            m_mainDepthTexture = 0;
        }

        // Shutdown ShaderManager and ShaderStateManager
        ShaderManager::GetInstance().Shutdown();
        ShaderStateManager::GetInstance().Shutdown();
        
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    void OpenGLRenderer::BeginFrame() {
        // Begin frame for shader state management
        ShaderStateManager::GetInstance().BeginFrame();
        
        if (m_postProcessingEnabled && m_mainFramebuffer != 0) {
            // Render to main framebuffer for post-processing
            glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);
        } else {
            // Render directly to screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void OpenGLRenderer::EndFrame() {
        if (m_postProcessingEnabled && m_postProcessingPipeline && m_mainFramebuffer != 0) {
            // Apply post-processing effects
            ApplyPostProcessing();
        }
        
        // End frame for shader state management
        ShaderStateManager::GetInstance().EndFrame();
    }

    void OpenGLRenderer::Present() {
        glfwSwapBuffers(m_window);
    }

    void OpenGLRenderer::Update(float deltaTime) {
        // Update ShaderManager for hot-reload functionality
        ShaderManager::GetInstance().Update(deltaTime);
    }

    void OpenGLRenderer::SetViewport(int x, int y, int width, int height) {
        glViewport(x, y, width, height);
        
        // Update settings for framebuffer recreation
        if (width != m_settings.windowWidth || height != m_settings.windowHeight) {
            m_settings.windowWidth = width;
            m_settings.windowHeight = height;
            
            // Resize post-processing pipeline
            if (m_postProcessingPipeline) {
                m_postProcessingPipeline->Resize(width, height);
            }
            
            // Recreate main framebuffer with new size
            if (m_mainFramebuffer != 0) {
                SetupMainFramebuffer();
            }
        }
    }

    void OpenGLRenderer::Clear(const Math::Vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderer::DrawMesh(const Mesh* mesh, const Material* material, const Math::Mat4& transform) {
        if (!mesh || !material) {
            LOG_WARNING("Cannot draw mesh: null mesh or material");
            return;
        }

        // Get the material's shader
        auto shader = material->GetShader();
        if (!shader || !shader->IsValid()) {
            LOG_WARNING("Cannot draw mesh: invalid shader");
            return;
        }

        // Use the shader
        shader->Use();

        // Set standard matrices
        shader->SetUniform("u_model", transform);
        shader->SetUniform("u_view", m_viewMatrix);
        shader->SetUniform("u_projection", m_projectionMatrix);
        shader->SetUniform("u_mvp", m_projectionMatrix * m_viewMatrix * transform);

        // Calculate normal matrix for lighting
        Math::Mat3 normalMatrix = Math::Mat3(glm::transpose(glm::inverse(transform)));
        shader->SetUniform("u_normalMatrix", normalMatrix);

        // Apply material properties to shader
        material->ApplyToShader(shader);

        // Apply PBR lighting uniforms if this is a PBR material
        if (material->GetType() == Material::Type::PBR) {
            // Directional light
            shader->SetUniform("u_directionalLight.direction", m_directionalLight.direction);
            shader->SetUniform("u_directionalLight.color", m_directionalLight.color);
            shader->SetUniform("u_directionalLight.intensity", m_directionalLight.intensity);

            // Point lights
            shader->SetUniform("u_pointLightCount", static_cast<int>(m_pointLights.size()));
            for (size_t i = 0; i < m_pointLights.size() && i < MAX_POINT_LIGHTS; ++i) {
                std::string prefix = "u_pointLights[" + std::to_string(i) + "].";
                shader->SetUniform(prefix + "position", m_pointLights[i].position);
                shader->SetUniform(prefix + "color", m_pointLights[i].color);
                shader->SetUniform(prefix + "intensity", m_pointLights[i].intensity);
                shader->SetUniform(prefix + "radius", m_pointLights[i].radius);
            }

            // Camera position for specular calculations
            if (m_viewMatrix != Math::Mat4(1.0f)) {
                Math::Vec3 cameraPos = Math::Vec3(glm::inverse(m_viewMatrix)[3]);
                shader->SetUniform("u_cameraPosition", cameraPos);
            }
        }

        // Bind mesh and draw
        mesh->Bind();
        mesh->Draw();
        mesh->Unbind();

        shader->Unuse();
    }

    void OpenGLRenderer::SetCamera(const Camera* camera) {
        if (camera) {
            m_viewMatrix = camera->GetViewMatrix();
            m_projectionMatrix = camera->GetProjectionMatrix();
        }
    }

    std::shared_ptr<Shader> OpenGLRenderer::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {
        // Generate a unique name for the shader based on source hash
        std::string shaderName = "runtime_shader_" + std::to_string(std::hash<std::string>{}(vertexSource + fragmentSource));
        
        // Use ShaderManager to create and manage the shader
        return ShaderManager::GetInstance().LoadShaderFromSource(shaderName, vertexSource, fragmentSource);
    }

    std::shared_ptr<Shader> OpenGLRenderer::LoadShaderFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        return ShaderManager::GetInstance().LoadShaderFromFiles(name, vertexPath, fragmentPath);
    }

    std::shared_ptr<Shader> OpenGLRenderer::GetShader(const std::string& name) {
        return ShaderManager::GetInstance().GetShader(name);
    }

    std::shared_ptr<Texture> OpenGLRenderer::CreateTexture(const std::string& filepath) {
        // Texture creation implementation would go here
        return nullptr;
    }

    std::shared_ptr<Mesh> OpenGLRenderer::CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        // Mesh creation implementation would go here
        return nullptr;
    }

    bool OpenGLRenderer::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, bool enableHotReload) {
        ShaderDesc desc;
        desc.name = name;
        desc.vertexPath = vertexPath;
        desc.fragmentPath = fragmentPath;
        desc.enableHotReload = enableHotReload;
        desc.enableOptimization = true;

        auto shader = ShaderManager::GetInstance().LoadShader(name, desc);
        return shader != nullptr;
    }

    bool OpenGLRenderer::UnloadShader(const std::string& name) {
        ShaderManager::GetInstance().UnloadShader(name);
        return true; // ShaderManager doesn't return success/failure for unload
    }

    void OpenGLRenderer::ReloadShader(const std::string& name) {
        ShaderManager::GetInstance().ReloadShader(name);
    }

    void OpenGLRenderer::EnableShaderHotReload(bool enable) {
        ShaderManager::GetInstance().EnableHotReload(enable);
    }

    std::vector<std::string> OpenGLRenderer::GetLoadedShaderNames() const {
        return ShaderManager::GetInstance().GetShaderNames();
    }

    bool OpenGLRenderer::InitializePostProcessing() {
        m_postProcessingPipeline = std::make_unique<PostProcessingPipeline>();
        
        if (!m_postProcessingPipeline->Initialize(m_settings.windowWidth, m_settings.windowHeight)) {
            LOG_ERROR("Failed to initialize post-processing pipeline");
            return false;
        }

        // Enable default post-processing effects
        m_postProcessingPipeline->EnableToneMapping(true, ToneMappingType::ACES);
        m_postProcessingPipeline->EnableFXAA(true, 0.75f);

        // Setup main framebuffer for rendering
        SetupMainFramebuffer();

        LOG_INFO("Post-processing pipeline initialized successfully");
        return true;
    }

    void OpenGLRenderer::SetupMainFramebuffer() {
        // Clean up existing framebuffer
        if (m_mainFramebuffer != 0) {
            glDeleteFramebuffers(1, &m_mainFramebuffer);
            glDeleteTextures(1, &m_mainColorTexture);
            glDeleteTextures(1, &m_mainDepthTexture);
        }

        // Create framebuffer
        glGenFramebuffers(1, &m_mainFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_mainFramebuffer);

        // Create color texture
        glGenTextures(1, &m_mainColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_mainColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_settings.windowWidth, m_settings.windowHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mainColorTexture, 0);

        // Create depth texture
        glGenTextures(1, &m_mainDepthTexture);
        glBindTexture(GL_TEXTURE_2D, m_mainDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_settings.windowWidth, m_settings.windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_mainDepthTexture, 0);

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Main framebuffer is not complete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLRenderer::ApplyPostProcessing() {
        // Bind default framebuffer for final output
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Process the main color texture through the post-processing pipeline
        m_postProcessingPipeline->ProcessToScreen(m_mainColorTexture);
    }

    void OpenGLRenderer::EnablePostProcessing(bool enable) {
        m_postProcessingEnabled = enable;
        
        if (enable && !m_postProcessingPipeline) {
            InitializePostProcessing();
        }
    }

    void OpenGLRenderer::SetupPBRLighting() {
        // Set default directional light (sun)
        m_directionalLight.direction = glm::normalize(Math::Vec3(0.3f, -1.0f, 0.3f));
        m_directionalLight.color = Math::Vec3(1.0f, 0.95f, 0.8f);
        m_directionalLight.intensity = 3.0f;

        LOG_INFO("PBR lighting setup completed");
    }

    void OpenGLRenderer::SetDirectionalLight(const Math::Vec3& direction, const Math::Vec3& color, float intensity) {
        m_directionalLight.direction = glm::normalize(direction);
        m_directionalLight.color = color;
        m_directionalLight.intensity = intensity;
    }

    void OpenGLRenderer::AddPointLight(const Math::Vec3& position, const Math::Vec3& color, float intensity, float radius) {
        if (m_pointLights.size() >= MAX_POINT_LIGHTS) {
            LOG_WARNING("Maximum number of point lights reached (" + std::to_string(MAX_POINT_LIGHTS) + ")");
            return;
        }

        PointLight light;
        light.position = position;
        light.color = color;
        light.intensity = intensity;
        light.radius = radius;
        
        m_pointLights.push_back(light);
    }

    Math::Vec3 OpenGLRenderer::GetCameraPosition() const {
        if (m_viewMatrix != Math::Mat4(1.0f)) {
            return Math::Vec3(glm::inverse(m_viewMatrix)[3]);
        }
        return Math::Vec3(0.0f, 5.0f, 10.0f); // Default camera position
    }

    void OpenGLRenderer::SyncWithPrimitiveRenderer(PrimitiveRenderer* primitiveRenderer) {
        if (!primitiveRenderer) return;
        
        // Sync matrices
        primitiveRenderer->SetViewMatrix(m_viewMatrix);
        primitiveRenderer->SetProjectionMatrix(m_projectionMatrix);
        primitiveRenderer->SetViewProjectionMatrix(m_projectionMatrix * m_viewMatrix);
        
        // Sync camera position
        primitiveRenderer->SetCameraPosition(GetCameraPosition());
        
        // Sync lighting
        primitiveRenderer->SetDirectionalLight(
            m_directionalLight.direction,
            m_directionalLight.color,
            m_directionalLight.intensity
        );
        
        // Clear and sync point lights
        primitiveRenderer->ClearPointLights();
        for (const auto& light : m_pointLights) {
            primitiveRenderer->AddPointLight(
                light.position,
                light.color,
                light.intensity,
                light.radius
            );
        }
    }
}