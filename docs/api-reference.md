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

3D spatial audio system with OpenAL backend.

```cpp
class AudioEngine {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();
    void Update();

    // Audio Clips
    uint32_t LoadAudioClip(const std::string& filepath);
    void UnloadAudioClip(uint32_t clipId);

    // Audio Sources
    uint32_t CreateAudioSource();
    void DestroyAudioSource(uint32_t sourceId);

    // Playback Control
    void PlayAudioSource(uint32_t sourceId, uint32_t clipId);
    void StopAudioSource(uint32_t sourceId);
    void PauseAudioSource(uint32_t sourceId);
    void ResumeAudioSource(uint32_t sourceId);

    // 3D Audio Properties
    void SetAudioSourcePosition(uint32_t sourceId, const Math::Vec3& position);
    void SetAudioSourceVelocity(uint32_t sourceId, const Math::Vec3& velocity);
    void SetAudioSourceVolume(uint32_t sourceId, float volume);
    void SetAudioSourcePitch(uint32_t sourceId, float pitch);
    void SetAudioSourceLooping(uint32_t sourceId, bool looping);

    // Listener (Camera) Properties
    void SetListenerPosition(const Math::Vec3& position);
    void SetListenerVelocity(const Math::Vec3& velocity);
    void SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up);

    // Global Settings
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
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

Template-based resource management with automatic caching and lifetime management.

```cpp
template<typename T>
class ResourceManager {
public:
    // Lifecycle
    bool Initialize();
    void Shutdown();

    // Resource Loading
    std::shared_ptr<T> Load(const std::string& filepath);
    void Unload(const std::string& filepath);
    void UnloadAll();

    // Cache Management
    void SetCacheSize(size_t maxSize);
    size_t GetCacheSize() const;
    void ClearCache();

    // Statistics
    size_t GetLoadedResourceCount() const;
    size_t GetMemoryUsage() const;
};

// Specialized resource managers
using TextureManager = ResourceManager<Texture>;
using MeshManager = ResourceManager<Mesh>;
using AudioClipManager = ResourceManager<AudioClip>;
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

---

This API reference provides comprehensive coverage of all public interfaces in Game Engine Kiro, including the new component-based movement system that enables deterministic character physics and hybrid collision detection. For implementation details and advanced usage patterns, refer to the examples in the `examples/` directory and the architecture documentation.
