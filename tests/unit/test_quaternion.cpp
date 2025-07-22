#include <iostream>
#include <cassert>
#include <cmath>
#include "TestUtils.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test quaternion construction and basic properties
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestQuaternionConstruction() {
    TestOutput::PrintTestStart("quaternion construction");
    
    // Test identity quaternion
    Math::Quat identity = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_NEARLY_EQUAL(identity.w, 1.0f);
    EXPECT_NEARLY_EQUAL(identity.x, 0.0f);
    EXPECT_NEARLY_EQUAL(identity.y, 0.0f);
    EXPECT_NEARLY_EQUAL(identity.z, 0.0f);
    
    // Test GLM identity quaternion using proper identity function
    Math::Quat glmIdentity = glm::identity<Math::Quat>();
    EXPECT_NEARLY_EQUAL(glmIdentity.w, 1.0f);
    EXPECT_NEARLY_EQUAL(glmIdentity.x, 0.0f);
    EXPECT_NEARLY_EQUAL(glmIdentity.y, 0.0f);
    EXPECT_NEARLY_EQUAL(glmIdentity.z, 0.0f);
    
    // Test custom quaternion construction
    Math::Quat custom(0.5f, 0.5f, 0.5f, 0.5f);
    EXPECT_NEARLY_EQUAL(custom.w, 0.5f);
    EXPECT_NEARLY_EQUAL(custom.x, 0.5f);
    EXPECT_NEARLY_EQUAL(custom.y, 0.5f);
    EXPECT_NEARLY_EQUAL(custom.z, 0.5f);
    
    // Test quaternion from axis-angle
    Math::Vec3 axis(0.0f, 1.0f, 0.0f);
    float angle = Math::ToRadians(90.0f);
    Math::Quat axisAngle = glm::angleAxis(angle, axis);
    
    // For 90-degree Y rotation: w = cos(45°), y = sin(45°)
    float expected = std::sqrt(2.0f) / 2.0f; // cos(45°) = sin(45°)
    EXPECT_NEARLY_EQUAL_EPSILON(axisAngle.w, expected, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(axisAngle.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(axisAngle.y, expected, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(axisAngle.z, 0.0f, 0.001f);
    
    TestOutput::PrintTestPass("quaternion construction");
    return true;
}

/**
 * Test quaternion multiplication operations
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestQuaternionMultiplication() {
    TestOutput::PrintTestStart("quaternion multiplication");
    
    // Test identity multiplication
    Math::Quat identity = glm::identity<Math::Quat>();
    Math::Quat test(0.5f, 0.5f, 0.5f, 0.5f);
    
    Math::Quat result1 = identity * test;
    Math::Quat result2 = test * identity;
    
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(result1, test));
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(result2, test));
    
    // Test rotation composition (90° Y then 90° X)
    Math::Quat rotY = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    Math::Quat rotX = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
    
    Math::Quat combined = rotX * rotY;
    
    // Apply to a test vector
    Math::Vec3 testVec(1.0f, 0.0f, 0.0f);
    Math::Vec3 rotated = glm::rotate(combined, testVec);
    
    // After Y rotation: X -> -Z, then X rotation: -Z -> Y
    EXPECT_NEARLY_EQUAL_EPSILON(rotated.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotated.y, 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotated.z, 0.0f, 0.001f);
    
    // Test quaternion conjugate multiplication by testing rotation effect
    // Create a normalized quaternion for testing
    Math::Quat q = glm::normalize(Math::Quat(0.6f, 0.8f, 0.0f, 0.0f));
    Math::Quat qConj = glm::conjugate(q);
    Math::Quat product = q * qConj;
    
    // Test that q * q* acts as identity rotation on vectors using glm::rotate
    Math::Vec3 conjugateTestVec(1.0f, 2.0f, 3.0f);
    Math::Vec3 conjugateRotatedVec = glm::rotate(product, conjugateTestVec);
    
    // The vector should remain unchanged (identity rotation)
    EXPECT_NEARLY_EQUAL_EPSILON(conjugateRotatedVec.x, conjugateTestVec.x, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(conjugateRotatedVec.y, conjugateTestVec.y, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(conjugateRotatedVec.z, conjugateTestVec.z, 0.001f);
    
    TestOutput::PrintTestPass("quaternion multiplication");
    return true;
}

/**
 * Test quaternion normalization operations
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestQuaternionNormalization() {
    TestOutput::PrintTestStart("quaternion normalization");
    
    // Test already normalized quaternion
    Math::Quat identity = glm::identity<Math::Quat>();
    float identityLength = glm::length(identity);
    EXPECT_NEARLY_EQUAL(identityLength, 1.0f);
    
    Math::Quat normalizedIdentity = glm::normalize(identity);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(identity, normalizedIdentity));
    
    // Test unnormalized quaternion
    Math::Quat unnormalized(2.0f, 3.0f, 4.0f, 5.0f);
    float unnormalizedLength = glm::length(unnormalized);
    float expectedLength = std::sqrt(2.0f*2.0f + 3.0f*3.0f + 4.0f*4.0f + 5.0f*5.0f);
    EXPECT_NEARLY_EQUAL_EPSILON(unnormalizedLength, expectedLength, 0.001f);
    
    Math::Quat normalized = glm::normalize(unnormalized);
    float normalizedLength = glm::length(normalized);
    EXPECT_NEARLY_EQUAL_EPSILON(normalizedLength, 1.0f, 0.001f);
    
    // Test zero quaternion normalization (edge case)
    Math::Quat zero(0.0f, 0.0f, 0.0f, 0.0f);
    Math::Quat normalizedZero = glm::normalize(zero);
    // GLM should handle this gracefully, but result may be undefined
    // We just check it doesn't crash
    EXPECT_TRUE(true); // If we get here, normalization didn't crash
    
    // Test very small quaternion
    Math::Quat tiny(1e-6f, 1e-6f, 1e-6f, 1e-6f);
    Math::Quat normalizedTiny = glm::normalize(tiny);
    float tinyNormalizedLength = glm::length(normalizedTiny);
    EXPECT_NEARLY_EQUAL_EPSILON(tinyNormalizedLength, 1.0f, 0.001f);
    
    TestOutput::PrintTestPass("quaternion normalization");
    return true;
}

/**
 * Test quaternion to matrix conversion
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestQuaternionToMatrix() {
    TestOutput::PrintTestStart("quaternion to matrix conversion");
    
    // Test identity quaternion to matrix
    Math::Quat identity = glm::identity<Math::Quat>();
    Math::Mat4 identityMatrix = glm::mat4_cast(identity);
    Math::Mat4 expectedIdentity = glm::mat4(1.0f);
    
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(identityMatrix, expectedIdentity, 0.001f));
    
    // Test 90-degree Y rotation
    Math::Quat rotY = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    Math::Mat4 rotYMatrix = glm::mat4_cast(rotY);
    
    // Test transformation of X-axis vector
    Math::Vec4 xAxis(1.0f, 0.0f, 0.0f, 1.0f);
    Math::Vec4 rotatedX = rotYMatrix * xAxis;
    
    // 90° Y rotation should transform X to -Z
    EXPECT_NEARLY_EQUAL_EPSILON(rotatedX.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotatedX.y, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotatedX.z, -1.0f, 0.001f);
    
    // Test 90-degree X rotation
    Math::Quat rotX = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
    Math::Mat4 rotXMatrix = glm::mat4_cast(rotX);
    
    // Test transformation of Y-axis vector
    Math::Vec4 yAxis(0.0f, 1.0f, 0.0f, 1.0f);
    Math::Vec4 rotatedY = rotXMatrix * yAxis;
    
    // 90° X rotation should transform Y to Z
    EXPECT_NEARLY_EQUAL_EPSILON(rotatedY.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotatedY.y, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotatedY.z, 1.0f, 0.001f);
    
    TestOutput::PrintTestPass("quaternion to matrix conversion");
    return true;
}

/**
 * Test SLERP (Spherical Linear Interpolation)
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestQuaternionSLERP() {
    TestOutput::PrintTestStart("quaternion SLERP");
    
    // Test SLERP between identity and 90-degree rotation
    Math::Quat identity = glm::identity<Math::Quat>();
    Math::Quat rot90 = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    
    // Test t=0 (should be identity)
    Math::Quat slerp0 = glm::slerp(identity, rot90, 0.0f);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(slerp0, identity, 0.001f));
    
    // Test t=1 (should be rot90)
    Math::Quat slerp1 = glm::slerp(identity, rot90, 1.0f);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(slerp1, rot90, 0.001f));
    
    // Test t=0.5 (should be 45-degree rotation)
    Math::Quat slerp05 = glm::slerp(identity, rot90, 0.5f);
    Math::Quat expected45 = glm::angleAxis(Math::ToRadians(45.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(slerp05, expected45, 0.001f));
    
    // Test SLERP maintains unit length
    for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
        Math::Quat interpolated = glm::slerp(identity, rot90, t);
        float length = glm::length(interpolated);
        EXPECT_NEARLY_EQUAL_EPSILON(length, 1.0f, 0.001f);
    }
    
    // Test SLERP between opposite quaternions
    Math::Quat q1 = glm::angleAxis(Math::ToRadians(30.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    Math::Quat q2 = glm::angleAxis(Math::ToRadians(150.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    
    Math::Quat slerpMid = glm::slerp(q1, q2, 0.5f);
    Math::Quat expectedMid = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(slerpMid, expectedMid, 0.01f));
    
    TestOutput::PrintTestPass("quaternion SLERP");
    return true;
}

/**
 * Test rotation composition and order
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestRotationComposition() {
    TestOutput::PrintTestStart("rotation composition");

    // Create 90-degree rotations around X and Y axes
    Math::Quat rotX = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
    Math::Quat rotY = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));

    // Compose rotations in different orders
    // Note: quatA * quatB applies quatB first, then quatA
    Math::Quat rotYX = rotX * rotY; // Applies Y then X
    Math::Quat rotXY = rotY * rotX; // Applies X then Y

    // Quaternion multiplication is not commutative
    EXPECT_FALSE(FloatComparison::IsNearlyEqual(rotXY, rotYX, 0.001f));

    // Apply composed rotation to a vector
    Math::Vec3 testVec(1.0f, 0.0f, 0.0f);

    // Apply Y then X to (1,0,0)
    // Y rotates (1,0,0) → (0,0,-1)
    // X rotates (0,0,-1) → (0,1,0)
    Math::Vec3 result_yx = glm::rotate(rotYX, testVec);
    EXPECT_NEARLY_EQUAL_EPSILON(result_yx.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(result_yx.y, 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(result_yx.z, 0.0f, 0.001f);

    // Apply X then Y to (1,0,0)
    // X rotates (1,0,0) → (1,0,0)
    // Y rotates (1,0,0) → (0,0,-1)
    Math::Vec3 result_xy = glm::rotate(rotXY, testVec);
    EXPECT_NEARLY_EQUAL_EPSILON(result_xy.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(result_xy.y, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(result_xy.z, -1.0f, 0.001f);

    // Test that a quaternion followed by its inverse results in an identity rotation
    Math::Quat rotInverse = glm::inverse(rotY);
    Math::Quat identity = rotY * rotInverse;

    // Rotating with the identity quaternion should leave the vector unchanged
    Math::Vec3 testVectors[] = {
        Math::Vec3(1.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 1.0f, 0.0f),
        Math::Vec3(0.0f, 0.0f, 1.0f),
        Math::Vec3(1.0f, 1.0f, 1.0f)
    };

    for (const auto& vec : testVectors) {
        Math::Vec3 rotated = glm::rotate(identity, vec);
        EXPECT_NEARLY_EQUAL_EPSILON(rotated.x, vec.x, 0.001f);
        EXPECT_NEARLY_EQUAL_EPSILON(rotated.y, vec.y, 0.001f);
        EXPECT_NEARLY_EQUAL_EPSILON(rotated.z, vec.z, 0.001f);
    }

    TestOutput::PrintTestPass("rotation composition");
    return true;
}

/**
 * Test quaternion edge cases and boundary conditions
 * Requirements: 3.1, 3.4 (Simple test structure and edge cases)
 */
bool TestQuaternionEdgeCases() {
    TestOutput::PrintTestStart("quaternion edge cases");
    
    // Test very small angle rotation
    float tinyAngle = Math::ToRadians(0.001f);
    Math::Quat tinyRot = glm::angleAxis(tinyAngle, Math::Vec3(0.0f, 1.0f, 0.0f));
    
    // Should be very close to identity
    Math::Quat identity = glm::identity<Math::Quat>();
    EXPECT_NEARLY_EQUAL_EPSILON(tinyRot.w, 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(tinyRot.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(tinyRot.y, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(tinyRot.z, 0.0f, 0.001f);
    
    // Test 180-degree rotation
    Math::Quat rot180 = glm::angleAxis(Math::ToRadians(180.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    EXPECT_NEARLY_EQUAL_EPSILON(rot180.w, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rot180.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rot180.y, 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rot180.z, 0.0f, 0.001f);
    
    // Test 360-degree rotation (should be equivalent to identity)
    Math::Quat rot360 = glm::angleAxis(Math::ToRadians(360.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    // Note: 360° rotation gives -identity quaternion (double cover)
    EXPECT_NEARLY_EQUAL_EPSILON(std::abs(rot360.w), 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rot360.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rot360.y, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rot360.z, 0.0f, 0.001f);
    
    // Test negative angle
    Math::Quat rotNeg90 = glm::angleAxis(Math::ToRadians(-90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    Math::Quat rotPos90 = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    Math::Quat rotPos90Inv = glm::inverse(rotPos90);
    
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(rotNeg90, rotPos90Inv, 0.001f));
    
    TestOutput::PrintTestPass("quaternion edge cases");
    return true;
}

/**
 * Test quaternion conversion to/from Euler angles
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestEulerConversion() {
    TestOutput::PrintTestStart("Euler angle conversion");
    
    // Test simple rotations
    Math::Vec3 eulerY(0.0f, Math::ToRadians(90.0f), 0.0f);
    Math::Quat quatFromEuler = glm::quat(eulerY);
    Math::Quat expectedY = glm::angleAxis(Math::ToRadians(90.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(quatFromEuler, expectedY, 0.001f));
    
    // Test conversion back to Euler
    Math::Vec3 eulerBack = glm::eulerAngles(quatFromEuler);
    EXPECT_NEARLY_EQUAL_EPSILON(eulerBack.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(eulerBack.y, Math::ToRadians(90.0f), 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(eulerBack.z, 0.0f, 0.001f);
    
    // Test combined rotations
    Math::Vec3 eulerXYZ(Math::ToRadians(30.0f), Math::ToRadians(45.0f), Math::ToRadians(60.0f));
    Math::Quat quatXYZ = glm::quat(eulerXYZ);
    Math::Vec3 eulerBackXYZ = glm::eulerAngles(quatXYZ);
    
    // Due to gimbal lock and multiple representations, we test by applying rotation
    Math::Vec3 testVec(1.0f, 0.0f, 0.0f);
    Math::Vec3 rotatedOriginal = quatXYZ * testVec;
    
    Math::Quat quatFromBack = glm::quat(eulerBackXYZ);
    Math::Vec3 rotatedBack = quatFromBack * testVec;
    
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(rotatedOriginal, rotatedBack, 0.001f));
    
    TestOutput::PrintTestPass("Euler angle conversion");
    return true;
}

/**
 * Test quaternion performance characteristics
 * Requirements: 6.4 (Performance testing)
 */
bool TestQuaternionPerformance() {
    TestOutput::PrintTestStart("quaternion performance");
    
    // Test quaternion multiplication performance
    Math::Quat q1 = glm::angleAxis(Math::ToRadians(45.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    Math::Quat q2 = glm::angleAxis(Math::ToRadians(30.0f), Math::Vec3(1.0f, 0.0f, 0.0f));
    
    const int iterations = 10000;
    
    bool performanceTest = PerformanceTest::ValidatePerformance(
        "quaternion multiplication",
        [&]() {
            volatile Math::Quat result = q1 * q2;
            (void)result; // Prevent optimization
        },
        0.001, // 1 microsecond threshold
        iterations
    );
    
    // Test quaternion normalization performance
    Math::Quat unnormalized(2.0f, 3.0f, 4.0f, 5.0f);
    
    bool normalizationTest = PerformanceTest::ValidatePerformance(
        "quaternion normalization",
        [&]() {
            volatile Math::Quat result = glm::normalize(unnormalized);
            (void)result; // Prevent optimization
        },
        0.001, // 1 microsecond threshold
        iterations
    );
    
    // Test SLERP performance
    bool slerpTest = PerformanceTest::ValidatePerformance(
        "quaternion SLERP",
        [&]() {
            volatile Math::Quat result = glm::slerp(q1, q2, 0.5f);
            (void)result; // Prevent optimization
        },
        0.01, // 10 microseconds threshold (SLERP is more expensive)
        iterations / 10 // Fewer iterations for more expensive operation
    );
    
    TestOutput::PrintTestPass("quaternion performance");
    return performanceTest && normalizationTest && slerpTest;
}

int main() {
    TestOutput::PrintHeader("Quaternion");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Quaternion Tests");

        // Run all tests
        allPassed &= suite.RunTest("Quaternion Construction", TestQuaternionConstruction);
        allPassed &= suite.RunTest("Quaternion Multiplication", TestQuaternionMultiplication);
        allPassed &= suite.RunTest("Quaternion Normalization", TestQuaternionNormalization);
        allPassed &= suite.RunTest("Quaternion to Matrix", TestQuaternionToMatrix);
        allPassed &= suite.RunTest("Quaternion SLERP", TestQuaternionSLERP);
        allPassed &= suite.RunTest("Rotation Composition", TestRotationComposition);
        allPassed &= suite.RunTest("Quaternion Edge Cases", TestQuaternionEdgeCases);
        allPassed &= suite.RunTest("Euler Conversion", TestEulerConversion);
        allPassed &= suite.RunTest("Quaternion Performance", TestQuaternionPerformance);

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