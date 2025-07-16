#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletUtils.h"
#include <iostream>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Physics;

bool FloatEqual(float a, float b, float tolerance = 1e-6f) {
    return std::abs(a - b) < tolerance;
}

bool Vec3Equal(const Math::Vec3& a, const Math::Vec3& b, float tolerance = 1e-6f) {
    return FloatEqual(a.x, b.x, tolerance) && 
           FloatEqual(a.y, b.y, tolerance) && 
           FloatEqual(a.z, b.z, tolerance);
}

bool QuatEqual(const Math::Quat& a, const Math::Quat& b, float tolerance = 1e-6f) {
    return FloatEqual(a.x, b.x, tolerance) && 
           FloatEqual(a.y, b.y, tolerance) && 
           FloatEqual(a.z, b.z, tolerance) && 
           FloatEqual(a.w, b.w, tolerance);
}

int main() {
    std::cout << "Testing Bullet Physics conversion utilities (simple tests)..." << std::endl;
    
    int testsPassed = 0;
    int totalTests = 0;
    
    // Test Vec3 to Bullet conversion
    {
        totalTests++;
        Math::Vec3 engineVec(1.0f, 2.0f, 3.0f);
        btVector3 bulletVec = BulletUtils::ToBullet(engineVec);
        
        bool success = FloatEqual(bulletVec.getX(), 1.0f) &&
                      FloatEqual(bulletVec.getY(), 2.0f) &&
                      FloatEqual(bulletVec.getZ(), 3.0f);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Vec3 to Bullet conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Vec3 to Bullet conversion: FAIL" << std::endl;
        }
    }
    
    // Test Vec3 from Bullet conversion
    {
        totalTests++;
        btVector3 bulletVec(4.0f, 5.0f, 6.0f);
        Math::Vec3 engineVec = BulletUtils::FromBullet(bulletVec);
        
        bool success = FloatEqual(engineVec.x, 4.0f) &&
                      FloatEqual(engineVec.y, 5.0f) &&
                      FloatEqual(engineVec.z, 6.0f);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Vec3 from Bullet conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Vec3 from Bullet conversion: FAIL" << std::endl;
        }
    }
    
    // Test Vec3 round-trip conversion
    {
        totalTests++;
        Math::Vec3 original(7.5f, -2.3f, 0.0f);
        btVector3 bulletVec = BulletUtils::ToBullet(original);
        Math::Vec3 converted = BulletUtils::FromBullet(bulletVec);
        
        bool success = Vec3Equal(original, converted);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Vec3 round-trip conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Vec3 round-trip conversion: FAIL" << std::endl;
        }
    }
    
    // Test Quaternion to Bullet conversion
    {
        totalTests++;
        Math::Quat engineQuat(0.707f, 0.0f, 0.707f, 0.0f); // 90 degree rotation around Y
        btQuaternion bulletQuat = BulletUtils::ToBullet(engineQuat);
        
        bool success = FloatEqual(bulletQuat.getX(), 0.0f) &&
                      FloatEqual(bulletQuat.getY(), 0.707f) &&
                      FloatEqual(bulletQuat.getZ(), 0.0f) &&
                      FloatEqual(bulletQuat.getW(), 0.707f);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Quaternion to Bullet conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Quaternion to Bullet conversion: FAIL" << std::endl;
        }
    }
    
    // Test Quaternion from Bullet conversion
    {
        totalTests++;
        btQuaternion bulletQuat(0.5f, 0.5f, 0.5f, 0.5f);
        Math::Quat engineQuat = BulletUtils::FromBullet(bulletQuat);
        
        bool success = FloatEqual(engineQuat.x, 0.5f) &&
                      FloatEqual(engineQuat.y, 0.5f) &&
                      FloatEqual(engineQuat.z, 0.5f) &&
                      FloatEqual(engineQuat.w, 0.5f);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Quaternion from Bullet conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Quaternion from Bullet conversion: FAIL" << std::endl;
        }
    }
    
    // Test Quaternion round-trip conversion
    {
        totalTests++;
        Math::Quat original(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        btQuaternion bulletQuat = BulletUtils::ToBullet(original);
        Math::Quat converted = BulletUtils::FromBullet(bulletQuat);
        
        bool success = QuatEqual(original, converted);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Quaternion round-trip conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Quaternion round-trip conversion: FAIL" << std::endl;
        }
    }
    
    // Test zero vector conversion
    {
        totalTests++;
        Math::Vec3 zero(0.0f, 0.0f, 0.0f);
        btVector3 bulletZero = BulletUtils::ToBullet(zero);
        Math::Vec3 convertedZero = BulletUtils::FromBullet(bulletZero);
        
        bool success = Vec3Equal(zero, convertedZero);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Zero vector conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Zero vector conversion: FAIL" << std::endl;
        }
    }
    
    // Test negative values conversion
    {
        totalTests++;
        Math::Vec3 negative(-1.0f, -2.0f, -3.0f);
        btVector3 bulletNegative = BulletUtils::ToBullet(negative);
        Math::Vec3 convertedNegative = BulletUtils::FromBullet(bulletNegative);
        
        bool success = Vec3Equal(negative, convertedNegative);
        
        if (success) {
            testsPassed++;
            std::cout << "✓ Negative values conversion: PASS" << std::endl;
        } else {
            std::cout << "✗ Negative values conversion: FAIL" << std::endl;
        }
    }
    
    std::cout << std::endl;
    std::cout << "Test Results: " << testsPassed << "/" << totalTests << " tests passed" << std::endl;
    
    if (testsPassed == totalTests) {
        std::cout << "All Bullet Physics conversion utility tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests failed!" << std::endl;
        return 1;
    }
}

#else

#include <iostream>

int main() {
    std::cout << "Bullet Physics not available - skipping conversion utility tests" << std::endl;
    return 0;
}

#endif // GAMEENGINE_HAS_BULLET