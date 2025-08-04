#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Game/Character.h"
#include "Game/GameAudioManager.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/GridRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Texture.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include <GLFW/glfw3.h>

using namespace GameEngine;

class GameApplication {
public:
  enum class CharacterType {
    CharacterMovement,        // Character with CharacterMovementComponent (basic)
    Physics,                  // Character with PhysicsMovementComponent (realistic)
    Hybrid                    // Character with HybridMovementComponent (balanced) - DEFAULT
  };

  struct EnvironmentObject {
    Math::Vec3 position;
    Math::Vec3 scale;
    std::shared_ptr<Texture> texture;
    Math::Vec4 color;
    bool useTexture;
    bool useColor;
  };

  GameApplication() = default;
  ~GameApplication() = default;

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

    m_character = std::make_unique<Character>();
    if (!m_character->Initialize(m_engine.GetPhysics())) {
      LOG_ERROR("Failed to initialize character");
      return false;
    }
    
    Math::Vec3 spawnPosition(0.0f, 1.0f, 0.0f);
    m_character->SetSpawnPosition(spawnPosition);
    m_character->SetPosition(spawnPosition);
    m_character->SetFallLimit(-5.0f);

    m_character->SwitchToHybridMovement();
    LOG_INFO("Character initialized with HybridMovement (default)");

    // Try to load FBX T-Poser character model
    if (m_character->LoadFBXModel("assets/meshes/XBot.fbx")) {
      LOG_INFO("Successfully loaded FBX T-Poser character model");
      
      // Configure for Mixamo model (they export very large, need small scale)
      m_character->SetModelScale(0.01f); // Standard Mixamo scale - they export huge
      
      // Keep standard physics capsule dimensions (radius=0.3f, height=1.8f)
      // This provides good collision detection for a human-sized character
      m_character->SetCharacterSize(0.3f, 1.8f); // Standard human capsule
      
      // Calculate proper model offset to center scaled FBX model within physics capsule
      // At 0.01f scale, the Mixamo model becomes very small (about 1.8 units tall becomes 0.018 units)
      // We need to position it so the scaled model's feet align with the capsule bottom
      // Physics capsule: center at character position, extends from -0.9 to +0.9 in Y
      // Scaled model height: ~0.018 units, so we offset it down slightly
      Math::Vec3 modelOffset(0.0f, -0.89f, 0.0f); // Position near bottom of capsule
      m_character->SetModelOffset(modelOffset);
      
      LOG_INFO("Configured FBX model with 0.01f scale (Mixamo standard) and proper capsule alignment");
    } else {
      LOG_INFO("FBX model loading failed, using capsule representation as fallback");
    }

    m_camera = std::make_unique<ThirdPersonCameraSystem>();
    m_camera->SetTarget(m_character.get());
    m_camera->SetArmLength(10.0f);
    m_camera->SetRotationLimits(-45.0f, 45.0f);
    m_camera->SetSensitivity(0.8f, 0.6f);
    m_camera->SetMouseSensitivity(0.15f);

    m_engine.GetRenderer()->SetCamera(m_camera.get());
    m_engine.SetMainCamera(m_camera.get());

    auto *input = m_engine.GetInput();
    input->BindAction("move_forward", KeyCode::W);
    input->BindAction("move_backward", KeyCode::S);
    input->BindAction("move_left", KeyCode::A);
    input->BindAction("move_right", KeyCode::D);
    input->BindAction("jump", KeyCode::Space);
    input->BindAction("quit", KeyCode::Escape);

    // Initialize audio manager
    m_audioManager = std::make_unique<GameAudioManager>();
    if (!m_audioManager->Initialize(m_engine.GetAudio())) {
      LOG_WARNING("Failed to initialize audio manager - continuing without audio");
    }

    // Initialize grid renderer
    m_gridRenderer = std::make_unique<GridRenderer>();
    if (!m_gridRenderer->Initialize(m_primitiveRenderer.get())) {
      LOG_ERROR("Failed to initialize grid renderer");
      return false;
    }

    // Create environment objects
    CreateEnvironmentObjects();

    m_engine.SetUpdateCallback([this](float deltaTime) { this->Update(deltaTime); });
    m_engine.SetRenderCallback([this]() { this->Render(); });

    LOG_INFO("========================================");
    LOG_INFO("GAME ENGINE KIRO - ENHANCED EXAMPLE");
    LOG_INFO("========================================");
    LOG_INFO("Controls:");
    LOG_INFO("  WASD - Move character");
    LOG_INFO("  Space - Jump (with sound effect)");
    LOG_INFO("  Mouse - Look around");
    LOG_INFO("");
    LOG_INFO("Movement Types:");
    LOG_INFO("  1 - CharacterMovement (basic)");
    LOG_INFO("  2 - PhysicsMovement (realistic)");
    LOG_INFO("  3 - HybridMovement (balanced) - DEFAULT");
    LOG_INFO("");
    LOG_INFO("Audio Features:");
    LOG_INFO("  - Background music playing");
    LOG_INFO("  - Footstep sounds when walking");
    LOG_INFO("  - Jump sound effects");
    LOG_INFO("  - 3D spatial audio");
    LOG_INFO("");
    LOG_INFO("Debug Controls:");
    LOG_INFO("  F3 - Toggle debug capsule visualization");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("  F11 - Toggle fullscreen");
    LOG_INFO("  F1 - Exit");
    LOG_INFO("========================================");
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

    if (input->IsKeyPressed(KeyCode::Num1)) {
      m_activeCharacter = CharacterType::CharacterMovement;
      m_character->SwitchToCharacterMovement();
      m_camera->SetTarget(m_character.get());
      if (m_audioManager) {
        m_audioManager->OnCharacterTypeChanged();
      }
      LOG_INFO("Switched to CharacterMovement (basic)");
    }
    if (input->IsKeyPressed(KeyCode::Num2)) {
      m_activeCharacter = CharacterType::Physics;
      m_character->SwitchToPhysicsMovement();
      m_camera->SetTarget(m_character.get());
      if (m_audioManager) {
        m_audioManager->OnCharacterTypeChanged();
      }
      LOG_INFO("Switched to PhysicsMovement (realistic)");
    }
    if (input->IsKeyPressed(KeyCode::Num3)) {
      m_activeCharacter = CharacterType::Hybrid;
      m_character->SwitchToHybridMovement();
      m_camera->SetTarget(m_character.get());
      if (m_audioManager) {
        m_audioManager->OnCharacterTypeChanged();
      }
      LOG_INFO("Switched to HybridMovement (balanced) - RECOMMENDED");
    }

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

    if (input->IsKeyPressed(KeyCode::F11)) {
      static bool isFullscreen = true;
      isFullscreen = !isFullscreen;

      if (isFullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        LOG_INFO("Switched to fullscreen");
      } else {
        glfwSetWindowMonitor(window, nullptr, 100, 100, 1280, 720, 0);
        LOG_INFO("Switched to windowed mode");
      }
    }

    if (input->IsKeyPressed(KeyCode::F1)) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
      LOG_INFO("Exiting game");
      return;
    }

    if (input->IsKeyPressed(KeyCode::F2)) {
      Math::Vec3 testFallPosition(0.0f, 20.0f, 0.0f);
      m_character->SetPosition(testFallPosition);
      LOG_INFO("Testing fall detection - Character teleported high");
    }

    if (input->IsKeyPressed(KeyCode::F3)) {
      m_showDebugCapsule = !m_showDebugCapsule;
      LOG_INFO("Debug capsule visualization: " + std::string(m_showDebugCapsule ? "ON" : "OFF"));
    }

    m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
    
    if (m_character->HasFallen()) {
      LOG_INFO("Character has fallen! Resetting to spawn position");
      m_character->ResetToSpawnPosition();
    }
    
    // Update audio manager with character state
    if (m_audioManager) {
      m_audioManager->Update(deltaTime, m_character.get());
    }
    
    m_camera->Update(deltaTime, m_engine.GetInput());
  }

private:

  void CreateEnvironmentObjects() {
    // Create exactly 3 cubes with different material properties
    m_environmentObjects.clear();
    
    // Cube 1: Textured cube (using wall.jpg texture)
    EnvironmentObject texturedCube;
    texturedCube.position = Math::Vec3(-5.0f, 1.0f, 5.0f);
    texturedCube.scale = Math::Vec3(2.0f, 2.0f, 2.0f);
    texturedCube.texture = std::make_shared<Texture>();
    if (texturedCube.texture->LoadFromFile("assets/textures/wall.jpg")) {
      texturedCube.useTexture = true;
      texturedCube.useColor = false;
      LOG_INFO("Successfully loaded texture for environment cube 1");
    } else {
      // Fallback to solid color if texture fails
      texturedCube.useTexture = false;
      texturedCube.useColor = true;
      texturedCube.color = Math::Vec4(0.8f, 0.4f, 0.2f, 1.0f); // Orange fallback
      LOG_WARNING("Failed to load texture for cube 1, using color fallback");
    }
    m_environmentObjects.push_back(texturedCube);
    
    // Cube 2: Solid color cube (blue)
    EnvironmentObject colorCube;
    colorCube.position = Math::Vec3(5.0f, 1.0f, 5.0f);
    colorCube.scale = Math::Vec3(2.0f, 2.0f, 2.0f);
    colorCube.useTexture = false;
    colorCube.useColor = true;
    colorCube.color = Math::Vec4(0.2f, 0.4f, 0.8f, 1.0f); // Blue
    m_environmentObjects.push_back(colorCube);
    
    // Cube 3: Default material cube (no texture, no color - uses default rendering)
    EnvironmentObject defaultCube;
    defaultCube.position = Math::Vec3(0.0f, 1.0f, 8.0f);
    defaultCube.scale = Math::Vec3(2.0f, 2.0f, 2.0f);
    defaultCube.useTexture = false;
    defaultCube.useColor = false;
    defaultCube.color = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f); // White (default)
    m_environmentObjects.push_back(defaultCube);
    
    LOG_INFO("Created 3 environment objects with different material properties");
  }

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

    m_primitiveRenderer->DrawPlane(Math::Vec3(0.0f, 0.0f, 0.0f),
                                   Math::Vec2(100.0f),
                                   Math::Vec4(0.4f, 0.8f, 0.4f, 1.0f));

    // Render professional grid system
    if (m_gridRenderer) {
      m_gridRenderer->Render(viewProjection);
    }
    
    // Render environment objects
    RenderEnvironmentObjects();
    
    // Set debug capsule visualization state
    m_character->SetShowDebugCapsule(m_showDebugCapsule);
    m_character->Render(m_primitiveRenderer.get());
  }

  void RenderEnvironmentObjects() {
    for (const auto& obj : m_environmentObjects) {
      if (obj.useTexture && obj.texture) {
        // Render with texture
        m_primitiveRenderer->DrawCube(obj.position, obj.scale, obj.texture);
      } else if (obj.useColor) {
        // Render with solid color
        m_primitiveRenderer->DrawCube(obj.position, obj.scale, obj.color);
      } else {
        // Render with default material (white)
        m_primitiveRenderer->DrawCube(obj.position, obj.scale, Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
      }
    }
  }



  Engine m_engine;
  std::unique_ptr<ThirdPersonCameraSystem> m_camera;
  std::unique_ptr<Character> m_character;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
  std::unique_ptr<GameAudioManager> m_audioManager;
  std::unique_ptr<GridRenderer> m_gridRenderer;
  
  std::vector<EnvironmentObject> m_environmentObjects;
  
  CharacterType m_activeCharacter = CharacterType::Hybrid;
  bool m_showDebugCapsule = false;
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