#include "TestUtils.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/Pose.h"
#include "Animation/AnimationSkeleton.h"
#include "Core/Logger.h"

using namespace GameEngine;
using namespace GameEngine::Animation;
using namespace GameEngine::Testing;

/**
 * Test animation creation and keyframe management
 * Requirements: 1.2, 1.3, 7.1
 */
bool TestAnimationCreation() {
    TestOutput::PrintTestStart("animation creation and keyframe management");

    Animation animation("TestAnimation");
    EXPECT_EQUAL(animation.GetName(), "TestAnimation");
    EXPECT_EQUAL(animation.GetDuration(), 0.0f);
    EXPECT_TRUE(animation.IsEmpty());

    // Add keyframes
    Math::Vec3 pos1(1.0f, 0.0f, 0.0f);
    Math::Vec3 pos2(2.0f, 0.0f, 0.0f);
    Math::Quat rot1(1.0f, 0.0f, 0.0f, 0.0f);
    Math::Quat rot2(0.707f, 0.0f, 0.707f, 0.0f);

    animation.AddPositionKeyframe("TestBone", 0.0f, pos1);
    animation.AddPositionKeyframe("TestBone", 1.0f, pos2);
    animation.AddRotationKeyframe("TestBone", 0.0f, rot1);
    animation.AddRotationKeyframe("TestBone", 1.0f, rot2);

    EXPECT_FALSE(animation.IsEmpty());
    EXPECT_EQUAL(animation.GetDuration(), 1.0f);
    EXPECT_TRUE(animation.HasBone("TestBone"));
    EXPECT_EQUAL(animation.GetBoneCount(), static_cast<size_t>(1));

    TestOutput::PrintTestPass("animation creation and keyframe management");
    return true;
}

/**
 * Test keyframe interpolation and sampling
 * Requirements: 1.3, 1.6, 1.7
 */
bool TestKeyframeInterpolation() {
    TestOutput::PrintTestStart("keyframe interpolation and sampling");

    Animation animation("InterpolationTest");

    // Create simple position animation
    Math::Vec3 startPos(0.0f, 0.0f, 0.0f);
    Math::Vec3 endPos(10.0f, 0.0f, 0.0f);

    animation.AddPositionKeyframe("Bone", 0.0f, startPos);
    animation.AddPositionKeyframe("Bone", 2.0f, endPos);

    // Test sampling at different times
    auto pose0 = animation.SampleBone("Bone", 0.0f);
    auto pose1 = animation.SampleBone("Bone", 1.0f); // Middle
    auto pose2 = animation.SampleBone("Bone", 2.0f);

    EXPECT_TRUE(pose0.hasPosition);
    EXPECT_TRUE(pose1.hasPosition);
    EXPECT_TRUE(pose2.hasPosition);

    EXPECT_VEC3_NEARLY_EQUAL(pose0.position, startPos);
    EXPECT_VEC3_NEARLY_EQUAL(pose2.position, endPos);
    
    // Middle should be interpolated
    Math::Vec3 expectedMiddle(5.0f, 0.0f, 0.0f);
    EXPECT_VEC3_NEARLY_EQUAL(pose1.position, expectedMiddle);

    TestOutput::PrintTestPass("keyframe interpolation and sampling");
    return true;
}

/**
 * Test animation loop modes
 * Requirements: 1.6, 1.7
 */
bool TestAnimationLoopModes() {
    TestOutput::PrintTestStart("animation loop modes");

    Animation animation("LoopTest");
    animation.SetDuration(2.0f);

    // Test different loop modes
    animation.SetLoopMode(LoopMode::Once);
    EXPECT_NEARLY_EQUAL(animation.WrapTime(-1.0f), 0.0f);
    EXPECT_NEARLY_EQUAL(animation.WrapTime(3.0f), 2.0f);

    animation.SetLoopMode(LoopMode::Loop);
    EXPECT_NEARLY_EQUAL(animation.WrapTime(3.0f), 1.0f);
    EXPECT_NEARLY_EQUAL(animation.WrapTime(4.0f), 0.0f);

    animation.SetLoopMode(LoopMode::PingPong);
    EXPECT_NEARLY_EQUAL(animation.WrapTime(1.0f), 1.0f);
    EXPECT_NEARLY_EQUAL(animation.WrapTime(3.0f), 1.0f);

    TestOutput::PrintTestPass("animation loop modes");
    return true;
}

/**
 * Test pose creation and bone transforms
 * Requirements: 1.4, 1.5, 9.2
 */
bool TestPoseCreation() {
    TestOutput::PrintTestStart("pose creation and bone transforms");

    // Create skeleton
    auto skeleton = std::make_shared<Skeleton>("TestSkeleton");
    auto rootBone = skeleton->CreateBone("Root");
    auto childBone = skeleton->CreateBone("Child");
    skeleton->SetBoneParent("Child", "Root");

    // Create pose
    Pose pose(skeleton);
    EXPECT_TRUE(pose.HasValidSkeleton());
    EXPECT_EQUAL(pose.GetBoneCount(), static_cast<size_t>(2));

    // Set bone transforms
    BoneTransform rootTransform;
    rootTransform.position = Math::Vec3(1.0f, 0.0f, 0.0f);
    rootTransform.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);
    rootTransform.scale = Math::Vec3(1.0f);

    pose.SetBoneTransform("Root", rootTransform);
    EXPECT_TRUE(pose.HasBoneTransform("Root"));

    BoneTransform retrievedTransform = pose.GetBoneTransform("Root");
    EXPECT_VEC3_NEARLY_EQUAL(retrievedTransform.position, rootTransform.position);

    TestOutput::PrintTestPass("pose creation and bone transforms");
    return true;
}

/**
 * Test pose blending
 * Requirements: 3.1, 3.4, 3.5
 */
bool TestPoseBlending() {
    TestOutput::PrintTestStart("pose blending");

    // Create skeleton
    auto skeleton = std::make_shared<Skeleton>("BlendSkeleton");
    auto bone = skeleton->CreateBone("TestBone");

    // Create two poses
    Pose poseA(skeleton);
    Pose poseB(skeleton);

    BoneTransform transformA;
    transformA.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    transformA.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f);

    BoneTransform transformB;
    transformB.position = Math::Vec3(10.0f, 0.0f, 0.0f);
    transformB.rotation = Math::Quat(0.707f, 0.0f, 0.707f, 0.0f);

    poseA.SetBoneTransform("TestBone", transformA);
    poseB.SetBoneTransform("TestBone", transformB);

    // Blend poses
    Pose blendedPose = Pose::Blend(poseA, poseB, 0.5f);
    BoneTransform blendedTransform = blendedPose.GetBoneTransform("TestBone");

    // Check blended position (should be halfway)
    Math::Vec3 expectedPos(5.0f, 0.0f, 0.0f);
    EXPECT_VEC3_NEARLY_EQUAL(blendedTransform.position, expectedPos);

    TestOutput::PrintTestPass("pose blending");
    return true;
}

/**
 * Test pose evaluation from animation
 * Requirements: 1.3, 1.6, 3.4
 */
bool TestPoseEvaluation() {
    TestOutput::PrintTestStart("pose evaluation from animation");

    // Create skeleton
    auto skeleton = std::make_shared<Skeleton>("EvalSkeleton");
    auto bone = skeleton->CreateBone("TestBone");

    // Create animation
    Animation animation("EvalAnimation");
    animation.AddPositionKeyframe("TestBone", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("TestBone", 1.0f, Math::Vec3(5.0f, 0.0f, 0.0f));

    // Evaluate pose at specific time
    Pose evaluatedPose = PoseEvaluator::EvaluateAnimation(animation, 0.5f, skeleton);
    BoneTransform transform = evaluatedPose.GetBoneTransform("TestBone");

    // Should be interpolated to middle position
    Math::Vec3 expectedPos(2.5f, 0.0f, 0.0f);
    EXPECT_VEC3_NEARLY_EQUAL(transform.position, expectedPos);

    TestOutput::PrintTestPass("pose evaluation from animation");
    return true;
}

/**
 * Test animation serialization
 * Requirements: 7.1
 */
bool TestAnimationSerialization() {
    TestOutput::PrintTestStart("animation serialization");

    // Create original animation
    Animation originalAnimation("SerializationTest");
    originalAnimation.SetDuration(2.0f);
    originalAnimation.SetFrameRate(60.0f);
    originalAnimation.SetLoopMode(LoopMode::Loop);

    originalAnimation.AddPositionKeyframe("Bone1", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    originalAnimation.AddPositionKeyframe("Bone1", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    originalAnimation.AddRotationKeyframe("Bone1", 0.0f, Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));

    // Serialize
    auto data = originalAnimation.Serialize();
    EXPECT_EQUAL(data.name, "SerializationTest");
    EXPECT_NEARLY_EQUAL(data.duration, 2.0f);
    EXPECT_EQUAL(data.bones.size(), static_cast<size_t>(1));

    // Deserialize
    Animation newAnimation;
    EXPECT_TRUE(newAnimation.Deserialize(data));

    // Verify deserialized animation
    EXPECT_EQUAL(newAnimation.GetName(), "SerializationTest");
    EXPECT_NEARLY_EQUAL(newAnimation.GetDuration(), 2.0f);
    EXPECT_NEARLY_EQUAL(newAnimation.GetFrameRate(), 60.0f);
    EXPECT_EQUAL(newAnimation.GetLoopMode(), LoopMode::Loop);
    EXPECT_TRUE(newAnimation.HasBone("Bone1"));

    TestOutput::PrintTestPass("animation serialization");
    return true;
}

int main() {
    TestOutput::PrintHeader("Animation");

    // Initialize logger for tests
    Logger::GetInstance();

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Animation Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Creation", TestAnimationCreation);
        allPassed &= suite.RunTest("Keyframe Interpolation", TestKeyframeInterpolation);
        allPassed &= suite.RunTest("Animation Loop Modes", TestAnimationLoopModes);
        allPassed &= suite.RunTest("Pose Creation", TestPoseCreation);
        allPassed &= suite.RunTest("Pose Blending", TestPoseBlending);
        allPassed &= suite.RunTest("Pose Evaluation", TestPoseEvaluation);
        allPassed &= suite.RunTest("Animation Serialization", TestAnimationSerialization);

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