#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Game/DeterministicMovementComponent.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <memory>
#include <chrono>

using namespace GameEngine;

class DeterministicMovementComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_deterministic_movement_component.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_component = std::make_unique<DeterministicMovementComponent>();
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
    
    std::unique_ptr<DeterministicMovementComponent> m_component;
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    float m_epsilon;
};

// Construction and Destruction Tests
TEST_F(DeterministicMovementComponentTest, Constructor_DefaultValues_Valid) {
    EXPECT_NE(m_component, nullptr);
    EXPECT_STREQ(m_component->GetComponentTypeName(), "DeterministicMovementComponent");
    
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
    
    // Check default ground level
    EXPECT_GT(m_component->GetGroundLevel(), 0.0f);
    
    // Check default gravity
    EXPECT_LT(m_component->GetGravity(), 0.0f); // Should be negative
}

// Initialization Tests
TEST_F(DeterministicMovementComponentTest, Initialize_ValidPhysicsEngine_Success) {
    bool result = m_component->Initialize(m_physicsEngine.get());
    EXPECT_TRUE(result);
}

TEST_F(DeterministicMovementComponentTest, Initialize_NullPhysicsEngine_Success) {
    // Deterministic component should work without physics engine
    bool result = m_component->Initialize(nullptr);
    EXPECT_TRUE(result);
}

TEST_F(DeterministicMovementComponentTest, Initialize_MultipleInitialization_HandledGracefully) {
    EXPECT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    EXPECT_TRUE(m_component->Initialize(m_physicsEngine.get())); // Second init should succeed
}

TEST_F(DeterministicMovementComponentTest, Shutdown_AfterInitialization_Success) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Shutdown());
}

TEST_F(DeterministicMovementComponentTest, Shutdown_WithoutInitialization_Success) {
    EXPECT_NO_THROW(m_component->Shutdown());
}

// Transform Interface Tests
TEST_F(DeterministicMovementComponentTest, SetPosition_ValidPosition_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 newPosition(5.0f, 10.0f, -3.0f);
    m_component->SetPosition(newPosition);
    
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, newPosition.x, m_epsilon);
    EXPECT_NEAR(position.y, newPosition.y, m_epsilon);
    EXPECT_NEAR(position.z, newPosition.z, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetRotation_ValidYaw_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newYaw = 45.0f;
    m_component->SetRotation(newYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), newYaw, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetRotation_LargeAngle_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float largeYaw = 720.0f; // Two full rotations
    m_component->SetRotation(largeYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), largeYaw, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetRotation_NegativeAngle_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float negativeYaw = -90.0f;
    m_component->SetRotation(negativeYaw);
    
    EXPECT_NEAR(m_component->GetRotation(), negativeYaw, m_epsilon);
}

// Velocity Interface Tests
TEST_F(DeterministicMovementComponentTest, SetVelocity_ValidVelocity_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 newVelocity(5.0f, 2.0f, -1.0f);
    m_component->SetVelocity(newVelocity);
    
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, newVelocity.x, m_epsilon);
    EXPECT_NEAR(velocity.y, newVelocity.y, m_epsilon);
    EXPECT_NEAR(velocity.z, newVelocity.z, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, AddVelocity_ValidDelta_Added) {
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

TEST_F(DeterministicMovementComponentTest, AddVelocity_ZeroDelta_NoChange) {
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
TEST_F(DeterministicMovementComponentTest, IsGrounded_AtGroundLevel_True) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set position at ground level
    Math::Vec3 groundPosition(0.0f, m_component->GetGroundLevel(), 0.0f);
    m_component->SetPosition(groundPosition);
    
    EXPECT_TRUE(m_component->IsGrounded());
}

TEST_F(DeterministicMovementComponentTest, IsGrounded_AboveGround_False) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set position above ground
    Math::Vec3 airPosition(0.0f, m_component->GetGroundLevel() + 1.0f, 0.0f);
    m_component->SetPosition(airPosition);
    
    EXPECT_FALSE(m_component->IsGrounded());
}

TEST_F(DeterministicMovementComponentTest, IsJumping_InitialState_False) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_FALSE(m_component->IsJumping());
}

TEST_F(DeterministicMovementComponentTest, IsFalling_WithNegativeVelocity_True) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set position above ground with downward velocity
    Math::Vec3 airPosition(0.0f, m_component->GetGroundLevel() + 1.0f, 0.0f);
    m_component->SetPosition(airPosition);
    m_component->SetVelocity(Math::Vec3(0.0f, -5.0f, 0.0f));
    
    EXPECT_TRUE(m_component->IsFalling());
}

TEST_F(DeterministicMovementComponentTest, IsFalling_OnGround_False) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set position at ground level
    Math::Vec3 groundPosition(0.0f, m_component->GetGroundLevel(), 0.0f);
    m_component->SetPosition(groundPosition);
    
    EXPECT_FALSE(m_component->IsFalling());
}

// Movement Commands Tests
TEST_F(DeterministicMovementComponentTest, Jump_WhenGrounded_SetsUpwardVelocity) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Ensure character is grounded
    Math::Vec3 groundPosition(0.0f, m_component->GetGroundLevel(), 0.0f);
    m_component->SetPosition(groundPosition);
    
    m_component->Jump();
    
    EXPECT_TRUE(m_component->IsJumping());
    
    // Should have upward velocity after jump
    const auto& velocity = m_component->GetVelocity();
    EXPECT_GT(velocity.y, 0.0f);
}

TEST_F(DeterministicMovementComponentTest, Jump_WhenInAir_NoEffect) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set position in air
    Math::Vec3 airPosition(0.0f, m_component->GetGroundLevel() + 2.0f, 0.0f);
    m_component->SetPosition(airPosition);
    
    Math::Vec3 initialVelocity = m_component->GetVelocity();
    m_component->Jump();
    
    // Velocity should not change when jumping in air
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, initialVelocity.x, m_epsilon);
    EXPECT_NEAR(velocity.y, initialVelocity.y, m_epsilon);
    EXPECT_NEAR(velocity.z, initialVelocity.z, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, StopJumping_AfterJump_ClearsJumpingState) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Ensure character is grounded and jump
    Math::Vec3 groundPosition(0.0f, m_component->GetGroundLevel(), 0.0f);
    m_component->SetPosition(groundPosition);
    m_component->Jump();
    
    EXPECT_TRUE(m_component->IsJumping());
    
    m_component->StopJumping();
    
    EXPECT_FALSE(m_component->IsJumping());
}

TEST_F(DeterministicMovementComponentTest, AddMovementInput_ValidDirection_Processed) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 direction(1.0f, 0.0f, 0.0f);
    float scale = 0.5f;
    
    EXPECT_NO_THROW(m_component->AddMovementInput(direction, scale));
}

TEST_F(DeterministicMovementComponentTest, AddMovementInput_ZeroDirection_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    Math::Vec3 zeroDirection(0.0f, 0.0f, 0.0f);
    
    EXPECT_NO_THROW(m_component->AddMovementInput(zeroDirection, 1.0f));
}

TEST_F(DeterministicMovementComponentTest, AddMovementInput_MultipleInputs_Accumulated) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Add multiple movement inputs
    m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 0.5f);
    m_component->AddMovementInput(Math::Vec3(0.0f, 0.0f, 1.0f), 0.3f);
    
    // Should not crash and should accumulate inputs
    EXPECT_NO_THROW(m_component->Update(1.0f / 60.0f, nullptr, nullptr));
}

// Deterministic-Specific Configuration Tests
TEST_F(DeterministicMovementComponentTest, SetGroundLevel_ValidLevel_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newGroundLevel = 5.0f;
    m_component->SetGroundLevel(newGroundLevel);
    
    EXPECT_NEAR(m_component->GetGroundLevel(), newGroundLevel, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetGroundLevel_NegativeLevel_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float negativeGroundLevel = -2.0f;
    m_component->SetGroundLevel(negativeGroundLevel);
    
    EXPECT_NEAR(m_component->GetGroundLevel(), negativeGroundLevel, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetGravity_ValidGravity_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float newGravity = -20.0f;
    m_component->SetGravity(newGravity);
    
    EXPECT_NEAR(m_component->GetGravity(), newGravity, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetGravity_PositiveGravity_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float positiveGravity = 10.0f; // Upward gravity
    m_component->SetGravity(positiveGravity);
    
    EXPECT_NEAR(m_component->GetGravity(), positiveGravity, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, SetGravity_ZeroGravity_Applied) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float zeroGravity = 0.0f;
    m_component->SetGravity(zeroGravity);
    
    EXPECT_NEAR(m_component->GetGravity(), zeroGravity, m_epsilon);
}

// Update Tests
TEST_F(DeterministicMovementComponentTest, Update_ValidDeltaTime_Success) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float deltaTime = 1.0f / 60.0f;
    EXPECT_NO_THROW(m_component->Update(deltaTime, nullptr, nullptr));
}

TEST_F(DeterministicMovementComponentTest, Update_ZeroDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(0.0f, nullptr, nullptr));
}

TEST_F(DeterministicMovementComponentTest, Update_NegativeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(-1.0f, nullptr, nullptr));
}

TEST_F(DeterministicMovementComponentTest, Update_LargeDeltaTime_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    EXPECT_NO_THROW(m_component->Update(10.0f, nullptr, nullptr));
}

// Deterministic Behavior Tests
TEST_F(DeterministicMovementComponentTest, Deterministic_SameInputsSameResults_Consistent) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set initial state
    Math::Vec3 initialPosition(0.0f, 5.0f, 0.0f);
    Math::Vec3 initialVelocity(0.0f, 0.0f, 0.0f);
    m_component->SetPosition(initialPosition);
    m_component->SetVelocity(initialVelocity);
    
    // Run simulation 1
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 60; ++i) {
        m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    Math::Vec3 result1 = m_component->GetPosition();
    
    // Reset and run simulation 2 with same inputs
    m_component->SetPosition(initialPosition);
    m_component->SetVelocity(initialVelocity);
    
    for (int i = 0; i < 60; ++i) {
        m_component->AddMovementInput(Math::Vec3(1.0f, 0.0f, 0.0f), 1.0f);
        m_component->Update(deltaTime, nullptr, nullptr);
    }
    
    Math::Vec3 result2 = m_component->GetPosition();
    
    // Results should be identical (deterministic)
    EXPECT_NEAR(result1.x, result2.x, m_epsilon);
    EXPECT_NEAR(result1.y, result2.y, m_epsilon);
    EXPECT_NEAR(result1.z, result2.z, m_epsilon);
}

// Integration Tests
TEST_F(DeterministicMovementComponentTest, Integration_GravityEffect_RealisticBehavior) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set initial position above ground
    Math::Vec3 initialPosition(0.0f, m_component->GetGroundLevel() + 5.0f, 0.0f);
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

TEST_F(DeterministicMovementComponentTest, Integration_GroundCollision_StopsAtGround) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Set initial position above ground
    Math::Vec3 initialPosition(0.0f, m_component->GetGroundLevel() + 10.0f, 0.0f);
    m_component->SetPosition(initialPosition);
    
    // Simulate falling until grounded
    float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 300; ++i) { // 5 seconds max
        m_component->Update(deltaTime, nullptr, nullptr);
        
        if (m_component->IsGrounded()) {
            break;
        }
    }
    
    // Should be grounded and at ground level
    EXPECT_TRUE(m_component->IsGrounded());
    EXPECT_NEAR(m_component->GetPosition().y, m_component->GetGroundLevel(), 0.1f);
}

TEST_F(DeterministicMovementComponentTest, Integration_JumpAndFall_RealisticBehavior) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Start at ground level
    Math::Vec3 groundPosition(0.0f, m_component->GetGroundLevel(), 0.0f);
    m_component->SetPosition(groundPosition);
    
    // Jump
    m_component->Jump();
    
    float deltaTime = 1.0f / 60.0f;
    float maxHeight = groundPosition.y;
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
    EXPECT_GT(maxHeight, groundPosition.y + 0.1f);
    EXPECT_TRUE(wasInAir);
    EXPECT_TRUE(m_component->IsGrounded()); // Should land back on ground
}

TEST_F(DeterministicMovementComponentTest, Integration_HorizontalMovement_Responsive) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Start at ground level
    Math::Vec3 initialPosition(0.0f, m_component->GetGroundLevel(), 0.0f);
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
    EXPECT_NEAR(finalPosition.y, initialPosition.y, 0.1f); // Y should remain roughly the same
}

// Performance Tests
TEST_F(DeterministicMovementComponentTest, Performance_ManyUpdates_Efficient) {
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
    
    EXPECT_LT(duration.count(), 100); // Should be very fast (deterministic)
    
    std::cout << "Performed " << numUpdates << " deterministic updates in " << duration.count() << "ms" << std::endl;
}

// Edge Cases and Error Handling
TEST_F(DeterministicMovementComponentTest, OperationsWithoutInitialization_HandledGracefully) {
    // Test operations without initialization
    EXPECT_NO_THROW(m_component->SetPosition(Math::Vec3(0.0f)));
    EXPECT_NO_THROW(m_component->SetVelocity(Math::Vec3(0.0f)));
    EXPECT_NO_THROW(m_component->Jump());
    EXPECT_NO_THROW(m_component->Update(1.0f / 60.0f, nullptr, nullptr));
}

TEST_F(DeterministicMovementComponentTest, ExtremePositions_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme positions
    Math::Vec3 extremePosition(1000000.0f, -1000000.0f, 1000000.0f);
    EXPECT_NO_THROW(m_component->SetPosition(extremePosition));
    
    const auto& position = m_component->GetPosition();
    EXPECT_NEAR(position.x, extremePosition.x, m_epsilon);
    EXPECT_NEAR(position.y, extremePosition.y, m_epsilon);
    EXPECT_NEAR(position.z, extremePosition.z, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, ExtremeVelocities_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme velocities
    Math::Vec3 extremeVelocity(10000.0f, -10000.0f, 10000.0f);
    EXPECT_NO_THROW(m_component->SetVelocity(extremeVelocity));
    
    const auto& velocity = m_component->GetVelocity();
    EXPECT_NEAR(velocity.x, extremeVelocity.x, m_epsilon);
    EXPECT_NEAR(velocity.y, extremeVelocity.y, m_epsilon);
    EXPECT_NEAR(velocity.z, extremeVelocity.z, m_epsilon);
}

TEST_F(DeterministicMovementComponentTest, ExtremeGravity_HandledGracefully) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    // Test extreme gravity values
    EXPECT_NO_THROW(m_component->SetGravity(-1000.0f));
    EXPECT_NEAR(m_component->GetGravity(), -1000.0f, m_epsilon);
    
    EXPECT_NO_THROW(m_component->SetGravity(1000.0f));
    EXPECT_NEAR(m_component->GetGravity(), 1000.0f, m_epsilon);
}

// Stress Tests
TEST_F(DeterministicMovementComponentTest, Stress_RapidStateChanges_Stable) {
    ASSERT_TRUE(m_component->Initialize(m_physicsEngine.get()));
    
    float deltaTime = 1.0f / 60.0f;
    
    for (int i = 0; i < 100; ++i) {
        // Rapidly change state
        m_component->SetPosition(Math::Vec3(i % 10, (i % 5) + m_component->GetGroundLevel(), i % 7));
        m_component->SetVelocity(Math::Vec3((i % 3) - 1, (i % 5) - 2, (i % 4) - 1));
        m_component->SetRotation(i * 10.0f);
        m_component->SetGroundLevel((i % 3) + 0.5f);
        m_component->SetGravity(-10.0f - (i % 5));
        
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