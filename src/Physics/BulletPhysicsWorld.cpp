#include "Physics/BulletPhysicsWorld.h"
#include "Physics/BulletUtils.h"
#include "Core/Logger.h"

#ifdef GAMEENGINE_HAS_BULLET

namespace GameEngine {
    
    BulletPhysicsWorld::BulletPhysicsWorld(const Math::Vec3& gravity) 
        : PhysicsWorld(gravity), m_gravity(gravity) {
        // Initialize with default configuration but override gravity
        m_configuration = PhysicsConfiguration::Default();
        m_configuration.gravity = gravity;
        
        InitializeBulletComponents();
        SetGravity(gravity);
        LOG_INFO("BulletPhysicsWorld created with gravity");
    }
    
    BulletPhysicsWorld::BulletPhysicsWorld(const PhysicsConfiguration& config)
        : PhysicsWorld(config.gravity), m_configuration(config), m_gravity(config.gravity) {
        InitializeBulletComponents();
        SetConfiguration(config);
        LOG_INFO("BulletPhysicsWorld created with configuration");
    }
    
    BulletPhysicsWorld::~BulletPhysicsWorld() {
        LOG_INFO("BulletPhysicsWorld destructor called");
        CleanupBulletComponents();
    }
    
    void BulletPhysicsWorld::InitializeBulletComponents() {
        // Create broadphase
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        
        // Create collision configuration
        m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        
        // Create collision dispatcher
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
        
        // Create constraint solver
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        
        // Create dynamics world
        m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
            m_dispatcher.get(),
            m_broadphase.get(),
            m_solver.get(),
            m_collisionConfig.get()
        );
        
        LOG_INFO("Bullet Physics components initialized successfully");
    }
    
    void BulletPhysicsWorld::CleanupBulletComponents() {
        if (m_dynamicsWorld) {
            // Remove all rigid bodies from the world
            for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
                btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    delete body->getMotionState();
                }
                m_dynamicsWorld->removeCollisionObject(obj);
                delete obj;
            }
            
            // Clear our body mapping
            m_rigidBodies.clear();
            
            LOG_INFO("Cleaned up rigid bodies from Bullet world");
        }
        
        // Reset all components in reverse order of creation
        m_dynamicsWorld.reset();
        m_solver.reset();
        m_dispatcher.reset();
        m_collisionConfig.reset();
        m_broadphase.reset();
        
        LOG_INFO("Bullet Physics components cleaned up");
    }
    
    void BulletPhysicsWorld::Step(float deltaTime) {
        if (!m_dynamicsWorld) {
            LOG_ERROR("Cannot step physics: Bullet dynamics world is null");
            return;
        }
        
        // Step the simulation using configuration parameters
        m_dynamicsWorld->stepSimulation(deltaTime, m_configuration.maxSubSteps, m_configuration.timeStep);
    }
    
    void BulletPhysicsWorld::SetGravity(const Math::Vec3& gravity) {
        m_gravity = gravity;
        
        if (m_dynamicsWorld) {
            btVector3 bulletGravity = Physics::BulletUtils::ToBullet(gravity);
            m_dynamicsWorld->setGravity(bulletGravity);
            LOG_DEBUG("Bullet world gravity set");
        }
    }
    
    const Math::Vec3& BulletPhysicsWorld::GetGravity() const {
        return m_gravity;
    }
    
    void BulletPhysicsWorld::AddRigidBody(uint32_t bodyId, btRigidBody* rigidBody) {
        if (!m_dynamicsWorld) {
            LOG_ERROR("Cannot add rigid body: Bullet dynamics world is null");
            return;
        }
        
        if (!rigidBody) {
            LOG_ERROR("Cannot add null rigid body");
            return;
        }
        
        // Check if body ID already exists
        if (m_rigidBodies.find(bodyId) != m_rigidBodies.end()) {
            LOG_WARNING("Rigid body already exists, replacing");
            RemoveRigidBody(bodyId);
        }
        
        // Add to Bullet world
        m_dynamicsWorld->addRigidBody(rigidBody);
        
        // Store in our mapping
        m_rigidBodies[bodyId] = rigidBody;
        
        LOG_DEBUG("Added rigid body to Bullet world");
    }
    
    void BulletPhysicsWorld::RemoveRigidBody(uint32_t bodyId) {
        auto it = m_rigidBodies.find(bodyId);
        if (it == m_rigidBodies.end()) {
            LOG_WARNING("Attempted to remove non-existent rigid body");
            return;
        }
        
        btRigidBody* rigidBody = it->second;
        
        if (m_dynamicsWorld && rigidBody) {
            // Remove from Bullet world
            m_dynamicsWorld->removeRigidBody(rigidBody);
            LOG_DEBUG("Removed rigid body from Bullet world");
        }
        
        // Remove from our mapping
        m_rigidBodies.erase(it);
    }
    
    btRigidBody* BulletPhysicsWorld::GetRigidBody(uint32_t bodyId) const {
        auto it = m_rigidBodies.find(bodyId);
        if (it != m_rigidBodies.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    void BulletPhysicsWorld::SetConfiguration(const PhysicsConfiguration& config) {
        m_configuration = config;
        
        // Apply configuration to the Bullet world
        SetGravity(config.gravity);
        
        if (m_dynamicsWorld) {
            // Set solver iterations through the solver info
            btContactSolverInfo& solverInfo = m_dynamicsWorld->getSolverInfo();
            solverInfo.m_numIterations = config.solverIterations;
            
            // Set contact thresholds through solver info
            solverInfo.m_splitImpulse = config.enableCCD;
            
            LOG_DEBUG("Applied physics configuration to Bullet world");
        }
    }
    
    void BulletPhysicsWorld::SetTimeStep(float timeStep) {
        m_configuration.timeStep = timeStep;
        LOG_DEBUG("Updated timestep in BulletPhysicsWorld to: " + std::to_string(timeStep));
    }
    
    void BulletPhysicsWorld::SetSolverIterations(int iterations) {
        m_configuration.solverIterations = iterations;
        
        if (m_dynamicsWorld) {
            btContactSolverInfo& solverInfo = m_dynamicsWorld->getSolverInfo();
            solverInfo.m_numIterations = iterations;
            LOG_DEBUG("Updated solver iterations in BulletPhysicsWorld to: " + std::to_string(iterations));
        }
    }
    
    void BulletPhysicsWorld::SetContactThresholds(float breakingThreshold, float processingThreshold) {
        m_configuration.contactBreakingThreshold = breakingThreshold;
        m_configuration.contactProcessingThreshold = processingThreshold;
        
        // Set contact thresholds through solver info
        if (m_dynamicsWorld) {
            btContactSolverInfo& solverInfo = m_dynamicsWorld->getSolverInfo();
            // Note: Bullet Physics handles contact thresholds internally
            // These values are stored in configuration for reference
        }
        
        LOG_DEBUG("Updated contact thresholds in BulletPhysicsWorld");
    }
    
} // namespace GameEngine

#endif // GAMEENGINE_HAS_BULLET