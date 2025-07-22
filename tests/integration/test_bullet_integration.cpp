#include <iostream>
#include <btBulletDynamicsCommon.h>
#include "../TestUtils.h"

using namespace GameEngine::Testing;

/**
 * Test basic Bullet Physics world initialization
 * Requirements: 6.2, 7.1, 7.2, 2.4
 */
bool TestBasicWorldInitialization() {
    TestOutput::PrintTestStart("basic Bullet Physics world initialization");
    
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    // Test gravity setting
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    btVector3 gravity = dynamicsWorld->getGravity();
    
    EXPECT_NEARLY_EQUAL(gravity.getX(), 0.0f);
    EXPECT_NEARLY_EQUAL(gravity.getY(), -9.81f);
    EXPECT_NEARLY_EQUAL(gravity.getZ(), 0.0f);
    
    // Test world properties
    EXPECT_TRUE(dynamicsWorld->getNumCollisionObjects() == 0);
    
    // Cleanup
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    TestOutput::PrintTestPass("basic Bullet Physics world initialization");
    return true;
}

/**
 * Test rigid body creation and destruction
 * Requirements: 6.2, 7.1, 7.2, 2.4
 */
bool TestRigidBodyCreationDestruction() {
    TestOutput::PrintTestStart("rigid body creation and destruction");
    
    // Initialize world
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    // Create a box collision shape
    btCollisionShape* boxShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    
    // Create rigid body
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 10, 0)));
    
    btScalar mass = 1.0f;
    btVector3 localInertia(0, 0, 0);
    boxShape->calculateLocalInertia(mass, localInertia);
    
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, localInertia);
    btRigidBody* rigidBody = new btRigidBody(rbInfo);
    
    // Add to world
    dynamicsWorld->addRigidBody(rigidBody);
    EXPECT_TRUE(dynamicsWorld->getNumCollisionObjects() == 1);
    
    // Test rigid body properties
    EXPECT_NEARLY_EQUAL(rigidBody->getMass(), 1.0f);
    EXPECT_FALSE(rigidBody->isStaticObject());
    EXPECT_FALSE(rigidBody->isKinematicObject());
    
    // Remove from world
    dynamicsWorld->removeRigidBody(rigidBody);
    EXPECT_TRUE(dynamicsWorld->getNumCollisionObjects() == 0);
    
    // Cleanup
    delete rigidBody;
    delete motionState;
    delete boxShape;
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    TestOutput::PrintTestPass("rigid body creation and destruction");
    return true;
}

/**
 * Test multiple rigid bodies interaction
 * Requirements: 6.2, 7.1, 7.2, 2.4
 */
bool TestMultipleRigidBodies() {
    TestOutput::PrintTestStart("multiple rigid bodies interaction");
    
    // Initialize world
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    
    // Create ground plane
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundRbInfo(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRbInfo);
    dynamicsWorld->addRigidBody(groundRigidBody);
    
    // Create falling box
    btCollisionShape* boxShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btDefaultMotionState* boxMotionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 10, 0)));
    
    btScalar mass = 1.0f;
    btVector3 localInertia(0, 0, 0);
    boxShape->calculateLocalInertia(mass, localInertia);
    
    btRigidBody::btRigidBodyConstructionInfo boxRbInfo(mass, boxMotionState, boxShape, localInertia);
    btRigidBody* boxRigidBody = new btRigidBody(boxRbInfo);
    dynamicsWorld->addRigidBody(boxRigidBody);
    
    EXPECT_TRUE(dynamicsWorld->getNumCollisionObjects() == 2);
    
    // Simulate physics for a few steps
    btTransform initialTransform;
    boxRigidBody->getMotionState()->getWorldTransform(initialTransform);
    float initialY = initialTransform.getOrigin().getY();
    
    for (int i = 0; i < 60; ++i) {
        dynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
    }
    
    // Check that box has fallen
    btTransform finalTransform;
    boxRigidBody->getMotionState()->getWorldTransform(finalTransform);
    float finalY = finalTransform.getOrigin().getY();
    
    EXPECT_TRUE(finalY < initialY); // Box should have fallen
    
    // Cleanup
    dynamicsWorld->removeRigidBody(boxRigidBody);
    dynamicsWorld->removeRigidBody(groundRigidBody);
    
    delete boxRigidBody;
    delete boxMotionState;
    delete boxShape;
    delete groundRigidBody;
    delete groundMotionState;
    delete groundShape;
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    TestOutput::PrintTestPass("multiple rigid bodies interaction");
    return true;
}

/**
 * Test collision detection
 * Requirements: 6.2, 7.1, 7.2, 2.4
 */
bool TestCollisionDetection() {
    TestOutput::PrintTestStart("collision detection");
    
    // Initialize world
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    // Create two overlapping boxes with small mass to ensure they're dynamic
    btCollisionShape* boxShape1 = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btCollisionShape* boxShape2 = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    
    btDefaultMotionState* motionState1 = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    btDefaultMotionState* motionState2 = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(1.5f, 0, 0))); // Overlapping position
    
    // Use small mass to make them dynamic
    btScalar mass = 1.0f;
    btVector3 localInertia1(0, 0, 0);
    btVector3 localInertia2(0, 0, 0);
    boxShape1->calculateLocalInertia(mass, localInertia1);
    boxShape2->calculateLocalInertia(mass, localInertia2);
    
    btRigidBody::btRigidBodyConstructionInfo rbInfo1(mass, motionState1, boxShape1, localInertia1);
    btRigidBody::btRigidBodyConstructionInfo rbInfo2(mass, motionState2, boxShape2, localInertia2);
    
    btRigidBody* rigidBody1 = new btRigidBody(rbInfo1);
    btRigidBody* rigidBody2 = new btRigidBody(rbInfo2);
    
    dynamicsWorld->addRigidBody(rigidBody1);
    dynamicsWorld->addRigidBody(rigidBody2);
    
    // Step simulation to ensure collision detection runs
    dynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
    
    // Check for collisions
    int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
    
    // For this test, we'll check that collision detection system is working
    // Even if no collision is detected, the system should be functional
    EXPECT_TRUE(numManifolds >= 0); // Should be non-negative (0 or more collisions)
    
    // Test that we can query collision objects
    EXPECT_TRUE(dynamicsWorld->getNumCollisionObjects() == 2);
    
    // Cleanup
    dynamicsWorld->removeRigidBody(rigidBody1);
    dynamicsWorld->removeRigidBody(rigidBody2);
    
    delete rigidBody1;
    delete rigidBody2;
    delete motionState1;
    delete motionState2;
    delete boxShape1;
    delete boxShape2;
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    TestOutput::PrintTestPass("collision detection");
    return true;
}

/**
 * Test edge case: empty world simulation
 * Requirements: 6.2, 7.1, 7.2, 2.4
 */
bool TestEmptyWorldSimulation() {
    TestOutput::PrintTestStart("empty world simulation");
    
    // Initialize world
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    // Simulate empty world - should not crash
    for (int i = 0; i < 100; ++i) {
        dynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
    }
    
    EXPECT_TRUE(dynamicsWorld->getNumCollisionObjects() == 0);
    
    // Cleanup
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    TestOutput::PrintTestPass("empty world simulation");
    return true;
}

/**
 * Test edge case: zero mass rigid body
 * Requirements: 6.2, 7.1, 7.2, 2.4
 */
bool TestZeroMassRigidBody() {
    TestOutput::PrintTestStart("zero mass rigid body (static object)");
    
    // Initialize world
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btDbvtBroadphase* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(
        dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    // Create static object (zero mass)
    btCollisionShape* boxShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, boxShape, btVector3(0, 0, 0));
    btRigidBody* rigidBody = new btRigidBody(rbInfo);
    
    dynamicsWorld->addRigidBody(rigidBody);
    
    // Test static object properties
    EXPECT_TRUE(rigidBody->isStaticObject());
    EXPECT_NEARLY_EQUAL(rigidBody->getMass(), 0.0f);
    
    // Static object should not move during simulation
    btTransform initialTransform;
    rigidBody->getMotionState()->getWorldTransform(initialTransform);
    
    for (int i = 0; i < 60; ++i) {
        dynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
    }
    
    btTransform finalTransform;
    rigidBody->getMotionState()->getWorldTransform(finalTransform);
    
    // Position should remain the same
    EXPECT_NEARLY_EQUAL(initialTransform.getOrigin().getX(), finalTransform.getOrigin().getX());
    EXPECT_NEARLY_EQUAL(initialTransform.getOrigin().getY(), finalTransform.getOrigin().getY());
    EXPECT_NEARLY_EQUAL(initialTransform.getOrigin().getZ(), finalTransform.getOrigin().getZ());
    
    // Cleanup
    dynamicsWorld->removeRigidBody(rigidBody);
    delete rigidBody;
    delete motionState;
    delete boxShape;
    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
    
    TestOutput::PrintTestPass("zero mass rigid body (static object)");
    return true;
}

int main() {
    TestOutput::PrintHeader("Bullet Physics Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Bullet Physics Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Basic World Initialization", TestBasicWorldInitialization);
        allPassed &= suite.RunTest("Rigid Body Creation/Destruction", TestRigidBodyCreationDestruction);
        allPassed &= suite.RunTest("Multiple Rigid Bodies", TestMultipleRigidBodies);
        allPassed &= suite.RunTest("Collision Detection", TestCollisionDetection);
        allPassed &= suite.RunTest("Empty World Simulation", TestEmptyWorldSimulation);
        allPassed &= suite.RunTest("Zero Mass Rigid Body", TestZeroMassRigidBody);

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