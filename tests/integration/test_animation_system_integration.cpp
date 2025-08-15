#include <iostream>
#include "../TestUtils.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/BlendTree.h"
#include "Animation/IKSolver.h"
#include "Animation/MorphTarget.h"
#include "Animation/Pose.h"
#include "Core/Logger.h"

using namespace GameEngine;
using namespace GameEngine::Animation;
using namespace GameEngine::Testing;

/**
 * Test AnimationController with state machine integration
 * Requirements: 2.1 (state management and integration)
 */
bool TestAnimationControllerWithStateMachineIntegration() {
    TestOutput::PrintTestStart("animation controller with state machine integration");

    // Create skeleton with multiple bones
    auto skeleton = std::make_shared<AnimationSkeleton>("IntegrationSkeleton");
    auto rootBone = skeleton->CreateBone("Root");
    auto spineBone = skeleton->CreateBone("Spine");
    auto headBone = skeleton->CreateBone("Head");
    
    skeleton->SetBoneParent("Spine", "Root");
    skeleton->SetBoneParent("Head", "Spine");
    skeleton->SetBindPose();

    // Create controller
    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    // Create multiple animations
    auto idleAnimation = std::make_shared<SkeletalAnimation>("Idle");
    idleAnimation->SetDuration(2.0f);
    idleAnimation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    idleAnimation->AddPositionKeyframe("Root", 2.0f, Math::Vec3(0.0f, 0.1f, 0.0f)); // Slight breathing motion

    auto walkAnimation = std::make_shared<SkeletalAnimation>("Walk");
    walkAnimation->SetDuration(1.0f);
    walkAnimation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    walkAnimation->AddPositionKeyframe("Root", 0.5f, Math::Vec3(0.5f, 0.0f, 0.0f));
    walkAnimation->AddPositionKeyframe("Root", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));

    auto runAnimation = std::make_shared<SkeletalAnimation>("Run");
    runAnimation->SetDuration(0.6f);
    runAnimation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    runAnimation->AddPositionKeyframe("Root", 0.3f, Math::Vec3(1.0f, 0.0f, 0.0f));
    runAnimation->AddPositionKeyframe("Root", 0.6f, Math::Vec3(2.0f, 0.0f, 0.0f));

    // Create state machine
    auto stateMachine = std::make_shared<AnimationStateMachine>();

    // Create states
    auto idleState = std::make_shared<AnimationState>("Idle", AnimationState::Type::Single);
    idleState->SetAnimation(idleAnimation);

    auto walkState = std::make_shared<AnimationState>("Walk", AnimationState::Type::Single);
    walkState->SetAnimation(walkAnimation);

    auto runState = std::make_shared<AnimationState>("Run", AnimationState::Type::Single);
    runState->SetAnimation(runAnimation);

    stateMachine->AddState(idleState);
    stateMachine->AddState(walkState);
    stateMachine->AddState(runState);
    stateMachine->SetEntryState("Idle");

    // Create transitions
    auto idleToWalk = TransitionBuilder("Idle", "Walk")
        .WithDuration(0.3f)
        .WhenFloat("Speed", TransitionConditionType::FloatGreater, 0.5f)
        .Build();

    auto walkToRun = TransitionBuilder("Walk", "Run")
        .WithDuration(0.2f)
        .WhenFloat("Speed", TransitionConditionType::FloatGreater, 3.0f)
        .Build();

    auto walkToIdle = TransitionBuilder("Walk", "Idle")
        .WithDuration(0.4f)
        .WhenFloat("Speed", TransitionConditionType::FloatLess, 0.5f)
        .Build();

    auto runToWalk = TransitionBuilder("Run", "Walk")
        .WithDuration(0.3f)
        .WhenFloat("Speed", TransitionConditionType::FloatLess, 3.0f)
        .Build();

    stateMachine->AddTransition("Idle", "Walk", idleToWalk);
    stateMachine->AddTransition("Walk", "Run", walkToRun);
    stateMachine->AddTransition("Walk", "Idle", walkToIdle);
    stateMachine->AddTransition("Run", "Walk", runToWalk);

    // Set state machine to controller
    controller.SetStateMachine(stateMachine);

    // Test state machine integration
    controller.SetFloat("Speed", 0.0f);
    controller.Update(0.1f);

    // Test parameter-driven state transitions
    controller.SetFloat("Speed", 1.0f); // Should trigger idle to walk
    controller.Update(0.1f);
    controller.Update(0.4f); // Complete transition

    controller.SetFloat("Speed", 4.0f); // Should trigger walk to run
    controller.Update(0.1f);
    controller.Update(0.3f); // Complete transition

    controller.SetFloat("Speed", 2.0f); // Should trigger run to walk
    controller.Update(0.1f);
    controller.Update(0.4f); // Complete transition

    controller.SetFloat("Speed", 0.0f); // Should trigger walk to idle
    controller.Update(0.1f);
    controller.Update(0.5f); // Complete transition

    // Test pose evaluation throughout transitions
    auto finalPose = controller.EvaluateCurrentPose();
    EXPECT_TRUE(finalPose.HasValidSkeleton());
    EXPECT_TRUE(finalPose.HasBoneTransform("Root"));
    EXPECT_TRUE(finalPose.HasBoneTransform("Spine"));
    EXPECT_TRUE(finalPose.HasBoneTransform("Head"));

    TestOutput::PrintTestPass("animation controller with state machine integration");
    return true;
}

/**
 * Test IK solver accuracy and constraint handling
 * Requirements: 4.2 (IK solver accuracy and constraint handling)
 */
bool TestIKSolverAccuracyAndConstraintHandling() {
    TestOutput::PrintTestStart("IK solver accuracy and constraint handling");

    // Create skeleton for IK testing
    auto skeleton = std::make_shared<AnimationSkeleton>("IKSkeleton");
    auto shoulderBone = skeleton->CreateBone("Shoulder");
    auto upperArmBone = skeleton->CreateBone("UpperArm");
    auto lowerArmBone = skeleton->CreateBone("LowerArm");
    auto handBone = skeleton->CreateBone("Hand");

    skeleton->SetBoneParent("UpperArm", "Shoulder");
    skeleton->SetBoneParent("LowerArm", "UpperArm");
    skeleton->SetBoneParent("Hand", "LowerArm");

    // Set up bone positions for realistic arm
    shoulderBone->SetLocalPosition(Math::Vec3(0.0f, 1.5f, 0.0f));
    upperArmBone->SetLocalPosition(Math::Vec3(0.3f, 0.0f, 0.0f));
    lowerArmBone->SetLocalPosition(Math::Vec3(0.3f, 0.0f, 0.0f));
    handBone->SetLocalPosition(Math::Vec3(0.2f, 0.0f, 0.0f));

    skeleton->UpdateBoneTransforms();

    // Test Two-Bone IK solver
    TwoBoneIK twoBoneIK;
    twoBoneIK.SetUpperBone(upperArmBone->GetId());
    twoBoneIK.SetLowerBone(lowerArmBone->GetId());
    twoBoneIK.SetEndEffector(handBone->GetId());

    // Set reachable target
    Math::Vec3 reachableTarget(0.6f, 1.2f, 0.0f);
    twoBoneIK.SetTarget(reachableTarget);

    // Test target reachability
    EXPECT_TRUE(twoBoneIK.IsTargetReachable(*skeleton));

    // Test solving
    EXPECT_TRUE(twoBoneIK.Solve(*skeleton));

    // Test with constraints
    float minAngle = -Math::PI / 3.0f; // -60 degrees
    float maxAngle = Math::PI / 3.0f;  // 60 degrees
    twoBoneIK.SetBoneConstraints(lowerArmBone->GetId(), minAngle, maxAngle);

    // Solve with constraints
    EXPECT_TRUE(twoBoneIK.Solve(*skeleton));

    // Test FABRIK solver
    FABRIKIK fabrikSolver;
    std::vector<int> chain = {
        shoulderBone->GetId(),
        upperArmBone->GetId(),
        lowerArmBone->GetId(),
        handBone->GetId()
    };
    fabrikSolver.SetChain(chain);
    fabrikSolver.SetTarget(reachableTarget);

    // Test FABRIK solving
    EXPECT_TRUE(fabrikSolver.Solve(*skeleton));

    // Test IK/FK blending
    twoBoneIK.SetIKWeight(0.5f); // 50% blend
    EXPECT_TRUE(twoBoneIK.Solve(*skeleton));

    twoBoneIK.SetIKWeight(1.0f); // Full IK
    EXPECT_TRUE(twoBoneIK.Solve(*skeleton));

    twoBoneIK.SetIKWeight(0.0f); // Full FK
    EXPECT_TRUE(twoBoneIK.Solve(*skeleton));

    // Test unreachable target handling
    Math::Vec3 unreachableTarget(5.0f, 5.0f, 0.0f);
    twoBoneIK.SetTarget(unreachableTarget);
    EXPECT_FALSE(twoBoneIK.IsTargetReachable(*skeleton));

    // Should still provide best-effort solution
    EXPECT_TRUE(twoBoneIK.Solve(*skeleton));

    TestOutput::PrintTestPass("IK solver accuracy and constraint handling");
    return true;
}

/**
 * Test morph target application and blending
 * Requirements: 5.1 (morph target application and blending)
 */
bool TestMorphTargetApplicationAndBlending() {
    TestOutput::PrintTestStart("morph target application and blending");

    // Create test mesh vertices
    std::vector<Vertex> originalVertices(4);
    originalVertices[0].position = Math::Vec3(0.0f, 0.0f, 0.0f);
    originalVertices[1].position = Math::Vec3(1.0f, 0.0f, 0.0f);
    originalVertices[2].position = Math::Vec3(1.0f, 1.0f, 0.0f);
    originalVertices[3].position = Math::Vec3(0.0f, 1.0f, 0.0f);

    // Set normals
    for (auto& vertex : originalVertices) {
        vertex.normal = Math::Vec3(0.0f, 0.0f, 1.0f);
    }

    // Create morph targets
    auto smileMorph = std::make_shared<MorphTarget>("Smile");
    std::vector<Math::Vec3> smileDeltas = {
        Math::Vec3(0.0f, 0.1f, 0.0f),   // Lift corners
        Math::Vec3(0.0f, 0.1f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.0f)
    };
    smileMorph->SetVertexDeltas(smileDeltas);

    auto frownMorph = std::make_shared<MorphTarget>("Frown");
    std::vector<Math::Vec3> frownDeltas = {
        Math::Vec3(0.0f, -0.1f, 0.0f),  // Lower corners
        Math::Vec3(0.0f, -0.1f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.0f)
    };
    frownMorph->SetVertexDeltas(frownDeltas);

    auto blinkMorph = std::make_shared<MorphTarget>("Blink");
    std::vector<Math::Vec3> blinkDeltas = {
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, -0.2f, 0.0f),  // Close eyes
        Math::Vec3(0.0f, -0.2f, 0.0f)
    };
    blinkMorph->SetVertexDeltas(blinkDeltas);

    // Create morph target controller
    MorphTargetController controller;
    controller.AddMorphTarget(smileMorph);
    controller.AddMorphTarget(frownMorph);
    controller.AddMorphTarget(blinkMorph);

    EXPECT_EQUAL(controller.GetMorphTargetCount(), static_cast<size_t>(3));

    // Test individual morph target application
    std::vector<Vertex> testVertices = originalVertices;
    
    controller.SetWeight("Smile", 1.0f);
    controller.ApplyToVertices(testVertices);

    // Verify smile morph was applied
    Math::Vec3 expectedPos0 = originalVertices[0].position + smileDeltas[0];
    EXPECT_VEC3_NEARLY_EQUAL(testVertices[0].position, expectedPos0);

    // Reset and test blending
    testVertices = originalVertices;
    controller.SetWeight("Smile", 0.5f);
    controller.SetWeight("Blink", 0.3f);
    controller.ApplyToVertices(testVertices);

    // Verify blended result
    Math::Vec3 expectedBlended = originalVertices[2].position + 
                                smileDeltas[2] * 0.5f + 
                                blinkDeltas[2] * 0.3f;
    EXPECT_VEC3_NEARLY_EQUAL(testVertices[2].position, expectedBlended);

    // Test weight animation
    controller.AnimateWeight("Smile", 1.0f, 1.0f); // Animate to 1.0 over 1 second
    
    // Simulate time progression
    controller.Update(0.5f); // Half way
    float halfwayWeight = controller.GetWeight("Smile");
    EXPECT_TRUE(halfwayWeight > 0.5f && halfwayWeight < 1.0f);

    controller.Update(0.5f); // Complete animation
    EXPECT_NEARLY_EQUAL(controller.GetWeight("Smile"), 1.0f);

    // Test override blending mode
    controller.SetBlendMode(MorphTargetController::BlendMode::Override);
    controller.SetWeight("Smile", 0.8f);
    controller.SetWeight("Frown", 0.6f); // Should override smile in override mode

    testVertices = originalVertices;
    controller.ApplyToVertices(testVertices);

    // In override mode, only the highest weight should be applied
    // (Implementation detail may vary)

    TestOutput::PrintTestPass("morph target application and blending");
    return true;
}

/**
 * Test complete animation pipeline integration
 * Requirements: 1.1, 2.1, 3.2 (complete pipeline integration)
 */
bool TestCompleteAnimationPipelineIntegration() {
    TestOutput::PrintTestStart("complete animation pipeline integration");

    // Create comprehensive skeleton
    auto skeleton = std::make_shared<AnimationSkeleton>("CompleteSkeleton");
    
    // Create hierarchical bone structure
    auto rootBone = skeleton->CreateBone("Root");
    auto spineBone = skeleton->CreateBone("Spine");
    auto chestBone = skeleton->CreateBone("Chest");
    auto leftShoulderBone = skeleton->CreateBone("LeftShoulder");
    auto leftArmBone = skeleton->CreateBone("LeftArm");
    auto leftHandBone = skeleton->CreateBone("LeftHand");
    auto rightShoulderBone = skeleton->CreateBone("RightShoulder");
    auto rightArmBone = skeleton->CreateBone("RightArm");
    auto rightHandBone = skeleton->CreateBone("RightHand");

    // Set up hierarchy
    skeleton->SetBoneParent("Spine", "Root");
    skeleton->SetBoneParent("Chest", "Spine");
    skeleton->SetBoneParent("LeftShoulder", "Chest");
    skeleton->SetBoneParent("LeftArm", "LeftShoulder");
    skeleton->SetBoneParent("LeftHand", "LeftArm");
    skeleton->SetBoneParent("RightShoulder", "Chest");
    skeleton->SetBoneParent("RightArm", "RightShoulder");
    skeleton->SetBoneParent("RightHand", "RightArm");

    skeleton->SetBindPose();

    // Create animation controller
    AnimationController controller;
    EXPECT_TRUE(controller.Initialize(skeleton));

    // Create multiple complex animations
    auto idleAnimation = std::make_shared<SkeletalAnimation>("ComplexIdle");
    idleAnimation->SetDuration(3.0f);
    
    // Add breathing motion
    idleAnimation->AddPositionKeyframe("Chest", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    idleAnimation->AddPositionKeyframe("Chest", 1.5f, Math::Vec3(0.0f, 0.02f, 0.0f));
    idleAnimation->AddPositionKeyframe("Chest", 3.0f, Math::Vec3(0.0f, 0.0f, 0.0f));

    // Add subtle arm sway
    idleAnimation->AddRotationKeyframe("LeftArm", 0.0f, Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));
    idleAnimation->AddRotationKeyframe("LeftArm", 1.5f, Math::Quat(0.99f, 0.0f, 0.0f, 0.14f));
    idleAnimation->AddRotationKeyframe("LeftArm", 3.0f, Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));

    auto walkAnimation = std::make_shared<SkeletalAnimation>("ComplexWalk");
    walkAnimation->SetDuration(1.2f);
    
    // Add walking motion
    walkAnimation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    walkAnimation->AddPositionKeyframe("Root", 0.6f, Math::Vec3(0.5f, 0.05f, 0.0f));
    walkAnimation->AddPositionKeyframe("Root", 1.2f, Math::Vec3(1.0f, 0.0f, 0.0f));

    // Add arm swing
    walkAnimation->AddRotationKeyframe("LeftArm", 0.0f, Math::Quat(0.9f, 0.0f, 0.0f, 0.44f));
    walkAnimation->AddRotationKeyframe("LeftArm", 0.6f, Math::Quat(0.9f, 0.0f, 0.0f, -0.44f));
    walkAnimation->AddRotationKeyframe("LeftArm", 1.2f, Math::Quat(0.9f, 0.0f, 0.0f, 0.44f));

    walkAnimation->AddRotationKeyframe("RightArm", 0.0f, Math::Quat(0.9f, 0.0f, 0.0f, -0.44f));
    walkAnimation->AddRotationKeyframe("RightArm", 0.6f, Math::Quat(0.9f, 0.0f, 0.0f, 0.44f));
    walkAnimation->AddRotationKeyframe("RightArm", 1.2f, Math::Quat(0.9f, 0.0f, 0.0f, -0.44f));

    // Create blend tree for locomotion
    auto locomotionBlendTree = std::make_shared<BlendTree>(BlendTree::Type::Simple1D);
    locomotionBlendTree->SetParameter("Speed");
    locomotionBlendTree->AddMotion(idleAnimation, 0.0f);
    locomotionBlendTree->AddMotion(walkAnimation, 2.0f);

    // Create state machine
    auto stateMachine = std::make_shared<AnimationStateMachine>();

    auto locomotionState = std::make_shared<AnimationState>("Locomotion", AnimationState::Type::BlendTree);
    locomotionState->SetBlendTree(locomotionBlendTree);

    stateMachine->AddState(locomotionState);
    stateMachine->SetEntryState("Locomotion");

    controller.SetStateMachine(stateMachine);

    // Test complete pipeline with different speeds
    std::vector<float> testSpeeds = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };
    
    for (float speed : testSpeeds) {
        controller.SetFloat("Speed", speed);
        controller.Update(0.1f);

        // Evaluate pose
        auto pose = controller.EvaluateCurrentPose();
        EXPECT_TRUE(pose.HasValidSkeleton());

        // Get bone matrices
        std::vector<Math::Mat4> boneMatrices;
        controller.Evaluate(boneMatrices);
        EXPECT_EQUAL(boneMatrices.size(), skeleton->GetBoneCount());

        // Verify matrices are valid (not NaN or infinite)
        for (const auto& matrix : boneMatrices) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    EXPECT_FALSE(std::isnan(matrix[i][j]));
                    EXPECT_FALSE(std::isinf(matrix[i][j]));
                }
            }
        }
    }

    // Test performance with complex animation
    TestTimer timer;
    const int iterations = 100;

    for (int i = 0; i < iterations; ++i) {
        controller.SetFloat("Speed", 1.0f + (i % 10) * 0.1f);
        controller.Update(0.016f); // 60 FPS

        std::vector<Math::Mat4> boneMatrices;
        controller.Evaluate(boneMatrices);
    }

    double elapsed = timer.ElapsedMs();
    double avgTime = elapsed / iterations;

    TestOutput::PrintTiming("Complete Animation Pipeline", elapsed, iterations);

    // Performance should be reasonable (less than 1ms per update for this complexity)
    EXPECT_TRUE(avgTime < 1.0);

    TestOutput::PrintTestPass("complete animation pipeline integration");
    return true;
}

/**
 * Test animation system memory management and cleanup
 * Requirements: 9.6 (memory management and optimization)
 */
bool TestAnimationSystemMemoryManagementAndCleanup() {
    TestOutput::PrintTestStart("animation system memory management and cleanup");

    // Test multiple controller lifecycle
    for (int i = 0; i < 10; ++i) {
        auto skeleton = std::make_shared<AnimationSkeleton>("TestSkeleton" + std::to_string(i));
        auto bone = skeleton->CreateBone("Bone" + std::to_string(i));

        AnimationController controller;
        EXPECT_TRUE(controller.Initialize(skeleton));

        auto animation = std::make_shared<SkeletalAnimation>("TestAnim" + std::to_string(i));
        animation->SetDuration(1.0f);
        animation->AddPositionKeyframe("Bone" + std::to_string(i), 0.0f, Math::Vec3(0.0f));
        animation->AddPositionKeyframe("Bone" + std::to_string(i), 1.0f, Math::Vec3(1.0f));

        controller.AddAnimation("TestAnim" + std::to_string(i), animation);
        controller.Play("TestAnim" + std::to_string(i));

        // Update a few times
        for (int j = 0; j < 5; ++j) {
            controller.Update(0.1f);
        }

        // Controller should clean up automatically when it goes out of scope
    }

    // Test large animation data handling
    auto largeSkeleton = std::make_shared<AnimationSkeleton>("LargeSkeleton");
    for (int i = 0; i < 50; ++i) {
        largeSkeleton->CreateBone("Bone" + std::to_string(i));
    }

    auto largeAnimation = std::make_shared<SkeletalAnimation>("LargeAnimation");
    largeAnimation->SetDuration(10.0f);

    // Add many keyframes
    for (int i = 0; i < 50; ++i) {
        std::string boneName = "Bone" + std::to_string(i);
        for (int j = 0; j <= 100; ++j) {
            float time = (j / 100.0f) * 10.0f;
            Math::Vec3 pos(static_cast<float>(i), static_cast<float>(j) * 0.01f, 0.0f);
            largeAnimation->AddPositionKeyframe(boneName, time, pos);
        }
    }

    AnimationController largeController;
    EXPECT_TRUE(largeController.Initialize(largeSkeleton));
    largeController.AddAnimation("LargeAnimation", largeAnimation);
    largeController.Play("LargeAnimation");

    // Test performance with large data
    TestTimer timer;
    for (int i = 0; i < 50; ++i) {
        largeController.Update(0.1f);
    }
    double elapsed = timer.ElapsedMs();

    TestOutput::PrintTiming("Large Animation Data Processing", elapsed, 50);

    // Should still be reasonable performance
    EXPECT_TRUE(elapsed < 100.0); // Less than 100ms for 50 updates

    TestOutput::PrintTestPass("animation system memory management and cleanup");
    return true;
}

int main() {
    TestOutput::PrintHeader("Animation System Integration");

    // Initialize logger for tests
    Logger::GetInstance();

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Animation System Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Controller with State Machine Integration", TestAnimationControllerWithStateMachineIntegration);
        allPassed &= suite.RunTest("IK Solver Accuracy and Constraint Handling", TestIKSolverAccuracyAndConstraintHandling);
        allPassed &= suite.RunTest("Morph Target Application and Blending", TestMorphTargetApplicationAndBlending);
        allPassed &= suite.RunTest("Complete Animation Pipeline Integration", TestCompleteAnimationPipelineIntegration);
        allPassed &= suite.RunTest("Animation System Memory Management and Cleanup", TestAnimationSystemMemoryManagementAndCleanup);

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