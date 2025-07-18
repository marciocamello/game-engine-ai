#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Game/CharacterMovementComponent.h"
#include "Game/PhysicsMovementComponent.h"
#include "Game/DeterministicMovementComponent.h"
#include "Game/HybridMovementComponent.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <memory>

using namespace GameEngine;

// Mock implementation for testing base class functionality
class MockCharacterMovementComponent : public CharacterMovementComponent {
public:
    MockCharacterMovementComponent() = default;
    virtual ~MockCharacterMovementComponent() = default;

    // Mock implementations of pure virtual methods
    MOCK_METHOD(bool, Initialize, (PhysicsEngine* physicsEngine), (override));
    MOCK_METHOD(void, Update, (float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    
    MOCK_METHOD(void, SetPosition, (const Math::Vec3& position), (override));
    MOCK_METHOD(const Math::Vec3&, GetPosition, (), (const, override));
    MOCK_METHOD(void, SetRotation, (float yaw), (override));
    MOCK_METHOD(float, GetRotation, (), (const, override));
    
    MOCK_METHOD(const Math::Vec3&, GetVelocity, (), (const, override));
    MOCK_METHOD(void, SetVelocity, (const Math::Vec3& velocity), (override));
    MOCK_METHOD(void, AddVelocity, (const Math::Vec3& deltaVelocity), (override));
    
    MOCK_METHOD(bool, IsGrounded, (), (const, override));
    MOCK_METHOD(bool, IsJumping, (), (const, override));
    MOCK_METHOD(bool, IsFalling, (), (const, override));
    
    MOCK_METHOD(void, Jump, (), (override));
    MOCK_METHOD(void, StopJumping, (), (override));
    MOCK_METHOD(void, AddMovementInput, (const Math::Vec3& worldDirection, float scaleValue), (override));
    
    MOCK_METHOD(const char*, GetComponentTypeName, (), (const, override));

    // Expose protected members for testing
    using CharacterMovementComponent::m_config;
    using CharacterMovementComponent::m_movementMode;
    using CharacterMovementComponent::m_characterRadius;
    using CharacterMovementComponent::m_characterHeight;
    using CharacterMovementComponent::m_physicsEngine;
    using CharacterMovementComponent::m_pendingInputVector;
    using CharacterMovementComponent::m_jumpRequested;
    using CharacterMovementComponent::m_deltaTime;
    using CharacterMovementComponent::ConstrainInputVector;
    using CharacterMovementComponent::ScaleInputAcceleration;
};

class CharacterMovementComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_character_movement_component.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_component = std::make_unique<MockCharacterMovementComponent>();
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        m_physicsEngine->Initialize();
        
        m_epsilon = 1e-6f;
    }
    
    void TearDown() override {
        if (m_physicsEngine) {
            m_physicsEngine->Shutdown();
        }
    }
    
    std::unique_ptr<MockCharacterMovementComponent> m_component;
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    float m_epsilon;
};

// Configuration Tests
TEST_F(CharacterMovementComponentTest, DefaultConfiguration_ValidValues) {
    CharacterMovementComponent::MovementConfig config;
    
    EXPECT_GT(config.maxWalkSpeed, 0.0f);
    EXPECT_GT(config.maxAcceleration, 0.0f);
    EXPECT_GT(config.brakingDeceleration, 0.0f);
    EXPECT_GT(config.jumpZVelocity, 0.0f);
    EXPECT_GT(config.gravityScale, 0.0f);
    EXPECT_GE(config.airControl, 0.0f);
    EXPECT_LE(config.airControl, 1.0f);
    EXPECT_GT(config.groundFriction, 0.0f);
    EXPECT_GT(config.maxStepHeight, 0.0f);
    EXPECT_GT(config.maxSlopeAngle, 0.0f);
    EXPECT_LE(config.maxSlopeAngle, 90.0f);
    EXPECT_TRUE(config.canJump);
    EXPECT_TRUE(config.canWalkOffLedges);
}

TEST_F(CharacterMovementComponentTest, SetMovementConfig_ValidConfig_Applied) {
    CharacterMovementComponent::MovementConfig config;
    config.maxWalkSpeed = 8.0f;
    config.maxAcceleration = 25.0f;
    config.jumpZVelocity = 12.0f;
    config.gravityScale = 1.5f;
    config.airControl = 0.3f;
    config.canJump = false;
    
    m_component->SetMovementConfig(config);
    
    const auto& appliedConfig = m_component->GetMovementConfig();
    EXPECT_NEAR(appliedConfig.maxWalkSpeed, 8.0f, m_epsilon);
    EXPECT_NEAR(appliedConfig.maxAcceleration, 25.0f, m_epsilon);
    EXPECT_NEAR(appliedConfig.jumpZVelocity, 12.0f, m_epsilon);
    EXPECT_NEAR(appliedConfig.gravityScale, 1.5f, m_epsilon);
    EXPECT_NEAR(appliedConfig.airControl, 0.3f, m_epsilon);
    EXPECT_FALSE(appliedConfig.canJump);
}

TEST_F(CharacterMovementComponentTest, GetMovementConfig_ReturnsReference) {
    const auto& config1 = m_component->GetMovementConfig();
    const auto& config2 = m_component->GetMovementConfig();
    
    // Should return the same reference
    EXPECT_EQ(&config1, &config2);
}

// Character Size Tests
TEST_F(CharacterMovementComponentTest, SetCharacterSize_ValidValues_Applied) {
    float radius = 0.5f;
    float height = 2.0f;
    
    m_component->SetCharacterSize(radius, height);
    
    EXPECT_NEAR(m_component->GetCharacterRadius(), radius, m_epsilon);
    EXPECT_NEAR(m_component->GetCharacterHeight(), height, m_epsilon);
}

TEST_F(CharacterMovementComponentTest, DefaultCharacterSize_ReasonableValues) {
    EXPECT_GT(m_component->GetCharacterRadius(), 0.0f);
    EXPECT_GT(m_component->GetCharacterHeight(), 0.0f);
    EXPECT_LT(m_component->GetCharacterRadius(), m_component->GetCharacterHeight());
}

// Physics Engine Integration Tests
TEST_F(CharacterMovementComponentTest, SetPhysicsEngine_ValidEngine_Stored) {
    m_component->SetPhysicsEngine(m_physicsEngine.get());
    
    EXPECT_EQ(m_component->GetPhysicsEngine(), m_physicsEngine.get());
}

TEST_F(CharacterMovementComponentTest, SetPhysicsEngine_NullEngine_Handled) {
    m_component->SetPhysicsEngine(nullptr);
    
    EXPECT_EQ(m_component->GetPhysicsEngine(), nullptr);
}

// Movement Mode Tests
TEST_F(CharacterMovementComponentTest, DefaultMovementMode_Walking) {
    EXPECT_EQ(m_component->GetMovementMode(), CharacterMovementComponent::MovementMode::Walking);
}

TEST_F(CharacterMovementComponentTest, MovementMode_AllEnumValues) {
    // Test that all enum values are valid
    auto walking = CharacterMovementComponent::MovementMode::Walking;
    auto falling = CharacterMovementComponent::MovementMode::Falling;
    auto flying = CharacterMovementComponent::MovementMode::Flying;
    auto swimming = CharacterMovementComponent::MovementMode::Swimming;
    auto custom = CharacterMovementComponent::MovementMode::Custom;
    
    EXPECT_NE(walking, falling);
    EXPECT_NE(walking, flying);
    EXPECT_NE(walking, swimming);
    EXPECT_NE(walking, custom);
}

// Utility Method Tests
TEST_F(CharacterMovementComponentTest, ConstrainInputVector_ZeroVector_ReturnsZero) {
    Math::Vec3 input(0.0f, 0.0f, 0.0f);
    Math::Vec3 result = m_component->ConstrainInputVector(input);
    
    EXPECT_NEAR(result.x, 0.0f, m_epsilon);
    EXPECT_NEAR(result.y, 0.0f, m_epsilon);
    EXPECT_NEAR(result.z, 0.0f, m_epsilon);
}

TEST_F(CharacterMovementComponentTest, ConstrainInputVector_UnitVector_ReturnsUnit) {
    Math::Vec3 input(1.0f, 0.0f, 0.0f);
    Math::Vec3 result = m_component->ConstrainInputVector(input);
    
    float length = glm::length(result);
    EXPECT_NEAR(length, 1.0f, m_epsilon);
}

TEST_F(CharacterMovementComponentTest, ConstrainInputVector_LargeVector_Normalized) {
    Math::Vec3 input(10.0f, 10.0f, 10.0f);
    Math::Vec3 result = m_component->ConstrainInputVector(input);
    
    float length = glm::length(result);
    EXPECT_LE(length, 1.0f + m_epsilon);
}

TEST_F(CharacterMovementComponentTest, ScaleInputAcceleration_ZeroInput_ReturnsZero) {
    Math::Vec3 input(0.0f, 0.0f, 0.0f);
    Math::Vec3 result = m_component->ScaleInputAcceleration(input);
    
    EXPECT_NEAR(result.x, 0.0f, m_epsilon);
    EXPECT_NEAR(result.y, 0.0f, m_epsilon);
    EXPECT_NEAR(result.z, 0.0f, m_epsilon);
}

TEST_F(CharacterMovementComponentTest, ScaleInputAcceleration_ValidInput_ScaledByConfig) {
    Math::Vec3 input(1.0f, 0.0f, 0.0f);
    Math::Vec3 result = m_component->ScaleInputAcceleration(input);
    
    float expectedMagnitude = m_component->GetMovementConfig().maxAcceleration;
    float actualMagnitude = glm::length(result);
    EXPECT_NEAR(actualMagnitude, expectedMagnitude, m_epsilon);
}

// Edge Case Tests
TEST_F(CharacterMovementComponentTest, SetCharacterSize_ZeroRadius_HandledGracefully) {
    EXPECT_NO_THROW(m_component->SetCharacterSize(0.0f, 2.0f));
    EXPECT_NEAR(m_component->GetCharacterRadius(), 0.0f, m_epsilon);
}

TEST_F(CharacterMovementComponentTest, SetCharacterSize_ZeroHeight_HandledGracefully) {
    EXPECT_NO_THROW(m_component->SetCharacterSize(0.5f, 0.0f));
    EXPECT_NEAR(m_component->GetCharacterHeight(), 0.0f, m_epsilon);
}

TEST_F(CharacterMovementComponentTest, SetCharacterSize_NegativeValues_HandledGracefully) {
    EXPECT_NO_THROW(m_component->SetCharacterSize(-0.5f, -2.0f));
    EXPECT_NEAR(m_component->GetCharacterRadius(), -0.5f, m_epsilon);
    EXPECT_NEAR(m_component->GetCharacterHeight(), -2.0f, m_epsilon);
}

// Configuration Edge Cases
TEST_F(CharacterMovementComponentTest, MovementConfig_ExtremeValues_HandledGracefully) {
    CharacterMovementComponent::MovementConfig config;
    config.maxWalkSpeed = 1000.0f;
    config.maxAcceleration = 0.001f;
    config.jumpZVelocity = 100.0f;
    config.gravityScale = 10.0f;
    config.airControl = 2.0f; // Above 1.0
    config.maxSlopeAngle = 180.0f; // Above 90
    
    EXPECT_NO_THROW(m_component->SetMovementConfig(config));
    
    const auto& appliedConfig = m_component->GetMovementConfig();
    EXPECT_NEAR(appliedConfig.maxWalkSpeed, 1000.0f, m_epsilon);
    EXPECT_NEAR(appliedConfig.maxAcceleration, 0.001f, m_epsilon);
    EXPECT_NEAR(appliedConfig.airControl, 2.0f, m_epsilon);
    EXPECT_NEAR(appliedConfig.maxSlopeAngle, 180.0f, m_epsilon);
}

// Parameterized Tests for Different Movement Configurations
class CharacterMovementConfigTest : public ::testing::TestWithParam<CharacterMovementComponent::MovementConfig> {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_character_movement_config.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
        
        m_component = std::make_unique<MockCharacterMovementComponent>();
    }
    
    std::unique_ptr<MockCharacterMovementComponent> m_component;
};

TEST_P(CharacterMovementConfigTest, SetConfiguration_ValidConfig_Applied) {
    CharacterMovementComponent::MovementConfig config = GetParam();
    
    EXPECT_NO_THROW(m_component->SetMovementConfig(config));
    
    const auto& appliedConfig = m_component->GetMovementConfig();
    EXPECT_NEAR(appliedConfig.maxWalkSpeed, config.maxWalkSpeed, 1e-6f);
    EXPECT_NEAR(appliedConfig.maxAcceleration, config.maxAcceleration, 1e-6f);
    EXPECT_NEAR(appliedConfig.brakingDeceleration, config.brakingDeceleration, 1e-6f);
    EXPECT_NEAR(appliedConfig.jumpZVelocity, config.jumpZVelocity, 1e-6f);
    EXPECT_NEAR(appliedConfig.gravityScale, config.gravityScale, 1e-6f);
    EXPECT_NEAR(appliedConfig.airControl, config.airControl, 1e-6f);
    EXPECT_EQ(appliedConfig.canJump, config.canJump);
    EXPECT_EQ(appliedConfig.canWalkOffLedges, config.canWalkOffLedges);
}

// Create test configurations
CharacterMovementComponent::MovementConfig CreateFastConfig() {
    CharacterMovementComponent::MovementConfig config;
    config.maxWalkSpeed = 12.0f;
    config.maxAcceleration = 30.0f;
    config.brakingDeceleration = 30.0f;
    config.jumpZVelocity = 15.0f;
    config.airControl = 0.5f;
    return config;
}

CharacterMovementComponent::MovementConfig CreateSlowConfig() {
    CharacterMovementComponent::MovementConfig config;
    config.maxWalkSpeed = 3.0f;
    config.maxAcceleration = 10.0f;
    config.brakingDeceleration = 10.0f;
    config.jumpZVelocity = 5.0f;
    config.airControl = 0.1f;
    return config;
}

CharacterMovementComponent::MovementConfig CreateNoJumpConfig() {
    CharacterMovementComponent::MovementConfig config;
    config.canJump = false;
    config.canWalkOffLedges = false;
    return config;
}

INSTANTIATE_TEST_SUITE_P(
    ConfigurationTests,
    CharacterMovementConfigTest,
    ::testing::Values(
        CharacterMovementComponent::MovementConfig{}, // Default
        CreateFastConfig(),
        CreateSlowConfig(),
        CreateNoJumpConfig()
    )
);

#endif // GAMEENGINE_HAS_BULLET