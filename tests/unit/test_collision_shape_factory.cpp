#ifdef GAMEENGINE_HAS_BULLET

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Physics/CollisionShapeFactory.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <memory>
#include <random>
#include <chrono>

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

// Parameterized tests for comprehensive shape testing
class CollisionShapeParameterizedTest : public ::testing::TestWithParam<std::tuple<CollisionShape::Type, Math::Vec3, bool>> {
protected:
    void SetUp() override {
        Logger::GetInstance().Initialize("test_collision_shape_factory.log");
        Logger::GetInstance().SetLogLevel(LogLevel::Debug);
    }
    
    void TearDown() override {
        // Cleanup
    }
};

TEST_P(CollisionShapeParameterizedTest, CreateShape_VariousParameters) {
    auto [shapeType, dimensions, shouldSucceed] = GetParam();
    
    CollisionShape desc;
    desc.type = shapeType;
    desc.dimensions = dimensions;
    
    auto shape = CollisionShapeFactory::CreateShape(desc);
    
    if (shouldSucceed) {
        ASSERT_NE(shape, nullptr) << "Shape creation should succeed for type " << static_cast<int>(shapeType);
        
        // Verify shape type matches expected Bullet shape type
        switch (shapeType) {
            case CollisionShape::Box:
                EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
                break;
            case CollisionShape::Sphere:
                EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
                break;
            case CollisionShape::Capsule:
                EXPECT_EQ(shape->getShapeType(), CAPSULE_SHAPE_PROXYTYPE);
                break;
            case CollisionShape::Mesh:
                // Mesh shapes are not implemented, should return null
                EXPECT_EQ(shape, nullptr);
                break;
        }
    } else {
        EXPECT_EQ(shape, nullptr) << "Shape creation should fail for invalid parameters";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ShapeCreationTests,
    CollisionShapeParameterizedTest,
    ::testing::Values(
        // Valid box shapes
        std::make_tuple(CollisionShape::Box, Math::Vec3(1.0f, 1.0f, 1.0f), true),
        std::make_tuple(CollisionShape::Box, Math::Vec3(0.1f, 0.1f, 0.1f), true),
        std::make_tuple(CollisionShape::Box, Math::Vec3(10.0f, 5.0f, 2.0f), true),
        
        // Invalid box shapes
        std::make_tuple(CollisionShape::Box, Math::Vec3(0.0f, 1.0f, 1.0f), false),
        std::make_tuple(CollisionShape::Box, Math::Vec3(-1.0f, 1.0f, 1.0f), false),
        
        // Valid sphere shapes
        std::make_tuple(CollisionShape::Sphere, Math::Vec3(1.0f, 0.0f, 0.0f), true),
        std::make_tuple(CollisionShape::Sphere, Math::Vec3(0.5f, 0.0f, 0.0f), true),
        std::make_tuple(CollisionShape::Sphere, Math::Vec3(100.0f, 0.0f, 0.0f), true),
        
        // Invalid sphere shapes
        std::make_tuple(CollisionShape::Sphere, Math::Vec3(0.0f, 0.0f, 0.0f), false),
        std::make_tuple(CollisionShape::Sphere, Math::Vec3(-1.0f, 0.0f, 0.0f), false),
        
        // Valid capsule shapes
        std::make_tuple(CollisionShape::Capsule, Math::Vec3(1.0f, 2.0f, 0.0f), true),
        std::make_tuple(CollisionShape::Capsule, Math::Vec3(0.5f, 3.0f, 0.0f), true),
        
        // Invalid capsule shapes
        std::make_tuple(CollisionShape::Capsule, Math::Vec3(0.0f, 2.0f, 0.0f), false),
        std::make_tuple(CollisionShape::Capsule, Math::Vec3(1.0f, 0.0f, 0.0f), false),
        
        // Mesh shapes (not implemented)
        std::make_tuple(CollisionShape::Mesh, Math::Vec3(1.0f, 1.0f, 1.0f), false)
    )
);

// Performance tests
TEST_F(CollisionShapeFactoryTest, ShapeCreationPerformance) {
    const int numShapes = 1000;
    std::vector<std::unique_ptr<btCollisionShape>> shapes;
    shapes.reserve(numShapes);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create many shapes
    for (int i = 0; i < numShapes; ++i) {
        CollisionShape desc;
        desc.type = static_cast<CollisionShape::Type>(i % 3); // Box, Sphere, Capsule
        
        switch (desc.type) {
            case CollisionShape::Box:
                desc.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
                break;
            case CollisionShape::Sphere:
                desc.dimensions = Math::Vec3(1.0f, 0.0f, 0.0f);
                break;
            case CollisionShape::Capsule:
                desc.dimensions = Math::Vec3(0.5f, 2.0f, 0.0f);
                break;
            default:
                continue;
        }
        
        auto shape = CollisionShapeFactory::CreateShape(desc);
        if (shape) {
            shapes.push_back(std::move(shape));
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(shapes.size(), numShapes);
    EXPECT_LT(duration.count(), 100) << "Shape creation took too long: " << duration.count() << "ms";
    
    std::cout << "Created " << numShapes << " shapes in " << duration.count() << "ms" << std::endl;
}

// Test using GoogleMock matchers
TEST_F(CollisionShapeFactoryTest, ShapePropertiesWithMatchers) {
    // Test box shape properties
    CollisionShape boxDesc;
    boxDesc.type = CollisionShape::Box;
    boxDesc.dimensions = Math::Vec3(2.0f, 4.0f, 6.0f);
    
    auto boxShape = CollisionShapeFactory::CreateShape(boxDesc);
    ASSERT_NE(boxShape, nullptr);
    
    btBoxShape* box = static_cast<btBoxShape*>(boxShape.get());
    btVector3 halfExtents = box->getHalfExtentsWithMargin();
    
    // Use GoogleMock matchers for more expressive assertions
    EXPECT_THAT(halfExtents.x(), ::testing::FloatNear(1.0f, 0.01f));
    EXPECT_THAT(halfExtents.y(), ::testing::FloatNear(2.0f, 0.01f));
    EXPECT_THAT(halfExtents.z(), ::testing::FloatNear(3.0f, 0.01f));
    
    // Test that all half-extents are positive
    std::vector<float> extents = {halfExtents.x(), halfExtents.y(), halfExtents.z()};
    EXPECT_THAT(extents, ::testing::Each(::testing::Gt(0.0f)));
}

// Memory management tests
TEST_F(CollisionShapeFactoryTest, ShapeMemoryManagement) {
    std::vector<std::unique_ptr<btCollisionShape>> shapes;
    
    // Create shapes and verify they're properly managed
    for (int i = 0; i < 100; ++i) {
        CollisionShape desc;
        desc.type = CollisionShape::Box;
        desc.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
        
        auto shape = CollisionShapeFactory::CreateShape(desc);
        ASSERT_NE(shape, nullptr);
        
        // Verify shape is valid
        EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
        
        shapes.push_back(std::move(shape));
    }
    
    // All shapes should be valid
    EXPECT_EQ(shapes.size(), 100);
    
    // Clear shapes - this should properly deallocate memory
    shapes.clear();
    
    // No memory leaks should occur (verified by external tools like Valgrind)
}

// Stress test with random parameters
TEST_F(CollisionShapeFactoryTest, RandomParameterStressTest) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> sizeDis(0.1f, 10.0f);
    std::uniform_int_distribution<int> typeDis(0, 2); // Box, Sphere, Capsule
    
    int successCount = 0;
    const int numTests = 1000;
    
    for (int i = 0; i < numTests; ++i) {
        CollisionShape desc;
        desc.type = static_cast<CollisionShape::Type>(typeDis(gen));
        
        switch (desc.type) {
            case CollisionShape::Box:
                desc.dimensions = Math::Vec3(sizeDis(gen), sizeDis(gen), sizeDis(gen));
                break;
            case CollisionShape::Sphere:
                desc.dimensions = Math::Vec3(sizeDis(gen), 0.0f, 0.0f);
                break;
            case CollisionShape::Capsule:
                desc.dimensions = Math::Vec3(sizeDis(gen), sizeDis(gen) * 2.0f, 0.0f);
                break;
            default:
                continue;
        }
        
        auto shape = CollisionShapeFactory::CreateShape(desc);
        if (shape) {
            successCount++;
            
            // Verify shape type is correct
            switch (desc.type) {
                case CollisionShape::Box:
                    EXPECT_EQ(shape->getShapeType(), BOX_SHAPE_PROXYTYPE);
                    break;
                case CollisionShape::Sphere:
                    EXPECT_EQ(shape->getShapeType(), SPHERE_SHAPE_PROXYTYPE);
                    break;
                case CollisionShape::Capsule:
                    EXPECT_EQ(shape->getShapeType(), CAPSULE_SHAPE_PROXYTYPE);
                    break;
                default:
                    break;
            }
        }
    }
    
    // Should succeed for most random valid parameters
    EXPECT_GT(successCount, numTests * 0.9) << "Success rate too low: " << successCount << "/" << numTests;
    
    std::cout << "Random parameter test: " << successCount << "/" << numTests << " succeeded" << std::endl;
}

#endif // GAMEENGINE_HAS_BULLET