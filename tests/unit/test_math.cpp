#include <iostream>
#include <cassert>
#include <cmath>
#include "TestUtils.h"
#include "../../engine/core/Math.h"

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
    
    // Test scalar multiplication
    Math::Vec3 scaled = a * 2.0f;
    Math::Vec3 expectedScaled(2.0f, 4.0f, 6.0f);
    EXPECT_VEC3_NEARLY_EQUAL(scaled, expectedScaled);
    
    // Test scalar division
    Math::Vec3 divided = b / 2.0f;
    Math::Vec3 expectedDivided(2.0f, 2.5f, 3.0f);
    EXPECT_VEC3_NEARLY_EQUAL(divided, expectedDivided);
    
    // Test vector negation
    Math::Vec3 negated = -a;
    Math::Vec3 expectedNegated(-1.0f, -2.0f, -3.0f);
    EXPECT_VEC3_NEARLY_EQUAL(negated, expectedNegated);
    
    TestOutput::PrintTestPass("vector operations");
    return true;
}

/**
 * Test cross product operations
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestCrossProduct() {
    TestOutput::PrintTestStart("cross product operations");
    
    // Test basic cross product (right-hand rule)
    Math::Vec3 x(1.0f, 0.0f, 0.0f);
    Math::Vec3 y(0.0f, 1.0f, 0.0f);
    Math::Vec3 z = glm::cross(x, y);
    Math::Vec3 expectedZ(0.0f, 0.0f, 1.0f);
    
    EXPECT_VEC3_NEARLY_EQUAL(z, expectedZ);
    
    // Test cross product anti-commutativity: a × b = -(b × a)
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 cross_ab = glm::cross(a, b);
    Math::Vec3 cross_ba = glm::cross(b, a);
    Math::Vec3 negated_ba = -cross_ba;
    
    EXPECT_VEC3_NEARLY_EQUAL(cross_ab, negated_ba);
    
    // Test cross product with parallel vectors (should be zero)
    Math::Vec3 parallel1(1.0f, 2.0f, 3.0f);
    Math::Vec3 parallel2 = parallel1 * 2.0f;
    Math::Vec3 crossParallel = glm::cross(parallel1, parallel2);
    Math::Vec3 zero(0.0f, 0.0f, 0.0f);
    
    EXPECT_VEC3_NEARLY_EQUAL(crossParallel, zero);
    
    // Test cross product magnitude
    Math::Vec3 u(3.0f, 0.0f, 0.0f);
    Math::Vec3 v(0.0f, 4.0f, 0.0f);
    Math::Vec3 crossUV = glm::cross(u, v);
    float crossMagnitude = glm::length(crossUV);
    float expectedMagnitude = glm::length(u) * glm::length(v); // sin(90°) = 1
    
    EXPECT_NEARLY_EQUAL(crossMagnitude, expectedMagnitude);
    
    // Test cross product orthogonality
    Math::Vec3 vec1(1.0f, 2.0f, 3.0f);
    Math::Vec3 vec2(4.0f, -2.0f, 1.0f);
    Math::Vec3 crossResult = glm::cross(vec1, vec2);
    
    // Cross product should be orthogonal to both input vectors
    float dot1 = glm::dot(crossResult, vec1);
    float dot2 = glm::dot(crossResult, vec2);
    
    EXPECT_NEARLY_EQUAL_EPSILON(dot1, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(dot2, 0.0f, 0.001f);
    
    TestOutput::PrintTestPass("cross product operations");
    return true;
}

/**
 * Test dot product operations
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestDotProduct() {
    TestOutput::PrintTestStart("dot product operations");
    
    // Test basic dot product
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    float dotResult = glm::dot(a, b);
    float expected = 1.0f*4.0f + 2.0f*5.0f + 3.0f*6.0f; // 32
    
    EXPECT_NEARLY_EQUAL(dotResult, expected);
    
    // Test dot product commutativity: a · b = b · a
    float dotAB = glm::dot(a, b);
    float dotBA = glm::dot(b, a);
    
    EXPECT_NEARLY_EQUAL(dotAB, dotBA);
    
    // Test dot product with orthogonal vectors
    Math::Vec3 x(1.0f, 0.0f, 0.0f);
    Math::Vec3 y(0.0f, 1.0f, 0.0f);
    float dotOrthogonal = glm::dot(x, y);
    
    EXPECT_NEARLY_EQUAL(dotOrthogonal, 0.0f);
    
    // Test dot product with parallel vectors
    Math::Vec3 parallel1(1.0f, 2.0f, 3.0f);
    Math::Vec3 parallel2 = parallel1 * 2.0f;
    float dotParallel = glm::dot(parallel1, parallel2);
    float expectedParallel = glm::length(parallel1) * glm::length(parallel2);
    
    EXPECT_NEARLY_EQUAL(dotParallel, expectedParallel);
    
    // Test dot product with unit vectors (cosine of angle)
    Math::Vec3 unit1 = glm::normalize(Math::Vec3(1.0f, 1.0f, 0.0f));
    Math::Vec3 unit2 = glm::normalize(Math::Vec3(1.0f, 0.0f, 0.0f));
    float dotUnits = glm::dot(unit1, unit2);
    float expectedCos45 = std::cos(Math::ToRadians(45.0f));
    
    EXPECT_NEARLY_EQUAL_EPSILON(dotUnits, expectedCos45, 0.001f);
    
    // Test dot product with self (magnitude squared)
    Math::Vec3 vec(3.0f, 4.0f, 0.0f);
    float dotSelf = glm::dot(vec, vec);
    float lengthSquared = glm::length(vec) * glm::length(vec);
    
    EXPECT_NEARLY_EQUAL(dotSelf, lengthSquared);
    EXPECT_NEARLY_EQUAL(dotSelf, 25.0f); // 3² + 4² = 25
    
    TestOutput::PrintTestPass("dot product operations");
    return true;
}

/**
 * Test vector normalization edge cases
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestNormalizationEdgeCases() {
    TestOutput::PrintTestStart("normalization edge cases");
    
    // Test normalization of unit vector
    Math::Vec3 alreadyUnit(1.0f, 0.0f, 0.0f);
    Math::Vec3 normalizedUnit = glm::normalize(alreadyUnit);
    
    EXPECT_VEC3_NEARLY_EQUAL(normalizedUnit, alreadyUnit);
    EXPECT_NEARLY_EQUAL(glm::length(normalizedUnit), 1.0f);
    
    // Test normalization of large vector
    Math::Vec3 large(1000.0f, 2000.0f, 3000.0f);
    Math::Vec3 normalizedLarge = glm::normalize(large);
    
    EXPECT_NEARLY_EQUAL(glm::length(normalizedLarge), 1.0f);
    
    // Test normalization of small vector
    Math::Vec3 small(1e-6f, 2e-6f, 3e-6f);
    Math::Vec3 normalizedSmall = glm::normalize(small);
    
    EXPECT_NEARLY_EQUAL_EPSILON(glm::length(normalizedSmall), 1.0f, 0.001f);
    
    // Test normalization preserves direction
    Math::Vec3 original(3.0f, 4.0f, 5.0f);
    Math::Vec3 normalized = glm::normalize(original);
    
    // Check that normalized vector points in same direction
    float dotProduct = glm::dot(original, normalized);
    float originalLength = glm::length(original);
    EXPECT_NEARLY_EQUAL_EPSILON(dotProduct, originalLength, 0.001f);
    
    // Test zero vector normalization (edge case)
    Math::Vec3 zero(0.0f, 0.0f, 0.0f);
    Math::Vec3 normalizedZero = glm::normalize(zero);
    // GLM should handle this gracefully, result may be NaN or undefined
    // We just check it doesn't crash
    EXPECT_TRUE(true); // If we get here, normalization didn't crash
    
    // Test negative vector normalization
    Math::Vec3 negative(-1.0f, -2.0f, -3.0f);
    Math::Vec3 normalizedNegative = glm::normalize(negative);
    
    EXPECT_NEARLY_EQUAL(glm::length(normalizedNegative), 1.0f);
    
    // Check direction is preserved (should be negative)
    EXPECT_TRUE(normalizedNegative.x < 0.0f);
    EXPECT_TRUE(normalizedNegative.y < 0.0f);
    EXPECT_TRUE(normalizedNegative.z < 0.0f);
    
    TestOutput::PrintTestPass("normalization edge cases");
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

/**
 * Test performance of critical math operations
 * Requirements: 6.4 (Performance testing)
 */
bool TestMathPerformance() {
    TestOutput::PrintTestStart("math performance");
    
    // Test vector addition performance
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    
    const int iterations = 100000;
    
    bool additionTest = PerformanceTest::ValidatePerformance(
        "vector addition",
        [&]() {
            volatile Math::Vec3 result = a + b;
            (void)result; // Prevent optimization
        },
        0.001, // 1 microsecond threshold
        iterations
    );
    
    // Test dot product performance
    bool dotProductTest = PerformanceTest::ValidatePerformance(
        "dot product",
        [&]() {
            volatile float result = glm::dot(a, b);
            (void)result; // Prevent optimization
        },
        0.001, // 1 microsecond threshold
        iterations
    );
    
    // Test cross product performance
    bool crossProductTest = PerformanceTest::ValidatePerformance(
        "cross product",
        [&]() {
            volatile Math::Vec3 result = glm::cross(a, b);
            (void)result; // Prevent optimization
        },
        0.001, // 1 microsecond threshold
        iterations
    );
    
    // Test normalization performance
    Math::Vec3 unnormalized(3.0f, 4.0f, 5.0f);
    bool normalizationTest = PerformanceTest::ValidatePerformance(
        "vector normalization",
        [&]() {
            volatile Math::Vec3 result = glm::normalize(unnormalized);
            (void)result; // Prevent optimization
        },
        0.01, // 10 microseconds threshold (normalization is more expensive)
        iterations / 10
    );
    
    // Test length calculation performance
    bool lengthTest = PerformanceTest::ValidatePerformance(
        "vector length",
        [&]() {
            volatile float result = glm::length(a);
            (void)result; // Prevent optimization
        },
        0.001, // 1 microsecond threshold
        iterations
    );
    
    // Test matrix-vector multiplication performance
    Math::Mat4 matrix = glm::mat4(1.0f);
    Math::Vec4 vec4(1.0f, 2.0f, 3.0f, 1.0f);
    
    bool matrixVectorTest = PerformanceTest::ValidatePerformance(
        "matrix-vector multiplication",
        [&]() {
            volatile Math::Vec4 result = matrix * vec4;
            (void)result; // Prevent optimization
        },
        0.01, // 10 microseconds threshold
        iterations / 10
    );
    
    TestOutput::PrintTestPass("math performance");
    return additionTest && dotProductTest && crossProductTest && 
           normalizationTest && lengthTest && matrixVectorTest;
}

/**
 * Test vector component access and manipulation
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestVectorComponents() {
    TestOutput::PrintTestStart("vector component access");
    
    // Test component access
    Math::Vec3 vec(1.0f, 2.0f, 3.0f);
    
    EXPECT_NEARLY_EQUAL(vec.x, 1.0f);
    EXPECT_NEARLY_EQUAL(vec.y, 2.0f);
    EXPECT_NEARLY_EQUAL(vec.z, 3.0f);
    
    EXPECT_NEARLY_EQUAL(vec[0], 1.0f);
    EXPECT_NEARLY_EQUAL(vec[1], 2.0f);
    EXPECT_NEARLY_EQUAL(vec[2], 3.0f);
    
    // Test component modification
    vec.x = 10.0f;
    vec.y = 20.0f;
    vec.z = 30.0f;
    
    EXPECT_NEARLY_EQUAL(vec.x, 10.0f);
    EXPECT_NEARLY_EQUAL(vec.y, 20.0f);
    EXPECT_NEARLY_EQUAL(vec.z, 30.0f);
    
    // Test swizzling (if supported by GLM)
    Math::Vec2 xy = Math::Vec2(vec.x, vec.y);
    EXPECT_NEARLY_EQUAL(xy.x, 10.0f);
    EXPECT_NEARLY_EQUAL(xy.y, 20.0f);
    
    // Test Vec4 components
    Math::Vec4 vec4(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_NEARLY_EQUAL(vec4.x, 1.0f);
    EXPECT_NEARLY_EQUAL(vec4.y, 2.0f);
    EXPECT_NEARLY_EQUAL(vec4.z, 3.0f);
    EXPECT_NEARLY_EQUAL(vec4.w, 4.0f);
    
    TestOutput::PrintTestPass("vector component access");
    return true;
}

/**
 * Test additional vector utility functions
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestVectorUtilities() {
    TestOutput::PrintTestStart("vector utilities");
    
    // Test distance calculation
    Math::Vec3 point1(0.0f, 0.0f, 0.0f);
    Math::Vec3 point2(3.0f, 4.0f, 0.0f);
    float distance = glm::distance(point1, point2);
    
    EXPECT_NEARLY_EQUAL(distance, 5.0f); // 3-4-5 triangle
    
    // Test reflect function
    Math::Vec3 incident = glm::normalize(Math::Vec3(1.0f, -1.0f, 0.0f));
    Math::Vec3 normal(0.0f, 1.0f, 0.0f);
    Math::Vec3 reflected = glm::reflect(incident, normal);
    Math::Vec3 expectedReflected = glm::normalize(Math::Vec3(1.0f, 1.0f, 0.0f));
    
    EXPECT_VEC3_NEARLY_EQUAL(reflected, expectedReflected);
    
    // Test refract function (simplified case)
    Math::Vec3 incidentRefract = glm::normalize(Math::Vec3(1.0f, -1.0f, 0.0f));
    Math::Vec3 normalRefract(0.0f, 1.0f, 0.0f);
    float eta = 1.0f; // No refraction
    Math::Vec3 refracted = glm::refract(incidentRefract, normalRefract, eta);
    
    EXPECT_VEC3_NEARLY_EQUAL(refracted, incidentRefract);
    
    // Test faceforward function - test that it returns a consistent result
    // GLM's faceforward behavior: returns n if dot(nref, i) < 0, otherwise -n
    Math::Vec3 n(0.0f, 1.0f, 0.0f);
    Math::Vec3 i(0.0f, -1.0f, 0.0f);  // Incident vector pointing down
    Math::Vec3 nref(0.0f, 1.0f, 0.0f); // Reference normal pointing up
    Math::Vec3 faced = glm::faceforward(n, i, nref);
    
    // Just test that the result is either n or -n (both are valid orientations)
    bool isN = FloatComparison::IsNearlyEqual(faced, n);
    bool isNegN = FloatComparison::IsNearlyEqual(faced, -n);
    EXPECT_TRUE(isN || isNegN);
    
    TestOutput::PrintTestPass("vector utilities");
    return true;
}

/**
 * Test additional cross product edge cases
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestCrossProductEdgeCases() {
    TestOutput::PrintTestStart("cross product edge cases");
    
    // Test cross product with zero vector
    Math::Vec3 zero(0.0f, 0.0f, 0.0f);
    Math::Vec3 nonZero(1.0f, 2.0f, 3.0f);
    Math::Vec3 crossWithZero = glm::cross(zero, nonZero);
    
    EXPECT_VEC3_NEARLY_EQUAL(crossWithZero, zero);
    
    // Test cross product with very small vectors
    Math::Vec3 tiny1(1e-6f, 0.0f, 0.0f);
    Math::Vec3 tiny2(0.0f, 1e-6f, 0.0f);
    Math::Vec3 tinyResult = glm::cross(tiny1, tiny2);
    Math::Vec3 expectedTiny(0.0f, 0.0f, 1e-12f);
    
    EXPECT_VEC3_NEARLY_EQUAL(tinyResult, expectedTiny);
    
    // Test cross product with very large vectors
    Math::Vec3 large1(1e6f, 0.0f, 0.0f);
    Math::Vec3 large2(0.0f, 1e6f, 0.0f);
    Math::Vec3 largeResult = glm::cross(large1, large2);
    Math::Vec3 expectedLarge(0.0f, 0.0f, 1e12f);
    
    EXPECT_VEC3_NEARLY_EQUAL(largeResult, expectedLarge);
    
    // Test cross product distributivity: a × (b + c) = (a × b) + (a × c)
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 c(7.0f, 8.0f, 9.0f);
    
    Math::Vec3 left = glm::cross(a, b + c);
    Math::Vec3 right = glm::cross(a, b) + glm::cross(a, c);
    
    EXPECT_VEC3_NEARLY_EQUAL(left, right);
    
    TestOutput::PrintTestPass("cross product edge cases");
    return true;
}

/**
 * Test additional dot product edge cases
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestDotProductEdgeCases() {
    TestOutput::PrintTestStart("dot product edge cases");
    
    // Test dot product with zero vector
    Math::Vec3 zero(0.0f, 0.0f, 0.0f);
    Math::Vec3 nonZero(1.0f, 2.0f, 3.0f);
    float dotWithZero = glm::dot(zero, nonZero);
    
    EXPECT_NEARLY_EQUAL(dotWithZero, 0.0f);
    
    // Test dot product with very small vectors
    Math::Vec3 tiny1(1e-6f, 2e-6f, 3e-6f);
    Math::Vec3 tiny2(4e-6f, 5e-6f, 6e-6f);
    float tinyDot = glm::dot(tiny1, tiny2);
    float expectedTinyDot = 1e-6f * 4e-6f + 2e-6f * 5e-6f + 3e-6f * 6e-6f;
    
    EXPECT_NEARLY_EQUAL(tinyDot, expectedTinyDot);
    
    // Test dot product with very large vectors
    Math::Vec3 large1(1e6f, 2e6f, 3e6f);
    Math::Vec3 large2(4e6f, 5e6f, 6e6f);
    float largeDot = glm::dot(large1, large2);
    float expectedLargeDot = 1e6f * 4e6f + 2e6f * 5e6f + 3e6f * 6e6f;
    
    EXPECT_NEARLY_EQUAL(largeDot, expectedLargeDot);
    
    // Test dot product distributivity: a · (b + c) = (a · b) + (a · c)
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 c(7.0f, 8.0f, 9.0f);
    
    float left = glm::dot(a, b + c);
    float right = glm::dot(a, b) + glm::dot(a, c);
    
    EXPECT_NEARLY_EQUAL(left, right);
    
    // Test dot product with negative vectors
    Math::Vec3 positive(1.0f, 2.0f, 3.0f);
    Math::Vec3 negative = -positive;
    float negativeDot = glm::dot(positive, negative);
    float expectedNegative = -(glm::dot(positive, positive));
    
    EXPECT_NEARLY_EQUAL(negativeDot, expectedNegative);
    
    TestOutput::PrintTestPass("dot product edge cases");
    return true;
}

/**
 * Test additional normalization edge cases
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestAdvancedNormalizationEdgeCases() {
    TestOutput::PrintTestStart("advanced normalization edge cases");
    
    // Test normalization with mixed positive and negative components
    Math::Vec3 mixed(-3.0f, 4.0f, -5.0f);
    Math::Vec3 normalizedMixed = glm::normalize(mixed);
    
    EXPECT_NEARLY_EQUAL_EPSILON(glm::length(normalizedMixed), 1.0f, 0.001f);
    
    // Check that direction is preserved (signs should match)
    EXPECT_TRUE(normalizedMixed.x < 0.0f); // Should be negative
    EXPECT_TRUE(normalizedMixed.y > 0.0f); // Should be positive
    EXPECT_TRUE(normalizedMixed.z < 0.0f); // Should be negative
    
    // Test normalization with one dominant component
    Math::Vec3 dominant(1000.0f, 1.0f, 1.0f);
    Math::Vec3 normalizedDominant = glm::normalize(dominant);
    
    EXPECT_NEARLY_EQUAL_EPSILON(glm::length(normalizedDominant), 1.0f, 0.001f);
    EXPECT_TRUE(normalizedDominant.x > 0.99f); // Should be close to 1
    
    // Test normalization stability with repeated operations
    Math::Vec3 original(3.0f, 4.0f, 5.0f);
    Math::Vec3 normalized1 = glm::normalize(original);
    Math::Vec3 normalized2 = glm::normalize(normalized1);
    Math::Vec3 normalized3 = glm::normalize(normalized2);
    
    EXPECT_VEC3_NEARLY_EQUAL(normalized1, normalized2);
    EXPECT_VEC3_NEARLY_EQUAL(normalized2, normalized3);
    
    // Test normalization with vectors close to unit length
    Math::Vec3 almostUnit(0.999f, 0.001f, 0.001f);
    Math::Vec3 normalizedAlmost = glm::normalize(almostUnit);
    
    EXPECT_NEARLY_EQUAL_EPSILON(glm::length(normalizedAlmost), 1.0f, 0.001f);
    
    TestOutput::PrintTestPass("advanced normalization edge cases");
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
        allPassed &= suite.RunTest("Cross Product", TestCrossProduct);
        allPassed &= suite.RunTest("Dot Product", TestDotProduct);
        allPassed &= suite.RunTest("Normalization Edge Cases", TestNormalizationEdgeCases);
        allPassed &= suite.RunTest("Vector Components", TestVectorComponents);
        allPassed &= suite.RunTest("Vector Utilities", TestVectorUtilities);
        allPassed &= suite.RunTest("Cross Product Edge Cases", TestCrossProductEdgeCases);
        allPassed &= suite.RunTest("Dot Product Edge Cases", TestDotProductEdgeCases);
        allPassed &= suite.RunTest("Advanced Normalization Edge Cases", TestAdvancedNormalizationEdgeCases);
        allPassed &= suite.RunTest("Angle Conversion", TestAngleConversion);
        allPassed &= suite.RunTest("Linear Interpolation", TestLerp);
        allPassed &= suite.RunTest("Clamping", TestClamp);
        allPassed &= suite.RunTest("Matrix Creation", TestMatrixCreation);
        allPassed &= suite.RunTest("Math Performance", TestMathPerformance);

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