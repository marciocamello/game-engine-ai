# Deterministic Physics Implementation

Game Engine Kiro features a revolutionary **component-based movement system** that provides three distinct approaches to character physics, enabling developers to choose the perfect movement solution for each character type.

## 🎯 Overview

Traditional game engines force developers to choose between:

- **Full Physics Simulation**: Realistic but unpredictable, hard to control
- **Manual Movement**: Predictable but lacks realistic collision detection

Game Engine Kiro solves this with a **hybrid approach** that provides the best of both worlds through modular movement components.

## 🧩 Movement Component Architecture

### Base Component Interface

All movement components inherit from `CharacterMovementComponent`:

```cpp
class CharacterMovementComponent {
public:
    // Component lifecycle
    virtual bool Initialize(PhysicsEngine* physicsEngine) = 0;
    virtual void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) = 0;
    virtual void Shutdown() = 0;

    // Transform and movement interface
    virtual const Math::Vec3& GetPosition() const = 0;
    virtual const Math::Vec3& GetVelocity() const = 0;
    virtual bool IsGrounded() const = 0;

    // Component identification
    virtual const char* GetComponentTypeName() const = 0;
};
```

### Runtime Component Switching

Characters can switch between movement types seamlessly:

```cpp
// Switch movement types at runtime
character->SwitchToDeterministicMovement();  // Precise control
character->SwitchToHybridMovement();         // Physics collision + direct control
character->SwitchToPhysicsMovement();        // Full physics simulation

// State is preserved across transitions
LOG_INFO("Now using: " + std::string(character->GetMovementTypeName()));
```

## 🎮 Movement Component Types

### 1. DeterministicMovementComponent ✅

**Perfect for:** Player characters, NPCs, precision platforming, networking

#### Key Features

- **Immediate Response**: No physics simulation delays
- **Predictable Behavior**: Identical results across platforms and runs
- **High Performance**: Minimal CPU overhead
- **Networking Friendly**: Fully deterministic for multiplayer

#### Implementation Highlights

```cpp
class DeterministicMovementComponent : public CharacterMovementComponent {
private:
    // Direct position and velocity control
    Math::Vec3 m_position{0.0f, 0.9f, 0.0f};
    Math::Vec3 m_velocity{0.0f};

    // Manual physics parameters
    float m_gravity = -15.0f;
    float m_acceleration = 25.0f;
    float m_friction = 15.0f;

    void UpdateMovement(float deltaTime) {
        // Apply gravity manually
        if (!m_isGrounded) {
            m_velocity.y += m_gravity * deltaTime;
        }

        // Direct position update
        m_position += m_velocity * deltaTime;

        // Simple ground collision
        CheckGroundCollision();
    }
};
```

#### Use Cases

- **Player Characters**: Precise, responsive control
- **Competitive Games**: Consistent behavior for esports
- **Platformers**: Exact jump distances and timing
- **Multiplayer**: Deterministic for network synchronization

### 2. HybridMovementComponent ✅

**Perfect for:** Advanced character controllers, complex environments

#### Key Features

- **Physics Collision Detection**: Uses Bullet Physics for accurate collision queries
- **Direct Position Control**: Maintains responsive character movement
- **Ghost Objects**: Kinematic collision detection without physics simulation
- **Advanced Features**: Surface sliding, step-up detection, slope handling

#### Implementation Highlights

```cpp
class HybridMovementComponent : public CharacterMovementComponent {
private:
    uint32_t m_ghostObjectId = 0;  // For collision detection

    Math::Vec3 ResolveMovement(const Math::Vec3& desiredMovement) {
        // Use physics engine for collision detection
        CollisionInfo collision = SweepTest(m_position,
                                          m_position + desiredMovement,
                                          m_characterRadius,
                                          m_characterHeight);

        if (!collision.hasCollision) {
            return desiredMovement;  // Move freely
        }

        // Slide along collision surface
        return SlideAlongSurface(desiredMovement, collision.normal);
    }

    bool IsGroundedCheck() const {
        // Use physics raycast for ground detection
        RaycastHit hit = m_physicsEngine->Raycast(m_position,
                                                Math::Vec3(0, -1, 0),
                                                m_groundCheckDistance);
        return hit.hasHit && hit.normal.y > 0.5f;
    }
};
```

#### Advanced Collision Features

- **Sweep Testing**: Continuous collision detection
- **Surface Sliding**: Smooth movement along walls
- **Step-Up Detection**: Automatic stair climbing
- **Slope Limits**: Configurable walkable angles

#### Use Cases

- **Open World Games**: Complex terrain navigation
- **First/Third Person Shooters**: Precise movement with realistic collision
- **Adventure Games**: Exploration with accurate environment interaction

### 3. PhysicsMovementComponent ✅

**Perfect for:** Vehicles, ragdolls, dynamic objects

#### Key Features

- **Full Physics Simulation**: Complete integration with physics engine
- **Force-Based Movement**: Realistic momentum and acceleration
- **Automatic Collision Response**: Physics engine handles all collisions
- **Mass and Inertia**: Realistic physical properties

#### Implementation Highlights

```cpp
class PhysicsMovementComponent : public CharacterMovementComponent {
private:
    uint32_t m_rigidBodyId = 0;

    void ApplyMovementForces(const Math::Vec3& inputDirection) {
        // Apply forces to rigid body instead of direct position updates
        Math::Vec3 force = inputDirection * m_config.maxAcceleration;
        m_physicsEngine->ApplyForce(m_rigidBodyId, force);
    }

    void Jump() {
        if (IsGrounded()) {
            Math::Vec3 jumpImpulse(0, m_config.jumpZVelocity, 0);
            m_physicsEngine->ApplyImpulse(m_rigidBodyId, jumpImpulse);
        }
    }
};
```

#### Use Cases

- **Vehicles**: Cars, boats, aircraft with realistic physics
- **Ragdoll Characters**: Physics-driven character animation
- **Dynamic Objects**: Objects that need to interact realistically with environment

## 📊 Performance Comparison

| Feature                | Deterministic | Hybrid     | Physics    |
| ---------------------- | ------------- | ---------- | ---------- |
| **CPU Usage**          | ⭐⭐⭐⭐⭐    | ⭐⭐⭐⭐   | ⭐⭐⭐     |
| **Memory Usage**       | ⭐⭐⭐⭐⭐    | ⭐⭐⭐⭐   | ⭐⭐⭐     |
| **Precision**          | ⭐⭐⭐⭐⭐    | ⭐⭐⭐⭐   | ⭐⭐⭐     |
| **Collision Accuracy** | ⭐⭐          | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **Realism**            | ⭐⭐          | ⭐⭐⭐⭐   | ⭐⭐⭐⭐⭐ |
| **Determinism**        | ⭐⭐⭐⭐⭐    | ⭐⭐⭐⭐   | ⭐⭐       |
| **Networking**         | ⭐⭐⭐⭐⭐    | ⭐⭐⭐⭐   | ⭐⭐⭐     |

## 🔧 Configuration System

Each movement component supports extensive configuration:

```cpp
// Movement configuration
CharacterMovementComponent::MovementConfig config;
config.maxWalkSpeed = 6.0f;          // Walking speed
config.jumpZVelocity = 10.0f;        // Jump strength
config.maxAcceleration = 20.0f;      // Acceleration rate
config.airControl = 0.2f;            // Air control factor
config.maxStepHeight = 0.3f;         // Step-up height
config.maxSlopeAngle = 45.0f;        // Walkable slope angle

// Apply to any movement component
character->GetMovementComponent()->SetMovementConfig(config);
```

### Preset Configurations

```cpp
// Factory presets for common use cases
auto platformingConfig = MovementComponentFactory::GetPlatformingConfig();
auto realisticConfig = MovementComponentFactory::GetRealisticConfig();
auto arcadeConfig = MovementComponentFactory::GetArcadeConfig();
```

## 🎨 Visual Debugging System

Each movement type has distinct visual representation:

- **Deterministic**: Bright blue (precise control)
- **Hybrid**: Cyan (balanced approach)
- **Physics**: Dark blue (realistic simulation)

```cpp
// Get visual feedback for current movement type
Math::Vec4 color = character->GetMovementTypeColor();
renderer->DrawCube(character->GetPosition(), size, color);
```

## 🚀 Usage Examples

### Basic Character Setup

```cpp
// Create character with default deterministic movement
auto player = std::make_unique<Character>();
player->Initialize(engine.GetPhysics());

// Character starts with DeterministicMovementComponent
LOG_INFO("Movement type: " + std::string(player->GetMovementTypeName()));
```

### Dynamic Movement Switching

```cpp
void HandleGameplayMechanics(Character* player) {
    if (player->IsInVehicle()) {
        // Use physics for realistic vehicle behavior
        player->SwitchToPhysicsMovement();
    }
    else if (player->IsInCombat()) {
        // Use deterministic for precise combat movement
        player->SwitchToDeterministicMovement();
    }
    else {
        // Use hybrid for exploration
        player->SwitchToHybridMovement();
    }
}
```

### Custom Movement Component

```cpp
class CustomMovementComponent : public CharacterMovementComponent {
public:
    const char* GetComponentTypeName() const override {
        return "CustomMovementComponent";
    }

    void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) override {
        // Implement custom movement logic
        // Could combine aspects of multiple movement types
        HandleCustomMovement(deltaTime, input, camera);
    }

    // Implement all pure virtual methods...
};

// Use custom component
auto customComponent = std::make_unique<CustomMovementComponent>();
character->SetMovementComponent(std::move(customComponent));
```

## 🔬 Technical Implementation Details

### State Preservation During Switching

When switching movement components, the system preserves:

- **Position**: Exact world position
- **Velocity**: Current movement velocity
- **Rotation**: Character orientation
- **Configuration**: Movement parameters

```cpp
void Character::SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component) {
    // Preserve current state
    Math::Vec3 currentPosition = m_movementComponent->GetPosition();
    Math::Vec3 currentVelocity = m_movementComponent->GetVelocity();
    float currentRotation = m_movementComponent->GetRotation();

    // Shutdown old component
    m_movementComponent->Shutdown();

    // Initialize new component
    m_movementComponent = std::move(component);
    m_movementComponent->Initialize(m_physicsEngine);

    // Restore state
    m_movementComponent->SetPosition(currentPosition);
    m_movementComponent->SetVelocity(currentVelocity);
    m_movementComponent->SetRotation(currentRotation);
}
```

### Memory Management

All movement components use RAII principles:

- **Automatic Cleanup**: Destructors handle resource deallocation
- **Smart Pointers**: Automatic memory management
- **Physics Resource Management**: Proper cleanup of rigid bodies and ghost objects

### Thread Safety

The current implementation is single-threaded but designed for future multi-threading:

- **Component Isolation**: Each component manages its own state
- **Immutable Configuration**: Thread-safe parameter access
- **Physics Engine Integration**: Prepared for threaded physics updates

## 🎯 Best Practices

### Choosing the Right Movement Type

#### Use Deterministic When:

- **Precision is Critical**: Platformers, puzzle games
- **Networking Required**: Multiplayer games needing synchronization
- **Performance Matters**: Mobile games, large numbers of NPCs
- **Predictability Needed**: AI characters, scripted sequences

#### Use Hybrid When:

- **Balanced Approach Needed**: Most third-person games
- **Complex Environments**: Games with varied terrain
- **Good Collision Required**: Without full physics overhead
- **Exploration Focus**: Adventure games, open-world games

#### Use Physics When:

- **Realism is Priority**: Simulation games, realistic shooters
- **Dynamic Interactions**: Objects that push/pull characters
- **Vehicle Integration**: Characters that enter/exit vehicles
- **Ragdoll Needed**: Characters that can be knocked around

### Performance Optimization

```cpp
// Optimize for your use case
MovementConfig config;

// For mobile/low-end hardware
config.maxAcceleration = 15.0f;  // Lower values = less computation
config.airControl = 0.1f;        // Reduce air control complexity

// For high-end/desktop
config.maxAcceleration = 30.0f;  // Higher responsiveness
config.airControl = 0.5f;        // More complex air movement
```

## 🔮 Future Enhancements

The component-based architecture enables future features:

### Planned Movement Types

- **SwimmingMovementComponent**: Water-based movement
- **FlyingMovementComponent**: Aerial movement with lift/drag
- **ClimbingMovementComponent**: Wall/ladder climbing
- **VehicleMovementComponent**: Specialized vehicle physics

### Advanced Features

- **Movement Blending**: Smooth transitions between movement types
- **State Machines**: Complex movement state management
- **Animation Integration**: Movement-driven animation systems
- **Network Optimization**: Deterministic networking protocols

## 📈 Impact and Benefits

### For Developers

- **Flexibility**: Choose the right movement type for each character
- **Productivity**: No need to implement custom physics from scratch
- **Maintainability**: Clean, modular architecture
- **Debugging**: Visual feedback and clear component separation

### For Games

- **Better Feel**: Each character can have optimal movement behavior
- **Performance**: Use only the physics complexity you need
- **Consistency**: Unified interface across different movement types
- **Scalability**: Easy to add new movement types as needed

### For Players

- **Responsive Controls**: Deterministic movement for precise control
- **Realistic Interaction**: Hybrid/Physics movement for immersion
- **Consistent Experience**: Smooth transitions between movement types
- **Better Performance**: Optimized physics usage

---

This deterministic physics implementation represents a significant advancement in game engine character movement, providing developers with unprecedented flexibility and control while maintaining high performance and realistic behavior.

**Game Engine Kiro** - Where precision meets realism in character physics.
