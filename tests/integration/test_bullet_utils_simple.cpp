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