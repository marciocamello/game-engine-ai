#include "Core/TestRunner.h"
#include "Core/Logger.h"
#include "Core/Math.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Example visual tests registration
void RegisterPhysicsVisualizationTests() {
    // Physics collision visualization test
    TestRunner::GetInstance().RegisterVisualTest({
        "Physics Collision Visualization",
        "Shows collision shapes, raycasts, and physics debug information",
        []() {
            // Setup: Create physics world with test objects
            Logger::GetInstance().Info("Setting up physics collision test scene");
        },
        [](float deltaTime) {
            // Update: Simulate physics and draw debug shapes
            static float time = 0.0f;
            time += deltaTime;
            
            // Draw some debug shapes that move over time
            Math::Vec3 spherePos(std::sin(time) * 5.0f, 0.0f, 0.0f);
            TestRunner::GetInstance().DrawDebugSphere(spherePos, 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
            
            Math::Vec3 boxPos(0.0f, std::cos(time) * 3.0f, 0.0f);
            TestRunner::GetInstance().DrawDebugBox(boxPos, Math::Vec3(2.0f, 1.0f, 1.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
            
            // Draw a raycast
            Math::Vec3 rayStart(-10.0f, 0.0f, 0.0f);
            Math::Vec3 rayEnd(10.0f, 0.0f, 0.0f);
            TestRunner::GetInstance().DrawDebugLine(rayStart, rayEnd, Math::Vec3(0.0f, 0.0f, 1.0f));
            
            // Draw debug text
            TestRunner::GetInstance().DrawDebugText("Physics Test Active", Math::Vec3(0.0f, 5.0f, 0.0f));
        },
        []() {
            // Render: Additional rendering if needed
        },
        []() {
            // Cleanup: Clean up test resources
            Logger::GetInstance().Info("Cleaning up physics collision test");
        }
    });
    
    // Character movement visualization test
    TestRunner::GetInstance().RegisterVisualTest({
        "Character Movement Debug",
        "Visualizes character movement, ground detection, and camera behavior",
        []() {
            Logger::GetInstance().Info("Setting up character movement test");
        },
        [](float deltaTime) {
            static Math::Vec3 characterPos(0.0f, 1.0f, 0.0f);
            static float moveSpeed = 5.0f;
            static float time = 0.0f;
            time += deltaTime;
            
            // Simulate character movement in a circle
            characterPos.x = std::cos(time * 0.5f) * 8.0f;
            characterPos.z = std::sin(time * 0.5f) * 8.0f;
            
            // Draw character capsule
            TestRunner::GetInstance().DrawDebugSphere(characterPos, 0.5f, Math::Vec3(1.0f, 1.0f, 0.0f));
            
            // Draw ground detection rays
            Math::Vec3 groundRayStart = characterPos + Math::Vec3(0.0f, 0.5f, 0.0f);
            Math::Vec3 groundRayEnd = characterPos - Math::Vec3(0.0f, 2.0f, 0.0f);
            TestRunner::GetInstance().DrawDebugLine(groundRayStart, groundRayEnd, Math::Vec3(1.0f, 0.0f, 1.0f));
            
            // Draw movement direction
            Math::Vec3 velocity(-std::sin(time * 0.5f), 0.0f, std::cos(time * 0.5f));
            velocity = glm::normalize(velocity) * 2.0f;
            TestRunner::GetInstance().DrawDebugLine(characterPos, characterPos + velocity, Math::Vec3(0.0f, 1.0f, 1.0f));
            
            // Draw debug info
            TestRunner::GetInstance().DrawDebugText("Character: (" + 
                std::to_string(characterPos.x) + ", " + 
                std::to_string(characterPos.y) + ", " + 
                std::to_string(characterPos.z) + ")", 
                characterPos + Math::Vec3(0.0f, 2.0f, 0.0f));
        },
        []() {},
        []() {
            Logger::GetInstance().Info("Cleaning up character movement test");
        }
    });
}

void RegisterRenderingTests() {
    // Shader debugging test
    TestRunner::GetInstance().RegisterRenderTest("Shader Debug Visualization", []() {
        // This would render debug information about shaders
        Logger::GetInstance().Debug("Rendering shader debug information");
    });
    
    // Camera system test
    TestRunner::GetInstance().RegisterVisualTest({
        "Camera System Debug",
        "Shows camera frustum, look-at targets, and camera movement",
        []() {
            Logger::GetInstance().Info("Setting up camera debug test");
        },
        [](float deltaTime) {
            static float cameraAngle = 0.0f;
            cameraAngle += deltaTime * 0.5f;
            
            // Draw camera position and look direction
            Math::Vec3 cameraPos(std::cos(cameraAngle) * 10.0f, 5.0f, std::sin(cameraAngle) * 10.0f);
            Math::Vec3 lookTarget(0.0f, 0.0f, 0.0f);
            
            TestRunner::GetInstance().DrawDebugSphere(cameraPos, 0.3f, Math::Vec3(1.0f, 0.5f, 0.0f));
            TestRunner::GetInstance().DrawDebugLine(cameraPos, lookTarget, Math::Vec3(1.0f, 1.0f, 1.0f));
            
            // Draw camera frustum (simplified)
            float fovAngle = glm::radians(45.0f);
            float distance = 5.0f;
            Math::Vec3 forward = glm::normalize(lookTarget - cameraPos);
            Math::Vec3 right = glm::normalize(glm::cross(forward, Math::Vec3(0.0f, 1.0f, 0.0f)));
            Math::Vec3 up = glm::cross(right, forward);
            
            // Draw frustum edges
            float halfWidth = distance * std::tan(fovAngle * 0.5f);
            Math::Vec3 frustumCenter = cameraPos + forward * distance;
            
            Math::Vec3 topLeft = frustumCenter + up * halfWidth - right * halfWidth;
            Math::Vec3 topRight = frustumCenter + up * halfWidth + right * halfWidth;
            Math::Vec3 bottomLeft = frustumCenter - up * halfWidth - right * halfWidth;
            Math::Vec3 bottomRight = frustumCenter - up * halfWidth + right * halfWidth;
            
            TestRunner::GetInstance().DrawDebugLine(cameraPos, topLeft, Math::Vec3(0.5f, 0.5f, 1.0f));
            TestRunner::GetInstance().DrawDebugLine(cameraPos, topRight, Math::Vec3(0.5f, 0.5f, 1.0f));
            TestRunner::GetInstance().DrawDebugLine(cameraPos, bottomLeft, Math::Vec3(0.5f, 0.5f, 1.0f));
            TestRunner::GetInstance().DrawDebugLine(cameraPos, bottomRight, Math::Vec3(0.5f, 0.5f, 1.0f));
            
            TestRunner::GetInstance().DrawDebugText("Camera Debug", cameraPos + Math::Vec3(0.0f, 1.0f, 0.0f));
        },
        []() {},
        []() {
            Logger::GetInstance().Info("Cleaning up camera debug test");
        }
    });
}

void RegisterAITests() {
    // AI pathfinding visualization
    TestRunner::GetInstance().RegisterVisualTest({
        "AI Pathfinding Debug",
        "Shows AI navigation paths, waypoints, and decision making",
        []() {
            Logger::GetInstance().Info("Setting up AI pathfinding test");
        },
        [](float deltaTime) {
            static std::vector<Math::Vec3> waypoints = {
                Math::Vec3(-5.0f, 0.0f, -5.0f),
                Math::Vec3(5.0f, 0.0f, -5.0f),
                Math::Vec3(5.0f, 0.0f, 5.0f),
                Math::Vec3(-5.0f, 0.0f, 5.0f)
            };
            
            static int currentWaypoint = 0;
            static Math::Vec3 aiPosition(-5.0f, 0.0f, -5.0f);
            static float moveSpeed = 3.0f;
            
            // Move AI towards current waypoint
            Math::Vec3 target = waypoints[currentWaypoint];
            Math::Vec3 direction = target - aiPosition;
            float distance = glm::length(direction);
            
            if (distance > 0.1f) {
                direction = glm::normalize(direction);
                aiPosition += direction * moveSpeed * deltaTime;
            } else {
                currentWaypoint = (currentWaypoint + 1) % waypoints.size();
            }
            
            // Draw AI agent
            TestRunner::GetInstance().DrawDebugSphere(aiPosition, 0.5f, Math::Vec3(1.0f, 0.0f, 0.0f));
            
            // Draw waypoints
            for (size_t i = 0; i < waypoints.size(); ++i) {
                Math::Vec3 color = (i == currentWaypoint) ? Math::Vec3(0.0f, 1.0f, 0.0f) : Math::Vec3(0.5f, 0.5f, 0.5f);
                TestRunner::GetInstance().DrawDebugSphere(waypoints[i], 0.2f, color);
            }
            
            // Draw path
            for (size_t i = 0; i < waypoints.size(); ++i) {
                size_t next = (i + 1) % waypoints.size();
                TestRunner::GetInstance().DrawDebugLine(waypoints[i], waypoints[next], Math::Vec3(0.0f, 0.0f, 1.0f));
            }
            
            // Draw current path to target
            TestRunner::GetInstance().DrawDebugLine(aiPosition, target, Math::Vec3(1.0f, 1.0f, 0.0f));
            
            TestRunner::GetInstance().DrawDebugText("AI Agent", aiPosition + Math::Vec3(0.0f, 1.0f, 0.0f));
        },
        []() {},
        []() {
            Logger::GetInstance().Info("Cleaning up AI pathfinding test");
        }
    });
}

// Command line interface for running tests
void RunTestFromCommandLine(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Available visual tests:\n";
        auto tests = TestRunner::GetInstance().GetAvailableTests();
        for (const auto& test : tests) {
            std::cout << "  - " << test << "\n";
        }
        return;
    }
    
    std::string command = argv[1];
    
    if (command == "--list-tests") {
        std::cout << "Available visual tests:\n";
        auto tests = TestRunner::GetInstance().GetAvailableTests();
        for (const auto& test : tests) {
            std::cout << "  - " << test << "\n";
        }
    }
    else if (command == "--run-test" && argc >= 3) {
        std::string testName = argv[2];
        TestRunner::GetInstance().RunTest(testName);
        std::cout << "Running visual test: " << testName << "\n";
        std::cout << "Press any key to stop...\n";
        
        // Simple test loop (in real implementation, this would be integrated with the main game loop)
        for (int i = 0; i < 300; ++i) { // 5 seconds at 60fps
            TestRunner::GetInstance().Update(1.0f / 60.0f);
            TestRunner::GetInstance().Render();
            
            // Simple delay (in real implementation, this would be frame-rate controlled)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        TestRunner::GetInstance().StopCurrentTest();
    }
    else if (command == "--run-all-tests") {
        TestRunner::GetInstance().RunAllTests();
    }
    else if (command == "--debug-overlay") {
        TestRunner::GetInstance().EnableDebugOverlay(true);
        std::cout << "Debug overlay enabled\n";
    }
    else {
        std::cout << "Unknown command: " << command << "\n";
        std::cout << "Available commands:\n";
        std::cout << "  --list-tests\n";
        std::cout << "  --run-test <test_name>\n";
        std::cout << "  --run-all-tests\n";
        std::cout << "  --debug-overlay\n";
    }
}

// Auto-registration of tests
static bool testsRegistered = []() {
    RegisterPhysicsVisualizationTests();
    RegisterRenderingTests();
    RegisterAITests();
    return true;
}();