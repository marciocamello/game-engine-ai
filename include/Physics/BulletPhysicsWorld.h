#pragma once

#include "Physics/PhysicsEngine.h"
#include "Core/Math.h"
#include <memory>
#include <unordered_map>

#ifdef GAMEENGINE_HAS_BULLET
#include <btBulletDynamicsCommon.h>

namespace GameEngine {
    
    /**
     * @brief Bullet Physics implementation of PhysicsWorld
     * 
     * This class wraps Bullet Physics components to provide a concrete
     * implementation of the PhysicsWorld interface. It manages the Bullet
     * dynamics world and all associated components.
     */
    class BulletPhysicsWorld : public PhysicsWorld {
    public:
        /**
         * @brief Construct a new BulletPhysicsWorld
         * @param gravity The gravity vector for the physics world
         */
        explicit BulletPhysicsWorld(const Math::Vec3& gravity);
        
        /**
         * @brief Construct a new BulletPhysicsWorld with configuration
         * @param config The physics configuration
         */
        explicit BulletPhysicsWorld(const PhysicsConfiguration& config);
        
        /**
         * @brief Destroy the BulletPhysicsWorld and clean up all resources
         */
        ~BulletPhysicsWorld() override;
        
        /**
         * @brief Step the physics simulation forward by deltaTime
         * @param deltaTime Time step in seconds
         */
        void Step(float deltaTime) override;
        
        /**
         * @brief Set the gravity for the physics world
         * @param gravity New gravity vector
         */
        void SetGravity(const Math::Vec3& gravity) override;
        
        /**
         * @brief Get the current gravity vector
         * @return Current gravity vector
         */
        const Math::Vec3& GetGravity() const override;
        
        /**
         * @brief Get the underlying Bullet dynamics world
         * @return Pointer to the btDiscreteDynamicsWorld
         */
        btDiscreteDynamicsWorld* GetBulletWorld() const { return m_dynamicsWorld.get(); }
        
        /**
         * @brief Add a rigid body to the physics world
         * @param bodyId The ID of the rigid body
         * @param rigidBody Pointer to the Bullet rigid body
         */
        void AddRigidBody(uint32_t bodyId, btRigidBody* rigidBody);
        
        /**
         * @brief Remove a rigid body from the physics world
         * @param bodyId The ID of the rigid body to remove
         */
        void RemoveRigidBody(uint32_t bodyId);
        
        /**
         * @brief Get a rigid body by ID
         * @param bodyId The ID of the rigid body
         * @return Pointer to the rigid body, or nullptr if not found
         */
        btRigidBody* GetRigidBody(uint32_t bodyId) const;
        
        /**
         * @brief Set the physics configuration for this world
         * @param config The new physics configuration
         */
        void SetConfiguration(const PhysicsConfiguration& config);
        
        /**
         * @brief Set the timestep for physics simulation
         * @param timeStep The new timestep in seconds
         */
        void SetTimeStep(float timeStep);
        
        /**
         * @brief Set the number of solver iterations
         * @param iterations Number of constraint solver iterations
         */
        void SetSolverIterations(int iterations);
        
        /**
         * @brief Set contact thresholds for collision detection
         * @param breakingThreshold Contact breaking threshold
         * @param processingThreshold Contact processing threshold
         */
        void SetContactThresholds(float breakingThreshold, float processingThreshold);
        
    private:
        // Bullet Physics components
        std::unique_ptr<btBroadphaseInterface> m_broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;
        
        // Body management
        std::unordered_map<uint32_t, btRigidBody*> m_rigidBodies;
        
        // Configuration storage
        PhysicsConfiguration m_configuration;
        
        // Cached gravity for GetGravity()
        Math::Vec3 m_gravity;
        
        /**
         * @brief Initialize all Bullet Physics components
         */
        void InitializeBulletComponents();
        
        /**
         * @brief Clean up all Bullet Physics resources
         */
        void CleanupBulletComponents();
    };
    
} // namespace GameEngine

#endif // GAMEENGINE_HAS_BULLET