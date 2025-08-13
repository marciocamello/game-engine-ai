#include "TestUtils.h"
#include "Animation/Animation.h"
#include "Animation/Pose.h"
#include "Animation/Skeleton.h"
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

    // Create simple