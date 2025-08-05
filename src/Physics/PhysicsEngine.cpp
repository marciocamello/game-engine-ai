#include "Physics/PhysicsEngine.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "Core/Logger.h"

#ifdef GAMEENGINE_HAS_BULLET
#include "Physics/BulletPhysicsWorld.h"
#include "Physics/BulletUtils.h"
#include "Physics/CollisionShapeFactory.h"
#endif

namespace GameEngine {
    PhysicsEngine::PhysicsEngine() 
        : m_debugMode(Physics::PhysicsDebugMode::None), m_debugDrawingEnabled(false) {
    }

    PhysicsEngine::~PhysicsEngine() {
        Shutdown();
    }

    bool PhysicsEngine::Initialize(const PhysicsConfiguration& config) {
        m_configuration = config;
        
#ifdef GAMEENGINE_HAS_BULLET
        LOG_INFO("Physics Engine initialized with Bullet Physics support");
        LOG_DEBUG("Physics configuration - Gravity: (" + 
                 std::to_string(config.gravity.x) + ", " + 
                 std::to_string(config.gravity.y) + ", " + 
                 std::to_string(config.gravity.z) + ")");
        LOG_DEBUG("Physics configuration - TimeStep: " + std::to_string(config.timeStep) + 
                 ", MaxSubSteps: " + std::to_string(config.maxSubSteps) + 
                 ", SolverIterations: " + std::to_string(config.solverIterations));
        
        // Create and set default physics world
        m_activeWorld = CreateWorld(config);
        if (!m_activeWorld) {
            LOG_ERROR("Failed to create default physics world");
            return false;
        }
        
        LOG_INFO("Default physics world created and activated");
        return true;
#else
        LOG_WARNING("Physics Engine initialized without Bullet Physics support");
        return false;
#endif
    }

    void PhysicsEngine::Shutdown() {
#ifdef GAMEENGINE_HAS_BULLET
        m_bulletBodies.clear();
        m_bulletGhostObjects.clear();
#endif
        m_activeWorld.reset();
        LOG_INFO("Physics Engine shutdown");
    }

    void PhysicsEngine::Update(float deltaTime) {
        if (m_activeWorld) {
            m_activeWorld->Step(deltaTime);
        }
    }

    void PhysicsEngine::SetConfiguration(const PhysicsConfiguration& config) {
        m_configuration = config;
        
        // Apply configuration to active world if it exists
        if (m_activeWorld) {
            m_activeWorld->SetGravity(config.gravity);
            
#ifdef GAMEENGINE_HAS_BULLET
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                bulletWorldPtr->SetConfiguration(config);
            }
#endif
        }
        
        LOG_DEBUG("Physics configuration updated");
    }

    void PhysicsEngine::SetGravity(const Math::Vec3& gravity) {
        m_configuration.gravity = gravity;
        
        if (m_activeWorld) {
            m_activeWorld->SetGravity(gravity);
        }
        
        LOG_DEBUG("Physics gravity updated");
    }

    void PhysicsEngine::SetTimeStep(float timeStep) {
        m_configuration.timeStep = timeStep;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_activeWorld) {
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                bulletWorldPtr->SetTimeStep(timeStep);
            }
        }
#endif
        
        LOG_DEBUG("Physics timestep updated to: " + std::to_string(timeStep));
    }

    void PhysicsEngine::SetSolverIterations(int iterations) {
        m_configuration.solverIterations = iterations;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_activeWorld) {
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                bulletWorldPtr->SetSolverIterations(iterations);
            }
        }
#endif
        
        LOG_DEBUG("Physics solver iterations updated to: " + std::to_string(iterations));
    }

    void PhysicsEngine::SetContactThresholds(float breakingThreshold, float processingThreshold) {
        m_configuration.contactBreakingThreshold = breakingThreshold;
        m_configuration.contactProcessingThreshold = processingThreshold;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_activeWorld) {
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                bulletWorldPtr->SetContactThresholds(breakingThreshold, processingThreshold);
            }
        }
#endif
        
        LOG_DEBUG("Physics contact thresholds updated");
    }

    std::shared_ptr<PhysicsWorld> PhysicsEngine::CreateWorld(const Math::Vec3& gravity) {
#ifdef GAMEENGINE_HAS_BULLET
        return std::make_shared<BulletPhysicsWorld>(gravity);
#else
        return std::make_shared<PhysicsWorld>(gravity);
#endif
    }

    std::shared_ptr<PhysicsWorld> PhysicsEngine::CreateWorld(const PhysicsConfiguration& config) {
#ifdef GAMEENGINE_HAS_BULLET
        return std::make_shared<BulletPhysicsWorld>(config);
#else
        return std::make_shared<PhysicsWorld>(config.gravity);
#endif
    }

    void PhysicsEngine::SetActiveWorld(std::shared_ptr<PhysicsWorld> world) {
        m_activeWorld = world;
    }

    uint32_t PhysicsEngine::CreateRigidBody(const RigidBody& bodyDesc, const CollisionShape& shape) {
        uint32_t id = m_nextBodyId++;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_activeWorld) {
            // Create Bullet collision shape
            auto bulletShape = Physics::CollisionShapeFactory::CreateShape(shape);
            if (!bulletShape) {
                LOG_ERROR("Failed to create collision shape for rigid body");
                return 0;
            }
            
            // Calculate local inertia
            btVector3 localInertia(0, 0, 0);
            if (!bodyDesc.isStatic && bodyDesc.mass > 0.0f) {
                bulletShape->calculateLocalInertia(bodyDesc.mass, localInertia);
            }
            
            // Create motion state
            btTransform startTransform = Physics::BulletUtils::ToBullet(bodyDesc.position, bodyDesc.rotation);
            auto motionState = std::make_unique<btDefaultMotionState>(startTransform);
            
            // Create rigid body construction info
            btRigidBody::btRigidBodyConstructionInfo rbInfo(
                bodyDesc.isStatic ? 0.0f : bodyDesc.mass,
                motionState.release(),
                bulletShape.release(),
                localInertia
            );
            
            // Set material properties
            rbInfo.m_restitution = bodyDesc.restitution;
            rbInfo.m_friction = bodyDesc.friction;
            
            // Create the rigid body
            auto bulletBody = std::make_unique<btRigidBody>(rbInfo);
            
            // Set kinematic flag if needed
            if (bodyDesc.isKinematic) {
                bulletBody->setCollisionFlags(bulletBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                bulletBody->setActivationState(DISABLE_DEACTIVATION);
            }
            
            // Set initial velocity
            if (!bodyDesc.isStatic) {
                bulletBody->setLinearVelocity(Physics::BulletUtils::ToBullet(bodyDesc.velocity));
                bulletBody->setAngularVelocity(Physics::BulletUtils::ToBullet(bodyDesc.angularVelocity));
            }
            
            // Add to the world
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                btRigidBody* rawBodyPtr = bulletBody.release();
                bulletWorldPtr->AddRigidBody(id, rawBodyPtr);
                m_bulletBodies[id] = rawBodyPtr;
                LOG_DEBUG("Created Bullet rigid body with ID: " + std::to_string(id));
            } else {
                LOG_ERROR("Active world is not a BulletPhysicsWorld");
                return 0;
            }
        }
#endif
        
        return id;
    }

    void PhysicsEngine::DestroyRigidBody(uint32_t bodyId) {
#ifdef GAMEENGINE_HAS_BULLET
        // Remove from Bullet world if it exists
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            
            // Remove from the world
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                bulletWorldPtr->RemoveRigidBody(bodyId);
            }
            
            // Clean up the rigid body and its components
            if (bulletBody) {
                // Delete motion state
                if (bulletBody->getMotionState()) {
                    delete bulletBody->getMotionState();
                }
                
                // Delete collision shape
                if (bulletBody->getCollisionShape()) {
                    delete bulletBody->getCollisionShape();
                }
                
                // Delete the rigid body itself
                delete bulletBody;
            }
            
            m_bulletBodies.erase(bulletBodyIt);
            LOG_DEBUG("Destroyed Bullet rigid body with ID: " + std::to_string(bodyId));
        } else {
            LOG_WARNING("Attempted to destroy non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#else
        LOG_WARNING("Attempted to destroy rigid body but Bullet Physics not available");
#endif
    }

    void PhysicsEngine::SetRigidBodyTransform(uint32_t bodyId, const Math::Vec3& position, const Math::Quat& rotation) {
#ifdef GAMEENGINE_HAS_BULLET
        // Update Bullet rigid body
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody) {
                btTransform transform = Physics::BulletUtils::ToBullet(position, rotation);
                
                // For kinematic bodies, use setWorldTransform
                if (bulletBody->isKinematicObject()) {
                    bulletBody->setWorldTransform(transform);
                    bulletBody->getMotionState()->setWorldTransform(transform);
                } else {
                    // For dynamic bodies, use motion state
                    bulletBody->setWorldTransform(transform);
                    if (bulletBody->getMotionState()) {
                        bulletBody->getMotionState()->setWorldTransform(transform);
                    }
                    bulletBody->activate(true);
                }
                
                LOG_DEBUG("Updated transform for rigid body with ID: " + std::to_string(bodyId));
            }
        } else {
            LOG_WARNING("Attempted to set transform for non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#else
        LOG_WARNING("Attempted to set transform but Bullet Physics not available");
#endif
    }

    void PhysicsEngine::ApplyForce(uint32_t bodyId, const Math::Vec3& force) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody && !bulletBody->isStaticObject()) {
                btVector3 bulletForce = Physics::BulletUtils::ToBullet(force);
                bulletBody->applyCentralForce(bulletForce);
                bulletBody->activate(true);
                LOG_DEBUG("Applied force to rigid body with ID: " + std::to_string(bodyId));
            }
        } else {
            LOG_WARNING("Attempted to apply force to non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#endif
    }

    void PhysicsEngine::ApplyImpulse(uint32_t bodyId, const Math::Vec3& impulse) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody && !bulletBody->isStaticObject()) {
                btVector3 bulletImpulse = Physics::BulletUtils::ToBullet(impulse);
                bulletBody->applyCentralImpulse(bulletImpulse);
                bulletBody->activate(true);
                LOG_DEBUG("Applied impulse to rigid body with ID: " + std::to_string(bodyId));
            }
        } else {
            LOG_WARNING("Attempted to apply impulse to non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#endif
    }

    void PhysicsEngine::SetAngularFactor(uint32_t bodyId, const Math::Vec3& factor) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody && !bulletBody->isStaticObject()) {
                btVector3 bulletFactor = Physics::BulletUtils::ToBullet(factor);
                bulletBody->setAngularFactor(bulletFactor);
                LOG_DEBUG("Set angular factor for rigid body with ID: " + std::to_string(bodyId));
            }
        } else {
            LOG_WARNING("Attempted to set angular factor for non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#endif
    }

    void PhysicsEngine::SetLinearDamping(uint32_t bodyId, float damping) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody && !bulletBody->isStaticObject()) {
                bulletBody->setDamping(damping, bulletBody->getAngularDamping());
                LOG_DEBUG("Set linear damping for rigid body with ID: " + std::to_string(bodyId) + " to " + std::to_string(damping));
            }
        } else {
            LOG_WARNING("Attempted to set linear damping for non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#endif
    }

    void PhysicsEngine::SetAngularDamping(uint32_t bodyId, float damping) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody && !bulletBody->isStaticObject()) {
                bulletBody->setDamping(bulletBody->getLinearDamping(), damping);
                LOG_DEBUG("Set angular damping for rigid body with ID: " + std::to_string(bodyId) + " to " + std::to_string(damping));
            }
        } else {
            LOG_WARNING("Attempted to set angular damping for non-existent rigid body with ID: " + std::to_string(bodyId));
        }
#endif
    }

    bool PhysicsEngine::GetRigidBodyTransform(uint32_t bodyId, Math::Vec3& position, Math::Quat& rotation) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody) {
                btTransform transform = bulletBody->getWorldTransform();
                position = Physics::BulletUtils::FromBullet(transform.getOrigin());
                rotation = Physics::BulletUtils::FromBullet(transform.getRotation());
                return true;
            }
        }
#endif
        return false;
    }

    bool PhysicsEngine::GetRigidBodyVelocity(uint32_t bodyId, Math::Vec3& velocity, Math::Vec3& angularVelocity) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody) {
                velocity = Physics::BulletUtils::FromBullet(bulletBody->getLinearVelocity());
                angularVelocity = Physics::BulletUtils::FromBullet(bulletBody->getAngularVelocity());
                return true;
            }
        }
#endif
        return false;
    }

    bool PhysicsEngine::IsRigidBodyGrounded(uint32_t bodyId, float groundCheckDistance) {
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletBodyIt = m_bulletBodies.find(bodyId);
        if (bulletBodyIt != m_bulletBodies.end()) {
            btRigidBody* bulletBody = bulletBodyIt->second;
            if (bulletBody) {
                // Get the body's current position
                btTransform transform = bulletBody->getWorldTransform();
                btVector3 bodyPosition = transform.getOrigin();
                
                // Cast a ray downward from the body's position
                btVector3 rayStart = bodyPosition;
                btVector3 rayEnd = bodyPosition - btVector3(0, groundCheckDistance, 0);
                
                // Get the bullet world
                auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
                if (bulletWorldPtr) {
                    btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
                    if (bulletWorld) {
                        btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
                        // Ignore the character's own body in the raycast
                        rayCallback.m_collisionFilterMask = ~0; // Hit everything
                        rayCallback.m_collisionFilterGroup = 1;
                        
                        bulletWorld->rayTest(rayStart, rayEnd, rayCallback);
                        
                        // Check if we hit something that's not ourselves
                        if (rayCallback.hasHit()) {
                            const btRigidBody* hitBody = btRigidBody::upcast(rayCallback.m_collisionObject);
                            return hitBody != bulletBody; // Grounded if we hit something other than ourselves
                        }
                    }
                }
            }
        }
#endif
        return false;
    }

    RaycastHit PhysicsEngine::Raycast(const Math::Vec3& origin, const Math::Vec3& direction, float maxDistance) {
        RaycastHit result;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (!m_activeWorld) {
            LOG_WARNING("Cannot perform raycast: No active physics world");
            return result;
        }
        
        auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
        if (!bulletWorldPtr) {
            LOG_WARNING("Cannot perform raycast: Active world is not a BulletPhysicsWorld");
            return result;
        }
        
        btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
        if (!bulletWorld) {
            LOG_WARNING("Cannot perform raycast: Bullet world is null");
            return result;
        }
        
        // Calculate ray end point
        Math::Vec3 normalizedDirection = glm::normalize(direction);
        Math::Vec3 rayEnd = origin + normalizedDirection * maxDistance;
        
        // Convert to Bullet vectors
        btVector3 rayFrom = Physics::BulletUtils::ToBullet(origin);
        btVector3 rayTo = Physics::BulletUtils::ToBullet(rayEnd);
        
        // Perform raycast
        btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
        bulletWorld->rayTest(rayFrom, rayTo, rayCallback);
        
        if (rayCallback.hasHit()) {
            // Find the body ID that corresponds to this Bullet rigid body
            const btRigidBody* hitBody = btRigidBody::upcast(rayCallback.m_collisionObject);
            if (hitBody) {
                // Search for the body ID in our mapping
                for (const auto& pair : m_bulletBodies) {
                    if (pair.second == hitBody) {
                        result.hasHit = true;
                        result.bodyId = pair.first;
                        result.point = Physics::BulletUtils::FromBullet(rayCallback.m_hitPointWorld);
                        result.normal = Physics::BulletUtils::FromBullet(rayCallback.m_hitNormalWorld);
                        result.distance = glm::length(result.point - origin);
                        
                        LOG_DEBUG("Raycast hit rigid body with ID: " + std::to_string(result.bodyId) + 
                                 " at distance: " + std::to_string(result.distance));
                        break;
                    }
                }
            }
        }
        
        return result;
#else
        LOG_WARNING("Raycast not supported: Bullet Physics not available");
        return result;
#endif
    }

    std::vector<OverlapResult> PhysicsEngine::OverlapSphere(const Math::Vec3& center, float radius) {
        std::vector<OverlapResult> overlappingBodies;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (!m_activeWorld) {
            LOG_WARNING("Cannot perform overlap sphere: No active physics world");
            return overlappingBodies;
        }
        
        auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
        if (!bulletWorldPtr) {
            LOG_WARNING("Cannot perform overlap sphere: Active world is not a BulletPhysicsWorld");
            return overlappingBodies;
        }
        
        btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
        if (!bulletWorld) {
            LOG_WARNING("Cannot perform overlap sphere: Bullet world is null");
            return overlappingBodies;
        }
        
        // Create a sphere shape for the overlap test
        btSphereShape sphereShape(radius);
        
        // Create transform for the sphere at the center position
        btTransform sphereTransform;
        sphereTransform.setIdentity();
        sphereTransform.setOrigin(Physics::BulletUtils::ToBullet(center));
        
        // Create collision object for the sphere
        btCollisionObject sphereObject;
        sphereObject.setCollisionShape(&sphereShape);
        sphereObject.setWorldTransform(sphereTransform);
        
        // Perform contact test
        struct OverlapCallback : public btCollisionWorld::ContactResultCallback {
            std::vector<OverlapResult>& results;
            const std::unordered_map<uint32_t, btRigidBody*>& bulletBodies;
            
            OverlapCallback(std::vector<OverlapResult>& r, const std::unordered_map<uint32_t, btRigidBody*>& bb) 
                : results(r), bulletBodies(bb) {}
            
            btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, 
                                   int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, 
                                   int partId1, int index1) override {
                // Find which collision object is not our sphere
                const btCollisionObject* hitObject = nullptr;
                if (colObj0Wrap->getCollisionObject()->getCollisionShape()->getShapeType() != SPHERE_SHAPE_PROXYTYPE) {
                    hitObject = colObj0Wrap->getCollisionObject();
                } else if (colObj1Wrap->getCollisionObject()->getCollisionShape()->getShapeType() != SPHERE_SHAPE_PROXYTYPE) {
                    hitObject = colObj1Wrap->getCollisionObject();
                }
                
                if (hitObject) {
                    const btRigidBody* hitBody = btRigidBody::upcast(hitObject);
                    if (hitBody) {
                        // Find the body ID in our mapping
                        for (const auto& pair : bulletBodies) {
                            if (pair.second == hitBody) {
                                OverlapResult result;
                                result.bodyId = pair.first;
                                result.contactPoint = Physics::BulletUtils::FromBullet(cp.getPositionWorldOnA());
                                result.contactNormal = Physics::BulletUtils::FromBullet(cp.m_normalWorldOnB);
                                result.penetrationDepth = cp.getDistance();
                                
                                results.push_back(result);
                                break;
                            }
                        }
                    }
                }
                
                return 0; // Continue processing
            }
        };
        
        OverlapCallback callback(overlappingBodies, m_bulletBodies);
        bulletWorld->contactTest(&sphereObject, callback);
        
        LOG_DEBUG("Overlap sphere found " + std::to_string(overlappingBodies.size()) + " overlapping bodies");
#else
        LOG_WARNING("Overlap sphere not supported: Bullet Physics not available");
#endif
        
        return overlappingBodies;
    }

    PhysicsEngine::SweepHit PhysicsEngine::SweepCapsule(const Math::Vec3& from, const Math::Vec3& to, float radius, float height) {
        SweepHit result;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (!m_activeWorld) {
            LOG_WARNING("Cannot perform capsule sweep: No active physics world");
            return result;
        }
        
        auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
        if (!bulletWorldPtr) {
            LOG_WARNING("Cannot perform capsule sweep: Active world is not a BulletPhysicsWorld");
            return result;
        }
        
        btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
        if (!bulletWorld) {
            LOG_WARNING("Cannot perform capsule sweep: Bullet world is null");
            return result;
        }
        
        // Debug log for sweep parameters
        float sweepDistance = glm::length(to - from);
        if (sweepDistance > 0.001f) {
            LOG_DEBUG("SweepCapsule: from(" + std::to_string(from.x) + ", " + std::to_string(from.y) + ", " + std::to_string(from.z) + 
                     ") to(" + std::to_string(to.x) + ", " + std::to_string(to.y) + ", " + std::to_string(to.z) + 
                     ") radius=" + std::to_string(radius) + " height=" + std::to_string(height) + 
                     " distance=" + std::to_string(sweepDistance));
            
            // Log number of objects in world
            int numObjects = bulletWorld->getNumCollisionObjects();
            LOG_DEBUG("SweepCapsule: World has " + std::to_string(numObjects) + " collision objects");
        }
        
        // Create capsule shape for sweep test
        btCapsuleShape capsuleShape(radius, height);
        
        // Create transforms for start and end positions
        btTransform fromTransform, toTransform;
        fromTransform.setIdentity();
        toTransform.setIdentity();
        fromTransform.setOrigin(Physics::BulletUtils::ToBullet(from));
        toTransform.setOrigin(Physics::BulletUtils::ToBullet(to));
        
        // Perform convex sweep test
        btCollisionWorld::ClosestConvexResultCallback sweepCallback(fromTransform.getOrigin(), toTransform.getOrigin());
        bulletWorld->convexSweepTest(&capsuleShape, fromTransform, toTransform, sweepCallback);
        
        if (sweepCallback.hasHit()) {
            LOG_DEBUG("SweepCapsule: Hit detected at fraction " + std::to_string(sweepCallback.m_closestHitFraction));
            
            // Find the body ID that corresponds to this Bullet rigid body
            const btRigidBody* hitBody = btRigidBody::upcast(sweepCallback.m_hitCollisionObject);
            if (hitBody) {
                LOG_DEBUG("SweepCapsule: Hit object is a rigid body");
                
                // Search for the body ID in our mapping
                for (const auto& pair : m_bulletBodies) {
                    if (pair.second == hitBody) {
                        result.hasHit = true;
                        result.bodyId = pair.first;
                        result.point = Physics::BulletUtils::FromBullet(sweepCallback.m_hitPointWorld);
                        result.normal = Physics::BulletUtils::FromBullet(sweepCallback.m_hitNormalWorld);
                        result.fraction = sweepCallback.m_closestHitFraction;
                        result.distance = glm::length(to - from) * result.fraction;
                        
                        LOG_INFO("SweepCapsule: Hit rigid body ID " + std::to_string(result.bodyId) + 
                                " at fraction " + std::to_string(result.fraction) + 
                                " distance " + std::to_string(result.distance));
                        break;
                    }
                }
                
                if (!result.hasHit) {
                    LOG_WARNING("SweepCapsule: Hit rigid body not found in mapping");
                }
            } else {
                LOG_DEBUG("SweepCapsule: Hit object is not a rigid body");
            }
        } else {
            if (sweepDistance > 0.001f) {
                LOG_DEBUG("SweepCapsule: No hit detected");
            }
        }
        
        return result;
#else
        LOG_WARNING("Capsule sweep not supported: Bullet Physics not available");
        return result;
#endif
    }

    uint32_t PhysicsEngine::CreateGhostObject(const CollisionShape& shape, const Math::Vec3& position) {
        uint32_t id = m_nextBodyId++;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_activeWorld) {
            // Create Bullet collision shape
            auto bulletShape = Physics::CollisionShapeFactory::CreateShape(shape);
            if (!bulletShape) {
                LOG_ERROR("Failed to create collision shape for ghost object");
                return 0;
            }
            
            // Create ghost object
            auto ghostObject = std::make_unique<btGhostObject>();
            ghostObject->setCollisionShape(bulletShape.release());
            
            // Set initial transform
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(Physics::BulletUtils::ToBullet(position));
            ghostObject->setWorldTransform(transform);
            
            // Set collision flags for ghost object
            ghostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
            
            // Add to the world
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
                if (bulletWorld) {
                    btGhostObject* rawGhostPtr = ghostObject.release();
                    bulletWorld->addCollisionObject(rawGhostPtr, btBroadphaseProxy::SensorTrigger, 
                                                   btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
                    m_bulletGhostObjects[id] = rawGhostPtr;
                    LOG_DEBUG("Created Bullet ghost object with ID: " + std::to_string(id));
                } else {
                    LOG_ERROR("Bullet world is null");
                    return 0;
                }
            } else {
                LOG_ERROR("Active world is not a BulletPhysicsWorld");
                return 0;
            }
        }
#endif
        
        return id;
    }

    void PhysicsEngine::DestroyGhostObject(uint32_t ghostId) {
#ifdef GAMEENGINE_HAS_BULLET
        auto ghostIt = m_bulletGhostObjects.find(ghostId);
        if (ghostIt != m_bulletGhostObjects.end()) {
            btGhostObject* ghostObject = ghostIt->second;
            
            // Remove from the world
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
                if (bulletWorld) {
                    bulletWorld->removeCollisionObject(ghostObject);
                }
            }
            
            // Clean up the ghost object and its components
            if (ghostObject) {
                // Delete collision shape
                if (ghostObject->getCollisionShape()) {
                    delete ghostObject->getCollisionShape();
                }
                
                // Delete the ghost object itself
                delete ghostObject;
            }
            
            m_bulletGhostObjects.erase(ghostIt);
            LOG_DEBUG("Destroyed Bullet ghost object with ID: " + std::to_string(ghostId));
        } else {
            LOG_WARNING("Attempted to destroy non-existent ghost object with ID: " + std::to_string(ghostId));
        }
#else
        LOG_WARNING("Attempted to destroy ghost object but Bullet Physics not available");
#endif
    }

    void PhysicsEngine::SetGhostObjectTransform(uint32_t ghostId, const Math::Vec3& position, const Math::Quat& rotation) {
#ifdef GAMEENGINE_HAS_BULLET
        auto ghostIt = m_bulletGhostObjects.find(ghostId);
        if (ghostIt != m_bulletGhostObjects.end()) {
            btGhostObject* ghostObject = ghostIt->second;
            if (ghostObject) {
                btTransform transform = Physics::BulletUtils::ToBullet(position, rotation);
                ghostObject->setWorldTransform(transform);
                LOG_DEBUG("Updated transform for ghost object with ID: " + std::to_string(ghostId));
            }
        } else {
            LOG_WARNING("Attempted to set transform for non-existent ghost object with ID: " + std::to_string(ghostId));
        }
#else
        LOG_WARNING("Attempted to set ghost object transform but Bullet Physics not available");
#endif
    }

    std::vector<OverlapResult> PhysicsEngine::GetGhostObjectOverlaps(uint32_t ghostId) {
        std::vector<OverlapResult> overlappingBodies;
        
#ifdef GAMEENGINE_HAS_BULLET
        auto ghostIt = m_bulletGhostObjects.find(ghostId);
        if (ghostIt != m_bulletGhostObjects.end()) {
            btGhostObject* ghostObject = ghostIt->second;
            if (ghostObject) {
                // Get overlapping objects from the ghost object
                int numOverlapping = ghostObject->getNumOverlappingObjects();
                
                for (int i = 0; i < numOverlapping; ++i) {
                    const btCollisionObject* overlappingObject = ghostObject->getOverlappingObject(i);
                    const btRigidBody* overlappingBody = btRigidBody::upcast(overlappingObject);
                    
                    if (overlappingBody) {
                        // Find the body ID in our mapping
                        for (const auto& pair : m_bulletBodies) {
                            if (pair.second == overlappingBody) {
                                OverlapResult result;
                                result.bodyId = pair.first;
                                // Note: Ghost objects don't provide detailed contact info
                                // For detailed collision info, use contact tests
                                overlappingBodies.push_back(result);
                                break;
                            }
                        }
                    }
                }
                
                LOG_DEBUG("Ghost object " + std::to_string(ghostId) + " has " + 
                         std::to_string(overlappingBodies.size()) + " overlapping bodies");
            }
        } else {
            LOG_WARNING("Attempted to get overlaps for non-existent ghost object with ID: " + std::to_string(ghostId));
        }
#else
        LOG_WARNING("Ghost object overlaps not supported: Bullet Physics not available");
#endif
        
        return overlappingBodies;
    }

    // PhysicsWorld implementation
    PhysicsWorld::PhysicsWorld(const Math::Vec3& gravity) : m_gravity(gravity) {
    }

    PhysicsWorld::~PhysicsWorld() {
    }

    void PhysicsWorld::Step(float deltaTime) {
        // Physics simulation step would be implemented here
        // This would integrate forces, detect collisions, resolve constraints, etc.
    }

    // Debug visualization implementation
    void PhysicsEngine::SetDebugDrawer(std::shared_ptr<Physics::IPhysicsDebugDrawer> drawer) {
        m_debugDrawer = drawer;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (drawer) {
            m_bulletDebugDrawer = std::make_unique<Physics::BulletDebugDrawer>(drawer);
            
            // Set the debug drawer in the Bullet world
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
                if (bulletWorld) {
                    bulletWorld->setDebugDrawer(m_bulletDebugDrawer.get());
                }
            }
        } else {
            // Remove debug drawer
            auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
            if (bulletWorldPtr) {
                btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
                if (bulletWorld) {
                    bulletWorld->setDebugDrawer(nullptr);
                }
            }
            m_bulletDebugDrawer.reset();
        }
#endif
        
        LOG_DEBUG("Physics debug drawer " + std::string(drawer ? "set" : "removed"));
    }
    
    void PhysicsEngine::SetDebugMode(Physics::PhysicsDebugMode mode) {
        m_debugMode = mode;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_bulletDebugDrawer) {
            int bulletDebugMode = 0;
            
            if (static_cast<int>(mode) & static_cast<int>(Physics::PhysicsDebugMode::Wireframe)) {
                bulletDebugMode |= btIDebugDraw::DBG_DrawWireframe;
            }
            if (static_cast<int>(mode) & static_cast<int>(Physics::PhysicsDebugMode::AABB)) {
                bulletDebugMode |= btIDebugDraw::DBG_DrawAabb;
            }
            if (static_cast<int>(mode) & static_cast<int>(Physics::PhysicsDebugMode::ContactPoints)) {
                bulletDebugMode |= btIDebugDraw::DBG_DrawContactPoints;
            }
            if (static_cast<int>(mode) & static_cast<int>(Physics::PhysicsDebugMode::Constraints)) {
                bulletDebugMode |= btIDebugDraw::DBG_DrawConstraints;
            }
            
            m_bulletDebugDrawer->setDebugMode(bulletDebugMode);
        }
#endif
        
        LOG_DEBUG("Physics debug mode set to: " + std::to_string(static_cast<int>(mode)));
    }
    
    Physics::PhysicsDebugMode PhysicsEngine::GetDebugMode() const {
        return m_debugMode;
    }
    
    void PhysicsEngine::EnableDebugDrawing(bool enabled) {
        m_debugDrawingEnabled = enabled;
        
#ifdef GAMEENGINE_HAS_BULLET
        if (m_bulletDebugDrawer) {
            m_bulletDebugDrawer->SetEnabled(enabled);
        }
#endif
        
        LOG_DEBUG("Physics debug drawing " + std::string(enabled ? "enabled" : "disabled"));
    }
    
    bool PhysicsEngine::IsDebugDrawingEnabled() const {
        return m_debugDrawingEnabled;
    }
    
    void PhysicsEngine::DrawDebugWorld() {
        if (!m_debugDrawingEnabled || !m_debugDrawer) {
            return;
        }
        
        // Clear previous debug drawings
        m_debugDrawer->Clear();
        
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
        if (bulletWorldPtr) {
            btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
            if (bulletWorld && m_bulletDebugDrawer) {
                bulletWorld->debugDrawWorld();
            }
        }
#endif
    }
    
    PhysicsEngine::PhysicsDebugInfo PhysicsEngine::GetDebugInfo() const {
        PhysicsDebugInfo info;
        
        info.numRigidBodies = static_cast<int>(m_bulletBodies.size());
        info.numGhostObjects = static_cast<int>(m_bulletGhostObjects.size());
        info.worldGravity = m_configuration.gravity;
        
#ifdef GAMEENGINE_HAS_BULLET
        auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
        if (bulletWorldPtr) {
            btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
            if (bulletWorld) {
                int numCollisionObjects = bulletWorld->getNumCollisionObjects();
                info.numActiveObjects = 0;
                info.numSleepingObjects = 0;
                
                for (int i = 0; i < numCollisionObjects; i++) {
                    btCollisionObject* obj = bulletWorld->getCollisionObjectArray()[i];
                    if (obj->isActive()) {
                        info.numActiveObjects++;
                    } else {
                        info.numSleepingObjects++;
                    }
                }
            }
        }
#endif
        
        return info;
    }
    
    void PhysicsEngine::PrintDebugInfo() const {
        PhysicsDebugInfo info = GetDebugInfo();
        
        LOG_INFO("=== Physics Debug Info ===");
        LOG_INFO("Rigid Bodies: " + std::to_string(info.numRigidBodies));
        LOG_INFO("Ghost Objects: " + std::to_string(info.numGhostObjects));
        LOG_INFO("Active Objects: " + std::to_string(info.numActiveObjects));
        LOG_INFO("Sleeping Objects: " + std::to_string(info.numSleepingObjects));
        LOG_INFO("World Gravity: (" + 
                std::to_string(info.worldGravity.x) + ", " + 
                std::to_string(info.worldGravity.y) + ", " + 
                std::to_string(info.worldGravity.z) + ")");
        LOG_INFO("Simulation Time: " + std::to_string(info.simulationTime) + "ms");
        LOG_INFO("========================");
    }
}