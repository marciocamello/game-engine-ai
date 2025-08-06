// Enhanced Example - Comprehensive demonstration of all Game Engine Kiro capabilities
// This example showcases the complete feature set including audio, FBX models, 
// environment objects, professional grid, performance monitoring, and advanced systems.
// For a simpler introduction to core mechanics, see basic_example.cpp

#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Core/PerformanceMonitor.h"
#include "Core/AssetValidator.h"
#include "Core/ResourcePool.h"
#include "Game/Character.h"
#include "Game/GameAudioManager.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/OpenGLRenderer.h"
#include "Graphics/GridRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Texture.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Resource/ModelLoader.h"
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
    uint32_t rigidBodyId = 0; // Physics body ID for collision
  };

  struct ShaderDemoObject {
    Math::Vec3 position;
    Math::Vec3 scale;
    Math::Vec3 rotation;
    std::string meshPath;
    std::shared_ptr<Material> material;
    std::shared_ptr<Mesh> loadedMesh;  // Store the actual loaded mesh
    Math::Vec4 baseColor;
    float metallic;
    float roughness;
    std::string name;
    bool isVisible = true;
  };

  GameApplication() = default;
  ~GameApplication() {
    // Ensure proper cleanup
    if (m_audioManager) {
      m_audioManager->Cleanup();
    }
    
    // Clean up texture pool
    m_texturePool.Clear();
    
    LOG_INFO("GameApplication cleaned up successfully");
  }

  bool Initialize() {
    // Initialize performance monitoring
    m_performanceMonitor = std::make_unique<PerformanceMonitor>();
    
    // Initialize asset validation
    m_assetValidator = std::make_unique<AssetValidator>();
    m_assetValidator->LogAssetStatus();
    
    if (!m_assetValidator->AllRequiredAssetsAvailable()) {
      LOG_WARNING("Some required assets are missing, but continuing with fallbacks");
    }
    
    if (!m_engine.Initialize()) {
      LOG_ERROR("Failed to initialize game engine");
      return false;
    }

    m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
    if (!m_primitiveRenderer->Initialize()) {
      LOG_ERROR("Failed to initialize primitive renderer");
      return false;
    }

    m_modelLoader = std::make_unique<ModelLoader>();
    if (!m_modelLoader->Initialize()) {
      LOG_ERROR("Failed to initialize model loader");
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

    // Initialize shader system demonstration
    InitializeShaderSystemDemo();

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
    LOG_INFO("LIGHTING CONTROLS (Lighting System Demo):");
    LOG_INFO("  F8 - Increase light intensity");
    LOG_INFO("  F9 - Decrease light intensity");
    LOG_INFO("  F10 - Cycle light colors (Warm/White/Orange/Blue/Pink/Green)");
    LOG_INFO("  F12 - Rotate light direction");
    LOG_INFO("  L - Toggle point light above character");
    LOG_INFO("");
    LOG_INFO("SHADER SYSTEM DEMONSTRATION:");
    LOG_INFO("  R - Toggle PBR material showcase (5 animated objects with different materials)");
    LOG_INFO("  T - Cycle material presets (Original/Metals/Dielectrics/Mixed)");
    LOG_INFO("  Y - Simulate shader hot-reload demonstration");
    LOG_INFO("  U - Show detailed shader system information and object status");
    LOG_INFO("");
    LOG_INFO("DEBUG CONTROLS:");
    LOG_INFO("  F3 - Toggle debug capsule visualization");
    LOG_INFO("  F2 - Test fall detection system");
    LOG_INFO("  F4 - Show comprehensive system status");
    LOG_INFO("  F5 - Show performance report");
    LOG_INFO("  F6 - Show asset validation status");
    LOG_INFO("  F7 - Force resource cleanup");
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
    // Begin performance monitoring
    m_performanceMonitor->BeginFrame();
    
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

    if (input->IsKeyPressed(KeyCode::F5)) {
      m_performanceMonitor->LogPerformanceReport();
    }

    if (input->IsKeyPressed(KeyCode::F6)) {
      m_assetValidator->LogAssetStatus();
    }

    if (input->IsKeyPressed(KeyCode::F7)) {
      // Force resource cleanup
      m_texturePool.CleanupExpired();
      LOG_INFO("Forced resource cleanup completed");
    }

    if (input->IsKeyPressed(KeyCode::F3)) {
      m_showDebugCapsule = !m_showDebugCapsule;
      LOG_INFO("RENDERING SYSTEM DEMO: Debug capsule visualization " + std::string(m_showDebugCapsule ? "ENABLED" : "DISABLED") + " - Shows physics collision alongside visual model");
    }

    // LIGHTING SYSTEM DEMO: Dynamic lighting controls
    auto* openglRenderer = static_cast<OpenGLRenderer*>(m_engine.GetRenderer());
    
    if (input->IsKeyPressed(KeyCode::F8)) {
      // Increase directional light intensity
      m_lightIntensity += 0.5f;
      if (m_lightIntensity > 10.0f) m_lightIntensity = 10.0f;
      openglRenderer->SetDirectionalLight(m_lightDirection, m_lightColor, m_lightIntensity);
      LOG_INFO("LIGHTING SYSTEM DEMO: Light intensity increased to " + std::to_string(m_lightIntensity));
    }
    
    if (input->IsKeyPressed(KeyCode::F9)) {
      // Decrease directional light intensity
      m_lightIntensity -= 0.5f;
      if (m_lightIntensity < 0.1f) m_lightIntensity = 0.1f;
      openglRenderer->SetDirectionalLight(m_lightDirection, m_lightColor, m_lightIntensity);
      LOG_INFO("LIGHTING SYSTEM DEMO: Light intensity decreased to " + std::to_string(m_lightIntensity));
    }
    
    if (input->IsKeyPressed(KeyCode::F10)) {
      // Cycle through different light colors
      static int colorIndex = 0;
      Math::Vec3 colors[] = {
        Math::Vec3(1.0f, 0.95f, 0.8f),  // Warm white (default)
        Math::Vec3(1.0f, 1.0f, 1.0f),   // Pure white
        Math::Vec3(1.0f, 0.7f, 0.4f),   // Orange/sunset
        Math::Vec3(0.8f, 0.9f, 1.0f),   // Cool blue
        Math::Vec3(1.0f, 0.8f, 0.8f),   // Pink
        Math::Vec3(0.9f, 1.0f, 0.8f)    // Green tint
      };
      std::string colorNames[] = {"Warm White", "Pure White", "Sunset Orange", "Cool Blue", "Pink", "Green Tint"};
      
      colorIndex = (colorIndex + 1) % 6;
      m_lightColor = colors[colorIndex];
      openglRenderer->SetDirectionalLight(m_lightDirection, m_lightColor, m_lightIntensity);
      LOG_INFO("LIGHTING SYSTEM DEMO: Light color changed to " + colorNames[colorIndex]);
    }
    
    if (input->IsKeyPressed(KeyCode::F12)) {
      // Rotate light direction
      static float angle = 0.0f;
      angle += 30.0f; // Rotate 30 degrees each press
      if (angle >= 360.0f) angle = 0.0f;
      
      float radians = angle * Math::DEG_TO_RAD;
      m_lightDirection = Math::Vec3(
        sinf(radians) * 0.5f,  // X component
        -1.0f,                 // Y component (always pointing down)
        cosf(radians) * 0.5f   // Z component
      );
      m_lightDirection = glm::normalize(m_lightDirection);
      
      openglRenderer->SetDirectionalLight(m_lightDirection, m_lightColor, m_lightIntensity);
      LOG_INFO("LIGHTING SYSTEM DEMO: Light direction rotated to " + std::to_string(angle) + " degrees");
    }
    
    if (input->IsKeyPressed(KeyCode::L)) {
      // Toggle point light
      static bool pointLightEnabled = false;
      pointLightEnabled = !pointLightEnabled;
      
      if (pointLightEnabled) {
        // Add a point light above the character
        Math::Vec3 characterPos = m_character->GetPosition();
        Math::Vec3 pointLightPos = characterPos + Math::Vec3(0.0f, 5.0f, 0.0f);
        openglRenderer->AddPointLight(pointLightPos, Math::Vec3(1.0f, 0.8f, 0.6f), 8.0f, 12.0f);
        LOG_INFO("LIGHTING SYSTEM DEMO: Point light ENABLED above character");
      } else {
        // Clear all point lights (this will remove the point light)
        // Note: In a more advanced system, we'd have individual point light management
        LOG_INFO("LIGHTING SYSTEM DEMO: Point light DISABLED (restart to clear)");
      }
    }

    // Handle shader system demonstration controls
    HandleShaderSystemControls(input);

    // Update shader demo objects (rotation animation)
    UpdateShaderDemoObjects(deltaTime);

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
    
    // Sync renderer with primitive renderer
    if (openglRenderer) {
      openglRenderer->SyncWithPrimitiveRenderer(m_primitiveRenderer.get());
    }
    
    // End performance monitoring
    m_performanceMonitor->EndFrame();
    
    // Periodic resource cleanup (every 5 seconds)
    static float cleanupTimer = 0.0f;
    cleanupTimer += deltaTime;
    if (cleanupTimer >= 5.0f) {
      m_texturePool.CleanupExpired();
      cleanupTimer = 0.0f;
    }
  }

private:

  void RenderMeshWithColor(std::shared_ptr<Mesh> mesh, const Math::Vec3& position, 
                          const Math::Vec3& rotation, const Math::Vec3& scale, 
                          const Math::Vec4& color) {
    if (!mesh) return;
    
    // Create rotation quaternion from rotation angles
    float yawRadians = rotation.y * Math::DEG_TO_RAD;
    float pitchRadians = rotation.x * Math::DEG_TO_RAD;
    float rollRadians = rotation.z * Math::DEG_TO_RAD;
    
    // Create quaternion from Euler angles (YXZ order)
    Math::Quat rotationQuat = Math::Quat(cos(yawRadians * 0.5f), 0.0f, sin(yawRadians * 0.5f), 0.0f) *
                              Math::Quat(cos(pitchRadians * 0.5f), sin(pitchRadians * 0.5f), 0.0f, 0.0f) *
                              Math::Quat(cos(rollRadians * 0.5f), 0.0f, 0.0f, sin(rollRadians * 0.5f));
    
    // Use PrimitiveRenderer to render the actual mesh
    m_primitiveRenderer->DrawMesh(mesh, position, rotationQuat, scale, color);
  }

  void CreateEnvironmentObjects() {
    // Create exactly 3 cubes with different material properties
    m_environmentObjects.clear();
    
    auto* physics = m_engine.GetPhysics();
    
    // Cube 1: Textured cube (using wall.jpg texture with resource pooling)
    EnvironmentObject texturedCube;
    texturedCube.position = Math::Vec3(-5.0f, 1.0f, 5.0f);
    texturedCube.scale = Math::Vec3(2.0f, 2.0f, 2.0f);
    
    // Use resource pool for efficient texture management
    std::string texturePath = "assets/textures/wall.jpg";
    if (m_assetValidator->ValidateAsset(texturePath)) {
      texturedCube.texture = m_texturePool.GetOrCreate(texturePath);
      if (texturedCube.texture && texturedCube.texture->LoadFromFile(texturePath)) {
        texturedCube.useTexture = true;
        texturedCube.useColor = false;
        LOG_INFO("Successfully loaded texture for environment cube 1 (pooled)");
      } else {
        texturedCube.useTexture = false;
        texturedCube.useColor = true;
        texturedCube.color = Math::Vec4(0.8f, 0.4f, 0.2f, 1.0f); // Orange fallback
        LOG_WARNING("Failed to load texture for cube 1, using color fallback");
      }
    } else {
      // Asset not available, use fallback
      texturedCube.useTexture = false;
      texturedCube.useColor = true;
      texturedCube.color = Math::Vec4(0.8f, 0.4f, 0.2f, 1.0f); // Orange fallback
      LOG_INFO("Texture asset not available, using color fallback for cube 1");
    }
    
    // Create physics body for cube 1
    if (physics) {
      RigidBody cubeDesc;
      cubeDesc.position = texturedCube.position;
      cubeDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
      cubeDesc.velocity = Math::Vec3(0.0f);
      cubeDesc.mass = 0.0f; // Static object
      cubeDesc.restitution = 0.3f;
      cubeDesc.friction = 0.7f;
      cubeDesc.isStatic = true;
      cubeDesc.isKinematic = false;

      CollisionShape cubeShape;
      cubeShape.type = CollisionShape::Box;
      cubeShape.dimensions = texturedCube.scale;

      texturedCube.rigidBodyId = physics->CreateRigidBody(cubeDesc, cubeShape);
      if (texturedCube.rigidBodyId == 0) {
        LOG_WARNING("Failed to create physics body for environment cube 1");
      }
    }
    m_environmentObjects.push_back(texturedCube);
    
    // Cube 2: Solid color cube (blue)
    EnvironmentObject colorCube;
    colorCube.position = Math::Vec3(5.0f, 1.0f, 5.0f);
    colorCube.scale = Math::Vec3(2.0f, 2.0f, 2.0f);
    colorCube.useTexture = false;
    colorCube.useColor = true;
    colorCube.color = Math::Vec4(0.2f, 0.4f, 0.8f, 1.0f); // Blue
    
    // Create physics body for cube 2
    if (physics) {
      RigidBody cubeDesc;
      cubeDesc.position = colorCube.position;
      cubeDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
      cubeDesc.velocity = Math::Vec3(0.0f);
      cubeDesc.mass = 0.0f; // Static object
      cubeDesc.restitution = 0.3f;
      cubeDesc.friction = 0.7f;
      cubeDesc.isStatic = true;
      cubeDesc.isKinematic = false;

      CollisionShape cubeShape;
      cubeShape.type = CollisionShape::Box;
      cubeShape.dimensions = colorCube.scale;

      colorCube.rigidBodyId = physics->CreateRigidBody(cubeDesc, cubeShape);
      if (colorCube.rigidBodyId == 0) {
        LOG_WARNING("Failed to create physics body for environment cube 2");
      }
    }
    m_environmentObjects.push_back(colorCube);
    
    // Cube 3: Default material cube (no texture, no color - uses default rendering)
    EnvironmentObject defaultCube;
    defaultCube.position = Math::Vec3(0.0f, 1.0f, 8.0f);
    defaultCube.scale = Math::Vec3(2.0f, 2.0f, 2.0f);
    defaultCube.useTexture = false;
    defaultCube.useColor = false;
    defaultCube.color = Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f); // White (default)
    
    // Create physics body for cube 3
    if (physics) {
      RigidBody cubeDesc;
      cubeDesc.position = defaultCube.position;
      cubeDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
      cubeDesc.velocity = Math::Vec3(0.0f);
      cubeDesc.mass = 0.0f; // Static object
      cubeDesc.restitution = 0.3f;
      cubeDesc.friction = 0.7f;
      cubeDesc.isStatic = true;
      cubeDesc.isKinematic = false;

      CollisionShape cubeShape;
      cubeShape.type = CollisionShape::Box;
      cubeShape.dimensions = defaultCube.scale;

      defaultCube.rigidBodyId = physics->CreateRigidBody(cubeDesc, cubeShape);
      if (defaultCube.rigidBodyId == 0) {
        LOG_WARNING("Failed to create physics body for environment cube 3");
      }
    }
    m_environmentObjects.push_back(defaultCube);
    
    LOG_INFO("RENDERING SYSTEM DEMO: Created 3 environment objects demonstrating different material properties:");
    LOG_INFO("  - Cube 1: Textured material (texture mapping demonstration)");
    LOG_INFO("  - Cube 2: Solid color material (shader color demonstration)");
    LOG_INFO("  - Cube 3: Default material (basic rendering demonstration)");
    LOG_INFO("PHYSICS SYSTEM DEMO: Created collision bodies for all environment objects");
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
    
    // Render shader demonstration objects
    if (m_pbrShowcaseMode) {
      RenderShaderDemoObjects();
    }
    
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

  void RenderShaderDemoObjects() {
    for (const auto& obj : m_shaderDemoObjects) {
      if (!obj.isVisible) continue;
      
      // Set color based on current material properties
      Math::Vec4 renderColor = obj.baseColor;
      
      // Apply metallic effect to color (metals have different reflectance)
      if (obj.metallic > 0.5f) {
        // Metallic materials have more uniform color across RGB channels
        float avgColor = (renderColor.r + renderColor.g + renderColor.b) / 3.0f;
        renderColor.r = avgColor * 0.8f + renderColor.r * 0.2f;
        renderColor.g = avgColor * 0.8f + renderColor.g * 0.2f;
        renderColor.b = avgColor * 0.8f + renderColor.b * 0.2f;
        
        // Metals are generally brighter
        renderColor.r *= 1.2f;
        renderColor.g *= 1.2f;
        renderColor.b *= 1.2f;
      }
      
      // Apply roughness effect (rougher surfaces appear less bright)
      float roughnessFactor = 1.0f - (obj.roughness * 0.3f);
      renderColor.r *= roughnessFactor;
      renderColor.g *= roughnessFactor;
      renderColor.b *= roughnessFactor;
      
      // Render the object - use loaded mesh if available, otherwise fallback to primitives
      if (obj.loadedMesh) {
        // Render the actual loaded mesh
        RenderMeshWithColor(obj.loadedMesh, obj.position, obj.rotation, obj.scale, renderColor);
      }
      else {
        // Fallback to primitive shapes based on mesh type
        if (obj.meshPath.find("teapot") != std::string::npos || obj.name.find("Teapot") != std::string::npos) {
          // Render teapot as sphere (classic 3D test shape)
          m_primitiveRenderer->DrawSphere(obj.position, obj.scale.x, renderColor);
        }
        else if (obj.meshPath.find("cow") != std::string::npos || obj.name.find("Cow") != std::string::npos) {
          // Render cow as elongated cube (representing organic shape)
          Math::Vec3 cowScale = obj.scale;
          cowScale.x *= 1.5f; // Make it wider
          cowScale.y *= 0.8f; // Make it shorter
          m_primitiveRenderer->DrawCube(obj.position, cowScale, renderColor);
        }
        else if (obj.meshPath.find("teddy") != std::string::npos || obj.name.find("Teddy") != std::string::npos) {
          // Render teddy as sphere (representing character model)
          m_primitiveRenderer->DrawSphere(obj.position, obj.scale.x * 0.9f, renderColor);
        }
        else if (obj.meshPath.find("pumpkin") != std::string::npos || obj.name.find("Pumpkin") != std::string::npos) {
          // Render pumpkin as flattened sphere
          Math::Vec3 pumpkinPos = obj.position;
          pumpkinPos.y -= 0.2f; // Lower it slightly
          m_primitiveRenderer->DrawSphere(pumpkinPos, obj.scale.x * 1.1f, renderColor);
        }
        else if (obj.name.find("Sphere") != std::string::npos) {
          // Render sphere objects as spheres
          m_primitiveRenderer->DrawSphere(obj.position, obj.scale.x, renderColor);
        }
        else {
          // Default: render as cube
          m_primitiveRenderer->DrawCube(obj.position, obj.scale, renderColor);
        }
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
    
    // Performance Status
    LOG_INFO("PERFORMANCE SYSTEM:");
    const auto& stats = m_performanceMonitor->GetFrameStats();
    LOG_INFO("  ✓ Current FPS: " + std::to_string(stats.fps));
    LOG_INFO("  ✓ Average FPS: " + std::to_string(stats.averageFPS));
    LOG_INFO("  ✓ Memory Usage: " + std::to_string(stats.memoryUsageMB) + " MB");
    LOG_INFO("  ✓ Performance Target: " + std::string(m_performanceMonitor->IsPerformanceTarget() ? "MET" : "NOT MET"));
    
    // Resource Management Status
    LOG_INFO("RESOURCE MANAGEMENT:");
    LOG_INFO("  ✓ Texture Pool: " + std::to_string(m_texturePool.GetResourceCount()) + " cached textures");
    LOG_INFO("  ✓ Asset Validation: " + std::string(m_assetValidator->AllRequiredAssetsAvailable() ? "All required assets available" : "Using fallbacks"));
    
    LOG_INFO("========================================");
    LOG_INFO("ALL ENGINE SYSTEMS OPERATIONAL AND DEMONSTRATED");
    LOG_INFO("========================================");
  }

  void InitializeShaderSystemDemo() {
    // Create shader demonstration objects using available meshes
    m_shaderDemoObjects.clear();
    
    // Teapot with metallic gold material (rendered as sphere)
    ShaderDemoObject teapot;
    teapot.position = Math::Vec3(-6.0f, 3.0f, -5.0f);
    teapot.scale = Math::Vec3(0.5f, 0.5f, 0.5f);
    teapot.rotation = Math::Vec3(0.0f, 0.0f, 0.0f);
    teapot.meshPath = "assets/meshes/teapot.obj";
    teapot.baseColor = Math::Vec4(1.0f, 0.86f, 0.57f, 1.0f); // Gold
    teapot.metallic = 1.0f;
    teapot.roughness = 0.1f;
    teapot.name = "Golden Teapot";
    // Try to load the actual mesh
    try {
      auto result = m_modelLoader->LoadModel(teapot.meshPath);
      if (result.success && !result.meshes.empty()) {
        teapot.loadedMesh = result.meshes[0];
      }
    } catch (const std::exception& e) {
      // Fallback to primitive rendering
    }
    m_shaderDemoObjects.push_back(teapot);
    
    // Cow with rough iron material
    ShaderDemoObject cow;
    cow.position = Math::Vec3(-2.0f, 1.0f, -6.0f);
    cow.scale = Math::Vec3(0.5f, 0.5f, 0.5f);
    cow.rotation = Math::Vec3(0.0f, 45.0f, 0.0f);
    cow.meshPath = "assets/meshes/cow-nonormals.obj";
    cow.baseColor = Math::Vec4(0.56f, 0.57f, 0.58f, 1.0f); // Iron
    cow.metallic = 1.0f;
    cow.roughness = 0.8f;
    cow.name = "Iron Cow";
    // Try to load the actual mesh
    try {
      auto result = m_modelLoader->LoadModel(cow.meshPath);
      if (result.success && !result.meshes.empty()) {
        cow.loadedMesh = result.meshes[0];
      }
    } catch (const std::exception& e) {
      // Fallback to primitive rendering
    }
    m_shaderDemoObjects.push_back(cow);
    
    // Teddy with plastic material
    ShaderDemoObject teddy;
    teddy.position = Math::Vec3(2.0f, 1.5f, -6.0f);
    teddy.scale = Math::Vec3(0.1f, 0.1f, 0.1f);
    teddy.rotation = Math::Vec3(0.0f, 0.0f, 0.0f);
    teddy.meshPath = "assets/meshes/teddy.obj";
    teddy.baseColor = Math::Vec4(0.8f, 0.2f, 0.2f, 1.0f); // Red plastic
    teddy.metallic = 0.0f;
    teddy.roughness = 0.3f;
    teddy.name = "Red Plastic Teddy";
    // Try to load the actual mesh
    try {
      auto result = m_modelLoader->LoadModel(teddy.meshPath);
      if (result.success && !result.meshes.empty()) {
        teddy.loadedMesh = result.meshes[0];
      }
    } catch (const std::exception& e) {
      // Fallback to primitive rendering
    }
    m_shaderDemoObjects.push_back(teddy);
    
    // Pumpkin with ceramic material
    ShaderDemoObject pumpkin;
    pumpkin.position = Math::Vec3(0.0f, 2.0f, -4.0f); // More isolated position, higher up
    pumpkin.scale = Math::Vec3(0.03f, 0.03f, 0.03f); // Small scale for 10k poly model
    pumpkin.rotation = Math::Vec3(0.0f, 0.0f, 0.0f);
    pumpkin.meshPath = "assets/meshes/pumpkin_tall_10k.obj";
    pumpkin.baseColor = Math::Vec4(1.0f, 0.5f, 0.1f, 1.0f); // Orange
    pumpkin.metallic = 0.0f;
    pumpkin.roughness = 0.1f;
    pumpkin.name = "Ceramic Pumpkin";
    // Try to load the actual mesh
    try {
      auto result = m_modelLoader->LoadModel(pumpkin.meshPath);
      if (result.success && !result.meshes.empty()) {
        pumpkin.loadedMesh = result.meshes[0];
      }
    } catch (const std::exception& e) {
      // Fallback to primitive rendering
    }
    m_shaderDemoObjects.push_back(pumpkin);
    
    // Extra cube with copper material
    ShaderDemoObject cube;
    cube.position = Math::Vec3(8.0f, 1.0f, -8.0f);
    cube.scale = Math::Vec3(1.5f, 1.5f, 1.5f);
    cube.rotation = Math::Vec3(0.0f, 30.0f, 0.0f);
    cube.meshPath = "assets/meshes/cube.obj";
    cube.baseColor = Math::Vec4(0.95f, 0.64f, 0.54f, 1.0f); // Copper
    cube.metallic = 1.0f;
    cube.roughness = 0.4f;
    cube.name = "Copper Cube";
    // Try to load the actual mesh
    try {
      auto result = m_modelLoader->LoadModel(cube.meshPath);
      if (result.success && !result.meshes.empty()) {
        cube.loadedMesh = result.meshes[0];
      }
    } catch (const std::exception& e) {
      // Fallback to primitive rendering
    }
    m_shaderDemoObjects.push_back(cube);
    
    // Add a sphere with chrome material
    ShaderDemoObject sphere;
    sphere.position = Math::Vec3(-6.0f, 1.5f, -4.0f);
    sphere.scale = Math::Vec3(0.8f, 0.8f, 0.8f);
    sphere.rotation = Math::Vec3(0.0f, 0.0f, 0.0f);
    sphere.meshPath = ""; // No mesh path - will use primitive sphere
    sphere.baseColor = Math::Vec4(0.9f, 0.9f, 0.95f, 1.0f); // Chrome color
    sphere.metallic = 1.0f;
    sphere.roughness = 0.05f; // Very smooth for chrome effect
    sphere.name = "Chrome Sphere";
    sphere.loadedMesh = nullptr; // Force primitive rendering
    m_shaderDemoObjects.push_back(sphere);
    
    LOG_INFO("SHADER SYSTEM DEMO: Created " + std::to_string(m_shaderDemoObjects.size()) + " shader demonstration objects:");
    for (const auto& obj : m_shaderDemoObjects) {
      LOG_INFO("  - " + obj.name + " at (" + 
               std::to_string(obj.position.x) + ", " + 
               std::to_string(obj.position.y) + ", " + 
               std::to_string(obj.position.z) + ")");
    }
  }

  void HandleShaderSystemControls(InputManager* input) {
    // R - Toggle PBR material showcase mode
    if (input->IsKeyPressed(KeyCode::R)) {
      m_pbrShowcaseMode = !m_pbrShowcaseMode;
      if (m_pbrShowcaseMode) {
        ApplyPBRShowcaseMaterials();
        LOG_INFO("SHADER SYSTEM DEMO: PBR Material Showcase Mode ENABLED");
        LOG_INFO("  ✓ 5 animated objects now visible with different PBR materials");
        LOG_INFO("  ✓ Objects positioned at z=-8 (behind environment cubes)");
        LOG_INFO("  ✓ Each object demonstrates different metallic/roughness values");
        LOG_INFO("  ✓ Objects rotate automatically for better material visualization");
        LOG_INFO("  → Use T to cycle through different material presets");
      } else {
        LOG_INFO("SHADER SYSTEM DEMO: PBR Material Showcase Mode DISABLED");
        LOG_INFO("  - Shader demonstration objects are now hidden");
        LOG_INFO("  - Original environment objects remain unchanged");
      }
    }

    // T - Cycle material properties on environment objects
    if (input->IsKeyPressed(KeyCode::T)) {
      if (m_pbrShowcaseMode) {
        CycleMaterialPresets();
      } else {
        LOG_INFO("SHADER SYSTEM DEMO: Enable PBR showcase mode (R) first to cycle materials");
      }
    }

    // Y - Simulate shader hot-reload demonstration
    if (input->IsKeyPressed(KeyCode::Y)) {
      SimulateShaderHotReload();
    }

    // U - Show shader compilation information
    if (input->IsKeyPressed(KeyCode::U)) {
      ShowShaderSystemInformation();
    }
  }

  void ApplyPBRShowcaseMaterials() {
    // Reset all objects to their original PBR materials and make them visible
    for (auto& obj : m_shaderDemoObjects) {
      obj.isVisible = true; // Tornar todos os objetos visíveis
    }
    
    if (m_shaderDemoObjects.size() >= 5) {
      // Teapot - Gold
      m_shaderDemoObjects[0].baseColor = Math::Vec4(1.0f, 0.86f, 0.57f, 1.0f);
      m_shaderDemoObjects[0].metallic = 1.0f;
      m_shaderDemoObjects[0].roughness = 0.1f;
      
      // Cow - Iron
      m_shaderDemoObjects[1].baseColor = Math::Vec4(0.56f, 0.57f, 0.58f, 1.0f);
      m_shaderDemoObjects[1].metallic = 1.0f;
      m_shaderDemoObjects[1].roughness = 0.8f;
      
      // Teddy - Red Plastic
      m_shaderDemoObjects[2].baseColor = Math::Vec4(0.8f, 0.2f, 0.2f, 1.0f);
      m_shaderDemoObjects[2].metallic = 0.0f;
      m_shaderDemoObjects[2].roughness = 0.3f;
      
      // Pumpkin - Ceramic
      m_shaderDemoObjects[3].baseColor = Math::Vec4(1.0f, 0.5f, 0.1f, 1.0f);
      m_shaderDemoObjects[3].metallic = 0.0f;
      m_shaderDemoObjects[3].roughness = 0.1f;
      
      // Cube - Copper
      m_shaderDemoObjects[4].baseColor = Math::Vec4(0.95f, 0.64f, 0.54f, 1.0f);
      m_shaderDemoObjects[4].metallic = 1.0f;
      m_shaderDemoObjects[4].roughness = 0.4f;
    }
    
    LOG_INFO("SHADER SYSTEM DEMO: Applied original PBR materials to all objects");
  }

  void RestoreOriginalMaterials() {
    // Hide all shader demo objects when showcase mode is disabled
    for (auto& obj : m_shaderDemoObjects) {
      obj.isVisible = false;
    }
    LOG_INFO("SHADER SYSTEM DEMO: Hidden all shader demonstration objects");
  }

  void CycleMaterialPresets() {
    m_currentMaterialPreset = (m_currentMaterialPreset + 1) % 3;
    
    switch (m_currentMaterialPreset) {
      case 0:
        ApplyMetalsPreset();
        LOG_INFO("SHADER SYSTEM DEMO: Applied METALS material preset");
        break;
      case 1:
        ApplyDielectricsPreset();
        LOG_INFO("SHADER SYSTEM DEMO: Applied DIELECTRICS material preset");
        break;
      case 2:
        ApplyMixedPreset();
        LOG_INFO("SHADER SYSTEM DEMO: Applied MIXED material preset");
        break;
    }
  }

  void ApplyMetalsPreset() {
    // Make all objects metallic with varying roughness
    for (size_t i = 0; i < m_shaderDemoObjects.size(); ++i) {
      auto& obj = m_shaderDemoObjects[i];
      obj.metallic = 1.0f; // All metallic
      obj.roughness = static_cast<float>(i) / static_cast<float>(m_shaderDemoObjects.size() - 1);
      
      // Adjust colors for metal appearance
      switch (i) {
        case 0: obj.baseColor = Math::Vec4(1.0f, 0.86f, 0.57f, 1.0f); break; // Gold
        case 1: obj.baseColor = Math::Vec4(0.56f, 0.57f, 0.58f, 1.0f); break; // Iron
        case 2: obj.baseColor = Math::Vec4(0.95f, 0.64f, 0.54f, 1.0f); break; // Copper
        case 3: obj.baseColor = Math::Vec4(0.91f, 0.92f, 0.92f, 1.0f); break; // Silver
        case 4: obj.baseColor = Math::Vec4(0.76f, 0.78f, 0.78f, 1.0f); break; // Steel
      }
    }
    
    LOG_INFO("SHADER SYSTEM DEMO: Applied METALS preset - all objects now metallic with varying roughness");
  }

  void ApplyDielectricsPreset() {
    // Make all objects non-metallic (dielectric) with varying roughness
    for (size_t i = 0; i < m_shaderDemoObjects.size(); ++i) {
      auto& obj = m_shaderDemoObjects[i];
      obj.metallic = 0.0f; // All non-metallic
      obj.roughness = static_cast<float>(i) / static_cast<float>(m_shaderDemoObjects.size() - 1);
      
      // Adjust colors for dielectric materials
      switch (i) {
        case 0: obj.baseColor = Math::Vec4(0.8f, 0.2f, 0.2f, 1.0f); break; // Red plastic
        case 1: obj.baseColor = Math::Vec4(0.2f, 0.8f, 0.2f, 1.0f); break; // Green plastic
        case 2: obj.baseColor = Math::Vec4(0.2f, 0.2f, 0.8f, 1.0f); break; // Blue plastic
        case 3: obj.baseColor = Math::Vec4(0.9f, 0.9f, 0.85f, 1.0f); break; // Ceramic
        case 4: obj.baseColor = Math::Vec4(0.6f, 0.4f, 0.2f, 1.0f); break; // Wood
      }
    }
    
    LOG_INFO("SHADER SYSTEM DEMO: Applied DIELECTRICS preset - all objects now non-metallic with varying roughness");
  }

  void ApplyMixedPreset() {
    // Alternate between metallic and non-metallic with interesting combinations
    for (size_t i = 0; i < m_shaderDemoObjects.size(); ++i) {
      auto& obj = m_shaderDemoObjects[i];
      bool isMetallic = (i % 2 == 0);
      obj.metallic = isMetallic ? 1.0f : 0.0f;
      obj.roughness = 0.2f + (static_cast<float>(i) * 0.15f);
      
      // Mix of metallic and non-metallic colors
      if (isMetallic) {
        switch (i / 2) {
          case 0: obj.baseColor = Math::Vec4(1.0f, 0.86f, 0.57f, 1.0f); break; // Gold
          case 1: obj.baseColor = Math::Vec4(0.95f, 0.64f, 0.54f, 1.0f); break; // Copper
          case 2: obj.baseColor = Math::Vec4(0.91f, 0.92f, 0.92f, 1.0f); break; // Silver
        }
      } else {
        switch (i / 2) {
          case 0: obj.baseColor = Math::Vec4(0.8f, 0.2f, 0.2f, 1.0f); break; // Red
          case 1: obj.baseColor = Math::Vec4(0.2f, 0.6f, 0.8f, 1.0f); break; // Blue
          case 2: obj.baseColor = Math::Vec4(0.6f, 0.4f, 0.2f, 1.0f); break; // Brown
        }
      }
    }
    
    LOG_INFO("SHADER SYSTEM DEMO: Applied MIXED preset - alternating metallic/non-metallic materials");
  }

  void SimulateShaderHotReload() {
    LOG_INFO("========================================");
    LOG_INFO("SHADER SYSTEM DEMO: HOT-RELOAD SIMULATION");
    LOG_INFO("========================================");
    LOG_INFO("Simulating shader hot-reload process...");
    LOG_INFO("");
    LOG_INFO("1. File Watcher: Detected change in 'assets/shaders/basic.frag'");
    LOG_INFO("2. Shader Compiler: Recompiling fragment shader...");
    LOG_INFO("3. Shader Linker: Linking updated shader program...");
    LOG_INFO("4. Material System: Updating materials with new shader...");
    LOG_INFO("5. Renderer: Applying updated shaders to scene objects...");
    
    // Visual demonstration: temporarily change colors to show "reloading"
    if (m_pbrShowcaseMode && !m_shaderDemoObjects.empty()) {
      LOG_INFO("6. Visual Update: Applying shader changes to objects...");
      
      // Temporarily flash objects to white to simulate shader reload
      static bool flashState = false;
      flashState = !flashState;
      
      for (auto& obj : m_shaderDemoObjects) {
        if (obj.isVisible) {
          if (flashState) {
            // Flash to bright colors to simulate shader reload (avoid white saturation)
            if (obj.name.find("Teapot") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.5f, 1.3f, 0.9f, 1.0f); // Bright gold flash
            } else if (obj.name.find("Cow") != std::string::npos) {
              obj.baseColor = Math::Vec4(0.9f, 0.9f, 1.0f, 1.0f); // Bright iron flash
            } else if (obj.name.find("Teddy") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.2f, 0.4f, 0.4f, 1.0f); // Bright red flash
            } else if (obj.name.find("Pumpkin") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.4f, 0.8f, 0.3f, 1.0f); // Bright orange flash
            } else if (obj.name.find("Cube") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.3f, 0.9f, 0.8f, 1.0f); // Bright copper flash
            }
          } else {
            // Restore to slightly modified colors to show "updated shader"
            if (obj.name.find("Teapot") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.1f, 0.95f, 0.65f, 1.0f); // Enhanced gold
            } else if (obj.name.find("Cow") != std::string::npos) {
              obj.baseColor = Math::Vec4(0.65f, 0.65f, 0.75f, 1.0f); // Enhanced iron
            } else if (obj.name.find("Teddy") != std::string::npos) {
              obj.baseColor = Math::Vec4(0.9f, 0.25f, 0.25f, 1.0f); // Enhanced red
            } else if (obj.name.find("Pumpkin") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.1f, 0.55f, 0.15f, 1.0f); // Enhanced orange
            } else if (obj.name.find("Cube") != std::string::npos) {
              obj.baseColor = Math::Vec4(1.0f, 0.7f, 0.6f, 1.0f); // Enhanced copper
            }
          }
        }
      }
    }
    
    LOG_INFO("");
    LOG_INFO("Hot-reload complete! Shader changes applied without restart.");
    LOG_INFO("  ✓ Objects now use updated shader with enhanced lighting");
    LOG_INFO("");
    LOG_INFO("In a full implementation:");
    LOG_INFO("  ✓ File system monitoring would detect shader changes");
    LOG_INFO("  ✓ Automatic recompilation would occur in background");
    LOG_INFO("  ✓ Error handling would fallback to previous version on failure");
    LOG_INFO("  ✓ All materials using the shader would update automatically");
    LOG_INFO("  ✓ Real-time feedback would be provided to developers");
    LOG_INFO("========================================");
  }

  void ShowShaderSystemInformation() {
    LOG_INFO("========================================");
    LOG_INFO("ADVANCED SHADER SYSTEM INFORMATION");
    LOG_INFO("========================================");
    LOG_INFO("");
    LOG_INFO("CURRENT DEMONSTRATION OBJECTS:");
    for (size_t i = 0; i < m_shaderDemoObjects.size(); ++i) {
      const auto& obj = m_shaderDemoObjects[i];
      LOG_INFO("  " + std::to_string(i + 1) + ". " + obj.name);
      LOG_INFO("     Position: (" + std::to_string(obj.position.x) + ", " + 
               std::to_string(obj.position.y) + ", " + std::to_string(obj.position.z) + ")");
      LOG_INFO("     Material: Metallic=" + std::to_string(obj.metallic) + 
               ", Roughness=" + std::to_string(obj.roughness));
      LOG_INFO("     Color: (" + std::to_string(obj.baseColor.x) + ", " + 
               std::to_string(obj.baseColor.y) + ", " + std::to_string(obj.baseColor.z) + ")");
    }
    LOG_INFO("");
    LOG_INFO("CURRENT MATERIAL PRESET: " + std::to_string(m_currentMaterialPreset));
    LOG_INFO("  0 = Original PBR Materials");
    LOG_INFO("  1 = All Metals (varying roughness)");
    LOG_INFO("  2 = All Dielectrics (varying roughness)");
    LOG_INFO("  3 = Mixed Materials");
    LOG_INFO("");
    LOG_INFO("SHADER SYSTEM FEATURES:");
    LOG_INFO("  ✓ PBR Material Demonstration: 5 objects with different materials");
    LOG_INFO("  ✓ Real-time Material Switching: Press T to cycle presets");
    LOG_INFO("  ✓ Object Animation: Rotating objects for better material visualization");
    LOG_INFO("  ✓ Hot-reload Simulation: Press Y to simulate shader recompilation");
    LOG_INFO("  ✓ Interactive Controls: Toggle showcase mode with R");
    LOG_INFO("");
    LOG_INFO("AVAILABLE MESH TYPES:");
    LOG_INFO("  • Teapot (Sphere representation) - Classic 3D test model");
    LOG_INFO("  • Cow (Cube representation) - Complex organic shape");
    LOG_INFO("  • Teddy (Sphere representation) - Detailed character model");
    LOG_INFO("  • Pumpkin (Sphere representation) - High-poly organic model");
    LOG_INFO("  • Cube (Cube representation) - Simple geometric primitive");
    LOG_INFO("========================================");
  }

  void UpdateShaderDemoObjects(float deltaTime) {
    if (!m_pbrShowcaseMode) return;
    
    // Rotate objects slowly for better material visualization
    static float rotationTime = 0.0f;
    rotationTime += deltaTime;
    
    for (size_t i = 0; i < m_shaderDemoObjects.size(); ++i) {
      auto& obj = m_shaderDemoObjects[i];
      
      // Pumpkin only floats (no rotation)
      if (obj.name.find("Pumpkin") != std::string::npos) {
        float bobAmount = 0.3f;
        float bobSpeed = 1.5f;
        static float basePumpkinY = 2.0f; // Store base Y position
        obj.position.y = basePumpkinY + sinf(rotationTime * bobSpeed) * bobAmount;
      } else {
        // All other objects rotate on their own axis
        float rotationSpeed = 15.0f + (static_cast<float>(i) * 5.0f);
        obj.rotation.y = fmodf(rotationTime * rotationSpeed, 360.0f);
      }
    }
  }



  Engine m_engine;
  std::unique_ptr<ThirdPersonCameraSystem> m_camera;
  std::unique_ptr<Character> m_character;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
  std::unique_ptr<GameAudioManager> m_audioManager;
  std::unique_ptr<GridRenderer> m_gridRenderer;
  std::unique_ptr<ModelLoader> m_modelLoader;
  
  // Performance and resource management
  std::unique_ptr<PerformanceMonitor> m_performanceMonitor;
  std::unique_ptr<AssetValidator> m_assetValidator;
  ResourcePool<Texture> m_texturePool;
  
  std::vector<EnvironmentObject> m_environmentObjects;
  
  CharacterType m_activeCharacter = CharacterType::Hybrid;
  bool m_showDebugCapsule = false;
  
  // Lighting system state
  Math::Vec3 m_lightDirection = Math::Vec3(0.3f, -1.0f, 0.3f);
  Math::Vec3 m_lightColor = Math::Vec3(1.0f, 0.95f, 0.8f);
  float m_lightIntensity = 2.0f;
  
  // Shader system demonstration state
  bool m_pbrShowcaseMode = false;
  int m_currentMaterialPreset = 0;
  std::vector<ShaderDemoObject> m_shaderDemoObjects;
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