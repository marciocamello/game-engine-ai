#include <iostream>
#include <cassert>
#include <cmath>
#include "TestUtils.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test basic vector operations
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestVectorOperations() {
    TestOutput::PrintTestStart("vector operations");
    
    // Test vector addition
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 result = a + b;
    Math::Vec3 expected(5.0f, 7.0f, 9.0f);
    
    EXPECT_VEC3_NEARLY_EQUAL(result, expected);
    
    // Test vector subtraction
    Math::Vec3 diff = b - a;
    Math::Vec3 expectedDiff(3.0f, 3.0f, 3.0f);
    EXPECT_VEC3_NEARLY_EQUAL(diff, expectedDiff);
    
    // Test vector length
    Math::Vec3 unit(1.0f, 0.0f, 0.0f);
    float length = glm::length(unit);
    EXPECT_NEARLY_EQUAL(length, 1.0f);
    
    TestOutput::PrintTestPass("vector operations");
    return true;
}

/**
 * Test angle conversion functions
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestAngleConversion() {
    TestOutput::PrintTestStart("angle conversions");
    
    // Test degrees to radians
    float degrees = 90.0f;
    float radians = Math::ToRadians(degrees);
    EXPECT_NEARLY_EQUAL(radians, Math::HALF_PI);
    
    // Test radians to degrees
    float backToDegrees = Math::ToDegrees(radians);
    EXPECT_NEARLY_EQUAL(backToDegrees, 90.0f);
    
    // Test constants
    EXPECT_NEARLY_EQUAL(Math::PI, 3.14159265359f);
    EXPECT_NEARLY_EQUAL(Math::TWO_PI, 2.0f * Math::PI);
    
    TestOutput::PrintTestPass("angle conversions");
    return true;
}

/**
 * Test linear interpolation
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestLerp() {
    TestOutput::PrintTestStart("linear interpolation");
    
    // Test float lerp
    float result = Math::Lerp(0.0f, 10.0f, 0.5f);
    EXPECT_NEARLY_EQUAL(result, 5.0f);
    
    // Test vector lerp
    Math::Vec3 start(0.0f, 0.0f, 0.0f);
    Math::Vec3 end(10.0f, 20.0f, 30.0f);
    Math::Vec3 mid = Math::Lerp(start, end, 0.5f);
    Math::Vec3 expectedMid(5.0f, 10.0f, 15.0f);
    
    EXPECT_VEC3_NEARLY_EQUAL(mid, expectedMid);
    
    TestOutput::PrintTestPass("linear interpolation");
    return true;
}

/**
 * Test clamping functions
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestClamp() {
    TestOutput::PrintTestStart("clamping");
    
    // Test float clamp - value within range
    EXPECT_NEARLY_EQUAL(Math::Clamp(5.0f, 0.0f, 10.0f), 5.0f);
    
    // Test float clamp - value below range
    EXPECT_NEARLY_EQUAL(Math::Clamp(-5.0f, 0.0f, 10.0f), 0.0f);
    
    // Test float clamp - value above range
    EXPECT_NEARLY_EQUAL(Math::Clamp(15.0f, 0.0f, 10.0f), 10.0f);
    
    TestOutput::PrintTestPass("clamping");
    return true;
}

/**
 * Test matrix creation functions
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixCreation() {
    TestOutput::PrintTestStart("matrix creation");
    
    // Test transform matrix creation
    Math::Vec3 position(1.0f, 2.0f, 3.0f);
    Math::Quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    Math::Vec3 scale(1.0f, 1.0f, 1.0f);
    
    Math::Mat4 transform = Math::CreateTransform(position, rotation, scale);
    
    // Test that position is correctly encoded in the matrix
    EXPECT_NEARLY_EQUAL(transform[3][0], position.x);
    EXPECT_NEARLY_EQUAL(transform[3][1], position.y);
    EXPECT_NEARLY_EQUAL(transform[3][2], position.z);
    
    // Test perspective matrix creation
    Math::Mat4 perspective = Math::CreatePerspectiveMatrix(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
    EXPECT_TRUE(perspective[0][0] != 0.0f); // Should have valid projection values
    
    TestOutput::PrintTestPass("matrix creation");
    return true;
}

int main() {
    TestOutput::PrintHeader("Math");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Math Tests");

        // Run all tests
        allPassed &= suite.RunTest("Vector Operations", TestVectorOperations);
        allPassed &= suite.RunTest("Angle Conversion", TestAngleConversion);
        allPassed &= suite.RunTest("Linear Interpolation", TestLerp);
        allPassed &= suite.RunTest("Clamping", TestClamp);
        allPassed &= suite.RunTest("Matrix Creation", TestMatrixCreation);

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