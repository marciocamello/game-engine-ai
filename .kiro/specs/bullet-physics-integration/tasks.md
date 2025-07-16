# Implementation Plan

**Target Platform:** Windows 10/11, Visual Studio 2022, PowerShell/pwsh scripts

**Current Status:** Bullet3 is configured in vcpkg.json but not actually integrated. PhysicsEngine contains only placeholder methods.

## Phase 1: Foundation Setup

- [x] 1. Verify and fix Bullet Physics build integration

  - Confirm Bullet3 is properly installed via vcpkg (run `.\build.bat` to verify)
  - Add Bullet Physics headers to include paths in CMakeLists.txt
  - Fix the existing `find_package(Bullet QUIET)` configuration
  - Create basic Bullet Physics initialization test to verify library integration
  - Test build on Windows with PowerShell: `.\build.bat`
  - _Requirements: 1.1, 1.4_

- [x] 2. Create math type conversion utilities

  - Implement conversion functions between Math::Vec3/Quat and btVector3/btQuaternion
  - Add conversion utilities to a new `include/Physics/BulletUtils.h` file
  - Write unit tests for conversion accuracy and round-trip conversions
  - Verify compilation with Windows/MSVC toolchain
  - _Requirements: 1.2, 1.3_

- [x] 3. Implement collision shape factory

  - Create CollisionShapeFactory class with static methods for each shape type
  - Implement CreateBoxShape, CreateSphereShape, CreateCapsuleShape methods
  - Add to `include/Physics/CollisionShapeFactory.h`
  - Write unit tests for collision shape creation with various parameters
  - _Requirements: 3.3, 1.2_

## Phase 2: Core Physics Implementation

- [x] 4. Create BulletPhysicsWorld wrapper class

  - Create `include/Physics/BulletPhysicsWorld.h` and `src/Physics/BulletPhysicsWorld.cpp`
  - Implement BulletPhysicsWorld inheriting from PhysicsWorld
  - Initialize Bullet Physics components (broadphase, collision config, dispatcher, solver, dynamics world)
  - Implement Step method to call Bullet's stepSimulation
  - Add proper resource cleanup in destructor
  - Test compilation with `.\build.bat` on Windows
  - _Requirements: 1.1, 1.2, 5.3_

- [x] 5. Replace PhysicsEngine placeholder methods with Bullet implementations

  - Replace `PhysicsEngine::Initialize()` stub with real Bullet Physics initialization
  - Update `CreateWorld()` to return BulletPhysicsWorld instances instead of stub PhysicsWorld
  - Replace all "// Implementation would..." comments with actual Bullet Physics calls
  - Maintain existing method signatures for API compatibility
  - _Requirements: 1.1, 1.2, 4.1_

- [x] 6. Implement rigid body creation and management

  - Replace `CreateRigidBody()` stub to create real btRigidBody objects with collision shapes
  - Implement `DestroyRigidBody()` to properly clean up Bullet Physics resources
  - Add rigid body ID to btRigidBody mapping for efficient lookups
  - Remove placeholder `m_rigidBodies` vector, use Bullet's world management
  - _Requirements: 1.2, 5.1, 5.2_

- [x] 7. Implement rigid body transform and force operations

  - Replace `SetRigidBodyTransform()` stub with btRigidBody transform methods ✓
  - Replace `ApplyForce()` and `ApplyImpulse()` stubs with Bullet Physics force application ✓
  - Add proper error handling for invalid body IDs ✓
  - Remove placeholder comments, add real implementations ✓
  - _Requirements: 1.2, 2.2, 4.1_

- [x] 8. Implement physics queries (raycast and overlap)

  - Replace `Raycast()` stub with btCollisionWorld::rayTest implementation
  - Replace `OverlapSphere()` stub with btCollisionWorld contact testing
  - Return proper hit information and body IDs from queries instead of empty results
  - Update method signatures if needed for proper hit information
  - _Requirements: 3.1, 3.2_

## Phase 3: Configuration and Integration

- [x] 9. Add physics configuration and parameter management

  - Create PhysicsConfiguration struct with gravity, timestep, and solver settings
  - Add configuration parameter to PhysicsEngine::Initialize() method
  - Implement configuration methods in PhysicsEngine and BulletPhysicsWorld
  - Add runtime parameter modification capabilities
  - _Requirements: 6.1, 6.2, 6.3, 6.4_

## Phase 4: Character Physics Integration

- [ ] 10. Integrate Character class with physics engine

  - Add PhysicsEngine reference and rigid body ID to Character class
  - Create character rigid body during Character::Initialize using capsule shape
  - Replace manual physics calculations in `UpdatePhysics()` with rigid body queries
  - Remove manual gravity/velocity calculations: `m_velocity.y += m_gravity * deltaTime`
  - _Requirements: 2.1, 2.2, 4.1_

- [ ] 11. Replace character movement with physics forces

  - Replace direct position updates (`m_position += movement`) with ApplyForce calls
  - Replace manual jumping (`m_velocity.y = m_jumpSpeed`) with ApplyImpulse for vertical velocity
  - Add character rotation constraints to keep character upright
  - Remove hardcoded world boundaries, use physics collision instead
  - _Requirements: 2.2, 2.3_

- [ ] 12. Replace manual ground collision with physics-based detection

  - Remove hardcoded ground collision: `if (m_position.y <= groundLevel)`
  - Implement collision callbacks to detect ground contact
  - Update character grounded state based on collision information instead of y-position check
  - Remove manual ground level positioning
  - _Requirements: 2.3, 3.1, 3.2_

## Phase 5: Testing and Finalization

- [ ] 13. Implement proper resource cleanup and error handling

  - Add comprehensive error handling throughout physics integration
  - Implement proper Bullet Physics resource cleanup in destructors
  - Add memory leak detection and prevention measures
  - Test with Windows debugging tools and Visual Studio diagnostics
  - _Requirements: 5.1, 5.2, 5.3, 1.4_

- [ ] 14. Create comprehensive physics integration tests

  - Write unit tests for all physics operations (creation, forces, queries)
  - Create integration tests for character physics behavior
  - Add performance benchmarks comparing before/after integration
  - Test memory usage and leak detection over extended runtime
  - Use Windows testing framework and PowerShell test scripts
  - _Requirements: 1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 3.1, 3.2_

- [ ] 15. Add physics debugging and visualization support

  - Implement debug drawing interface for Bullet Physics debug renderer
  - Add physics object visualization (collision shapes, forces, contacts)
  - Create debug console commands for physics parameter tuning
  - Add PowerShell scripts for physics debugging and profiling
  - _Requirements: 6.3, 1.4_

## Build and Development Notes

**Windows Development Workflow:**

- Use `.\build.bat` to build project with Bullet Physics integration
- Use `.\dev.bat` for development builds with debug symbols
- PowerShell scripts for testing: `.\run_tests.ps1` (to be created)
- Visual Studio 2022 for debugging and development

**Key Files to Modify:**

- `CMakeLists.txt` - Bullet Physics linking
- `include/Physics/PhysicsEngine.h` - API definitions
- `src/Physics/PhysicsEngine.cpp` - Replace placeholder implementations
- `src/Game/Character.cpp` - Replace manual physics in `UpdatePhysics()`
- `include/Game/Character.h` - Add physics body ID member

**Testing Strategy:**

- Unit tests for each physics component
- Integration tests with Character movement
- Performance comparison before/after
- Memory leak detection with Visual Studio diagnostics
