#pragma once

#include <memory>
#include <chrono>
#include <functional>

namespace GameEngine {
    class GraphicsRenderer;
    class ResourceManager;
    class PhysicsEngine;
    class AudioEngine;
    class InputManager;
    class ScriptingEngine;
    class Camera;
    
    namespace Physics {
        class PhysicsDebugManager;
    }

    class Engine {
    public:
        Engine();
        ~Engine();

        bool Initialize();
        void Run();
        void Shutdown();

        // Getters for engine subsystems
        GraphicsRenderer* GetRenderer() const { return m_renderer.get(); }
        ResourceManager* GetResourceManager() const { return m_resourceManager.get(); }
        PhysicsEngine* GetPhysics() const { return m_physics.get(); }
        AudioEngine* GetAudio() const { return m_audio.get(); }
        InputManager* GetInput() const { return m_input.get(); }
        ScriptingEngine* GetScripting() const { return m_scripting.get(); }
        Physics::PhysicsDebugManager* GetPhysicsDebugManager() const { return m_physicsDebugManager.get(); }

        float GetDeltaTime() const { return m_deltaTime; }
        bool IsRunning() const { return m_isRunning; }

        // Callback system for custom game logic
        void SetUpdateCallback(std::function<void(float)> callback) { m_updateCallback = callback; }
        void SetRenderCallback(std::function<void()> callback) { m_renderCallback = callback; }
        
        // Camera management for debug rendering
        void SetMainCamera(const Camera* camera);

    private:
        void Update(float deltaTime);
        void Render();

        std::unique_ptr<GraphicsRenderer> m_renderer;
        std::unique_ptr<ResourceManager> m_resourceManager;
        std::unique_ptr<PhysicsEngine> m_physics;
        std::unique_ptr<AudioEngine> m_audio;
        std::unique_ptr<InputManager> m_input;
        std::unique_ptr<ScriptingEngine> m_scripting;
        std::unique_ptr<Physics::PhysicsDebugManager> m_physicsDebugManager;

        bool m_isRunning;
        float m_deltaTime;
        std::chrono::high_resolution_clock::time_point m_lastFrameTime;

        // Callbacks for custom game logic
        std::function<void(float)> m_updateCallback;
        std::function<void()> m_renderCallback;
    };
}