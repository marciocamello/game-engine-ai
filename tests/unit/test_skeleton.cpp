#include "TestUtils.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/Bone.h"
#include "Core/Logger.h"

using namespace GameEngine;
using namespace GameEngine::Animation;
using namespace GameEngine::Testing;

/**
 * Test skeleton creation and basic bone management
 * Requirements: 1.1, 1.4, 8.2
 */
bool TestSkeletonCreation() {
    TestOutput::PrintTestStart("skeleton creation and bone management");

    AnimationSkeleton skeleton("TestSkeleton");
    EXPECT_EQUAL(skeleton.GetName(), "TestSkeleton");
    EXPECT_EQUAL(skeleton.GetBoneCount(), static_cast<size_t>(0));

    // Create root bone
    auto rootBone = skeleton.CreateBone("Root");
    EXPECT_NOT_NULL(rootBone);
    EXPECT_EQUAL(skeleton.GetBoneCount(), static_cast<size_t>(1));
    EXPECT_EQUAL(skeleton.GetRootBone(), rootBone);

    // Create child bones
    auto childBone1 = skeleton.CreateBone("Child1");
    auto childBone2 = skeleton.CreateBone("Child2");
    EXPECT_NOT_NULL(childBone1);
    EXPECT_NOT_NULL(childBone2);
    EXPECT_EQUAL(skeleton.GetBoneCount(), static_cast<size_t>(3));

    // Set up hierarchy
    skeleton.SetBoneParent("Child1", "Root");
    skeleton.SetBoneParent("Child2", "Root");

    EXPECT_EQUAL(childBone1->GetParent(), rootBone);
    EXPECT_EQUAL(childBone2->GetParent(), rootBone);
    EXPECT_EQUAL(rootBone->GetChildren().size(), static_cast<size_t>(2));

    TestOutput::PrintTestPass("skeleton creation and bone management");
    return true;
}

/**
 * Test bone hierarchy and transform calculations
 * Requirements: 1.4, 1.5, 9.2
 */
bool TestBoneTransformCalculations() {
    TestOutput::PrintTestStart("bone transform calculations");

    AnimationSkeleton skeleton("TransformTest");

    // Create simple hierarchy: Root -> Child -> Grandchild
    auto root = skeleton.CreateBone("Root");
    auto child = skeleton.CreateBone("Child");
    auto grandchild = skeleton.CreateBone("Grandchild");

    skeleton.SetBoneParent("Child", "Root");
    skeleton.SetBoneParent("Grandchild", "Child");

    // Set local transforms
    Math::Vec3 rootPos(1.0f, 0.0f, 0.0f);
    Math::Vec3 childPos(0.0f, 1.0f, 0.0f);
    Math::Vec3 grandchildPos(0.0f, 0.0f, 1.0f);

    root->SetLocalPosition(rootPos);
    child->SetLocalPosition(childPos);
    grandchild->SetLocalPosition(grandchildPos);

    // Update transforms
    skeleton.UpdateBoneTransforms();

    // Verify world positions
    Math::Vec3 rootWorldPos = root->GetWorldPosition();
    Math::Vec3 childWorldPos = child->GetWorldPosition();
    Math::Vec3 grandchildWorldPos = grandchild->GetWorldPosition();

    EXPECT_VEC3_NEARLY_EQUAL(rootWorldPos, rootPos);
    EXPECT_VEC3_NEARLY_EQUAL(childWorldPos, rootPos + childPos);
    EXPECT_VEC3_NEARLY_EQUAL(grandchildWorldPos, rootPos + childPos + grandchildPos);

    TestOutput::PrintTestPass("bone transform calculations");
    return true;
}

/**
 * Test skinning matrix generation
 * Requirements: 1.5, 9.2
 */
bool TestSkinningMatrixGeneration() {
    TestOutput::PrintTestStart("skinning matrix generation");

    AnimationSkeleton skeleton("SkinningTest");

    // Create bones
    auto bone1 = skeleton.CreateBone("Bone1");
    auto bone2 = skeleton.CreateBone("Bone2");

    // Set bind poses
    Math::Mat4 bindPose1 = glm::translate(Math::Mat4(1.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
    Math::Mat4 bindPose2 = glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, 1.0f, 0.0f));

    bone1->SetBindPose(bindPose1);
    bone1->SetInverseBindPose(glm::inverse(bindPose1));
    bone2->SetBindPose(bindPose2);
    bone2->SetInverseBindPose(glm::inverse(bindPose2));

    // Set current transforms
    Math::Mat4 currentTransform1 = glm::translate(Math::Mat4(1.0f), Math::Vec3(2.0f, 0.0f, 0.0f));
    Math::Mat4 currentTransform2 = glm::translate(Math::Mat4(1.0f), Math::Vec3(0.0f, 2.0f, 0.0f));

    bone1->SetLocalTransform(currentTransform1);
    bone2->SetLocalTransform(currentTransform2);

    skeleton.UpdateBoneTransforms();

    // Get skinning matrices
    auto skinningMatrices = skeleton.GetSkinningMatrices();
    EXPECT_EQUAL(skinningMatrices.size(), static_cast<size_t>(2));

    // Verify skinning matrices are calculated correctly
    Math::Mat4 expectedSkinning1 = bone1->GetWorldTransform() * bone1->GetInverseBindPose();
    Math::Mat4 expectedSkinning2 = bone2->GetWorldTransform() * bone2->GetInverseBindPose();

    // Compare matrices (check a few key elements)
    EXPECT_NEARLY_EQUAL(skinningMatrices[0][3][0], expectedSkinning1[3][0]);
    EXPECT_NEARLY_EQUAL(skinningMatrices[1][3][1], expectedSkinning2[3][1]);

    TestOutput::PrintTestPass("skinning matrix generation");
    return true;
}

/**
 * Test skeleton hierarchy validation
 * Requirements: 1.1, 8.2
 */
bool TestSkeletonValidation() {
    TestOutput::PrintTestStart("skeleton hierarchy validation");

    AnimationSkeleton skeleton("ValidationTest");

    // Create valid hierarchy
    auto root = skeleton.CreateBone("Root");
    auto child1 = skeleton.CreateBone("Child1");
    auto child2 = skeleton.CreateBone("Child2");
    auto grandchild = skeleton.CreateBone("Grandchild");

    skeleton.SetBoneParent("Child1", "Root");
    skeleton.SetBoneParent("Child2", "Root");
    skeleton.SetBoneParent("Grandchild", "Child1");

    // Validate hierarchy
    EXPECT_TRUE(skeleton.ValidateHierarchy());

    // Test depth calculation
    EXPECT_EQUAL(root->GetDepth(), 0);
    EXPECT_EQUAL(child1->GetDepth(), 1);
    EXPECT_EQUAL(grandchild->GetDepth(), 2);
    EXPECT_EQUAL(skeleton.GetMaxDepth(), 2);

    TestOutput::PrintTestPass("skeleton hierarchy validation");
    return true;
}

/**
 * Test skeleton serialization and deserialization
 * Requirements: 8.2
 */
bool TestSkeletonSerialization() {
    TestOutput::PrintTestStart("skeleton serialization");

    // Create original skeleton
    AnimationSkeleton originalSkeleton("SerializationTest");
    auto root = originalSkeleton.CreateBone("Root");
    auto child = originalSkeleton.CreateBone("Child");
    originalSkeleton.SetBoneParent("Child", "Root");

    // Set bind poses
    originalSkeleton.SetBindPose();

    // Serialize
    auto data = originalSkeleton.Serialize();
    EXPECT_EQUAL(data.name, "SerializationTest");
    EXPECT_EQUAL(data.boneNames.size(), static_cast<size_t>(2));

    // Create new skeleton and deserialize
    AnimationSkeleton newSkeleton;
    EXPECT_TRUE(newSkeleton.Deserialize(data));

    // Verify deserialized skeleton
    EXPECT_EQUAL(newSkeleton.GetName(), "SerializationTest");
    EXPECT_EQUAL(newSkeleton.GetBoneCount(), static_cast<size_t>(2));
    
    auto deserializedRoot = newSkeleton.GetBone("Root");
    auto deserializedChild = newSkeleton.GetBone("Child");
    EXPECT_NOT_NULL(deserializedRoot);
    EXPECT_NOT_NULL(deserializedChild);
    EXPECT_EQUAL(deserializedChild->GetParent(), deserializedRoot);

    TestOutput::PrintTestPass("skeleton serialization");
    return true;
}

int main() {
    TestOutput::PrintHeader("Skeleton");

    // Initialize logger for tests
    Logger::GetInstance();

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Skeleton Tests");

        // Run all tests
        allPassed &= suite.RunTest("Skeleton Creation", TestSkeletonCreation);
        allPassed &= suite.RunTest("Bone Transform Calculations", TestBoneTransformCalculations);
        allPassed &= suite.RunTest("Skinning Matrix Generation", TestSkinningMatrixGeneration);
        allPassed &= suite.RunTest("Skeleton Validation", TestSkeletonValidation);
        allPassed &= suite.RunTest("Skeleton Serialization", TestSkeletonSerialization);

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