#pragma once

#include "Physics/PhysicsDebugRenderer.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Input/InputManager.h"
#include <memory>

namespace GameEngine {
    class PhysicsEngine;
    class Camera;
    
    namespace Physics {
        
        /**
         * @brief Manager class for physics debug visualization
         * 
         * This class handles the integration between the physics engine,
         * debug renderer, and input system to provide seamless debug
         * visualization with keyboard toggle support.
         */
        class PhysicsDebugManager {
        public:
            PhysicsDebugManager();
            ~PhysicsDebugManager();
            
            // Initialization and cleanup
            bool Initialize(PhysicsEngine* physicsEngine, InputManager* inputManager);
            void Shutdown();
            
            // Configuration
            void SetConfig(const PhysicsDebugConfig& config);
            const PhysicsDebugConfig& GetConfig() const;
            
            // Camera setup
            void SetCamera(const Camera* camera);
            
            // Debug mode control
            void SetDebugMode(PhysicsDebugMode mode);
            PhysicsDebugMode GetDebugMode() const;
            void ToggleDebugMode();
            
            // Enable/disable debug rendering
            void SetEnabled(bool enabled);
            bool IsEnabled() const;
            
            // Frame rendering
            void BeginFrame();
            void EndFrame();
            void Render();
            
            // Input handling
            void HandleInput();
            
            // Statistics
            const PhysicsDebugRenderer::RenderStats& GetRenderStats() const;
            void PrintDebugInfo() const;
            
        private:
            void UpdateInputBindings();
            void OnDebugTogglePressed();
            void OnDebugModeChanged();
            
            // Core components
            PhysicsEngine* m_physicsEngine = nullptr;
            InputManager* m_inputManager = nullptr;
            std::shared_ptr<PhysicsDebugRenderer> m_debugRenderer;
            
            // State
            bool m_initialized = false;
            bool m_enabled = false;
            PhysicsDebugMode m_currentMode = PhysicsDebugMode::None;
            
            // Input handling
            bool m_debugKeyPressed = false;
            bool m_lastDebugKeyState = false;
        };
        
    } // namespace Physics
} // namespace GameEngine