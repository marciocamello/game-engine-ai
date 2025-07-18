#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletUtils.h"
#include "Core/Logger.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <limits>

using namespace GameEngine;
using namespace GameEngine::Physics;

class BulletUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test data
        testVec3 = Math::Vec3(1.5f, -2.3f, 4.7f);
        testQuat = glm::angleAxis(glm::radians(45.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
        
        // Epsilon for floating point comparisons
        epsilon = 1e-6f;
    }
    
    Math::Vec3 testVec3;
    Math::Quat testQuat;
    float epsilon;
};

// Vec3 conversion tests
TEST_F(BulletUtilsTest, Vec3ToBulletConversion) {
    btVector3 bulletVec = BulletUtils::ToBullet(testVec3);
    
    EXPECT_NEAR(bulletVec.getX(), testVec3.x, epsilon);
    EXPECT_NEAR(bulletVec.getY(), testVec3.y, epsilon);
    EXPECT_NEAR(bulletVec.getZ(), testVec3.z, epsilon);
}

TEST_F(BulletUtilsTest, Vec3FromBulletConversion) {
    btVector3 bulletVec(1.5f, -2.3f, 4.7f);
    Math::Vec3 glmVec = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_NEAR(glmVec.x, bulletVec.getX(), epsilon);
    EXPECT_NEAR(glmVec.y, bulletVec.getY(), epsilon);
    EXPECT_NEAR(glmVec.z, bulletVec.getZ(), epsilon);
}

TEST_F(BulletUtilsTest, Vec3RoundTripConversion) {
    // GLM -> Bullet -> GLM
    btVector3 bulletVec = BulletUtils::ToBullet(testVec3);
    Math::Vec3 roundTripVec = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_NEAR(roundTripVec.x, testVec3.x, epsilon);
    EXPECT_NEAR(roundTripVec.y, testVec3.y, epsilon);
    EXPECT_NEAR(roundTripVec.z, testVec3.z, epsilon);
    
    // Bullet -> GLM -> Bullet
    btVector3 originalBullet(3.2f, -1.8f, 0.9f);
    Math::Vec3 glmVec = BulletUtils::FromBullet(originalBullet);
    btVector3 roundTripBullet = BulletUtils::ToBullet(glmVec);
    
    EXPECT_NEAR(roundTripBullet.getX(), originalBullet.getX(), epsilon);
    EXPECT_NEAR(roundTripBullet.getY(), originalBullet.getY(), epsilon);
    EXPECT_NEAR(roundTripBullet.getZ(), originalBullet.getZ(), epsilon);
}

// Quaternion conversion tests
TEST_F(BulletUtilsTest, QuatToBulletConversion) {
    btQuaternion bulletQuat = BulletUtils::ToBullet(testQuat);
    
    EXPECT_NEAR(bulletQuat.getX(), testQuat.x, epsilon);
    EXPECT_NEAR(bulletQuat.getY(), testQuat.y, epsilon);
    EXPECT_NEAR(bulletQuat.getZ(), testQuat.z, epsilon);
    EXPECT_NEAR(bulletQuat.getW(), testQuat.w, epsilon);
}

TEST_F(BulletUtilsTest, QuatFromBulletConversion) {
    btQuaternion bulletQuat(0.1f, 0.2f, 0.3f, 0.9f);
    bulletQuat.normalize(); // Ensure normalized quaternion
    Math::Quat glmQuat = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_NEAR(glmQuat.x, bulletQuat.getX(), epsilon);
    EXPECT_NEAR(glmQuat.y, bulletQuat.getY(), epsilon);
    EXPECT_NEAR(glmQuat.z, bulletQuat.getZ(), epsilon);
    EXPECT_NEAR(glmQuat.w, bulletQuat.getW(), epsilon);
}

TEST_F(BulletUtilsTest, QuatRoundTripConversion) {
    // Normalize test quaternion
    Math::Quat normalizedQuat = glm::normalize(testQuat);
    
    // GLM -> Bullet -> GLM
    btQuaternion bulletQuat = BulletUtils::ToBullet(normalizedQuat);
    Math::Quat roundTripQuat = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_NEAR(roundTripQuat.x, normalizedQuat.x, epsilon);
    EXPECT_NEAR(roundTripQuat.y, normalizedQuat.y, epsilon);
    EXPECT_NEAR(roundTripQuat.z, normalizedQuat.z, epsilon);
    EXPECT_NEAR(roundTripQuat.w, normalizedQuat.w, epsilon);
    
    // Bullet -> GLM -> Bullet
    btQuaternion originalBullet(0.5f, 0.5f, 0.5f, 0.5f);
    originalBullet.normalize();
    Math::Quat glmQuat = BulletUtils::FromBullet(originalBullet);
    btQuaternion roundTripBullet = BulletUtils::ToBullet(glmQuat);
    
    EXPECT_NEAR(roundTripBullet.getX(), originalBullet.getX(), epsilon);
    EXPECT_NEAR(roundTripBullet.getY(), originalBullet.getY(), epsilon);
    EXPECT_NEAR(roundTripBullet.getZ(), originalBullet.getZ(), epsilon);
    EXPECT_NEAR(roundTripBullet.getW(), originalBullet.getW(), epsilon);
}

// Transform conversion tests
TEST_F(BulletUtilsTest, TransformConversion) {
    Math::Vec3 position(1.0f, 2.0f, 3.0f);
    Math::Quat rotation = glm::normalize(glm::angleAxis(glm::radians(90.0f), Math::Vec3(0.0f, 0.0f, 1.0f)));
    
    btTransform bulletTransform = BulletUtils::ToBullet(position, rotation);
    
    Math::Vec3 extractedPos;
    Math::Quat extractedRot;
    BulletUtils::FromBullet(bulletTransform, extractedPos, extractedRot);
    
    EXPECT_NEAR(extractedPos.x, position.x, epsilon);
    EXPECT_NEAR(extractedPos.y, position.y, epsilon);
    EXPECT_NEAR(extractedPos.z, position.z, epsilon);
    
    EXPECT_NEAR(extractedRot.x, rotation.x, epsilon);
    EXPECT_NEAR(extractedRot.y, rotation.y, epsilon);
    EXPECT_NEAR(extractedRot.z, rotation.z, epsilon);
    EXPECT_NEAR(extractedRot.w, rotation.w, epsilon);
}

// Matrix conversion tests
TEST_F(BulletUtilsTest, MatrixConversion) {
    Math::Vec3 position(2.0f, 3.0f, 4.0f);
    Math::Quat rotation = glm::normalize(glm::angleAxis(glm::radians(30.0f), Math::Vec3(1.0f, 0.0f, 0.0f)));
    Math::Vec3 scale(1.0f, 1.0f, 1.0f);
    
    Math::Mat4 originalMatrix = Math::CreateTransform(position, rotation, scale);
    
    btTransform bulletTransform = BulletUtils::ToBullet(originalMatrix);
    Math::Mat4 roundTripMatrix = BulletUtils::FromBullet(bulletTransform);
    
    // Compare matrices element by element (with some tolerance for floating point precision)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(roundTripMatrix[i][j], originalMatrix[i][j], epsilon * 10.0f) 
                << "Matrix element [" << i << "][" << j << "] differs";
        }
    }
}

// Edge case tests
TEST_F(BulletUtilsTest, ZeroVectorConversion) {
    Math::Vec3 zeroVec(0.0f, 0.0f, 0.0f);
    btVector3 bulletZero = BulletUtils::ToBullet(zeroVec);
    Math::Vec3 roundTripZero = BulletUtils::FromBullet(bulletZero);
    
    EXPECT_NEAR(roundTripZero.x, 0.0f, epsilon);
    EXPECT_NEAR(roundTripZero.y, 0.0f, epsilon);
    EXPECT_NEAR(roundTripZero.z, 0.0f, epsilon);
}

TEST_F(BulletUtilsTest, IdentityQuaternionConversion) {
    Math::Quat identityQuat(1.0f, 0.0f, 0.0f, 0.0f); // GLM quaternion constructor: w, x, y, z
    btQuaternion bulletIdentity = BulletUtils::ToBullet(identityQuat);
    Math::Quat roundTripIdentity = BulletUtils::FromBullet(bulletIdentity);
    
    EXPECT_NEAR(roundTripIdentity.w, 1.0f, epsilon);
    EXPECT_NEAR(roundTripIdentity.x, 0.0f, epsilon);
    EXPECT_NEAR(roundTripIdentity.y, 0.0f, epsilon);
    EXPECT_NEAR(roundTripIdentity.z, 0.0f, epsilon);
}

// Parameterized tests for comprehensive coverage
class BulletUtilsParameterizedTest : public ::testing::TestWithParam<Math::Vec3> {
protected:
    void SetUp() override {
        epsilon = 1e-6f;
    }
    
    float epsilon;
};

TEST_P(BulletUtilsParameterizedTest, Vec3ConversionConsistency) {
    Math::Vec3 testVec = GetParam();
    
    // Test round-trip conversion
    btVector3 bulletVec = BulletUtils::ToBullet(testVec);
    Math::Vec3 roundTripVec = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_NEAR(roundTripVec.x, testVec.x, epsilon);
    EXPECT_NEAR(roundTripVec.y, testVec.y, epsilon);
    EXPECT_NEAR(roundTripVec.z, testVec.z, epsilon);
}

INSTANTIATE_TEST_SUITE_P(
    VectorTestCases,
    BulletUtilsParameterizedTest,
    ::testing::Values(
        Math::Vec3(0.0f, 0.0f, 0.0f),      // Zero vector
        Math::Vec3(1.0f, 1.0f, 1.0f),      // Unit vector
        Math::Vec3(-1.0f, -1.0f, -1.0f),   // Negative unit vector
        Math::Vec3(100.0f, -50.0f, 25.0f), // Large values
        Math::Vec3(0.001f, 0.002f, 0.003f), // Small values
        Math::Vec3(std::numeric_limits<float>::max() * 0.1f, 0.0f, 0.0f), // Large positive
        Math::Vec3(std::numeric_limits<float>::lowest() * 0.1f, 0.0f, 0.0f) // Large negative
    )
);

// Performance and stress tests
TEST_F(BulletUtilsTest, ConversionPerformanceTest) {
    const int numIterations = 10000;
    std::vector<Math::Vec3> testVectors;
    std::vector<Math::Quat> testQuaternions;
    
    // Generate random test data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-100.0f, 100.0f);
    
    for (int i = 0; i < numIterations; ++i) {
        testVectors.emplace_back(dis(gen), dis(gen), dis(gen));
        
        // Generate normalized quaternions
        Math::Quat q(dis(gen), dis(gen), dis(gen), dis(gen));
        testQuaternions.push_back(glm::normalize(q));
    }
    
    // Time vector conversions
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& vec : testVectors) {
        btVector3 bulletVec = BulletUtils::ToBullet(vec);
        Math::Vec3 roundTrip = BulletUtils::FromBullet(bulletVec);
        // Prevent optimization from removing the computation
        volatile float dummy = roundTrip.x + roundTrip.y + roundTrip.z;
        (void)dummy;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto vectorDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Time quaternion conversions
    start = std::chrono::high_resolution_clock::now();
    
    for (const auto& quat : testQuaternions) {
        btQuaternion bulletQuat = BulletUtils::ToBullet(quat);
        Math::Quat roundTrip = BulletUtils::FromBullet(bulletQuat);
        // Prevent optimization from removing the computation
        volatile float dummy = roundTrip.x + roundTrip.y + roundTrip.z + roundTrip.w;
        (void)dummy;
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto quatDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Performance should be reasonable (less than 1ms per 1000 conversions)
    EXPECT_LT(vectorDuration.count(), 1000) << "Vector conversions took too long: " << vectorDuration.count() << "μs";
    EXPECT_LT(quatDuration.count(), 1000) << "Quaternion conversions took too long: " << quatDuration.count() << "μs";
    
    std::cout << "Vector conversion performance: " << vectorDuration.count() << "μs for " << numIterations << " conversions\n";
    std::cout << "Quaternion conversion performance: " << quatDuration.count() << "μs for " << numIterations << " conversions\n";
}

// Test with extreme values
TEST_F(BulletUtilsTest, ExtremeValueHandling) {
    // Test with very large values
    Math::Vec3 largeVec(1e6f, -1e6f, 1e6f);
    btVector3 bulletLarge = BulletUtils::ToBullet(largeVec);
    Math::Vec3 roundTripLarge = BulletUtils::FromBullet(bulletLarge);
    
    EXPECT_NEAR(roundTripLarge.x, largeVec.x, largeVec.x * 1e-6f);
    EXPECT_NEAR(roundTripLarge.y, largeVec.y, std::abs(largeVec.y) * 1e-6f);
    EXPECT_NEAR(roundTripLarge.z, largeVec.z, largeVec.z * 1e-6f);
    
    // Test with very small values
    Math::Vec3 smallVec(1e-6f, -1e-6f, 1e-6f);
    btVector3 bulletSmall = BulletUtils::ToBullet(smallVec);
    Math::Vec3 roundTripSmall = BulletUtils::FromBullet(bulletSmall);
    
    EXPECT_NEAR(roundTripSmall.x, smallVec.x, 1e-9f);
    EXPECT_NEAR(roundTripSmall.y, smallVec.y, 1e-9f);
    EXPECT_NEAR(roundTripSmall.z, smallVec.z, 1e-9f);
}

// Test quaternion normalization consistency
TEST_F(BulletUtilsTest, QuaternionNormalizationConsistency) {
    // Create an unnormalized quaternion
    Math::Quat unnormalizedQuat(2.0f, 1.0f, 1.0f, 1.0f);
    Math::Quat normalizedQuat = glm::normalize(unnormalizedQuat);
    
    // Convert both to Bullet and back
    btQuaternion bulletUnnormalized = BulletUtils::ToBullet(unnormalizedQuat);
    btQuaternion bulletNormalized = BulletUtils::ToBullet(normalizedQuat);
    
    // Bullet should normalize automatically
    EXPECT_NEAR(bulletUnnormalized.length(), 1.0f, epsilon);
    EXPECT_NEAR(bulletNormalized.length(), 1.0f, epsilon);
    
    Math::Quat roundTripUnnormalized = BulletUtils::FromBullet(bulletUnnormalized);
    Math::Quat roundTripNormalized = BulletUtils::FromBullet(bulletNormalized);
    
    // Both should be normalized after round trip
    float lengthUnnormalized = glm::length(roundTripUnnormalized);
    float lengthNormalized = glm::length(roundTripNormalized);
    
    EXPECT_NEAR(lengthUnnormalized, 1.0f, epsilon);
    EXPECT_NEAR(lengthNormalized, 1.0f, epsilon);
}

// Test using GoogleMock matchers for more expressive assertions
TEST_F(BulletUtilsTest, ConversionWithMatchers) {
    Math::Vec3 testVec(1.5f, -2.3f, 4.7f);
    btVector3 bulletVec = BulletUtils::ToBullet(testVec);
    
    // Use GoogleMock matchers for more expressive tests
    EXPECT_THAT(bulletVec.getX(), ::testing::FloatNear(testVec.x, epsilon));
    EXPECT_THAT(bulletVec.getY(), ::testing::FloatNear(testVec.y, epsilon));
    EXPECT_THAT(bulletVec.getZ(), ::testing::FloatNear(testVec.z, epsilon));
    
    // Test that all components are within expected range
    std::vector<float> components = {bulletVec.getX(), bulletVec.getY(), bulletVec.getZ()};
    EXPECT_THAT(components, ::testing::Each(::testing::AllOf(
        ::testing::Ge(-10.0f),
        ::testing::Le(10.0f)
    )));
}

#endif // GAMEENGINE_HAS_BULLET