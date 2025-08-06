#pragma once

#include "IPhysicsBackend.h"
#include <unordered_map>
#include <memory>

#ifdef GAMEENGINE_HAS_PHYSX
#include <PxPhysicsAPI.h>
#endif

namespace GameEngine {

    /**
     * @brief NVIDIA PhysX physics backend implementation
     * 
     * This class implements the IPhysicsBackend interface using NVIDIA PhysX
     * for high-performance physics simulation with optional GPU acceleration.
     */
    class PhysXBackend : public IPhysicsBackend {
    public:
        PhysXBackend();
        ~PhysXBackend();

        // IPhysicsBackend implementation
        bool Initialize(const PhysicsConfig& config) override;
        void Shutdown() override;
        void Update(float deltaTime) override;

        // World management
        void SetGravity(const Math::Vec3& gravity) override;
        Math::Vec3 GetGravity() const override;

        // Rigid body management
        uint32_t CreateRigidBody(const RigidBodyDesc& desc, const CollisionShapeDesc& shape) override;
        void DestroyRigidBody(uint32_t bodyId) override;
        void SetRigidBodyTransform(uint32_t bodyId, const Math::Vec3& position, const Math::Quat& rotation) override;
        void GetRigidBodyTransform(uint32_t bodyId, Math::Vec3& position, Math::Quat& rotation) const override;
        void ApplyForce(uint32_t bodyId, const Math::Vec3& force, const Math::Vec3& point = Math::Vec3(0.0f)) override;
        void ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse, const Math::Vec3& point = Math::Vec3(0.0f)) override;

        // Collision detection
        bool Raycast(const Ray& ray, RaycastHit& hit) const override;
        bool SphereCast(const Math::Vec3& origin, float radius, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const override;
        bool BoxCast(const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const override;
        bool CapsuleCast(const Math::Vec3& point1, const Math::Vec3& point2, float radius, const Math::Vec3& direction, float maxDistance, RaycastHit& hit) const override;

        // Overlap queries
        std::vector<uint32_t> OverlapSphere(const Math::Vec3& center, float radius) const override;
        std::vector<uint32_t> OverlapBox(const Math::Vec3& center, const Math::Vec3& halfExtents, const Math::Quat& rotation) const override;
        std::vector<uint32_t> OverlapCapsule(const Math::Vec3& point1, const Math::Vec3& point2, float radius) const override;

        // Performance and debugging
        PhysicsStats GetStats() const override;
        void SetDebugVisualization(bool enabled) override;
        void DrawDebugVisualization() const override;

        // Backend-specific information
        const char* GetBackendName() const override;
        const char* GetBackendVersion() const override;
        bool SupportsGPUAcceleration() const override;

        // PhysX-specific methods
        bool InitializePhysXFoundation();
        bool InitializePhysXPhysics();
        bool InitializePhysXScene();
        bool InitializeGPUAcceleration();

        void SetSolverIterations(uint32_t positionIterations, uint32_t velocityIterations);
        void SetSimulationThreads(uint32_t threadCount);
        void EnableContinuousCollisionDetection(bool enable);

    private:
#ifdef GAMEENGINE_HAS_PHYSX
        // PhysX core objects
        physx::PxFoundation* m_foundation = nullptr;
        physx::PxPhysics* m_physics = nullptr;
        physx::PxScene* m_scene = nullptr;
        physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
        physx::PxCudaContextManager* m_cudaContextManager = nullptr;

        // PhysX configuration
        physx::PxTolerancesScale m_toleranceScale;
        physx::PxSceneDesc* m_sceneDesc = nullptr;

        // Object management
        std::unordered_map<uint32_t, physx::PxRigidActor*> m_rigidBodies;
        uint32_t m_nextBodyId = 1;

        // Error and allocation callbacks
        class PhysXErrorCallback;
        class PhysXAllocatorCallback;
        std::unique_ptr<PhysXErrorCallback> m_errorCallback;
        std::unique_ptr<PhysXAllocatorCallback> m_allocatorCallback;
#endif

        // Configuration
        PhysicsConfig m_config;
        bool m_initialized = false;
        bool m_gpuAccelerationEnabled = false;

        // Performance monitoring
        mutable PhysicsStats m_stats;

        // Helper methods
        void CleanupPhysX();
        bool IsPhysXAvailable() const;

#ifdef GAMEENGINE_HAS_PHYSX
        physx::PxRigidDynamic* CreateDynamicRigidBody(const RigidBodyDesc& desc, const CollisionShapeDesc& shapeDesc);
        physx::PxRigidStatic* CreateStaticRigidBody(const RigidBodyDesc& desc, const CollisionShapeDesc& shapeDesc);
        physx::PxShape* CreateCollisionShape(const CollisionShapeDesc& desc);
        physx::PxMaterial* GetOrCreateMaterial(float staticFriction, float dynamicFriction, float restitution);
#endif
    };

} // namespace GameEngine