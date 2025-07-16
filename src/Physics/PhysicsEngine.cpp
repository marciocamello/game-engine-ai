#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

#ifdef GAMEENGINE_HAS_BULLET
#include "Physics/BulletPhysicsWorld.h"
#include "Physics/BulletUtils.h"
#include "Physics/CollisionShapeFactory.h"
#endif

namespace GameEngine {
    PhysicsEngine::PhysicsEngine() {
    }

    PhysicsEngine::~PhysicsEngine() {
        Shutdown();
    }

    bool PhysicsEngine::Initialize() {
#ifdef GAMEENGINE_HAS_BULLET
        LOG_INFO("Physics Engine initialized with Bullet Physics support");
        return true;
#else
        LOG_WARNING("Physics Engine initialized without Bullet Physics support");
        return false;
#endif
    }

    void PhysicsEngine::Shutdown() {
#ifdef GAMEENGINE_HAS_BULLET
        m_bulletBodies.clear();
#endif
        m_activeWorld.reset();
        LOG_INFO("Physics Engine shutdown");
    }

    void PhysicsEngine::Update(float deltaTime) {
        if (m_activeWorld) {
            m_activeWorld->Step(deltaTime);
        }
    }

    std::shared_ptr<PhysicsWorld> PhysicsEngine::CreateWorld(const Math::Vec3& gravity) {
#ifdef GAMEENGINE_HAS_BULLET
        return std::make_shared<BulletPhysicsWorld>(gravity);
#else
        return std::make_shared<PhysicsWorld>(gravity);
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

    bool PhysicsEngine::Raycast(const Math::Vec3& origin, const Math::Vec3& direction, float maxDistance, uint32_t& hitBodyId) {
#ifdef GAMEENGINE_HAS_BULLET
        if (!m_activeWorld) {
            LOG_WARNING("Cannot perform raycast: No active physics world");
            return false;
        }
        
        auto bulletWorldPtr = std::dynamic_pointer_cast<BulletPhysicsWorld>(m_activeWorld);
        if (!bulletWorldPtr) {
            LOG_WARNING("Cannot perform raycast: Active world is not a BulletPhysicsWorld");
            return false;
        }
        
        btDiscreteDynamicsWorld* bulletWorld = bulletWorldPtr->GetBulletWorld();
        if (!bulletWorld) {
            LOG_WARNING("Cannot perform raycast: Bullet world is null");
            return false;
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
                        hitBodyId = pair.first;
                        LOG_DEBUG("Raycast hit rigid body with ID: " + std::to_string(hitBodyId));
                        return true;
                    }
                }
            }
        }
        
        return false;
#else
        LOG_WARNING("Raycast not supported: Bullet Physics not available");
        return false;
#endif
    }

    std::vector<uint32_t> PhysicsEngine::OverlapSphere(const Math::Vec3& center, float radius) {
        std::vector<uint32_t> overlappingBodies;
        
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
            std::vector<uint32_t>& bodies;
            const std::unordered_map<uint32_t, btRigidBody*>& bulletBodies;
            
            OverlapCallback(std::vector<uint32_t>& b, const std::unordered_map<uint32_t, btRigidBody*>& bb) 
                : bodies(b), bulletBodies(bb) {}
            
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
                                bodies.push_back(pair.first);
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

    // PhysicsWorld implementation
    PhysicsWorld::PhysicsWorld(const Math::Vec3& gravity) : m_gravity(gravity) {
    }

    PhysicsWorld::~PhysicsWorld() {
    }

    void PhysicsWorld::Step(float deltaTime) {
        // Physics simulation step would be implemented here
        // This would integrate forces, detect collisions, resolve constraints, etc.
    }
}