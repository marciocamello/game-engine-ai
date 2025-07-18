#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>

using namespace GameEngine;

/**
 * @brief Simple physics performance test
 * 
 * Tests basic performance characteristics of physics operations:
 * - Rigid body creation and destruction
 * - Physics world updates
 * - Raycast queries
 * - Overlap queries
 */
class SimplePhysicsPerformanceTest {
public:
    SimplePhysicsPerformanceTest() {
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        if (!m_physicsEngine->Initialize()) {
            LOG_ERROR("Failed to initialize physics engine for performance test");
        }
    }

    ~SimplePhysicsPerformanceTest() {
        if (m_physicsEngine) {
            m_physicsEngine->Shutdown();
        }
    }

    void RunPerformanceTests() {
        std::cout << "=== Simple Physics Performance Tests ===" << std::endl;
        
        TestRigidBodyCreation();
        TestPhysicsUpdates();
        TestRaycastPerformance();
        TestOverlapPerformance();
        
        std::cout << "=== Performance Tests Complete ===" << std::endl;
    }

private:
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    
    void TestRigidBodyCreation() {
        std::cout << "\nTesting Rigid Body Creation Performance..." << std::endl;
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        const int NUM_BODIES = 1000;
        std::vector<uint32_t> bodyIds;
        bodyIds.reserve(NUM_BODIES);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Create bodies
        for (int i = 0; i < NUM_BODIES; ++i) {
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(
                static_cast<float>(i % 10), 
                static_cast<float>(i / 10), 
                0.0f
            );
            bodyDesc.isStatic = (i % 2 == 0);
            
            CollisionShape shape;
            shape.type = CollisionShape::Box;
            shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            bodyIds.push_back(bodyId);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Created " << NUM_BODIES << " rigid bodies in " << duration.count() << "ms" << std::endl;
        std::cout << "Average: " << (static_cast<double>(duration.count()) / NUM_BODIES) << "ms per body" << std::endl;
        
        // Cleanup
        startTime = std::chrono::high_resolution_clock::now();
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        endTime = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Destroyed " << NUM_BODIES << " rigid bodies in " << duration.count() << "ms" << std::endl;
    }
    
    void TestPhysicsUpdates() {
        std::cout << "\nTesting Physics Update Performance..." << std::endl;
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        // Create some objects to simulate
        std::vector<uint32_t> bodyIds;
        for (int i = 0; i < 100; ++i) {
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(
                static_cast<float>(i % 10) * 2.0f, 
                static_cast<float>(10 + i / 10), 
                0.0f
            );
            bodyDesc.isStatic = false;
            
            CollisionShape shape;
            shape.type = CollisionShape::Box;
            shape.dimensions = Math::Vec3(0.5f, 0.5f, 0.5f);
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            bodyIds.push_back(bodyId);
        }
        
        const int NUM_UPDATES = 1000;
        float deltaTime = 1.0f / 60.0f;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_UPDATES; ++i) {
            m_physicsEngine->Update(deltaTime);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Performed " << NUM_UPDATES << " physics updates in " << duration.count() << "ms" << std::endl;
        std::cout << "Average: " << (static_cast<double>(duration.count()) / NUM_UPDATES) << "ms per update" << std::endl;
        
        // Cleanup
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
    }
    
    void TestRaycastPerformance() {
        std::cout << "\nTesting Raycast Performance..." << std::endl;
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        // Create target objects
        std::vector<uint32_t> bodyIds;
        for (int i = 0; i < 50; ++i) {
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(static_cast<float>(i * 2), 0.0f, 0.0f);
            bodyDesc.isStatic = true;
            
            CollisionShape shape;
            shape.type = CollisionShape::Box;
            shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            bodyIds.push_back(bodyId);
        }
        
        const int NUM_RAYCASTS = 1000;
        int hits = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_RAYCASTS; ++i) {
            Math::Vec3 rayOrigin(-5.0f, 0.0f, 0.0f);
            Math::Vec3 rayDirection(1.0f, 0.0f, 0.0f);
            float maxDistance = 105.0f;
            
            RaycastHit hit = m_physicsEngine->Raycast(rayOrigin, rayDirection, maxDistance);
            if (hit.hasHit) {
                hits++;
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Performed " << NUM_RAYCASTS << " raycasts in " << duration.count() << "ms" << std::endl;
        std::cout << "Average: " << (static_cast<double>(duration.count()) / NUM_RAYCASTS) << "ms per raycast" << std::endl;
        std::cout << "Hits: " << hits << "/" << NUM_RAYCASTS << std::endl;
        
        // Cleanup
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
    }
    
    void TestOverlapPerformance() {
        std::cout << "\nTesting Overlap Query Performance..." << std::endl;
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        // Create target objects
        std::vector<uint32_t> bodyIds;
        for (int i = 0; i < 100; ++i) {
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(
                static_cast<float>((i % 10) * 2), 
                0.0f, 
                static_cast<float>((i / 10) * 2)
            );
            bodyDesc.isStatic = true;
            
            CollisionShape shape;
            shape.type = CollisionShape::Sphere;
            shape.dimensions = Math::Vec3(0.5f, 0.0f, 0.0f); // For sphere: radius in x component
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            bodyIds.push_back(bodyId);
        }
        
        const int NUM_OVERLAPS = 1000;
        int totalHits = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_OVERLAPS; ++i) {
            Math::Vec3 center(10.0f, 0.0f, 10.0f);
            float radius = 15.0f;
            
            std::vector<OverlapResult> hits = m_physicsEngine->OverlapSphere(center, radius);
            totalHits += static_cast<int>(hits.size());
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Performed " << NUM_OVERLAPS << " overlap queries in " << duration.count() << "ms" << std::endl;
        std::cout << "Average: " << (static_cast<double>(duration.count()) / NUM_OVERLAPS) << "ms per query" << std::endl;
        std::cout << "Total hits found: " << totalHits << std::endl;
        
        // Cleanup
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
    }
};

int main() {
    try {
        SimplePhysicsPerformanceTest test;
        test.RunPerformanceTests();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Performance test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}