#pragma once

#include "Core/Math.h"
#include <vector>
#include <memory>
#include <string>

namespace GameEngine {
    
    struct RaycastHit {
        bool hasHit = false;
        uint32_t bodyId = 0;
        Math::Vec3 point{0.0f};
        Math::Vec3 normal{0.0f};
        float distance = 0.0f;
    };

    struct OverlapResult {
        uint32_t bodyId = 0;
        Math::Vec3 contactPoint{0.0f};
        Math::Vec3 contactNormal{0.0f};
        float penetrationDepth = 0.0f;
    };

    struct RigidBodyDesc {
        Math::Vec3 position{0.0f};
        Math::Quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        Math::Vec3 velocity{0.0f};
        Math::Vec3 angularVelocity{0.0f};
        float mass = 1.0f;
        float restitution = 0.5f;
        float friction = 0.5f;
        bool isStatic = false;
        bool isKinematic = false;
    };

    struct CollisionShapeDesc {
        enum Type {
            Box,
            Sphere,
            Capsule,
            Mesh
        } type = Box;
        
        Math::Vec3 dimensions{1.0f}; // For box: width, height, depth; For sphere: radius, 0, 0
    };

    struct PhysicsConfig {
        Math::Vec3 gravity{0.0f, -9.81f, 0.0f};
        float timeStep = 1.0f / 60.0f;
        int maxSubSteps = 10;
        int solverIterations = 10;
        bool enableCCD = true;
        float linearDamping = 0.0f;
        float angularDamping = 0.0f;
        bool enableGPUAcceleration = true;
        size_t gpuMemoryBudgetMB = 512;
        uint32_t simulationThreads = 0; // 0 = auto-detect
    };

    struct PhysicsStats {
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

    struct Ray {
        Math::Vec3 origin{0.0f};
        Math::Vec3 direction{0.0f, 0.0f, 1.0f};
    };

    /**
     * @brief Abstract interface for physics backends
     * 
     * This interface provides a unified API for different physics engines
     * (Bullet Physics, NVIDIA PhysX, etc.) allowing seamless switching
     * between backends while maintaining the same functionality.
     */
    class IPhysicsBackend {
    public:
        virtual ~IPhysicsBackend() = default;

        // Lifecycle management
        virtual bool Initialize(const PhysicsConfig& config) = 0;
        virtual void Shutdown() = 0;
        virtual void Update(float deltaTime) = 0;

        // World management
        virtual void SetGravity(const Math::Vec3& gravity) = 0;
        virtual Math::Vec3 GetGravity() const = 0;

        // Rigid body management
        virtual uint32_t CreateRigidBody(const RigidBodyDesc& desc, const CollisionShapeDesc& shape) = 0;
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

} // namespace GameEngine