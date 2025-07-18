#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletPhysicsWorld.h"
#include "Physics/BulletUtils.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Physics;

class BulletPhysicsWorldTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_bullet_physics_world.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_epsilon = 1e-6f;
    }
    
    void TearDown() override {
        // Cleanup handled by unique_ptr destructors
    }
    
    float m_epsilon;
};

// Construction and Destruction Tests
TEST_F(BulletPhysicsWorldTest, Constructor_DefaultGravity_Success) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    
    EXPECT_NO_THROW({
        auto world = std::make_unique<BulletPhysicsWorld>(gravity);
        ASSERT_NE(world, nullptr);
        
        const auto& worldGravity = world->GetGravity();
        EXPECT_NEAR(worldGravity.x, gravity.x, m_epsilon);
        EXPECT_NEAR(worldGravity.y, gravity.y, m_epsilon);
        EXPECT_NEAR(worldGravity.z, gravity.z, m_epsilon);
    });
}

TEST_F(BulletPhysicsWorldTest, Constructor_CustomGravity_Success) {
    Math::Vec3 customGravity(1.0f, -5.0f, 2.0f);
    
    EXPECT_NO_THROW({
        auto world = std::make_unique<BulletPhysicsWorld>(customGravity);
        ASSERT_NE(world, nullptr);
        
        const auto& worldGravity = world->GetGravity();
        EXPECT_NEAR(worldGravity.x, customGravity.x, m_epsilon);
        EXPECT_NEAR(worldGravity.y, customGravity.y, m_epsilon);
        EXPECT_NEAR(worldGravity.z, customGravity.z, m_epsilon);
    });
}

TEST_F(BulletPhysicsWorldTest, Constructor_WithConfiguration_Success) {
    PhysicsConfiguration config = PhysicsConfiguration::ForCharacterMovement();
    
    EXPECT_NO_THROW({
        auto world = std::make_unique<BulletPhysicsWorld>(config);
        ASSERT_NE(world, nullptr);
        
        const auto& worldGravity = world->GetGravity();
        EXPECT_NEAR(worldGravity.x, config.gravity.x, m_epsilon);
        EXPECT_NEAR(worldGravity.y, config.gravity.y, m_epsilon);
        EXPECT_NEAR(worldGravity.z, config.gravity.z, m_epsilon);
    });
}

// Gravity Management Tests
TEST_F(BulletPhysicsWorldTest, SetGravity_ValidVector_Applied) {
    Math::Vec3 initialGravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(initialGravity);
    
    Math::Vec3 newGravity(2.0f, -5.0f, -1.0f);
    world->SetGravity(newGravity);
    
    const auto& appliedGravity = world->GetGravity();
    EXPECT_NEAR(appliedGravity.x, newGravity.x, m_epsilon);
    EXPECT_NEAR(appliedGravity.y, newGravity.y, m_epsilon);
    EXPECT_NEAR(appliedGravity.z, newGravity.z, m_epsilon);
    
    // Verify Bullet world gravity is also updated
    btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
    ASSERT_NE(bulletWorld, nullptr);
    
    btVector3 bulletGravity = bulletWorld->getGravity();
    EXPECT_NEAR(bulletGravity.getX(), newGravity.x, m_epsilon);
    EXPECT_NEAR(bulletGravity.getY(), newGravity.y, m_epsilon);
    EXPECT_NEAR(bulletGravity.getZ(), newGravity.z, m_epsilon);
}

// Bullet World Access Tests
TEST_F(BulletPhysicsWorldTest, GetBulletWorld_ReturnsValidPointer) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
    ASSERT_NE(bulletWorld, nullptr);
    
    // Verify it's a valid Bullet world
    btVector3 worldGravity = bulletWorld->getGravity();
    EXPECT_NEAR(worldGravity.getY(), -9.81f, m_epsilon);
}

// Rigid Body Management Tests
TEST_F(BulletPhysicsWorldTest, AddRigidBody_ValidBody_Success) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    // Create a rigid body
    btCollisionShape* shape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform::getIdentity());
    btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState, shape);
    btRigidBody* body = new btRigidBody(rbInfo);
    
    uint32_t bodyId = 1;
    EXPECT_NO_THROW(world->AddRigidBody(bodyId, body));
    
    // Verify body was added
    btRigidBody* retrievedBody = world->GetRigidBody(bodyId);
    EXPECT_EQ(retrievedBody, body);
    
    // Verify body is in Bullet world
    btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
    EXPECT_GT(bulletWorld->getNumCollisionObjects(), 0);
}

TEST_F(BulletPhysicsWorldTest, RemoveRigidBody_ValidId_Success) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    // Create and add a rigid body
    btCollisionShape* shape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform::getIdentity());
    btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState, shape);
    btRigidBody* body = new btRigidBody(rbInfo);
    
    uint32_t bodyId = 1;
    world->AddRigidBody(bodyId, body);
    
    // Verify body exists
    EXPECT_NE(world->GetRigidBody(bodyId), nullptr);
    
    // Remove body
    EXPECT_NO_THROW(world->RemoveRigidBody(bodyId));
    
    // Verify body is removed
    EXPECT_EQ(world->GetRigidBody(bodyId), nullptr);
    
    btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
    EXPECT_EQ(bulletWorld->getNumCollisionObjects(), 0);
}

TEST_F(BulletPhysicsWorldTest, GetRigidBody_InvalidId_ReturnsNull) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    uint32_t invalidId = 999999;
    btRigidBody* retrievedBody = world->GetRigidBody(invalidId);
    EXPECT_EQ(retrievedBody, nullptr);
}

// Configuration Management Tests
TEST_F(BulletPhysicsWorldTest, SetConfiguration_ValidConfig_Applied) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    PhysicsConfiguration config = PhysicsConfiguration::HighPrecision();
    config.gravity = Math::Vec3(1.0f, -5.0f, 2.0f);
    
    EXPECT_NO_THROW(world->SetConfiguration(config));
    
    // Verify gravity was updated
    const auto& worldGravity = world->GetGravity();
    EXPECT_NEAR(worldGravity.x, config.gravity.x, m_epsilon);
    EXPECT_NEAR(worldGravity.y, config.gravity.y, m_epsilon);
    EXPECT_NEAR(worldGravity.z, config.gravity.z, m_epsilon);
}

TEST_F(BulletPhysicsWorldTest, SetSolverIterations_ValidValue_Applied) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    int newIterations = 25;
    EXPECT_NO_THROW(world->SetSolverIterations(newIterations));
    
    // Verify solver iterations were set in Bullet world
    btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
    btContactSolverInfo& solverInfo = bulletWorld->getSolverInfo();
    EXPECT_EQ(solverInfo.m_numIterations, newIterations);
}

// Physics Simulation Tests
TEST_F(BulletPhysicsWorldTest, Step_ValidDeltaTime_Success) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    // Create a dynamic rigid body
    btCollisionShape* shape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion::getIdentity(), btVector3(0.0f, 10.0f, 0.0f))
    );
    btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState, shape);
    btRigidBody* body = new btRigidBody(rbInfo);
    
    world->AddRigidBody(1, body);
    
    // Get initial position
    btTransform initialTransform;
    body->getMotionState()->getWorldTransform(initialTransform);
    float initialY = initialTransform.getOrigin().getY();
    
    // Step simulation
    float deltaTime = 1.0f / 60.0f;
    EXPECT_NO_THROW(world->Step(deltaTime));
    
    // Verify object has moved due to gravity
    btTransform newTransform;
    body->getMotionState()->getWorldTransform(newTransform);
    float newY = newTransform.getOrigin().getY();
    
    // Object should have fallen (Y position should be lower)
    EXPECT_LT(newY, initialY);
}

TEST_F(BulletPhysicsWorldTest, Step_ZeroDeltaTime_HandledGracefully) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    EXPECT_NO_THROW(world->Step(0.0f));
}

// Performance Tests
TEST_F(BulletPhysicsWorldTest, AddManyRigidBodies_PerformanceTest) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    const int numBodies = 100;
    std::vector<btRigidBody*> bodies;
    bodies.reserve(numBodies);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numBodies; ++i) {
        btCollisionShape* shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
        btDefaultMotionState* motionState = new btDefaultMotionState(
            btTransform(btQuaternion::getIdentity(), btVector3(i % 10, i / 10, 0.0f))
        );
        btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, motionState, shape);
        btRigidBody* body = new btRigidBody(rbInfo);
        
        world->AddRigidBody(i + 1, body);
        bodies.push_back(body);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(bodies.size(), numBodies);
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    
    btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
    EXPECT_EQ(bulletWorld->getNumCollisionObjects(), numBodies);
    
    std::cout << "Added " << numBodies << " rigid bodies in " << duration.count() << "ms" << std::endl;
}

// Integration Tests with Real Physics Scenarios
TEST_F(BulletPhysicsWorldTest, Integration_FallingBox_RealisticBehavior) {
    Math::Vec3 gravity(0.0f, -9.81f, 0.0f);
    auto world = std::make_unique<BulletPhysicsWorld>(gravity);
    
    // Create ground
    btCollisionShape* groundShape = new btBoxShape(btVector3(50.0f, 1.0f, 50.0f));
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(
        btTransform(btQuaternion::getIdentity(), btVector3(0.0f, -1.0f, 0.0f))
    );
    btRigidBody::btRigidBodyConstructionInfo groundRbInfo(0.0f, groundMotionState, groundShape);
    btRigidBody* groundBody = new btRigidBody(groundRbInfo);
    world->AddRigidBody(1, groundBody);
    
    // Create falling box
    btCollisionShape* boxShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
    btDefaultMotionState* boxMotionState = new btDefaultMotionState(
        btTransform(btQuaternion::getIdentity(), btVector3(0.0f, 10.0f, 0.0f))
    );
    btRigidBody::btRigidBodyConstructionInfo boxRbInfo(1.0f, boxMotionState, boxShape);
    btRigidBody* boxBody = new btRigidBody(boxRbInfo);
    world->AddRigidBody(2, boxBody);
    
    // Simulate until box hits ground
    float deltaTime = 1.0f / 60.0f;
    float previousY = 10.0f;
    bool hitGround = false;
    
    for (int i = 0; i < 300; ++i) { // 5 seconds max
        world->Step(deltaTime);
        
        btTransform transform;
        boxBody->getMotionState()->getWorldTransform(transform);
        float currentY = transform.getOrigin().getY();
        
        // Check if box has stopped falling (hit ground)
        if (std::abs(currentY - previousY) < 0.001f && currentY < 5.0f) {
            hitGround = true;
            break;
        }
        
        previousY = currentY;
    }
    
    EXPECT_TRUE(hitGround) << "Box should have hit the ground";
    EXPECT_LT(previousY, 5.0f) << "Box should be near ground level";
}

// Parameterized Tests for Different Configurations
class BulletPhysicsWorldConfigTest : public ::testing::TestWithParam<PhysicsConfiguration> {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_bullet_physics_world_config.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_epsilon = 1e-6f;
    }
    
    float m_epsilon;
};

TEST_P(BulletPhysicsWorldConfigTest, ConstructorWithConfiguration_Success) {
    PhysicsConfiguration config = GetParam();
    
    EXPECT_NO_THROW({
        auto world = std::make_unique<BulletPhysicsWorld>(config);
        ASSERT_NE(world, nullptr);
        
        const auto& worldGravity = world->GetGravity();
        EXPECT_NEAR(worldGravity.x, config.gravity.x, m_epsilon);
        EXPECT_NEAR(worldGravity.y, config.gravity.y, m_epsilon);
        EXPECT_NEAR(worldGravity.z, config.gravity.z, m_epsilon);
        
        // Verify Bullet world is properly initialized
        btDiscreteDynamicsWorld* bulletWorld = world->GetBulletWorld();
        ASSERT_NE(bulletWorld, nullptr);
        
        btVector3 bulletGravity = bulletWorld->getGravity();
        EXPECT_NEAR(bulletGravity.getX(), config.gravity.x, m_epsilon);
        EXPECT_NEAR(bulletGravity.getY(), config.gravity.y, m_epsilon);
        EXPECT_NEAR(bulletGravity.getZ(), config.gravity.z, m_epsilon);
    });
}

INSTANTIATE_TEST_SUITE_P(
    ConfigurationTests,
    BulletPhysicsWorldConfigTest,
    ::testing::Values(
        PhysicsConfiguration::Default(),
        PhysicsConfiguration::ForCharacterMovement(),
        PhysicsConfiguration::HighPrecision()
    )
);

#endif // GAMEENGINE_HAS_BULLET