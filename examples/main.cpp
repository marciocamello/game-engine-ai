#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Game/Character.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include <GLFW/glfw3.h>


using namespace GameEngine;

class GameApplication {
public:
  GameApplication() = default;
  ~GameApplication() = default;

  bool Initialize() {
    // Initialize the engine
    if (!m_engine.Initialize()) {
      LOG_ERROR("Failed to initialize game engine");
      return false;
    }

    // Initialize primitive renderer
    m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
    if (!m_primitiveRenderer->Initialize()) {
      LOG_ERROR("Failed to initialize primitive renderer");
      return false;
    }

    // Initialize character
    m_character = std::make_unique<Character>();
    if (!m_character->Initialize(m_engine.GetPhysics())) {
      LOG_ERROR("Failed to initialize character");
      return false;
    }

    // Setup third-person camera system
    m_camera = std::make_unique<ThirdPersonCameraSystem>();
    m_camera->SetTarget(m_character.get());
    m_camera->SetArmLength(10.0f);
    m_camera->SetRotationLimits(-45.0f, 30.0f);
    m_camera->SetSensitivity(0.8f, 0.6f);
    m_camera->SetMouseSensitivity(0.15f);

    // Set camera in renderer
    m_engine.GetRenderer()->SetCamera(m_camera.get());

    // Setup input bindings
    auto *input = m_engine.GetInput();
    input->BindAction("move_forward", KeyCode::W);
    input->BindAction("move_backward", KeyCode::S);
    input->BindAction("move_left", KeyCode::A);
    input->BindAction("move_right", KeyCode::D);
    input->BindAction("jump", KeyCode::Space);
    input->BindAction("quit", KeyCode::Escape);

    // Setup engine callbacks
    m_engine.SetUpdateCallback(
        [this](float deltaTime) { this->Update(deltaTime); });
    m_engine.SetRenderCallback([this]() { this->Render(); });

    LOG_INFO("Game application initialized successfully");
    return true;
  }

  void Run() {
    LOG_INFO("Starting game loop...");

    // Use the engine's built-in run method with our callbacks
    m_engine.Run();
  }

  void Update(float deltaTime) {
    auto *input = m_engine.GetInput();
    auto *window = m_engine.GetRenderer()->GetWindow();

    // ESC to release mouse cursor (for debugging/exiting)
    if (input->IsKeyPressed(KeyCode::Escape)) {
      static bool mouseCaptured = true;
      mouseCaptured = !mouseCaptured;
      if (mouseCaptured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        LOG_INFO("Mouse captured for camera control");
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        LOG_INFO("Mouse released - press ESC again to recapture");
      }
    }

    // F11 to toggle fullscreen
    if (input->IsKeyPressed(KeyCode::F11)) {
      static bool isFullscreen = true; // Start in fullscreen
      isFullscreen = !isFullscreen;

      if (isFullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                             mode->refreshRate);
        LOG_INFO("Switched to fullscreen");
      } else {
        glfwSetWindowMonitor(window, nullptr, 100, 100, 1280, 720, 0);
        LOG_INFO("Switched to windowed mode");
      }
    }

    // F1 to exit game
    if (input->IsKeyPressed(KeyCode::F1)) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      LOG_INFO("Exiting game");
      return;
    }

    // Update character
    m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());

    // Update camera
    m_camera->Update(deltaTime, m_engine.GetInput());
  }

private:
  void Render() {
    auto *renderer = m_engine.GetRenderer();

    // Set view-projection matrix for primitive renderer
    Math::Mat4 viewProjection = m_camera->GetViewProjectionMatrix();
    m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);

    // Draw ground plane
    m_primitiveRenderer->DrawPlane(Math::Vec3(0.0f, 0.0f, 0.0f),
                                   Math::Vec2(100.0f),
                                   Math::Vec4(0.4f, 0.8f, 0.4f, 1.0f));

    // Draw grid lines to see movement
    DrawGrid();

    // Draw only the character
    m_character->Render(m_primitiveRenderer.get());
  }

  void DrawGrid() {
    // Draw grid lines every 2 units
    float gridSize = 50.0f;
    float gridSpacing = 2.0f;
    Math::Vec4 gridColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Draw lines along X axis
    for (float z = -gridSize; z <= gridSize; z += gridSpacing) {
      for (float x = -gridSize; x < gridSize; x += gridSpacing) {
        m_primitiveRenderer->DrawCube(
            Math::Vec3(x, 0.01f, z),
            Math::Vec3(gridSpacing * 0.9f, 0.02f, 0.1f), gridColor);
      }
    }

    // Draw lines along Z axis
    for (float x = -gridSize; x <= gridSize; x += gridSpacing) {
      for (float z = -gridSize; z < gridSize; z += gridSpacing) {
        m_primitiveRenderer->DrawCube(
            Math::Vec3(x, 0.01f, z),
            Math::Vec3(0.1f, 0.02f, gridSpacing * 0.9f), gridColor);
      }
    }
  }

  Engine m_engine;
  std::unique_ptr<ThirdPersonCameraSystem> m_camera;
  std::unique_ptr<Character> m_character;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
};

int main() {
  GameApplication app;

  if (!app.Initialize()) {
    LOG_CRITICAL("Failed to initialize application");
    return -1;
  }

  app.Run();

  LOG_INFO("Application terminated successfully");
  return 0;
}