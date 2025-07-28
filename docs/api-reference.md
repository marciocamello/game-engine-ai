# API Reference

Complete API documentation for Game Engine Kiro. This reference covers all public classes, methods, and interfaces available to game developers.

## 🎯 Core Systems

### Engine Class

The main engine orchestrator that manages all subsystems.

```cpp
namespace GameEngine {
    class Engine {
    public:
        // Lifecycle
        bool Initialize();
        void Run();
        void Shutdown();

        // Subsystem Access
        GraphicsRenderer* GetRenderer() const;
        InputManager* GetInput() const;
        AudioEngine* GetAudio() const;
        PhysicsEngine* GetPhysics() const;
        ResourceManager* GetResourceManager() const;

        // Callbacks
        void SetUpdateCallback(std::function<void(float)> callback);
        void SetRenderCallback(std::function<void()> callback);

        // State
        bool IsRunning() const;
        float GetDeltaTime() const;
    };
}
```

**Usage Example:**

```cpp
Engine engine;
if (!engine.Initialize()) {
    return -1;
}

engine.SetUpdateCallback([](float deltaTime) {
    // Game logic here
});

engine.Run();
```

## 🎨 Graphics System

### GraphicsRenderer

Abstract base class for all renderers.

```cpp
class GraphicsRenderer {
public:
    // Lifecycle
    virtual bool Initialize(const RenderSettings& settings) = 0;
    virtual void Shutdown() = 0;

    // Frame Management
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;

    // Rendering
    virtual void Clear(const Math::Vec4& color = Math::Vec4(0.0f)) = 0;
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void SetCamera(const Camera* camera) = 0;
    virtual void DrawMesh(const Mesh* mesh, const Material* material,
                         const Math::Mat4& transform) = 0;

    // Resource Creation
    virtual std::shared_ptr<Shader> CreateShader(const std::string& vertexSource,
                                                 const std::string& fragmentSource) = 0;
    virtual std::shared_ptr<Texture> CreateTexture(const std::string& filepath) = 0;
    virtual std::shared_ptr<Mesh> CreateMesh(const std::vector<float>& vertices,
                                            const std::vector<unsigned int>& indices) = 0;

    // Properties
    GLFWwindow* GetWindow() const;
    const RenderSettings& GetSettings() const;
};
```

### RenderSettings

Configuration structure for renderer initialization.

```cpp
struct RenderSettings {
    int windowWidth = 1920;
    int windowHeight = 1080;
    bool fullscreen = false;
    bool vsync = true;
    int msaaSamples = 4;
    GraphicsAPI api = GraphicsAPI::OpenGL;
};
```

### Camera

Base camera class with perspective and orthographic projection support.

```cpp
class Camera {
public:
    enum class CameraType { Perspective, Orthographic };

    // Construction
    Camera(CameraType type = CameraType::Perspective);

    // Transform
    void SetPosition(const Math::Vec3& position);
    void SetRotation(const Math::Quat& rotation);
    const Math::Vec3& GetPosition() const;
    const Math::Quat& GetRotation() const;

    // Projection
    void SetPerspective(float fov, float aspect, float nearPlane, float farPlane);
    void SetOrthographic(float left, float right, float bottom, float top,
                        float nearPlane, float farPlane);

    // View
    void LookAt(const Math::Vec3& target, const Math::Vec3& up = Math::Vec3(0, 1, 0));
    void Orbit(const Math::Vec3& target, float deltaYaw, float deltaPitch, float distance);

    // Matrices
    const Math::Mat4& GetViewMatrix() const;
    const Math::Mat4& GetProjectionMatrix() const;
    Math::Mat4 GetViewProjectionMatrix() const;
};
```

### PrimitiveRenderer

Utility class for rendering basic shapes.

```cpp
class PrimitiveRenderer {
public:
    bool Initialize();
    void Shutdown();

    // Matrix Setup
    void SetViewProjectionMatrix(const Math::Mat4& viewProjection);

    // Drawing Methods
    void DrawCube(const Math::Vec3& position, const Math::Vec3& size,
                  const Math::Vec4& color = Math::Vec4(1.0f));
    void DrawSphere(const Math::Vec3& position, float radius,
                    const Math::Vec4& color = Math::Vec4(1.0f));
    void DrawPlane(const Math::Vec3& position, const Math::Vec2& size,
                   const Math::Vec4& color = Math::Vec4(1.0f));
    void DrawLine(const Math::Vec3& start, const Math::Vec3& end,
                  const Math::Vec4& color = Math::Vec4(1.0f));
};
```

## 🎮 Input System

### InputManager

Comprehensive input handling with support for keyboard, mouse, and gamepad.

```cpp
class InputManager {
public:
    // Lifecycle
    bool Initialize(GLFWwindow* window);
    void Shutdown();
    void Update();

    // Keyboard Input
    bool IsKeyPressed(KeyCode key) const;   // Single press
    bool IsKeyDown(KeyCode key) const;      // Held down
    bool IsKeyReleased(KeyCode key) const;  // Single release

    // Mouse Input
    bool IsMouseButtonPressed(MouseButton button) const;
    bool IsMouseButtonDown(MouseButton button) const;
    bool IsMouseButtonReleased(MouseButton button) const;
    Math::Vec2 GetMousePosition() const;
    Math::Vec2 GetMouseDelta() const;
    float GetMouseScrollDelta() const;

    // Gamepad Input
    bool IsGamepadConnected(int gamepadId = 0) const;
    bool IsGamepadButtonPressed(GamepadButton button, int gamepadId = 0) const;
    bool IsGamepadButtonDown(GamepadButton button, int gamepadId = 0) const;
    float GetGamepadAxis(GamepadAxis axis, int gamepadId = 0) const;

    // Action Mapping
    void BindAction(const std::string& actionName, KeyCode key);
    void BindAction(const std::string& actionName, MouseButton button);
    void BindAction(const std::string& actionName, GamepadButton button, int gamepadId = 0);
    bool IsActionPressed(const std::string& actionName) const;
    bool IsActionDown(const std::string& actionName) const;
    bool IsActionReleased(const std::string& actionName) const;
};
```

### KeyCode Enum

```cpp
enum class KeyCode {
    // Letters
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Numbers
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

    // Function Keys
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    // Special Keys
    Space = 32, Enter = 257, Tab = 258, Backspace = 259, Delete = 261,
    Escape = 256, LeftShift = 340, RightShift = 344, LeftCtrl = 341,
    RightCtrl = 345, LeftAlt = 342, RightAlt = 346,

    // Arrow Keys
    Up = 265, Down = 264, Left = 263, Right = 262
};
```

## 🔊 Audio System

### AudioEngine

Complete 3D spatial audio system with OpenAL backend, supporting multiple audio formats and advanced performance optimizations.

```cpp
class AudioEngine {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);

    // Error handling and recovery
    bool IsAudioAvailable() const;
    bool IsOpenALInitialized() const;
    bool AttemptAudioRecovery();
    void HandleDeviceDisconnection();

    // Audio clip management
    std::shared_ptr<AudioClip> LoadAudioClip(const std::string& path);
    void UnloadAudioClip(const std::string& path);

    // Audio source management
    uint32_t CreateAudioSource();
    void DestroyAudioSource(uint32_t sourceId);
    void PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip);
    void StopAudioSource(uint32_t sourceId);
    void PauseAudioSource(uint32_t sourceId);
    void SetAudioSourcePosition(uint32_t sourceId, const Math::Vec3& position);
    void SetAudioSourceVolume(uint32_t sourceId, float volume);
    void SetAudioSourcePitch(uint32_t sourceId, float pitch);
    void SetAudioSourceLooping(uint32_t sourceId, bool looping);

    // Listener management
    void SetListenerPosition(const Math::Vec3& position);
    void SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up);
    void SetListenerVelocity(const Math::Vec3& velocity);

    // Global settings
    void SetMasterVolume(float volume);
    void SetMusicVolume(float volume);
    void SetSFXVolume(float volume);

    // Performance optimization controls
    void EnableBufferPooling(bool enabled);
    void EnableSourcePooling(bool enabled);
    void EnableOptimized3DAudio(bool enabled);
    void SetBufferPoolSize(size_t maxSize);
    void SetSourcePoolSize(size_t minSize, size_t maxSize);
    void MarkAudioAsHot(const std::string& filepath);
    void UnmarkAudioAsHot(const std::string& filepath);

    // Performance statistics
    float GetBufferPoolHitRatio() const;
    float GetSourcePoolUtilization() const;
    size_t GetBufferPoolMemoryUsage() const;
    int GetAudio3DCalculationsPerSecond() const;

    // OpenAL error checking
    static bool CheckOpenALError(const std::string& operation);
    static std::string GetOpenALErrorString(ALenum error);
};
```

### AudioClip

Audio clip structure with format support and metadata.

```cpp
enum class AudioFormat {
    WAV,    // PCM WAV files (fully supported)
    OGG,    // Ogg Vorbis files (fully supported)
    MP3     // MP3 files (not implemented)
};

struct AudioClip {
    std::string path;
    AudioFormat format;
    float duration = 0.0f;
    int sampleRate = 44100;
    int channels = 2;
    bool is3D = true;

#ifdef GAMEENGINE_HAS_OPENAL
    ALuint bufferId = 0;
#endif
};
```

### AudioSource

Individual audio source with 3D positioning and playback control.

````cpp
class AudioSource {
public:
    AudioSource(uint32_t id);
    ~AudioSource();

    void Play(std::shared_ptr<AudioClip> clip);
    void Stop();
    void Pause();
    void Resume();

    void SetPosition(const Math::Vec3& position);
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLooping(bool looping);

    bool IsPlaying() const;
    bool IsPaused() const;
    bool IsStopped() const;

    // Get current playback state from OpenAL
    bool GetOpenALPlayingState() const;

    // Get source ID
    uint32_t GetId() const;
};

### AudioListener

Audio listener representing the player's ears in 3D space.

```cpp
class AudioListener {
public:
    AudioListener();
    ~AudioListener();

    void SetPosition(const Math::Vec3& position);
    void SetOrientation(const Math::Vec3& forward, const Math::Vec3& up);
    void SetVelocity(const Math::Vec3& velocity);
};
````

**Audio System Usage Examples:**

```cpp
// Initialize audio system
auto* audioEngine = engine.GetAudio();
if (!audioEngine->Initialize()) {
    LOG_ERROR("Failed to initialize audio system");
    return false;
}

// Load audio clips
auto jumpSound = audioEngine->LoadAudioClip("assets/audio/jump.wav");
auto backgroundMusic = audioEngine->LoadAudioClip("assets/audio/music.ogg");

// Create and configure audio sources
uint32_t jumpSourceId = audioEngine->CreateAudioSource();
uint32_t musicSourceId = audioEngine->CreateAudioSource();

// Configure 3D positioned sound effect
audioEngine->SetAudioSourcePosition(jumpSourceId, Math::Vec3(0.0f, 1.0f, 0.0f));
audioEngine->SetAudioSourceVolume(jumpSourceId, 0.8f);

// Configure background music (non-positional)
audioEngine->SetAudioSourceLooping(musicSourceId, true);
audioEngine->SetAudioSourceVolume(musicSourceId, 0.3f);

// Play audio
audioEngine->PlayAudioSource(jumpSourceId, jumpSound);
audioEngine->PlayAudioSource(musicSourceId, backgroundMusic);

// Update listener position (typically in game loop)
Math::Vec3 playerPosition = character->GetPosition();
Math::Vec3 cameraForward = camera->GetForward();
Math::Vec3 cameraUp = camera->GetUp();

audioEngine->SetListenerPosition(playerPosition);
audioEngine->SetListenerOrientation(cameraForward, cameraUp);

// Performance optimization
audioEngine->EnableBufferPooling(true);
audioEngine->EnableSourcePooling(true);
audioEngine->SetBufferPoolSize(64);
audioEngine->SetSourcePoolSize(8, 32);

// Mark frequently used audio as "hot" for optimization
audioEngine->MarkAudioAsHot("assets/audio/footstep.wav");
audioEngine->MarkAudioAsHot("assets/audio/jump.wav");

// Error handling and recovery
if (!audioEngine->IsAudioAvailable()) {
    LOG_WARNING("Audio system not available, continuing in silent mode");
} else if (!audioEngine->IsOpenALInitialized()) {
    if (audioEngine->AttemptAudioRecovery()) {
        LOG_INFO("Audio system recovered successfully");
    }
}

// Performance monitoring
float bufferHitRatio = audioEngine->GetBufferPoolHitRatio();
float sourceUtilization = audioEngine->GetSourcePoolUtilization();
size_t bufferMemory = audioEngine->GetBufferPoolMemoryUsage();
int calculations = audioEngine->GetAudio3DCalculationsPerSecond();

LOG_INFO("Audio Performance:");
LOG_INFO("  Buffer hit ratio: " + std::to_string(bufferHitRatio * 100) + "%");
LOG_INFO("  Source utilization: " + std::to_string(sourceUtilization * 100) + "%");
LOG_INFO("  Buffer memory: " + std::to_string(bufferMemory / 1024) + " KB");
LOG_INFO("  3D calculations/sec: " + std::to_string(calculations));

// Cleanup
audioEngine->DestroyAudioSource(jumpSourceId);
audioEngine->DestroyAudioSource(musicSourceId);
audioEngine->UnloadAudioClip("assets/audio/jump.wav");
audioEngine->UnloadAudioClip("assets/audio/music.ogg");
```

};

````

### AudioListener

3D audio listener (typically follows the camera/player).

```cpp
class AudioListener {
public:
    AudioListener();
    ~AudioListener();

    void SetPosition(const Math::Vec3& position);
    void SetOrientation(const Math::Vec3& forward, const Math::Vec3& up);
    void SetVelocity(const Math::Vec3& velocity);
};
````

**Supported Audio Formats:**

- **WAV**: Full PCM support with custom parser
- **OGG**: Ogg Vorbis support via STB Vorbis integration
- **MP3**: Not implemented (patent concerns)

**Performance Features:**

- **Buffer Pooling**: Automatic caching of frequently used audio clips
- **Source Pooling**: Reusable audio sources to reduce allocation overhead
- **3D Audio Optimization**: Efficient spatial audio calculations
- **Hot Audio Marking**: Priority loading for critical audio files
- **Memory Management**: Automatic cleanup of unused audio resources

**Usage Examples:**

```cpp
// Basic audio setup
auto* audioEngine = engine.GetAudio();
if (!audioEngine->Initialize()) {
    LOG_ERROR("Failed to initialize audio system");
    return false;
}

// Load audio clips (supports WAV and OGG)
auto jumpSound = audioEngine->LoadAudioClip("assets/audio/jump.wav");
auto backgroundMusic = audioEngine->LoadAudioClip("assets/audio/music.ogg");

// Create and configure audio source
uint32_t sourceId = audioEngine->CreateAudioSource();
audioEngine->SetAudioSourcePosition(sourceId, Math::Vec3(10.0f, 1.0f, 0.0f));
audioEngine->SetAudioSourceVolume(sourceId, 0.8f);
audioEngine->SetAudioSourceLooping(sourceId, false);

// Play audio
audioEngine->PlayAudioSource(sourceId, jumpSound);

// Background music with looping
uint32_t musicSourceId = audioEngine->CreateAudioSource();
audioEngine->SetAudioSourceLooping(musicSourceId, true);
audioEngine->SetAudioSourceVolume(musicSourceId, 0.3f);
audioEngine->PlayAudioSource(musicSourceId, backgroundMusic);

// Update listener position (typically in game loop)
Math::Vec3 playerPos = character->GetPosition();
Math::Vec3 cameraForward = camera->GetForward();
Math::Vec3 cameraUp = camera->GetUp();

audioEngine->SetListenerPosition(playerPos);
audioEngine->SetListenerOrientation(cameraForward, cameraUp);

// Performance optimization
audioEngine->EnableBufferPooling(true);
audioEngine->EnableSourcePooling(true);
audioEngine->SetSourcePoolSize(8, 32); // Min 8, Max 32 sources
audioEngine->MarkAudioAsHot("assets/audio/footstep.wav"); // Priority loading

// Error handling and recovery
if (!audioEngine->IsAudioAvailable()) {
    LOG_WARNING("Audio system not available, continuing in silent mode");
} else if (!audioEngine->IsOpenALInitialized()) {
    if (audioEngine->AttemptAudioRecovery()) {
        LOG_INFO("Audio system recovered successfully");
    }
}

// Performance monitoring
float bufferHitRatio = audioEngine->GetBufferPoolHitRatio();
float sourceUtilization = audioEngine->GetSourcePoolUtilization();
size_t audioMemoryUsage = audioEngine->GetBufferPoolMemoryUsage();

LOG_INFO("Audio Performance:");
LOG_INFO("  Buffer hit ratio: " + std::to_string(bufferHitRatio * 100) + "%");
LOG_INFO("  Source utilization: " + std::to_string(sourceUtilization * 100) + "%");
LOG_INFO("  Memory usage: " + std::to_string(audioMemoryUsage / 1024) + " KB");

// Cleanup
audioEngine->DestroyAudioSource(sourceId);
audioEngine->DestroyAudioSource(musicSourceId);
audioEngine->Shutdown();
```

**Integration with Game Application:**

```cpp
// Audio integration should be handled at the game application level
class GameApplication {
private:
    std::unique_ptr<Character> m_character;
    std::unique_ptr<AudioEngine> m_audioEngine;
    uint32_t m_jumpSoundSource = 0;
    uint32_t m_footstepSoundSource = 0;
    std::shared_ptr<AudioClip> m_jumpSound;
    std::shared_ptr<AudioClip> m_footstepSound;

public:
    bool Initialize() {
        // Initialize character without audio dependencies
        m_character = std::make_unique<Character>();
        m_character->Initialize(m_engine.GetPhysics());

        // Initialize audio system separately
        if (m_audioEngine && m_audioEngine->IsAudioAvailable()) {
            // Load character sounds
            m_jumpSound = m_audioEngine->LoadAudioClip("assets/audio/jump.wav");
            m_footstepSound = m_audioEngine->LoadAudioClip("assets/audio/footstep.wav");

            // Create audio sources
            m_jumpSoundSource = m_audioEngine->CreateAudioSource();
            m_footstepSoundSource = m_audioEngine->CreateAudioSource();

            // Configure sources
            m_audioEngine->SetAudioSourceVolume(m_jumpSoundSource, 0.7f);
            m_audioEngine->SetAudioSourceVolume(m_footstepSoundSource, 0.5f);
        }
        return true;
    }

    void Update(float deltaTime) {
        // Update character logic
        m_character->Update(deltaTime, input, camera);

        // Handle audio feedback based on character state
        if (m_audioEngine && m_jumpSoundSource != 0) {
            // Update audio source positions based on character position
            Math::Vec3 characterPos = m_character->GetPosition();
            m_audioEngine->SetAudioSourcePosition(m_jumpSoundSource, characterPos);
            m_audioEngine->SetAudioSourcePosition(m_footstepSoundSource, characterPos);

            // Play jump sound when character jumps
            if (m_character->IsJumping() && m_jumpSound) {
                m_audioEngine->PlayAudioSource(m_jumpSoundSource, m_jumpSound);
            }
        }
    }
};
```

## ⚡ Physics System

### PhysicsEngine

Dual-backend physics system supporting Bullet Physics and NVIDIA PhysX.

```cpp
class PhysicsEngine {
public:
    // Lifecycle
    bool Initialize(const PhysicsConfig& config = {});
    void Shutdown();
    void Update(float deltaTime);

    // World Management
    void SetGravity(const Math::Vec3& gravity);
    Math::Vec3 GetGravity() const;

    // Rigid Bodies
    uint32_t CreateRigidBody(const RigidBodyDesc& desc);
    void DestroyRigidBody(uint32_t bodyId);
    void SetRigidBodyPosition(uint32_t bodyId, const Math::Vec3& position);
    void SetRigidBodyRotation(uint32_t bodyId, const Math::Quat& rotation);
    void SetRigidBodyVelocity(uint32_t bodyId, const Math::Vec3& velocity);
    void ApplyForce(uint32_t bodyId, const Math::Vec3& force);
    void ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse);

    // Collision Detection
    bool Raycast(const Ray& ray, RaycastHit& hit);
    bool SphereCast(const Math::Vec3& origin, float radius, const Math::Vec3& direction,
                   float maxDistance, RaycastHit& hit);
    std::vector<uint32_t> OverlapSphere(const Math::Vec3& center, float radius);

    // Backend Management
    PhysicsBackend GetCurrentBackend() const;
    bool SetBackend(PhysicsBackend backend);

    // Debug
    void SetDebugDrawEnabled(bool enabled);
    bool IsDebugDrawEnabled() const;
};
```

### PhysicsConfig

```cpp
struct PhysicsConfig {
    PhysicsBackend backend = PhysicsBackend::Auto;
    bool fallbackToBullet = true;
    bool enableGPU = false;
    bool enableDebugDraw = false;
    bool deterministic = false;
    int maxRigidBodies = 1000;
    Math::Vec3 gravity = Math::Vec3(0.0f, -9.81f, 0.0f);
};

enum class PhysicsBackend {
    Auto,    // Automatic selection
    Bullet,  // Bullet Physics
    PhysX    // NVIDIA PhysX (planned)
};
```

### RigidBodyDesc

```cpp
struct RigidBodyDesc {
    Math::Vec3 position = Math::Vec3(0.0f);
    Math::Quat rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
    Math::Vec3 velocity = Math::Vec3(0.0f);
    Math::Vec3 angularVelocity = Math::Vec3(0.0f);
    float mass = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    bool isKinematic = false;
    bool isTrigger = false;
};
```

## 🎮 Game Components

### Character

Component-based character controller with modular movement system.

```cpp
class Character {
public:
    // Lifecycle
    Character();
    ~Character();
    bool Initialize(PhysicsEngine* physicsEngine = nullptr);

    // Update
    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr);
    void Render(PrimitiveRenderer* renderer);

    // Transform (delegated to movement component)
    void SetPosition(const Math::Vec3& position);
    const Math::Vec3& GetPosition() const;
    void SetRotation(float yaw);
    float GetRotation() const;

    // Movement Properties (delegated to movement component)
    void SetMoveSpeed(float speed);
    float GetMoveSpeed() const;
    const Math::Vec3& GetVelocity() const;

    // Character Properties
    float GetHeight() const;
    float GetRadius() const;
    void SetCharacterSize(float radius, float height);

    // Movement State Queries
    bool IsGrounded() const;
    bool IsJumping() const;
    bool IsFalling() const;

    // Movement Component Management
    void SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component);
    CharacterMovementComponent* GetMovementComponent() const;

    // Convenience Methods for Switching Movement Types
    void SwitchToPhysicsMovement();
    void SwitchToDeterministicMovement();
    void SwitchToHybridMovement();

    // Movement Type Information
    const char* GetMovementTypeName() const;
    Math::Vec4 GetMovementTypeColor() const;
};
```

### CharacterMovementComponent

Base class for all character movement implementations.

```cpp
class CharacterMovementComponent {
public:
    enum class MovementMode { Walking, Falling, Flying, Swimming, Custom };

    struct MovementConfig {
        float maxWalkSpeed = 6.0f;          // Maximum walking speed (m/s)
        float maxAcceleration = 20.0f;      // Maximum acceleration (m/s²)
        float brakingDeceleration = 20.0f;  // Braking deceleration (m/s²)
        float jumpZVelocity = 10.0f;        // Initial jump velocity (m/s)
        float gravityScale = 1.0f;          // Gravity multiplier
        float airControl = 0.2f;            // Air control factor (0-1)
        float groundFriction = 8.0f;        // Ground friction coefficient
        float maxStepHeight = 0.3f;         // Maximum step height (m)
        float maxSlopeAngle = 45.0f;        // Maximum walkable slope (degrees)
        bool canJump = true;                // Whether jumping is allowed
        bool canWalkOffLedges = true;       // Whether character can walk off edges
    };

    // Component Lifecycle
    virtual bool Initialize(PhysicsEngine* physicsEngine) = 0;
    virtual void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) = 0;
    virtual void Shutdown() = 0;

    // Transform Interface
    virtual void SetPosition(const Math::Vec3& position) = 0;
    virtual const Math::Vec3& GetPosition() const = 0;
    virtual void SetRotation(float yaw) = 0;
    virtual float GetRotation() const = 0;

    // Velocity Interface
    virtual const Math::Vec3& GetVelocity() const = 0;
    virtual void SetVelocity(const Math::Vec3& velocity) = 0;
    virtual void AddVelocity(const Math::Vec3& deltaVelocity) = 0;

    // Movement State
    virtual MovementMode GetMovementMode() const;
    virtual bool IsGrounded() const = 0;
    virtual bool IsJumping() const = 0;
    virtual bool IsFalling() const = 0;

    // Configuration
    virtual void SetMovementConfig(const MovementConfig& config);
    virtual const MovementConfig& GetMovementConfig() const;

    // Character Properties
    virtual void SetCharacterSize(float radius, float height);
    virtual float GetCharacterRadius() const;
    virtual float GetCharacterHeight() const;

    // Movement Commands
    virtual void Jump() = 0;
    virtual void StopJumping() = 0;
    virtual void AddMovementInput(const Math::Vec3& worldDirection, float scaleValue = 1.0f) = 0;

    // Component Type Identification
    virtual const char* GetComponentTypeName() const = 0;
};
```

### DeterministicMovementComponent

Precise character control without physics simulation.

```cpp
class DeterministicMovementComponent : public CharacterMovementComponent {
public:
    // Component Lifecycle
    bool Initialize(PhysicsEngine* physicsEngine) override;
    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) override;
    void Shutdown() override;

    // Component Identification
    const char* GetComponentTypeName() const override { return "DeterministicMovementComponent"; }

    // Deterministic-Specific Configuration
    void SetGroundLevel(float groundLevel);
    float GetGroundLevel() const;
    void SetGravity(float gravity);
    float GetGravity() const;

    // All other methods inherited from CharacterMovementComponent
};
```

### HybridMovementComponent

Physics collision detection with direct position control.

```cpp
class HybridMovementComponent : public CharacterMovementComponent {
public:
    struct CollisionInfo {
        bool hasCollision = false;
        Math::Vec3 contactPoint{0.0f};
        Math::Vec3 contactNormal{0.0f};
        float penetrationDepth = 0.0f;
        float distance = 0.0f;
        uint32_t hitBodyId = 0;
    };

    struct StepInfo {
        bool canStepUp = false;
        float stepHeight = 0.0f;
        Math::Vec3 stepPosition{0.0f};
    };

    // Component Lifecycle
    bool Initialize(PhysicsEngine* physicsEngine) override;
    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) override;
    void Shutdown() override;

    // Component Identification
    const char* GetComponentTypeName() const override { return "HybridMovementComponent"; }

    // Hybrid-Specific Configuration
    void SetSkinWidth(float width);
    float GetSkinWidth() const;
    void SetGroundCheckDistance(float distance);
    float GetGroundCheckDistance() const;

    // All other methods inherited from CharacterMovementComponent
};
```

### PhysicsMovementComponent

Full physics simulation for dynamic character movement.

```cpp
class PhysicsMovementComponent : public CharacterMovementComponent {
public:
    // Component Lifecycle
    bool Initialize(PhysicsEngine* physicsEngine) override;
    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) override;
    void Shutdown() override;

    // Component Identification
    const char* GetComponentTypeName() const override { return "PhysicsMovementComponent"; }

    // Physics-Specific Methods
    uint32_t GetRigidBodyId() const;
    void SetMass(float mass);
    float GetMass() const;
    void SetLinearDamping(float damping);
    void SetAngularDamping(float damping);

    // All other methods inherited from CharacterMovementComponent
};
```

### MovementComponentFactory

Factory for creating movement components.

```cpp
class MovementComponentFactory {
public:
    enum class ComponentType {
        Deterministic,  // Precise control without physics simulation
        Hybrid,         // Physics collision with direct control
        Physics         // Full physics simulation
    };

    // Component Creation
    static std::unique_ptr<CharacterMovementComponent> CreateComponent(ComponentType type);

    // Configuration Presets
    static CharacterMovementComponent::MovementConfig GetDefaultConfig(ComponentType type);
    static CharacterMovementComponent::MovementConfig GetPlatformingConfig();
    static CharacterMovementComponent::MovementConfig GetRealisticConfig();
    static CharacterMovementComponent::MovementConfig GetArcadeConfig();
};
```

### ThirdPersonCameraSystem

Professional third-person camera with SpringArm component.

```cpp
class ThirdPersonCameraSystem : public Camera {
public:
    // Lifecycle
    ThirdPersonCameraSystem();
    ~ThirdPersonCameraSystem() = default;

    // Update
    void Update(float deltaTime, InputManager* input);

    // Target
    void SetTarget(Character* target);
    Character* GetTarget() const;

    // Configuration
    void SetArmLength(float length);
    void SetRotationLimits(float minPitch, float maxPitch);
    void SetSensitivity(float yawSensitivity, float pitchSensitivity);
    void SetSmoothingSpeed(float rotationSpeed, float positionSpeed);
    void SetShoulderOffset(float rightOffset, float upOffset);
    void SetMouseSensitivity(float sensitivity);
    void SetFollowCameraMode(bool enabled);

    // Direction Queries
    Math::Vec3 GetForwardDirection() const;
    Math::Vec3 GetRightDirection() const;
    float GetCameraYaw() const;

    // Movement Helper
    Math::Vec3 GetMovementDirection(float inputForward, float inputRight) const;
};
```

### SpringArm

Camera positioning component for third-person cameras.

```cpp
class SpringArm {
public:
    // Lifecycle
    SpringArm();
    ~SpringArm() = default;

    // Update
    void Update(float deltaTime, const Math::Vec3& targetPosition,
                float inputYaw, float inputPitch);

    // Position Calculation
    Math::Vec3 GetCameraPosition() const;
    Math::Vec3 GetViewDirection() const;

    // Configuration
    void SetLength(float length);
    void SetRotationLimits(float minPitch, float maxPitch);
    void SetSensitivity(float yawSensitivity, float pitchSensitivity);
    void SetSmoothingSpeed(float rotationSpeed, float positionSpeed);
    void SetShoulderOffset(float rightOffset, float upOffset);

    // State Queries
    float GetLength() const;
    float GetYaw() const;
    float GetPitch() const;
    const Math::Vec3& GetTargetPosition() const;
};
```

## 📦 Resource Management

### ResourceManager

Comprehensive resource management system with automatic caching, memory management, and debugging capabilities. Enhanced with new statistics tracking and memory management features for improved testing and debugging.

**Supported Resource Types:**

- **Texture**: PNG, JPG, TGA image formats with STB integration
- **Mesh**: OBJ 3D model format with vertex/normal/texture coordinate support
- **AudioClip**: WAV and OGG audio formats (integrated with AudioEngine)

**Key Features:**

- **Automatic Caching**: Weak pointer-based caching prevents memory leaks
- **Fallback Resources**: Default resources when loading fails (pink textures, cube meshes)
- **Memory Management**: Automatic cleanup and memory pressure handling
- **Performance Optimization**: GPU upload optimization and memory pooling
- **Path Handling**: Smart path resolution to avoid asset directory duplication

```cpp
class ResourceManager {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();

    // Resource Loading (Template-based)
    template<typename T>
    std::shared_ptr<T> Load(const std::string& path);

    template<typename T>
    void Unload(const std::string& path);

    void UnloadAll();
    void UnloadUnused();

    // Memory Management
    void UnloadLeastRecentlyUsed(size_t targetMemoryReduction = 0);
    void SetMemoryPressureThreshold(size_t thresholdBytes);
    void CheckMemoryPressure();

    // Statistics and Debugging (Enhanced)
    size_t GetMemoryUsage() const;           // Total memory usage across all resources
    size_t GetResourceCount() const;         // Total number of loaded resources
    ResourceStats GetResourceStats() const;  // Detailed statistics breakdown
    void LogResourceUsage() const;           // Log basic usage information
    void LogDetailedResourceInfo() const;    // Log comprehensive resource details

    // Asset Pipeline
    bool ImportAsset(const std::string& sourcePath, const std::string& targetPath);
    bool ExportAsset(const std::string& assetPath, const std::string& exportPath);
};
```

### ResourceStats

Detailed statistics structure for resource monitoring and debugging. Enhanced with additional metrics for comprehensive resource analysis.

```cpp
struct ResourceStats {
    size_t totalResources = 0;           // Total number of loaded resources
    size_t totalMemoryUsage = 0;         // Total memory usage in bytes
    size_t expiredReferences = 0;        // Number of expired weak references
    std::unordered_map<std::string, size_t> resourcesByType;  // Resources by type
    std::unordered_map<std::string, size_t> memoryByType;     // Memory usage by type

    // Enhanced statistics for testing and debugging
    size_t cacheHits = 0;                // Number of cache hits
    size_t cacheMisses = 0;              // Number of cache misses
    double averageLoadTime = 0.0;        // Average resource loading time (ms)
    size_t peakMemoryUsage = 0;          // Peak memory usage since last reset
};
```

### Resource Base Class

Base class for all resources with memory tracking and access time monitoring.

```cpp
class Resource {
public:
    Resource(const std::string& path);
    virtual ~Resource() = default;

    // Path and Identity
    const std::string& GetPath() const;

    // Memory Management
    virtual size_t GetMemoryUsage() const;

    // Access Tracking
    std::chrono::steady_clock::time_point GetLoadTime() const;
    std::chrono::steady_clock::time_point GetLastAccessTime() const;
    void UpdateLastAccessTime() const;
};
```

### Texture Resource

Image texture resource with multiple format support.

```cpp
class Texture : public Resource {
public:
    Texture(const std::string& path);
    ~Texture();

    // Loading
    bool LoadFromFile(const std::string& filepath);
    void CreateDefault(); // Creates pink/magenta fallback texture

    // Properties
    int GetWidth() const;
    int GetHeight() const;
    int GetChannels() const;
    GLuint GetTextureId() const;
    bool IsValid() const;

    // Memory Management
    size_t GetMemoryUsage() const override;
};
```

**Supported Texture Formats:**

- **PNG**: Full RGBA support with transparency
- **JPG**: RGB support, good for photographs
- **TGA**: RGB/RGBA support, good for game assets

### Mesh Resource

3D mesh resource with OBJ format support.

```cpp
class Mesh : public Resource {
public:
    Mesh(const std::string& path);
    ~Mesh();

    // Loading
    bool LoadFromFile(const std::string& filepath);
    void CreateDefault(); // Creates default cube mesh

    // Rendering
    void Render() const;
    GLuint GetVAO() const;

    // Properties
    const std::vector<float>& GetVertices() const;
    const std::vector<unsigned int>& GetIndices() const;
    size_t GetVertexCount() const;
    size_t GetIndexCount() const;

    // Memory Management
    size_t GetMemoryUsage() const override;
};
```

**Supported Mesh Features:**

- **Vertices**: Position, normal, and texture coordinates
- **Indices**: Triangle-based indexing for efficiency
- **Materials**: Basic material support (future enhancement)

### AudioClip Resource

Audio clip resource integrated with AudioEngine.

```cpp
class AudioClip : public Resource {
public:
    AudioClip(const std::string& path);
    ~AudioClip();

    // Properties (from AudioEngine integration)
    AudioFormat format;
    float duration = 0.0f;
    int sampleRate = 44100;
    int channels = 2;
    bool is3D = true;

#ifdef GAMEENGINE_HAS_OPENAL
    ALuint bufferId = 0;
#endif

    // Memory Management
    size_t GetMemoryUsage() const override;
};
```

**Supported Audio Formats:**

- **WAV**: PCM audio with custom parser
- **OGG**: Ogg Vorbis with STB integration
- **MP3**: Not supported (patent concerns)

**Usage Examples:**

```cpp
// Basic resource loading with automatic fallbacks
auto* resourceManager = engine.GetResourceManager();

// Load textures (supports PNG, JPG, TGA)
auto wallTexture = resourceManager->Load<Texture>("assets/textures/wall.png");
auto playerTexture = resourceManager->Load<Texture>("assets/textures/player.jpg");
auto earthTexture = resourceManager->Load<Texture>("assets/textures/earth.tga");
auto cowTexture = resourceManager->Load<Texture>("assets/textures/cow.png")

// Load meshes (supports OBJ format)
auto cubeMesh = resourceManager->Load<Mesh>("assets/meshes/cube.obj");
auto characterMesh = resourceManager->Load<Mesh>("assets/meshes/character.obj");

// Resource validation and fallback handling
if (wallTexture && wallTexture->IsValid()) {
    LOG_INFO("Texture loaded: " + std::to_string(wallTexture->GetWidth()) + "x" +
             std::to_string(wallTexture->GetHeight()));
} else {
    LOG_INFO("Using fallback texture (pink/magenta)");
}

// Memory management and optimization
resourceManager->SetMemoryPressureThreshold(256 * 1024 * 1024); // 256 MB
resourceManager->CheckMemoryPressure(); // Manual check
resourceManager->UnloadLeastRecentlyUsed(50 * 1024 * 1024); // Free 50 MB

// Resource caching demonstration
auto texture1 = resourceManager->Load<Texture>("assets/textures/wall.png");
auto texture2 = resourceManager->Load<Texture>("assets/textures/wall.png");
// texture1.get() == texture2.get() (same instance due to caching)

// Statistics and debugging
size_t totalMemory = resourceManager->GetMemoryUsage();
size_t resourceCount = resourceManager->GetResourceCount();
ResourceStats stats = resourceManager->GetResourceStats();

LOG_INFO("Resource Statistics:");
LOG_INFO("  Total resources: " + std::to_string(stats.totalResources));
LOG_INFO("  Total memory: " + std::to_string(stats.totalMemoryUsage / 1024) + " KB");
LOG_INFO("  Cache hit ratio: " + std::to_string(
    (float)stats.cacheHits / (stats.cacheHits + stats.cacheMisses) * 100) + "%");

// Detailed logging
resourceManager->LogResourceUsage();
resourceManager->LogDetailedResourceInfo();

// Resource type breakdown
for (const auto& pair : stats.resourcesByType) {
    LOG_INFO("  " + pair.first + ": " + std::to_string(pair.second) + " resources");
}

for (const auto& pair : stats.memoryByType) {
    LOG_INFO("  " + pair.first + ": " + std::to_string(pair.second / 1024) + " KB");
}

// Advanced usage examples
// Load multiple texture formats
auto wallPng = resourceManager->Load<Texture>("assets/textures/wall.png");
auto wallJpg = resourceManager->Load<Texture>("assets/textures/wall.jpg");
auto earthTga = resourceManager->Load<Texture>("assets/textures/earth.tga");

// Load various mesh complexities
auto cubeMesh = resourceManager->Load<Mesh>("assets/meshes/cube.obj");
auto teapotMesh = resourceManager->Load<Mesh>("assets/meshes/teapot.obj");
auto characterMesh = resourceManager->Load<Mesh>("assets/meshes/character.obj");

// Resource caching demonstration
auto texture1 = resourceManager->Load<Texture>("assets/textures/wall.png");
auto texture2 = resourceManager->Load<Texture>("assets/textures/wall.png");
// texture1.get() == texture2.get() (same instance due to caching)

// Error handling with fallback resources
auto missingTexture = resourceManager->Load<Texture>("nonexistent.png");
// Returns pink/magenta fallback texture instead of nullptr

// Performance optimization
resourceManager->EnableMemoryPooling(true);
resourceManager->EnableLRUCache(true);
resourceManager->EnableGPUUploadOptimization(true);
resourceManager->SetMemoryPoolSize(64 * 1024 * 1024); // 64 MB pool
resourceManager->SetLRUCacheSize(1000, 128 * 1024 * 1024); // 1000 items, 128 MB max

// Performance monitoring
float cacheHitRatio = resourceManager->GetLRUCacheHitRatio();
float poolUtilization = resourceManager->GetMemoryPoolUtilization();
size_t uploadQueueSize = resourceManager->GetGPUUploadQueueSize();

LOG_INFO("Resource Performance:");
LOG_INFO("  Cache hit ratio: " + std::to_string(cacheHitRatio * 100) + "%");
LOG_INFO("  Memory pool utilization: " + std::to_string(poolUtilization * 100) + "%");
LOG_INFO("  GPU upload queue: " + std::to_string(uploadQueueSize) + " items");

// Resource statistics analysis
ResourceStats stats = resourceManager->GetResourceStats();
LOG_INFO("Resource Statistics:");
LOG_INFO("  Total resources: " + std::to_string(stats.totalResources));
LOG_INFO("  Total memory: " + std::to_string(stats.totalMemoryUsage / 1024 / 1024) + " MB");
LOG_INFO("  Cache hits: " + std::to_string(stats.cacheHits));
LOG_INFO("  Cache misses: " + std::to_string(stats.cacheMisses));

// Memory management strategies
if (stats.totalMemoryUsage > 256 * 1024 * 1024) { // Over 256 MB
    resourceManager->UnloadUnused(); // Remove unused resources first
    resourceManager->UnloadLeastRecentlyUsed(64 * 1024 * 1024); // Free 64 MB
}

// Cleanup
resourceManager->UnloadUnused(); // Remove unused resources
resourceManager->UnloadAll();    // Remove all resources

// Cleanup
resourceManager->UnloadUnused(); // Remove unused resources
resourceManager->UnloadAll();    // Remove all resources
```

## 🧮 Math Library

### Core Math Types

```cpp
namespace Math {
    // Vector Types
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Vec4 = glm::vec4;

    // Matrix Types
    using Mat3 = glm::mat3;
    using Mat4 = glm::mat4;

    // Quaternion
    using Quat = glm::quat;

    // Utility Functions
    float ToRadians(float degrees);
    float ToDegrees(float radians);
    float Clamp(float value, float min, float max);
    float Lerp(float a, float b, float t);
    Vec3 Lerp(const Vec3& a, const Vec3& b, float t);

    // Matrix Creation
    Mat4 CreatePerspectiveMatrix(float fov, float aspect, float nearPlane, float farPlane);
    Mat4 CreateOrthographicMatrix(float left, float right, float bottom, float top,
                                 float nearPlane, float farPlane);
    Mat4 CreateLookAtMatrix(const Vec3& eye, const Vec3& center, const Vec3& up);
    Mat4 CreateTransformMatrix(const Vec3& position, const Quat& rotation, const Vec3& scale);
}
```

## 🔧 Utility Classes

### OpenGLContext

Utility class for OpenGL context management and checking, essential for testing and resource creation. Critical for handling headless testing environments and ensuring robust resource creation.

**Related Documentation**: See [OpenGL Context Limitations in Testing](testing-opengl-limitations.md) for comprehensive testing patterns.

```cpp
class OpenGLContext {
public:
    // Context Detection
    static bool HasActiveContext();
    static bool IsReady();
    static const char* GetVersionString();
};
```

**Related Testing Documentation:**

- **[OpenGL Context Limitations](testing-opengl-limitations.md)**: Comprehensive guide for testing without OpenGL context
- **[Resource Testing Patterns](testing-resource-patterns.md)**: Testing ResourceManager functionality
- **[Mock Resource Implementation](testing-mock-resources.md)**: Mock resources for context-free testing

**Usage Examples:**

```cpp
// Context-aware resource creation
if (OpenGLContext::HasActiveContext()) {
    auto texture = std::make_shared<Texture>("player.png");
    texture->LoadFromFile("assets/textures/player.png");
} else {
    // Use mock resource or skip OpenGL-dependent operations
    LOG_WARNING("No OpenGL context available, using fallback");
    auto mockTexture = std::make_shared<MockTexture>("player.png");
}

// Testing with context awareness
bool TestTextureLoading() {
    TestOutput::PrintTestStart("texture loading");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("texture loading");
        return true;
    }

    // Perform actual OpenGL texture test
    auto texture = resourceManager->Load<Texture>("test.png");
    EXPECT_NOT_NULL(texture);

    TestOutput::PrintTestPass("texture loading");
    return true;
}

// Debug information
void PrintOpenGLInfo() {
    if (OpenGLContext::IsReady()) {
        LOG_INFO("OpenGL Version: " + std::string(OpenGLContext::GetVersionString()));
    } else {
        LOG_WARNING("OpenGL context not available");
    }
}

// Safe resource initialization pattern
template<typename ResourceType>
std::shared_ptr<ResourceType> SafeCreateResource(const std::string& path) {
    if (OpenGLContext::HasActiveContext()) {
        return std::make_shared<ResourceType>(path);
    } else {
        // Return mock or null based on testing requirements
        return std::make_shared<MockResource<ResourceType>>(path);
    }
}
```

### Logger

Comprehensive logging system with multiple output targets.

```cpp
class Logger {
public:
    enum class Level { Debug, Info, Warning, Error, Critical };

    // Singleton Access
    static Logger& GetInstance();

    // Lifecycle
    bool Initialize();
    void Shutdown();

    // Logging Methods
    void Log(Level level, const std::string& message);
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);
    void Critical(const std::string& message);

    // Configuration
    void SetLevel(Level minLevel);
    void SetOutputFile(const std::string& filepath);
    void SetConsoleOutput(bool enabled);
};

// Convenience Macros
#define LOG_DEBUG(msg)    Logger::GetInstance().Debug(msg)
#define LOG_INFO(msg)     Logger::GetInstance().Info(msg)
#define LOG_WARNING(msg)  Logger::GetInstance().Warning(msg)
#define LOG_ERROR(msg)    Logger::GetInstance().Error(msg)
#define LOG_CRITICAL(msg) Logger::GetInstance().Critical(msg)
```

## 📝 Usage Examples

### Basic Game Setup

```cpp
#include "Core/Engine.h"
#include "Game/Character.h"
#include "Game/ThirdPersonCameraSystem.h"

int main() {
    Engine engine;
    if (!engine.Initialize()) {
        return -1;
    }

    // Create game objects
    auto character = std::make_unique<Character>();
    character->Initialize();

    auto camera = std::make_unique<ThirdPersonCameraSystem>();
    camera->SetTarget(character.get());

    engine.GetRenderer()->SetCamera(camera.get());

    // Game loop
    engine.SetUpdateCallback([&](float deltaTime) {
        character->Update(deltaTime, engine.GetInput(), camera.get());
        camera->Update(deltaTime, engine.GetInput());
    });

    engine.Run();
    return 0;
}
```

### Physics Integration

```cpp
// Create physics world
auto* physics = engine.GetPhysics();
physics->SetGravity(Math::Vec3(0.0f, -9.81f, 0.0f));

// Create a falling box
RigidBodyDesc boxDesc;
boxDesc.position = Math::Vec3(0.0f, 10.0f, 0.0f);
boxDesc.mass = 1.0f;

uint32_t boxId = physics->CreateRigidBody(boxDesc);

// Apply force
physics->ApplyForce(boxId, Math::Vec3(100.0f, 0.0f, 0.0f));
```

### Complete Audio and Resource Integration

```cpp
#include "Core/Engine.h"
#include "Game/Character.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"

class GameApplication {
private:
    Engine m_engine;
    std::unique_ptr<Character> m_character;
    std::unique_ptr<ThirdPersonCameraSystem> m_camera;
    std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;

    // Audio resources
    std::shared_ptr<AudioClip> m_jumpSound;
    std::shared_ptr<AudioClip> m_backgroundMusic;
    uint32_t m_musicSourceId = 0;

    // Visual resources
    std::shared_ptr<Texture> m_wallTexture;
    std::shared_ptr<Mesh> m_characterMesh;

public:
    bool Initialize() {
        // Initialize engine
        if (!m_engine.Initialize()) {
            return false;
        }

        // Initialize systems
        m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
        m_primitiveRenderer->Initialize();

        // Load resources
        auto* resourceManager = m_engine.GetResourceManager();
        m_jumpSound = resourceManager->Load<AudioClip>("assets/audio/jump.wav");
        m_backgroundMusic = resourceManager->Load<AudioClip>("assets/audio/music.ogg");
        m_wallTexture = resourceManager->Load<Texture>("assets/textures/wall.png");
        m_characterMesh = resourceManager->Load<Mesh>("assets/meshes/character.obj");

        // Setup audio
        auto* audioEngine = m_engine.GetAudio();
        if (audioEngine && audioEngine->IsAudioAvailable()) {
            // Create background music source
            m_musicSourceId = audioEngine->CreateAudioSource();
            audioEngine->SetAudioSourceLooping(m_musicSourceId, true);
            audioEngine->SetAudioSourceVolume(m_musicSourceId, 0.3f);
            audioEngine->PlayAudioSource(m_musicSourceId, m_backgroundMusic);

            LOG_INFO("Audio system initialized with background music");
        }

        // Setup character without audio dependencies
        m_character = std::make_unique<Character>();
        m_character->Initialize(m_engine.GetPhysics());

        // Setup camera
        m_camera = std::make_unique<ThirdPersonCameraSystem>();
        m_camera->SetTarget(m_character.get());
        m_engine.GetRenderer()->SetCamera(m_camera.get());

        return true;
    }

    void Update(float deltaTime) {
        // Update game objects
        m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
        m_camera->Update(deltaTime, m_engine.GetInput());

        // Update 3D audio listener
        auto* audioEngine = m_engine.GetAudio();
        if (audioEngine && audioEngine->IsAudioAvailable()) {
            Math::Vec3 characterPos = m_character->GetPosition();
            Math::Vec3 cameraForward = m_camera->GetForward();
            Math::Vec3 cameraUp = m_camera->GetUp();

            audioEngine->SetListenerPosition(characterPos);
            audioEngine->SetListenerOrientation(cameraForward, cameraUp);
        }

        // Handle input for audio testing
        auto* input = m_engine.GetInput();
        if (input->IsKeyPressed(KeyCode::F3) && m_jumpSound) {
            // Play jump sound at character position
            uint32_t sourceId = audioEngine->CreateAudioSource();
            audioEngine->SetAudioSourcePosition(sourceId, m_character->GetPosition());
            audioEngine->PlayAudioSource(sourceId, m_jumpSound);
        }
    }

    void Render() {
        // Set view-projection matrix
        Math::Mat4 viewProjection = m_camera->GetViewProjectionMatrix();
        m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);

        // Render character with loaded mesh and texture
        if (m_characterMesh && m_wallTexture) {
            m_primitiveRenderer->DrawMesh(
                m_characterMesh,
                m_character->GetPosition(),
                Math::Vec3(1.0f, 1.0f, 1.0f),
                m_wallTexture
            );
        } else {
            // Fallback to primitive rendering
            m_character->Render(m_primitiveRenderer.get());
        }

        // Render environment with loaded textures
        if (m_wallTexture && m_wallTexture->IsValid()) {
            m_primitiveRenderer->DrawCube(
                Math::Vec3(5.0f, 1.0f, 0.0f),
                Math::Vec3(2.0f, 2.0f, 2.0f),
                m_wallTexture
            );
        }
    }

    void Run() {
        m_engine.SetUpdateCallback([this](float deltaTime) { Update(deltaTime); });
        m_engine.SetRenderCallback([this]() { Render(); });
        m_engine.Run();
    }

    ~GameApplication() {
        // Cleanup audio sources
        auto* audioEngine = m_engine.GetAudio();
        if (audioEngine && m_musicSourceId != 0) {
            audioEngine->DestroyAudioSource(m_musicSourceId);
        }
    }
};

int main() {
    GameApplication app;
    if (!app.Initialize()) {
        LOG_CRITICAL("Failed to initialize application");
        return -1;
    }

    app.Run();
    return 0;
}
```

### Audio Setup

```cpp
// Initialize audio
auto* audio = engine.GetAudio();

// Load sound
uint32_t jumpSound = audio->LoadAudioClip("assets/sounds/jump.wav");

// Create 3D audio source
uint32_t source = audio->CreateAudioSource();
audio->SetAudioSourcePosition(source, Math::Vec3(0.0f, 0.0f, 0.0f));

// Play sound
audio->PlayAudioSource(source, jumpSound);
```

### Movement Component System Usage

```cpp
// Create character with default deterministic movement
auto character = std::make_unique<Character>();
character->Initialize(engine.GetPhysics());

// Switch to hybrid movement for better collision detection
character->SwitchToHybridMovement();

// Configure movement parameters
auto config = character->GetMovementComponent()->GetMovementConfig();
config.maxWalkSpeed = 8.0f;
config.jumpZVelocity = 12.0f;
config.maxStepHeight = 0.4f;
character->GetMovementComponent()->SetMovementConfig(config);

// Runtime movement type switching based on game state
if (player->IsInVehicle()) {
    player->SwitchToPhysicsMovement();  // Full physics for vehicles
} else if (player->IsInPrecisionMode()) {
    player->SwitchToDeterministicMovement();  // Precise platforming
} else {
    player->SwitchToHybridMovement();  // Best balance
}

// Check current movement type
LOG_INFO("Character using: " + std::string(character->GetMovementTypeName()));
```

### Custom Movement Component

```cpp
class CustomMovementComponent : public CharacterMovementComponent {
public:
    bool Initialize(PhysicsEngine* physicsEngine) override {
        m_physicsEngine = physicsEngine;
        return true;
    }

    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) override {
        // Custom movement logic here
        HandleCustomMovement(deltaTime, input, camera);
    }

    const char* GetComponentTypeName() const override {
        return "CustomMovementComponent";
    }

    // Implement all pure virtual methods...
};

// Use custom component
auto customComponent = std::make_unique<CustomMovementComponent>();
character->SetMovementComponent(std::move(customComponent));
```

### Movement Component Factory Usage

```cpp
// Create different movement types using factory
auto deterministicComponent = MovementComponentFactory::CreateComponent(
    MovementComponentFactory::ComponentType::Deterministic);

auto hybridComponent = MovementComponentFactory::CreateComponent(
    MovementComponentFactory::ComponentType::Hybrid);

auto physicsComponent = MovementComponentFactory::CreateComponent(
    MovementComponentFactory::ComponentType::Physics);

// Use configuration presets
auto platformingConfig = MovementComponentFactory::GetPlatformingConfig();
auto realisticConfig = MovementComponentFactory::GetRealisticConfig();
auto arcadeConfig = MovementComponentFactory::GetArcadeConfig();

deterministicComponent->SetMovementConfig(platformingConfig);
```

## 🧪 Testing Framework

Comprehensive testing framework with standardized output formatting, OpenGL context awareness, and mock resource support for robust testing across all environments.

**Related Documentation**:

- [Testing Guide](testing-guide.md) - Complete testing instructions and examples
- [Test Output Formatting Standards](testing-output-formatting.md) - Detailed formatting requirements
- [OpenGL Context Limitations](testing-opengl-limitations.md) - Context-aware testing patterns
- [Resource Testing Patterns](testing-resource-patterns.md) - Resource management testing
- [Mock Resource Implementation](testing-mock-resources.md) - Mock resource creation and usage

### TestOutput

Standardized test output formatting utilities for consistent test reporting across the engine. All tests MUST use these methods instead of direct console output.

```cpp
class TestOutput {
public:
    // Test Structure
    static void PrintHeader(const std::string& testSuiteName);
    static void PrintFooter(bool allPassed);

    // Test Lifecycle
    static void PrintTestStart(const std::string& testName);
    static void PrintTestPass(const std::string& testName);
    static void PrintTestFail(const std::string& testName);
    static void PrintTestFail(const std::string& testName, const std::string& expected,
                             const std::string& actual);

    // Information and Diagnostics
    static void PrintInfo(const std::string& message);
    static void PrintWarning(const std::string& message);
    static void PrintError(const std::string& message);
    static void PrintTiming(const std::string& operation, double timeMs, int iterations = 1);
};
```

### FloatComparison

Floating-point comparison utilities with configurable epsilon tolerance for mathematical testing.

```cpp
class FloatComparison {
public:
    static constexpr float DEFAULT_EPSILON = 0.001f;
    static constexpr float LOOSE_EPSILON = 0.01f;
    static constexpr float TIGHT_EPSILON = 0.0001f;

    // Basic Comparisons
    static bool IsNearlyEqual(float a, float b, float epsilon = DEFAULT_EPSILON);
    static bool IsNearlyZero(float value, float epsilon = DEFAULT_EPSILON);

    // Vector Comparisons
    static bool IsNearlyEqual(const Math::Vec3& a, const Math::Vec3& b, float epsilon = DEFAULT_EPSILON);
    static bool IsNearlyEqual(const Math::Vec4& a, const Math::Vec4& b, float epsilon = DEFAULT_EPSILON);
    static bool IsNearlyEqual(const Math::Quat& a, const Math::Quat& b, float epsilon = DEFAULT_EPSILON);
    static bool IsNearlyEqual(const Math::Mat4& a, const Math::Mat4& b, float epsilon = DEFAULT_EPSILON);
    static bool IsNearlyZero(const Math::Vec3& vec, float epsilon = DEFAULT_EPSILON);
};
```

### TestTimer

High-precision timing utility for performance testing and benchmarking.

```cpp
class TestTimer {
public:
    TestTimer();

    // Time Measurement
    double ElapsedMs() const;      // Milliseconds
    long long ElapsedUs() const;   // Microseconds
    long long ElapsedNs() const;   // Nanoseconds
    void Restart();
};
```

### PerformanceTest

Performance testing utilities with threshold validation and statistical analysis.

```cpp
class PerformanceTest {
public:
    // Performance Measurement
    template<typename Func>
    static double MeasureAverageTime(Func&& func, int iterations = 1000);

    // Performance Validation
    template<typename Func>
    static bool ValidatePerformance(const std::string& testName, Func&& func,
                                   double thresholdMs, int iterations = 1000);
};
```

### TestSuite

Test result tracking and management for organized test execution.

```cpp
struct TestResult {
    std::string testName;
    bool passed;
    std::string errorMessage;
    double executionTimeMs;
};

class TestSuite {
public:
    TestSuite(const std::string& suiteName);

    // Test Execution
    template<typename TestFunc>
    bool RunTest(const std::string& testName, TestFunc&& testFunc);

    // Results
    void PrintSummary() const;
    bool AllTestsPassed() const;
};
```

### Mock Resource System

Base classes and patterns for creating mock resources for testing without OpenGL context. Essential for unit testing and headless CI/CD environments.

**Related Documentation**: See [Mock Resource Implementation Guide](testing-mock-resources.md) for comprehensive patterns and examples.

```cpp
// Base Mock Resource Pattern
template<typename ResourceType>
class MockResource : public ResourceType {
public:
    MockResource(const std::string& path) : ResourceType(path), m_simulateError(false) {}

    // Error Simulation
    void SimulateLoadError(bool shouldFail) { m_simulateError = shouldFail; }
    void SimulateMemoryUsage(size_t bytes) { m_mockMemoryUsage = bytes; }

    // Overridden Methods
    bool LoadFromFile(const std::string& filepath) override {
        if (m_simulateError) {
            throw std::runtime_error("Simulated load error");
        }
        m_loaded = true;
        return true;
    }

    size_t GetMemoryUsage() const override {
        return m_mockMemoryUsage > 0 ? m_mockMemoryUsage : ResourceType::GetMemoryUsage();
    }

    void CreateDefault() override {
        m_loaded = true;
        // Create minimal default resource data
    }

protected:
    bool m_simulateError;
    bool m_loaded = false;
    size_t m_mockMemoryUsage = 0;
};

// Specialized Mock Resources
class MockTexture : public MockResource<Texture> {
public:
    MockTexture(const std::string& path) : MockResource<Texture>(path) {
        m_width = 256;
        m_height = 256;
        m_channels = 4;
    }

    void SetDimensions(int width, int height, int channels = 4) {
        m_width = width;
        m_height = height;
        m_channels = channels;
        m_mockMemoryUsage = width * height * channels;
    }

private:
    int m_width, m_height, m_channels;
};

class MockMesh : public MockResource<Mesh> {
public:
    MockMesh(const std::string& path) : MockResource<Mesh>(path) {
        m_vertexCount = 8;  // Default cube
        m_indexCount = 36;
    }

    void SetGeometry(size_t vertexCount, size_t indexCount) {
        m_vertexCount = vertexCount;
        m_indexCount = indexCount;
        m_mockMemoryUsage = (vertexCount * sizeof(float) * 8) + (indexCount * sizeof(unsigned int));
    }

private:
    size_t m_vertexCount, m_indexCount;
};

class MockAudioClip : public MockResource<AudioClip> {
public:
    MockAudioClip(const std::string& path) : MockResource<AudioClip>(path) {
        m_duration = 1.0f;  // 1 second default
        m_sampleRate = 44100;
        m_channels = 2;
    }

    void SetAudioProperties(float duration, int sampleRate = 44100, int channels = 2) {
        m_duration = duration;
        m_sampleRate = sampleRate;
        m_channels = channels;
        m_mockMemoryUsage = static_cast<size_t>(duration * sampleRate * channels * sizeof(float));
    }

private:
    float m_duration;
    int m_sampleRate, m_channels;
};
```

### Testing Assertion Macros

Enhanced assertion macros with detailed error reporting and context information.

```cpp
// Basic Assertions
#define EXPECT_TRUE(condition)
#define EXPECT_FALSE(condition)
#define EXPECT_NULL(ptr)
#define EXPECT_NOT_NULL(ptr)
#define EXPECT_EQUAL(a, b)
#define EXPECT_NOT_EQUAL(a, b)

// Floating-Point Assertions
#define EXPECT_NEARLY_EQUAL(a, b)
#define EXPECT_NEARLY_EQUAL_EPSILON(a, b, epsilon)
#define EXPECT_NEARLY_ZERO(value)

// Vector and Matrix Assertions
#define EXPECT_NEAR_VEC3(a, b)
#define EXPECT_NEAR_VEC3_EPSILON(a, b, epsilon)
#define EXPECT_NEAR_VEC4(a, b)
#define EXPECT_NEAR_VEC4_EPSILON(a, b, epsilon)
#define EXPECT_NEAR_QUAT(a, b)
#define EXPECT_NEAR_QUAT_EPSILON(a, b, epsilon)
#define EXPECT_MATRIX_EQUAL(a, b)
#define EXPECT_MATRIX_EQUAL_EPSILON(a, b, epsilon)

// Range and String Assertions
#define EXPECT_IN_RANGE(value, min, max)
#define EXPECT_STRING_EQUAL(a, b)
```

**Testing Framework Usage Examples:**

```cpp
// Basic test structure
bool TestResourceManager() {
    TestOutput::PrintTestStart("ResourceManager functionality");

    auto resourceManager = std::make_unique<ResourceManager>();
    EXPECT_TRUE(resourceManager->Initialize());

    // Test with mock resources if no OpenGL context
    if (!OpenGLContext::HasActiveContext()) {
        auto mockTexture = std::make_shared<MockTexture>("test.png");
        mockTexture->SetDimensions(512, 512, 4);
        EXPECT_EQUAL(mockTexture->GetMemoryUsage(), 512 * 512 * 4);
    }

    TestOutput::PrintTestPass("ResourceManager functionality");
    return true;
}

// Performance testing
bool TestResourceLoadingPerformance() {
    return PerformanceTest::ValidatePerformance(
        "resource loading",
        []() {
            auto texture = resourceManager->Load<Texture>("test.png");
            // Use texture...
        },
        5.0, // 5ms threshold
        100  // 100 iterations
    );
}

// Test suite usage
int main() {
    TestSuite suite("Resource Management Tests");

    suite.RunTest("ResourceManager Basic", TestResourceManager);
    suite.RunTest("Resource Loading Performance", TestResourceLoadingPerformance);
    suite.RunTest("Memory Management", TestMemoryManagement);

    suite.PrintSummary();
    return suite.AllTestsPassed() ? 0 : 1;
}

// Mock resource integration with ResourceManager
template<typename T>
std::shared_ptr<T> CreateTestResource(const std::string& path) {
    if (OpenGLContext::HasActiveContext()) {
        return resourceManager->Load<T>(path);
    } else {
        // Use appropriate mock resource
        if constexpr (std::is_same_v<T, Texture>) {
            return std::make_shared<MockTexture>(path);
        } else if constexpr (std::is_same_v<T, Mesh>) {
            return std::make_shared<MockMesh>(path);
        } else if constexpr (std::is_same_v<T, AudioClip>) {
            return std::make_shared<MockAudioClip>(path);
        }
        return nullptr;
    }
}
```

---

## 📚 Testing Documentation Cross-References

This API reference works in conjunction with comprehensive testing documentation:

### Core Testing Documentation

- **[Testing Guide](testing-guide.md)** - Complete testing instructions, examples, and best practices
- **[Testing Guidelines](testing-guidelines.md)** - High-level guidelines for writing effective tests
- **[Testing Standards](testing-standards.md)** - Coding standards and conventions for test code

### Specialized Testing Topics

- **[OpenGL Context Limitations](testing-opengl-limitations.md)** - Comprehensive guide for handling OpenGL context issues in testing environments
- **[Resource Testing Patterns](testing-resource-patterns.md)** - Best practices for testing ResourceManager functionality and resource operations
- **[Mock Resource Implementation](testing-mock-resources.md)** - Detailed patterns for creating and using mock resources in tests

### Output and Consistency

- **[Test Output Formatting Standards](testing-output-formatting.md)** - Complete standards for consistent test output formatting
- **[Test Output Consistency Guidelines](testing-output-consistency-guide.md)** - Guidelines for maintaining consistency across different test types

### Key Integration Points

1. **ResourceManager API** integrates with [Resource Testing Patterns](testing-resource-patterns.md) for comprehensive resource testing
2. **OpenGLContext utilities** work with [OpenGL Context Limitations](testing-opengl-limitations.md) patterns for context-aware testing
3. **TestOutput methods** follow [Test Output Formatting Standards](testing-output-formatting.md) for consistent reporting
4. **Mock Resource System** implements patterns from [Mock Resource Implementation Guide](testing-mock-resources.md)

---

This API reference provides comprehensive coverage of all public interfaces in Game Engine Kiro, including the enhanced resource management system with memory tracking and debugging capabilities, OpenGL context utilities for safe resource creation, and a complete testing framework with mock resource support for context-independent testing. For implementation details and advanced usage patterns, refer to the testing documentation above and the examples in the `examples/` directory.
