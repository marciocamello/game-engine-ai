#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletUtils.h"
#include "TestUtils.h"
#include <iostream>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Physics;
using namespace GameEngine::Testing;

/**
 * Test Vec3 to Bullet conversion
 * Requirements: Physics system integration
 */
bool TestVec3ToBulletConversion() {
    TestOutput::PrintTestStart("Vec3 to Bullet conversion");
    
    Math::Vec3 engineVec(1.0f, 2.0f, 3.0f);
    btVector3 bulletVec = BulletUtils::ToBullet(engineVec);
    
    EXPECT_NEARLY_EQUAL(bulletVec.getX(), 1.0f);
    EXPECT_NEARLY_EQUAL(bulletVec.getY(), 2.0f);
    EXPECT_NEARLY_EQUAL(bulletVec.getZ(), 3.0f);
    
    TestOutput::PrintTestPass("Vec3 to Bullet conversion");
    return true;
}

/**
 * Test Vec3 from Bullet conversion
 * Requirements: Physics system integration
 */
bool TestVec3FromBulletConversion() {
    TestOutput::PrintTestStart("Vec3 from Bullet conversion");
    
    btVector3 bulletVec(4.0f, 5.0f, 6.0f);
    Math::Vec3 engineVec = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_NEARLY_EQUAL(engineVec.x, 4.0f);
    EXPECT_NEARLY_EQUAL(engineVec.y, 5.0f);
    EXPECT_NEARLY_EQUAL(engineVec.z, 6.0f);
    
    TestOutput::PrintTestPass("Vec3 from Bullet conversion");
    return true;
}

/**
 * Test Vec3 round-trip conversion
 * Requirements: Physics system integration
 */
bool TestVec3RoundTripConversion() {
    TestOutput::PrintTestStart("Vec3 round-trip conversion");
    
    Math::Vec3 original(7.5f, -2.3f, 0.0f);
    btVector3 bulletVec = BulletUtils::ToBullet(original);
    Math::Vec3 converted = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_VEC3_NEARLY_EQUAL(original, converted);
    
    TestOutput::PrintTestPass("Vec3 round-trip conversion");
    return true;
}

/**
 * Test Quaternion to Bullet conversion
 * Requirements: Physics system integration
 */
bool TestQuaternionToBulletConversion() {
    TestOutput::PrintTestStart("Quaternion to Bullet conversion");
    
    Math::Quat engineQuat(0.707f, 0.0f, 0.707f, 0.0f); // 90 degree rotation around Y
    btQuaternion bulletQuat = BulletUtils::ToBullet(engineQuat);
    
    EXPECT_NEARLY_EQUAL(bulletQuat.getX(), 0.0f);
    EXPECT_NEARLY_EQUAL(bulletQuat.getY(), 0.707f);
    EXPECT_NEARLY_EQUAL(bulletQuat.getZ(), 0.0f);
    EXPECT_NEARLY_EQUAL(bulletQuat.getW(), 0.707f);
    
    TestOutput::PrintTestPass("Quaternion to Bullet conversion");
    return true;
}

/**
 * Test Quaternion from Bullet conversion
 * Requirements: Physics system integration
 */
bool TestQuaternionFromBulletConversion() {
    TestOutput::PrintTestStart("Quaternion from Bullet conversion");
    
    btQuaternion bulletQuat(0.5f, 0.5f, 0.5f, 0.5f);
    Math::Quat engineQuat = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_NEARLY_EQUAL(engineQuat.x, 0.5f);
    EXPECT_NEARLY_EQUAL(engineQuat.y, 0.5f);
    EXPECT_NEARLY_EQUAL(engineQuat.z, 0.5f);
    EXPECT_NEARLY_EQUAL(engineQuat.w, 0.5f);
    
    TestOutput::PrintTestPass("Quaternion from Bullet conversion");
    return true;
}

/**
 * Test Quaternion round-trip conversion
 * Requirements: Physics system integration
 */
bool TestQuaternionRoundTripConversion() {
    TestOutput::PrintTestStart("Quaternion round-trip conversion");
    
    Math::Quat original(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    btQuaternion bulletQuat = BulletUtils::ToBullet(original);
    Math::Quat converted = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_NEARLY_EQUAL(original.x, converted.x);
    EXPECT_NEARLY_EQUAL(original.y, converted.y);
    EXPECT_NEARLY_EQUAL(original.z, converted.z);
    EXPECT_NEARLY_EQUAL(original.w, converted.w);
    
    TestOutput::PrintTestPass("Quaternion round-trip conversion");
    return true;
}

/**
 * Test zero vector conversion
 * Requirements: Physics system integration
 */
bool TestZeroVectorConversion() {
    TestOutput::PrintTestStart("zero vector conversion");
    
    Math::Vec3 zero(0.0f, 0.0f, 0.0f);
    btVector3 bulletZero = BulletUtils::ToBullet(zero);
    Math::Vec3 convertedZero = BulletUtils::FromBullet(bulletZero);
    
    EXPECT_VEC3_NEARLY_EQUAL(zero, convertedZero);
    
    TestOutput::PrintTestPass("zero vector conversion");
    return true;
}

/**
 * Test negative values conversion
 * Requirements: Physics system integration
 */
bool TestNegativeValuesConversion() {
    TestOutput::PrintTestStart("negative values conversion");
    
    Math::Vec3 negative(-1.0f, -2.0f, -3.0f);
    btVector3 bulletNegative = BulletUtils::ToBullet(negative);
    Math::Vec3 convertedNegative = BulletUtils::FromBullet(bulletNegative);
    
    EXPECT_VEC3_NEARLY_EQUAL(negative, convertedNegative);
    
    TestOutput::PrintTestPass("negative values conversion");
    return true;
}

/**
 * Test extreme values conversion (boundary conditions)
 * Requirements: 6.1, 6.2, 6.4
 */
bool TestExtremeValuesConversion() {
    TestOutput::PrintTestStart("extreme values conversion");
    
    // Test very large values
    Math::Vec3 large(1e6f, -1e6f, 1e5f);
    btVector3 bulletLarge = BulletUtils::ToBullet(large);
    Math::Vec3 convertedLarge = BulletUtils::FromBullet(bulletLarge);
    EXPECT_VEC3_NEARLY_EQUAL(large, convertedLarge);
    
    // Test very small values
    Math::Vec3 small(1e-6f, -1e-6f, 1e-5f);
    btVector3 bulletSmall = BulletUtils::ToBullet(small);
    Math::Vec3 convertedSmall = BulletUtils::FromBullet(bulletSmall);
    EXPECT_VEC3_NEARLY_EQUAL(small, convertedSmall);
    
    TestOutput::PrintTestPass("extreme values conversion");
    return true;
}

/**
 * Test floating-point precision limits
 * Requirements: 6.1, 6.2, 6.4
 */
bool TestFloatingPointPrecision() {
    TestOutput::PrintTestStart("floating-point precision limits");
    
    // Test values near floating-point precision limits
    Math::Vec3 precise(1.0f + 1e-7f, 2.0f - 1e-7f, 3.0f + 1e-6f);
    btVector3 bulletPrecise = BulletUtils::ToBullet(precise);
    Math::Vec3 convertedPrecise = BulletUtils::FromBullet(bulletPrecise);
    
    // Use tighter epsilon for precision testing
    EXPECT_NEARLY_EQUAL_EPSILON(precise.x, convertedPrecise.x, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(precise.y, convertedPrecise.y, 1e-6f);
    EXPECT_NEARLY_EQUAL_EPSILON(precise.z, convertedPrecise.z, 1e-6f);
    
    TestOutput::PrintTestPass("floating-point precision limits");
    return true;
}

/**
 * Test normalized vector conversion
 * Requirements: 6.1, 6.2, 6.4
 */
bool TestNormalizedVectorConversion() {
    TestOutput::PrintTestStart("normalized vector conversion");
    
    // Test unit vectors
    Math::Vec3 unitX(1.0f, 0.0f, 0.0f);
    Math::Vec3 unitY(0.0f, 1.0f, 0.0f);
    Math::Vec3 unitZ(0.0f, 0.0f, 1.0f);
    
    btVector3 bulletUnitX = BulletUtils::ToBullet(unitX);
    btVector3 bulletUnitY = BulletUtils::ToBullet(unitY);
    btVector3 bulletUnitZ = BulletUtils::ToBullet(unitZ);
    
    Math::Vec3 convertedUnitX = BulletUtils::FromBullet(bulletUnitX);
    Math::Vec3 convertedUnitY = BulletUtils::FromBullet(bulletUnitY);
    Math::Vec3 convertedUnitZ = BulletUtils::FromBullet(bulletUnitZ);
    
    EXPECT_VEC3_NEARLY_EQUAL(unitX, convertedUnitX);
    EXPECT_VEC3_NEARLY_EQUAL(unitY, convertedUnitY);
    EXPECT_VEC3_NEARLY_EQUAL(unitZ, convertedUnitZ);
    
    // Test normalized arbitrary vector
    Math::Vec3 arbitrary(3.0f, 4.0f, 5.0f);
    float length = std::sqrt(arbitrary.x * arbitrary.x + arbitrary.y * arbitrary.y + arbitrary.z * arbitrary.z);
    Math::Vec3 normalized(arbitrary.x / length, arbitrary.y / length, arbitrary.z / length);
    
    btVector3 bulletNormalized = BulletUtils::ToBullet(normalized);
    Math::Vec3 convertedNormalized = BulletUtils::FromBullet(bulletNormalized);
    
    EXPECT_VEC3_NEARLY_EQUAL(normalized, convertedNormalized);
    
    TestOutput::PrintTestPass("normalized vector conversion");
    return true;
}

/**
 * Test quaternion boundary conditions
 * Requirements: 6.1, 6.2, 6.4
 */
bool TestQuaternionBoundaryConditions() {
    TestOutput::PrintTestStart("quaternion boundary conditions");
    
    // Test identity quaternion
    Math::Quat identity(1.0f, 0.0f, 0.0f, 0.0f);
    btQuaternion bulletIdentity = BulletUtils::ToBullet(identity);
    Math::Quat convertedIdentity = BulletUtils::FromBullet(bulletIdentity);
    
    EXPECT_NEARLY_EQUAL(identity.w, convertedIdentity.w);
    EXPECT_NEARLY_EQUAL(identity.x, convertedIdentity.x);
    EXPECT_NEARLY_EQUAL(identity.y, convertedIdentity.y);
    EXPECT_NEARLY_EQUAL(identity.z, convertedIdentity.z);
    
    // Test 180-degree rotations around each axis
    Math::Quat rotX180(0.0f, 1.0f, 0.0f, 0.0f);
    Math::Quat rotY180(0.0f, 0.0f, 1.0f, 0.0f);
    Math::Quat rotZ180(0.0f, 0.0f, 0.0f, 1.0f);
    
    btQuaternion bulletRotX180 = BulletUtils::ToBullet(rotX180);
    btQuaternion bulletRotY180 = BulletUtils::ToBullet(rotY180);
    btQuaternion bulletRotZ180 = BulletUtils::ToBullet(rotZ180);
    
    Math::Quat convertedRotX180 = BulletUtils::FromBullet(bulletRotX180);
    Math::Quat convertedRotY180 = BulletUtils::FromBullet(bulletRotY180);
    Math::Quat convertedRotZ180 = BulletUtils::FromBullet(bulletRotZ180);
    
    EXPECT_NEARLY_EQUAL(rotX180.w, convertedRotX180.w);
    EXPECT_NEARLY_EQUAL(rotX180.x, convertedRotX180.x);
    EXPECT_NEARLY_EQUAL(rotX180.y, convertedRotX180.y);
    EXPECT_NEARLY_EQUAL(rotX180.z, convertedRotX180.z);
    
    EXPECT_NEARLY_EQUAL(rotY180.w, convertedRotY180.w);
    EXPECT_NEARLY_EQUAL(rotY180.x, convertedRotY180.x);
    EXPECT_NEARLY_EQUAL(rotY180.y, convertedRotY180.y);
    EXPECT_NEARLY_EQUAL(rotY180.z, convertedRotY180.z);
    
    EXPECT_NEARLY_EQUAL(rotZ180.w, convertedRotZ180.w);
    EXPECT_NEARLY_EQUAL(rotZ180.x, convertedRotZ180.x);
    EXPECT_NEARLY_EQUAL(rotZ180.y, convertedRotZ180.y);
    EXPECT_NEARLY_EQUAL(rotZ180.z, convertedRotZ180.z);
    
    TestOutput::PrintTestPass("quaternion boundary conditions");
    return true;
}

/**
 * Test coordinate system consistency
 * Requirements: 6.1, 6.2, 6.4
 */
bool TestCoordinateSystemConsistency() {
    TestOutput::PrintTestStart("coordinate system consistency");
    
    // Test that coordinate system handedness is preserved
    Math::Vec3 forward(0.0f, 0.0f, 1.0f);
    Math::Vec3 right(1.0f, 0.0f, 0.0f);
    Math::Vec3 up(0.0f, 1.0f, 0.0f);
    
    btVector3 bulletForward = BulletUtils::ToBullet(forward);
    btVector3 bulletRight = BulletUtils::ToBullet(right);
    btVector3 bulletUp = BulletUtils::ToBullet(up);
    
    Math::Vec3 convertedForward = BulletUtils::FromBullet(bulletForward);
    Math::Vec3 convertedRight = BulletUtils::FromBullet(bulletRight);
    Math::Vec3 convertedUp = BulletUtils::FromBullet(bulletUp);
    
    EXPECT_VEC3_NEARLY_EQUAL(forward, convertedForward);
    EXPECT_VEC3_NEARLY_EQUAL(right, convertedRight);
    EXPECT_VEC3_NEARLY_EQUAL(up, convertedUp);
    
    TestOutput::PrintTestPass("coordinate system consistency");
    return true;
}

/**
 * Test conversion performance
 * Requirements: 6.1, 6.2, 6.4
 */
bool TestConversionPerformance() {
    TestOutput::PrintTestStart("conversion performance");
    
    const int iterations = 10000;
    Math::Vec3 testVec(1.5f, 2.5f, 3.5f);
    Math::Quat testQuat(0.707f, 0.0f, 0.707f, 0.0f);
    
    // Test Vec3 conversion performance
    TestTimer vecTimer;
    for (int i = 0; i < iterations; ++i) {
        btVector3 bulletVec = BulletUtils::ToBullet(testVec);
        Math::Vec3 convertedVec = BulletUtils::FromBullet(bulletVec);
        // Prevent optimization from removing the conversion
        volatile float dummy = convertedVec.x;
        (void)dummy;
    }
    double vecTime = vecTimer.ElapsedMs();
    
    // Test Quaternion conversion performance
    TestTimer quatTimer;
    for (int i = 0; i < iterations; ++i) {
        btQuaternion bulletQuat = BulletUtils::ToBullet(testQuat);
        Math::Quat convertedQuat = BulletUtils::FromBullet(bulletQuat);
        // Prevent optimization from removing the conversion
        volatile float dummy = convertedQuat.w;
        (void)dummy;
    }
    double quatTime = quatTimer.ElapsedMs();
    
    TestOutput::PrintTiming("Vec3 conversions", vecTime, iterations);
    TestOutput::PrintTiming("Quaternion conversions", quatTime, iterations);
    
    // Performance should be reasonable (less than 1ms per 1000 conversions)
    EXPECT_TRUE(vecTime < 1.0f);
    EXPECT_TRUE(quatTime < 1.0f);
    
    TestOutput::PrintTestPass("conversion performance");
    return true;
}

int main() {
    TestOutput::PrintHeader("Bullet Utils Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Bullet Utils Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Vec3 to Bullet Conversion", TestVec3ToBulletConversion);
        allPassed &= suite.RunTest("Vec3 from Bullet Conversion", TestVec3FromBulletConversion);
        allPassed &= suite.RunTest("Vec3 Round-trip Conversion", TestVec3RoundTripConversion);
        allPassed &= suite.RunTest("Quaternion to Bullet Conversion", TestQuaternionToBulletConversion);
        allPassed &= suite.RunTest("Quaternion from Bullet Conversion", TestQuaternionFromBulletConversion);
        allPassed &= suite.RunTest("Quaternion Round-trip Conversion", TestQuaternionRoundTripConversion);
        allPassed &= suite.RunTest("Zero Vector Conversion", TestZeroVectorConversion);
        allPassed &= suite.RunTest("Negative Values Conversion", TestNegativeValuesConversion);
        allPassed &= suite.RunTest("Extreme Values Conversion", TestExtremeValuesConversion);
        allPassed &= suite.RunTest("Floating-Point Precision", TestFloatingPointPrecision);
        allPassed &= suite.RunTest("Normalized Vector Conversion", TestNormalizedVectorConversion);
        allPassed &= suite.RunTest("Quaternion Boundary Conditions", TestQuaternionBoundaryConditions);
        allPassed &= suite.RunTest("Coordinate System Consistency", TestCoordinateSystemConsistency);
        allPassed &= suite.RunTest("Conversion Performance", TestConversionPerformance);

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

#else

#include "TestUtils.h"
#include <iostream>

using namespace GameEngine::Testing;

int main() {
    TestOutput::PrintHeader("Bullet Utils Integration");
    TestOutput::PrintWarning("Bullet Physics not available - skipping conversion utility tests");
    TestOutput::PrintFooter(true);
    return 0;
}

#endif // GAMEENGINE_HAS_BULLET