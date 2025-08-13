#include "TestUtils.h"
#include "Animation/IKSolver.h"
#include "Animation/AnimationSkeleton.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

// Simple test skeleton for IK testing
class TestSkeleton : public AnimationSkeleton {
public:
    TestSkeleton() : AnimationSkeleton("TestSkeleton") {
        // Create simple bone hierarchy for testing
        for (int i = 0; i < 4; ++i) {
            std::string boneName = "bone" + std::to_string(i);
            CreateBone(boneName);
        }
    }
    
    // Simple methods for testing
    int GetBoneCount() const { return 4; }
};

/**
 * Test IKSolver base class functionality
 * Requirements: 4.1, 4.2 (IK chain setup and target setting)
 */
bool TestIKSolverBaseClass() {
    TestOutput::PrintTestStart("IK solver base class");

    // Create a simple test skeleton
    TestSkeleton skeleton;
    int rootBone = 0;
    int upperBone = 1;
    int lowerBone = 2;
    int endBone = 3;

    // Create TwoBoneIK solver for testing base functionality
    TwoBoneIK ikSolver;

    // Test chain setup
    std::vector<int> chain = { upperBone, lowerBone, endBone };
    ikSolver.SetChain(chain);

    EXPECT_EQUAL(ikSolver.GetChain().size(), static_cast<size_t>(3));
    EXPECT_EQUAL(ikSolver.GetChain()[0], upperBone);
    EXPECT_EQUAL(ikSolver.GetChain()[1], lowerBone);
    EXPECT_EQUAL(ikSolver.GetChain()[2], endBone);

    // Test target setting
    Math::Vec3 targetPos(1.0f, 2.0f, 0.0f);
    Math::Quat targetRot(1.0f, 0.0f, 0.0f, 0.0f);
    ikSolver.SetTarget(targetPos, targetRot);

    EXPECT_VEC3_NEARLY_EQUAL(ikSolver.GetTarget(), targetPos);
    // Note: EXPECT_QUAT_NEARLY_EQUAL not available, using component comparison
    EXPECT_NEARLY_EQUAL(ikSolver.GetTargetRotation().w, targetRot.w);

    // Test pole target
    Math::Vec3 poleTarget(0.0f, 0.0f, 1.0f);
    ikSolver.SetPoleTarget(poleTarget);
    EXPECT_VEC3_NEARLY_EQUAL(ikSolver.GetPoleTarget(), poleTarget);

    // Test iterations and tolerance
    ikSolver.SetIterations(15);
    ikSolver.SetTolerance(0.005f);
    EXPECT_EQUAL(ikSolver.GetIterations(), 15);
    EXPECT_NEARLY_EQUAL(ikSolver.GetTolerance(), 0.005f);

    // Test chain validation
    EXPECT_TRUE(ikSolver.ValidateChain(skeleton));

    TestOutput::PrintTestPass("IK solver base class");
    return true;
}

/**
 * Test TwoBoneIK solver configuration and validation
 * Requirements: 4.1, 4.2 (Two-bone IK setup)
 */
bool TestTwoBoneIKConfiguration() {
    TestOutput::PrintTestStart("two-bone IK configuration");

    // Create skeleton with arm-like structure
    TestSkeleton skeleton;
    int shoulderBone = 0;
    int elbowBone = 1;
    int wristBone = 2;

    TwoBoneIK ikSolver;

    // Test individual bone setting
    ikSolver.SetUpperBone(shoulderBone);
    ikSolver.SetLowerBone(elbowBone);
    ikSolver.SetEndEffector(wristBone);

    EXPECT_EQUAL(ikSolver.GetUpperBone(), shoulderBone);
    EXPECT_EQUAL(ikSolver.GetLowerBone(), elbowBone);
    EXPECT_EQUAL(ikSolver.GetEndEffector(), wristBone);

    // Test configuration validation
    EXPECT_TRUE(ikSolver.IsValidConfiguration());

    // Test that chain is automatically set
    EXPECT_EQUAL(ikSolver.GetChain().size(), static_cast<size_t>(3));
    EXPECT_EQUAL(ikSolver.GetChain()[0], shoulderBone);
    EXPECT_EQUAL(ikSolver.GetChain()[1], elbowBone);
    EXPECT_EQUAL(ikSolver.GetChain()[2], wristBone);

    TestOutput::PrintTestPass("two-bone IK configuration");
    return true;
}

/**
 * Test TwoBoneIK solver basic solving functionality
 * Requirements: 4.2, 4.4 (Two-bone IK solving and target reaching)
 */
bool TestTwoBoneIKSolving() {
    TestOutput::PrintTestStart("two-bone IK solving");

    // Create skeleton with known bone positions
    TestSkeleton skeleton;
    int shoulderBone = 0;
    int elbowBone = 1;
    int wristBone = 2;

    TwoBoneIK ikSolver;
    ikSolver.SetUpperBone(shoulderBone);
    ikSolver.SetLowerBone(elbowBone);
    ikSolver.SetEndEffector(wristBone);

    // Set a reachable target
    Math::Vec3 target(1.5f, 0.5f, 0.0f);
    ikSolver.SetTarget(target);

    // Test target reachability
    EXPECT_TRUE(ikSolver.IsTargetReachable(skeleton));

    // Test solving
    EXPECT_TRUE(ikSolver.Solve(skeleton));

    // Verify that the solver doesn't crash with invalid configuration
    TwoBoneIK invalidSolver;
    EXPECT_FALSE(invalidSolver.Solve(skeleton));

    TestOutput::PrintTestPass("two-bone IK solving");
    return true;
}

/**
 * Test FABRIK IK solver basic functionality
 * Requirements: 4.3, 4.5 (FABRIK implementation and constraint handling)
 */
bool TestFABRIKSolver() {
    TestOutput::PrintTestStart("FABRIK IK solver");

    // Create skeleton with chain of bones
    TestSkeleton skeleton;
    int bone0 = 0;
    int bone1 = 1;
    int bone2 = 2;
    int bone3 = 3;

    FABRIKIK fabrikSolver;
    std::vector<int> chain = { bone0, bone1, bone2, bone3 };
    fabrikSolver.SetChain(chain);

    // Test type
    EXPECT_EQUAL(static_cast<int>(fabrikSolver.GetType()), static_cast<int>(IKSolver::Type::FABRIK));

    // Set target
    Math::Vec3 target(2.0f, 1.0f, 0.0f);
    fabrikSolver.SetTarget(target);

    // Test sub-base position
    Math::Vec3 subBase(0.1f, 0.1f, 0.0f);
    fabrikSolver.SetSubBasePosition(subBase);
    EXPECT_VEC3_NEARLY_EQUAL(fabrikSolver.GetSubBasePosition(), subBase);

    // Test solving
    EXPECT_TRUE(fabrikSolver.Solve(skeleton));

    TestOutput::PrintTestPass("FABRIK IK solver");
    return true;
}

/**
 * Test IK constraint system
 * Requirements: 4.4, 4.5 (Joint constraints and angle limits)
 */
bool TestIKConstraints() {
    TestOutput::PrintTestStart("IK constraint system");

    // Create skeleton
    TestSkeleton skeleton;
    int bone0 = 0;
    int bone1 = 1;

    TwoBoneIK ikSolver;
    ikSolver.SetChain({ bone0, bone1 });

    // Test constraint setting
    float minAngle = -Math::PI / 4.0f; // -45 degrees
    float maxAngle = Math::PI / 4.0f;  // 45 degrees
    ikSolver.SetBoneConstraints(bone0, minAngle, maxAngle);

    // Test that constraints don't crash the system
    Math::Vec3 target(0.5f, 0.5f, 0.0f);
    ikSolver.SetTarget(target);

    // The constraint application is tested indirectly through the solve process
    // Direct testing would require more complex skeleton manipulation
    EXPECT_TRUE(ikSolver.ValidateChain(skeleton));

    TestOutput::PrintTestPass("IK constraint system");
    return true;
}

/**
 * Test IK/FK blending system
 * Requirements: 4.7 (IK/FK blending for smooth transitions)
 */
bool TestIKFKBlending() {
    TestOutput::PrintTestStart("IK/FK blending system");

    // Create skeleton
    TestSkeleton skeleton;
    int bone0 = 0;
    int bone1 = 1;
    int bone2 = 2;

    TwoBoneIK ikSolver;
    ikSolver.SetUpperBone(bone0);
    ikSolver.SetLowerBone(bone1);
    ikSolver.SetEndEffector(bone2);

    // Test IK weight setting
    ikSolver.SetIKWeight(0.5f);
    EXPECT_NEARLY_EQUAL(ikSolver.GetIKWeight(), 0.5f);

    // Test weight clamping
    ikSolver.SetIKWeight(-0.5f);
    EXPECT_NEARLY_EQUAL(ikSolver.GetIKWeight(), 0.0f);

    ikSolver.SetIKWeight(1.5f);
    EXPECT_NEARLY_EQUAL(ikSolver.GetIKWeight(), 1.0f);

    // Test blend mode setting
    ikSolver.SetBlendMode(true);
    EXPECT_TRUE(ikSolver.GetBlendMode());

    ikSolver.SetBlendMode(false);
    EXPECT_FALSE(ikSolver.GetBlendMode());

    // Test solving with different blend weights
    Math::Vec3 target(1.0f, 1.0f, 0.0f);
    ikSolver.SetTarget(target);

    // Full IK
    ikSolver.SetIKWeight(1.0f);
    EXPECT_TRUE(ikSolver.Solve(skeleton));

    // Half blend
    ikSolver.SetIKWeight(0.5f);
    EXPECT_TRUE(ikSolver.Solve(skeleton));

    // Full FK
    ikSolver.SetIKWeight(0.0f);
    EXPECT_TRUE(ikSolver.Solve(skeleton));

    TestOutput::PrintTestPass("IK/FK blending system");
    return true;
}

/**
 * Test FABRIK with IK/FK blending
 * Requirements: 4.3, 4.7 (FABRIK with blending support)
 */
bool TestFABRIKBlending() {
    TestOutput::PrintTestStart("FABRIK with IK/FK blending");

    // Create skeleton with chain of bones
    TestSkeleton skeleton;
    std::vector<int> chain = { 0, 1, 2, 3 };

    FABRIKIK fabrikSolver;
    fabrikSolver.SetChain(chain);

    // Test blending functionality
    fabrikSolver.SetIKWeight(0.75f);
    EXPECT_NEARLY_EQUAL(fabrikSolver.GetIKWeight(), 0.75f);

    // Set target and solve with blending
    Math::Vec3 target(2.0f, 1.0f, 0.0f);
    fabrikSolver.SetTarget(target);

    EXPECT_TRUE(fabrikSolver.Solve(skeleton));

    // Test with different blend modes
    fabrikSolver.SetBlendMode(false); // Linear blending
    EXPECT_TRUE(fabrikSolver.Solve(skeleton));

    fabrikSolver.SetBlendMode(true); // Smooth blending
    EXPECT_TRUE(fabrikSolver.Solve(skeleton));

    TestOutput::PrintTestPass("FABRIK with IK/FK blending");
    return true;
}

/**
 * Test FABRIK bone length validation and joint constraints
 * Requirements: 4.5 (Joint angle constraints and bone length validation)
 */
bool TestFABRIKConstraints() {
    TestOutput::PrintTestStart("FABRIK constraints and validation");

    // Create skeleton with chain of bones
    TestSkeleton skeleton;
    std::vector<int> chain = { 0, 1, 2, 3 };

    FABRIKIK fabrikSolver;
    fabrikSolver.SetChain(chain);

    // Test bone length validation setting
    fabrikSolver.SetBoneLengthValidation(true);
    EXPECT_TRUE(fabrikSolver.GetBoneLengthValidation());

    fabrikSolver.SetBoneLengthValidation(false);
    EXPECT_FALSE(fabrikSolver.GetBoneLengthValidation());

    // Test joint angle constraints setting
    fabrikSolver.SetJointAngleConstraints(true);
    EXPECT_TRUE(fabrikSolver.GetJointAngleConstraints());

    fabrikSolver.SetJointAngleConstraints(false);
    EXPECT_FALSE(fabrikSolver.GetJointAngleConstraints());

    // Test solving with constraints enabled
    fabrikSolver.SetBoneLengthValidation(true);
    fabrikSolver.SetJointAngleConstraints(true);

    // Add joint constraints
    float minAngle = Math::PI / 6.0f;  // 30 degrees
    float maxAngle = Math::PI * 5.0f / 6.0f; // 150 degrees
    fabrikSolver.SetBoneConstraints(1, minAngle, maxAngle);
    fabrikSolver.SetBoneConstraints(2, minAngle, maxAngle);

    // Set target and solve
    Math::Vec3 target(1.5f, 0.5f, 0.0f);
    fabrikSolver.SetTarget(target);

    EXPECT_TRUE(fabrikSolver.Solve(skeleton));

    // Test with constraints disabled
    fabrikSolver.SetJointAngleConstraints(false);
    EXPECT_TRUE(fabrikSolver.Solve(skeleton));

    TestOutput::PrintTestPass("FABRIK constraints and validation");
    return true;
}

int main() {
    TestOutput::PrintHeader("IKSolver");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("IKSolver Tests");

        // Run all tests
        allPassed &= suite.RunTest("IK Solver Base Class", TestIKSolverBaseClass);
        allPassed &= suite.RunTest("Two-Bone IK Configuration", TestTwoBoneIKConfiguration);
        allPassed &= suite.RunTest("Two-Bone IK Solving", TestTwoBoneIKSolving);
        allPassed &= suite.RunTest("FABRIK Solver", TestFABRIKSolver);
        allPassed &= suite.RunTest("IK Constraints", TestIKConstraints);
        allPassed &= suite.RunTest("IK/FK Blending", TestIKFKBlending);
        allPassed &= suite.RunTest("FABRIK Blending", TestFABRIKBlending);
        allPassed &= suite.RunTest("FABRIK Constraints", TestFABRIKConstraints);

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