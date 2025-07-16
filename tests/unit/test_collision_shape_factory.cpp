#ifdef GAMEENGINE_HAS_BULLET

#include <gtest/gtest.h>
#include "Physics/CollisionShapeFactory.h"
#include "Physics/PhysicsEngine.h"
#include <btBulletDynamicsCommon.h>

using namespace GameEngine;
using namespace GameEngine::Physics;

class CollisionShapeFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup common test data
    }

    void TearDown() override {
        // Cleanup
    }
};

// Test Box Shape Creation
TEST_F(CollisionShapeFactoryTest, CreateBoxShape_ValidDimensions_ReturnsValidShape) {
    CollisionShape desc;
    desc.type = CollisionShape::Box;
    desc.dimensions = Math::Vec3(2.0f, 4.0f, 6.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
    
    // Cast to btBoxShape to verify dimensions
    btBoxShape* boxShape = static_cast<btBoxShape*>(shape.get());
    btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();
    
    // Bullet stores half-extents, so our input should be divided by 2
    EXPECT_FLOAT_EQ(halfExtents.x(), 1.0f);
    EXPECT_FLOAT_EQ(halfExtents.y(), 2.0f);
    EXPECT_FLOAT_EQ(halfExtents.z(), 3.0f);
}

TEST_F(CollisionShapeFactoryTest, CreateBoxShape_ZeroDimensions_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Box;
    desc.dimensions = Math::Vec3(0.0f, 1.0f, 1.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

TEST_F(CollisionShapeFactoryTest, CreateBoxShape_NegativeDimensions_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Box;
    desc.dimensions = Math::Vec3(-1.0f, 1.0f, 1.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

// Test Sphere Shape Creation
TEST_F(CollisionShapeFactoryTest, CreateSphereShape_ValidRadius_ReturnsValidShape) {
    CollisionShape desc;
    desc.type = CollisionShape::Sphere;
    desc.dimensions = Math::Vec3(2.5f, 0.0f, 0.0f); // radius in x component

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
    
    // Cast to btSphereShape to verify radius
    btSphereShape* sphereShape = static_cast<btSphereShape*>(shape.get());
    EXPECT_FLOAT_EQ(sphereShape->getRadius(), 2.5f);
}

TEST_F(CollisionShapeFactoryTest, CreateSphereShape_ZeroRadius_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Sphere;
    desc.dimensions = Math::Vec3(0.0f, 0.0f, 0.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

TEST_F(CollisionShapeFactoryTest, CreateSphereShape_NegativeRadius_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Sphere;
    desc.dimensions = Math::Vec3(-1.0f, 0.0f, 0.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

// Test Capsule Shape Creation
TEST_F(CollisionShapeFactoryTest, CreateCapsuleShape_ValidParameters_ReturnsValidShape) {
    CollisionShape desc;
    desc.type = CollisionShape::Capsule;
    desc.dimensions = Math::Vec3(1.0f, 3.0f, 0.0f); // radius in x, height in y

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), CAPSULE_SHAPE_PROXYTYPE);
    
    // Cast to btCapsuleShape to verify parameters
    btCapsuleShape* capsuleShape = static_cast<btCapsuleShape*>(shape.get());
    EXPECT_FLOAT_EQ(capsuleShape->getRadius(), 1.0f);
    EXPECT_FLOAT_EQ(capsuleShape->getHalfHeight(), 1.5f); // Bullet stores half-height
}

TEST_F(CollisionShapeFactoryTest, CreateCapsuleShape_ZeroRadius_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Capsule;
    desc.dimensions = Math::Vec3(0.0f, 3.0f, 0.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

TEST_F(CollisionShapeFactoryTest, CreateCapsuleShape_ZeroHeight_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Capsule;
    desc.dimensions = Math::Vec3(1.0f, 0.0f, 0.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

// Test Mesh Shape (not implemented)
TEST_F(CollisionShapeFactoryTest, CreateMeshShape_ReturnsNull) {
    CollisionShape desc;
    desc.type = CollisionShape::Mesh;
    desc.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    EXPECT_EQ(shape, nullptr);
}

// Test Edge Cases
TEST_F(CollisionShapeFactoryTest, CreateShape_VerySmallDimensions_ReturnsValidShape) {
    CollisionShape desc;
    desc.type = CollisionShape::Box;
    desc.dimensions = Math::Vec3(0.001f, 0.001f, 0.001f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
}

TEST_F(CollisionShapeFactoryTest, CreateShape_VeryLargeDimensions_ReturnsValidShape) {
    CollisionShape desc;
    desc.type = CollisionShape::Sphere;
    desc.dimensions = Math::Vec3(1000.0f, 0.0f, 0.0f);

    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
}

// Test Direct Static Method Calls (as per task requirements)
TEST_F(CollisionShapeFactoryTest, CreateBoxShape_DirectCall_ValidDimensions) {
    Math::Vec3 dimensions(4.0f, 6.0f, 8.0f);
    
    auto shape = CollisionShapeFactory::CreateBoxShape(dimensions);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
    
    btVector3 halfExtents = shape->getHalfExtentsWithMargin();
    EXPECT_FLOAT_EQ(halfExtents.x(), 2.0f);
    EXPECT_FLOAT_EQ(halfExtents.y(), 3.0f);
    EXPECT_FLOAT_EQ(halfExtents.z(), 4.0f);
}

TEST_F(CollisionShapeFactoryTest, CreateSphereShape_DirectCall_ValidRadius) {
    float radius = 3.5f;
    
    auto shape = CollisionShapeFactory::CreateSphereShape(radius);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
    EXPECT_FLOAT_EQ(shape->getRadius(), 3.5f);
}

TEST_F(CollisionShapeFactoryTest, CreateCapsuleShape_DirectCall_ValidParameters) {
    float radius = 1.5f;
    float height = 4.0f;
    
    auto shape = CollisionShapeFactory::CreateCapsuleShape(radius, height);
    
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), CAPSULE_SHAPE_PROXYTYPE);
    EXPECT_FLOAT_EQ(shape->getRadius(), 1.5f);
    EXPECT_FLOAT_EQ(shape->getHalfHeight(), 2.0f); // Bullet stores half-height
}

TEST_F(CollisionShapeFactoryTest, CreateBoxShape_DirectCall_ZeroDimensions) {
    Math::Vec3 dimensions(0.0f, 1.0f, 1.0f);
    
    auto shape = CollisionShapeFactory::CreateBoxShape(dimensions);
    
    // Should still create shape but with zero dimension (Bullet handles this)
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
}

TEST_F(CollisionShapeFactoryTest, CreateSphereShape_DirectCall_ZeroRadius) {
    float radius = 0.0f;
    
    auto shape = CollisionShapeFactory::CreateSphereShape(radius);
    
    // Should still create shape but with zero radius (Bullet handles this)
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
}

TEST_F(CollisionShapeFactoryTest, CreateCapsuleShape_DirectCall_ZeroParameters) {
    float radius = 0.0f;
    float height = 0.0f;
    
    auto shape = CollisionShapeFactory::CreateCapsuleShape(radius, height);
    
    // Should still create shape but with zero parameters (Bullet handles this)
    ASSERT_NE(shape, nullptr);
    EXPECT_EQ(shape->getShapeType(), CAPSULE_SHAPE_PROXYTYPE);
}

#endif // GAMEENGINE_HAS_BULLET