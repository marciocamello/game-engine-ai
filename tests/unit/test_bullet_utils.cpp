#ifdef GAMEENGINE_HAS_BULLET

#include <gtest/gtest.h>
#include "Physics/BulletUtils.h"
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Physics;

class BulletUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
    
    // Helper function to compare floats with tolerance
    bool FloatEqual(float a, float b, float tolerance = 1e-6f) {
        return std::abs(a - b) < tolerance;
    }
    
    // Helper function to compare Vec3 with tolerance
    bool Vec3Equal(const Math::Vec3& a, const Math::Vec3& b, float tolerance = 1e-6f) {
        return FloatEqual(a.x, b.x, tolerance) && 
               FloatEqual(a.y, b.y, tolerance) && 
               FloatEqual(a.z, b.z, tolerance);
    }
    
    // Helper function to compare quaternions with tolerance
    bool QuatEqual(const Math::Quat& a, const Math::Quat& b, float tolerance = 1e-6f) {
        return FloatEqual(a.x, b.x, tolerance) && 
               FloatEqual(a.y, b.y, tolerance) && 
               FloatEqual(a.z, b.z, tolerance) && 
               FloatEqual(a.w, b.w, tolerance);
    }
};

TEST_F(BulletUtilsTest, Vec3ToBulletConversion) {
    Math::Vec3 engineVec(1.0f, 2.0f, 3.0f);
    btVector3 bulletVec = BulletUtils::ToBullet(engineVec);
    
    EXPECT_FLOAT_EQ(bulletVec.getX(), 1.0f);
    EXPECT_FLOAT_EQ(bulletVec.getY(), 2.0f);
    EXPECT_FLOAT_EQ(bulletVec.getZ(), 3.0f);
}

TEST_F(BulletUtilsTest, Vec3FromBulletConversion) {
    btVector3 bulletVec(4.0f, 5.0f, 6.0f);
    Math::Vec3 engineVec = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_FLOAT_EQ(engineVec.x, 4.0f);
    EXPECT_FLOAT_EQ(engineVec.y, 5.0f);
    EXPECT_FLOAT_EQ(engineVec.z, 6.0f);
}

TEST_F(BulletUtilsTest, Vec3RoundTripConversion) {
    Math::Vec3 original(7.5f, -2.3f, 0.0f);
    btVector3 bulletVec = BulletUtils::ToBullet(original);
    Math::Vec3 converted = BulletUtils::FromBullet(bulletVec);
    
    EXPECT_TRUE(Vec3Equal(original, converted));
}

TEST_F(BulletUtilsTest, QuatToBulletConversion) {
    Math::Quat engineQuat(0.707f, 0.0f, 0.707f, 0.0f); // 90 degree rotation around Y
    btQuaternion bulletQuat = BulletUtils::ToBullet(engineQuat);
    
    EXPECT_FLOAT_EQ(bulletQuat.getX(), 0.0f);
    EXPECT_FLOAT_EQ(bulletQuat.getY(), 0.707f);
    EXPECT_FLOAT_EQ(bulletQuat.getZ(), 0.0f);
    EXPECT_FLOAT_EQ(bulletQuat.getW(), 0.707f);
}

TEST_F(BulletUtilsTest, QuatFromBulletConversion) {
    btQuaternion bulletQuat(0.5f, 0.5f, 0.5f, 0.5f);
    Math::Quat engineQuat = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_FLOAT_EQ(engineQuat.x, 0.5f);
    EXPECT_FLOAT_EQ(engineQuat.y, 0.5f);
    EXPECT_FLOAT_EQ(engineQuat.z, 0.5f);
    EXPECT_FLOAT_EQ(engineQuat.w, 0.5f);
}

TEST_F(BulletUtilsTest, QuatRoundTripConversion) {
    Math::Quat original(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
    btQuaternion bulletQuat = BulletUtils::ToBullet(original);
    Math::Quat converted = BulletUtils::FromBullet(bulletQuat);
    
    EXPECT_TRUE(QuatEqual(original, converted));
}

TEST_F(BulletUtilsTest, ZeroVectorConversion) {
    Math::Vec3 zero(0.0f, 0.0f, 0.0f);
    btVector3 bulletZero = BulletUtils::ToBullet(zero);
    Math::Vec3 convertedZero = BulletUtils::FromBullet(bulletZero);
    
    EXPECT_TRUE(Vec3Equal(zero, convertedZero));
}

TEST_F(BulletUtilsTest, IdentityQuaternionConversion) {
    Math::Quat identity(1.0f, 0.0f, 0.0f, 0.0f);
    btQuaternion bulletIdentity = BulletUtils::ToBullet(identity);
    Math::Quat convertedIdentity = BulletUtils::FromBullet(bulletIdentity);
    
    EXPECT_TRUE(QuatEqual(identity, convertedIdentity));
}

TEST_F(BulletUtilsTest, NegativeValuesConversion) {
    Math::Vec3 negative(-1.0f, -2.0f, -3.0f);
    btVector3 bulletNegative = BulletUtils::ToBullet(negative);
    Math::Vec3 convertedNegative = BulletUtils::FromBullet(bulletNegative);
    
    EXPECT_TRUE(Vec3Equal(negative, convertedNegative));
}

#endif // GAMEENGINE_HAS_BULLET