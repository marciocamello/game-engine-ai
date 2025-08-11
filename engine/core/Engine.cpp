#include "Engine.h"
#include "Logger.h"
#include "../../include/Core/ModuleRegistry.h"
#include "../../include/Core/ModuleConfigLoader.h"
#include "../../include/Graphics/GraphicsRenderer.h"
#include "../../include/Resource/ResourceManager.h"
#include "../../include/Physics/PhysicsEngine.h"
#include "../../include/Physics/PhysicsDebugManager.h"
#include "../../include/Audio/AudioEngine.h"
#include "../../include/Input/InputManager.h"
#include "../../include/Scripting/ScriptingEngine.h"
#include "../../include/Graphics/Camera.h"
#include "../interfaces/IGraphicsModule.h"
#include "../interfaces/IPhysicsModule.h"
#include "../interfaces/IAudioModule.h"
#include "../modules/OpenGLGraphicsModule.h"
#include "../modules/BulletPhysicsModule.h"
#include "../modules/audio-openal/OpenALAudioModule.h"

#include <GLFW/glfw3.h>
#include <filesystem>

namespace GameEngine {
    Engine::Engine() 
        : m_moduleRegistry(nullptr), m_engineConfig(nullptr), m_useModuleSystem(true),
          m_isRunning(false), m_deltaTime(0.0f) {
    }

    Engine::~Engine() {
        Shutdown();
    }

    // Module system initialization methods (defined before Initialize)
    bool Engine::InitializeModuleSystem() {
        try {
            m_moduleRegistry = &ModuleRegistry::GetInstance();
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize module registry: " + std::string(e.what()));
            return false;
        }
    }

    bool Engine::LoadConfiguration(const std::string& configPath) {
        std::string actualConfigPath = configPath;
        
        // If no config path provided, try default locations
        if (actualConfigPath.empty()) {
            if (std::filesystem::exists("engine_config.json")) {
                actualConfigPath = "engine_config.json";
            } else if (std::filesystem::exists("config/engine.json")) {
                actualConfigPath = "config/engine.json";
            } else {
                LOG_INFO("No configuration file found, using default configuration");
                m_engineConfig = new EngineConfig(ModuleConfigLoader::CreateDefaultConfig());
                return true;
            }
        }

        auto configOpt = ModuleConfigLoader::LoadFromFile(actualConfigPath);
        if (configOpt) {
            m_engineConfig = new EngineConfig(*configOpt);
            LOG_INFO("Loaded engine configuration from: " + actualConfigPath);
            return true;
        } else {
            LOG_WARNING("Failed to load configuration from: " + actualConfigPath + ", using default");
            m_engineConfig = new EngineConfig(ModuleConfigLoader::CreateDefaultConfig());
            return true;
        }
    }

    bool Engine::RegisterDefaultModules() {
        try {
            // Register graphics module
            auto graphicsModule = std::make_unique<Graphics::OpenGLGraphicsModule>();
            m_moduleRegistry->RegisterModule(std::move(graphicsModule));

            // Register physics module
            auto physicsModule = std::make_unique<Physics::BulletPhysicsModule>();
            m_moduleRegistry->RegisterModule(std::move(physicsModule));

            // Register audio module
            auto audioModule = std::make_unique<Audio::OpenALAudioModule>();
            m_moduleRegistry->RegisterModule(std::move(audioModule));

            LOG_INFO("Default modules registered successfully");
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to register default modules: " + std::string(e.what()));
            return false;
        }
    }

    bool Engine::InitializeRemainingSubsystems() {
        // Initialize resource manager (not yet modularized)
        m_resourceManager = std::make_unique<ResourceManager>();
        if (!m_resourceManager->Initialize()) {
            LOG_ERROR("Failed to initialize resource manager");
            return false;
        }

        // Initialize input manager (not yet modularized)
        GraphicsRenderer* renderer = GetRenderer();
        if (renderer) {
            m_input = std::make_unique<InputManager>();
            if (!m_input->Initialize(renderer->GetWindow())) {
                LOG_ERROR("Failed to initialize input manager");
                return false;
            }
        }

        // Initialize scripting engine (not yet modularized)
        m_scripting = std::make_unique<ScriptingEngine>();
        if (!m_scripting->Initialize()) {
            LOG_ERROR("Failed to initialize scripting engine");
            return false;
        }

        // Initialize physics debug manager
        PhysicsEngine* physics = GetPhysics();
        if (physics && m_input) {
            m_physicsDebugManager = std::make_unique<Physics::PhysicsDebugManager>();
            if (!m_physicsDebugManager->Initialize(physics, m_input.get())) {
                LOG_ERROR("Failed to initialize physics debug manager");
                return false;
            }
        }

        return true;
    }

    bool Engine::Initialize(const std::string& configPath) {
        // Initialize GLFW
        if (!glfwInit()) {
            LOG_CRITICAL("Failed to initialize GLFW");
            return false;
        }

        // Initialize logger
        Logger::GetInstance().Initialize();
        LOG_INFO("Game Engine Kiro - Initializing with module system...");

        // Try to initialize with module system first
        if (InitializeModuleSystem()) {
            if (LoadConfiguration(configPath)) {
                if (RegisterDefaultModules()) {
                    if (m_moduleRegistry->InitializeModules(*m_engineConfig)) {
                        LOG_INFO("Module system initialized successfully");
                        
                        // Initialize remaining non-modular subsystems
                        if (InitializeRemainingSubsystems()) {
                            m_lastFrameTime = std::chrono::high_resolution_clock::now();
                            m_isRunning = true;
                            LOG_INFO("Game Engine Kiro - Initialization complete (module system)");
                            return true;
                        }
                    }
                }
            }
        }

        LOG_WARNING("Module system initialization failed, falling back to legacy mode");
        m_useModuleSystem = false;
        
        // Fallback to legacy initialization
        if (InitializeLegacySubsystems()) {
            m_lastFrameTime = std::chrono::high_resolution_clock::now();
            m_isRunning = true;
            LOG_INFO("Game Engine Kiro - Initialization complete (legacy mode)");
            return true;
        }

        LOG_ERROR("Both module system and legacy initialization failed");
        return false;
    }

    void Engine::Run() {
        GLFWwindow* window = nullptr;
        
        if (m_useModuleSystem) {
            Graphics::IGraphicsModule* graphicsModule = GetGraphicsModule();
            if (graphicsModule) {
                window = static_cast<GLFWwindow*>(graphicsModule->GetWindow());
            }
        }
        
        if (!window) {
            LOG_ERROR("No valid window found for main loop");
            return;
        }
        
        while (m_isRunning && !glfwWindowShouldClose(window)) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            m_deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
            m_lastFrameTime = currentTime;

            glfwPollEvents();
            
            Update(m_deltaTime);
            Render();
        }
    }

    void Engine::Update(float deltaTime) {
        if (m_useModuleSystem) {
            // Update all modules
            m_moduleRegistry->UpdateModules(deltaTime);
            
            // Update non-modular subsystems
            if (m_input) {
                m_input->Update();
            }
            if (m_scripting) {
                m_scripting->Update(deltaTime);
            }
            
            // Update audio listener with main camera position, orientation, and velocity
            if (m_mainCamera) {
                Audio::IAudioModule* audioModule = GetAudioModule();
                if (audioModule) {
                    // Note: We need to cast away const to update velocity, but this is safe in the update loop
                    Camera* mutableCamera = const_cast<Camera*>(m_mainCamera);
                    mutableCamera->UpdateVelocity(deltaTime);
                    
                    audioModule->SetListenerPosition(m_mainCamera->GetPosition());
                    audioModule->SetListenerOrientation(m_mainCamera->GetForward(), m_mainCamera->GetUp());
                    audioModule->SetListenerVelocity(m_mainCamera->GetVelocity());
                }
            }
        } else {
            // Legacy update path
            if (m_input) m_input->Update();
            
            PhysicsEngine* physics = GetPhysics();
            if (physics) physics->Update(deltaTime);
            
            AudioEngine* audio = GetAudio();
            if (audio) audio->Update(deltaTime);
            
            if (m_scripting) m_scripting->Update(deltaTime);
            
            // Update audio listener with main camera position, orientation, and velocity
            if (m_mainCamera && audio) {
                // Note: We need to cast away const to update velocity, but this is safe in the update loop
                Camera* mutableCamera = const_cast<Camera*>(m_mainCamera);
                mutableCamera->UpdateVelocity(deltaTime);
                
                audio->SetListenerPosition(m_mainCamera->GetPosition());
                audio->SetListenerOrientation(m_mainCamera->GetForward(), m_mainCamera->GetUp());
                audio->SetListenerVelocity(m_mainCamera->GetVelocity());
            }
        }
        
        // Handle physics debug input (common for both paths)
        if (m_physicsDebugManager) {
            m_physicsDebugManager->HandleInput();
        }
        
        // Call custom update callback if set
        if (m_updateCallback) {
            m_updateCallback(deltaTime);
        }
    }

    void Engine::Render() {
        GraphicsRenderer* renderer = GetRenderer();
        if (!renderer) {
            return;
        }
        
        renderer->BeginFrame();
        renderer->Clear(Math::Vec4(0.1f, 0.1f, 0.1f, 1.0f)); // Dark gray background
        
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
        
        renderer->EndFrame();
        renderer->Present();
    }

    void Engine::Shutdown() {
        if (m_isRunning) {
            LOG_INFO("Game Engine Kiro - Shutting down...");
            
            m_physicsDebugManager.reset();
            
            if (m_useModuleSystem) {
                ShutdownModuleSystem();
            } else {
                ShutdownLegacySubsystems();
            }
            
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
        if (m_mainCamera) {
            if (m_useModuleSystem) {
                Audio::IAudioModule* audioModule = GetAudioModule();
                if (audioModule) {
                    audioModule->SetListenerPosition(m_mainCamera->GetPosition());
                    audioModule->SetListenerOrientation(m_mainCamera->GetForward(), m_mainCamera->GetUp());
                }
            } else {
                AudioEngine* audio = GetAudio();
                if (audio) {
                    audio->SetListenerPosition(m_mainCamera->GetPosition());
                    audio->SetListenerOrientation(m_mainCamera->GetForward(), m_mainCamera->GetUp());
                }
            }
        }
    }

    // Legacy compatibility getters
    GraphicsRenderer* Engine::GetRenderer() const {
        if (m_useModuleSystem) {
            Graphics::IGraphicsModule* graphicsModule = GetGraphicsModule();
            return graphicsModule ? graphicsModule->GetRenderer() : nullptr;
        }
        return nullptr; // Legacy renderer is handled by modules now
    }

    ResourceManager* Engine::GetResourceManager() const {
        return m_resourceManager.get();
    }

    PhysicsEngine* Engine::GetPhysics() const {
        if (m_useModuleSystem) {
            Physics::IPhysicsModule* physicsModule = GetPhysicsModule();
            return physicsModule ? physicsModule->GetPhysicsEngine() : nullptr;
        }
        return nullptr; // Legacy physics is handled by modules now
    }

    AudioEngine* Engine::GetAudio() const {
        if (m_useModuleSystem) {
            Audio::IAudioModule* audioModule = GetAudioModule();
            return audioModule ? audioModule->GetAudioEngine() : nullptr;
        }
        return nullptr; // Legacy audio is handled by modules now
    }

    InputManager* Engine::GetInput() const {
        return m_input.get();
    }

    ScriptingEngine* Engine::GetScripting() const {
        return m_scripting.get();
    }

    // Module system access
    ModuleRegistry* Engine::GetModuleRegistry() const {
        return m_moduleRegistry;
    }

    Graphics::IGraphicsModule* Engine::GetGraphicsModule() const {
        if (!m_moduleRegistry) return nullptr;
        
        auto modules = m_moduleRegistry->GetModulesByType(ModuleType::Graphics);
        if (!modules.empty()) {
            return static_cast<Graphics::IGraphicsModule*>(modules[0]);
        }
        return nullptr;
    }

    Physics::IPhysicsModule* Engine::GetPhysicsModule() const {
        if (!m_moduleRegistry) return nullptr;
        
        auto modules = m_moduleRegistry->GetModulesByType(ModuleType::Physics);
        if (!modules.empty()) {
            return static_cast<Physics::IPhysicsModule*>(modules[0]);
        }
        return nullptr;
    }

    Audio::IAudioModule* Engine::GetAudioModule() const {
        if (!m_moduleRegistry) return nullptr;
        
        auto modules = m_moduleRegistry->GetModulesByType(ModuleType::Audio);
        if (!modules.empty()) {
            return static_cast<Audio::IAudioModule*>(modules[0]);
        }
        return nullptr;
    }



    void Engine::ShutdownModuleSystem() {
        if (m_moduleRegistry) {
            m_moduleRegistry->ShutdownModules();
        }
        
        // Shutdown non-modular subsystems
        m_scripting.reset();
        m_input.reset();
        m_resourceManager.reset();
        
        if (m_engineConfig) {
            delete m_engineConfig;
            m_engineConfig = nullptr;
        }
        
        m_moduleRegistry = nullptr;
    }

    // Legacy subsystem initialization (fallback)
    bool Engine::InitializeLegacySubsystems() {
        LOG_INFO("Initializing legacy subsystems...");

        // Initialize subsystems
        RenderSettings renderSettings;
        renderSettings.fullscreen = true;  // Enable fullscreen for better camera control
        renderSettings.windowWidth = 1920;
        renderSettings.windowHeight = 1080;
        renderSettings.vsync = true;
        
        // Note: Legacy renderer creation would go here, but we're transitioning to modules
        // For now, this is a placeholder for true legacy fallback
        LOG_ERROR("Legacy subsystem initialization not fully implemented - module system required");
        return false;
    }

    void Engine::ShutdownLegacySubsystems() {
        // Shutdown legacy subsystems
        m_scripting.reset();
        m_input.reset();
        m_resourceManager.reset();
    }
}