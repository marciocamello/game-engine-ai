#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Game/Character.h"
#include "Game/CharacterController.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include <GLFW/glfw3.h>


using namespace GameEngine;

class GameApplication {
public:
  enum class CharacterType {
    CharacterDeterministic,   // Character with DeterministicMovementComponent
    CharacterHybrid,          // Character with HybridMovementComponent  
    CharacterPhysics,         // Character with PhysicsMovementComponent
    ControllerHybrid,         // CharacterController with HybridMovementComponent
    ControllerDeterministic,  // CharacterController with DeterministicMovementComponent
    ControllerPhysics         // CharacterController with PhysicsMovementComponent
  };

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

    // Create ground plane for physics
    CreateGroundPlane();

    // Initialize physics-based character
    m_character = std::make_unique<Character>();
    if (!m_character->Initialize(m_engine.GetPhysics())) {
      LOG_ERROR("Failed to initialize character");
      return false;
    }
    
    // Set spawn position and fall limit for character
    Math::Vec3 spawnPosition(0.0f, 1.0f, 0.0f);
    m_character->SetSpawnPosition(spawnPosition);
    m_character->SetPosition(spawnPosition);
    m_character->SetFallLimit(-2.0f);  // More sensitive fall detection

    // Initialize hybrid character controller
    LOG_INFO("Initializing CharacterController...");
    m_characterController = std::make_unique<CharacterController>();
    if (!m_characterController->Initialize(m_engine.GetPhysics())) {
      LOG_ERROR("Failed to initialize character controller");
      return false;
    }
    
    // Set spawn position and fall limit for character controller
    m_characterController->SetSpawnPosition(spawnPosition);
    m_characterController->SetPosition(spawnPosition);
    m_characterController->SetFallLimit(-2.0f);  // More sensitive fall detection
    LOG_INFO("CharacterController initialized successfully");

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
    LOG_INFO("Controls:");
    LOG_INFO("  WASD - Move character");
    LOG_INFO("  Space - Jump");
    LOG_INFO("  1 - Character + DeterministicMovement (blue, precise control)");
    LOG_INFO("  2 - Character + HybridMovement (blue, physics collision + direct control)");
    LOG_INFO("  3 - Character + PhysicsMovement (blue, full physics simulation)");
    LOG_INFO("  4 - CharacterController + HybridMovement (red, physics collision + direct control)");
    LOG_INFO("  5 - CharacterController + DeterministicMovement (red, precise control)");
    LOG_INFO("  6 - CharacterController + PhysicsMovement (red, full physics simulation)");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("  F1 - Exit");
    LOG_INFO("  F2 - Test fall detection (teleport character high up)");
    LOG_INFO("Fall Detection System:");
    LOG_INFO("  - Characters automatically reset when falling below Y = -2.0");
    LOG_INFO("  - Test by walking off the ground plane edges or pressing F2");
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

    // Character switching - always switch regardless of movement state
    if (input->IsKeyPressed(KeyCode::Num1)) {
      m_activeCharacter = CharacterType::CharacterDeterministic;
      m_character->SwitchToDeterministicMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to Character + DeterministicMovement (blue, precise control)");
    }
    if (input->IsKeyPressed(KeyCode::Num2)) {
      m_activeCharacter = CharacterType::CharacterHybrid;
      m_character->SwitchToHybridMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to Character + HybridMovement (blue, physics collision + direct control)");
    }
    if (input->IsKeyPressed(KeyCode::Num3)) {
      m_activeCharacter = CharacterType::CharacterPhysics;
      m_character->SwitchToPhysicsMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to Character + PhysicsMovement (blue, full physics simulation)");
    }
    if (input->IsKeyPressed(KeyCode::Num4)) {
      m_activeCharacter = CharacterType::ControllerHybrid;
      m_characterController->SwitchToHybridMovement();
      m_camera->SetTarget(m_character.get()); // Camera still follows Character for consistency
      LOG_INFO("Switched to CharacterController + HybridMovement (red, physics collision + direct control)");
    }
    if (input->IsKeyPressed(KeyCode::Num5)) {
      m_activeCharacter = CharacterType::ControllerDeterministic;
      m_characterController->SwitchToDeterministicMovement();
      m_camera->SetTarget(m_character.get()); // Camera still follows Character for consistency
      LOG_INFO("Switched to CharacterController + DeterministicMovement (red, precise control)");
    }
    if (input->IsKeyPressed(KeyCode::Num6)) {
      m_activeCharacter = CharacterType::ControllerPhysics;
      m_characterController->SwitchToPhysicsMovement();
      m_camera->SetTarget(m_character.get()); // Camera still follows Character for consistency
      LOG_INFO("Switched to CharacterController + PhysicsMovement (red, full physics simulation)");
    }

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

    // F2 to test fall detection - teleport character high up to test falling
    if (input->IsKeyPressed(KeyCode::F2)) {
      Math::Vec3 testFallPosition(0.0f, 20.0f, 0.0f); // High up in the air
      switch (m_activeCharacter) {
        case CharacterType::CharacterDeterministic:
        case CharacterType::CharacterHybrid:
        case CharacterType::CharacterPhysics:
          m_character->SetPosition(testFallPosition);
          LOG_INFO("Testing fall detection - Character teleported to high position");
          break;
        case CharacterType::ControllerHybrid:
        case CharacterType::ControllerDeterministic:
        case CharacterType::ControllerPhysics:
          m_characterController->SetPosition(testFallPosition);
          LOG_INFO("Testing fall detection - CharacterController teleported to high position");
          break;
      }
    }

    // Update active character
    switch (m_activeCharacter) {
      case CharacterType::CharacterDeterministic:
      case CharacterType::CharacterHybrid:
      case CharacterType::CharacterPhysics:
        // Update Character with its movement component
        m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
        
        // Check for fall and reset if needed
        if (m_character->HasFallen()) {
          LOG_INFO("Character has fallen! Resetting to spawn position...");
          m_character->ResetToSpawnPosition();
        }
        
        m_camera->Update(deltaTime, m_engine.GetInput());
        break;
        
      case CharacterType::ControllerHybrid:
      case CharacterType::ControllerDeterministic:
      case CharacterType::ControllerPhysics:
        // Update CharacterController WITH camera system so it can rotate correctly
        m_characterController->Update(deltaTime, m_engine.GetInput(), m_camera.get());
        
        // Check for fall and reset if needed
        if (m_characterController->HasFallen()) {
          LOG_INFO("CharacterController has fallen! Resetting to spawn position...");
          m_characterController->ResetToSpawnPosition();
        }
        
        // TRICK: Sync Character position with CharacterController so camera follows correctly
        // This way the ThirdPersonCameraSystem works normally
        m_character->SetPosition(m_characterController->GetPosition());
        m_character->SetRotation(m_characterController->GetRotation());
        
        // Update camera normally (it follows the Character which now has CharacterController's position)
        m_camera->Update(deltaTime, m_engine.GetInput());
        break;
    }
  }

private:
  void CreateGroundPlane() {
    // Create a static ground plane for physics collision
    auto* physics = m_engine.GetPhysics();
    if (!physics) {
      LOG_WARNING("No physics engine available for ground plane creation");
      return;
    }

    RigidBody groundDesc;
    groundDesc.position = Math::Vec3(0.0f, -0.5f, 0.0f); // Position ground plane below y=0
    groundDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    groundDesc.velocity = Math::Vec3(0.0f);
    groundDesc.mass = 0.0f; // Static body (mass = 0)
    groundDesc.restitution = 0.1f; // Low bounce
    groundDesc.friction = 0.8f; // High friction for good walking
    groundDesc.isStatic = true; // Static ground
    groundDesc.isKinematic = false;

    CollisionShape groundShape;
    groundShape.type = CollisionShape::Box;
    groundShape.dimensions = Math::Vec3(100.0f, 1.0f, 100.0f); // Large flat box

    uint32_t groundId = physics->CreateRigidBody(groundDesc, groundShape);
    if (groundId == 0) {
      LOG_ERROR("Failed to create ground plane rigid body");
    } else {
      LOG_INFO("Ground plane created with rigid body ID: " + std::to_string(groundId));
    }
  }

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

    // Draw active character
    switch (m_activeCharacter) {
      case CharacterType::CharacterDeterministic:
      case CharacterType::CharacterHybrid:
      case CharacterType::CharacterPhysics:
        m_character->Render(m_primitiveRenderer.get());
        break;
        
      case CharacterType::ControllerHybrid:
      case CharacterType::ControllerDeterministic:
      case CharacterType::ControllerPhysics:
        m_characterController->Render(m_primitiveRenderer.get());
        break;
    }
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

  void UpdateCameraForCharacterController() {
    // Manual camera update for CharacterController since ThirdPersonCameraSystem only works with Character
    Math::Vec3 characterPos = m_characterController->GetPosition();
    Math::Vec3 cameraOffset(0.0f, 5.0f, 10.0f); // Behind and above character
    
    // Calculate camera position
    Math::Vec3 cameraPos = characterPos + cameraOffset;
    
    // Update camera position manually using the underlying Camera
    // We need to access the Camera inside ThirdPersonCameraSystem
    // For now, let's create a simple follow camera
    m_camera->SetPosition(cameraPos);
    
    LOG_DEBUG("Camera following CharacterController at position: (" + 
             std::to_string(characterPos.x) + ", " + 
             std::to_string(characterPos.y) + ", " + 
             std::to_string(characterPos.z) + ")");
  }

  Engine m_engine;
  std::unique_ptr<ThirdPersonCameraSystem> m_camera;
  std::unique_ptr<Character> m_character;
  std::unique_ptr<CharacterController> m_characterController;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
  
  CharacterType m_activeCharacter = CharacterType::CharacterDeterministic; // Start with Character + DeterministicMovement
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