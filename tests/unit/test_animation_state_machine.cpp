#include "TestUtils.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationTransition.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test AnimationStateMachine basic functionality
 * Requirements: 2.1, 2.2 (state management and transitions)
 */
bool TestAnimationStateMachineBasics() {
    TestOutput::PrintTestStart("animation state machine basics");

    // Create state machine
    auto stateMachine = std::make_shared<AnimationStateMachine>();

    // Create states
    auto idleState = std::make_shared<AnimationState>("Idle", AnimationState::Type::Single);
    auto walkState = std::make_shared<AnimationState>("Walk", AnimationState::Type::Single);

    // Add states
    stateMachine->AddState(idleState);
    stateMachine->AddState(walkState);

    EXPECT_TRUE(stateMachine->HasState("Idle"));
    EXPECT_TRUE(stateMachine->HasState("Walk"));
    EXPECT_FALSE(stateMachine->HasState("Run"));

    // Check state retrieval
    auto retrievedIdle = stateMachine->GetState("Idle");
    EXPECT_TRUE(retrievedIdle != nullptr);
    EXPECT_EQUAL(retrievedIdle->GetName(), "Idle");

    // Check state names
    auto stateNames = stateMachine->GetStateNames();
    EXPECT_EQUAL(stateNames.size(), static_cast<size_t>(2));

    TestOutput::PrintTestPass("animation state machine basics");
    return true;
}

/**
 * Test AnimationState configuration
 * Requirements: 2.1, 2.4 (state types and callbacks)
 */
bool TestAnimationStateConfiguration() {
    TestOutput::PrintTestStart("animation state configuration");

    // Create animation state
    auto state = std::make_shared<AnimationState>("TestState", AnimationState::Type::Single);

    EXPECT_EQUAL(state->GetName(), "TestState");
    EXPECT_EQUAL(static_cast<int>(state->GetType()), static_cast<int>(AnimationState::Type::Single));
    EXPECT_EQUAL(state->GetSpeed(), 1.0f);
    EXPECT_TRUE(state->IsLooping());

    // Test property setters
    state->SetSpeed(2.0f);
    state->SetLooping(false);

    EXPECT_EQUAL(state->GetSpeed(), 2.0f);
    EXPECT_FALSE(state->IsLooping());

    // Test callback setting (just verify no crash)
    bool callbackCalled = false;
    state->SetOnEnterCallback([&callbackCalled](AnimationController*) {
        callbackCalled = true;
    });

    state->OnEnter(nullptr);
    EXPECT_TRUE(callbackCalled);

    TestOutput::PrintTestPass("animation state configuration");
    return true;
}

/**
 * Test AnimationTransition condition evaluation
 * Requirements: 2.2, 2.3 (transition conditions and evaluation)
 */
bool TestAnimationTransitionConditions() {
    TestOutput::PrintTestStart("animation transition conditions");

    // Create skeleton and controller for testing
    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
    skeleton->AddBone(rootBone);

    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    // Set up parameters
    controller.SetFloat("Speed", 5.0f);
    controller.SetBool("IsGrounded", true);
    controller.SetTrigger("Jump");

    // Test float condition
    auto floatCondition = TransitionCondition::FloatGreater("Speed", 3.0f);
    EXPECT_TRUE(floatCondition.Evaluate(&controller));

    auto floatCondition2 = TransitionCondition::FloatLess("Speed", 3.0f);
    EXPECT_FALSE(floatCondition2.Evaluate(&controller));

    // Test bool condition
    auto boolCondition = TransitionCondition::BoolTrue("IsGrounded");
    EXPECT_TRUE(boolCondition.Evaluate(&controller));

    // Test trigger condition
    auto triggerCondition = TransitionCondition::TriggerSet("Jump");
    EXPECT_TRUE(triggerCondition.Evaluate(&controller));

    TestOutput::PrintTestPass("animation transition conditions");
    return true;
}

/**
 * Test AnimationTransition creation and validation
 * Requirements: 2.2, 2.5 (transition creation and smooth blending)
 */
bool TestAnimationTransitionCreation() {
    TestOutput::PrintTestStart("animation transition creation");

    // Create transition
    auto transition = std::make_shared<AnimationTransition>("Idle", "Walk");

    EXPECT_EQUAL(transition->GetFromState(), "Idle");
    EXPECT_EQUAL(transition->GetToState(), "Walk");
    EXPECT_EQUAL(transition->GetDuration(), 0.3f); // Default duration

    // Test property setters
    transition->SetDuration(0.5f);
    transition->SetExitTime(0.8f);
    transition->SetHasExitTime(true);

    EXPECT_EQUAL(transition->GetDuration(), 0.5f);
    EXPECT_EQUAL(transition->GetExitTime(), 0.8f);
    EXPECT_TRUE(transition->HasExitTime());

    // Test condition addition
    transition->AddCondition(TransitionCondition::FloatGreater("Speed", 2.0f));
    EXPECT_EQUAL(transition->GetConditionCount(), static_cast<size_t>(1));

    // Test validation
    EXPECT_TRUE(transition->IsValid());

    TestOutput::PrintTestPass("animation transition creation");
    return true;
}

/**
 * Test TransitionBuilder fluent interface
 * Requirements: 2.2, 2.3 (transition configuration)
 */
bool TestTransitionBuilder() {
    TestOutput::PrintTestStart("transition builder");

    // Create transition using builder
    auto transition = TransitionBuilder("Idle", "Walk")
        .WithDuration(0.4f)
        .WithExitTime(0.9f)
        .WhenFloat("Speed", TransitionConditionType::FloatGreater, 1.5f)
        .WhenBool("IsGrounded", true)
        .WithAnd()
        .Build();

    EXPECT_TRUE(transition != nullptr);
    EXPECT_EQUAL(transition->GetFromState(), "Idle");
    EXPECT_EQUAL(transition->GetToState(), "Walk");
    EXPECT_EQUAL(transition->GetDuration(), 0.4f);
    EXPECT_EQUAL(transition->GetExitTime(), 0.9f);
    EXPECT_TRUE(transition->HasExitTime());
    EXPECT_EQUAL(transition->GetConditionCount(), static_cast<size_t>(2));
    EXPECT_TRUE(transition->IsValid());

    TestOutput::PrintTestPass("transition builder");
    return true;
}

/**
 * Test state machine execution flow
 * Requirements: 2.1, 2.4 (state execution and callbacks)
 */
bool TestStateMachineExecution() {
    TestOutput::PrintTestStart("state machine execution");

    // Create skeleton and controller
    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
    skeleton->AddBone(rootBone);

    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    // Create state machine
    auto stateMachine = std::make_shared<AnimationStateMachine>();

    // Create states
    auto idleState = std::make_shared<AnimationState>("Idle", AnimationState::Type::Single);
    auto walkState = std::make_shared<AnimationState>("Walk", AnimationState::Type::Single);

    stateMachine->AddState(idleState);
    stateMachine->AddState(walkState);
    stateMachine->SetEntryState("Idle");

    // Start state machine
    stateMachine->Start();
    EXPECT_TRUE(stateMachine->IsRunning());
    EXPECT_EQUAL(stateMachine->GetCurrentStateName(), "Idle");

    // Update state machine
    stateMachine->Update(0.016f, &controller); // 60 FPS
    EXPECT_EQUAL(stateMachine->GetCurrentStateTime(), 0.016f);

    // Stop state machine
    stateMachine->Stop();
    EXPECT_FALSE(stateMachine->IsRunning());

    TestOutput::PrintTestPass("state machine execution");
    return true;
}

/**
 * Test state machine validation
 * Requirements: 2.1 (state machine validation)
 */
bool TestStateMachineValidation() {
    TestOutput::PrintTestStart("state machine validation");

    auto stateMachine = std::make_shared<AnimationStateMachine>();

    // Empty state machine should be invalid
    EXPECT_FALSE(stateMachine->ValidateStateMachine());

    auto errors = stateMachine->GetValidationErrors();
    EXPECT_TRUE(errors.size() > 0);

    // Add a state and set entry state
    auto idleState = std::make_shared<AnimationState>("Idle", AnimationState::Type::Single);
    
    // Create a dummy animation for the state to make it valid
    auto animation = std::make_shared<GameEngine::Animation::SkeletalAnimation>("IdleAnimation");
    idleState->SetAnimation(animation);
    
    stateMachine->AddState(idleState);
    stateMachine->SetEntryState("Idle");

    // Should be valid now with animation content
    EXPECT_TRUE(stateMachine->ValidateStateMachine());

    TestOutput::PrintTestPass("state machine validation");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationStateMachine");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationStateMachine Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation State Machine Basics", TestAnimationStateMachineBasics);
        allPassed &= suite.RunTest("Animation State Configuration", TestAnimationStateConfiguration);
        allPassed &= suite.RunTest("Animation Transition Conditions", TestAnimationTransitionConditions);
        allPassed &= suite.RunTest("Animation Transition Creation", TestAnimationTransitionCreation);
        allPassed &= suite.RunTest("Transition Builder", TestTransitionBuilder);
        allPassed &= suite.RunTest("State Machine Execution", TestStateMachineExecution);
        allPassed &= suite.RunTest("State Machine Validation", TestStateMachineValidation);

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