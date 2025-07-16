#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <iostream>
#include <cassert>

using namespace GameEngine;

void TestRaycast() {
    std::cout << "Testing Raycast functionality..." << std::endl;
    
    PhysicsEngine engine;
    if (!engine.Initialize()) {
        std::cerr << "Failed to initialize physics engine" << std::endl;
        return;
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
        std::cout << "✓ Raycast hit detected! Hit body ID: " << hitResult.bodyId << std::endl;
        std::cout << "  Hit point: (" << hitResult.point.x << ", " << hitResult.point.y << ", " << hitResult.point.z << ")" << std::endl;
        std::cout << "  Hit normal: (" << hitResult.normal.x << ", " << hitResult.normal.y << ", " << hitResult.normal.z << ")" << std::endl;
        std::cout << "  Hit distance: " << hitResult.distance << std::endl;
        assert(hitResult.bodyId == boxId);
    } else {
        std::cout << "✗ Raycast should have hit the box but didn't" << std::endl;
    }
    
    // Test raycast that should miss
    Math::Vec3 rayOriginMiss(-5.0f, 5.0f, 0.0f);  // Ray above the box
    
    RaycastHit missResult = engine.Raycast(rayOriginMiss, rayDirection, maxDistance);
    
    if (!missResult.hasHit) {
        std::cout << "✓ Raycast correctly missed the box" << std::endl;
    } else {
        std::cout << "✗ Raycast should have missed but hit body ID: " << missResult.bodyId << std::endl;
    }
    
    // Cleanup
    engine.DestroyRigidBody(boxId);
    engine.Shutdown();
    
    std::cout << "Raycast test completed!" << std::endl;
}

void TestOverlapSphere() {
    std::cout << "Testing OverlapSphere functionality..." << std::endl;
    
    PhysicsEngine engine;
    if (!engine.Initialize()) {
        std::cerr << "Failed to initialize physics engine" << std::endl;
        return;
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
    
    std::cout << "Found " << overlappingBodies.size() << " overlapping bodies" << std::endl;
    
    // Should find box1 and box2, but not box3
    bool foundBox1 = false, foundBox2 = false, foundBox3 = false;
    
    for (const OverlapResult& result : overlappingBodies) {
        std::cout << "Overlapping body ID: " << result.bodyId << std::endl;
        std::cout << "  Contact point: (" << result.contactPoint.x << ", " << result.contactPoint.y << ", " << result.contactPoint.z << ")" << std::endl;
        std::cout << "  Contact normal: (" << result.contactNormal.x << ", " << result.contactNormal.y << ", " << result.contactNormal.z << ")" << std::endl;
        std::cout << "  Penetration depth: " << result.penetrationDepth << std::endl;
        
        if (result.bodyId == box1Id) foundBox1 = true;
        if (result.bodyId == box2Id) foundBox2 = true;
        if (result.bodyId == box3Id) foundBox3 = true;
    }
    
    if (foundBox1) {
        std::cout << "✓ Found box1 in overlap (expected)" << std::endl;
    } else {
        std::cout << "✗ Did not find box1 in overlap (should have found it)" << std::endl;
    }
    
    if (foundBox2) {
        std::cout << "✓ Found box2 in overlap (expected)" << std::endl;
    } else {
        std::cout << "✗ Did not find box2 in overlap (should have found it)" << std::endl;
    }
    
    if (!foundBox3) {
        std::cout << "✓ Did not find box3 in overlap (expected)" << std::endl;
    } else {
        std::cout << "✗ Found box3 in overlap (should not have found it)" << std::endl;
    }
    
    // Cleanup
    for (uint32_t bodyId : bodyIds) {
        engine.DestroyRigidBody(bodyId);
    }
    engine.Shutdown();
    
    std::cout << "OverlapSphere test completed!" << std::endl;
}

int main() {
    std::cout << "=== Physics Queries Integration Test ===" << std::endl;
    
    TestRaycast();
    std::cout << std::endl;
    TestOverlapSphere();
    
    std::cout << "\n=== All Physics Query Tests Completed ===" << std::endl;
    return 0;
}