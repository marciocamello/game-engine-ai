#include <iostream>
#include <cassert>
#include "Core/Math.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test EXPECT_NEAR_VEC3 macro functionality
 */
bool TestVec3AssertionMacro() {
    TestOutput::PrintTestStart("Vec3 assertion macro");

    // Test successful comparison (within default epsilon of 0.001f)
    Math::Vec3 vec1(1.0f, 2.0f, 3.0f);
    Math::Vec3 vec2(1.0005f, 2.0005f, 3.0005f);
    
    // This should pass with default epsilon
    EXPECT_NEAR_VEC3(vec1, vec2);
    
    TestOutput::PrintTestPass("Vec3 assertion macro");
    return true;
}

/**
 * Test EXPECT_MATRIX_EQUAL macro functionality
 */
bool TestMatrixAssertionMacro() {
    TestOutput::PrintTestStart("Matrix assertion macro");

    // Create two nearly identical matrices
    Math::Mat4 mat1 = Math::Mat4(1.0f);
    Math::Mat4 mat2 = Math::Mat4(1.0f);
    mat2[0][0] = 1.0005f; // Small difference within epsilon
    
    // This should pass with default epsilon
    EXPECT_MATRIX_EQUAL(mat1, mat2);
    
    TestOutput::PrintTestPass("Matrix assertion macro");
    return true;
}

/**
 * Test EXPECT_NEAR_QUAT macro functionality
 */
bool TestQuaternionAssertionMacro() {
    TestOutput::PrintTestStart("Quaternion assertion macro");

    // Create two nearly identical quaternions
    Math::Quat quat1(1.0f, 0.0f, 0.0f, 0.0f);
    Math::Quat quat2(1.0005f, 0.0005f, 0.0005f, 0.0005f);
    
    // This should pass with default epsilon
    EXPECT_NEAR_QUAT(quat1, quat2);
    
    TestOutput::PrintTestPass("Quaternion assertion macro");
    return true;
}

/**
 * Test basic assertion macros
 */
bool TestBasicAssertionMacros() {
    TestOutput::PrintTestStart("Basic assertion macros");

    // Test EXPECT_TRUE
    EXPECT_TRUE(true);
    
    // Test EXPECT_FALSE
    EXPECT_FALSE(false);
    
    // Test EXPECT_EQUAL
    EXPECT_EQUAL(42, 42);
    
    // Test EXPECT_NOT_EQUAL
    EXPECT_NOT_EQUAL(42, 43);
    
    // Test EXPECT_NEARLY_EQUAL
    EXPECT_NEARLY_EQUAL(1.0f, 1.0005f);
    
    // Test EXPECT_IN_RANGE
    EXPECT_IN_RANGE(5, 1, 10);
    
    // Test EXPECT_STRING_EQUAL
    EXPECT_STRING_EQUAL("hello", "hello");
    
    TestOutput::PrintTestPass("Basic assertion macros");
    return true;
}

/**
 * Test pointer assertion macros
 */
bool TestPointerAssertionMacros() {
    TestOutput::PrintTestStart("Pointer assertion macros");

    int value = 42;
    int* ptr = &value;
    int* nullPtr = nullptr;
    
    // Test EXPECT_NOT_NULL
    EXPECT_NOT_NULL(ptr);
    
    // Test EXPECT_NULL
    EXPECT_NULL(nullPtr);
    
    TestOutput::PrintTestPass("Pointer assertion macros");
    return true;
}

/**
 * Test custom epsilon assertion macros
 */
bool TestCustomEpsilonMacros() {
    TestOutput::PrintTestStart("Custom epsilon assertion macros");

    // Test EXPECT_NEARLY_EQUAL_EPSILON
    EXPECT_NEARLY_EQUAL_EPSILON(1.0f, 1.05f, 0.1f);
    
    // Test EXPECT_NEAR_VEC3_EPSILON
    Math::Vec3 vec1(1.0f, 2.0f, 3.0f);
    Math::Vec3 vec2(1.05f, 2.05f, 3.05f);
    EXPECT_NEAR_VEC3_EPSILON(vec1, vec2, 0.1f);
    
    // Test EXPECT_NEAR_QUAT_EPSILON
    Math::Quat quat1(1.0f, 0.0f, 0.0f, 0.0f);
    Math::Quat quat2(1.05f, 0.05f, 0.05f, 0.05f);
    EXPECT_NEAR_QUAT_EPSILON(quat1, quat2, 0.1f);
    
    // Test EXPECT_MATRIX_EQUAL_EPSILON
    Math::Mat4 mat1 = Math::Mat4(1.0f);
    Math::Mat4 mat2 = Math::Mat4(1.0f);
    mat2[0][0] = 1.05f;
    EXPECT_MATRIX_EQUAL_EPSILON(mat1, mat2, 0.1f);
    
    TestOutput::PrintTestPass("Custom epsilon assertion macros");
    return true;
}

int main() {
    TestOutput::PrintHeader("Assertion Macros");

    bool allPassed = true;

    try {
        allPassed &= TestVec3AssertionMacro();
        allPassed &= TestMatrixAssertionMacro();
        allPassed &= TestQuaternionAssertionMacro();
        allPassed &= TestBasicAssertionMacros();
        allPassed &= TestPointerAssertionMacros();
        allPassed &= TestCustomEpsilonMacros();

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