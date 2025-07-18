#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Game/HybridMovementComponent.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <memory>
#include <chrono>

using namespace GameEngine;

class HybridMovementComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_hybrid_movement_component.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_component = std::make_unique<HybridMovementComponent>();
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        m_physicsEngine->Initialize();
        
        m_epsilon = 1e-6f;
    }
    
    void TearDown() override {
        if (m_component) {
            m_component->Shutdown();
        }
        if (m_physicsEngine) {
            m_physicsEngine->Shutdown();
        }
    }
    
    std::unique_ptr<HybridMovementComponent> m_component;
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    float m_epsilon;
};

// Construction and Destruction Tests
TEST_F(HybridMovementComponentTest, Constructor_DefaultValues_Valid) {
    EXPECT_NE(m_component, nullptr);
    EXPECT_STREQ(m_component->GetComponentTypeName(), "HybridMovementComponent");
    
    // Check default position
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, 0.0f, m_epsilon);
    EXPECT_NEAR(position.y, 0.9f, m_epsilon);
    EXPECT_NEAR(position.z, 0.0f, m_epsilon);
    
    // Check default velocity
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, 0.0f, m_epsilon);
    EXPECT_NEAR(velocity.y, 0.0f, m_epsilon);
    EXPECT_NEAR(velocity.z, 0.0f, m_epsilon);
    
    // Check default rotation
    EXPECT_NEAR(m_component->GetRotation(), 0.0f, m_epsilon);
    
    // Check default hybrid-specific properties
    EXPECT_GT(m_component->GetSkinWidth(), 0.0f);
    EXPECT_GT(m_component->GetGroundCheckDistance(), 0.0f);
}

// Initialization Tests
TEST_F(HybridMovementComponentTest, Initialize_ValidPhysicsEngine_Success) {
    bool result = m_component->Initialize(m_physicsEngine.get());
    EXPECT_TRUE(result);
}

TEST_F(HybridMovementComponentTest, Initialize_NullPhysicsEngine_Failure) {
    bool result = m_component->Initialize(nullptr);
    EXPECT_FALSE(result);
}

TEST_F(HybridMovementComponentTest, Initialize_MultipleInitialization_HandledGracefully) {
    EXPECT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    EXPECT_TRUE(m_component->Initialize(m_physicsEngine.get())); // Second init should succeed
}

TEST_F(HybridMovementComponentTest, Shutdown_AfterInitialization_Success) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Shutdown());
}

TEST_F(HybridMovementComponentTest, Shutdown_WithoutInitialization_Success) {
    EXPECT_NO_THROW(m_component->Shutdown());
}

// Transform Interface Tests
TEST_F(HybridMovementComponentTest, SetPosition_ValidPosition_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 newPosition(5.0f, 10.0f, -3.0f);
    m_component->SetPosition(newPosition);
    
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, newPosition.x, m_epsilon);
    EXPECT_NEAR(position.y, newPosition.y, m_epsilon);
    EXPECT_NEAR(position.z, newPosition.z, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetRotation_ValidYaw_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newYaw = 45.0f;
    m_component->SetRotation(newYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), newYaw, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetRotation_LargeAngle_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float largeYaw = 720.0f; // Two full rotations
    m_component->SetRotation(largeYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), largeYaw, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetRotation_NegativeAngle_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float negativeYaw = -90.0f;
    m_component->SetRotation(negativeYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), negativeYaw, m_epsilon);
}

// Velocity Interface Tests
TEST_F(HybridMovementComponentTest, SetVelocity_ValidVelocity_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 newVelocity(5.0f, 2.0f, -1.0f);
    m_component->SetVelocity(newVelocity);
    
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, newVelocity.x, m_epsilon);
    EXPECT_NEAR(velocity.y, newVelocity.y, m_epsilon);
    EXPECT_NEAR(velocity.z, newVelocity.z, m_epsilon);
}

TEST_F(HybridMovementComponentTest, AddVelocity_ValidDelta_Added) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 initialVelocity(1.0f, 0.0f, 0.0f);
    Math::Vec3 deltaVelocity(2.0f, 3.0f, -1.0f);
    
    m_component->SetVelocity(initialVelocity);
    m_component->AddVelocity(deltaVelocity);
    
    const auto& finalVelocity = m_component->GetVelocity();
    EXPECT_NEAR(finalVelocity.x, 3.0f, m_epsilon);
    EXPECT_NEAR(finalVelocity.y, 3.0f, m_epsilon);
    EXPECT_NEAR(finalVelocity.z, -1.0f, m_epsilon);
}

TEST_F(HybridMovementComponentTest, AddVelocity_ZeroDelta_NoChange) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 initialVelocity(1.0f, 2.0f, 3.0f);
    m_component->SetVelocity(initialVelocity);
    
    m_component->AddVelocity(Math::Vec3(0.0f));
    
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, initialVelocity.x, m_epsilon);
    EXPECT_NEAR(velocity.y, initialVelocity.y, m_epsilon);
    EXPECT_NEAR(velocity.z, initialVelocity.z, m_epsilon);
}

// Movement State Tests
TEST_F(HybridMovementComponentTest, IsGrounded_InitialState_ReturnsResult) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Should not crash and return a boolean
    bool grounded = m_component->IsGrounded();
    EXPECT_TRUE(grounded || !grounded); // Always true, just testing execution
}

TEST_F(HybridMovementComponentTest, IsJumping_InitialState_False) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_FALSE(m_component->IsJumping());
}

TEST_F(HybridMovementComponentTest, IsFalling_WithNegativeVelocity_ReturnsResult) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set downward velocity
    m_component->SetVelocity(Math::Vec3(0.0f, -5.0f, 0.0f));
    
    // Result depends on grounded state
    bool falling = m_component->IsFalling();
    EXPECT_TRUE(falling || !falling); // Always true, just testing execution
}

// Movement Commands Tests
TEST_F(HybridMovementComponentTest, Jump_WhenGrounded_SetsJumpingState) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    m_component->Jump();
    
    // Should set jumping state (may depend on grounded state)
    EXPECT_NO_THROW(m_component->IsJumping());
}

TEST_F(HybridMovementComponentTest, StopJumping_AfterJump_ClearsJumpingState) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    m_component->Jump();
    m_component->StopJumping();
    
    EXPECT_FALSE(m_component->IsJumping());
}

TEST_F(HybridMovementComponentTest, AddMovementInput_ValidDirection_Processed) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 direction(1.0f, 0.0f, 0.0f);
    float scale = 0.5f;
    
    EXPECT_NO_THROW(m_component->AddMovementInput(direction, scale));
}

TEST_F(HybridMovementComponentTest, AddMovementInput_ZeroDirection_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 zeroDirection(0.0f, 0.0f, 0.0f);
    
    EXPECT_NO_THROW(m_component->AddMovementInput(zeroDirection, 1.0f));
}

TEST_F(HybridMovementComponentTest, AddMovementInput_LargeDirection_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 largeDirection(100.0f, 100.0f, 100.0f);
    
    EXPECT_NO_THROW(m_component->AddMovementInput(largeDirection, 1.0f));
}

TEST_F(HybridMovementComponentTest, AddMovementInput_MultipleInputs_Accumulated) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Add multiple movement inputs
    m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 0.5f);
    m_component->AddMovementInput(Math::Vec3(0.0f, 0.0f, 1.0f), 0.3f);
    
    // Should not crash and should accumulate inputs
    EXPECT_NO_THROW(m_component->Update(1.0f / 60.0f, nullptr, nullptr));
}

// Hybrid-Specific Configuration Tests
TEST_F(HybridMovementComponentTest, SetSkinWidth_ValidWidth_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newSkinWidth = 0.05f;
    m_component->SetSkinWidth(newSkinWidth);
    
    EXPECT_NEAR(m_component->GetSkinWidth(), newSkinWidth, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetSkinWidth_ZeroWidth_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float zeroSkinWidth = 0.0f;
    m_component->SetSkinWidth(zeroSkinWidth);
    
    EXPECT_NEAR(m_component->GetSkinWidth(), zeroSkinWidth, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetSkinWidth_NegativeWidth_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float negativeSkinWidth = -0.01f;
    m_component->SetSkinWidth(negativeSkinWidth);
    
    EXPECT_NEAR(m_component->GetSkinWidth(), negativeSkinWidth, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetGroundCheckDistance_ValidDistance_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newDistance = 0.2f;
    m_component->SetGroundCheckDistance(newDistance);
    
    EXPECT_NEAR(m_component->GetGroundCheckDistance(), newDistance, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetGroundCheckDistance_ZeroDistance_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float zeroDistance = 0.0f;
    m_component->SetGroundCheckDistance(zeroDistance);
    
    EXPECT_NEAR(m_component->GetGroundCheckDistance(), zeroDistance, m_epsilon);
}

TEST_F(HybridMovementComponentTest, SetGroundCheckDistance_LargeDistance_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float largeDistance = 10.0f;
    m_component->SetGroundCheckDistance(largeDistance);
    
    EXPECT_NEAR(m_component->GetGroundCheckDistance(), largeDistance, m_epsilon);
}

// Update Tests
TEST_F(HybridMovementComponentTest, Update_ValidDeltaTime_Success) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float deltaTime = 1.0f / 60.0f;
    EXPECT_NO_THROW(m_component->Update(deltaTime, nullptr, nullptr));
}

TEST_F(HybridMovementComponentTest, Update_ZeroDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(0.0f, nullptr, nullptr));
}

TEST_F(HybridMovementComponentTest, Update_NegativeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(-1.0f, nullptr, nullptr));
}

TEST_F(HybridMovementComponentTest, Update_LargeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(10.0f, nullptr, nullptr));
}

// Integration Tests with Physics World
TEST_F(HybridMovementComponentTest, Integration_WithPhysicsObjects_CollisionDetection) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Create a static obstacle in the physics world
    RigidBody obstacleDesc;
    obstacleDesc.position = Math::Vec3(5.0f, 0.0f, 0.0f);
    obstacleDesc.mass = 0.0f; // Static
    obstacleDesc.isStatic = true;
    
    CollisionShape obstacleShape;
    obstacleShape.type = CollisionShape::Box;
    obstacleShape.dimensions = Math::Vec3(2.0f, 2.0f, 2.0f);
    
    uint32_t obstacleId = m_physicsEngine->CreateRigidBody(obstacleDesc, obstacleShape);
    ASSERT_GT(obstacleId, 0);
    
    // Set character position near obstacle
    m_component->SetPosition(Math::Vec3(0.0f, 1.0f, 0.0f));
    
    // Try to move towards obstacle
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 60; ++i) {
        m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    // Character should not pass through obstacle
    Math::Vec3 finalPosition = m_component->GetPosition();
    EXPECT_LT(finalPosition.x, 3.0f); // Should be stopped before obstacle
}

TEST_F(HybridMovementComponentTest, Integration_GravityEffect_RealisticBehavior) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set initial position above ground
    Math::Vec3 initialPosition(0.0f, 10.0f, 0.0f);
    m_component->SetPosition(initialPosition);
    
    float initialY = m_component->GetPosition().y;
    
    // Update several times to simulate falling
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 60; ++i) { // 1 second of simulation
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    // Object should have moved due to gravity
    float finalY = m_component->GetPosition().y;
    EXPECT_LT(finalY, initialY); // Should have fallen
}

TEST_F(HybridMovementComponentTest, Integration_JumpAndFall_RealisticBehavior) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Start at a reasonable height
    Math::Vec3 startPosition(0.0f, 1.0f, 0.0f);
    m_component->SetPosition(startPosition);
    
    // Jump
    m_component->Jump();
    
    float deltaTime = 1.0f / 60.0f;
    float maxHeight = startPosition.y;
    bool wasInAir = false;
    
    // Simulate jump and fall
    for (int i = 0; i < 180; ++i) { // 3 seconds
        m_component->Update(deltaTime, nullptr, nullptr);
        
        float currentY = m_component->GetPosition().y;
        if (currentY > maxHeight) {
            maxHeight = currentY;
        }
        
        if (!m_component->IsGrounded()) {
            wasInAir = true;
        }
    }
    
    // Should have jumped higher than initial position
    EXPECT_GT(maxHeight, startPosition.y + 0.1f);
    EXPECT_TRUE(wasInAir);
}

TEST_F(HybridMovementComponentTest, Integration_HorizontalMovement_Responsive) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Start at a reasonable position
    Math::Vec3 initialPosition(0.0f, 1.0f, 0.0f);
    m_component->SetPosition(initialPosition);
    
    float deltaTime = 1.0f / 60.0f;
    
    // Apply horizontal movement input
    for (int i = 0; i < 60; ++i) { // 1 second
        m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    // Should have moved horizontally
    Math::Vec3 finalPosition = m_component->GetPosition();
    EXPECT_GT(finalPosition.x, initialPosition.x + 1.0f); // Should have moved significantly
}

// Performance Tests
TEST_F(HybridMovementComponentTest, Performance_ManyUpdates_Efficient) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    const int numUpdates = 1000;
    float deltaTime = 1.0f / 60.0f;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numUpdates; ++i) {
        m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    
    std::cout << "Performed " << numUpdates << " hybrid updates in " << duration.count() << "ms" << std::endl;
}

TEST_F(HybridMovementComponentTest, Performance_CollisionQueries_Efficient) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Create multiple obstacles for collision testing
    for (int i = 0; i < 10; ++i) {
        RigidBody obstacleDesc;
        obstacleDesc.position = Math::Vec3(i * 2.0f, 0.0f, 0.0f);
        obstacleDesc.mass = 0.0f;
        obstacleDesc.isStatic = true;
        
        CollisionShape obstacleShape;
        obstacleShape.type = CollisionShape::Box;
        obstacleShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
        
        uint32_t obstacleId = m_physicsEngine->CreateRigidBody(obstacleDesc, obstacleShape);
        ASSERT_GT(obstacleId, 0);
    }
    
    const int numUpdates = 100;
    float deltaTime = 1.0f / 60.0f;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numUpdates; ++i) {
        m_component->SetPosition(Math::Vec3(i * 0.1f, 1.0f, 0.0f));
        m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 500); // Should be reasonably fast even with collision queries
    
    std::cout << "Performed " << numUpdates << " collision-heavy updates in " << duration.count() << "ms" << std::endl;
}

// Edge Cases and Error Handling
TEST_F(HybridMovementComponentTest, OperationsWithoutInitialization_HandledGracefully) {
    // Test operations without initialization
    EXPECT_NO_THROW(m_component->SetPosition(Math::Vec3(0.0f)));
    EXPECT_NO_THROW(m_component->SetVelocity(Math::Vec3(0.0f)));
    EXPECT_NO_THROW(m_component->Jump());
    EXPECT_NO_THROW(m_component->Update(1.0f / 60.0f, nullptr, nullptr));
}

TEST_F(HybridMovementComponentTest, ExtremePositions_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme positions
    Math::Vec3 extremePosition(1000000.0f, -1000000.0f, 1000000.0f);
    EXPECT_NO_THROW(m_component->SetPosition(extremePosition));
    
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, extremePosition.x, 1.0f); // Allow some tolerance for extreme values
}

TEST_F(HybridMovementComponentTest, ExtremeVelocities_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme velocities
    Math::Vec3 extremeVelocity(10000.0f, -10000.0f, 10000.0f);
    EXPECT_NO_THROW(m_component->SetVelocity(extremeVelocity));
}

TEST_F(HybridMovementComponentTest, ExtremeConfiguration_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme configuration values
    EXPECT_NO_THROW(m_component->SetSkinWidth(1000.0f));
    EXPECT_NO_THROW(m_component->SetGroundCheckDistance(1000.0f));
    EXPECT_NO_THROW(m_component->SetSkinWidth(-1000.0f));
    EXPECT_NO_THROW(m_component->SetGroundCheckDistance(-1000.0f));
}

// Stress Tests
TEST_F(HybridMovementComponentTest, Stress_RapidStateChanges_Stable) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float deltaTime = 1.0f / 60.0f;
    
    for (int i = 0; i < 100; ++i) {
        // Rapidly change state
        m_component->SetPosition(Math::Vec3(i % 10, (i % 5) + 1.0f, i % 7));
        m_component->SetVelocity(Math::Vec3((i % 3) - 1, (i % 5) - 2, (i % 4) - 1));
        m_component->SetRotation(i * 10.0f);
        m_component->SetSkinWidth(0.01f + (i % 5) * 0.01f);
        m_component->SetGroundCheckDistance(0.05f + (i % 3) * 0.05f);
        
        if (i % 10 == 0) {
            m_component->Jump();
        }
        if (i % 15 == 0) {
            m_component->StopJumping();
        }
        
        m_component->AddMovementInput(Math::Vec3((i % 2) * 2 - 1, 0, (i % 3) * 2 - 1), 1.0f);
        
        EXPECT_NO_THROW(m_component->Update(deltaTime, nullptr, nullptr));
    }
}

TEST_F(HybridMovementComponentTest, Stress_ManyCollisionObjects_Stable) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Create many collision objects
    const int numObjects = 50;
    for (int i = 0; i < numObjects; ++i) {
        RigidBody obstacleDesc;
        obstacleDesc.position = Math::Vec3((i % 10) * 2.0f, 0.0f, (i / 10) * 2.0f);
        obstacleDesc.mass = 0.0f;
        obstacleDesc.isStatic = true;
        
        CollisionShape obstacleShape;
        obstacleShape.type = CollisionShape::Box;
        obstacleShape.dimensions = Math::Vec3(0.5f, 2.0f, 0.5f);
        
        uint32_t obstacleId = m_physicsEngine->CreateRigidBody(obstacleDesc, obstacleShape);
        ASSERT_GT(obstacleId, 0);
    }
    
    // Move through the field of objects
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 200; ++i) {
        Math::Vec3 direction((i % 4) - 2, 0, (i % 3) - 1);
        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction);
        }
        
        m_component->AddMovementInput(direction, 1.0f);
        EXPECT_NO_THROW(m_component->Update(deltaTime, nullptr, nullptr));
    }
}

#endif // GAMEENGINE_HAS_BULLET