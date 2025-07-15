#include "Graphics/OpenGLRenderer.h"
#include "Graphics/Camera.h"
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

        SetViewport(0, 0, settings.windowWidth, settings.windowHeight);
        
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
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    void OpenGLRenderer::BeginFrame() {
        // Frame setup
    }

    void OpenGLRenderer::EndFrame() {
        // Frame cleanup
    }

    void OpenGLRenderer::Present() {
        glfwSwapBuffers(m_window);
    }

    void OpenGLRenderer::SetViewport(int x, int y, int width, int height) {
        glViewport(x, y, width, height);
    }

    void OpenGLRenderer::Clear(const Math::Vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRenderer::DrawMesh(const Mesh* mesh, const Material* material, const Math::Mat4& transform) {
        // Mesh rendering implementation would go here
        // For now, this is a placeholder
    }

    void OpenGLRenderer::SetCamera(const Camera* camera) {
        if (camera) {
            m_viewMatrix = camera->GetViewMatrix();
            m_projectionMatrix = camera->GetProjectionMatrix();
        }
    }

    std::shared_ptr<Shader> OpenGLRenderer::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {
        // Shader creation implementation would go here
        return nullptr;
    }

    std::shared_ptr<Texture> OpenGLRenderer::CreateTexture(const std::string& filepath) {
        // Texture creation implementation would go here
        return nullptr;
    }

    std::shared_ptr<Mesh> OpenGLRenderer::CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        // Mesh creation implementation would go here
        return nullptr;
    }
}