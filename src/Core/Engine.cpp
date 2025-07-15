#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Graphics/GraphicsRenderer.h"
#include "Resource/ResourceManager.h"
#include "Physics/PhysicsEngine.h"
#include "Audio/AudioEngine.h"
#include "Input/InputManager.h"
#include "Scripting/ScriptingEngine.h"

#include <GLFW/glfw3.h>

namespace GameEngine {
    Engine::Engine() 
        : m_isRunning(false), m_deltaTime(0.0f) {
    }

    Engine::~Engine() {
        Shutdown();
    }

    bool Engine::Initialize() {
        // Initialize GLFW
        if (!glfwInit()) {
            LOG_CRITICAL("Failed to initialize GLFW");
            return false;
        }

        // Initialize logger
        Logger::GetInstance().Initialize();
        LOG_INFO("Game Engine Kiro - Initializing...");

        // Initialize subsystems
        RenderSettings renderSettings;
        renderSettings.fullscreen = true;  // Enable fullscreen for better camera control
        renderSettings.windowWidth = 1920;
        renderSettings.windowHeight = 1080;
        renderSettings.vsync = true;
        
        m_renderer = GraphicsRenderer::Create(GraphicsAPI::OpenGL);
        if (!m_renderer->Initialize(renderSettings)) {
            LOG_ERROR("Failed to initialize graphics renderer");
            return false;
        }

        m_resourceManager = std::make_unique<ResourceManager>();
        if (!m_resourceManager->Initialize()) {
            LOG_ERROR("Failed to initialize resource manager");
            return false;
        }

        m_physics = std::make_unique<PhysicsEngine>();
        if (!m_physics->Initialize()) {
            LOG_ERROR("Failed to initialize physics engine");
            return false;
        }

        m_audio = std::make_unique<AudioEngine>();
        if (!m_audio->Initialize()) {
            LOG_ERROR("Failed to initialize audio engine");
            return false;
        }

        m_input = std::make_unique<InputManager>();
        if (!m_input->Initialize(m_renderer->GetWindow())) {
            LOG_ERROR("Failed to initialize input manager");
            return false;
        }

        m_scripting = std::make_unique<ScriptingEngine>();
        if (!m_scripting->Initialize()) {
            LOG_ERROR("Failed to initialize scripting engine");
            return false;
        }

        m_lastFrameTime = std::chrono::high_resolution_clock::now();
        m_isRunning = true;

        LOG_INFO("Game Engine Kiro - Initialization complete");
        return true;
    }

    void Engine::Run() {
        while (m_isRunning && !glfwWindowShouldClose(m_renderer->GetWindow())) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            m_deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
            m_lastFrameTime = currentTime;

            glfwPollEvents();
            
            Update(m_deltaTime);
            Render();
        }
    }

    void Engine::Update(float deltaTime) {
        m_input->Update();
        m_physics->Update(deltaTime);
        m_audio->Update(deltaTime);
        m_scripting->Update(deltaTime);
        
        // Call custom update callback if set
        if (m_updateCallback) {
            m_updateCallback(deltaTime);
        }
    }

    void Engine::Render() {
        m_renderer->BeginFrame();
        m_renderer->Clear(Math::Vec4(0.2f, 0.3f, 0.8f, 1.0f)); // Sky blue background
        
        // Call custom render callback if set
        if (m_renderCallback) {
            m_renderCallback();
        }
        
        m_renderer->EndFrame();
        m_renderer->Present();
    }

    void Engine::Shutdown() {
        if (m_isRunning) {
            LOG_INFO("Game Engine Kiro - Shutting down...");
            
            m_scripting.reset();
            m_input.reset();
            m_audio.reset();
            m_physics.reset();
            m_resourceManager.reset();
            m_renderer.reset();
            
            glfwTerminate();
            m_isRunning = false;
            
            LOG_INFO("Game Engine Kiro - Shutdown complete");
        }
    }
}