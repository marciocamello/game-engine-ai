#include <iostream>
#include <cassert>
#include <cmath>
#include "TestUtils.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test basic matrix construction and access
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixConstruction() {
    TestOutput::PrintTestStart("matrix construction and access");
    
    // Test identity matrix construction
    Math::Mat4 identity = glm::mat4(1.0f);
    
    // Check diagonal elements are 1
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEARLY_EQUAL(identity[i][i], 1.0f);
    }
    
    // Check off-diagonal elements are 0
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i != j) {
                EXPECT_NEARLY_EQUAL(identity[i][j], 0.0f);
            }
        }
    }
    
    // Test custom matrix construction
    Math::Mat4 custom(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    
    // Test element access (GLM uses column-major order)
    // custom[col][row] - first index is column, second is row
    EXPECT_NEARLY_EQUAL(custom[0][0], 1.0f);  // column 0, row 0
    EXPECT_NEARLY_EQUAL(custom[0][1], 2.0f);  // column 0, row 1
    EXPECT_NEARLY_EQUAL(custom[0][2], 3.0f);  // column 0, row 2
    EXPECT_NEARLY_EQUAL(custom[0][3], 4.0f);  // column 0, row 3
    
    // Test zero matrix
    Math::Mat4 zero(0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEARLY_EQUAL(zero[i][j], 0.0f);
        }
    }
    
    TestOutput::PrintTestPass("matrix construction and access");
    return true;
}

/**
 * Test matrix addition and subtraction
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixAdditionSubtraction() {
    TestOutput::PrintTestStart("matrix addition and subtraction");
    
    Math::Mat4 a(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    
    Math::Mat4 b(
        16.0f, 15.0f, 14.0f, 13.0f,
        12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f,
        4.0f, 3.0f, 2.0f, 1.0f
    );
    
    // Test matrix addition
    Math::Mat4 sum = a + b;
    Math::Mat4 expectedSum(17.0f); // All elements should be 17
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEARLY_EQUAL(sum[i][j], 17.0f);
        }
    }
    
    // Test matrix subtraction
    Math::Mat4 diff = a - b;
    
    // Check specific elements (remember column-major order)
    EXPECT_NEARLY_EQUAL(diff[0][0], -15.0f); // 1 - 16
    EXPECT_NEARLY_EQUAL(diff[0][1], -13.0f); // 2 - 15
    EXPECT_NEARLY_EQUAL(diff[3][3], 15.0f);  // 16 - 1
    
    // Test addition with identity
    Math::Mat4 identity = glm::mat4(1.0f);
    Math::Mat4 identitySum = a + identity;
    
    // Diagonal elements should be incremented by 1
    EXPECT_NEARLY_EQUAL(identitySum[0][0], 2.0f); // 1 + 1
    EXPECT_NEARLY_EQUAL(identitySum[1][1], 7.0f); // 6 + 1
    EXPECT_NEARLY_EQUAL(identitySum[2][2], 12.0f); // 11 + 1
    EXPECT_NEARLY_EQUAL(identitySum[3][3], 17.0f); // 16 + 1
    
    TestOutput::PrintTestPass("matrix addition and subtraction");
    return true;
}

/**
 * Test matrix multiplication
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixMultiplication() {
    TestOutput::PrintTestStart("matrix multiplication");
    
    // Test identity multiplication
    Math::Mat4 identity = glm::mat4(1.0f);
    Math::Mat4 test(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    
    Math::Mat4 identityResult = test * identity;
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(identityResult, test));
    
    Math::Mat4 identityResult2 = identity * test;
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(identityResult2, test));
    
    // Test simple 2x2 case (using upper-left portion)
    Math::Mat4 a = glm::mat4(1.0f);
    a[0][0] = 1.0f; a[0][1] = 2.0f;  // column 0: [1, 2, 0, 0]
    a[1][0] = 3.0f; a[1][1] = 4.0f;  // column 1: [3, 4, 0, 0]
    
    Math::Mat4 b = glm::mat4(1.0f);
    b[0][0] = 5.0f; b[0][1] = 6.0f;  // column 0: [5, 6, 0, 0]
    b[1][0] = 7.0f; b[1][1] = 8.0f;  // column 1: [7, 8, 0, 0]
    
    Math::Mat4 product = a * b;
    
    // Expected result for 2x2 portion (column-major):
    // Matrix A: [1 3]  Matrix B: [5 7]
    //           [2 4]            [6 8]
    // Result:   [1*5+3*6  1*7+3*8] = [23 31]
    //           [2*5+4*6  2*7+4*8]   [34 46]
    EXPECT_NEARLY_EQUAL(product[0][0], 23.0f); // 1*5 + 3*6
    EXPECT_NEARLY_EQUAL(product[1][0], 31.0f); // 1*7 + 3*8
    EXPECT_NEARLY_EQUAL(product[0][1], 34.0f); // 2*5 + 4*6
    EXPECT_NEARLY_EQUAL(product[1][1], 46.0f); // 2*7 + 4*8
    
    // Test matrix-vector multiplication
    Math::Vec4 vec(1.0f, 2.0f, 3.0f, 1.0f);
    Math::Mat4 transform = glm::mat4(1.0f);
    transform[3][0] = 10.0f; // Translation in X
    transform[3][1] = 20.0f; // Translation in Y
    transform[3][2] = 30.0f; // Translation in Z
    
    Math::Vec4 transformed = transform * vec;
    EXPECT_NEARLY_EQUAL(transformed.x, 11.0f); // 1 + 10
    EXPECT_NEARLY_EQUAL(transformed.y, 22.0f); // 2 + 20
    EXPECT_NEARLY_EQUAL(transformed.z, 33.0f); // 3 + 30
    EXPECT_NEARLY_EQUAL(transformed.w, 1.0f);
    
    TestOutput::PrintTestPass("matrix multiplication");
    return true;
}

/**
 * Test matrix inversion
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixInversion() {
    TestOutput::PrintTestStart("matrix inversion");
    
    // Test identity matrix inversion
    Math::Mat4 identity = glm::mat4(1.0f);
    Math::Mat4 identityInverse = glm::inverse(identity);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(identityInverse, identity));
    
    // Test simple translation matrix inversion
    Math::Mat4 translation = glm::translate(glm::mat4(1.0f), Math::Vec3(10.0f, 20.0f, 30.0f));
    Math::Mat4 translationInverse = glm::inverse(translation);
    
    // Inverse of translation should be negative translation
    EXPECT_NEARLY_EQUAL(translationInverse[3][0], -10.0f);
    EXPECT_NEARLY_EQUAL(translationInverse[3][1], -20.0f);
    EXPECT_NEARLY_EQUAL(translationInverse[3][2], -30.0f);
    
    // Test that matrix * inverse = identity
    Math::Mat4 shouldBeIdentity = translation * translationInverse;
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(shouldBeIdentity, identity, 0.001f));
    
    // Test scale matrix inversion
    Math::Mat4 scale = glm::scale(glm::mat4(1.0f), Math::Vec3(2.0f, 3.0f, 4.0f));
    Math::Mat4 scaleInverse = glm::inverse(scale);
    
    // Inverse of scale should be 1/scale
    EXPECT_NEARLY_EQUAL(scaleInverse[0][0], 0.5f);  // 1/2
    EXPECT_NEARLY_EQUAL(scaleInverse[1][1], 1.0f/3.0f); // 1/3
    EXPECT_NEARLY_EQUAL(scaleInverse[2][2], 0.25f); // 1/4
    
    // Test rotation matrix inversion (transpose should equal inverse for rotation)
    Math::Mat4 rotation = glm::rotate(glm::mat4(1.0f), Math::ToRadians(45.0f), Math::Vec3(0.0f, 0.0f, 1.0f));
    Math::Mat4 rotationInverse = glm::inverse(rotation);
    Math::Mat4 rotationTranspose = glm::transpose(rotation);
    
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(rotationInverse, rotationTranspose, 0.001f));
    
    TestOutput::PrintTestPass("matrix inversion");
    return true;
}

/**
 * Test matrix transpose
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixTranspose() {
    TestOutput::PrintTestStart("matrix transpose");
    
    Math::Mat4 original(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f
    );
    
    Math::Mat4 transposed = glm::transpose(original);
    
    // Check that transpose works correctly (columns become rows)
    // Original matrix column 0: [1, 2, 3, 4] becomes row 0 in transpose
    EXPECT_NEARLY_EQUAL(transposed[0][0], 1.0f);  // was original[0][0]
    EXPECT_NEARLY_EQUAL(transposed[1][0], 2.0f);  // was original[0][1]
    EXPECT_NEARLY_EQUAL(transposed[2][0], 3.0f);  // was original[0][2]
    EXPECT_NEARLY_EQUAL(transposed[3][0], 4.0f);  // was original[0][3]
    
    // Original matrix column 1: [5, 6, 7, 8] becomes row 1 in transpose
    EXPECT_NEARLY_EQUAL(transposed[0][1], 5.0f);  // was original[1][0]
    EXPECT_NEARLY_EQUAL(transposed[1][1], 6.0f);  // was original[1][1]
    EXPECT_NEARLY_EQUAL(transposed[2][1], 7.0f);  // was original[1][2]
    
    // Test double transpose returns original
    Math::Mat4 doubleTransposed = glm::transpose(transposed);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(doubleTransposed, original));
    
    // Test symmetric matrix transpose
    Math::Mat4 symmetric = glm::mat4(1.0f);
    symmetric[1][0] = symmetric[0][1] = 5.0f;
    symmetric[2][0] = symmetric[0][2] = 9.0f;
    
    Math::Mat4 symmetricTransposed = glm::transpose(symmetric);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(symmetricTransposed, symmetric));
    
    TestOutput::PrintTestPass("matrix transpose");
    return true;
}

/**
 * Test matrix determinant calculation
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestMatrixDeterminant() {
    TestOutput::PrintTestStart("matrix determinant");
    
    // Test identity matrix determinant (should be 1)
    Math::Mat4 identity = glm::mat4(1.0f);
    float identityDet = glm::determinant(identity);
    EXPECT_NEARLY_EQUAL(identityDet, 1.0f);
    
    // Test zero matrix determinant (should be 0)
    Math::Mat4 zero(0.0f);
    float zeroDet = glm::determinant(zero);
    EXPECT_NEARLY_EQUAL(zeroDet, 0.0f);
    
    // Test scale matrix determinant (should be product of diagonal elements)
    Math::Mat4 scale = glm::scale(glm::mat4(1.0f), Math::Vec3(2.0f, 3.0f, 4.0f));
    float scaleDet = glm::determinant(scale);
    EXPECT_NEARLY_EQUAL(scaleDet, 24.0f); // 2 * 3 * 4 * 1
    
    // Test that determinant of transpose equals original
    Math::Mat4 test(
        1.0f, 2.0f, 0.0f, 0.0f,
        3.0f, 4.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    
    float originalDet = glm::determinant(test);
    float transposedDet = glm::determinant(glm::transpose(test));
    EXPECT_NEARLY_EQUAL(originalDet, transposedDet);
    
    TestOutput::PrintTestPass("matrix determinant");
    return true;
}

/**
 * Test transformation matrices
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestTransformationMatrices() {
    TestOutput::PrintTestStart("transformation matrices");
    
    // Test translation matrix
    Math::Vec3 translation(10.0f, 20.0f, 30.0f);
    Math::Mat4 translateMat = glm::translate(glm::mat4(1.0f), translation);
    
    Math::Vec4 point(1.0f, 2.0f, 3.0f, 1.0f);
    Math::Vec4 translated = translateMat * point;
    
    EXPECT_NEARLY_EQUAL(translated.x, 11.0f);
    EXPECT_NEARLY_EQUAL(translated.y, 22.0f);
    EXPECT_NEARLY_EQUAL(translated.z, 33.0f);
    EXPECT_NEARLY_EQUAL(translated.w, 1.0f);
    
    // Test scale matrix
    Math::Vec3 scale(2.0f, 3.0f, 4.0f);
    Math::Mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
    
    Math::Vec4 scaled = scaleMat * point;
    EXPECT_NEARLY_EQUAL(scaled.x, 2.0f);  // 1 * 2
    EXPECT_NEARLY_EQUAL(scaled.y, 6.0f);  // 2 * 3
    EXPECT_NEARLY_EQUAL(scaled.z, 12.0f); // 3 * 4
    EXPECT_NEARLY_EQUAL(scaled.w, 1.0f);
    
    // Test rotation matrix (90 degrees around Z-axis)
    Math::Mat4 rotationMat = glm::rotate(glm::mat4(1.0f), Math::ToRadians(90.0f), Math::Vec3(0.0f, 0.0f, 1.0f));
    
    Math::Vec4 rotated = rotationMat * Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotated.x, 0.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotated.y, 1.0f, 0.001f);
    EXPECT_NEARLY_EQUAL_EPSILON(rotated.z, 0.0f, 0.001f);
    
    // Test combined transformation (TRS order)
    Math::Mat4 combined = translateMat * rotationMat * scaleMat;
    Math::Vec4 transformedPoint = combined * point;
    
    // Should first scale, then rotate, then translate
    EXPECT_TRUE(transformedPoint.w == 1.0f);
    
    TestOutput::PrintTestPass("transformation matrices");
    return true;
}

/**
 * Test singular matrix edge cases
 * Requirements: 3.1, 3.4 (Edge case testing)
 */
bool TestSingularMatrices() {
    TestOutput::PrintTestStart("singular matrix edge cases");
    
    // Test matrix with zero row
    Math::Mat4 zeroRow = glm::mat4(1.0f);
    zeroRow[1] = Math::Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float zeroRowDet = glm::determinant(zeroRow);
    EXPECT_NEARLY_EQUAL(zeroRowDet, 0.0f);
    
    // Test matrix with identical rows
    Math::Mat4 identicalRows = glm::mat4(1.0f);
    identicalRows[0] = Math::Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    identicalRows[1] = Math::Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    
    float identicalRowsDet = glm::determinant(identicalRows);
    EXPECT_NEARLY_EQUAL_EPSILON(identicalRowsDet, 0.0f, 0.001f);
    
    // Test matrix with proportional rows
    Math::Mat4 proportionalRows = glm::mat4(1.0f);
    proportionalRows[0] = Math::Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    proportionalRows[1] = Math::Vec4(2.0f, 4.0f, 6.0f, 8.0f); // 2x first row
    
    float proportionalDet = glm::determinant(proportionalRows);
    EXPECT_NEARLY_EQUAL_EPSILON(proportionalDet, 0.0f, 0.001f);
    
    // Test very small determinant (near-singular)
    Math::Mat4 nearSingular = glm::mat4(1.0f);
    nearSingular[0][0] = 1e-10f;
    
    float nearSingularDet = glm::determinant(nearSingular);
    EXPECT_TRUE(std::abs(nearSingularDet) < 1e-9f);
    
    TestOutput::PrintTestPass("singular matrix edge cases");
    return true;
}

/**
 * Test matrix boundary conditions
 * Requirements: 3.1, 3.4 (Boundary condition testing)
 */
bool TestMatrixBoundaryConditions() {
    TestOutput::PrintTestStart("matrix boundary conditions");
    
    // Test very large values
    Math::Mat4 large = glm::mat4(1e6f);
    Math::Mat4 largeInverse = glm::inverse(large);
    
    // Product should be approximately identity
    Math::Mat4 largeProduct = large * largeInverse;
    Math::Mat4 identity = glm::mat4(1.0f);
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(largeProduct, identity, 0.01f));
    
    // Test very small values
    Math::Mat4 small = glm::mat4(1e-6f);
    Math::Mat4 smallInverse = glm::inverse(small);
    
    Math::Mat4 smallProduct = small * smallInverse;
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(smallProduct, identity, 0.01f));
    
    // Test mixed large and small values
    Math::Mat4 mixed = glm::mat4(1.0f);
    mixed[0][0] = 1e6f;
    mixed[1][1] = 1e-6f;
    mixed[2][2] = 1.0f;
    mixed[3][3] = 1.0f;
    
    Math::Mat4 mixedInverse = glm::inverse(mixed);
    Math::Mat4 mixedProduct = mixed * mixedInverse;
    EXPECT_TRUE(FloatComparison::IsNearlyEqual(mixedProduct, identity, 0.01f));
    
    // Test negative values
    Math::Mat4 negative = glm::mat4(-1.0f);
    negative[3][3] = 1.0f; // Keep homogeneous coordinate positive
    
    float negativeDet = glm::determinant(negative);
    EXPECT_TRUE(negativeDet < 0.0f);
    
    TestOutput::PrintTestPass("matrix boundary conditions");
    return true;
}

int main() {
    TestOutput::PrintHeader("Matrix");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Matrix Tests");

        // Run all tests
        allPassed &= suite.RunTest("Matrix Construction", TestMatrixConstruction);
        allPassed &= suite.RunTest("Matrix Addition/Subtraction", TestMatrixAdditionSubtraction);
        allPassed &= suite.RunTest("Matrix Multiplication", TestMatrixMultiplication);
        allPassed &= suite.RunTest("Matrix Inversion", TestMatrixInversion);
        allPassed &= suite.RunTest("Matrix Transpose", TestMatrixTranspose);
        allPassed &= suite.RunTest("Matrix Determinant", TestMatrixDeterminant);
        allPassed &= suite.RunTest("Transformation Matrices", TestTransformationMatrices);
        allPassed &= suite.RunTest("Singular Matrices", TestSingularMatrices);
        allPassed &= suite.RunTest("Matrix Boundary Conditions", TestMatrixBoundaryConditions);

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