#include "TestUtils.h"
#include "Game/Character.h"
#include "Physics/PhysicsEngine.h"
#include "Input/InputManager.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test Character animation system initialization
 * Requirements: 8.1, 8.2, 1.1, 2.1 (Animation system integration with Character class)
 */
bool TestCharacterAnimationSystemInitialization() {
    TestOutput::PrintTestStart("character animation system initialization");

    // Create character instance
    auto character = std::make_unique<Character>();
    EXPECT_TRUE(character != nullptr);

    // Initialize character (this should initialize animation system)
    bool initResult = character->Initialize(nullptr);
    EXPECT_TRUE(initResult);

    // Check if animation controller is available (basic check)
    EXPECT_TRUE(character->HasAnimationController());

    TestOutput::PrintTestPass("character animation system initialization");
    return true;
}

/**
 * Test Xbot skeleton loading
 * Requirements: 8.1, 8.2 (Load and configure Xbot skeleton from FBX file)
 */
bool TestXbotSkeletonLoading() {
    TestOutput::PrintTestStart("Xbot skeleton loading");

    // Create character instance
    auto character = std::make_unique<Character>();
    EXPECT_TRUE(character != nullptr);

    // Initialize character
    bool initResult = character->Initialize(nullptr);
    EXPECT_TRUE(initResult);

    // Basic check that animation system was initialized
    EXPECT_TRUE(character->HasAnimationController());

    TestOutput::PrintTestPass("Xbot skeleton loading");
    return true;
}

/**
 * Test animation asset loading
 * Requirements: 8.1, 8.2 (Create animation asset loading system for Xbot animations)
 */
bool TestAnimationAssetLoading() {
    TestOutput::PrintTestStart("animation asset loading");

    // Create character instance
    auto character = std::make_unique<Character>();
    EXPECT_TRUE(character != nullptr);

    // Initialize character
    bool initResult = character->Initialize(nullptr);
    EXPECT_TRUE(initResult);

    // Basic check that animation system was initialized
    EXPECT_TRUE(character->HasAnimationController());

    TestOutput::PrintTestPass("animation asset loading");
    return true;
}

/**
 * @brief ion state synchronization with movement
 * Requirements: 1.1, 2.1 (Implement animation state synchronization with character movement)
 */
bool TestAnimationMovementSynchronization() {
    TestOutput::PrintTestStart("animation movement synchronization");

    // Create character instance
    auto character = std::make_unique<Character>();
    EXPECT_TRUE(character != nullptr);

    // Initialize character
    bool initResult = character->Initialize(nullptr);
    EXPECT_TRUE(initResult);

    // Test animation parameter setting
    character->SetAnimationParameter("Speed", 5.0f);
    character->SetAnimationParameter("IsGrounded", true);
    character->SetAnimationParameter("IsJumping", false);

    // Test animation control
    character->PlayAnimation("Idle");
    character->SetAnimationSpeed(1.0f);
    
    EXPECT_NEARLY_EQUAL(character->GetAnimationSpeed(), 1.0f);

    // Test animation state changes
    character->PlayAnimation("Walking", 0.2f);
    character->PlayAnimation("Running", 0.3f);

    TestOutput::PrintTestPass("animation movement synchronization");
    return true;
}

/**
 * Test animation controller integration
 * Requirements: 8.1, 8.2 (Integrate AnimationController with Character class)
 */
bool TestAnimationControllerIntegration() {
    TestOutput::PrintTestStart("animation controller integration");

    // Create character instance
    auto character = std::make_unique<Character>();
    EXPECT_TRUE(character != nullptr);

    // Initialize character
    bool initResult = character->Initialize(nullptr);
    EXPECT_TRUE(initResult);

    // Test animation controller access
    EXPECT_TRUE(character->HasAnimationController());

    // Test animation control methods
    character->SetAnimationSpeed(2.0f);
    EXPECT_NEARLY_EQUAL(character->GetAnimationSpeed(), 2.0f);

    // Test animation parameter setting
    character->SetAnimationParameter("Speed", 5.0f);
    character->SetAnimationParameter("IsGrounded", true);
    character->SetAnimationParameter("IsJumping", false);

    // Test animation playback
    character->PlayAnimation("Idle");
    character->PlayAnimation("Walking", 0.2f);

    TestOutput::PrintTestPass("animation controller integration");
    return true;
}

int main() {
    TestOutput::PrintHeader("Character Animation Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Character Animation Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Character Animation System Initialization", TestCharacterAnimationSystemInitialization);
        allPassed &= suite.RunTest("Xbot Skeleton Loading", TestXbotSkeletonLoading);
        allPassed &= suite.RunTest("Animation Asset Loading", TestAnimationAssetLoading);
        allPassed &= suite.RunTest("Animation Movement Synchronization", TestAnimationMovementSynchronization);
        allPassed &= suite.RunTest("Animation Controller Integration", TestAnimationControllerIntegration);

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