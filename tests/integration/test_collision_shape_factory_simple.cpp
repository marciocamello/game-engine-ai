#ifdef GAMEENGINE_HAS_BULLET

#include "Physics/CollisionShapeFactory.h"
#include "Physics/PhysicsEngine.h"
#include "../TestUtils.h"
#include <iostream>
#include <btBulletDynamicsCommon.h>

using namespace GameEngine;
using namespace GameEngine::Physics;
using namespace GameEngine::Testing;

/**
 * Test box shape creation
 * Requirements: Physics collision shape creation
 */
bool TestBoxShapeCreation() {
    TestOutput::PrintTestStart("box shape creation");
    
    CollisionShape boxDesc;
    boxDesc.type = CollisionShape::Box;
    boxDesc.dimensions = Math::Vec3(2.0f, 4.0f, 6.0f);

    auto boxShape = CollisionShapeFactory::CreateShape(boxDesc);
    EXPECT_NOT_NULL(boxShape);
    EXPECT_TRUE(boxShape->getShapeType() == BOX_SHAPE_PROXYTYPE);
    
    TestOutput::PrintTestPass("box shape creation");
    return true;
}

/**
 * Test sphere shape creation
 * Requirements: Physics collision shape creation
 */
bool TestSphereShapeCreation() {
    TestOutput::PrintTestStart("sphere shape creation");
    
    CollisionShape sphereDesc;
    sphereDesc.type = CollisionShape::Sphere;
    sphereDesc.dimensions = Math::Vec3(2.5f, 0.0f, 0.0f);

    auto sphereShape = CollisionShapeFactory::CreateShape(sphereDesc);
    EXPECT_NOT_NULL(sphereShape);
    EXPECT_TRUE(sphereShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE);
    
    TestOutput::PrintTestPass("sphere shape creation");
    return true;
}

/**
 * Test capsule shape creation
 * Requirements: Physics collision shape creation
 */
bool TestCapsuleShapeCreation() {
    TestOutput::PrintTestStart("capsule shape creation");
    
    CollisionShape capsuleDesc;
    capsuleDesc.type = CollisionShape::Capsule;
    capsuleDesc.dimensions = Math::Vec3(1.0f, 3.0f, 0.0f);

    auto capsuleShape = CollisionShapeFactory::CreateShape(capsuleDesc);
    EXPECT_NOT_NULL(capsuleShape);
    EXPECT_TRUE(capsuleShape->getShapeType() == CAPSULE_SHAPE_PROXYTYPE);
    
    TestOutput::PrintTestPass("capsule shape creation");
    return true;
}

/**
 * Test invalid shape rejection
 * Requirements: Physics collision shape validation
 */
bool TestInvalidShapeRejection() {
    TestOutput::PrintTestStart("invalid shape rejection");
    
    CollisionShape invalidDesc;
    invalidDesc.type = CollisionShape::Box;
    invalidDesc.dimensions = Math::Vec3(0.0f, 1.0f, 1.0f); // Zero dimension should be invalid

    auto invalidShape = CollisionShapeFactory::CreateShape(invalidDesc);
    EXPECT_NULL(invalidShape);
    
    TestOutput::PrintTestPass("invalid shape rejection");
    return true;
}

/**
 * Test mesh shape handling (not implemented)
 * Requirements: Physics collision shape creation
 */
bool TestMeshShapeHandling() {
    TestOutput::PrintTestStart("mesh shape handling");
    
    CollisionShape meshDesc;
    meshDesc.type = CollisionShape::Mesh;
    meshDesc.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);

    auto meshShape = CollisionShapeFactory::CreateShape(meshDesc);
    EXPECT_NULL(meshShape); // Should return null as mesh shapes are not implemented
    
    TestOutput::PrintTestPass("mesh shape handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Collision Shape Factory Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Collision Shape Factory Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Box Shape Creation", TestBoxShapeCreation);
        allPassed &= suite.RunTest("Sphere Shape Creation", TestSphereShapeCreation);
        allPassed &= suite.RunTest("Capsule Shape Creation", TestCapsuleShapeCreation);
        allPassed &= suite.RunTest("Invalid Shape Rejection", TestInvalidShapeRejection);
        allPassed &= suite.RunTest("Mesh Shape Handling", TestMeshShapeHandling);

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

#else

#include "TestUtils.h"
#include <iostream>

using namespace GameEngine::Testing;

/**
 * Test Bullet Physics availability
 * Requirements: Physics system availability check
 */
bool TestBulletPhysicsAvailability() {
    TestOutput::PrintTestStart("bullet physics availability");
    
    TestOutput::PrintWarning("Bullet Physics not available - collision shape factory tests skipped");
    
    TestOutput::PrintTestPass("bullet physics availability");
    return true;
}

int main() {
    TestOutput::PrintHeader("Collision Shape Factory Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Collision Shape Factory Integration Tests");

        // Run availability test
        allPassed &= suite.RunTest("Bullet Physics Availability", TestBulletPhysicsAvailability);

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

#endif // GAMEENGINE_HAS_BULLET