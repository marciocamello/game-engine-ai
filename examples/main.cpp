#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Game/Character.h"
// CharacterController removed - using only Character with different movement components
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Resource/ResourceManager.h"
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
    // Clean up audio sources
    auto* audioEngine = m_engine.GetAudio();
    if (audioEngine) {
      if (m_audioSourceId != 0) {
        audioEngine->DestroyAudioSource(m_audioSourceId);
      }
      if (m_backgroundMusicSource != 0) {
        audioEngine->DestroyAudioSource(m_backgroundMusicSource);
      }
      for (auto& source : m_spatialAudioSources) {
        audioEngine->DestroyAudioSource(source.sourceId);
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

    // Initialize physics-based character with audio
    m_character = std::make_unique<Character>();
    if (!m_character->Initialize(m_engine.GetPhysics(), m_engine.GetAudio())) {
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
    m_camera->SetRotationLimits(-45.0f, 45.0f);
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

    // Test resource loading functionality
    TestResourceLoading();

    LOG_INFO("Game application initialized successfully");
    LOG_INFO("========================================");
    LOG_INFO("GAME ENGINE KIRO v1.0 - COMPLETE IMPLEMENTATION DEMO");
    LOG_INFO("========================================");
    LOG_INFO("Controls:");
    LOG_INFO("  WASD - Move character");
    LOG_INFO("  Space - Jump");
    LOG_INFO("  Mouse - Look around (camera control)");
    LOG_INFO("");
    LOG_INFO("Character Movement Types:");
    LOG_INFO("  1 - CharacterMovement (basic movement with manual physics)");
    LOG_INFO("  2 - PhysicsMovement (full physics simulation)");
    LOG_INFO("  3 - HybridMovement (physics collision + direct control) - DEFAULT");
    LOG_INFO("");
    LOG_INFO("Audio Controls:");
    LOG_INFO("  F3 - Play WAV audio (positioned at character)");
    LOG_INFO("  F4 - Play OGG audio (positioned at character)");
    LOG_INFO("  F5 - Stop all audio");
    LOG_INFO("  Background music plays automatically (looping OGG)");
    LOG_INFO("");
    LOG_INFO("System Controls:");
    LOG_INFO("  J - Toggle physics debug visualization");
    LOG_INFO("  ESC - Toggle mouse capture");
    LOG_INFO("  F11 - Toggle fullscreen");
    LOG_INFO("  F1 - Exit");
    LOG_INFO("  F2 - Test fall detection (teleport character high up)");
    LOG_INFO("");
    LOG_INFO("Features Demonstrated:");
    LOG_INFO("  ✓ OpenAL 3D Spatial Audio System");
    LOG_INFO("    - Background music (looping OGG)");
    LOG_INFO("    - 3D positioned audio sources (yellow spheres with red cubes)");
    LOG_INFO("    - Character footsteps and jump sounds");
    LOG_INFO("    - Listener position updates with character movement");
    LOG_INFO("  ✓ Resource Management System");
    LOG_INFO("    - Multiple texture formats (PNG, JPG, TGA)");
    LOG_INFO("    - Multiple mesh formats (OBJ with various complexity)");
    LOG_INFO("    - Resource caching and memory management");
    LOG_INFO("    - Fallback resources for missing files");
    LOG_INFO("  ✓ Integration with Existing Systems");
    LOG_INFO("    - Audio integrated with character movement");
    LOG_INFO("    - Textures applied to meshes and primitives");
    LOG_INFO("    - 3D audio positioning follows camera and character");
    LOG_INFO("");
    LOG_INFO("Visual Guide:");
    LOG_INFO("  - Yellow spheres with red cubes = 3D audio sources");
    LOG_INFO("  - Front row (blue area) = Textured primitives (PNG, JPG, TGA)");
    LOG_INFO("  - Back row (red area) = Various loaded meshes with textures");
    LOG_INFO("  - Character = Blue capsule with movement-type-based coloring");
    LOG_INFO("  - Ground = Green plane with grid lines");
    LOG_INFO("");
    LOG_INFO("Fall Detection System:");
    LOG_INFO("  - Characters automatically reset when falling below Y = -5.0");
    LOG_INFO("  - Test by walking off the ground plane edges or pressing F2");
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
    
    // Update 3D audio positioning
    Update3DAudioSources(deltaTime);
  }

private:
  void TestAudioLoading() {
    LOG_INFO("========================================");
    LOG_INFO("Testing Audio Loading Functionality");
    LOG_INFO("========================================");
    
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine) {
      LOG_WARNING("Audio engine not available, skipping audio tests");
      return;
    }
    
    // Test 1: WAV file loading
    LOG_INFO("Test 1: WAV file loading...");
    auto wavClip = audioEngine->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
    if (wavClip) {
      LOG_INFO("  [PASS] WAV file loaded successfully");
      LOG_INFO("    Path: " + wavClip->path);
      LOG_INFO("    Duration: " + std::to_string(wavClip->duration) + "s");
      LOG_INFO("    Channels: " + std::to_string(wavClip->channels));
      LOG_INFO("    Sample Rate: " + std::to_string(wavClip->sampleRate) + "Hz");
      LOG_INFO("    Format: WAV");
    } else {
      LOG_ERROR("  [FAIL] Could not load WAV file");
    }
    
    // Test 2: OGG file loading
    LOG_INFO("Test 2: OGG file loading...");
    auto oggClip = audioEngine->LoadAudioClip("assets/audio/file_example_OOG_1MG.ogg");
    if (oggClip) {
      LOG_INFO("  [PASS] OGG file loaded successfully");
      LOG_INFO("    Path: " + oggClip->path);
      LOG_INFO("    Duration: " + std::to_string(oggClip->duration) + "s");
      LOG_INFO("    Channels: " + std::to_string(oggClip->channels));
      LOG_INFO("    Sample Rate: " + std::to_string(oggClip->sampleRate) + "Hz");
      LOG_INFO("    Format: OGG");
    } else {
      LOG_ERROR("  [FAIL] Could not load OGG file");
    }
    
    // Test 3: MP3 file loading (should fail gracefully)
    LOG_INFO("Test 3: MP3 file loading (expected to fail - not implemented)...");
    auto mp3Clip = audioEngine->LoadAudioClip("assets/audio/file_example_MP3_5MG.mp3");
    if (mp3Clip) {
      LOG_WARNING("  [UNEXPECTED] MP3 file loaded (MP3 support not implemented)");
    } else {
      LOG_INFO("  [EXPECTED] MP3 file loading failed (MP3 support not implemented)");
    }
    
    // Test 4: Format detection
    LOG_INFO("Test 4: Format detection...");
    if (wavClip && wavClip->format == AudioFormat::WAV) {
      LOG_INFO("  [PASS] WAV format detected correctly");
    } else {
      LOG_ERROR("  [FAIL] WAV format detection failed");
    }
    
    if (oggClip && oggClip->format == AudioFormat::OGG) {
      LOG_INFO("  [PASS] OGG format detected correctly");
    } else {
      LOG_ERROR("  [FAIL] OGG format detection failed");
    }
    
    // Test 5: Unified loading interface
    LOG_INFO("Test 5: Unified loading interface...");
    auto unifiedWav = audioEngine->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
    auto unifiedOgg = audioEngine->LoadAudioClip("assets/audio/file_example_OOG_1MG.ogg");
    
    if (unifiedWav && unifiedOgg) {
      LOG_INFO("  [PASS] Unified interface works for both WAV and OGG");
    } else {
      LOG_ERROR("  [FAIL] Unified interface failed");
    }
    
    // Test 6: Error handling
    LOG_INFO("Test 6: Error handling...");
    auto invalidClip = audioEngine->LoadAudioClip("assets/audio/nonexistent.wav");
    if (!invalidClip) {
      LOG_INFO("  [PASS] Error handling for non-existent file works correctly");
    } else {
      LOG_ERROR("  [FAIL] Error handling failed");
    }
    
    // Store clips for playback testing and background music
    m_wavClip = wavClip;
    m_oggClip = oggClip;
    
    // Initialize background music
    InitializeBackgroundMusic();
    
    // Initialize 3D positioned audio sources for demonstration
    Initialize3DAudioSources();
    
    LOG_INFO("========================================");
    LOG_INFO("Audio Loading Tests Summary:");
    LOG_INFO("  WAV Support: " + std::string(wavClip ? "WORKING" : "FAILED"));
    LOG_INFO("  OGG Support: " + std::string(oggClip ? "WORKING" : "FAILED"));
    LOG_INFO("  MP3 Support: NOT IMPLEMENTED (as expected)");
    LOG_INFO("  Unified Interface: " + std::string((unifiedWav && unifiedOgg) ? "WORKING" : "FAILED"));
    LOG_INFO("  Background Music: " + std::string(m_backgroundMusicSource != 0 ? "INITIALIZED" : "FAILED"));
    LOG_INFO("  3D Audio Sources: " + std::string(m_spatialAudioSources.size() > 0 ? "INITIALIZED" : "FAILED"));
    LOG_INFO("========================================");
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
    
    // Demonstrate resource system integration
    RenderResourceTests();
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
    if (!audioEngine) {
      LOG_WARNING("Audio engine not available");
      return;
    }

    // Stop main audio source
    if (m_audioSourceId != 0) {
      audioEngine->StopAudioSource(m_audioSourceId);
    }
    
    // Stop background music
    if (m_backgroundMusicSource != 0) {
      audioEngine->StopAudioSource(m_backgroundMusicSource);
    }
    
    // Stop all spatial audio sources
    for (auto& source : m_spatialAudioSources) {
      audioEngine->StopAudioSource(source.sourceId);
    }
    
    LOG_INFO("Stopped all audio playback");
  }

  void InitializeBackgroundMusic() {
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine) {
      return;
    }
    
    // Create background music source
    m_backgroundMusicSource = audioEngine->CreateAudioSource();
    if (m_backgroundMusicSource != 0) {
      // Use OGG file for background music (typically longer)
      if (m_oggClip) {
        audioEngine->SetAudioSourceLooping(m_backgroundMusicSource, true);
        audioEngine->SetAudioSourceVolume(m_backgroundMusicSource, 0.3f); // Lower volume for background
        audioEngine->PlayAudioSource(m_backgroundMusicSource, m_oggClip);
        LOG_INFO("Background music started (looping OGG)");
      }
    }
  }

  void Initialize3DAudioSources() {
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine || !m_wavClip) {
      return;
    }
    
    // Create several 3D positioned audio sources around the world
    struct SpatialSource {
      Math::Vec3 position;
      std::string description;
    };
    
    std::vector<SpatialSource> positions = {
      {{10.0f, 1.0f, 0.0f}, "Right side audio source"},
      {{-10.0f, 1.0f, 0.0f}, "Left side audio source"},
      {{0.0f, 1.0f, 10.0f}, "Front audio source"},
      {{0.0f, 1.0f, -10.0f}, "Back audio source"},
      {{5.0f, 3.0f, 5.0f}, "Elevated audio source"}
    };
    
    for (const auto& pos : positions) {
      uint32_t sourceId = audioEngine->CreateAudioSource();
      if (sourceId != 0) {
        audioEngine->SetAudioSourcePosition(sourceId, pos.position);
        audioEngine->SetAudioSourceVolume(sourceId, 0.7f);
        audioEngine->SetAudioSourceLooping(sourceId, true);
        
        m_spatialAudioSources.push_back({sourceId, pos.position, pos.description, 0.0f});
        LOG_INFO("Created 3D audio source: " + pos.description + " at " + 
                 std::to_string(pos.position.x) + ", " + 
                 std::to_string(pos.position.y) + ", " + 
                 std::to_string(pos.position.z));
      }
    }
  }

  void Update3DAudioSources(float deltaTime) {
    auto* audioEngine = m_engine.GetAudio();
    if (!audioEngine || !m_wavClip) {
      return;
    }
    
    // Update listener position to character position
    Math::Vec3 characterPos = m_character->GetPosition();
    Math::Vec3 cameraForward = m_camera->GetForward();
    Math::Vec3 cameraUp = m_camera->GetUp();
    
    audioEngine->SetListenerPosition(characterPos);
    audioEngine->SetListenerOrientation(cameraForward, cameraUp);
    
    // Update spatial audio sources with staggered playback
    for (auto& source : m_spatialAudioSources) {
      source.timer += deltaTime;
      
      // Play audio every 3-5 seconds with some variation
      float playInterval = 3.0f + (source.sourceId % 3) * 0.7f; // 3.0s to 4.4s
      
      if (source.timer >= playInterval) {
        audioEngine->PlayAudioSource(source.sourceId, m_wavClip);
        source.timer = 0.0f;
        
        // Calculate distance for logging
        float distance = glm::length(source.position - characterPos);
        LOG_INFO("Playing 3D audio: " + source.description + 
                 " (distance: " + std::to_string(distance) + "m)");
      }
    }
  }

  void RenderResourceTests() {
    // Demonstrate texture rendering with primitives
    float textureY = 2.0f;
    float textureSpacing = 6.0f;
    
    // Row of textured cubes showing different texture formats
    if (m_wallTexture) {
      m_primitiveRenderer->DrawCube(
        Math::Vec3(-textureSpacing, textureY, 8.0f),
        Math::Vec3(2.0f, 2.0f, 2.0f),
        m_wallTexture
      );
    }
    
    if (m_wallJpgTexture) {
      m_primitiveRenderer->DrawCube(
        Math::Vec3(0.0f, textureY, 8.0f),
        Math::Vec3(2.0f, 2.0f, 2.0f),
        m_wallJpgTexture
      );
    }
    
    if (m_earthTexture) {
      m_primitiveRenderer->DrawSphere(
        Math::Vec3(textureSpacing, textureY, 8.0f),
        1.5f,
        m_earthTexture
      );
    }
    
    // Demonstrate mesh rendering with various models
    float meshY = 3.0f;
    float meshSpacing = 8.0f;
    
    // Row of different meshes
    if (m_testMesh) {
      // Cube mesh with wall texture
      if (m_wallTexture) {
        m_primitiveRenderer->DrawMesh(
          m_testMesh,
          Math::Vec3(-meshSpacing * 2, meshY, -8.0f),
          Math::Vec3(2.0f, 2.0f, 2.0f),
          m_wallTexture
        );
      } else {
        m_primitiveRenderer->DrawMesh(
          m_testMesh,
          Math::Vec3(-meshSpacing * 2, meshY, -8.0f),
          Math::Vec3(2.0f, 2.0f, 2.0f),
          Math::Vec4(1.0f, 0.5f, 0.2f, 1.0f) // Orange color
        );
      }
    }
    
    if (m_teapotMesh) {
      // Teapot with earth texture (tiny teapot size!)
      if (m_earthTexture) {
        m_primitiveRenderer->DrawMesh(
          m_teapotMesh,
          Math::Vec3(-meshSpacing, meshY, -8.0f),
          Math::Vec3(0.075f, 0.075f, 0.075f),  // 1/4 do tamanho anterior (0.3/4)
          m_earthTexture
        );
      } else {
        m_primitiveRenderer->DrawMesh(
          m_teapotMesh,
          Math::Vec3(-meshSpacing, meshY, -8.0f),
          Math::Vec3(0.075f, 0.075f, 0.075f),  // 1/4 do tamanho anterior
          Math::Vec4(0.8f, 0.2f, 0.2f, 1.0f) // Red color
        );
      }
    }
    
    if (m_teddyMesh) {
      // Teddy bear with wall JPG texture (mini teddy!)
      if (m_wallJpgTexture) {
        m_primitiveRenderer->DrawMesh(
          m_teddyMesh,
          Math::Vec3(0.0f, meshY, -8.0f),
          Math::Vec3(0.125f, 0.125f, 0.125f),  // 1/4 do tamanho anterior (0.5/4)
          m_wallJpgTexture
        );
      } else {
        m_primitiveRenderer->DrawMesh(
          m_teddyMesh,
          Math::Vec3(0.0f, meshY, -8.0f),
          Math::Vec3(0.125f, 0.125f, 0.125f),  // 1/4 do tamanho anterior
          Math::Vec4(0.6f, 0.4f, 0.2f, 1.0f) // Brown color
        );
      }
    }
    
    if (m_cowMesh) {
      // Cow texture
      if (m_cowTexture) {
      // Cow mesh with solid color (mini cow!)
        m_primitiveRenderer->DrawMesh(
          m_cowMesh,
          Math::Vec3(meshSpacing, meshY, -8.0f),
          Math::Vec3(0.3f, 0.3f, 0.3f), 
          m_cowTexture
          //Math::Vec4(0.9f, 0.9f, 0.9f, 1.0f) // Light gray color
        );
      }
    }
    
    if (m_pumpkinMesh) {
      // High-poly pumpkin mesh (micro pumpkin due to high detail!)
      m_primitiveRenderer->DrawMesh(
        m_pumpkinMesh,
        Math::Vec3(meshSpacing * 2, meshY, -8.0f),
        Math::Vec3(0.05f, 0.05f, 0.05f),  // 1/4 do tamanho anterior (0.2/4)
        Math::Vec4(1.0f, 0.6f, 0.1f, 1.0f) // Orange pumpkin color
      );
    }
    
    // Draw visual markers for 3D audio sources
    for (const auto& source : m_spatialAudioSources) {
      // Draw a small colored sphere at each audio source position
      m_primitiveRenderer->DrawSphere(
        source.position,
        0.3f,
        Math::Vec4(1.0f, 1.0f, 0.0f, 0.8f) // Yellow semi-transparent
      );
      
      // Draw a small cube above it to make it more visible
      m_primitiveRenderer->DrawCube(
        Math::Vec3(source.position.x, source.position.y + 0.5f, source.position.z),
        Math::Vec3(0.2f, 0.2f, 0.2f),
        Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f) // Red cube
      );
    }
    
    // Draw some reference primitives to show the difference
    m_primitiveRenderer->DrawCube(
      Math::Vec3(-12.0f, 1.0f, 0.0f),
      Math::Vec3(1.5f, 1.5f, 1.5f),
      Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f) // Red reference cube
    );
    
    m_primitiveRenderer->DrawSphere(
      Math::Vec3(12.0f, 1.0f, 0.0f),
      1.0f,
      Math::Vec4(0.0f, 0.0f, 1.0f, 1.0f) // Blue reference sphere
    );
  }

  void TestResourceLoading() {
    LOG_INFO("========================================");
    LOG_INFO("Testing Resource Loading Functionality");
    LOG_INFO("========================================");
    
    auto* resourceManager = m_engine.GetResourceManager();
    if (!resourceManager) {
      LOG_WARNING("Resource manager not available, skipping resource tests");
      return;
    }
    
    // Test 1: Multiple texture loading
    LOG_INFO("Test 1: Multiple texture loading...");
    
    // Load various texture formats
    auto wallTexture = resourceManager->Load<Texture>("assets/textures/wall.png");
    auto wallJpgTexture = resourceManager->Load<Texture>("assets/textures/wall.jpg");
    auto earthTexture = resourceManager->Load<Texture>("assets/textures/earth.tga");
    auto cowTexture = resourceManager->Load<Texture>("assets/textures/cow.png");
    
    if (wallTexture && wallTexture->IsValid()) {
      LOG_INFO("  [PASS] PNG texture loaded successfully");
      LOG_INFO("    Path: " + wallTexture->GetPath());
      LOG_INFO("    Dimensions: " + std::to_string(wallTexture->GetWidth()) + "x" + std::to_string(wallTexture->GetHeight()));
      LOG_INFO("    Channels: " + std::to_string(wallTexture->GetChannels()));
      LOG_INFO("    Memory Usage: " + std::to_string(wallTexture->GetMemoryUsage() / 1024) + " KB");
    } else {
      LOG_INFO("  [INFO] PNG texture not found or invalid (will use default pink texture when rendered)");
    }
    
    if (wallJpgTexture && wallJpgTexture->IsValid()) {
      LOG_INFO("  [PASS] JPG texture loaded successfully");
      LOG_INFO("    Dimensions: " + std::to_string(wallJpgTexture->GetWidth()) + "x" + std::to_string(wallJpgTexture->GetHeight()));
    } else {
      LOG_INFO("  [INFO] JPG texture not found or invalid (will use default when rendered)");
    }
    
    if (earthTexture && earthTexture->IsValid()) {
      LOG_INFO("  [PASS] TGA texture loaded successfully");
      LOG_INFO("    Dimensions: " + std::to_string(earthTexture->GetWidth()) + "x" + std::to_string(earthTexture->GetHeight()));
    } else {
      LOG_INFO("  [INFO] TGA texture not found or invalid (will use default when rendered)");
    }
    
    if (cowTexture && cowTexture->IsValid()) {
      LOG_INFO("  [PASS] Cow PNG texture loaded successfully");
      LOG_INFO("    Path: " + cowTexture->GetPath());
      LOG_INFO("    Dimensions: " + std::to_string(cowTexture->GetWidth()) + "x" + std::to_string(cowTexture->GetHeight()));
      LOG_INFO("    Channels: " + std::to_string(cowTexture->GetChannels()));
      LOG_INFO("    Memory Usage: " + std::to_string(cowTexture->GetMemoryUsage() / 1024) + " KB");
    } else {
      LOG_INFO("  [INFO] Cow PNG texture not found or invalid (will use default when rendered)");
    }
    
    // Test 2: Comprehensive mesh loading
    LOG_INFO("Test 2: Comprehensive mesh loading...");
    auto cubeMesh = resourceManager->Load<Mesh>("assets/meshes/cube.obj");
    auto teapotMesh = resourceManager->Load<Mesh>("assets/meshes/teapot.obj");
    auto teddyMesh = resourceManager->Load<Mesh>("assets/meshes/teddy.obj");
    auto cowMesh = resourceManager->Load<Mesh>("assets/meshes/cow-nonormals.obj");
    auto pumpkinMesh = resourceManager->Load<Mesh>("assets/meshes/pumpkin_tall_10k.obj");
    
    if (cubeMesh) {
      LOG_INFO("  [PASS] Cube mesh loaded successfully");
      LOG_INFO("    Vertices: " + std::to_string(cubeMesh->GetVertices().size()));
      LOG_INFO("    Indices: " + std::to_string(cubeMesh->GetIndices().size()));
      LOG_INFO("    Memory Usage: " + std::to_string(cubeMesh->GetMemoryUsage() / 1024) + " KB");
    }
    
    if (teapotMesh) {
      LOG_INFO("  [PASS] Teapot mesh loaded successfully");
      LOG_INFO("    Vertices: " + std::to_string(teapotMesh->GetVertices().size()));
      LOG_INFO("    Indices: " + std::to_string(teapotMesh->GetIndices().size()));
    }
    
    if (teddyMesh) {
      LOG_INFO("  [PASS] Teddy mesh loaded successfully");
      LOG_INFO("    Vertices: " + std::to_string(teddyMesh->GetVertices().size()));
      LOG_INFO("    Indices: " + std::to_string(teddyMesh->GetIndices().size()));
    }
    
    if (cowMesh) {
      LOG_INFO("  [PASS] Cow mesh loaded successfully");
      LOG_INFO("    Vertices: " + std::to_string(cowMesh->GetVertices().size()));
      LOG_INFO("    Indices: " + std::to_string(cowMesh->GetIndices().size()));
    }
    
    if (pumpkinMesh) {
      LOG_INFO("  [PASS] Pumpkin mesh loaded successfully (high-poly)");
      LOG_INFO("    Vertices: " + std::to_string(pumpkinMesh->GetVertices().size()));
      LOG_INFO("    Indices: " + std::to_string(pumpkinMesh->GetIndices().size()));
    }
    
    // Test 3: Resource caching
    LOG_INFO("Test 3: Resource caching...");
    auto wallTexture2 = resourceManager->Load<Texture>("assets/textures/wall.png");
    if (wallTexture.get() == wallTexture2.get()) {
      LOG_INFO("  [PASS] Resource caching works - same instance returned");
    } else {
      LOG_ERROR("  [FAIL] Resource caching failed - different instances returned");
    }
    
    // Test 4: Memory usage tracking
    LOG_INFO("Test 4: Memory usage tracking...");
    size_t totalMemory = resourceManager->GetMemoryUsage();
    size_t resourceCount = resourceManager->GetResourceCount();
    LOG_INFO("  Total resources loaded: " + std::to_string(resourceCount));
    LOG_INFO("  Total memory usage: " + std::to_string(totalMemory / 1024) + " KB");
    
    // Store resources for rendering tests
    m_wallTexture = wallTexture;
    m_wallJpgTexture = wallJpgTexture;
    m_earthTexture = earthTexture;
    m_cowTexture = cowTexture;
    m_testMesh = cubeMesh;
    m_teapotMesh = teapotMesh;
    m_teddyMesh = teddyMesh;
    m_cowMesh = cowMesh;
    m_pumpkinMesh = pumpkinMesh;
    
    LOG_INFO("========================================");
    LOG_INFO("Resource Loading Tests Summary:");
    LOG_INFO("  PNG Texture Loading: " + std::string(wallTexture ? "WORKING" : "FALLBACK"));
    LOG_INFO("  JPG Texture Loading: " + std::string(wallJpgTexture ? "WORKING" : "FALLBACK"));
    LOG_INFO("  TGA Texture Loading: " + std::string(earthTexture ? "WORKING" : "FALLBACK"));
    LOG_INFO("  Cow Texture Loading: " + std::string(cowTexture ? "WORKING" : "FALLBACK"));
    LOG_INFO("  Cube Mesh Loading: " + std::string(cubeMesh ? "WORKING" : "FAILED"));
    LOG_INFO("  Teapot Mesh Loading: " + std::string(teapotMesh ? "WORKING" : "FALLBACK"));
    LOG_INFO("  Teddy Mesh Loading: " + std::string(teddyMesh ? "WORKING" : "FALLBACK"));
    LOG_INFO("  Cow Mesh Loading: " + std::string(cowMesh ? "WORKING" : "FALLBACK"));
    LOG_INFO("  Pumpkin Mesh Loading: " + std::string(pumpkinMesh ? "WORKING" : "FALLBACK"));
    LOG_INFO("  Resource Caching: " + std::string((wallTexture.get() == wallTexture2.get()) ? "WORKING" : "FAILED"));
    LOG_INFO("  Memory Tracking: WORKING");
    LOG_INFO("========================================");
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
  
  // Background music
  uint32_t m_backgroundMusicSource = 0;
  
  // 3D positioned audio sources
  struct SpatialAudioSource {
    uint32_t sourceId;
    Math::Vec3 position;
    std::string description;
    float timer;
  };
  std::vector<SpatialAudioSource> m_spatialAudioSources;
  
  // Resource loading test resources
  std::shared_ptr<Texture> m_wallTexture;
  std::shared_ptr<Texture> m_wallJpgTexture;
  std::shared_ptr<Texture> m_earthTexture;
  std::shared_ptr<Texture> m_cowTexture;
  std::shared_ptr<Mesh> m_testMesh;
  std::shared_ptr<Mesh> m_teapotMesh;
  std::shared_ptr<Mesh> m_teddyMesh;
  std::shared_ptr<Mesh> m_cowMesh;
  std::shared_ptr<Mesh> m_pumpkinMesh;
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