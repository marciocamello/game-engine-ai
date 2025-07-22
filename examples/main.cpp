#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Game/Character.h"
// CharacterController removed - using only Character with different movement components
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Physics/PhysicsDebugManager.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Audio/AudioEngine.h"
#include <GLFW/glfw3.h>


using namespace GameEngine;

class GameApplication {
public:
  enum class CharacterType {
    CharacterMovement,        // Character with CharacterMovementComponent (basic)
    Physics,                  // Character with PhysicsMovementComponent (realistic)
    Hybrid                    // Character with HybridMovementComponent (balanced) - DEFAULT
  };

  GameApplication() = default;
  ~GameApplication() {
    // Clean up audio source
    if (m_audioSourceId != 0) {
      auto* audioEngine = m_engine.GetAudio();
      if (audioEngine) {
        audioEngine->DestroyAudioSource(m_audioSourceId);
      }
    }
  }

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

    // The physics debug manager is now handled by the engine automatically
    // No need to manually initialize it here

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
    m_character->SetFallLimit(-5.0f);  // Fall detection at reasonable depth

    // Initialize character with Hybrid movement component as default (best for third-person games)
    m_character->SwitchToHybridMovement();
    LOG_INFO("Character initialized with HybridMovement (default for third-person games)");

    // Setup third-person camera system
    m_camera = std::make_unique<ThirdPersonCameraSystem>();
    m_camera->SetTarget(m_character.get());
    m_camera->SetArmLength(10.0f);
    m_camera->SetRotationLimits(-45.0f, 30.0f);
    m_camera->SetSensitivity(0.8f, 0.6f);
    m_camera->SetMouseSensitivity(0.15f);

    // Set camera in renderer and debug manager
    m_engine.GetRenderer()->SetCamera(m_camera.get());
    m_engine.SetMainCamera(m_camera.get());

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

    // Test audio loading functionality
    TestAudioLoading();

    LOG_INFO("Game application initialized successfully");
    LOG_INFO("Controls:");
    LOG_INFO("  WASD - Move character");
    LOG_INFO("  Space - Jump");
    LOG_INFO("  1 - DeterministicMovement (basic movement with manual physics)");
    LOG_INFO("  2 - PhysicsMovement (full physics simulation)");
    LOG_INFO("  3 - HybridMovement (physics collision + direct control) - DEFAULT");
    LOG_INFO("  J - Toggle physics debug visualization");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("  F1 - Exit");
    LOG_INFO("  F2 - Test fall detection (teleport character high up)");
    LOG_INFO("  F3 - Play WAV audio");
    LOG_INFO("  F4 - Play OGG audio");
    LOG_INFO("  F5 - Stop all audio");
    LOG_INFO("Fall Detection System:");
    LOG_INFO("  - Characters automatically reset when falling below Y = -5.0");
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

    // Character switching - simplified to 3 components only
    // Default is Hybrid (balanced approach for third-person games)
    if (input->IsKeyPressed(KeyCode::Num1)) {
      m_activeCharacter = CharacterType::CharacterMovement;
      m_character->SwitchToCharacterMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to CharacterMovement (basic movement with manual physics)");
    }
    if (input->IsKeyPressed(KeyCode::Num2)) {
      m_activeCharacter = CharacterType::Physics;
      m_character->SwitchToPhysicsMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to PhysicsMovement (full physics simulation)");
    }
    if (input->IsKeyPressed(KeyCode::Num3)) {
      m_activeCharacter = CharacterType::Hybrid;
      m_character->SwitchToHybridMovement();
      m_camera->SetTarget(m_character.get());
      LOG_INFO("Switched to HybridMovement (physics collision + direct control) - RECOMMENDED");
    }

    // Physics debug visualization is now handled automatically by the debug manager
    // The 'D' key toggle is handled in the engine's physics debug manager

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
      m_character->SetPosition(testFallPosition);
      LOG_INFO("Testing fall detection - Character teleported to high position");
    }

    // F3 to play WAV audio
    if (input->IsKeyPressed(KeyCode::F3)) {
      TestAudioPlayback(true); // Play WAV
    }

    // F4 to play OGG audio  
    if (input->IsKeyPressed(KeyCode::F4)) {
      TestAudioPlayback(false); // Play OGG
    }

    // F5 to stop all audio
    if (input->IsKeyPressed(KeyCode::F5)) {
      StopAllAudio();
    }

    // Update active character - simplified to use only Character with different movement components
    m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
    
    // Check for fall and reset if needed
    if (m_character->HasFallen()) {
      LOG_INFO("Character has fallen! Resetting to spawn position...");
      m_character->ResetToSpawnPosition();
    }
    
    m_camera->Update(deltaTime, m_engine.GetInput());
  }

private:
  void TestAudioLoading() {
    LOG_INFO("Testing audio loading functionality...");
    
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine) {
      LOG_WARNING("Audio engine not available, skipping audio tests");
      return;
    }
    
    // Test loading WAV file
    LOG_INFO("Testing WAV file loading...");
    auto wavClip = audioEngine->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
    if (wavClip) {
      LOG_INFO("SUCCESS: WAV file loaded - Duration: " + std::to_string(wavClip->duration) + "s, " +
               std::to_string(wavClip->channels) + " channels, " + std::to_string(wavClip->sampleRate) + "Hz");
    } else {
      LOG_ERROR("FAILED: Could not load WAV file");
    }
    
    // Test loading OGG file
    LOG_INFO("Testing OGG file loading...");
    auto oggClip = audioEngine->LoadAudioClip("assets/audio/file_example_OOG_1MG.ogg");
    if (oggClip) {
      LOG_INFO("SUCCESS: OGG file loaded - Duration: " + std::to_string(oggClip->duration) + "s, " +
               std::to_string(oggClip->channels) + " channels, " + std::to_string(oggClip->sampleRate) + "Hz");
    } else {
      LOG_ERROR("FAILED: Could not load OGG file");
    }
    
    // Test unified loading interface
    LOG_INFO("Testing unified audio loading interface...");
    auto unifiedClip = audioEngine->LoadAudioClip("assets/audio/file_example_OOG_1MG.ogg");
    if (unifiedClip) {
      LOG_INFO("SUCCESS: Unified interface loaded OGG file successfully");
    } else {
      LOG_ERROR("FAILED: Unified interface could not load OGG file");
    }
    
    // Store clips for playback testing
    m_wavClip = wavClip;
    m_oggClip = oggClip;
    
    LOG_INFO("Audio loading tests completed");
  }

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

    // Draw character - simplified to always use Character
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

  void TestAudioPlayback(bool playWav) {
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine) {
      LOG_WARNING("Audio engine not available");
      return;
    }

    // Create audio source if not exists
    if (m_audioSourceId == 0) {
      m_audioSourceId = audioEngine->CreateAudioSource();
      LOG_INFO("Created audio source with ID: " + std::to_string(m_audioSourceId));
    }

    // Stop current playback
    audioEngine->StopAudioSource(m_audioSourceId);

    // Play selected audio clip
    if (playWav && m_wavClip) {
      LOG_INFO("Playing WAV audio: " + m_wavClip->path);
      audioEngine->PlayAudioSource(m_audioSourceId, m_wavClip);
      audioEngine->SetAudioSourceVolume(m_audioSourceId, 0.5f); // 50% volume
    } else if (!playWav && m_oggClip) {
      LOG_INFO("Playing OGG audio: " + m_oggClip->path);
      audioEngine->PlayAudioSource(m_audioSourceId, m_oggClip);
      audioEngine->SetAudioSourceVolume(m_audioSourceId, 0.5f); // 50% volume
    } else {
      LOG_WARNING("Audio clip not available for playback");
    }
  }

  void StopAllAudio() {
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine || m_audioSourceId == 0) {
      LOG_WARNING("No audio source to stop");
      return;
    }

    audioEngine->StopAudioSource(m_audioSourceId);
    LOG_INFO("Stopped all audio playback");
  }

  // UpdateCameraForCharacterController removed - using only Character now

  Engine m_engine;
  std::unique_ptr<ThirdPersonCameraSystem> m_camera;
  std::unique_ptr<Character> m_character;
  // CharacterController removed - using only Character with different movement components
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
  
  CharacterType m_activeCharacter = CharacterType::Hybrid; // Start with Character + HybridMovement (default)
  bool m_debugPhysicsEnabled = false;
  
  // Audio clips for testing
  std::shared_ptr<AudioClip> m_wavClip;
  std::shared_ptr<AudioClip> m_oggClip;
  uint32_t m_audioSourceId = 0;
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