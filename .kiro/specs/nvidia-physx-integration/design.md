# Design Document - NVIDIA PhysX Integration

## Overview

This design document outlines the integration of NVIDIA PhysX as a high-performance alternative physics backend for Game Engine Kiro v1.1. The design focuses on creating a dual-backend architecture that allows seamless switching between Bullet Physics and NVIDIA PhysX while maintaining API compatibility and maximizing performance benefits.

### PhysX Version Considerations

**Current Implementation**: PhysX 5.5.0 (via vcpkg)
**Target Version**: PhysX 5.6.1+ (when available)

**Key Changes in PhysX 5.6.1+**:

- GPU source code is now available (no more binaries)
- Friction type simplification (only PxFrictionType::ePATCH supported)
- Binary platform conversion removed
- Angular joint drive parameters reworked
- Character controller improvements
- Custom geometry enhancements

## Architecture

### Dual-Backend Physics Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    PhysicsEngine                            │
├─────────────────────────────────────────────────────────────┤
│  Backend Selection │ Configuration │ Performance Monitor   │
├─────────────────────────────────────────────────────────────┤
│                  IPhysicsBackend                            │
├─────────────────────────────────────────────────────────────┤
│ BulletPhysicsBackend │        │ PhysXBackend               │
├──────────────────────┼────────┼────────────────────────────┤
│   Bullet Physics     │  API   │    NVIDIA PhysX            │
│   - CPU Simulation   │ Bridge │    - GPU Acceleration      │
│   - Deterministic    │        │    - Advanced Features     │
│   - Cross-Platform   │        │    - High Performance      │
└──────────────────────┴────────┴────────────────────────────┘
```

## Components and Interfaces

### 1. Enhanced PhysicsEngine

```cpp
class PhysicsEngine {
public:
    enum class BackendType { Auto, Bullet, PhysX };

    // Initialization with backend selection
    bool Initialize(const PhysicsConfig& config = {});
    void Shutdown();

    // Backend management
    bool SetBackend(BackendType backend);
    BackendType GetCurrentBackend() const;
    bool IsBackendAvailable(BackendType backend) const;

    // Performance monitoring
    PhysicsPerformanceStats GetPerformanceStats() const;
    void EnablePerformanceProfiling(bool enable);

    // Hardware detection
    HardwareCapabilities DetectHardwareCapabilities() const;
    void OptimizeForHardware(const HardwareCapabilities& caps);

private:
    std::unique_ptr<IPhysicsBackend> m_activeBackend;
    std::unique_ptr<BulletPhysicsBackend> m_bulletBackend;
    std::unique_ptr<PhysXBackend> m_physxBackend;

    BackendType m_currentBackendType = BackendType::Auto;
    std::unique_ptr<PhysicsPerformanceMonitor> m_performanceMonitor;
    std::unique_ptr<HardwareDetector> m_hardwareDetector;
};
```

### 2. IPhysicsBackend Interface

```cpp
class IPhysicsBackend {
public:
    virtual ~IPhysicsBackend() = default;

    // Lifecycle
    virtual bool Initialize(const PhysicsConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;

    // World management
    virtual void SetGravity(const Math::Vec3& gravity) = 0;
    virtual Math::Vec3 GetGravity() const = 0;

    // Rigid body management
    virtual uint32_t CreateRigidBody(const RigidBodyDesc& desc) = 0;
    virtual void DestroyRigidBody(uint32_t bodyId) = 0;
    virtual void SetRigidBodyTransform(uint32_t bodyId, const Math::Vec3& position, const Math::Quat& rotation) = 0;
    virtual void GetRigidBodyTransform(uint32_t bodyId, Math::Vec3& position, Math::Quat& rotation) const = 0;
    virtual void ApplyForce(uint32_t bodyId, const Math::Vec3& force, const Math::Vec3& point = Math::Vec3(0.0f)) = 0;
    virtual void ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse, const Math::Vec3& point = Math::Vec3(0.0f)) = 0;

    // Collision detection
    virtual bool Raycast(const Ray& ray, RaycastHit& hit) const = 0;
    virtual bool SphereCast(const Math::Vec3& origin, float radius, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const = 0;
    virtual bool BoxCast(const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const = 0;
    virtual bool CapsuleCast(const Math::Vec3& point1, const Math::Vec3& point2, float radius, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const = 0;

    // Overlap queries
    virtual std::vector<uint32_t> OverlapSphere(const Math::Vec3& center, float radius) const = 0;
    virtual std::vector<uint32_t> OverlapBox(const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation) const = 0;
    virtual std::vector<uint32_t> OverlapCapsule(const Math::Vec3& point1, const Math::Vec3& point2, float radius) const = 0;

    // Performance and debugging
    virtual PhysicsStats GetStats() const = 0;
    virtual void SetDebugVisualization(bool enabled) = 0;
    virtual void DrawDebugVisualization() const = 0;

    // Backend-specific information
    virtual const char* GetBackendName() const = 0;
    virtual const char* GetBackendVersion() const = 0;
    virtual bool SupportsGPUAcceleration() const = 0;
};
```

### 3. PhysXBackend Implementation

```cpp
class PhysXBackend : public IPhysicsBackend {
public:
    PhysXBackend();
    ~PhysXBackend();

    // IPhysicsBackend implementation
    bool Initialize(const PhysicsConfig& config) override;
    void Shutdown() override;
    void Update(float deltaTime) override;

    // PhysX-specific methods
    bool InitializePhysXFoundation();
    bool InitializePhysXPhysics();
    bool InitializePhysXScene();
    bool InitializeGPUAcceleration();

    void SetSolverIterations(uint32_t positionIterations, uint32_t velocityIterations);
    void SetSimulationThreads(uint32_t threadCount);
    void EnableContinuousCollisionDetection(bool enable);

    // Advanced PhysX features
    void SetGPUMemoryBudget(size_t memoryBudget);
    void EnableAdaptiveForce(bool enable);
    void SetBroadPhaseType(BroadPhaseType type);

    // PhysX 5.6.1+ specific features
    void ConfigureAngularDriveModel(AngularDriveModel model);
    void SetupCustomGeometry(const CustomGeometryDesc& desc);
    void EnableCharacterControllerEnhancements(bool enable);

private:
    // PhysX core objects
    physx::PxFoundation* m_foundation = nullptr;
    physx::PxPhysics* m_physics = nullptr;
    physx::PxScene* m_scene = nullptr;
    physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
    physx::PxCudaContextManager* m_cudaContextManager = nullptr;

    // PhysX configuration
    physx::PxTolerancesScale m_toleranceScale;
    physx::PxSceneDesc m_sceneDesc;

    // Object management
    std::unordered_map<uint32_t, physx::PxRigidActor*> m_rigidBodies;
    std::unordered_map<uint32_t, physx::PxController*> m_characterControllers;
    std::unique_ptr<PhysXErrorCallback> m_errorCallback;
    std::unique_ptr<PhysXAllocatorCallback> m_allocatorCallback;

    // Performance monitoring
    mutable PhysicsStats m_stats;
    bool m_gpuAccelerationEnabled = false;

    // PhysX 5.6.1+ compatibility
    bool m_useModernFrictionModel = true;
    AngularDriveModel m_angularDriveModel = AngularDriveModel::SwingTwist;

    // Helper methods
    physx::PxRigidDynamic* CreateDynamicRigidBody(const RigidBodyDesc& desc);
    physx::PxRigidStatic* CreateStaticRigidBody(const RigidBodyDesc& desc);
    physx::PxShape* CreateCollisionShape(const CollisionShapeDesc& desc);
    physx::PxMaterial* GetOrCreateMaterial(float staticFriction, float dynamicFriction, float restitution);

    // PhysX 5.6.1+ helpers
    void SetupModernFrictionModel(physx::PxMaterial* material);
    void ConfigureJointDriveParameters(physx::PxD6Joint* joint, const JointDriveConfig& config);
};
```

### 4. Hardware Detection System

```cpp
struct HardwareCapabilities {
    bool hasNVIDIAGPU = false;
    bool supportsCUDA = false;
    std::string gpuName;
    size_t gpuMemoryMB = 0;
    int cudaComputeCapability = 0;
    int cpuCoreCount = 0;
    size_t systemMemoryMB = 0;
    bool supportsPhysX = false;
    std::string physxVersion;
};

class HardwareDetector {
public:
    HardwareCapabilities DetectCapabilities() const;
    bool IsPhysXSupported() const;
    bool IsCUDAAvailable() const;
    std::vector<std::string> GetAvailableGPUs() const;

private:
    bool DetectNVIDIAGPU() const;
    bool DetectCUDASupport() const;
    size_t GetGPUMemory() const;
    int GetCUDAComputeCapability() const;
};
```

### 5. Performance Monitoring

```cpp
struct PhysicsPerformanceStats {
    float updateTimeMs = 0.0f;
    float collisionTimeMs = 0.0f;
    float solverTimeMs = 0.0f;
    uint32_t activeBodies = 0;
    uint32_t totalBodies = 0;
    uint32_t collisionPairs = 0;
    size_t memoryUsageMB = 0;
    bool usingGPUAcceleration = false;
    float gpuUtilization = 0.0f;
};

class PhysicsPerformanceMonitor {
public:
    void BeginFrame();
    void EndFrame();
    void RecordUpdateTime(float timeMs);
    void RecordCollisionTime(float timeMs);
    void RecordSolverTime(float timeMs);

    PhysicsPerformanceStats GetStats() const;
    void ResetStats();

    // Benchmarking
    void RunBenchmark(IPhysicsBackend* backend, const BenchmarkConfig& config);
    BenchmarkResults GetBenchmarkResults() const;

private:
    PhysicsPerformanceStats m_currentStats;
    std::vector<PhysicsPerformanceStats> m_frameHistory;
    std::unique_ptr<Timer> m_timer;
};
```

## Data Models

### Configuration System

```cpp
struct PhysicsConfig {
    // Backend selection
    PhysicsEngine::BackendType preferredBackend = PhysicsEngine::BackendType::Auto;
    bool fallbackToBullet = true;
    bool enableGPUAcceleration = true;

    // Performance settings
    uint32_t maxRigidBodies = 10000;
    uint32_t solverPositionIterations = 4;
    uint32_t solverVelocityIterations = 1;
    uint32_t simulationThreads = 0;  // 0 = auto-detect

    // Memory settings
    size_t gpuMemoryBudgetMB = 512;
    size_t cpuMemoryBudgetMB = 256;

    // Simulation settings
    Math::Vec3 gravity = Math::Vec3(0.0f, -9.81f, 0.0f);
    float fixedTimestep = 1.0f / 60.0f;
    bool enableCCD = false;
    bool enableAdaptiveForce = true;

    // Debug settings
    bool enableDebugVisualization = false;
    bool enablePerformanceProfiling = false;
    bool verboseLogging = false;
};
```

### Backend Selection Logic

```cpp
class BackendSelector {
public:
    PhysicsEngine::BackendType SelectOptimalBackend(const PhysicsConfig& config, const HardwareCapabilities& hardware) const;

private:
    float ScoreBackend(PhysicsEngine::BackendType backend, const HardwareCapabilities& hardware) const;
    bool MeetsMinimumRequirements(PhysicsEngine::BackendType backend, const HardwareCapabilities& hardware) const;
};

// Selection algorithm
PhysicsEngine::BackendType BackendSelector::SelectOptimalBackend(const PhysicsConfig& config, const HardwareCapabilities& hardware) const {
    if (config.preferredBackend != PhysicsEngine::BackendType::Auto) {
        if (MeetsMinimumRequirements(config.preferredBackend, hardware)) {
            return config.preferredBackend;
        } else if (config.fallbackToBullet) {
            return PhysicsEngine::BackendType::Bullet;
        }
    }

    // Automatic selection based on hardware capabilities
    if (hardware.hasNVIDIAGPU && hardware.supportsCUDA && hardware.gpuMemoryMB >= 1024) {
        return PhysicsEngine::BackendType::PhysX;
    }

    if (hardware.cpuCoreCount >= 8 && hardware.systemMemoryMB >= 8192) {
        return PhysicsEngine::BackendType::PhysX;  // CPU PhysX
    }

    return PhysicsEngine::BackendType::Bullet;  // Default fallback
}
```

## Error Handling

### PhysX Error Handling

```cpp
class PhysXErrorCallback : public physx::PxErrorCallback {
public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;

private:
    void LogPhysXError(physx::PxErrorCode::Enum code, const std::string& message, const std::string& file, int line);
    bool ShouldTreatAsFatal(physx::PxErrorCode::Enum code) const;
};

class PhysXAllocatorCallback : public physx::PxAllocatorCallback {
public:
    void* allocate(size_t size, const char* typeName, const char* filename, int line) override;
    void deallocate(void* ptr) override;

private:
    std::atomic<size_t> m_totalAllocated{0};
    std::mutex m_allocationMutex;
    std::unordered_map<void*, size_t> m_allocations;
};
```

### Graceful Degradation

```cpp
class PhysicsBackendManager {
public:
    bool InitializeWithFallback(const PhysicsConfig& config);
    void HandleBackendFailure(PhysicsEngine::BackendType failedBackend);

private:
    bool TryInitializePhysX(const PhysicsConfig& config);
    bool TryInitializeBullet(const PhysicsConfig& config);
    void LogBackendFailure(PhysicsEngine::BackendType backend, const std::string& reason);
};
```

## Testing Strategy

### Performance Benchmarking

```cpp
struct BenchmarkConfig {
    uint32_t rigidBodyCount = 1000;
    float duration = 10.0f;
    bool enableGPU = true;
    std::string scenarioName;
};

struct BenchmarkResults {
    float averageFPS = 0.0f;
    float minFPS = 0.0f;
    float maxFPS = 0.0f;
    float averageUpdateTime = 0.0f;
    size_t peakMemoryUsage = 0;
    PhysicsEngine::BackendType backend;
};

class PhysicsBenchmark {
public:
    void RunRigidBodyBenchmark(const BenchmarkConfig& config);
    void RunCollisionBenchmark(const BenchmarkConfig& config);
    void RunRaycastBenchmark(const BenchmarkConfig& config);

    BenchmarkResults GetResults(const std::string& scenarioName) const;
    void CompareBackends(const std::vector<PhysicsEngine::BackendType>& backends);

private:
    std::unordered_map<std::string, BenchmarkResults> m_results;
};
```

### Integration Testing

```cpp
class PhysicsIntegrationTests {
public:
    void TestBackendSwitching();
    void TestAPICompatibility();
    void TestPerformanceRegression();
    void TestMemoryLeaks();
    void TestErrorHandling();
    void TestHardwareDetection();

private:
    void CompareBackendResults(const std::string& testName);
    bool ValidatePhysicsState(const PhysicsState& expected, const PhysicsState& actual);
};
```

## Implementation Phases

### Phase 1: Foundation and Interface

- Implement IPhysicsBackend interface
- Create PhysXBackend skeleton
- Add hardware detection system
- Implement backend selection logic

### Phase 2: Core PhysX Integration

- PhysX SDK initialization and cleanup
- Basic rigid body creation and management
- Collision detection implementation
- Force and impulse application

### Phase 3: GPU Acceleration

- CUDA context initialization
- GPU memory management
- GPU-accelerated simulation
- Performance monitoring

### Phase 4: Advanced Features

- Continuous collision detection
- Multi-threading support
- Advanced collision shapes
- Debug visualization

### Phase 5: Optimization and Polish

- Performance benchmarking
- Memory optimization
- Error handling robustness
- Documentation and examples

## Performance Considerations

### Memory Management

```cpp
class PhysXMemoryManager {
public:
    void SetGPUMemoryBudget(size_t budgetMB);
    void SetCPUMemoryBudget(size_t budgetMB);

    bool AllocateGPUMemory(size_t sizeBytes);
    void FreeGPUMemory(void* ptr);

    MemoryStats GetMemoryStats() const;
    void OptimizeMemoryUsage();

private:
    size_t m_gpuMemoryBudget = 0;
    size_t m_cpuMemoryBudget = 0;
    size_t m_gpuMemoryUsed = 0;
    size_t m_cpuMemoryUsed = 0;
};
```

### GPU Optimization

```cpp
class PhysXGPUOptimizer {
public:
    void OptimizeForGPU(physx::PxScene* scene);
    void ConfigureGPUSettings(const HardwareCapabilities& hardware);
    void MonitorGPUPerformance();

private:
    void SetOptimalBroadPhase();
    void ConfigureGPUBuffers();
    void OptimizeParticleSystem();
};
```

## PhysX 5.6.1+ Migration Considerations

### GPU Development Changes

```cpp
// PhysX 5.6.1+ requires building GPU binaries from source
class PhysXGPUBuilder {
public:
    bool BuildGPUBinaries(const std::string& cudaPath);
    bool ValidateGPUBinaries();
    void SetupGPUBuildEnvironment();

private:
    std::string m_cudaPath;
    std::string m_physxSourcePath;
    bool m_gpuBinariesBuilt = false;
};
```

### Friction Model Updates

```cpp
// PhysX 5.6.1+ only supports PxFrictionType::ePATCH
class ModernFrictionManager {
public:
    void SetupPatchFriction(physx::PxMaterial* material);
    void MigrateLegacyFrictionSettings(const LegacyFrictionConfig& legacy);

private:
    // Only ePATCH friction type is supported in 5.6.1+
    void ValidateFrictionType(physx::PxFrictionType::Enum type);
};
```

### Angular Joint Drive Rework

```cpp
enum class AngularDriveModel {
    SLERP,      // Use PxD6Drive::eSLERP
    SwingTwist  // Use PxD6Drive::eTWIST/eSWING1/eSWING2
};

class ModernJointDriveManager {
public:
    void SetAngularDriveConfig(physx::PxD6Joint* joint, AngularDriveModel model);
    void MigrateLegacySwingDrive(physx::PxD6Joint* joint);

private:
    void SetupSLERPDrive(physx::PxD6Joint* joint);
    void SetupSwingTwistDrive(physx::PxD6Joint* joint);
    void ValidateDriveConfiguration(physx::PxD6Joint* joint, AngularDriveModel model);
};
```

### Character Controller Enhancements

```cpp
class EnhancedCharacterController {
public:
    // PhysX 5.6.1+ character controller improvements
    void SetupAdvancedCollisionDetection();
    void EnableImprovedSlopeHandling();
    void ConfigureEnhancedStepOffset();

private:
    physx::PxController* m_controller = nullptr;
    bool m_enhancedFeaturesEnabled = false;
};
```

### Custom Geometry Support

```cpp
class CustomGeometryManager {
public:
    // PhysX 5.6.1+ enhanced custom geometry
    uint32_t CreateCustomGeometry(const CustomGeometryDesc& desc);
    void UpdateCustomGeometry(uint32_t geometryId, const CustomGeometryData& data);
    void SetCustomGeometryCallbacks(uint32_t geometryId, const CustomGeometryCallbacks& callbacks);

private:
    std::unordered_map<uint32_t, physx::PxCustomGeometry*> m_customGeometries;
    uint32_t m_nextGeometryId = 1;
};
```

### Serialization Changes

```cpp
// PhysX 5.6.1+ removes binary platform conversion
class ModernSerializationManager {
public:
    // Note: Deterministic binary serialization removed in 5.6.1+
    bool SerializeScene(physx::PxScene* scene, const std::string& filename);
    bool DeserializeScene(const std::string& filename, physx::PxScene*& scene);

private:
    // Use regular binary serialization (determinism may be recovered in future)
    void WarnAboutDeterminismChanges();
};
```

## Future Extensibility

### Advanced PhysX Features (v1.2+)

```cpp
// Cloth simulation
class PhysXClothSystem {
public:
    uint32_t CreateCloth(const ClothDesc& desc);
    void UpdateCloth(uint32_t clothId, float deltaTime);
    void SetClothWind(uint32_t clothId, const Math::Vec3& wind);
};

// Fluid simulation
class PhysXFluidSystem {
public:
    uint32_t CreateFluidSystem(const FluidDesc& desc);
    void AddFluidParticles(uint32_t systemId, const std::vector<Math::Vec3>& positions);
    void UpdateFluidSystem(uint32_t systemId, float deltaTime);
};

// Destruction physics
class PhysXDestructionSystem {
public:
    uint32_t CreateDestructibleMesh(const DestructibleDesc& desc);
    void ApplyDamage(uint32_t meshId, const Math::Vec3& position, float damage);
    std::vector<DestructionFragment> GetFragments(uint32_t meshId);
};
```

This design provides a comprehensive foundation for integrating NVIDIA PhysX while maintaining compatibility with existing Bullet Physics code and enabling significant performance improvements for supported hardware.
