/**
 * Character Movement Comparison Example
 * 
 * This example demonstrates the difference between:
 * 1. Character (physics-based with forces and rigid body)
 * 2. CharacterController (hybrid approach with collision detection only)
 * 
 * Use keys 1 and 2 to switch between the two approaches
 */

#include "Core/Engine.h"
#include "Game/Character.h"
#include "Game/CharacterController.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

using namespace GameEngine;

class CharacterComparisonApp {
public:
    bool Initialize() {
        // Initialize engine
        if (!m_engine.Initialize()) {
            LOG_ERROR("Failed to initialize engine");
            return false;
        }

        // Get engine systems
        m_renderer = m_engine.GetRenderer();
        m_input = m_engine.GetInput();
        m_physics = m_engine.GetPhysics();

        if (!m_renderer || !m_input || !m_physics) {
            LOG_ERROR("Failed to get engine systems");
            return false;
        }

        // Initialize camera system
        m_cameraSystem.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
        m_cameraSystem.SetTarget(&m_physicsCharacter);

        // Initialize physics-based character
        if (!m_physicsCharacter.Initialize(m_physics)) {
            LOG_ERROR("Failed to initialize physics character");
            return false;
        }
        m_physicsCharacter.SetPosition(Math::Vec3(-2.0f, 1.0f, 0.0f));

        // Initialize controller-based character
        if (!m_controllerCharacter.Initialize(m_physics)) {
            LOG_ERROR("Failed to initialize controller character");
            return false;
        }
        m_controllerCharacter.SetPosition(Math::Vec3(2.0f, 1.0f, 0.0f));

        // Set up engine callbacks
        m_engine.SetUpdateCallback([this](float deltaTime) { Update(deltaTime); });
        m_engine.SetRenderCallback([this]() { Render(); });

        LOG_INFO("Character Comparison initialized");
        LOG_INFO("Controls:");
        LOG_INFO("  WASD - Move character");
        LOG_INFO("  Space - Jump");
        LOG_INFO("  1 - Switch to Physics Character (forces/rigid body)");
        LOG_INFO("  2 - Switch to Controller Character (collision detection only)");

        return true;
    }

    void Run() {
        m_engine.Run();
    }

    void Update(float deltaTime) {
        // Handle character switching
        if (m_input->IsKeyPressed(KeyCode::Num1)) {
            m_activeCharacter = ActiveCharacter::Physics;
            m_cameraSystem.SetTarget(&m_physicsCharacter);
            LOG_INFO("Switched to Physics Character (rigid body with forces)");
        }
        if (m_input->IsKeyPressed(KeyCode::Num2)) {
            m_activeCharacter = ActiveCharacter::Controller;
            m_cameraSystem.SetTarget(&m_controllerCharacter);
            LOG_INFO("Switched to Controller Character (collision detection only)");
        }

        // Update active character
        if (m_activeCharacter == ActiveCharacter::Physics) {
            m_physicsCharacter.Update(deltaTime, m_input, &m_cameraSystem);
        } else {
            m_controllerCharacter.Update(deltaTime, m_input, &m_cameraSystem);
        }

        // Update camera
        m_cameraSystem.Update(deltaTime, m_input);
    }

    void Render() {
        // Set camera
        m_renderer->SetViewMatrix(m_cameraSystem.GetViewMatrix());
        m_renderer->SetProjectionMatrix(m_cameraSystem.GetProjectionMatrix());

        // Get primitive renderer
        auto primitiveRenderer = m_renderer->GetPrimitiveRenderer();
        if (!primitiveRenderer) return;

        // Render ground
        Math::Vec3 groundSize(20.0f, 0.1f, 20.0f);
        Math::Vec4 groundColor(0.3f, 0.7f, 0.3f, 1.0f);
        primitiveRenderer->DrawCube(Math::Vec3(0.0f, -0.05f, 0.0f), groundSize, groundColor);

        // Render both characters with different colors
        // Physics character (blue)
        Math::Vec3 physicsPos = m_physicsCharacter.GetPosition();
        Math::Vec3 physicsSize(0.6f, 1.8f, 0.6f);
        Math::Vec4 physicsColor = (m_activeCharacter == ActiveCharacter::Physics) ? 
            Math::Vec4(0.2f, 0.6f, 1.0f, 1.0f) : Math::Vec4(0.1f, 0.3f, 0.5f, 0.7f);
        primitiveRenderer->DrawCube(physicsPos, physicsSize, physicsColor);

        // Controller character (red)
        Math::Vec3 controllerPos = m_controllerCharacter.GetPosition();
        Math::Vec3 controllerSize(0.6f, 1.8f, 0.6f);
        Math::Vec4 controllerColor = (m_activeCharacter == ActiveCharacter::Controller) ? 
            Math::Vec4(1.0f, 0.2f, 0.2f, 1.0f) : Math::Vec4(0.5f, 0.1f, 0.1f, 0.7f);
        primitiveRenderer->DrawCube(controllerPos, controllerSize, controllerColor);

        // Render some obstacles for testing
        Math::Vec4 obstacleColor(0.6f, 0.4f, 0.2f, 1.0f);
        
        // Box obstacle
        primitiveRenderer->DrawCube(Math::Vec3(0.0f, 0.5f, -3.0f), Math::Vec3(1.0f, 1.0f, 1.0f), obstacleColor);
        
        // Step obstacle
        primitiveRenderer->DrawCube(Math::Vec3(3.0f, 0.15f, 2.0f), Math::Vec3(2.0f, 0.3f, 1.0f), obstacleColor);
        
        // Ramp
        primitiveRenderer->DrawCube(Math::Vec3(-3.0f, 0.25f, 2.0f), Math::Vec3(2.0f, 0.5f, 1.0f), obstacleColor);
    }

private:
    enum class ActiveCharacter {
        Physics,
        Controller
    };

    Engine m_engine;
    GraphicsRenderer* m_renderer = nullptr;
    InputManager* m_input = nullptr;
    PhysicsEngine* m_physics = nullptr;
    
    ThirdPersonCameraSystem m_cameraSystem;
    Character m_physicsCharacter;
    CharacterController m_controllerCharacter;
    
    ActiveCharacter m_activeCharacter = ActiveCharacter::Physics;
};

int main() {
    CharacterComparisonApp app;
    
    if (!app.Initialize()) {
        LOG_ERROR("Failed to initialize application");
        return -1;
    }
    
    app.Run();
    return 0;
}