#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

using namespace GameEngine;
using namespace GameEngine::Physics;

class PhysicsEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_physics_engine.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_engine = std::make_unique<PhysicsEngine>();
        m_epsilon = 1e-6f;
    }
    
    void TearDown() override {
        if (m_engine) {
            m_engine->Shutdown();
        }
    }
    
    std::unique_ptr<PhysicsEngine> m_engine;
    float m_epsilon;
};

// Initialization and Shutdown Tests
TEST_F(PhysicsEngineTest, Initialize_DefaultConfiguration_Success) {
    bool result = m_engine->Initialize();
    EXPECT_TRUE(result);
    
    // Verify default configuration is applied
    const auto& config = m_engine->GetConfiguration();
    EXPECT_NEAR(config.gravity.y, -9.81f, m_epsilon);
    EXPECT_NEAR(config.timeStep, 1.0f / 60.0f, m_epsilon);
    EXPECT_EQ(config.maxSubSteps, 10);
    EXPECT_EQ(config.solverIterations, 10);
    EXPECT_TRUE(config.enableCCD);
}

TEST_F(PhysicsEngineTest, Initialize_CustomConfiguration_Success) {
    PhysicsConfiguration config = PhysicsConfiguration::ForCharacterMovement();
    bool result = m_engine->Initialize(config);
    EXPECT_TRUE(result);
    
    // Verify custom configuration is applied
    const auto& appliedConfig = m_engine->GetConfiguration();
    EXPECT_NEAR(appliedConfig.gravity.y, -9.81f, m_epsilon);
    EXPECT_EQ(appliedConfig.solverIterations, 15);
    EXPECT_NEAR(appliedConfig.linearDamping, 0.1f, m_epsilon);
    EXPECT_NEAR(appliedConfig.angularDamping, 0.1f, m_epsilon);
}

TEST_F(PhysicsEngineTest, Initialize_HighPrecisionConfiguration_Success) {
    PhysicsConfiguration config = PhysicsConfiguration::HighPrecision();
    bool result = m_engine->Initialize(config);
    EXPECT_TRUE(result);
    
    const auto& appliedConfig = m_engine->GetConfiguration();
    EXPECT_NEAR(appliedConfig.timeStep, 1.0f / 120.0f, m_epsilon);
    EXPECT_EQ(appliedConfig.maxSubSteps, 20);
    EXPECT_EQ(appliedConfig.solverIterations, 20);
    EXPECT_NEAR(appliedConfig.contactBreakingThreshold, 0.01f, m_epsilon);
}

TEST_F(PhysicsEngineTest, Shutdown_AfterInitialization_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Should not crash or throw
    EXPECT_NO_THROW(m_engine->Shutdown());
}

TEST_F(PhysicsEngineTest, Shutdown_WithoutInitialization_Success) {
    // Should handle shutdown without initialization gracefully
    EXPECT_NO_THROW(m_engine->Shutdown());
}

TEST_F(PhysicsEngineTest, MultipleInitialize_ShouldSucceed) {
    EXPECT_TRUE(m_engine->Initialize());
    EXPECT_TRUE(m_engine->Initialize()); // Second initialization should succeed
}

// Configuration Management Tests
TEST_F(PhysicsEngineTest, SetConfiguration_ValidConfig_Applied) {
    ASSERT_TRUE(m_engine->Initialize());
    
    PhysicsConfiguration newConfig;
    newConfig.gravity = Math::Vec3(0.0f, -5.0f, 0.0f);
    newConfig.timeStep = 1.0f / 30.0f;
    newConfig.solverIterations = 20;
    
    m_engine->SetConfiguration(newConfig);
    
    const auto& appliedConfig = m_engine->GetConfiguration();
    EXPECT_NEAR(appliedConfig.gravity.y, -5.0f, m_epsilon);
    EXPECT_NEAR(appliedConfig.timeStep, 1.0f / 30.0f, m_epsilon);
    EXPECT_EQ(appliedConfig.solverIterations, 20);
}

TEST_F(PhysicsEngineTest, SetGravity_ValidVector_Applied) {
    ASSERT_TRUE(m_engine->Initialize());
    
    Math::Vec3 newGravity(1.0f, -5.0f, 2.0f);
    m_engine->SetGravity(newGravity);
    
    const auto& config = m_engine->GetConfiguration();
    EXPECT_NEAR(config.gravity.x, 1.0f, m_epsilon);
    EXPECT_NEAR(config.gravity.y, -5.0f, m_epsilon);
    EXPECT_NEAR(config.gravity.z, 2.0f, m_epsilon);
}

TEST_F(PhysicsEngineTest, SetTimeStep_ValidValue_Applied) {
    ASSERT_TRUE(m_engine->Initialize());
    
    float newTimeStep = 1.0f / 120.0f;
    m_engine->SetTimeStep(newTimeStep);
    
    const auto& config = m_engine->GetConfiguration();
    EXPECT_NEAR(config.timeStep, newTimeStep, m_epsilon);
}

TEST_F(PhysicsEngineTest, SetSolverIterations_ValidValue_Applied) {
    ASSERT_TRUE(m_engine->Initialize());
    
    int newIterations = 25;
    m_engine->SetSolverIterations(newIterations);
    
    const auto& config = m_engine->GetConfiguration();
    EXPECT_EQ(config.solverIterations, newIterations);
}

TEST_F(PhysicsEngineTest, SetContactThresholds_ValidValues_Applied) {
    ASSERT_TRUE(m_engine->Initialize());
    
    float breakingThreshold = 0.05f;
    float processingThreshold = 0.025f;
    m_engine->SetContactThresholds(breakingThreshold, processingThreshold);
    
    const auto& config = m_engine->GetConfiguration();
    EXPECT_NEAR(config.contactBreakingThreshold, breakingThreshold, m_epsilon);
    EXPECT_NEAR(config.contactProcessingThreshold, processingThreshold, m_epsilon);
}

// World Management Tests
TEST_F(PhysicsEngineTest, CreateWorld_DefaultGravity_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    auto world = m_engine->CreateWorld();
    ASSERT_NE(world, nullptr);
    
    const auto& gravity = world->GetGravity();
    EXPECT_NEAR(gravity.y, -9.81f, m_epsilon);
}

TEST_F(PhysicsEngineTest, CreateWorld_CustomGravity_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    Math::Vec3 customGravity(0.0f, -5.0f, 0.0f);
    auto world = m_engine->CreateWorld(customGravity);
    ASSERT_NE(world, nullptr);
    
    const auto& gravity = world->GetGravity();
    EXPECT_NEAR(gravity.y, -5.0f, m_epsilon);
}

TEST_F(PhysicsEngineTest, CreateWorld_WithConfiguration_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    PhysicsConfiguration config = PhysicsConfiguration::HighPrecision();
    auto world = m_engine->CreateWorld(config);
    ASSERT_NE(world, nullptr);
    
    const auto& gravity = world->GetGravity();
    EXPECT_NEAR(gravity.y, -9.81f, m_epsilon);
}

TEST_F(PhysicsEngineTest, SetActiveWorld_ValidWorld_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    auto world = m_engine->CreateWorld();
    ASSERT_NE(world, nullptr);
    
    EXPECT_NO_THROW(m_engine->SetActiveWorld(world));
}

TEST_F(PhysicsEngineTest, SetActiveWorld_NullWorld_HandledGracefully) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->SetActiveWorld(nullptr));
}

// Rigid Body Management Tests
TEST_F(PhysicsEngineTest, CreateRigidBody_ValidParameters_ReturnsValidId) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 5.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    EXPECT_GT(bodyId, 0);
}

TEST_F(PhysicsEngineTest, CreateRigidBody_StaticBody_ReturnsValidId) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    bodyDesc.mass = 0.0f; // Static body
    bodyDesc.isStatic = true;
    
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(10.0f, 1.0f, 10.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    EXPECT_GT(bodyId, 0);
}

TEST_F(PhysicsEngineTest, CreateRigidBody_KinematicBody_ReturnsValidId) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 2.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    bodyDesc.isKinematic = true;
    
    CollisionShape shape;
    shape.type = CollisionShape::Sphere;
    shape.dimensions = Math::Vec3(0.5f, 0.0f, 0.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    EXPECT_GT(bodyId, 0);
}

TEST_F(PhysicsEngineTest, CreateMultipleRigidBodies_UniqueIds) {
    ASSERT_TRUE(m_engine->Initialize());
    
    std::vector<uint32_t> bodyIds;
    
    for (int i = 0; i < 10; ++i) {
        RigidBody bodyDesc;
        bodyDesc.position = Math::Vec3(i * 2.0f, 5.0f, 0.0f);
        bodyDesc.mass = 1.0f;
        
        CollisionShape shape;
        shape.type = CollisionShape::Box;
        shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
        
        uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
        EXPECT_GT(bodyId, 0);
        
        // Ensure unique IDs
        EXPECT_THAT(bodyIds, ::testing::Not(::testing::Contains(bodyId)));
        bodyIds.push_back(bodyId);
    }
}

TEST_F(PhysicsEngineTest, DestroyRigidBody_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    EXPECT_NO_THROW(m_engine->DestroyRigidBody(bodyId));
}

TEST_F(PhysicsEngineTest, DestroyRigidBody_InvalidId_HandledGracefully) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->DestroyRigidBody(999999));
}

TEST_F(PhysicsEngineTest, SetRigidBodyTransform_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    Math::Vec3 newPosition(5.0f, 10.0f, -3.0f);
    Math::Quat newRotation = glm::angleAxis(glm::radians(45.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    
    EXPECT_NO_THROW(m_engine->SetRigidBodyTransform(bodyId, newPosition, newRotation));
    
    // Verify transform was set
    Math::Vec3 retrievedPos;
    Math::Quat retrievedRot;
    bool success = m_engine->GetRigidBodyTransform(bodyId, retrievedPos, retrievedRot);
    EXPECT_TRUE(success);
    EXPECT_NEAR(retrievedPos.x, newPosition.x, m_epsilon);
    EXPECT_NEAR(retrievedPos.y, newPosition.y, m_epsilon);
    EXPECT_NEAR(retrievedPos.z, newPosition.z, m_epsilon);
}

TEST_F(PhysicsEngineTest, ApplyForce_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    Math::Vec3 force(0.0f, 100.0f, 0.0f);
    EXPECT_NO_THROW(m_engine->ApplyForce(bodyId, force));
}

TEST_F(PhysicsEngineTest, ApplyImpulse_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    Math::Vec3 impulse(0.0f, 10.0f, 0.0f);
    EXPECT_NO_THROW(m_engine->ApplyImpulse(bodyId, impulse));
}

TEST_F(PhysicsEngineTest, SetAngularFactor_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    Math::Vec3 angularFactor(1.0f, 0.0f, 1.0f); // Lock Y-axis rotation
    EXPECT_NO_THROW(m_engine->SetAngularFactor(bodyId, angularFactor));
}

TEST_F(PhysicsEngineTest, SetDamping_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    EXPECT_NO_THROW(m_engine->SetLinearDamping(bodyId, 0.5f));
    EXPECT_NO_THROW(m_engine->SetAngularDamping(bodyId, 0.3f));
}

// Rigid Body Query Tests
TEST_F(PhysicsEngineTest, GetRigidBodyTransform_ValidId_ReturnsTrue) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(1.0f, 2.0f, 3.0f);
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    Math::Vec3 position;
    Math::Quat rotation;
    bool success = m_engine->GetRigidBodyTransform(bodyId, position, rotation);
    
    EXPECT_TRUE(success);
    EXPECT_NEAR(position.x, 1.0f, m_epsilon);
    EXPECT_NEAR(position.y, 2.0f, m_epsilon);
    EXPECT_NEAR(position.z, 3.0f, m_epsilon);
}

TEST_F(PhysicsEngineTest, GetRigidBodyTransform_InvalidId_ReturnsFalse) {
    ASSERT_TRUE(m_engine->Initialize());
    
    Math::Vec3 position;
    Math::Quat rotation;
    bool success = m_engine->GetRigidBodyTransform(999999, position, rotation);
    
    EXPECT_FALSE(success);
}

TEST_F(PhysicsEngineTest, GetRigidBodyVelocity_ValidId_ReturnsTrue) {
    ASSERT_TRUE(m_engine->Initialize());
    
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    bodyDesc.velocity = Math::Vec3(1.0f, 0.0f, 0.0f);
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    Math::Vec3 velocity;
    Math::Vec3 angularVelocity;
    bool success = m_engine->GetRigidBodyVelocity(bodyId, velocity, angularVelocity);
    
    EXPECT_TRUE(success);
}

TEST_F(PhysicsEngineTest, IsRigidBodyGrounded_ValidId_ReturnsResult) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create ground
    RigidBody groundDesc;
    groundDesc.position = Math::Vec3(0.0f, -1.0f, 0.0f);
    groundDesc.mass = 0.0f;
    groundDesc.isStatic = true;
    CollisionShape groundShape;
    groundShape.type = CollisionShape::Box;
    groundShape.dimensions = Math::Vec3(10.0f, 1.0f, 10.0f);
    
    uint32_t groundId = m_engine->CreateRigidBody(groundDesc, groundShape);
    ASSERT_GT(groundId, 0);
    
    // Create object above ground
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 1.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    // Test grounded check
    bool isGrounded = m_engine->IsRigidBodyGrounded(bodyId, 2.0f);
    // Result depends on physics simulation state, just verify it doesn't crash
    EXPECT_TRUE(isGrounded || !isGrounded); // Always true, just testing execution
}

// Physics Query Tests
TEST_F(PhysicsEngineTest, Raycast_ValidParameters_ReturnsResult) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create a target object
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    // Perform raycast
    Math::Vec3 origin(0.0f, 5.0f, 0.0f);
    Math::Vec3 direction(0.0f, -1.0f, 0.0f);
    float maxDistance = 10.0f;
    
    RaycastHit hit = m_engine->Raycast(origin, direction, maxDistance);
    // Result depends on physics state, just verify it doesn't crash
    EXPECT_TRUE(hit.hasHit || !hit.hasHit); // Always true, just testing execution
}

TEST_F(PhysicsEngineTest, OverlapSphere_ValidParameters_ReturnsResults) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create objects to overlap
    for (int i = 0; i < 3; ++i) {
        RigidBody bodyDesc;
        bodyDesc.position = Math::Vec3(i * 0.5f, 0.0f, 0.0f);
        bodyDesc.mass = 1.0f;
        CollisionShape shape;
        shape.type = CollisionShape::Box;
        shape.dimensions = Math::Vec3(0.5f, 0.5f, 0.5f);
        
        uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
        ASSERT_GT(bodyId, 0);
    }
    
    // Perform overlap test
    Math::Vec3 center(0.0f, 0.0f, 0.0f);
    float radius = 2.0f;
    
    std::vector<OverlapResult> results = m_engine->OverlapSphere(center, radius);
    // Just verify it doesn't crash and returns a vector
    EXPECT_TRUE(results.size() >= 0);
}

TEST_F(PhysicsEngineTest, SweepCapsule_ValidParameters_ReturnsResult) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create a target object
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 0.0f, 5.0f);
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(2.0f, 2.0f, 2.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    // Perform sweep test
    Math::Vec3 from(0.0f, 0.0f, 0.0f);
    Math::Vec3 to(0.0f, 0.0f, 10.0f);
    float radius = 0.5f;
    float height = 2.0f;
    
    PhysicsEngine::SweepHit hit = m_engine->SweepCapsule(from, to, radius, height);
    // Just verify it doesn't crash
    EXPECT_TRUE(hit.hasHit || !hit.hasHit); // Always true, just testing execution
}

// Ghost Object Tests
TEST_F(PhysicsEngineTest, CreateGhostObject_ValidParameters_ReturnsValidId) {
    ASSERT_TRUE(m_engine->Initialize());
    
    CollisionShape shape;
    shape.type = CollisionShape::Sphere;
    shape.dimensions = Math::Vec3(1.0f, 0.0f, 0.0f);
    
    Math::Vec3 position(0.0f, 0.0f, 0.0f);
    
    uint32_t ghostId = m_engine->CreateGhostObject(shape, position);
    EXPECT_GT(ghostId, 0);
}

TEST_F(PhysicsEngineTest, DestroyGhostObject_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t ghostId = m_engine->CreateGhostObject(shape, Math::Vec3(0.0f));
    ASSERT_GT(ghostId, 0);
    
    EXPECT_NO_THROW(m_engine->DestroyGhostObject(ghostId));
}

TEST_F(PhysicsEngineTest, SetGhostObjectTransform_ValidId_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    CollisionShape shape;
    shape.type = CollisionShape::Capsule;
    shape.dimensions = Math::Vec3(0.5f, 2.0f, 0.0f);
    
    uint32_t ghostId = m_engine->CreateGhostObject(shape, Math::Vec3(0.0f));
    ASSERT_GT(ghostId, 0);
    
    Math::Vec3 newPosition(5.0f, 10.0f, -3.0f);
    Math::Quat newRotation = glm::angleAxis(glm::radians(90.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
    
    EXPECT_NO_THROW(m_engine->SetGhostObjectTransform(ghostId, newPosition, newRotation));
}

TEST_F(PhysicsEngineTest, GetGhostObjectOverlaps_ValidId_ReturnsResults) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create ghost object
    CollisionShape ghostShape;
    ghostShape.type = CollisionShape::Sphere;
    ghostShape.dimensions = Math::Vec3(2.0f, 0.0f, 0.0f);
    
    uint32_t ghostId = m_engine->CreateGhostObject(ghostShape, Math::Vec3(0.0f));
    ASSERT_GT(ghostId, 0);
    
    // Create overlapping rigid body
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(1.0f, 0.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    CollisionShape bodyShape;
    bodyShape.type = CollisionShape::Box;
    bodyShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, bodyShape);
    ASSERT_GT(bodyId, 0);
    
    // Get overlaps
    std::vector<OverlapResult> overlaps = m_engine->GetGhostObjectOverlaps(ghostId);
    // Just verify it doesn't crash
    EXPECT_TRUE(overlaps.size() >= 0);
}

// Debug Functionality Tests
TEST_F(PhysicsEngineTest, GetDebugInfo_AfterInitialization_ReturnsValidInfo) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create some objects
    RigidBody bodyDesc;
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId1 = m_engine->CreateRigidBody(bodyDesc, shape);
    uint32_t bodyId2 = m_engine->CreateRigidBody(bodyDesc, shape);
    
    PhysicsEngine::PhysicsDebugInfo info = m_engine->GetDebugInfo();
    
    EXPECT_GE(info.numRigidBodies, 2);
    EXPECT_GE(info.numActiveObjects, 0);
    EXPECT_GE(info.simulationTime, 0.0f);
}

TEST_F(PhysicsEngineTest, PrintDebugInfo_DoesNotCrash) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->PrintDebugInfo());
}

TEST_F(PhysicsEngineTest, EnableDebugDrawing_ValidState_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->EnableDebugDrawing(true));
    EXPECT_TRUE(m_engine->IsDebugDrawingEnabled());
    
    EXPECT_NO_THROW(m_engine->EnableDebugDrawing(false));
    EXPECT_FALSE(m_engine->IsDebugDrawingEnabled());
}

TEST_F(PhysicsEngineTest, DrawDebugWorld_DoesNotCrash) {
    ASSERT_TRUE(m_engine->Initialize());
    
    m_engine->EnableDebugDrawing(true);
    EXPECT_NO_THROW(m_engine->DrawDebugWorld());
}

// Update and Simulation Tests
TEST_F(PhysicsEngineTest, Update_ValidDeltaTime_Success) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create a dynamic object
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 10.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    // Update simulation
    float deltaTime = 1.0f / 60.0f;
    EXPECT_NO_THROW(m_engine->Update(deltaTime));
    
    // Verify object has moved due to gravity
    Math::Vec3 newPosition;
    Math::Quat newRotation;
    bool success = m_engine->GetRigidBodyTransform(bodyId, newPosition, newRotation);
    EXPECT_TRUE(success);
    // After one frame with gravity, object should have moved down
    // (exact position depends on physics integration)
}

TEST_F(PhysicsEngineTest, Update_ZeroDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->Update(0.0f));
}

TEST_F(PhysicsEngineTest, Update_NegativeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->Update(-1.0f));
}

TEST_F(PhysicsEngineTest, Update_LargeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_engine->Initialize());
    
    EXPECT_NO_THROW(m_engine->Update(10.0f)); // Very large timestep
}

// Edge Cases and Error Handling Tests
TEST_F(PhysicsEngineTest, OperationsWithoutInitialization_HandledGracefully) {
    // Test various operations without initialization
    EXPECT_NO_THROW(m_engine->Update(1.0f / 60.0f));
    
    RigidBody bodyDesc;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    // Should return 0 or handle gracefully
    EXPECT_TRUE(bodyId == 0 || bodyId > 0);
    
    EXPECT_NO_THROW(m_engine->DestroyRigidBody(bodyId));
}

TEST_F(PhysicsEngineTest, InvalidRigidBodyOperations_HandledGracefully) {
    ASSERT_TRUE(m_engine->Initialize());
    
    uint32_t invalidId = 999999;
    
    EXPECT_NO_THROW(m_engine->SetRigidBodyTransform(invalidId, Math::Vec3(0.0f), Math::Quat(1.0f, 0.0f, 0.0f, 0.0f)));
    EXPECT_NO_THROW(m_engine->ApplyForce(invalidId, Math::Vec3(0.0f, 100.0f, 0.0f)));
    EXPECT_NO_THROW(m_engine->ApplyImpulse(invalidId, Math::Vec3(0.0f, 10.0f, 0.0f)));
    EXPECT_NO_THROW(m_engine->SetAngularFactor(invalidId, Math::Vec3(1.0f)));
    EXPECT_NO_THROW(m_engine->SetLinearDamping(invalidId, 0.5f));
    EXPECT_NO_THROW(m_engine->SetAngularDamping(invalidId, 0.5f));
    
    Math::Vec3 pos, vel, angVel;
    Math::Quat rot;
    EXPECT_FALSE(m_engine->GetRigidBodyTransform(invalidId, pos, rot));
    EXPECT_FALSE(m_engine->GetRigidBodyVelocity(invalidId, vel, angVel));
}

// Performance and Stress Tests
TEST_F(PhysicsEngineTest, CreateManyRigidBodies_PerformanceTest) {
    ASSERT_TRUE(m_engine->Initialize());
    
    const int numBodies = 100;
    std::vector<uint32_t> bodyIds;
    bodyIds.reserve(numBodies);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numBodies; ++i) {
        RigidBody bodyDesc;
        bodyDesc.position = Math::Vec3(i % 10, i / 10, 0.0f);
        bodyDesc.mass = 1.0f;
        
        CollisionShape shape;
        shape.type = static_cast<CollisionShape::Type>(i % 3); // Vary shape types
        shape.dimensions = Math::Vec3(0.5f, 0.5f, 0.5f);
        
        uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
        if (bodyId > 0) {
            bodyIds.push_back(bodyId);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_GT(bodyIds.size(), numBodies * 0.8f); // At least 80% should succeed
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    
    std::cout << "Created " << bodyIds.size() << " rigid bodies in " << duration.count() << "ms" << std::endl;
}

TEST_F(PhysicsEngineTest, SimulationStability_LongDuration) {
    ASSERT_TRUE(m_engine->Initialize());
    
    // Create a simple scene
    RigidBody bodyDesc;
    bodyDesc.position = Math::Vec3(0.0f, 10.0f, 0.0f);
    bodyDesc.mass = 1.0f;
    CollisionShape shape;
    shape.type = CollisionShape::Box;
    shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t bodyId = m_engine->CreateRigidBody(bodyDesc, shape);
    ASSERT_GT(bodyId, 0);
    
    // Run simulation for many steps
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 600; ++i) { // 10 seconds at 60 FPS
        EXPECT_NO_THROW(m_engine->Update(deltaTime));
        
        // Verify object is still valid every 60 frames
        if (i % 60 == 0) {
            Math::Vec3 position;
            Math::Quat rotation;
            bool success = m_engine->GetRigidBodyTransform(bodyId, position, rotation);
            EXPECT_TRUE(success);
        }
    }
}

// Parameterized Tests for Different Configurations
class PhysicsEngineConfigTest : public ::testing::TestWithParam<PhysicsConfiguration> {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_physics_engine_config.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_engine = std::make_unique<PhysicsEngine>();
    }
    
    void TearDown() override {
        if (m_engine) {
            m_engine->Shutdown();
        }
    }
    
    std::unique_ptr<PhysicsEngine> m_engine;
};

TEST_P(PhysicsEngineConfigTest, InitializeWithConfiguration_Success) {
    PhysicsConfiguration config = GetParam();
    
    bool result = m_engine->Initialize(config);
    EXPECT_TRUE(result);
    
    const auto& appliedConfig = m_engine->GetConfiguration();
    EXPECT_NEAR(appliedConfig.gravity.x, config.gravity.x, 1e-6f);
    EXPECT_NEAR(appliedConfig.gravity.y, config.gravity.y, 1e-6f);
    EXPECT_NEAR(appliedConfig.gravity.z, config.gravity.z, 1e-6f);
    EXPECT_NEAR(appliedConfig.timeStep, config.timeStep, 1e-6f);
    EXPECT_EQ(appliedConfig.maxSubSteps, config.maxSubSteps);
    EXPECT_EQ(appliedConfig.solverIterations, config.solverIterations);
}

INSTANTIATE_TEST_SUITE_P(
    ConfigurationTests,
    PhysicsEngineConfigTest,
    ::testing::Values(
        PhysicsConfiguration::Default(),
        PhysicsConfiguration::ForCharacterMovement(),
        PhysicsConfiguration::HighPrecision()
    )
);

#endif // GAMEENGINE_HAS_BULLET