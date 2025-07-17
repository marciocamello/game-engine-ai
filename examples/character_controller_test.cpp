/**
 * CharacterController Test Example
 * 
 * Simple test to verify the CharacterController hybrid physics approach works correctly.
 * This focuses on testing the collision detection and movement resolution.
 */

#include "Core/Engine.h"
#include "Game/CharacterController.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Camera.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <GLFW/glfw3.h>

using namespace GameEngine;

class CharacterControllerTest {
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

        // Initialize primitive renderer (like GameExample does)
        m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
        if (!m_primitiveRenderer->Initialize()) {
            LOG_ERROR("Failed to initialize primitive renderer");
            return false;
        }

        // Initialize simple camera
        m_camera = std::make_unique<Camera>();
        m_camera->SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
        m_camera->SetTarget(Math::Vec3(0.0f, 0.0f, 0.0f));
        m_camera->SetPerspective(45.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);

        // Set camera in renderer
        m_renderer->SetCamera(m_camera.get());

        // Initialize character controller
        if (!m_characterController.Initialize(m_physics)) {
            LOG_ERROR("Failed to initialize character controller");
            return false;
        }
        m_characterController.SetPosition(Math::Vec3(0.0f, 1.0f, 0.0f));

        // Create some test obstacles in the physics world
        CreateTestObstacles();

        // Set up engine callbacks
        m_engine.SetUpdateCallback([this](float deltaTime) { Update(deltaTime); });
        m_engine.SetRenderCallback([this]() { Render(); });

        LOG_INFO("CharacterController Test initialized");
        LOG_INFO("Controls:");
        LOG_INFO("  WASD - Move character");
        LOG_INFO("  Space - Jump");
        LOG_INFO("  ESC - Toggle mouse capture");
        LOG_INFO("  F1 - Exit");

        return true;
    }

    void Run() {
        m_engine.Run();
    }

private:
    void CreateTestObstacles() {
        // Create ground plane
        RigidBody groundDesc;
        groundDesc.position = Math::Vec3(0.0f, -0.5f, 0.0f);
        groundDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
        groundDesc.mass = 0.0f; // Static
        groundDesc.isStatic = true;
        
        CollisionShape groundShape;
        groundShape.type = CollisionShape::Box;
        groundShape.dimensions = Math::Vec3(20.0f, 1.0f, 20.0f);
        
        m_physics->CreateRigidBody(groundDesc, groundShape);

        // Create a box obstacle
        RigidBody boxDesc;
        boxDesc.position = Math::Vec3(3.0f, 0.5f, 0.0f);
        boxDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
        boxDesc.mass = 0.0f; // Static
        boxDesc.isStatic = true;
        
        CollisionShape boxShape;
        boxShape.type = CollisionShape::Box;
        boxShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
        
        m_physics->CreateRigidBody(boxDesc, boxShape);

        // Create a step obstacle
        RigidBody stepDesc;
        stepDesc.position = Math::Vec3(-3.0f, 0.15f, 0.0f);
        stepDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
        stepDesc.mass = 0.0f; // Static
        stepDesc.isStatic = true;
        
        CollisionShape stepShape;
        stepShape.type = CollisionShape::Box;
        stepShape.dimensions = Math::Vec3(2.0f, 0.3f, 1.0f);
        
        m_physics->CreateRigidBody(stepDesc, stepShape);

        LOG_INFO("Created test obstacles for CharacterController");
    }

    void Update(float deltaTime) {
        auto* window = m_renderer->GetWindow();

        // ESC to toggle mouse capture
        if (m_input->IsKeyPressed(KeyCode::Escape)) {
            static bool mouseCaptured = true;
            mouseCaptured = !mouseCaptured;
            if (mouseCaptured) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                LOG_INFO("Mouse captured");
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                LOG_INFO("Mouse released");
            }
        }

        // F1 to exit
        if (m_input->IsKeyPressed(KeyCode::F1)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            LOG_INFO("Exiting test");
            return;
        }

        // Update character controller
        m_characterController.Update(deltaTime, m_input, nullptr);

        // Update camera to follow character
        Math::Vec3 characterPos = m_characterController.GetPosition();
        Math::Vec3 cameraPos = characterPos + Math::Vec3(0.0f, 5.0f, 10.0f);
        m_camera->SetPosition(cameraPos);
        m_camera->SetTarget(characterPos);

        // Log character state periodically
        static float logTimer = 0.0f;
        logTimer += deltaTime;
        if (logTimer >= 2.0f) {
            Math::Vec3 pos = m_characterController.GetPosition();
            Math::Vec3 vel = m_characterController.GetVelocity();
            LOG_INFO("CharacterController - Position: (" + 
                    std::to_string(pos.x) + ", " + 
                    std::to_string(pos.y) + ", " + 
                    std::to_string(pos.z) + "), Velocity: (" +
                    std::to_string(vel.x) + ", " + 
                    std::to_string(vel.y) + ", " + 
                    std::to_string(vel.z) + "), State: " +
                    (m_characterController.IsGrounded() ? "Grounded" : "Airborne"));
            logTimer = 0.0f;
        }
    }

    void Render() {
        // Set view-projection matrix for primitive renderer (like GameExample)
        Math::Mat4 viewProjection = m_camera->GetViewProjectionMatrix();
        m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);

        // Render ground plane (green)
        m_primitiveRenderer->DrawPlane(Math::Vec3(0.0f, 0.0f, 0.0f),
                                      Math::Vec2(20.0f),
                                      Math::Vec4(0.3f, 0.7f, 0.3f, 1.0f));

        // Render character controller (red cube)
        m_characterController.Render(m_primitiveRenderer.get());
        
        // Render test obstacles (brown)
        Math::Vec4 obstacleColor(0.6f, 0.4f, 0.2f, 1.0f);
        
        // Box obstacle
        m_primitiveRenderer->DrawCube(Math::Vec3(3.0f, 0.5f, 0.0f), 
                                     Math::Vec3(1.0f, 1.0f, 1.0f), 
                                     obstacleColor);
        
        // Step obstacle  
        m_primitiveRenderer->DrawCube(Math::Vec3(-3.0f, 0.15f, 0.0f), 
                                     Math::Vec3(2.0f, 0.3f, 1.0f), 
                                     obstacleColor);

        // Draw simple grid for reference
        DrawGrid();
    }

    void DrawGrid() {
        // Draw simple grid lines
        float gridSize = 10.0f;
        float gridSpacing = 1.0f;
        Math::Vec4 gridColor(0.2f, 0.2f, 0.2f, 0.5f);

        // Draw lines along X axis
        for (float z = -gridSize; z <= gridSize; z += gridSpacing) {
            m_primitiveRenderer->DrawCube(
                Math::Vec3(0.0f, 0.01f, z),
                Math::Vec3(gridSize * 2.0f, 0.02f, 0.05f), 
                gridColor);
        }

        // Draw lines along Z axis
        for (float x = -gridSize; x <= gridSize; x += gridSpacing) {
            m_primitiveRenderer->DrawCube(
                Math::Vec3(x, 0.01f, 0.0f),
                Math::Vec3(0.05f, 0.02f, gridSize * 2.0f), 
                gridColor);
        }
    }

    Engine m_engine;
    GraphicsRenderer* m_renderer = nullptr;
    InputManager* m_input = nullptr;
    PhysicsEngine* m_physics = nullptr;
    
    std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
    std::unique_ptr<Camera> m_camera;
    CharacterController m_characterController;
};

int main() {
    CharacterControllerTest test;
    
    if (!test.Initialize()) {
        LOG_ERROR("Failed to initialize CharacterController test");
        return -1;
    }
    
    test.Run();
    return 0;
}