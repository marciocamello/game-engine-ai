#pragma once

#include <memory>
#include <chrono>
#include <functional>
#include <string>

namespace GameEngine {
    class GraphicsRenderer;
    class ResourceManager;
    class PhysicsEngine;
    class AudioEngine;
    class InputManager;
    class ScriptingEngine;
    class Camera;
    class ModuleRegistry;
    struct EngineConfig;
    
    namespace Graphics {
        class IGraphicsModule;
    }
    
    namespace Physics {
        class IPhysicsModule;
        class PhysicsDebugManager;
    }
    
    namespace Audio {
        class IAudioModule;
    }

    class Engine {
    public:
        Engine();
        ~Engine();

        bool Initialize(const std::string& configPath = "");
        void Run();
        void Shutdown();

        // Getters for engine subsystems (legacy compatibility)
        GraphicsRenderer* GetRenderer() const;
        ResourceManager* GetResourceManager() const;
        PhysicsEngine* GetPhysics() const;
        AudioEngine* GetAudio() const;
        InputManager* GetInput() const;
        ScriptingEngine* GetScripting() const;
        Physics::PhysicsDebugManager* GetPhysicsDebugManager() const { return m_physicsDebugManager.get(); }

        // Module system access
        ModuleRegistry* GetModuleRegistry() const;
        Graphics::IGraphicsModule* GetGraphicsModule() const;
        Physics::IPhysicsModule* GetPhysicsModule() const;
        Audio::IAudioModule* GetAudioModule() const;

        float GetDeltaTime() const { return m_deltaTime; }
        bool IsRunning() const { return m_isRunning; }

        // Callback system for custom game logic
        void SetUpdateCallback(std::function<void(float)> callback) { m_updateCallback = callback; }
        void SetRenderCallback(std::function<void()> callback) { m_renderCallback = callback; }
        
        // Camera management for debug rendering and audio
        void SetMainCamera(const Camera* camera);

    private:
        void Update(float deltaTime);
        void Render();
        
        // Module system initialization
        bool InitializeModuleSystem();
        bool LoadConfiguration(const std::string& configPath);
        bool RegisterDefaultModules();
        bool InitializeRemainingSubsystems();
        void ShutdownModuleSystem();
        
        // Legacy subsystem initialization (fallback)
        bool InitializeLegacySubsystems();
        void ShutdownLegacySubsystems();

        // Module system
        ModuleRegistry* m_moduleRegistry;
        EngineConfig* m_engineConfig;
        bool m_useModuleSystem;

        // Legacy subsystem pointers (for compatibility)
        std::unique_ptr<ResourceManager> m_resourceManager;
        std::unique_ptr<InputManager> m_input;
        std::unique_ptr<ScriptingEngine> m_scripting;
        std::unique_ptr<Physics::PhysicsDebugManager> m_physicsDebugManager;

        bool m_isRunning;
        float m_deltaTime;
        std::chrono::high_resolution_clock::time_point m_lastFrameTime;

        // Callbacks for custom game logic
        std::function<void(float)> m_updateCallback;
        std::function<void()> m_renderCallback;
        
        // Main camera for audio listener integration
        const Camera* m_mainCamera = nullptr;
    };
}