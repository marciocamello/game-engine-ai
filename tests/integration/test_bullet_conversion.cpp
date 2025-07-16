#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletUtils.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Physics;

int main() {
    std::cout << "Testing Bullet Physics conversion utilities..." << std::endl;
    
    // Test Vec3 conversions
    {
        Math::Vec3 engineVec(1.5f, -2.7f, 3.14f);
        btVector3 bulletVec = BulletUtils::ToBullet(engineVec);
        Math::Vec3 convertedBack = BulletUtils::FromBullet(bulletVec);
        
        bool vec3Success = (std::abs(engineVec.x - convertedBack.x) < 1e-6f) &&
                          (std::abs(engineVec.y - convertedBack.y) < 1e-6f) &&
                          (std::abs(engineVec.z - convertedBack.z) < 1e-6f);
        
        std::cout << "Vec3 round-trip conversion: " << (vec3Success ? "PASS" : "FAIL") << std::endl;
        if (!vec3Success) {
            std::cout << "  Original: (" << engineVec.x << ", " << engineVec.y << ", " << engineVec.z << ")" << std::endl;
            std::cout << "  Converted: (" << convertedBack.x << ", " << convertedBack.y << ", " << convertedBack.z << ")" << std::endl;
            return 1;
        }
    }
    
    // Test Quaternion conversions
    {
        Math::Quat engineQuat(0.707f, 0.0f, 0.707f, 0.0f); // 90 degree Y rotation
        btQuaternion bulletQuat = BulletUtils::ToBullet(engineQuat);
        Math::Quat convertedBack = BulletUtils::FromBullet(bulletQuat);
        
        bool quatSuccess = (std::abs(engineQuat.x - convertedBack.x) < 1e-6f) &&
                          (std::abs(engineQuat.y - convertedBack.y) < 1e-6f) &&
                          (std::abs(engineQuat.z - convertedBack.z) < 1e-6f) &&
                          (std::abs(engineQuat.w - convertedBack.w) < 1e-6f);
        
        std::cout << "Quaternion round-trip conversion: " << (quatSuccess ? "PASS" : "FAIL") << std::endl;
        if (!quatSuccess) {
            std::cout << "  Original: (" << engineQuat.w << ", " << engineQuat.x << ", " << engineQuat.y << ", " << engineQuat.z << ")" << std::endl;
            std::cout << "  Converted: (" << convertedBack.w << ", " << convertedBack.x << ", " << convertedBack.y << ", " << convertedBack.z << ")" << std::endl;
            return 1;
        }
    }
    
    // Test with actual Bullet Physics objects
    {
        std::cout << "Testing with actual Bullet Physics rigid body..." << std::endl;
        
        // Create a simple Bullet Physics world
        btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
        btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
        btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
        btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
            dispatcher, overlappingPairCache, solver, collisionConfiguration);
        
        // Create a rigid body
        btCollisionShape* groundShape = new btBoxShape(btVector3(50, 1, 50));
        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, -1, 0));
        
        btDefaultMotionState* groundMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        dynamicsWorld->addRigidBody(groundRigidBody);
        
        // Test position conversion
        Math::Vec3 testPosition(5.0f, 10.0f, -3.0f);
        btVector3 bulletPos = BulletUtils::ToBullet(testPosition);
        
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(bulletPos);
        groundRigidBody->setWorldTransform(transform);
        
        // Get position back and convert
        btTransform retrievedTransform = groundRigidBody->getWorldTransform();
        Math::Vec3 retrievedPosition = BulletUtils::FromBullet(retrievedTransform.getOrigin());
        
        bool positionSuccess = (std::abs(testPosition.x - retrievedPosition.x) < 1e-6f) &&
                              (std::abs(testPosition.y - retrievedPosition.y) < 1e-6f) &&
                              (std::abs(testPosition.z - retrievedPosition.z) < 1e-6f);
        
        std::cout << "Bullet rigid body position conversion: " << (positionSuccess ? "PASS" : "FAIL") << std::endl;
        
        // Cleanup
        dynamicsWorld->removeRigidBody(groundRigidBody);
        delete groundRigidBody;
        delete groundMotionState;
        delete groundShape;
        delete dynamicsWorld;
        delete solver;
        delete overlappingPairCache;
        delete dispatcher;
        delete collisionConfiguration;
        
        if (!positionSuccess) {
            return 1;
        }
    }
    
    std::cout << "All Bullet Physics conversion tests passed!" << std::endl;
    return 0;
}

#else

#include <iostream>

int main() {
    std::cout << "Bullet Physics not available - skipping conversion tests" << std::endl;
    return 0;
}

#endif // GAMEENGINE_HAS_BULLET