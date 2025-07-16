#include <gtest/gtest.h>

#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/BulletUtils.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>

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

#endif // GAMEENGINE_HAS_BULLET