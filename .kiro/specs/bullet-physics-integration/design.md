# Design Document

## Overview

This design outlines the integration of Bullet Physics library into the existing GameEngine architecture. The integration will replace the current stub/placeholder physics implementation with a full-featured physics engine while maintaining API compatibility.

**Target Platform:** Windows 10/11 with Visual Studio 2022, PowerShell/pwsh for build scripts.

**Current Implementation Status:**

- `PhysicsEngine::Initialize()` only logs "Physics Engine initialized"
- All physics methods contain placeholder comments: "// Implementation would..."
- `Character` uses manual physics: gravity, velocity, ground collision at y=0
- Bullet3 dependency exists in vcpkg.json but no actual Bullet headers included

**Integration Strategy:**

- Replace placeholder methods with real Bullet Physics implementations
- Convert Character from manual physics to rigid body dynamics
- Maintain existing PhysicsEngine API to minimize code changes
- Add proper Bullet Physics resource management

Key design principles:

- Maintain existing API compatibility to minimize code changes
- Provide proper resource management for Bullet Physics objects
- Enable seamless transition from manual physics to Bullet Physics
- Focus on Windows development workflow with PowerShell scripts

## Architecture

### High-Level Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Game Layer    │    │  Character       │    │  Other Game     │
│                 │    │  (uses physics)  │    │  Objects        │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
┌─────────────────────────────────────────────────────────────────┐
│                    PhysicsEngine (Wrapper)                     │
│  - Maintains existing API                                       │
│  - Manages Bullet Physics world and objects                    │
│  - Handles resource lifecycle                                   │
└─────────────────────────────────────────────────────────────────┘
                                 │
┌─────────────────────────────────────────────────────────────────┐
│                      Bullet Physics                            │
│  - btDiscreteDynamicsWorld                                      │
│  - btRigidBody objects                                          │
│  - btCollisionShapes                                            │
│  - btBroadphaseInterface, btCollisionDispatcher, etc.          │
└─────────────────────────────────────────────────────────────────┘
```

### Component Integration Strategy

The integration will follow a wrapper pattern where:

1. **PhysicsEngine** becomes a facade over Bullet Physics components
2. **PhysicsWorld** wraps `btDiscreteDynamicsWorld`
3. **RigidBody** struct maps to `btRigidBody` properties
4. **CollisionShape** enum maps to Bullet collision shapes

## Components and Interfaces

### Core Physics Components

#### BulletPhysicsWorld

```cpp
class BulletPhysicsWorld : public PhysicsWorld {
private:
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;

    // Body management
    std::unordered_map<uint32_t, btRigidBody*> m_rigidBodies;
    std::unordered_map<uint32_t, std::unique_ptr<btCollisionShape>> m_collisionShapes;
};
```

#### Enhanced PhysicsEngine

The existing PhysicsEngine will be enhanced to:

- Initialize Bullet Physics components during `Initialize()`
- Create `BulletPhysicsWorld` instances instead of stub `PhysicsWorld`
- Map all rigid body operations to Bullet Physics calls
- Handle proper cleanup of Bullet resources

#### Collision Shape Factory

```cpp
class CollisionShapeFactory {
public:
    static std::unique_ptr<btCollisionShape> CreateShape(const CollisionShape& desc);
private:
    static std::unique_ptr<btBoxShape> CreateBoxShape(const Math::Vec3& dimensions);
    static std::unique_ptr<btSphereShape> CreateSphereShape(float radius);
    static std::unique_ptr<btCapsuleShape> CreateCapsuleShape(float radius, float height);
};
```

### Character Integration

#### Current Character Physics (Manual Implementation)

The Character class currently uses basic manual physics in `UpdatePhysics()`:

```cpp
// Current manual physics implementation
void Character::UpdatePhysics(float deltaTime) {
    // Apply gravity
    if (!m_isGrounded) {
        m_velocity.y += m_gravity * deltaTime;  // -9.81f
    }

    // Apply velocity
    m_position += m_velocity * deltaTime;

    // Simple ground collision (y = 0)
    float groundLevel = m_height * 0.5f;
    if (m_position.y <= groundLevel) {
        m_position.y = groundLevel;
        m_velocity.y = 0.0f;
        m_isGrounded = true;
        m_isJumping = false;
    }
}
```

#### Physics-Enabled Character (Target Implementation)

The Character class will be modified to:

- Create a rigid body during initialization via PhysicsEngine
- Replace manual physics calculations with Bullet Physics rigid body
- Apply forces/impulses for movement instead of direct position updates
- Query rigid body transform for rendering position
- Handle ground detection through collision callbacks instead of hardcoded y=0

#### Character Physics Properties

- **Collision Shape**: Capsule shape matching character dimensions (radius: 0.5f, height: 1.8f)
- **Mass**: Appropriate mass for character (e.g., 70kg)
- **Friction**: Ground friction for realistic movement
- **Restitution**: Low bounce for character stability
- **Constraints**: Prevent rotation around X and Z axes (keep upright)
- **Linear Damping**: Prevent sliding, realistic movement feel

## Data Models

### Bullet Physics Integration Data

#### RigidBodyHandle

```cpp
struct RigidBodyHandle {
    uint32_t id;
    btRigidBody* bulletBody;
    btCollisionShape* shape;
    btMotionState* motionState;
};
```

#### PhysicsConfiguration

```cpp
struct PhysicsConfiguration {
    Math::Vec3 gravity{0.0f, -9.81f, 0.0f};
    float timeStep = 1.0f / 60.0f;
    int maxSubSteps = 10;
    int solverIterations = 10;
    bool enableCCD = true; // Continuous Collision Detection
};
```

### Type Conversions

#### Math Type Conversions

Utility functions to convert between engine math types and Bullet types:

```cpp
btVector3 ToBullet(const Math::Vec3& vec);
Math::Vec3 FromBullet(const btVector3& vec);
btQuaternion ToBullet(const Math::Quat& quat);
Math::Quat FromBullet(const btQuaternion& quat);
```

## Error Handling

### Initialization Errors

- **Bullet Physics Library Missing**: Graceful fallback with clear error messages
- **Memory Allocation Failures**: Proper cleanup and error reporting
- **Invalid Configuration**: Use safe defaults and log warnings

### Runtime Errors

- **Invalid Body IDs**: Return false/empty results, log warnings
- **Physics World Corruption**: Detect and attempt recovery
- **Resource Exhaustion**: Implement limits and cleanup strategies

### Error Recovery Strategies

1. **Graceful Degradation**: Fall back to basic physics if Bullet fails
2. **Resource Cleanup**: Ensure no memory leaks on errors
3. **State Validation**: Regular consistency checks
4. **Logging**: Comprehensive error logging for debugging

## Testing Strategy

### Unit Testing

- **Collision Shape Creation**: Test all shape types with various parameters
- **Rigid Body Management**: Test creation, destruction, and property updates
- **Type Conversions**: Test math type conversions for accuracy
- **Resource Management**: Test proper cleanup and memory management

### Integration Testing

- **Character Physics**: Test character movement, jumping, and collision
- **Multi-Body Interactions**: Test collision between multiple objects
- **Performance**: Measure physics simulation performance
- **Memory Usage**: Monitor for memory leaks during extended operation

### Physics Validation Testing

- **Gravity Simulation**: Verify objects fall at correct rates
- **Collision Response**: Test realistic collision behavior
- **Force Application**: Verify forces produce expected motion
- **Constraint Behavior**: Test character movement constraints

### Regression Testing

- **API Compatibility**: Ensure existing game code continues to work
- **Performance Benchmarks**: Compare performance before/after integration
- **Stability Testing**: Extended runtime testing for stability

## Implementation Phases

### Phase 1: Core Integration

- Set up Bullet Physics build integration
- Implement basic BulletPhysicsWorld wrapper
- Create collision shape factory
- Implement basic rigid body creation/destruction

### Phase 2: API Implementation

- Implement all PhysicsEngine methods with Bullet backend
- Add proper resource management and cleanup
- Implement type conversion utilities
- Add comprehensive error handling

### Phase 3: Character Integration

- Modify Character class to use PhysicsEngine
- Implement character-specific physics constraints
- Add collision detection for ground/environment
- Test and tune character physics behavior

### Phase 4: Testing and Optimization

- Comprehensive testing suite
- Performance optimization
- Memory usage optimization
- Documentation and examples
