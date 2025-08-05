// Basic Example - Minimal scene navigation with professional grid
// This example provides a clean environment for scene navigation, similar to Unreal Engine's viewport
// Demonstrates: Free camera navigation, professional grid system, minimal clean interface

#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/GridRenderer.h"
#include "Input/InputManager.h"
#include <GLFW/glfw3.h>

using namespace GameEngine;

/**
 * @brief Free camera for scene navigation (similar to Unreal Engine viewport camera)
 */
class FreeCamera : public Camera {
public:
    FreeCamera() {
        SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
        m_yaw = -90.0f;
        m_pitch = -20.0f;
        m_moveSpeed = 10.0f;
        m_mouseSensitivity = 0.1f;
        UpdateCameraVectors();
    }

    void Update(float deltaTime, InputManager* input) {
        // Movement with WASD + E/Q
        Math::Vec3 velocity(0.0f);
        
        if (input->IsActionDown("move_forward")) {
            velocity += m_front;
        }
        if (input->IsActionDown("move_backward")) {
            velocity -= m_front;
        }
        if (input->IsActionDown("move_left")) {
            velocity -= m_right;
        }
        if (input->IsActionDown("move_right")) {
            velocity += m_right;
        }
        if (input->IsActionDown("move_up")) {
            velocity += m_worldUp;
        }
        if (input->IsActionDown("move_down")) {
            velocity -= m_worldUp;
        }

        // Apply movement
        if (velocity.x != 0.0f || velocity.y != 0.0f || velocity.z != 0.0f) {
            Math::Vec3 normalizedVelocity = glm::normalize(velocity);
            Math::Vec3 currentPos = GetPosition();
            SetPosition(currentPos + normalizedVelocity * m_moveSpeed * deltaTime);
        }

        // Mouse look
        auto mouseDelta = input->GetMouseDelta();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            m_yaw += mouseDelta.x * m_mouseSensitivity;
            m_pitch -= mouseDelta.y * m_mouseSensitivity;

            // Constrain pitch
            if (m_pitch > 89.0f) m_pitch = 89.0f;
            if (m_pitch < -89.0f) m_pitch = -89.0f;

            UpdateCameraVectors();
        }
    }

    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    float GetMoveSpeed() const { return m_moveSpeed; }

private:
    void UpdateCameraVectors() {
        Math::Vec3 front;
        front.x = cosf(Math::ToRadians(m_yaw)) * cosf(Math::ToRadians(m_pitch));
        front.y = sinf(Math::ToRadians(m_pitch));
        front.z = sinf(Math::ToRadians(m_yaw)) * cosf(Math::ToRadians(m_pitch));
        
        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
        
        // Update camera rotation using GLM lookAt
        Math::Vec3 currentPos = GetPosition();
        LookAt(currentPos + m_front, m_up);
    }

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_moveSpeed = 10.0f;
    float m_mouseSensitivity = 0.1f;
    
    Math::Vec3 m_front{0.0f, 0.0f, -1.0f};
    Math::Vec3 m_up{0.0f, 1.0f, 0.0f};
    Math::Vec3 m_right{1.0f, 0.0f, 0.0f};
    Math::Vec3 m_worldUp{0.0f, 1.0f, 0.0f};
};

class BasicGameApplication {
public:
  BasicGameApplication() = default;
  ~BasicGameApplication() {
    LOG_INFO("BasicGameApplication cleaned up successfully");
  }

  bool Initialize() {
    if (!m_engine.Initialize()) {
      LOG_ERROR("Failed to initialize game engine");
      return false;
    }

    m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
    if (!m_primitiveRenderer->Initialize()) {
      LOG_ERROR("Failed to initialize primitive renderer");
      return false;
    }

    // Initialize free camera for scene navigation
    m_camera = std::make_unique<FreeCamera>();
    m_engine.GetRenderer()->SetCamera(m_camera.get());
    m_engine.SetMainCamera(m_camera.get());
    
    LOG_INFO("Free camera initialized for scene navigation");

    // Initialize professional grid renderer
    m_gridRenderer = std::make_unique<GridRenderer>();
    if (!m_gridRenderer->Initialize(m_primitiveRenderer.get())) {
      LOG_ERROR("Failed to initialize grid renderer");
      return false;
    } else {
      LOG_INFO("Professional grid system initialized");
    }

    // Bind navigation controls (similar to Unreal Engine viewport)
    auto *input = m_engine.GetInput();
    input->BindAction("move_forward", KeyCode::W);
    input->BindAction("move_backward", KeyCode::S);
    input->BindAction("move_left", KeyCode::A);
    input->BindAction("move_right", KeyCode::D);
    input->BindAction("move_up", KeyCode::E);      // Move up
    input->BindAction("move_down", KeyCode::Q);    // Move down
    input->BindAction("quit", KeyCode::Escape);
    
    LOG_INFO("Navigation controls bound successfully");

    m_engine.SetUpdateCallback([this](float deltaTime) { this->Update(deltaTime); });
    m_engine.SetRenderCallback([this]() { this->Render(); });

    LOG_INFO("========================================");
    LOG_INFO("GAME ENGINE KIRO - BASIC SCENE NAVIGATION");
    LOG_INFO("========================================");
    LOG_INFO("");
    LOG_INFO("MINIMAL FEATURES:");
    LOG_INFO("  ✓ Professional Grid System: Clean reference grid");
    LOG_INFO("  ✓ Free Camera Navigation: Unreal Engine-style viewport camera");
    LOG_INFO("  ✓ Clean Interface: No distractions, pure navigation");
    LOG_INFO("");
    LOG_INFO("NAVIGATION CONTROLS:");
    LOG_INFO("  WASD - Move camera horizontally");
    LOG_INFO("  E/Q - Move camera up/down");
    LOG_INFO("  Mouse - Look around (free camera)");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("  F1 - Exit application");
    LOG_INFO("");
    LOG_INFO("SPEED CONTROLS:");
    LOG_INFO("  Shift - Increase camera speed");
    LOG_INFO("  Ctrl - Decrease camera speed");
    LOG_INFO("");
    LOG_INFO("This basic example provides clean scene navigation");
    LOG_INFO("For comprehensive feature demonstration, see the enhanced example");
    LOG_INFO("========================================");
    return true;
  }

  void Run() {
    LOG_INFO("Starting basic scene navigation...");
    m_engine.Run();
  }

  void Update(float deltaTime) {
    auto *input = m_engine.GetInput();
    auto *window = m_engine.GetRenderer()->GetWindow();

    // Camera speed adjustment
    if (input->IsKeyPressed(KeyCode::LeftShift) || input->IsKeyPressed(KeyCode::RightShift)) {
      m_camera->SetMoveSpeed(m_camera->GetMoveSpeed() * 1.5f);
      LOG_INFO("Camera speed increased to " + std::to_string(m_camera->GetMoveSpeed()));
    }
    if (input->IsKeyPressed(KeyCode::LeftCtrl) || input->IsKeyPressed(KeyCode::RightCtrl)) {
      m_camera->SetMoveSpeed(m_camera->GetMoveSpeed() * 0.75f);
      LOG_INFO("Camera speed decreased to " + std::to_string(m_camera->GetMoveSpeed()));
    }

    // Mouse capture toggle
    if (input->IsKeyPressed(KeyCode::Escape)) {
      static bool mouseCaptured = true;
      mouseCaptured = !mouseCaptured;
      if (mouseCaptured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        LOG_INFO("Mouse captured for navigation");
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        LOG_INFO("Mouse released");
      }
    }

    // Exit application
    if (input->IsKeyPressed(KeyCode::F1)) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      LOG_INFO("Exiting basic scene navigation");
      return;
    }

    // Update camera
    m_camera->Update(deltaTime, m_engine.GetInput());
  }

private:
  void Render() {
    Math::Mat4 viewProjection = m_camera->GetViewProjectionMatrix();
    m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);

    // Render professional grid system only
    if (m_gridRenderer) {
      m_gridRenderer->Render(viewProjection);
    }
  }

  Engine m_engine;
  std::unique_ptr<FreeCamera> m_camera;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
  std::unique_ptr<GridRenderer> m_gridRenderer;
};

int main() {
  BasicGameApplication app;

  if (!app.Initialize()) {
    LOG_CRITICAL("Failed to initialize basic scene navigation application");
    return -1;
  }

  app.Run();

  LOG_INFO("Basic scene navigation terminated successfully");
  return 0;
}