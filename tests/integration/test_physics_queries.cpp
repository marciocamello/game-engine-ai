#include "Physics/PhysicsEngine.h"
#include "TestUtils.h"
#include "Core/Logger.h"
#include <iostream>
#include <cassert>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test raycast functionality
 * Requirements: Physics system integration, collision detection
 */
bool TestRaycast() {
    TestOutput::PrintTestStart("raycast functionality");
    
    PhysicsEngine engine;
    if (!engine.Initialize()) {
        TestOutput::PrintError("Failed to initialize physics engine");
        return false;
    }
    
    // Create a physics world
    auto world = engine.CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
    engine.SetActiveWorld(world);
    
    // Create a static box at origin
    RigidBody boxDesc;
    boxDesc.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    boxDesc.isStatic = true;
    
    CollisionShape boxShape;
    boxShape.type = CollisionShape::Box;
    boxShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t boxId = engine.CreateRigidBody(boxDesc, boxShape);
    assert(boxId != 0);
    
    // Test raycast that should hit the box
    Math::Vec3 rayOrigin(-5.0f, 0.0f, 0.0f);
    Math::Vec3 rayDirection(1.0f, 0.0f, 0.0f);
    float maxDistance = 10.0f;
    
    RaycastHit hitResult = engine.Raycast(rayOrigin, rayDirection, maxDistance);
    
    if (hitResult.hasHit) {
        TestOutput::PrintInfo("Raycast hit detected! Hit body ID: " + std::to_string(hitResult.bodyId));
        TestOutput::PrintInfo("Hit point: " + StringUtils::FormatVec3(hitResult.point));
        TestOutput::PrintInfo("Hit normal: " + StringUtils::FormatVec3(hitResult.normal));
        TestOutput::PrintInfo("Hit distance: " + StringUtils::FormatFloat(hitResult.distance));
        EXPECT_TRUE(hitResult.bodyId == boxId);
    } else {
        TestOutput::PrintTestFail("raycast functionality", "hit detected", "no hit");
        return false;
    }
    
    // Test raycast that should miss
    Math::Vec3 rayOriginMiss(-5.0f, 5.0f, 0.0f);  // Ray above the box
    
    RaycastHit missResult = engine.Raycast(rayOriginMiss, rayDirection, maxDistance);
    
    if (!missResult.hasHit) {
        TestOutput::PrintInfo("Raycast correctly missed the box");
    } else {
        TestOutput::PrintTestFail("raycast functionality", "no hit", "hit body ID: " + std::to_string(missResult.bodyId));
        return false;
    }
    
    // Cleanup
    engine.DestroyRigidBody(boxId);
    engine.Shutdown();
    
    TestOutput::PrintTestPass("raycast functionality");
    return true;
}

/**
 * Test overlap sphere functionality
 * Requirements: Physics system integration, collision detection
 */
bool TestOverlapSphere() {
    TestOutput::PrintTestStart("overlap sphere functionality");
    
    PhysicsEngine engine;
    if (!engine.Initialize()) {
        TestOutput::PrintError("Failed to initialize physics engine");
        return false;
    }
    
    // Create a physics world
    auto world = engine.CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
    engine.SetActiveWorld(world);
    
    // Create multiple rigid bodies at different positions
    std::vector<uint32_t> bodyIds;
    
    // Box 1 at origin (should overlap)
    RigidBody box1Desc;
    box1Desc.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    box1Desc.isStatic = true;
    
    CollisionShape boxShape;
    boxShape.type = CollisionShape::Box;
    boxShape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
    
    uint32_t box1Id = engine.CreateRigidBody(box1Desc, boxShape);
    bodyIds.push_back(box1Id);
    
    // Box 2 nearby (should overlap)
    RigidBody box2Desc;
    box2Desc.position = Math::Vec3(1.5f, 0.0f, 0.0f);
    box2Desc.isStatic = true;
    
    uint32_t box2Id = engine.CreateRigidBody(box2Desc, boxShape);
    bodyIds.push_back(box2Id);
    
    // Box 3 far away (should not overlap)
    RigidBody box3Desc;
    box3Desc.position = Math::Vec3(10.0f, 0.0f, 0.0f);
    box3Desc.isStatic = true;
    
    uint32_t box3Id = engine.CreateRigidBody(box3Desc, boxShape);
    bodyIds.push_back(box3Id);
    
    // Test overlap sphere at origin with radius 3.0
    Math::Vec3 sphereCenter(0.0f, 0.0f, 0.0f);
    float sphereRadius = 3.0f;
    
    std::vector<OverlapResult> overlappingBodies = engine.OverlapSphere(sphereCenter, sphereRadius);
    
    TestOutput::PrintInfo("Found " + std::to_string(overlappingBodies.size()) + " overlapping bodies");
    
    // Should find box1 and box2, but not box3
    bool foundBox1 = false, foundBox2 = false, foundBox3 = false;
    
    for (const OverlapResult& result : overlappingBodies) {
        TestOutput::PrintInfo("Overlapping body ID: " + std::to_string(result.bodyId));
        TestOutput::PrintInfo("  Contact point: " + StringUtils::FormatVec3(result.contactPoint));
        TestOutput::PrintInfo("  Contact normal: " + StringUtils::FormatVec3(result.contactNormal));
        TestOutput::PrintInfo("  Penetration depth: " + StringUtils::FormatFloat(result.penetrationDepth));
        
        if (result.bodyId == box1Id) foundBox1 = true;
        if (result.bodyId == box2Id) foundBox2 = true;
        if (result.bodyId == box3Id) foundBox3 = true;
    }
    
    EXPECT_TRUE(foundBox1);
    if (foundBox1) {
        TestOutput::PrintInfo("Found box1 in overlap (expected)");
    }
    
    EXPECT_TRUE(foundBox2);
    if (foundBox2) {
        TestOutput::PrintInfo("Found box2 in overlap (expected)");
    }
    
    EXPECT_FALSE(foundBox3);
    if (!foundBox3) {
        TestOutput::PrintInfo("Did not find box3 in overlap (expected)");
    }
    
    // Cleanup
    for (uint32_t bodyId : bodyIds) {
        engine.DestroyRigidBody(bodyId);
    }
    engine.Shutdown();
    
    TestOutput::PrintTestPass("overlap sphere functionality");
    return true;
}

int main() {
    TestOutput::PrintHeader("Physics Queries Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Physics Queries Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Raycast Functionality", TestRaycast);
        allPassed &= suite.RunTest("Overlap Sphere Functionality", TestOverlapSphere);

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