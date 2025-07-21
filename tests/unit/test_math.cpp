#include <iostream>
#include <cassert>
#include <cmath>
#include "Core/Math.h"

using namespace GameEngine;

// Helper function for floating point comparison
bool IsNearlyEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

// Test basic vector operations
bool TestVectorOperations() {
    std::cout << "Testing vector operations..." << std::endl;
    
    // Test vector addition
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 result = a + b;
    
    assert(IsNearlyEqual(result.x, 5.0f));
    assert(IsNearlyEqual(result.y, 7.0f));
    assert(IsNearlyEqual(result.z, 9.0f));
    
    // Test vector subtraction
    Math::Vec3 diff = b - a;
    assert(IsNearlyEqual(diff.x, 3.0f));
    assert(IsNearlyEqual(diff.y, 3.0f));
    assert(IsNearlyEqual(diff.z, 3.0f));
    
    // Test vector length
    Math::Vec3 unit(1.0f, 0.0f, 0.0f);
    float length = glm::length(unit);
    assert(IsNearlyEqual(length, 1.0f));
    
    std::cout << "  ✅ Vector operations passed" << std::endl;
    return true;
}

// Test angle conversion functions
bool TestAngleConversion() {
    std::cout << "Testing angle conversions..." << std::endl;
    
    // Test degrees to radians
    float degrees = 90.0f;
    float radians = Math::ToRadians(degrees);
    assert(IsNearlyEqual(radians, Math::HALF_PI));
    
    // Test radians to degrees
    float backToDegrees = Math::ToDegrees(radians);
    assert(IsNearlyEqual(backToDegrees, 90.0f));
    
    // Test constants
    assert(IsNearlyEqual(Math::PI, 3.14159265359f));
    assert(IsNearlyEqual(Math::TWO_PI, 2.0f * Math::PI));
    
    std::cout << "  ✅ Angle conversion passed" << std::endl;
    return true;
}

// Test linear interpolation
bool TestLerp() {
    std::cout << "Testing linear interpolation..." << std::endl;
    
    // Test float lerp
    float result = Math::Lerp(0.0f, 10.0f, 0.5f);
    assert(IsNearlyEqual(result, 5.0f));
    
    // Test vector lerp
    Math::Vec3 start(0.0f, 0.0f, 0.0f);
    Math::Vec3 end(10.0f, 20.0f, 30.0f);
    Math::Vec3 mid = Math::Lerp(start, end, 0.5f);
    
    assert(IsNearlyEqual(mid.x, 5.0f));
    assert(IsNearlyEqual(mid.y, 10.0f));
    assert(IsNearlyEqual(mid.z, 15.0f));
    
    std::cout << "  ✅ Linear interpolation passed" << std::endl;
    return true;
}

// Test clamping
bool TestClamp() {
    std::cout << "Testing clamping..." << std::endl;
    
    // Test float clamp
    assert(IsNearlyEqual(Math::Clamp(5.0f, 0.0f, 10.0f), 5.0f));
    assert(IsNearlyEqual(Math::Clamp(-5.0f, 0.0f, 10.0f), 0.0f));
    assert(IsNearlyEqual(Math::Clamp(15.0f, 0.0f, 10.0f), 10.0f));
    
    std::cout << "  ✅ Clamping passed" << std::endl;
    return true;
}

// Test matrix creation
bool TestMatrixCreation() {
    std::cout << "Testing matrix creation..." << std::endl;
    
    // Test transform matrix creation
    Math::Vec3 position(1.0f, 2.0f, 3.0f);
    Math::Quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    Math::Vec3 scale(1.0f, 1.0f, 1.0f);
    
    Math::Mat4 transform = Math::CreateTransform(position, rotation, scale);
    
    // Test that the matrix is not zero (basic sanity check)
    assert(transform[3][0] != 0.0f || transform[3][1] != 0.0f || transform[3][2] != 0.0f);
    
    // Test perspective matrix creation
    Math::Mat4 perspective = Math::CreatePerspectiveMatrix(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
    assert(perspective[0][0] != 0.0f); // Basic sanity check
    
    std::cout << "  ✅ Matrix creation passed" << std::endl;
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Game Engine Kiro - Math Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool allPassed = true;
    
    try {
        allPassed &= TestVectorOperations();
        allPassed &= TestAngleConversion();
        allPassed &= TestLerp();
        allPassed &= TestClamp();
        allPassed &= TestMatrixCreation();
        
        std::cout << "========================================" << std::endl;
        if (allPassed) {
            std::cout << "🎉 ALL MATH TESTS PASSED!" << std::endl;
            std::cout << "========================================" << std::endl;
            return 0;
        } else {
            std::cout << "❌ SOME TESTS FAILED!" << std::endl;
            std::cout << "========================================" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ TEST EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ UNKNOWN TEST ERROR!" << std::endl;
        return 1;
    }
}