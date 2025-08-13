#include "TestUtils.h"
#include "Animation/BlendTree.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/Bone.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test BlendTree creation and basic properties
 * Requirements: 3.2, 3.3 (blend tree types and parameter configuration)
 */
bool TestBlendTreeCreation() {
    TestOutput::PrintTestStart("blend tree creation");

    // Test 1D blend tree creation
    BlendTree blendTree1D(BlendTree::Type::Simple1D);
    EXPECT_EQUAL(static_cast<int>(blendTree1D.GetType()), static_cast<int>(BlendTree::Type::Simple1D));
    EXPECT_TRUE(blendTree1D.IsEmpty());
    EXPECT_EQUAL(blendTree1D.GetMotionCount(), static_cast<size_t>(0));

    // Test parameter setting for 1D
    blendTree1D.SetParameter("Speed");
    EXPECT_EQUAL(blendTree1D.GetParameterX(), "Speed");
    EXPECT_TRUE(blendTree1D.GetParameterY().empty());

    // Test 2D blend tree creation
    BlendTree blendTree2D(BlendTree::Type::FreeformCartesian2D);
    EXPECT_EQUAL(static_cast<int>(blendTree2D.GetType()), static_cast<int>(BlendTree::Type::FreeformCartesian2D));

    // Test parameter setting for 2D
    blendTree2D.SetParameters("VelocityX", "VelocityY");
    EXPECT_EQUAL(blendTree2D.GetParameterX(), "VelocityX");
    EXPECT_EQUAL(blendTree2D.GetParameterY(), "VelocityY");

    TestOutput::PrintTestPass("blend tree creation");
    return true;
}

/**
 * Test 1D blend tree motion management
 * Requirements: 3.2, 3.6 (1D blending and motion management)
 */
bool TestBlendTree1DMotions() {
    TestOutput::PrintTestStart("blend tree 1D motions");

    BlendTree blendTree(BlendTree::Type::Simple1D);
    blendTree.SetParameter("Speed");

    // Create test animations
    auto idleAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Idle");
    idleAnim->SetDuration(2.0f);
    
    auto walkAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Walk");
    walkAnim->SetDuration(1.5f);
    
    auto runAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Run");
    runAnim->SetDuration(1.0f);

    // Add motions with thresholds
    blendTree.AddMotion(idleAnim, 0.0f);
    blendTree.AddMotion(walkAnim, 2.0f);
    blendTree.AddMotion(runAnim, 6.0f);

    EXPECT_EQUAL(blendTree.GetMotionCount(), static_cast<size_t>(3));
    EXPECT_FALSE(blendTree.IsEmpty());

    // Test motion names
    auto motionNames = blendTree.GetMotionNames();
    EXPECT_EQUAL(motionNames.size(), static_cast<size_t>(3));

    // Test validation
    EXPECT_TRUE(blendTree.Validate());
    auto errors = blendTree.GetValidationErrors();
    EXPECT_TRUE(errors.empty());

    TestOutput::PrintTestPass("blend tree 1D motions");
    return true;
}

/**
 * Test 2D blend tree motion management
 * Requirements: 3.3, 3.6 (2D blending and motion management)
 */
bool TestBlendTree2DMotions() {
    TestOutput::PrintTestStart("blend tree 2D motions");

    BlendTree blendTree(BlendTree::Type::FreeformCartesian2D);
    blendTree.SetParameters("VelocityX", "VelocityY");

    // Create test animations
    auto idleAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Idle");
    auto walkForwardAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("WalkForward");
    auto walkBackwardAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("WalkBackward");
    auto strafeLeftAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("StrafeLeft");
    auto strafeRightAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("StrafeRight");

    // Add motions with 2D positions
    blendTree.AddMotion(idleAnim, Math::Vec2(0.0f, 0.0f));
    blendTree.AddMotion(walkForwardAnim, Math::Vec2(0.0f, 1.0f));
    blendTree.AddMotion(walkBackwardAnim, Math::Vec2(0.0f, -1.0f));
    blendTree.AddMotion(strafeLeftAnim, Math::Vec2(-1.0f, 0.0f));
    blendTree.AddMotion(strafeRightAnim, Math::Vec2(1.0f, 0.0f));

    EXPECT_EQUAL(blendTree.GetMotionCount(), static_cast<size_t>(5));
    EXPECT_FALSE(blendTree.IsEmpty());

    // Test validation
    EXPECT_TRUE(blendTree.Validate());

    TestOutput::PrintTestPass("blend tree 2D motions");
    return true;
}

/**
 * Test blend tree validation and error checking
 * Requirements: 3.7 (validation and error checking)
 */
bool TestBlendTreeValidation() {
    TestOutput::PrintTestStart("blend tree validation");

    // Test empty blend tree validation
    BlendTree emptyTree(BlendTree::Type::Simple1D);
    EXPECT_FALSE(emptyTree.Validate());
    auto errors = emptyTree.GetValidationErrors();
    EXPECT_FALSE(errors.empty());

    // Test blend tree without parameters
    BlendTree noParamTree(BlendTree::Type::Simple1D);
    auto testAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Test");
    noParamTree.AddMotion(testAnim, 0.0f);
    EXPECT_FALSE(noParamTree.Validate());

    // Test 2D blend tree with missing parameters
    BlendTree incomplete2D(BlendTree::Type::FreeformCartesian2D);
    incomplete2D.SetParameter("OnlyX"); // Missing Y parameter
    incomplete2D.AddMotion(testAnim, Math::Vec2(0.0f, 0.0f));
    EXPECT_FALSE(incomplete2D.Validate());

    // Test valid blend tree
    BlendTree validTree(BlendTree::Type::Simple1D);
    validTree.SetParameter("Speed");
    validTree.AddMotion(testAnim, 0.0f);
    EXPECT_TRUE(validTree.Validate());

    TestOutput::PrintTestPass("blend tree validation");
    return true;
}

/**
 * Test blend tree weight calculation algorithms
 * Requirements: 3.4, 3.5 (weight calculation and animation blending)
 */
bool TestBlendTreeWeightCalculation() {
    TestOutput::PrintTestStart("blend tree weight calculation");

    // Create a simple animation controller for testing
    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = std::make_shared<Bone>("Root", 0);
    skeleton->AddBone(rootBone);
    
    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    // Create 1D blend tree
    BlendTree blendTree1D(BlendTree::Type::Simple1D);
    blendTree1D.SetParameter("Speed");

    auto idleAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Idle");
    auto walkAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Walk");
    auto runAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Run");

    blendTree1D.AddMotion(idleAnim, 0.0f);
    blendTree1D.AddMotion(walkAnim, 2.0f);
    blendTree1D.AddMotion(runAnim, 6.0f);

    // Test weight calculation at different parameter values
    controller.SetFloat("Speed", 0.0f);
    auto samples1 = blendTree1D.GetAnimationSamples(&controller, 0.0f);
    EXPECT_FALSE(samples1.empty());

    controller.SetFloat("Speed", 1.0f);
    auto samples2 = blendTree1D.GetAnimationSamples(&controller, 0.0f);
    EXPECT_FALSE(samples2.empty());

    controller.SetFloat("Speed", 4.0f);
    auto samples3 = blendTree1D.GetAnimationSamples(&controller, 0.0f);
    EXPECT_FALSE(samples3.empty());

    // Verify that samples have valid weights
    for (const auto& sample : samples1) {
        EXPECT_TRUE(sample.IsValid());
        EXPECT_TRUE(sample.weight >= 0.0f && sample.weight <= 1.0f);
    }

    TestOutput::PrintTestPass("blend tree weight calculation");
    return true;
}

/**
 * Test blend tree duration calculation
 * Requirements: 3.4 (animation sampling and duration)
 */
bool TestBlendTreeDuration() {
    TestOutput::PrintTestStart("blend tree duration");

    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = std::make_shared<Bone>("Root", 0);
    skeleton->AddBone(rootBone);
    
    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    BlendTree blendTree(BlendTree::Type::Simple1D);
    blendTree.SetParameter("Speed");

    // Create animations with different durations
    auto shortAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Short");
    shortAnim->SetDuration(1.0f);
    
    auto mediumAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Medium");
    mediumAnim->SetDuration(2.5f);
    
    auto longAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Long");
    longAnim->SetDuration(4.0f);

    blendTree.AddMotion(shortAnim, 0.0f);
    blendTree.AddMotion(mediumAnim, 2.0f);
    blendTree.AddMotion(longAnim, 4.0f);

    // Duration should be the maximum of all animations
    float duration = blendTree.GetDuration(&controller);
    EXPECT_NEARLY_EQUAL(duration, 4.0f);

    TestOutput::PrintTestPass("blend tree duration");
    return true;
}

/**
 * Test child blend tree support
 * Requirements: 3.6 (nested blend trees)
 */
bool TestChildBlendTrees() {
    TestOutput::PrintTestStart("child blend trees");

    // Create parent blend tree
    auto parentTree = std::make_shared<BlendTree>(BlendTree::Type::Simple1D);
    parentTree->SetParameter("MainSpeed");

    // Create child blend tree
    auto childTree = std::make_shared<BlendTree>(BlendTree::Type::Simple1D);
    childTree->SetParameter("SubSpeed");

    auto testAnim = std::make_shared<GameEngine::Animation::SkeletalAnimation>("Test");
    childTree->AddMotion(testAnim, 0.0f);

    // Add child tree to parent
    parentTree->AddChildBlendTree(childTree, 2.0f);

    EXPECT_EQUAL(parentTree->GetMotionCount(), static_cast<size_t>(1));
    EXPECT_TRUE(parentTree->Validate());

    TestOutput::PrintTestPass("child blend trees");
    return true;
}

int main() {
    TestOutput::PrintHeader("BlendTree");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("BlendTree Tests");

        // Run all tests
        allPassed &= suite.RunTest("BlendTree Creation", TestBlendTreeCreation);
        allPassed &= suite.RunTest("BlendTree 1D Motions", TestBlendTree1DMotions);
        allPassed &= suite.RunTest("BlendTree 2D Motions", TestBlendTree2DMotions);
        allPassed &= suite.RunTest("BlendTree Validation", TestBlendTreeValidation);
        allPassed &= suite.RunTest("BlendTree Weight Calculation", TestBlendTreeWeightCalculation);
        allPassed &= suite.RunTest("BlendTree Duration", TestBlendTreeDuration);
        allPassed &= suite.RunTest("Child BlendTrees", TestChildBlendTrees);

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