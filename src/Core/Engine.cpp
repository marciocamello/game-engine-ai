#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Graphics/GraphicsRenderer.h"
#include "Resource/ResourceManager.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsDebugManager.h"
#include "Audio/AudioEngine.h"
#include "Input/InputManager.h"
#include "Scripting/ScriptingEngine.h"
#include "Graphics/Camera.h"

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

        // Initialize physics debug manager
        m_physicsDebugManager = std::make_unique<Physics::PhysicsDebugManager>();
        if (!m_physicsDebugManager->Initialize(m_physics.get(), m_input.get())) {
            LOG_ERROR("Failed to initialize physics debug manager");
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
        
        // Update audio listener with main camera position, orientation, and velocity
        if (m_mainCamera && m_audio) {
            // Note: We need to cast away const to update velocity, but this is safe in the update loop
            Camera* mutableCamera = const_cast<Camera*>(m_mainCamera);
            mutableCamera->UpdateVelocity(deltaTime);
            
            m_audio->SetListenerPosition(m_mainCamera->GetPosition());
            m_audio->SetListenerOrientation(m_mainCamera->GetForward(), m_mainCamera->GetUp());
            m_audio->SetListenerVelocity(m_mainCamera->GetVelocity());
        }
        
        // Handle physics debug input
        if (m_physicsDebugManager) {
            m_physicsDebugManager->HandleInput();
        }
        
        // Call custom update callback if set
        if (m_updateCallback) {
            m_updateCallback(deltaTime);
        }
    }

    void Engine::Render() {
        m_renderer->BeginFrame();
        m_renderer->Clear(Math::Vec4(0.2f, 0.3f, 0.8f, 1.0f)); // Sky blue background
        
        // Begin physics debug frame
        if (m_physicsDebugManager) {
            m_physicsDebugManager->BeginFrame();
        }
        
        // Call custom render callback if set
        if (m_renderCallback) {
            m_renderCallback();
        }
        
        // Render physics debug visualization
        if (m_physicsDebugManager) {
            m_physicsDebugManager->Render();
            m_physicsDebugManager->EndFrame();
        }
        
        m_renderer->EndFrame();
        m_renderer->Present();
    }

    void Engine::Shutdown() {
        if (m_isRunning) {
            LOG_INFO("Game Engine Kiro - Shutting down...");
            
            m_physicsDebugManager.reset();
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
    
    void Engine::SetMainCamera(const Camera* camera) {
        m_mainCamera = camera;
        
        if (m_physicsDebugManager) {
            m_physicsDebugManager->SetCamera(camera);
        }
        
        // Immediately update audio listener if camera is set
        if (m_mainCamera && m_audio) {
            m_audio->SetListenerPosition(m_mainCamera->GetPosition());
            m_audio->SetListenerOrientation(m_mainCamera->GetForward(), m_mainCamera->GetUp());
        }
    }
}