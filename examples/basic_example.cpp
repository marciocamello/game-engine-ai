// Basic Example - Clean demonstration of core character movement
// This example focuses on essential movement functionality without distractions
// Demonstrates: WASD movement, jumping, movement component switching, third-person camera

#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Game/Character.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include <GLFW/glfw3.h>

using namespace GameEngine;

class BasicGameApplication {
public:
  enum class CharacterType {
    CharacterMovement,        // Character with CharacterMovementComponent (basic)
    Physics,                  // Character with PhysicsMovementComponent (realistic)
    Hybrid                    // Character with HybridMovementComponent (balanced) - DEFAULT
  };

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

    CreateGroundPlane();

    // Initialize character with basic setup
    m_character = std::make_unique<Character>();
    if (!m_character->Initialize(m_engine.GetPhysics())) {
      LOG_ERROR("Failed to initialize character");
      return false;
    }
    
    Math::Vec3 spawnPosition(0.0f, 1.0f, 0.0f);
    m_character->SetSpawnPosition(spawnPosition);
    m_character->SetPosition(spawnPosition);
    m_character->SetFallLimit(-5.0f);

    // Set HybridMovement as default movement component
    m_character->SwitchToHybridMovement();
    LOG_INFO("Character initialized with HybridMovement (default)");

    // Initialize third-person camera system
    m_camera = std::make_unique<ThirdPersonCameraSystem>();
    m_camera->SetTarget(m_character.get());
    m_camera->SetArmLength(10.0f);
    m_camera->SetRotationLimits(-45.0f, 45.0f);
    m_camera->SetSensitivity(0.8f, 0.6f);
    m_camera->SetMouseSensitivity(0.15f);

    m_engine.GetRenderer()->SetCamera(m_camera.get());
    m_engine.SetMainCamera(m_camera.get());
    
    LOG_INFO("Third-person camera system initialized");

    // Bind essential input controls
    auto *input = m_engine.GetInput();
    input->BindAction("move_forward", KeyCode::W);
    input->BindAction("move_backward", KeyCode::S);
    input->BindAction("move_left", KeyCode::A);
    input->BindAction("move_right", KeyCode::D);
    input->BindAction("jump", KeyCode::Space);
    input->BindAction("quit", KeyCode::Escape);
    
    LOG_INFO("Input controls bound successfully");

    m_engine.SetUpdateCallback([this](float deltaTime) { this->Update(deltaTime); });
    m_engine.SetRenderCallback([this]() { this->Render(); });

    LOG_INFO("========================================");
    LOG_INFO("GAME ENGINE KIRO - BASIC EXAMPLE");
    LOG_INFO("========================================");
    LOG_INFO("");
    LOG_INFO("CORE FEATURES DEMONSTRATED:");
    LOG_INFO("  ✓ Character Movement: WASD controls with physics");
    LOG_INFO("  ✓ Jumping: Space key with physics simulation");
    LOG_INFO("  ✓ Camera System: Third-person camera with mouse control");
    LOG_INFO("  ✓ Movement Components: Three different movement types");
    LOG_INFO("");
    LOG_INFO("CONTROLS:");
    LOG_INFO("  WASD - Move character");
    LOG_INFO("  Space - Jump");
    LOG_INFO("  Mouse - Look around (third-person camera)");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("");
    LOG_INFO("MOVEMENT COMPONENTS:");
    LOG_INFO("  1 - CharacterMovement (basic movement)");
    LOG_INFO("  2 - PhysicsMovement (realistic physics)");
    LOG_INFO("  3 - HybridMovement (balanced) - DEFAULT");
    LOG_INFO("");
    LOG_INFO("This basic example focuses on core movement mechanics");
    LOG_INFO("For comprehensive feature demonstration, see the enhanced example");
    LOG_INFO("========================================");
    return true;
  }

  void Run() {
    LOG_INFO("Starting basic example game loop...");
    m_engine.Run();
  }

  void Update(float deltaTime) {
    auto *input = m_engine.GetInput();
    auto *window = m_engine.GetRenderer()->GetWindow();

    // Movement component switching
    if (input->IsKeyPressed(KeyCode::Num1)) {
      m_activeCharacter = CharacterType::CharacterMovement;
      m_character->SwitchToCharacterMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to CharacterMovement (basic movement component)");
    }
    if (input->IsKeyPressed(KeyCode::Num2)) {
      m_activeCharacter = CharacterType::Physics;
      m_character->SwitchToPhysicsMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to PhysicsMovement (realistic physics simulation)");
    }
    if (input->IsKeyPressed(KeyCode::Num3)) {
      m_activeCharacter = CharacterType::Hybrid;
      m_character->SwitchToHybridMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to HybridMovement (balanced physics + control)");
    }

    // Mouse capture toggle
    if (input->IsKeyPressed(KeyCode::Escape)) {
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

    // Exit application
    if (input->IsKeyPressed(KeyCode::F1)) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      LOG_INFO("Exiting basic example");
      return;
    }

    // Update character and camera
    m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
    
    // Handle character fall detection
    if (m_character->HasFallen()) {
      LOG_INFO("Character fall detection triggered - Resetting to spawn position");
      m_character->ResetToSpawnPosition();
    }
    
    m_camera->Update(deltaTime, m_engine.GetInput());
  }

private:
  void CreateGroundPlane() {
    auto* physics = m_engine.GetPhysics();
    if (!physics) {
      LOG_WARNING("No physics engine available for ground plane creation");
      return;
    }

    RigidBody groundDesc;
    groundDesc.position = Math::Vec3(0.0f, -0.5f, 0.0f);
    groundDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
    groundDesc.velocity = Math::Vec3(0.0f);
    groundDesc.mass = 0.0f;
    groundDesc.restitution = 0.1f;
    groundDesc.friction = 0.8f;
    groundDesc.isStatic = true;
    groundDesc.isKinematic = false;

    CollisionShape groundShape;
    groundShape.type = CollisionShape::Box;
    groundShape.dimensions = Math::Vec3(100.0f, 1.0f, 100.0f);

    uint32_t groundId = physics->CreateRigidBody(groundDesc, groundShape);
    if (groundId == 0) {
      LOG_ERROR("Failed to create ground plane rigid body");
    } else {
      LOG_INFO("Ground plane created successfully");
    }
  }

  void Render() {
    Math::Mat4 viewProjection = m_camera->GetViewProjectionMatrix();
    m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);

    // Render simple ground plane
    m_primitiveRenderer->DrawPlane(Math::Vec3(0.0f, 0.0f, 0.0f),
                                   Math::Vec2(100.0f),
                                   Math::Vec4(0.4f, 0.8f, 0.4f, 1.0f));

    // Render character (uses capsule fallback for clean basic example)
    m_character->Render(m_primitiveRenderer.get());
  }

  Engine m_engine;
  std::unique_ptr<ThirdPersonCameraSystem> m_camera;
  std::unique_ptr<Character> m_character;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
  
  CharacterType m_activeCharacter = CharacterType::Hybrid;
};

int main() {
  BasicGameApplication app;

  if (!app.Initialize()) {
    LOG_CRITICAL("Failed to initialize basic example application");
    return -1;
  }

  app.Run();

  LOG_INFO("Basic example terminated successfully");
  return 0;
}