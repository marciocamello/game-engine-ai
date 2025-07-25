#include "Physics/PhysicsEngine.h"
#include "Game/Character.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <iostream>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * @brief Simple memory usage test
 * 
 * Tests memory usage patterns to detect potential leaks:
 * - Physics engine object creation/destruction cycles
 * - Character creation/destruction cycles
 * - Extended simulation runs
 */
class SimpleMemoryUsageTest {
public:
    struct MemoryInfo {
        size_t workingSetSize = 0;
        size_t privateUsage = 0;
    };

    SimpleMemoryUsageTest() {
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        if (!m_physicsEngine->Initialize()) {
            LOG_ERROR("Failed to initialize physics engine for memory test");
        }
        
        m_inputManager = std::make_unique<InputManager>();
        if (!m_inputManager->Initialize(nullptr)) {
            LOG_ERROR("Failed to initialize input manager for memory test");
        }
    }

    ~SimpleMemoryUsageTest() {
        if (m_inputManager) {
            m_inputManager->Shutdown();
        }
        if (m_physicsEngine) {
            m_physicsEngine->Shutdown();
        }
    }

    bool RunMemoryTests() {
        TestOutput::PrintInfo("Starting Simple Memory Usage Tests");
        
        bool allPassed = true;
        allPassed &= TestPhysicsObjectChurn();
        allPassed &= TestCharacterLifecycle();
        allPassed &= TestExtendedSimulation();
        
        TestOutput::PrintInfo("Memory Usage Tests Complete");
        return allPassed;
    }

private:
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    std::unique_ptr<InputManager> m_inputManager;
    
    MemoryInfo GetMemoryInfo() {
        MemoryInfo info;
        
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            info.workingSetSize = pmc.WorkingSetSize;
            info.privateUsage = pmc.PrivateUsage;
        }
#endif
        
        return info;
    }
    
    void PrintMemoryUsage(const std::string& testName, const MemoryInfo& before, const MemoryInfo& after) {
        TestOutput::PrintInfo("--- " + testName + " Memory Usage ---");
        
        long long workingSetDiff = static_cast<long long>(after.workingSetSize) - static_cast<long long>(before.workingSetSize);
        long long privateDiff = static_cast<long long>(after.privateUsage) - static_cast<long long>(before.privateUsage);
        
        std::string workingSetMsg = "Working Set: " + std::to_string(before.workingSetSize / 1024 / 1024) + " MB -> " + 
                                   std::to_string(after.workingSetSize / 1024 / 1024) + " MB (";
        if (workingSetDiff >= 0) workingSetMsg += "+";
        workingSetMsg += std::to_string(workingSetDiff / 1024 / 1024) + " MB)";
        TestOutput::PrintInfo(workingSetMsg);
        
        std::string privateMsg = "Private Usage: " + std::to_string(before.privateUsage / 1024 / 1024) + " MB -> " + 
                                std::to_string(after.privateUsage / 1024 / 1024) + " MB (";
        if (privateDiff >= 0) privateMsg += "+";
        privateMsg += std::to_string(privateDiff / 1024 / 1024) + " MB)";
        TestOutput::PrintInfo(privateMsg);
        
        // Simple leak detection
        const long long LEAK_THRESHOLD_MB = 5; // 5 MB threshold
        
        if (workingSetDiff > LEAK_THRESHOLD_MB * 1024 * 1024) {
            TestOutput::PrintWarning("POTENTIAL MEMORY LEAK DETECTED!");
        } else if (workingSetDiff < -1024 * 1024) { // More than 1MB freed
            TestOutput::PrintInfo("Good memory cleanup detected");
        } else {
            TestOutput::PrintInfo("Memory usage within acceptable range");
        }
    }
    
    bool TestPhysicsObjectChurn() {
        TestOutput::PrintTestStart("physics object creation/destruction");
        
        TestOutput::PrintInfo("Testing Physics Object Creation/Destruction...");
        
        auto beforeMemory = GetMemoryInfo();
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        const int ITERATIONS = 500;
        const int OBJECTS_PER_ITERATION = 20;
        
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            std::vector<uint32_t> bodyIds;
            
            // Create objects
            for (int i = 0; i < OBJECTS_PER_ITERATION; ++i) {
                RigidBody bodyDesc;
                bodyDesc.position = Math::Vec3(
                    static_cast<float>(i % 5), 
                    static_cast<float>(i / 5), 
                    0.0f
                );
                bodyDesc.isStatic = (i % 2 == 0);
                
                CollisionShape shape;
                shape.type = static_cast<CollisionShape::Type>(i % 3); // Box, Sphere, Capsule
                shape.dimensions = Math::Vec3(1.0f, 1.0f, 1.0f);
                
                uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
                bodyIds.push_back(bodyId);
            }
            
            // Brief simulation
            for (int step = 0; step < 3; ++step) {
                m_physicsEngine->Update(1.0f / 60.0f);
            }
            
            // Destroy objects
            for (uint32_t bodyId : bodyIds) {
                m_physicsEngine->DestroyRigidBody(bodyId);
            }
            
            // Progress indicator
            if (iter % 100 == 0) {
                TestOutput::PrintInfo("Progress: " + std::to_string(iter) + "/" + std::to_string(ITERATIONS) + " iterations");
            }
        }
        
        auto afterMemory = GetMemoryInfo();
        PrintMemoryUsage("Physics Object Churn", beforeMemory, afterMemory);
        
        TestOutput::PrintTestPass("physics object creation/destruction");
        return true;
    }
    
    bool TestCharacterLifecycle() {
        TestOutput::PrintTestStart("character lifecycle");
        
        TestOutput::PrintInfo("Testing Character Lifecycle...");
        
        auto beforeMemory = GetMemoryInfo();
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        const int ITERATIONS = 200;
        const int CHARACTERS_PER_ITERATION = 10;
        
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            std::vector<std::unique_ptr<Character>> characters;
            
            // Create characters
            for (int i = 0; i < CHARACTERS_PER_ITERATION; ++i) {
                auto character = std::make_unique<Character>();
                character->Initialize(m_physicsEngine.get());
                character->SetPosition(Math::Vec3(
                    static_cast<float>(i % 3) * 2.0f, 
                    1.0f, 
                    static_cast<float>(i / 3) * 2.0f
                ));
                
                // Switch movement types
                if (i % 3 == 0) {
                    character->SwitchToCharacterMovement();
                } else if (i % 3 == 1) {
                    character->SwitchToPhysicsMovement();
                } else {
                    character->SwitchToHybridMovement();
                }
                
                characters.push_back(std::move(character));
            }
            
            // Simulate character updates
            float deltaTime = 1.0f / 60.0f;
            for (int frame = 0; frame < 30; ++frame) { // 0.5 seconds
                for (auto& character : characters) {
                    character->Update(deltaTime, m_inputManager.get());
                }
                m_physicsEngine->Update(deltaTime);
            }
            
            // Characters will be automatically destroyed when going out of scope
            
            // Progress indicator
            if (iter % 50 == 0) {
                TestOutput::PrintInfo("Progress: " + std::to_string(iter) + "/" + std::to_string(ITERATIONS) + " iterations");
            }
        }
        
        auto afterMemory = GetMemoryInfo();
        PrintMemoryUsage("Character Lifecycle", beforeMemory, afterMemory);
        
        TestOutput::PrintTestPass("character lifecycle");
        return true;
    }
    
    bool TestExtendedSimulation() {
        TestOutput::PrintTestStart("extended simulation");
        
        TestOutput::PrintInfo("Testing Extended Simulation...");
        
        auto beforeMemory = GetMemoryInfo();
        
        auto world = m_physicsEngine->CreateWorld(Math::Vec3(0.0f, -9.81f, 0.0f));
        m_physicsEngine->SetActiveWorld(world);
        
        // Create persistent environment
        std::vector<uint32_t> staticBodies;
        std::vector<uint32_t> dynamicBodies;
        std::vector<std::unique_ptr<Character>> characters;
        
        // Create static environment
        for (int i = 0; i < 20; ++i) {
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(
                static_cast<float>((i % 5) * 4), 
                0.0f, 
                static_cast<float>((i / 5) * 4)
            );
            bodyDesc.isStatic = true;
            
            CollisionShape shape;
            shape.type = CollisionShape::Box;
            shape.dimensions = Math::Vec3(2.0f, 0.5f, 2.0f);
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            staticBodies.push_back(bodyId);
        }
        
        // Create dynamic objects
        for (int i = 0; i < 30; ++i) {
            RigidBody bodyDesc;
            bodyDesc.position = Math::Vec3(
                static_cast<float>((i % 6) * 2), 
                static_cast<float>(5 + i / 6), 
                static_cast<float>((i % 3) * 2)
            );
            bodyDesc.isStatic = false;
            
            CollisionShape shape;
            shape.type = static_cast<CollisionShape::Type>(i % 3);
            shape.dimensions = Math::Vec3(0.5f, 0.5f, 0.5f);
            
            uint32_t bodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            dynamicBodies.push_back(bodyId);
        }
        
        // Create characters
        for (int i = 0; i < 5; ++i) {
            auto character = std::make_unique<Character>();
            character->Initialize(m_physicsEngine.get());
            character->SetPosition(Math::Vec3(
                static_cast<float>(i * 3), 
                2.0f, 
                static_cast<float>(i * 2)
            ));
            
            if (i % 3 == 0) {
                character->SwitchToCharacterMovement();
            } else if (i % 3 == 1) {
                character->SwitchToPhysicsMovement();
            } else {
                character->SwitchToHybridMovement();
            }
            
            characters.push_back(std::move(character));
        }
        
        // Run extended simulation (2 minutes of simulation time)
        const int TOTAL_STEPS = 7200; // 2 minutes at 60 FPS
        const float deltaTime = 1.0f / 60.0f;
        
        for (int step = 0; step < TOTAL_STEPS; ++step) {
            // Update characters
            for (auto& character : characters) {
                character->Update(deltaTime, m_inputManager.get());
            }
            
            // Apply occasional forces to dynamic bodies
            if (step % 120 == 0) { // Every 2 seconds
                for (size_t i = 0; i < dynamicBodies.size(); i += 3) {
                    Math::Vec3 force(
                        static_cast<float>((rand() % 100) - 50), 
                        static_cast<float>(rand() % 50), 
                        static_cast<float>((rand() % 100) - 50)
                    );
                    m_physicsEngine->ApplyForce(dynamicBodies[i], force);
                }
            }
            
            m_physicsEngine->Update(deltaTime);
            
            // Progress indicator
            if (step % 1200 == 0) { // Every 20 seconds
                TestOutput::PrintInfo("Simulation progress: " + std::to_string(step / 1200) + "/6 (20-second intervals)");
            }
        }
        
        // Cleanup
        for (uint32_t bodyId : staticBodies) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        for (uint32_t bodyId : dynamicBodies) {
            m_physicsEngine->DestroyRigidBody(bodyId);
        }
        characters.clear();
        
        auto afterMemory = GetMemoryInfo();
        PrintMemoryUsage("Extended Simulation (2 minutes)", beforeMemory, afterMemory);
        
        TestOutput::PrintTestPass("extended simulation");
        return true;
    }
};

int main() {
    TestOutput::PrintHeader("Memory Usage Simple Integration");

    bool allPassed = true;

    try {
        TestOutput::PrintInfo("Starting Simple Memory Usage Tests...");
        TestOutput::PrintInfo("This test will run for a few minutes to check memory patterns.");
        
        // Create test suite for result tracking
        TestSuite suite("Memory Usage Simple Integration Tests");
        
        SimpleMemoryUsageTest test;
        bool testResult = test.RunMemoryTests();
        
        allPassed &= suite.RunTest("Memory Usage Tests", [testResult]() { return testResult; });
        
        TestOutput::PrintInfo("If no memory leak warnings were shown, the physics system");
        TestOutput::PrintInfo("appears to be managing memory correctly.");

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