# Architecture Overview

Game Engine Kiro follows a modular, component-based architecture designed for flexibility, maintainability, and performance. This document outlines the core design principles and system interactions.

## 🏗️ Core Design Principles

### 1. Modular Architecture

- **Separation of Concerns**: Each module has a single, well-defined responsibility
- **Loose Coupling**: Modules communicate through well-defined interfaces
- **High Cohesion**: Related functionality is grouped together
- **Dependency Injection**: Dependencies are provided rather than created

### 2. Modern C++ Practices

- **RAII**: Resource Acquisition Is Initialization for automatic cleanup
- **Smart Pointers**: Automatic memory management with std::unique_ptr and std::shared_ptr
- **Move Semantics**: Efficient resource transfer with C++11 move operations
- **Template Metaprogramming**: Compile-time optimizations where appropriate

### 3. Performance-First Design

- **Cache-Friendly Data Structures**: Contiguous memory layouts where possible
- **Minimal Allocations**: Object pooling and pre-allocation strategies
- **SIMD Optimization**: Vectorized math operations using GLM
- **Multi-threading Ready**: Thread-safe designs for future parallelization

## 🎯 System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        Game Layer                           │
├─────────────────────────────────────────────────────────────┤
│  Character  │  Camera   │  Game Logic  │  Scene Management  │
├─────────────────────────────────────────────────────────────┤
│                      Engine Core                            │
├─────────────────────────────────────────────────────────────┤
│ Graphics │ Physics │ Audio │ Input │ Resource │ Scripting   │
├─────────────────────────────────────────────────────────────┤
│                    Foundation Layer                         │
├─────────────────────────────────────────────────────────────┤
│   Math   │  Logger  │ Memory │ Time  │  Events │   Utils    │
├─────────────────────────────────────────────────────────────┤
│                   Platform Layer                            │
└─────────────────────────────────────────────────────────────┘
│  OpenGL  │  GLFW   │ OpenAL │ Bullet │  vcpkg  │    OS      │
└─────────────────────────────────────────────────────────────┘
```

## 🔧 Core Systems

### Engine Core

The central orchestrator that manages all subsystems:

```cpp
class Engine {
public:
    bool Initialize();
    void Run();
    void Shutdown();

    // Subsystem access
    GraphicsRenderer* GetRenderer() const;
    InputManager* GetInput() const;
    AudioEngine* GetAudio() const;
    PhysicsEngine* GetPhysics() const;
    ResourceManager* GetResourceManager() const;

private:
    std::unique_ptr<GraphicsRenderer> m_renderer;
    std::unique_ptr<InputManager> m_input;
    std::unique_ptr<AudioEngine> m_audio;
    std::unique_ptr<PhysicsEngine> m_physics;
    std::unique_ptr<ResourceManager> m_resourceManager;
};
```

### Graphics System

Handles all rendering operations with a modern OpenGL pipeline:

```cpp
class GraphicsRenderer {
public:
    virtual bool Initialize(const RenderSettings& settings) = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void SetCamera(const Camera* camera) = 0;
    virtual void DrawMesh(const Mesh* mesh, const Material* material,
                         const Math::Mat4& transform) = 0;
};
```

**Key Components:**

- **OpenGLRenderer**: Concrete implementation using OpenGL 4.6+
- **Camera System**: Flexible camera management with specialized third-person camera
- **Shader Management**: Automatic compilation and hot-reloading
- **Primitive Renderer**: Built-in shapes for rapid prototyping

### Physics System

Hybrid physics architecture with component-based movement system:

```cpp
class PhysicsEngine {
public:
    bool Initialize(const PhysicsConfiguration& config = {});
    void Update(float deltaTime);
    uint32_t CreateRigidBody(const RigidBody& bodyDesc, const CollisionShape& shape);
    RaycastHit Raycast(const Math::Vec3& origin, const Math::Vec3& direction, float maxDistance);
    SweepHit SweepCapsule(const Math::Vec3& from, const Math::Vec3& to, float radius, float height);
    uint32_t CreateGhostObject(const CollisionShape& shape, const Math::Vec3& position);

private:
    std::shared_ptr<PhysicsWorld> m_activeWorld;
    std::unordered_map<uint32_t, btRigidBody*> m_bulletBodies;
    std::unordered_map<uint32_t, btGhostObject*> m_bulletGhostObjects;
};
```

**Movement Component System:**

- **DeterministicMovementComponent**: Precise character control without physics simulation
- **HybridMovementComponent**: Physics collision detection with direct position control
- **PhysicsMovementComponent**: Full physics simulation for dynamic objects

**Supported Backends:**

- **Bullet Physics**: Open source, highly compatible, deterministic
- **NVIDIA PhysX**: High performance, GPU acceleration (planned)

### Input System

Comprehensive input handling with action mapping:

```cpp
class InputManager {
public:
    bool Initialize(GLFWwindow* window);
    void Update();

    // Direct input queries
    bool IsKeyDown(KeyCode key) const;
    bool IsMouseButtonPressed(MouseButton button) const;
    Math::Vec2 GetMouseDelta() const;

    // Action mapping system
    void BindAction(const std::string& actionName, KeyCode key);
    bool IsActionPressed(const std::string& actionName) const;
};
```

### Audio System

3D spatial audio with OpenAL backend:

```cpp
class AudioEngine {
public:
    bool Initialize();
    uint32_t LoadAudioClip(const std::string& filepath);
    uint32_t CreateAudioSource();
    void PlayAudioSource(uint32_t sourceId, uint32_t clipId);
    void SetAudioSourcePosition(uint32_t sourceId, const Math::Vec3& position);
};
```

### Resource Management

Automatic loading, caching, and lifetime management:

```cpp
template<typename T>
class ResourceManager {
public:
    std::shared_ptr<T> Load(const std::string& filepath);
    void Unload(const std::string& filepath);
    void UnloadAll();

private:
    std::unordered_map<std::string, std::weak_ptr<T>> m_resources;
};
```

## 🎮 Game Layer Architecture

### Character System

Component-based character controller with modular movement system:

```cpp
class Character {
public:
    bool Initialize(PhysicsEngine* physicsEngine = nullptr);
    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera);
    void Render(PrimitiveRenderer* renderer);

    // Movement component management
    void SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component);
    CharacterMovementComponent* GetMovementComponent() const;

    // Convenience methods for switching movement types
    void SwitchToPhysicsMovement();
    void SwitchToDeterministicMovement();
    void SwitchToHybridMovement();

    // Get current movement type information
    const char* GetMovementTypeName() const;
    Math::Vec4 GetMovementTypeColor() const;

private:
    std::unique_ptr<CharacterMovementComponent> m_movementComponent;
    PhysicsEngine* m_physicsEngine;
    float m_height = 1.8f;
    float m_radius = 0.3f;
};
```

**Movement Component Architecture:**

```cpp
class CharacterMovementComponent {
public:
    virtual bool Initialize(PhysicsEngine* physicsEngine) = 0;
    virtual void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) = 0;
    virtual void Shutdown() = 0;

    // Transform interface
    virtual void SetPosition(const Math::Vec3& position) = 0;
    virtual const Math::Vec3& GetPosition() const = 0;
    virtual const Math::Vec3& GetVelocity() const = 0;

    // Movement state
    virtual bool IsGrounded() const = 0;
    virtual bool IsJumping() const = 0;
    virtual bool IsFalling() const = 0;

    // Component identification
    virtual const char* GetComponentTypeName() const = 0;
};
```

### Camera System

Professional third-person camera with SpringArm component:

```cpp
class ThirdPersonCameraSystem : public Camera {
public:
    void Update(float deltaTime, InputManager* input);
    void SetTarget(Character* target);
    Math::Vec3 GetForwardDirection() const;
    Math::Vec3 GetRightDirection() const;

private:
    SpringArm m_springArm;
    Character* m_target;
    float m_mouseSensitivity;
};

class SpringArm {
public:
    void Update(float deltaTime, const Math::Vec3& targetPosition,
                float inputYaw, float inputPitch);
    Math::Vec3 GetCameraPosition() const;
    void SetShoulderOffset(float rightOffset, float upOffset);

private:
    float m_currentYaw, m_currentPitch;
    float m_currentLength;
    float m_shoulderOffsetRight, m_shoulderOffsetUp;
};
```

## 🔄 Data Flow

### Frame Update Cycle

```
1. Input Processing
   ├── GLFW polls events
   ├── InputManager updates key states
   └── Action mappings are evaluated

2. Game Logic Update
   ├── Character processes input
   ├── Physics simulation step
   ├── Audio source updates
   └── Camera follows character

3. Rendering
   ├── Clear framebuffer
   ├── Set camera matrices
   ├── Render game objects
   ├── Render UI (future)
   └── Present frame

4. Resource Management
   ├── Load pending resources
   ├── Unload unused resources
   └── Update resource references
```

### Memory Management Strategy

```cpp
// RAII for automatic cleanup
class ManagedResource {
    std::unique_ptr<GLuint> m_handle;
public:
    ManagedResource() : m_handle(std::make_unique<GLuint>()) {
        glGenBuffers(1, m_handle.get());
    }
    ~ManagedResource() {
        if (m_handle) {
            glDeleteBuffers(1, m_handle.get());
        }
    }
};

// Shared ownership for resources
std::shared_ptr<Texture> texture = resourceManager->Load<Texture>("texture.png");
```

## 🧵 Threading Model

### Current (Single-Threaded)

- **Main Thread**: All systems run on the main thread
- **Synchronous Operations**: Simple and deterministic
- **Easy Debugging**: No race conditions or synchronization issues

### Future (Multi-Threaded)

```
Main Thread:
├── Game Logic
├── Input Processing
└── Rendering Commands

Physics Thread:
├── Physics Simulation
├── Collision Detection
└── Character Controller

Audio Thread:
├── Audio Processing
├── 3D Audio Calculations
└── Audio Streaming

Resource Thread:
├── Asset Loading
├── Texture Decompression
└── Model Processing
```

## 🔌 Extension Points

### Custom Components

```cpp
class CustomGameComponent {
public:
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;
};
```

### Custom Physics Backends

```cpp
class CustomPhysicsBackend : public IPhysicsBackend {
public:
    bool Initialize(const PhysicsConfig& config) override;
    void Update(float deltaTime) override;
    // Implement all required methods...
};
```

### Custom Renderers

```cpp
class VulkanRenderer : public GraphicsRenderer {
public:
    bool Initialize(const RenderSettings& settings) override;
    // Implement Vulkan-specific rendering...
};
```

## 📊 Performance Considerations

### Memory Layout

- **Structure of Arrays (SoA)**: For bulk operations on similar data
- **Array of Structures (AoS)**: For operations on complete objects
- **Cache-Friendly Access Patterns**: Sequential memory access where possible

### Optimization Strategies

- **Object Pooling**: Pre-allocate frequently used objects
- **Batch Operations**: Group similar operations together
- **Lazy Loading**: Load resources only when needed
- **Level-of-Detail**: Reduce complexity based on distance/importance

## 🔮 Future Architecture Plans

### Planned Enhancements

- **Entity-Component-System (ECS)**: For better data-oriented design
- **Job System**: Task-based parallelism for better CPU utilization
- **Vulkan Renderer**: Modern graphics API for better performance
- **Scripting Integration**: Lua scripting for gameplay logic
- **Networking Layer**: Multiplayer support with client-server architecture

### Scalability Considerations

- **Modular Loading**: Load only required engine modules
- **Platform Abstraction**: Support for multiple platforms
- **Renderer Abstraction**: Support for multiple graphics APIs
- **Physics Abstraction**: Support for multiple physics engines

---

This architecture provides a solid foundation for game development while remaining flexible enough to adapt to changing requirements and new technologies. The modular design ensures that individual systems can be improved or replaced without affecting the entire engine.
