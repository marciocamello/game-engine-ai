#include "Physics/PhysXBackend.h"
#include "Core/Logger.h"
#include <iostream>
#include <mutex>
#include <atomic>

#ifdef GAMEENGINE_HAS_PHYSX
#include <PxPhysicsAPI.h>
using namespace physx;
#endif

namespace GameEngine {

#ifdef GAMEENGINE_HAS_PHYSX
    /**
     * @brief PhysX error callback implementation
     */
    class PhysXBackend::PhysXErrorCallback : public PxErrorCallback {
    public:
        void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override {
            const char* errorType = "UNKNOWN";
            
            switch (code) {
                case PxErrorCode::eNO_ERROR:
                    errorType = "NO_ERROR";
                    break;
                case PxErrorCode::eDEBUG_INFO:
                    errorType = "DEBUG_INFO";
                    LOG_INFO(std::string("PhysX Debug: ") + message + " (" + file + ":" + std::to_string(line) + ")");
                    return;
                case PxErrorCode::eDEBUG_WARNING:
                    errorType = "DEBUG_WARNING";
                    LOG_WARNING(std::string("PhysX Warning: ") + message + " (" + file + ":" + std::to_string(line) + ")");
                    return;
                case PxErrorCode::eINVALID_PARAMETER:
                    errorType = "INVALID_PARAMETER";
                    break;
                case PxErrorCode::eINVALID_OPERATION:
                    errorType = "INVALID_OPERATION";
                    break;
                case PxErrorCode::eOUT_OF_MEMORY:
                    errorType = "OUT_OF_MEMORY";
                    break;
                case PxErrorCode::eINTERNAL_ERROR:
                    errorType = "INTERNAL_ERROR";
                    break;
                case PxErrorCode::eABORT:
                    errorType = "ABORT";
                    break;
                case PxErrorCode::ePERF_WARNING:
                    errorType = "PERF_WARNING";
                    LOG_WARNING(std::string("PhysX Performance Warning: ") + message + " (" + file + ":" + std::to_string(line) + ")");
                    return;
                case PxErrorCode::eMASK_ALL:
                    errorType = "MASK_ALL";
                    break;
            }
            
            LOG_ERROR(std::string("PhysX Error [") + errorType + "]: " + message + " (" + file + ":" + std::to_string(line) + ")");
        }
    };

    /**
     * @brief PhysX allocator callback implementation
     */
    class PhysXBackend::PhysXAllocatorCallback : public PxAllocatorCallback {
    public:
        void* allocate(size_t size, const char* typeName, const char* filename, int line) override {
            void* ptr = malloc(size);
            if (ptr) {
                m_totalAllocated += size;
                std::lock_guard<std::mutex> lock(m_allocationMutex);
                m_allocations[ptr] = size;
            }
            return ptr;
        }

        void deallocate(void* ptr) override {
            if (ptr) {
                std::lock_guard<std::mutex> lock(m_allocationMutex);
                auto it = m_allocations.find(ptr);
                if (it != m_allocations.end()) {
                    m_totalAllocated -= it->second;
                    m_allocations.erase(it);
                }
                free(ptr);
            }
        }

        size_t GetTotalAllocated() const {
            return m_totalAllocated;
        }

    private:
        std::atomic<size_t> m_totalAllocated{0};
        std::mutex m_allocationMutex;
        std::unordered_map<void*, size_t> m_allocations;
    };
#endif

    PhysXBackend::PhysXBackend() {
        LOG_INFO("PhysXBackend: Creating PhysX backend");
        
#ifdef GAMEENGINE_HAS_PHYSX
        m_errorCallback = std::make_unique<PhysXErrorCallback>();
        m_allocatorCallback = std::make_unique<PhysXAllocatorCallback>();
#else
        LOG_WARNING("PhysXBackend: PhysX not available - compiled without GAMEENGINE_HAS_PHYSX");
#endif
    }

    PhysXBackend::~PhysXBackend() {
        LOG_INFO("PhysXBackend: Destroying PhysX backend");
        Shutdown();
    }

    bool PhysXBackend::Initialize(const PhysicsConfig& config) {
        LOG_INFO("PhysXBackend: Initializing PhysX backend");
        
        if (m_initialized) {
            LOG_WARNING("PhysXBackend: Already initialized");
            return true;
        }

        if (!IsPhysXAvailable()) {
            LOG_ERROR("PhysXBackend: PhysX not available");
            return false;
        }

        m_config = config;

        // Initialize PhysX foundation
        if (!InitializePhysXFoundation()) {
            LOG_ERROR("PhysXBackend: Failed to initialize PhysX foundation");
            return false;
        }

        // Initialize PhysX physics system
        if (!InitializePhysXPhysics()) {
            LOG_ERROR("PhysXBackend: Failed to initialize PhysX physics system");
            CleanupPhysX();
            return false;
        }

        // Initialize PhysX scene
        if (!InitializePhysXScene()) {
            LOG_ERROR("PhysXBackend: Failed to initialize PhysX scene");
            CleanupPhysX();
            return false;
        }

        // Initialize GPU acceleration if requested and available
        if (config.enableGPUAcceleration) {
            if (InitializeGPUAcceleration()) {
                LOG_INFO("PhysXBackend: GPU acceleration enabled");
                m_gpuAccelerationEnabled = true;
            } else {
                LOG_WARNING("PhysXBackend: GPU acceleration requested but not available, using CPU");
                m_gpuAccelerationEnabled = false;
            }
        }

        m_initialized = true;
        LOG_INFO("PhysXBackend: Successfully initialized");
        return true;
    }

    void PhysXBackend::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("PhysXBackend: Shutting down PhysX backend");
        
        CleanupPhysX();
        m_initialized = false;
        m_gpuAccelerationEnabled = false;
        
        LOG_INFO("PhysXBackend: Shutdown complete");
    }

    void PhysXBackend::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

#ifdef GAMEENGINE_HAS_PHYSX
        if (m_scene) {
            // Update physics simulation
            m_scene->simulate(deltaTime);
            m_scene->fetchResults(true);
            
            // Update statistics
            m_stats.updateTimeMs = deltaTime * 1000.0f;
            m_stats.activeBodies = m_scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
            m_stats.totalBodies = m_rigidBodies.size();
            m_stats.usingGPUAcceleration = m_gpuAccelerationEnabled;
            
            if (m_allocatorCallback) {
                m_stats.memoryUsageMB = m_allocatorCallback->GetTotalAllocated() / (1024 * 1024);
            }
        }
#endif
    }

    void PhysXBackend::SetGravity(const Math::Vec3& gravity) {
        m_config.gravity = gravity;
        
#ifdef GAMEENGINE_HAS_PHYSX
        if (m_scene) {
            m_scene->setGravity(PxVec3(gravity.x, gravity.y, gravity.z));
        }
#endif
    }

    Math::Vec3 PhysXBackend::GetGravity() const {
        return m_config.gravity;
    }

    const char* PhysXBackend::GetBackendName() const {
        return "NVIDIA PhysX";
    }

    const char* PhysXBackend::GetBackendVersion() const {
#ifdef GAMEENGINE_HAS_PHYSX
        return "5.5.0"; // PhysX version from vcpkg
#else
        return "Not Available";
#endif
    }

    bool PhysXBackend::SupportsGPUAcceleration() const {
#ifdef GAMEENGINE_HAS_PHYSX
        return true; // PhysX supports GPU acceleration
#else
        return false;
#endif
    }

    PhysicsStats PhysXBackend::GetStats() const {
        return m_stats;
    }

    void PhysXBackend::SetDebugVisualization(bool enabled) {
        // TODO: Implement debug visualization
        LOG_INFO(std::string("PhysXBackend: Debug visualization ") + (enabled ? "enabled" : "disabled") + " (not yet implemented)");
    }

    void PhysXBackend::DrawDebugVisualization() const {
        // TODO: Implement debug visualization drawing
    }

    bool PhysXBackend::InitializePhysXFoundation() {
#ifdef GAMEENGINE_HAS_PHYSX
        LOG_INFO("PhysXBackend: Initializing PhysX foundation");
        
        m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_allocatorCallback, *m_errorCallback);
        if (!m_foundation) {
            LOG_ERROR("PhysXBackend: Failed to create PhysX foundation");
            return false;
        }
        
        LOG_INFO("PhysXBackend: PhysX foundation created successfully");
        return true;
#else
        return false;
#endif
    }

    bool PhysXBackend::InitializePhysXPhysics() {
#ifdef GAMEENGINE_HAS_PHYSX
        LOG_INFO("PhysXBackend: Initializing PhysX physics system");
        
        m_toleranceScale.length = 1.0f;        // 1 meter
        m_toleranceScale.speed = 10.0f;        // 10 m/s
        
        m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, m_toleranceScale, true);
        if (!m_physics) {
            LOG_ERROR("PhysXBackend: Failed to create PhysX physics system");
            return false;
        }
        
        LOG_INFO("PhysXBackend: PhysX physics system created successfully");
        return true;
#else
        return false;
#endif
    }

    bool PhysXBackend::InitializePhysXScene() {
#ifdef GAMEENGINE_HAS_PHYSX
        LOG_INFO("PhysXBackend: Initializing PhysX scene");
        
        m_sceneDesc = new PxSceneDesc(m_physics->getTolerancesScale());
        m_sceneDesc->gravity = PxVec3(m_config.gravity.x, m_config.gravity.y, m_config.gravity.z);
        
        // Create CPU dispatcher
        m_dispatcher = PxDefaultCpuDispatcherCreate(m_config.simulationThreads == 0 ? 2 : m_config.simulationThreads);
        if (!m_dispatcher) {
            LOG_ERROR("PhysXBackend: Failed to create CPU dispatcher");
            return false;
        }
        
        m_sceneDesc->cpuDispatcher = m_dispatcher;
        m_sceneDesc->filterShader = PxDefaultSimulationFilterShader;
        
        m_scene = m_physics->createScene(*m_sceneDesc);
        if (!m_scene) {
            LOG_ERROR("PhysXBackend: Failed to create PhysX scene");
            return false;
        }
        
        LOG_INFO("PhysXBackend: PhysX scene created successfully");
        return true;
#else
        return false;
#endif
    }

    bool PhysXBackend::InitializeGPUAcceleration() {
#ifdef GAMEENGINE_HAS_PHYSX
        LOG_INFO("PhysXBackend: Attempting to initialize GPU acceleration");
        
        // Try to create CUDA context manager
        PxCudaContextManagerDesc cudaContextManagerDesc;
        m_cudaContextManager = PxCreateCudaContextManager(*m_foundation, cudaContextManagerDesc);
        
        if (m_cudaContextManager) {
            if (m_cudaContextManager->contextIsValid()) {
                LOG_INFO("PhysXBackend: CUDA context created successfully");
                
                // GPU dispatcher setup for PhysX 5.x
                LOG_INFO("PhysXBackend: GPU context manager created successfully");
                
                return true;
            } else {
                LOG_WARNING("PhysXBackend: CUDA context is not valid");
                m_cudaContextManager->release();
                m_cudaContextManager = nullptr;
            }
        } else {
            LOG_WARNING("PhysXBackend: Failed to create CUDA context manager");
        }
        
        return false;
#else
        return false;
#endif
    }

    void PhysXBackend::CleanupPhysX() {
#ifdef GAMEENGINE_HAS_PHYSX
        LOG_INFO("PhysXBackend: Cleaning up PhysX resources");
        
        // Clear rigid bodies
        m_rigidBodies.clear();
        
        // Release scene
        if (m_scene) {
            m_scene->release();
            m_scene = nullptr;
        }
        
        // Release scene descriptor
        if (m_sceneDesc) {
            delete m_sceneDesc;
            m_sceneDesc = nullptr;
        }
        
        // Release dispatcher
        if (m_dispatcher) {
            m_dispatcher->release();
            m_dispatcher = nullptr;
        }
        
        // Release CUDA context manager
        if (m_cudaContextManager) {
            m_cudaContextManager->release();
            m_cudaContextManager = nullptr;
        }
        
        // Release physics
        if (m_physics) {
            m_physics->release();
            m_physics = nullptr;
        }
        
        // Release foundation
        if (m_foundation) {
            m_foundation->release();
            m_foundation = nullptr;
        }
        
        LOG_INFO("PhysXBackend: PhysX cleanup complete");
#endif
    }

    bool PhysXBackend::IsPhysXAvailable() const {
#ifdef GAMEENGINE_HAS_PHYSX
        return true;
#else
        return false;
#endif
    }

    // Stub implementations for methods that will be implemented in later tasks
    uint32_t PhysXBackend::CreateRigidBody(const RigidBodyDesc& desc, const CollisionShapeDesc& shape) {
        LOG_WARNING("PhysXBackend::CreateRigidBody: Not yet implemented");
        return 0;
    }

    void PhysXBackend::DestroyRigidBody(uint32_t bodyId) {
        LOG_WARNING("PhysXBackend::DestroyRigidBody: Not yet implemented");
    }

    void PhysXBackend::SetRigidBodyTransform(uint32_t bodyId, const Math::Vec3& position, const Math::Quat& rotation) {
        LOG_WARNING("PhysXBackend::SetRigidBodyTransform: Not yet implemented");
    }

    void PhysXBackend::GetRigidBodyTransform(uint32_t bodyId, Math::Vec3& position, Math::Quat& rotation) const {
        LOG_WARNING("PhysXBackend::GetRigidBodyTransform: Not yet implemented");
    }

    void PhysXBackend::ApplyForce(uint32_t bodyId, const Math::Vec3& force, const Math::Vec3& point) {
        LOG_WARNING("PhysXBackend::ApplyForce: Not yet implemented");
    }

    void PhysXBackend::ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse, const Math::Vec3& point) {
        LOG_WARNING("PhysXBackend::ApplyImpulse: Not yet implemented");
    }

    bool PhysXBackend::Raycast(const Ray& ray, RaycastHit& hit) const {
        LOG_WARNING("PhysXBackend::Raycast: Not yet implemented");
        return false;
    }

    bool PhysXBackend::SphereCast(const Math::Vec3& origin, float radius, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const {
        LOG_WARNING("PhysXBackend::SphereCast: Not yet implemented");
        return false;
    }

    bool PhysXBackend::BoxCast(const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const {
        LOG_WARNING("PhysXBackend::BoxCast: Not yet implemented");
        return false;
    }

    bool PhysXBackend::CapsuleCast(const Math::Vec3& point1, const Math::Vec3& point2, float radius, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const {
        LOG_WARNING("PhysXBackend::CapsuleCast: Not yet implemented");
        return false;
    }

    std::vector<uint32_t> PhysXBackend::OverlapSphere(const Math::Vec3& center, float radius) const {
        LOG_WARNING("PhysXBackend::OverlapSphere: Not yet implemented");
        return {};
    }

    std::vector<uint32_t> PhysXBackend::OverlapBox(const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation) const {
        LOG_WARNING("PhysXBackend::OverlapBox: Not yet implemented");
        return {};
    }

    std::vector<uint32_t> PhysXBackend::OverlapCapsule(const Math::Vec3& point1, const Math::Vec3& point2, float radius) const {
        LOG_WARNING("PhysXBackend::OverlapCapsule: Not yet implemented");
        return {};
    }

    void PhysXBackend::SetSolverIterations(uint32_t positionIterations, uint32_t velocityIterations) {
        LOG_WARNING("PhysXBackend::SetSolverIterations: Not yet implemented");
    }

    void PhysXBackend::SetSimulationThreads(uint32_t threadCount) {
        LOG_WARNING("PhysXBackend::SetSimulationThreads: Not yet implemented");
    }

    void PhysXBackend::EnableContinuousCollisionDetection(bool enable) {
        LOG_WARNING("PhysXBackend::EnableContinuousCollisionDetection: Not yet implemented");
    }

} // namespace GameEngine