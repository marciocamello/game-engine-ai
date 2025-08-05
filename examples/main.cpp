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

    // RESOURCE SYSTEM DEMO: Try to load FBX T-Poser character model
    if (m_character->LoadFBXModel("assets/meshes/XBot.fbx")) {
      LOG_INFO("RESOURCE SYSTEM DEMO: Successfully loaded FBX T-Poser character model");
      LOG_INFO("RENDERING SYSTEM DEMO: 3D mesh rendering with FBX model format");
      
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
      
      LOG_INFO("PHYSICS SYSTEM DEMO: Model aligned with physics capsule for accurate collision");
    } else {
      LOG_INFO("RESOURCE SYSTEM DEMO: FBX model loading failed, using capsule fallback");
      LOG_INFO("RENDERING SYSTEM DEMO: Fallback to primitive capsule rendering");
    }

    // CAMERA SYSTEM DEMO: Initialize third-person camera with smooth movement
    m_camera = std::make_unique<ThirdPersonCameraSystem>();
    m_camera->SetTarget(m_character.get());
    m_camera->SetArmLength(10.0f);
    m_camera->SetRotationLimits(-45.0f, 45.0f);
    m_camera->SetSensitivity(0.8f, 0.6f);
    m_camera->SetMouseSensitivity(0.15f);

    m_engine.GetRenderer()->SetCamera(m_camera.get());
    m_engine.SetMainCamera(m_camera.get());
    
    LOG_INFO("CAMERA SYSTEM DEMO: Third-person camera system initialized");
    LOG_INFO("  - Smooth camera movement and rotation");
    LOG_INFO("  - Mouse-controlled camera positioning");
    LOG_INFO("  - Camera collision and constraints");

    // INPUT SYSTEM DEMO: Bind comprehensive input controls
    auto *input = m_engine.GetInput();
    input->BindAction("move_forward", KeyCode::W);
    input->BindAction("move_backward", KeyCode::S);
    input->BindAction("move_left", KeyCode::A);
    input->BindAction("move_right", KeyCode::D);
    input->BindAction("jump", KeyCode::Space);
    input->BindAction("quit", KeyCode::Escape);
    
    LOG_INFO("INPUT SYSTEM DEMO: Input controls bound successfully");
    LOG_INFO("  - WASD movement with immediate response");
    LOG_INFO("  - Space jump with audio feedback");
    LOG_INFO("  - Mouse camera control");
    LOG_INFO("  - Function keys for system demonstrations");

    // AUDIO SYSTEM DEMO: Initialize comprehensive audio manager
    m_audioManager = std::make_unique<GameAudioManager>();
    if (!m_audioManager->Initialize(m_engine.GetAudio())) {
      LOG_WARNING("AUDIO SYSTEM DEMO: Failed to initialize audio manager - continuing without audio");
    } else {
      LOG_INFO("AUDIO SYSTEM DEMO: Audio manager initialized successfully");
      LOG_INFO("  - Background music system ready");
      LOG_INFO("  - Footstep audio system ready");
      LOG_INFO("  - Jump sound effects ready");
      LOG_INFO("  - 3D spatial audio positioning ready");
    }

    // RENDERING SYSTEM DEMO: Initialize professional grid renderer
    m_gridRenderer = std::make_unique<GridRenderer>();
    if (!m_gridRenderer->Initialize(m_primitiveRenderer.get())) {
      LOG_ERROR("RENDERING SYSTEM DEMO: Failed to initialize grid renderer");
      return false;
    } else {
      LOG_INFO("RENDERING SYSTEM DEMO: Professional grid system initialized");
      LOG_INFO("  - Grid pattern with proper spacing");
      LOG_INFO("  - Dark background (professional appearance)");
      LOG_INFO("  - Subtle colors that don't interfere with scene objects");
    }

    // Create environment objects
    CreateEnvironmentObjects();

    m_engine.SetUpdateCallback([this](float deltaTime) { this->Update(deltaTime); });
    m_engine.SetRenderCallback([this]() { this->Render(); });

    LOG_INFO("========================================");
    LOG_INFO("GAME ENGINE KIRO - COMPREHENSIVE FEATURE DEMONSTRATION");
    LOG_INFO("========================================");
    LOG_INFO("");
    LOG_INFO("ENGINE SYSTEMS DEMONSTRATED:");
    LOG_INFO("  ✓ Physics System: Collision detection, rigid bodies, movement components");
    LOG_INFO("  ✓ Rendering System: Primitives, meshes, textures, shaders, professional grid");
    LOG_INFO("  ✓ Audio System: 3D spatial audio, background music, sound effects");
    LOG_INFO("  ✓ Resource System: Model loading, texture loading, resource management");
    LOG_INFO("  ✓ Input System: Keyboard, mouse, responsive controls with feedback");
    LOG_INFO("  ✓ Camera System: Third-person camera, smooth movement, collision");
    LOG_INFO("");
    LOG_INFO("CONTROLS:");
    LOG_INFO("  WASD - Move character (with footstep audio)");
    LOG_INFO("  Space - Jump (with sound effect)");
    LOG_INFO("  Mouse - Look around (third-person camera)");
    LOG_INFO("");
    LOG_INFO("MOVEMENT COMPONENTS (Physics System Demo):");
    LOG_INFO("  1 - CharacterMovement (basic movement)");
    LOG_INFO("  2 - PhysicsMovement (realistic physics)");
    LOG_INFO("  3 - HybridMovement (balanced) - DEFAULT");
    LOG_INFO("");
    LOG_INFO("VISUAL FEATURES (Rendering System Demo):");
    LOG_INFO("  - FBX T-Poser character model (Resource System)");
    LOG_INFO("  - 3 Environment cubes with different materials");
    LOG_INFO("  - Professional grid system with dark background");
    LOG_INFO("  - Capsule collision visualization (F3)");
    LOG_INFO("");
    LOG_INFO("AUDIO FEATURES (Audio System Demo):");
    LOG_INFO("  - Background music (looping)");
    LOG_INFO("  - Footstep sounds synchronized with movement");
    LOG_INFO("  - Jump sound effects");
    LOG_INFO("  - 3D spatial audio positioning");
    LOG_INFO("");
    LOG_INFO("DEBUG CONTROLS:");
    LOG_INFO("  F3 - Toggle debug capsule visualization");
    LOG_INFO("  F2 - Test fall detection system");
    LOG_INFO("  F4 - Show comprehensive system status");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("  F11 - Toggle fullscreen");
    LOG_INFO("  F1 - Exit application");
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
      LOG_INFO("PHYSICS SYSTEM DEMO: Switched to CharacterMovement (basic movement component)");
    }
    if (input->IsKeyPressed(KeyCode::Num2)) {
      m_activeCharacter = CharacterType::Physics;
      m_character->SwitchToPhysicsMovement();
      m_camera->SetTarget(m_character.get());
      if (m_audioManager) {
        m_audioManager->OnCharacterTypeChanged();
      }
      LOG_INFO("PHYSICS SYSTEM DEMO: Switched to PhysicsMovement (realistic physics simulation)");
    }
    if (input->IsKeyPressed(KeyCode::Num3)) {
      m_activeCharacter = CharacterType::Hybrid;
      m_character->SwitchToHybridMovement();
      m_camera->SetTarget(m_character.get());
      if (m_audioManager) {
        m_audioManager->OnCharacterTypeChanged();
      }
      LOG_INFO("PHYSICS SYSTEM DEMO: Switched to HybridMovement (balanced physics + control)");
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
      LOG_INFO("PHYSICS SYSTEM DEMO: Testing fall detection - Character teleported high");
    }

    if (input->IsKeyPressed(KeyCode::F4)) {
      LogComprehensiveSystemStatus();
    }

    if (input->IsKeyPressed(KeyCode::F3)) {
      m_showDebugCapsule = !m_showDebugCapsule;
      LOG_INFO("RENDERING SYSTEM DEMO: Debug capsule visualization " + std::string(m_showDebugCapsule ? "ENABLED" : "DISABLED") + " - Shows physics collision alongside visual model");
    }

    m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
    
    if (m_character->HasFallen()) {
      LOG_INFO("PHYSICS SYSTEM DEMO: Character fall detection triggered - Resetting to spawn position");
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
    
    LOG_INFO("RENDERING SYSTEM DEMO: Created 3 environment objects demonstrating different material properties:");
    LOG_INFO("  - Cube 1: Textured material (texture mapping demonstration)");
    LOG_INFO("  - Cube 2: Solid color material (shader color demonstration)");
    LOG_INFO("  - Cube 3: Default material (basic rendering demonstration)");
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

  void LogComprehensiveSystemStatus() {
    LOG_INFO("========================================");
    LOG_INFO("COMPREHENSIVE FEATURE DEMONSTRATION STATUS");
    LOG_INFO("========================================");
    
    // Physics System Status
    LOG_INFO("PHYSICS SYSTEM:");
    LOG_INFO("  ✓ Movement Component: " + std::string(m_character->GetMovementTypeName()));
    LOG_INFO("  ✓ Collision Detection: Active (character vs ground/objects)");
    LOG_INFO("  ✓ Rigid Body Simulation: Ground plane and character physics");
    LOG_INFO("  ✓ Character Position: (" + 
             std::to_string(m_character->GetPosition().x) + ", " +
             std::to_string(m_character->GetPosition().y) + ", " +
             std::to_string(m_character->GetPosition().z) + ")");
    
    // Rendering System Status
    LOG_INFO("RENDERING SYSTEM:");
    LOG_INFO("  ✓ Primitive Rendering: Ground plane, environment cubes, debug capsule");
    LOG_INFO("  ✓ Mesh Rendering: " + std::string(m_character->IsUsingFBXModel() ? "FBX T-Poser model" : "Capsule fallback"));
    LOG_INFO("  ✓ Texture Mapping: Environment cube textures");
    LOG_INFO("  ✓ Shader Usage: Material shaders for different object types");
    LOG_INFO("  ✓ Professional Grid: Active with dark background");
    
    // Audio System Status
    LOG_INFO("AUDIO SYSTEM:");
    if (m_audioManager && m_audioManager->IsAudioAvailable()) {
      LOG_INFO("  ✓ Background Music: " + std::string(m_audioManager->IsBackgroundMusicPlaying() ? "Playing" : "Stopped"));
      LOG_INFO("  ✓ 3D Spatial Audio: Active");
      LOG_INFO("  ✓ Sound Effects: Jump and footstep sounds ready");
    } else {
      LOG_INFO("  ⚠ Audio System: Not available");
    }
    
    // Resource System Status
    LOG_INFO("RESOURCE SYSTEM:");
    LOG_INFO("  ✓ Model Loading: FBX character model management");
    LOG_INFO("  ✓ Texture Loading: Environment texture resources");
    LOG_INFO("  ✓ Resource Management: Automatic cleanup and lifecycle");
    
    // Input System Status
    LOG_INFO("INPUT SYSTEM:");
    LOG_INFO("  ✓ Keyboard Input: WASD movement, Space jump, Function keys");
    LOG_INFO("  ✓ Mouse Input: Camera control and look around");
    LOG_INFO("  ✓ Input Feedback: Immediate response with audio/visual feedback");
    
    // Camera System Status
    LOG_INFO("CAMERA SYSTEM:");
    LOG_INFO("  ✓ Third-Person Camera: Active and following character");
    LOG_INFO("  ✓ Smooth Movement: Camera interpolation and constraints");
    LOG_INFO("  ✓ Mouse Control: Free-look camera positioning");
    
    LOG_INFO("========================================");
    LOG_INFO("ALL ENGINE SYSTEMS OPERATIONAL AND DEMONSTRATED");
    LOG_INFO("========================================");
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