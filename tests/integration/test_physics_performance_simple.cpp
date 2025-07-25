#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

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

    bool RunPerformanceTests() {
        TestOutput::PrintInfo("Starting Simple Physics Performance Tests");
        
        bool allPassed = true;
        allPassed &= TestRigidBodyCreation();
        allPassed &= TestPhysicsUpdates();
        allPassed &= TestRaycastPerformance();
        allPassed &= TestOverlapPerformance();
        
        TestOutput::PrintInfo("Performance Tests Complete");
        return allPassed;
    }

private:
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    
    bool TestRigidBodyCreation() {
        TestOutput::PrintTestStart("rigid body creation performance");
        
        TestOutput::PrintInfo("Testing Rigid Body Creation Performance...");
        
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
        
        TestOutput::PrintInfo("Created " + std::to_string(NUM_BODIES) + " rigid bodies in " + std::to_string(duration.count()) + "ms");
        TestOutput::PrintInfo("Average: " + std::to_string(static_cast<double>(duration.count()) / NUM_BODIES) + "ms per body");
        
        // Cleanup
        startTime = std::chrono::high_resolution_clock::now();
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        endTime = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        TestOutput::PrintInfo("Destroyed " + std::to_string(NUM_BODIES) + " rigid bodies in " + std::to_string(duration.count()) + "ms");
        
        TestOutput::PrintTestPass("rigid body creation performance");
        return true;
    }
    
    bool TestPhysicsUpdates() {
        TestOutput::PrintTestStart("physics updates performance");
        
        TestOutput::PrintInfo("Testing Physics Update Performance...");
        
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
        
        TestOutput::PrintInfo("Performed " + std::to_string(NUM_UPDATES) + " physics updates in " + std::to_string(duration.count()) + "ms");
        TestOutput::PrintInfo("Average: " + std::to_string(static_cast<double>(duration.count()) / NUM_UPDATES) + "ms per update");
        
        // Cleanup
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        
        TestOutput::PrintTestPass("physics updates performance");
        return true;
    }
    
    bool TestRaycastPerformance() {
        TestOutput::PrintTestStart("raycast performance");
        
        TestOutput::PrintInfo("Testing Raycast Performance...");
        
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
        
        TestOutput::PrintInfo("Performed " + std::to_string(NUM_RAYCASTS) + " raycasts in " + std::to_string(duration.count()) + "ms");
        TestOutput::PrintInfo("Average: " + std::to_string(static_cast<double>(duration.count()) / NUM_RAYCASTS) + "ms per raycast");
        TestOutput::PrintInfo("Hits: " + std::to_string(hits) + "/" + std::to_string(NUM_RAYCASTS));
        
        // Cleanup
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        
        TestOutput::PrintTestPass("raycast performance");
        return true;
    }
    
    bool TestOverlapPerformance() {
        TestOutput::PrintTestStart("overlap performance");
        
        TestOutput::PrintInfo("Testing Overlap Query Performance...");
        
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
        
        TestOutput::PrintInfo("Performed " + std::to_string(NUM_OVERLAPS) + " overlap queries in " + std::to_string(duration.count()) + "ms");
        TestOutput::PrintInfo("Average: " + std::to_string(static_cast<double>(duration.count()) / NUM_OVERLAPS) + "ms per query");
        TestOutput::PrintInfo("Total hits found: " + std::to_string(totalHits));
        
        // Cleanup
        for (uint32_t bodyId : bodyIds) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        
        TestOutput::PrintTestPass("overlap performance");
        return true;
    }
};

int main() {
    TestOutput::PrintHeader("Physics Performance Simple Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Physics Performance Simple Integration Tests");
        
        SimplePhysicsPerformanceTest test;
        bool testResult = test.RunPerformanceTests();
        
        allPassed &= suite.RunTest("Physics Performance Tests", [testResult]() { return testResult; });

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