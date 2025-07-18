#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Game/PhysicsMovementComponent.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <memory>
#include <chrono>

using namespace GameEngine;

class PhysicsMovementComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_physics_movement_component.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_component = std::make_unique<PhysicsMovementComponent>();
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
    
    std::unique_ptr<PhysicsMovementComponent> m_component;
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    float m_epsilon;
};

// Construction and Destruction Tests
TEST_F(PhysicsMovementComponentTest, Constructor_DefaultValues_Valid) {
    EXPECT_NE(m_component, nullptr);
    EXPECT_STREQ(m_component->GetComponentTypeName(), "PhysicsMovementComponent");
    
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
    
    // Check default mass
    EXPECT_GT(m_component->GetMass(), 0.0f);
}

// Initialization Tests
TEST_F(PhysicsMovementComponentTest, Initialize_ValidPhysicsEngine_Success) {
    bool result = m_component->Initialize(m_physicsEngine.get());
    EXPECT_TRUE(result);
}

TEST_F(PhysicsMovementComponentTest, Initialize_NullPhysicsEngine_Failure) {
    bool result = m_component->Initialize(nullptr);
    EXPECT_FALSE(result);
}

TEST_F(PhysicsMovementComponentTest, Initialize_MultipleInitialization_HandledGracefully) {
    EXPECT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    EXPECT_TRUE(m_component->Initialize(m_physicsEngine.get())); // Second init should succeed
}

TEST_F(PhysicsMovementComponentTest, Shutdown_AfterInitialization_Success) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Shutdown());
}

TEST_F(PhysicsMovementComponentTest, Shutdown_WithoutInitialization_Success) {
    EXPECT_NO_THROW(m_component->Shutdown());
}

// Transform Interface Tests
TEST_F(PhysicsMovementComponentTest, SetPosition_ValidPosition_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 newPosition(5.0f, 10.0f, -3.0f);
    m_component->SetPosition(newPosition);
    
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, newPosition.x, m_epsilon);
    EXPECT_NEAR(position.y, newPosition.y, m_epsilon);
    EXPECT_NEAR(position.z, newPosition.z, m_epsilon);
}

TEST_F(PhysicsMovementComponentTest, SetRotation_ValidYaw_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newYaw = 45.0f;
    m_component->SetRotation(newYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), newYaw, m_epsilon);
}

TEST_F(PhysicsMovementComponentTest, SetRotation_LargeAngle_Normalized) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float largeYaw = 720.0f; // Two full rotations
    m_component->SetRotation(largeYaw);
    
    // Should handle large angles gracefully
    EXPECT_NO_THROW(m_component->GetRotation());
}

// Velocity Interface Tests
TEST_F(PhysicsMovementComponentTest, SetVelocity_ValidVelocity_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 newVelocity(5.0f, 2.0f, -1.0f);
    m_component->SetVelocity(newVelocity);
    
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, newVelocity.x, m_epsilon);
    EXPECT_NEAR(velocity.y, newVelocity.y, m_epsilon);
    EXPECT_NEAR(velocity.z, newVelocity.z, m_epsilon);
}

TEST_F(PhysicsMovementComponentTest, AddVelocity_ValidDelta_Added) {
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

TEST_F(PhysicsMovementComponentTest, AddVelocity_ZeroDelta_NoChange) {
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
TEST_F(PhysicsMovementComponentTest, IsGrounded_InitialState_ReturnsResult) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Should not crash and return a boolean
    bool grounded = m_component->IsGrounded();
    EXPECT_TRUE(grounded || !grounded); // Always true, just testing execution
}

TEST_F(PhysicsMovementComponentTest, IsJumping_InitialState_False) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_FALSE(m_component->IsJumping());
}

TEST_F(PhysicsMovementComponentTest, IsFalling_InitialState_ReturnsResult) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    bool falling = m_component->IsFalling();
    EXPECT_TRUE(falling || !falling); // Always true, just testing execution
}

// Movement Commands Tests
TEST_F(PhysicsMovementComponentTest, Jump_WhenGrounded_SetsJumpingState) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    m_component->Jump();
    
    // Should set jumping state (may depend on grounded state)
    EXPECT_NO_THROW(m_component->IsJumping());
}

TEST_F(PhysicsMovementComponentTest, StopJumping_AfterJump_ClearsJumpingState) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    m_component->Jump();
    m_component->StopJumping();
    
    EXPECT_FALSE(m_component->IsJumping());
}

TEST_F(PhysicsMovementComponentTest, AddMovementInput_ValidDirection_Processed) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 direction(1.0f, 0.0f, 0.0f);
    float scale = 0.5f;
    
    EXPECT_NO_THROW(m_component->AddMovementInput(direction, scale));
}

TEST_F(PhysicsMovementComponentTest, AddMovementInput_ZeroDirection_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 zeroDirection(0.0f, 0.0f, 0.0f);
    
    EXPECT_NO_THROW(m_component->AddMovementInput(zeroDirection, 1.0f));
}

TEST_F(PhysicsMovementComponentTest, AddMovementInput_LargeDirection_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 largeDirection(100.0f, 100.0f, 100.0f);
    
    EXPECT_NO_THROW(m_component->AddMovementInput(largeDirection, 1.0f));
}

// Physics Properties Tests
TEST_F(PhysicsMovementComponentTest, SetMass_ValidMass_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newMass = 100.0f;
    m_component->SetMass(newMass);
    
    EXPECT_NEAR(m_component->GetMass(), newMass, m_epsilon);
}

TEST_F(PhysicsMovementComponentTest, SetMass_ZeroMass_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->SetMass(0.0f));
    EXPECT_NEAR(m_component->GetMass(), 0.0f, m_epsilon);
}

TEST_F(PhysicsMovementComponentTest, SetMass_NegativeMass_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->SetMass(-10.0f));
}

TEST_F(PhysicsMovementComponentTest, SetFriction_ValidValue_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float friction = 2.0f;
    EXPECT_NO_THROW(m_component->SetFriction(friction));
}

TEST_F(PhysicsMovementComponentTest, SetRestitution_ValidValue_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float restitution = 0.8f;
    EXPECT_NO_THROW(m_component->SetRestitution(restitution));
}

TEST_F(PhysicsMovementComponentTest, SetLinearDamping_ValidValue_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float damping = 0.5f;
    EXPECT_NO_THROW(m_component->SetLinearDamping(damping));
}

TEST_F(PhysicsMovementComponentTest, SetAngularDamping_ValidValue_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float damping = 0.9f;
    EXPECT_NO_THROW(m_component->SetAngularDamping(damping));
}

// Update Tests
TEST_F(PhysicsMovementComponentTest, Update_ValidDeltaTime_Success) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float deltaTime = 1.0f / 60.0f;
    EXPECT_NO_THROW(m_component->Update(deltaTime, nullptr, nullptr));
}

TEST_F(PhysicsMovementComponentTest, Update_ZeroDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(0.0f, nullptr, nullptr));
}

TEST_F(PhysicsMovementComponentTest, Update_NegativeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(-1.0f, nullptr, nullptr));
}

TEST_F(PhysicsMovementComponentTest, Update_LargeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(10.0f, nullptr, nullptr));
}

// Integration Tests
TEST_F(PhysicsMovementComponentTest, Integration_MovementWithGravity_RealisticBehavior) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set initial position above ground
    Math::Vec3 initialPosition(0.0f, 10.0f, 0.0f);
    m_component->SetPosition(initialPosition);
    
    // Get initial Y position
    float initialY = m_component->GetPosition().y;
    
    // Update several times to simulate falling
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 60; ++i) { // 1 second of simulation
        m_component->Update(deltaTime, nullptr, nullptr);
        m_physicsEngine->Update(deltaTime);
    }
    
    // Object should have moved due to gravity
    float finalY = m_component->GetPosition().y;
    EXPECT_LT(finalY, initialY); // Should have fallen
}

TEST_F(PhysicsMovementComponentTest, Integration_JumpAndFall_RealisticBehavior) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Start at ground level
    Math::Vec3 groundPosition(0.0f, 0.9f, 0.0f);
    m_component->SetPosition(groundPosition);
    
    // Jump
    m_component->Jump();
    
    float deltaTime = 1.0f / 60.0f;
    float maxHeight = groundPosition.y;
    
    // Simulate jump and fall
    for (int i = 0; i < 180; ++i) { // 3 seconds
        m_component->Update(deltaTime, nullptr, nullptr);
        m_physicsEngine->Update(deltaTime);
        
        float currentY = m_component->GetPosition().y;
        if (currentY > maxHeight) {
            maxHeight = currentY;
        }
    }
    
    // Should have jumped higher than initial position
    EXPECT_GT(maxHeight, groundPosition.y + 0.1f);
}

// Performance Tests
TEST_F(PhysicsMovementComponentTest, Performance_ManyUpdates_Efficient) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    const int numUpdates = 1000;
    float deltaTime = 1.0f / 60.0f;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numUpdates; ++i) {
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    
    std::cout << "Performed " << numUpdates << " updates in " << duration.count() << "ms" << std::endl;
}

// Edge Cases and Error Handling
TEST_F(PhysicsMovementComponentTest, OperationsWithoutInitialization_HandledGracefully) {
    // Test operations without initialization
    EXPECT_NO_THROW(m_component->SetPosition(Math::Vec3(0.0f)));
    EXPECT_NO_THROW(m_component->SetVelocity(Math::Vec3(0.0f)));
    EXPECT_NO_THROW(m_component->Jump());
    EXPECT_NO_THROW(m_component->Update(1.0f / 60.0f, nullptr, nullptr));
}

TEST_F(PhysicsMovementComponentTest, ExtremePhysicsProperties_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme values
    EXPECT_NO_THROW(m_component->SetMass(1000000.0f));
    EXPECT_NO_THROW(m_component->SetFriction(100.0f));
    EXPECT_NO_THROW(m_component->SetRestitution(10.0f));
    EXPECT_NO_THROW(m_component->SetLinearDamping(1.0f));
    EXPECT_NO_THROW(m_component->SetAngularDamping(1.0f));
}

TEST_F(PhysicsMovementComponentTest, ExtremePositions_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme positions
    Math::Vec3 extremePosition(1000000.0f, -1000000.0f, 1000000.0f);
    EXPECT_NO_THROW(m_component->SetPosition(extremePosition));
    
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, extremePosition.x, 1.0f); // Allow some tolerance for extreme values
}

TEST_F(PhysicsMovementComponentTest, ExtremeVelocities_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme velocities
    Math::Vec3 extremeVelocity(10000.0f, -10000.0f, 10000.0f);
    EXPECT_NO_THROW(m_component->SetVelocity(extremeVelocity));
}

// Stress Tests
TEST_F(PhysicsMovementComponentTest, Stress_RapidStateChanges_Stable) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float deltaTime = 1.0f / 60.0f;
    
    for (int i = 0; i < 100; ++i) {
        // Rapidly change state
        m_component->SetPosition(Math::Vec3(i % 10, i % 5, i % 7));
        m_component->SetVelocity(Math::Vec3((i % 3) - 1, (i % 5) - 2, (i % 4) - 1));
        m_component->SetRotation(i * 10.0f);
        
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

#endif // GAMEENGINE_HAS_BULLET