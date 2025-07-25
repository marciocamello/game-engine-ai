#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletUtils.h"
#include "../TestUtils.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Physics;
using namespace GameEngine::Testing;

/**
 * Test Vec3 round-trip conversion
 * Requirements: Physics system integration
 */
bool TestVec3RoundTripConversion() {
    TestOutput::PrintTestStart("Vec3 round-trip conversion");
    
    Math::Vec3 engineVec(1.5f, -2.7f, 3.14f);
    btVector3 bulletVec = BulletUtils::ToBullet(engineVec);
    Math::Vec3 convertedBack = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_NEARLY_EQUAL_EPSILON(engineVec.x, convertedBack.x, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(engineVec.y, convertedBack.y, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(engineVec.z, convertedBack.z, 1e-6f);
    
    TestOutput::PrintTestPass("Vec3 round-trip conversion");
    return true;
}

/**
 * Test Quaternion round-trip conversion
 * Requirements: Physics system integration
 */
bool TestQuaternionRoundTripConversion() {
    TestOutput::PrintTestStart("Quaternion round-trip conversion");
    
    Math::Quat engineQuat(0.707f, 0.0f, 0.707f, 0.0f); // 90 degree Y rotation
    btQuaternion bulletQuat = BulletUtils::ToBullet(engineQuat);
    Math::Quat convertedBack = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_NEARLY_EQUAL_EPSILON(engineQuat.x, convertedBack.x, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(engineQuat.y, convertedBack.y, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(engineQuat.z, convertedBack.z, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(engineQuat.w, convertedBack.w, 1e-6f);
    
    TestOutput::PrintTestPass("Quaternion round-trip conversion");
    return true;
}

/**
 * Test conversion with actual Bullet Physics objects
 * Requirements: Physics system integration
 */
bool TestBulletPhysicsObjectConversion() {
    TestOutput::PrintTestStart("Bullet Physics object conversion");
    
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
    
    EXPECT_NEARLY_EQUAL_EPSILON(testPosition.x, retrievedPosition.x, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(testPosition.y, retrievedPosition.y, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(testPosition.z, retrievedPosition.z, 1e-6f);
    
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
    
    TestOutput::PrintTestPass("Bullet Physics object conversion");
    return true;
}

int main() {
    TestOutput::PrintHeader("Bullet Conversion Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Bullet Conversion Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Vec3 Round-trip Conversion", TestVec3RoundTripConversion);
        allPassed &= suite.RunTest("Quaternion Round-trip Conversion", TestQuaternionRoundTripConversion);
        allPassed &= suite.RunTest("Bullet Physics Object Conversion", TestBulletPhysicsObjectConversion);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}

#else

#include "TestUtils.h"
#include <iostream>

using namespace GameEngine::Testing;

int main() {
    TestOutput::PrintHeader("Bullet Conversion Integration");
    TestOutput::PrintWarning("Bullet Physics not available - skipping conversion tests");
    TestOutput::PrintFooter(true);
    return 0;
}

#endif // GAMEENGINE_HAS_BULLET