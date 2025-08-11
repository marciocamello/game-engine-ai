#pragma once

#include "../../engine/core/Math.h"
#include <vector>
#include <memory>
#include <unordered_map>

#ifdef GAMEENGINE_HAS_BULLET
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#endif

namespace GameEngine {
    namespace Physics {
        class IPhysicsDebugDrawer;
        class BulletDebugDrawer;
        enum class PhysicsDebugMode;
    }
}

namespace GameEngine {
    /**
     * @brief Configuration parameters for physics simulation
     */
    struct PhysicsConfiguration {
        Math::Vec3 gravity{0.0f, -9.81f, 0.0f};  ///< Gravity vector (m/s²)
        float timeStep = 1.0f / 60.0f;           ///< Fixed timestep for simulation (seconds)
        int maxSubSteps = 10;                    ///< Maximum number of substeps per frame
        int solverIterations = 10;               ///< Number of constraint solver iterations
        bool enableCCD = true;                   ///< Enable Continuous Collision Detection
        float linearDamping = 0.0f;              ///< Default linear damping for rigid bodies
        float angularDamping = 0.0f;             ///< Default angular damping for rigid bodies
        float contactBreakingThreshold = 0.02f;  ///< Contact breaking threshold
        float contactProcessingThreshold = 0.01f; ///< Contact processing threshold
        
        /**
         * @brief Create default physics configuration
         */
        static PhysicsConfiguration Default() {
            return PhysicsConfiguration{};
        }
        
        /**
         * @brief Create configuration optimized for character movement
         */
        static PhysicsConfiguration ForCharacterMovement() {
            PhysicsConfiguration config;
            config.gravity = Math::Vec3(0.0f, -9.81f, 0.0f);
            config.timeStep = 1.0f / 60.0f;
            config.maxSubSteps = 10;
            config.solverIterations = 15; // Higher for better character stability
            config.enableCCD = true;
            config.linearDamping = 0.1f;  // Some damping for realistic movement
            config.angularDamping = 0.1f;
            return config;
        }
        
        /**
         * @brief Create configuration optimized for high precision simulation
         */
        static PhysicsConfiguration HighPrecision() {
            PhysicsConfiguration config;
            config.gravity = Math::Vec3(0.0f, -9.81f, 0.0f);
            config.timeStep = 1.0f / 120.0f; // Higher frequency
            config.maxSubSteps = 20;
            config.solverIterations = 20;    // More iterations for accuracy
            config.enableCCD = true;
            config.contactBreakingThreshold = 0.01f;
            config.contactProcessingThreshold = 0.005f;
            return config;
        }
    };
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

    struct RigidBody {
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

    struct CollisionShape {
        enum Type {
            Box,
            Sphere,
            Capsule,
            Mesh
        } type = Box;
        
        Math::Vec3 dimensions{1.0f}; // For box: width, height, depth; For sphere: radius, 0, 0
    };

    class PhysicsWorld;

    class PhysicsEngine {
    public:
        PhysicsEngine();
        ~PhysicsEngine();

        bool Initialize(const PhysicsConfiguration& config = PhysicsConfiguration::Default());
        void Shutdown();
        void Update(float deltaTime);

        // Configuration management
        void SetConfiguration(const PhysicsConfiguration& config);
        const PhysicsConfiguration& GetConfiguration() const { return m_configuration; }
        
        // Runtime parameter modification
        void SetGravity(const Math::Vec3& gravity);
        void SetTimeStep(float timeStep);
        void SetSolverIterations(int iterations);
        void SetContactThresholds(float breakingThreshold, float processingThreshold);

        // World management
        std::shared_ptr<PhysicsWorld> CreateWorld(const Math::Vec3& gravity = Math::Vec3(0.0f, -9.81f, 0.0f));
        std::shared_ptr<PhysicsWorld> CreateWorld(const PhysicsConfiguration& config);
        void SetActiveWorld(std::shared_ptr<PhysicsWorld> world);

        // Rigid body management
        uint32_t CreateRigidBody(const RigidBody& bodyDesc, const CollisionShape& shape);
        void DestroyRigidBody(uint32_t bodyId);
        void SetRigidBodyTransform(uint32_t bodyId, const Math::Vec3& position, const Math::Quat& rotation);
        void ApplyForce(uint32_t bodyId, const Math::Vec3& force);
        void ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse);
        void SetAngularFactor(uint32_t bodyId, const Math::Vec3& factor);
        void SetLinearDamping(uint32_t bodyId, float damping);
        void SetAngularDamping(uint32_t bodyId, float damping);

        // Rigid body queries
        bool GetRigidBodyTransform(uint32_t bodyId, Math::Vec3& position, Math::Quat& rotation);
        bool GetRigidBodyVelocity(uint32_t bodyId, Math::Vec3& velocity, Math::Vec3& angularVelocity);
        bool IsRigidBodyGrounded(uint32_t bodyId, float groundCheckDistance = 0.1f);

        // Queries
        RaycastHit Raycast(const Math::Vec3& origin, const Math::Vec3& direction, float maxDistance);
        std::vector<OverlapResult> OverlapSphere(const Math::Vec3& center, float radius);
        
        // Sweep tests for character controller
        struct SweepHit {
            bool hasHit = false;
            uint32_t bodyId = 0;
            Math::Vec3 point{0.0f};
            Math::Vec3 normal{0.0f};
            float distance = 0.0f;
            float fraction = 0.0f; // 0.0 = start, 1.0 = end
        };
        
        SweepHit SweepCapsule(const Math::Vec3& from, const Math::Vec3& to, float radius, float height);
        
        // Ghost object management for kinematic collision detection
        uint32_t CreateGhostObject(const CollisionShape& shape, const Math::Vec3& position);
        void DestroyGhostObject(uint32_t ghostId);
        void SetGhostObjectTransform(uint32_t ghostId, const Math::Vec3& position, const Math::Quat& rotation);
        std::vector<OverlapResult> GetGhostObjectOverlaps(uint32_t ghostId);

        // Debug visualization
        void SetDebugDrawer(std::shared_ptr<Physics::IPhysicsDebugDrawer> drawer);
        void SetDebugMode(Physics::PhysicsDebugMode mode);
        Physics::PhysicsDebugMode GetDebugMode() const;
        void EnableDebugDrawing(bool enabled);
        bool IsDebugDrawingEnabled() const;
        void DrawDebugWorld();
        
        // Debug console commands
        struct PhysicsDebugInfo {
            int numRigidBodies = 0;
            int numGhostObjects = 0;
            int numActiveObjects = 0;
            int numSleepingObjects = 0;
            float simulationTime = 0.0f;
            Math::Vec3 worldGravity{0.0f};
        };
        
        PhysicsDebugInfo GetDebugInfo() const;
        void PrintDebugInfo() const;

    private:
        std::shared_ptr<PhysicsWorld> m_activeWorld;
        uint32_t m_nextBodyId = 1;
        PhysicsConfiguration m_configuration;
        
        // Debug visualization
        std::shared_ptr<Physics::IPhysicsDebugDrawer> m_debugDrawer;
        Physics::PhysicsDebugMode m_debugMode;
        bool m_debugDrawingEnabled = false;
        
#ifdef GAMEENGINE_HAS_BULLET
        // Mapping from body ID to Bullet rigid body for direct access
        std::unordered_map<uint32_t, btRigidBody*> m_bulletBodies;
        // Mapping from ghost ID to Bullet ghost object for collision detection
        std::unordered_map<uint32_t, btGhostObject*> m_bulletGhostObjects;
        // Bullet debug drawer
        std::unique_ptr<Physics::BulletDebugDrawer> m_bulletDebugDrawer;
#endif
    };

    class PhysicsWorld {
    public:
        PhysicsWorld(const Math::Vec3& gravity);
        virtual ~PhysicsWorld();

        virtual void SetGravity(const Math::Vec3& gravity) { m_gravity = gravity; }
        virtual const Math::Vec3& GetGravity() const { return m_gravity; }

        virtual void Step(float deltaTime);

    private:
        Math::Vec3 m_gravity;
    };
}