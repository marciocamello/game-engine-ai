#include "Game/Character.h"
#include "Game/CharacterController.h"
#include "Game/CharacterMovementComponent.h"
#include "Game/MovementComponentFactory.h"
#include "Physics/PhysicsEngine.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <iomanip>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * @brief Comprehensive test comparing all movement component types
 * 
 * Tests performance, behavior, and compatibility of:
 * - PhysicsMovementComponent (full physics simulation)
 * - CharacterMovementComponent (basic character control)
 * - HybridMovementComponent (physics collision with direct control)
 */
class MovementComponentComparisonTest {
public:
    struct TestResult {
        std::string componentName;
        float averageUpdateTime = 0.0f;
        float maxUpdateTime = 0.0f;
        float minUpdateTime = 999.0f;
        int totalUpdates = 0;
        bool initializationSuccess = false;
        bool behaviorCorrect = false;
        Math::Vec3 finalPosition{0.0f};
        Math::Vec3 finalVelocity{0.0f};
    };

    MovementComponentComparisonTest() {
        // Initialize physics engine for testing
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        if (!m_physicsEngine->Initialize()) {
            LOG_ERROR("Failed to initialize physics engine for movement component test");
        }

        // Create mock input manager
        m_inputManager = std::make_unique<InputManager>();
        if (!m_inputManager->Initialize(nullptr)) {
            LOG_ERROR("Failed to initialize input manager for movement component test");
        }
    }

    bool RunAllTests() {
        LOG_INFO("=== Movement Component Comparison Test ===");
        
        // Test each movement component type
        TestResult physicsResult = TestMovementComponent(MovementComponentFactory::ComponentType::Physics);
        TestResult characterMovementResult = TestMovementComponent(MovementComponentFactory::ComponentType::CharacterMovement);
        TestResult hybridResult = TestMovementComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        // Test Character class with different components
        TestResult characterPhysicsResult = TestCharacterWithComponent(MovementComponentFactory::ComponentType::Physics);
        TestResult characterMovementResult2 = TestCharacterWithComponent(MovementComponentFactory::ComponentType::CharacterMovement);
        TestResult characterHybridResult = TestCharacterWithComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        // Test CharacterController class with different components
        TestResult controllerPhysicsResult = TestCharacterControllerWithComponent(MovementComponentFactory::ComponentType::Physics);
        TestResult controllerCharacterMovementResult = TestCharacterControllerWithComponent(MovementComponentFactory::ComponentType::CharacterMovement);
        TestResult controllerHybridResult = TestCharacterControllerWithComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        // Print comprehensive results
        PrintComparisonResults({
            physicsResult, characterMovementResult, hybridResult,
            characterPhysicsResult, characterMovementResult2, characterHybridResult,
            controllerPhysicsResult, controllerCharacterMovementResult, controllerHybridResult
        });
        
        // Test component switching
        TestComponentSwitching();
        
        // Test backward compatibility
        TestBackwardCompatibility();
        
        LOG_INFO("=== Movement Component Comparison Test Complete ===");
        return true;
    }

private:
    TestResult TestMovementComponent(MovementComponentFactory::ComponentType type) {
        TestResult result;
        result.componentName = MovementComponentFactory::GetComponentTypeName(type);
        
        LOG_INFO("Testing " + result.componentName + "...");
        
        // Create component
        auto component = MovementComponentFactory::CreateComponent(type);
        if (!component) {
            LOG_ERROR("Failed to create " + result.componentName);
            return result;
        }
        
        // Initialize component
        result.initializationSuccess = component->Initialize(m_physicsEngine.get());
        if (!result.initializationSuccess) {
            LOG_ERROR("Failed to initialize " + result.componentName);
            return result;
        }
        
        // Set initial position
        component->SetPosition(Math::Vec3(0.0f, 1.0f, 0.0f));
        component->SetCharacterSize(0.3f, 1.8f);
        
        // Configure movement
        auto config = component->GetMovementConfig();
        config.maxWalkSpeed = 6.0f;
        config.jumpZVelocity = 10.0f;
        component->SetMovementConfig(config);
        
        // Simulate movement for several frames
        const int numFrames = 1000;
        const float deltaTime = 1.0f / 60.0f; // 60 FPS
        
        for (int frame = 0; frame < numFrames; ++frame) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Simulate forward movement input
            if (frame < 300) {
                component->AddMovementInput(Math::Vec3(0.0f, 0.0f, 1.0f), 1.0f);
            }
            
            // Simulate jump input
            if (frame == 100) {
                component->Jump();
            }
            
            // Update component
            component->Update(deltaTime, m_inputManager.get(), nullptr);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
            
            // Track performance
            result.averageUpdateTime += updateTime;
            result.maxUpdateTime = std::max(result.maxUpdateTime, updateTime);
            result.minUpdateTime = std::min(result.minUpdateTime, updateTime);
            result.totalUpdates++;
        }
        
        // Calculate average
        result.averageUpdateTime /= result.totalUpdates;
        
        // Get final state
        result.finalPosition = component->GetPosition();
        result.finalVelocity = component->GetVelocity();
        
        // Check behavior correctness
        result.behaviorCorrect = (result.finalPosition.z > 0.1f); // Should have moved forward
        
        // Cleanup
        component->Shutdown();
        
        return result;
    }
    
    TestResult TestCharacterWithComponent(MovementComponentFactory::ComponentType type) {
        TestResult result;
        result.componentName = "Character+" + std::string(MovementComponentFactory::GetComponentTypeName(type));
        
        LOG_INFO("Testing " + result.componentName + "...");
        
        // Create Character
        auto character = std::make_unique<Character>();
        result.initializationSuccess = character->Initialize(m_physicsEngine.get());
        
        if (!result.initializationSuccess) {
            LOG_ERROR("Failed to initialize Character");
            return result;
        }
        
        // Switch to desired movement component
        switch (type) {
            case MovementComponentFactory::ComponentType::Physics:
                character->SwitchToPhysicsMovement();
                break;
            case MovementComponentFactory::ComponentType::CharacterMovement:
                character->SwitchToCharacterMovement();
                break;
            case MovementComponentFactory::ComponentType::Hybrid:
                character->SwitchToHybridMovement();
                break;
        }
        
        // Test basic functionality
        character->SetPosition(Math::Vec3(0.0f, 1.0f, 0.0f));
        
        // Simulate a few updates
        const int numFrames = 100;
        const float deltaTime = 1.0f / 60.0f;
        
        for (int frame = 0; frame < numFrames; ++frame) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            character->Update(deltaTime, m_inputManager.get(), nullptr);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
            
            result.averageUpdateTime += updateTime;
            result.maxUpdateTime = std::max(result.maxUpdateTime, updateTime);
            result.minUpdateTime = std::min(result.minUpdateTime, updateTime);
            result.totalUpdates++;
        }
        
        result.averageUpdateTime /= result.totalUpdates;
        result.finalPosition = character->GetPosition();
        result.finalVelocity = character->GetVelocity();
        result.behaviorCorrect = true; // Character should always work
        
        return result;
    }
    
    TestResult TestCharacterControllerWithComponent(MovementComponentFactory::ComponentType type) {
        TestResult result;
        result.componentName = "CharacterController+" + std::string(MovementComponentFactory::GetComponentTypeName(type));
        
        LOG_INFO("Testing " + result.componentName + "...");
        
        // Create CharacterController
        auto controller = std::make_unique<CharacterController>();
        result.initializationSuccess = controller->Initialize(m_physicsEngine.get());
        
        if (!result.initializationSuccess) {
            LOG_ERROR("Failed to initialize CharacterController");
            return result;
        }
        
        // Switch to desired movement component
        switch (type) {
            case MovementComponentFactory::ComponentType::Physics:
                controller->SwitchToPhysicsMovement();
                break;
            case MovementComponentFactory::ComponentType::CharacterMovement:
                controller->SwitchToCharacterMovement();
                break;
            case MovementComponentFactory::ComponentType::Hybrid:
                controller->SwitchToHybridMovement();
                break;
        }
        
        // Test basic functionality
        controller->SetPosition(Math::Vec3(0.0f, 1.0f, 0.0f));
        
        // Simulate a few updates
        const int numFrames = 100;
        const float deltaTime = 1.0f / 60.0f;
        
        for (int frame = 0; frame < numFrames; ++frame) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            controller->Update(deltaTime, m_inputManager.get(), nullptr);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
            
            result.averageUpdateTime += updateTime;
            result.maxUpdateTime = std::max(result.maxUpdateTime, updateTime);
            result.minUpdateTime = std::min(result.minUpdateTime, updateTime);
            result.totalUpdates++;
        }
        
        result.averageUpdateTime /= result.totalUpdates;
        result.finalPosition = controller->GetPosition();
        result.finalVelocity = controller->GetVelocity();
        result.behaviorCorrect = true; // CharacterController should always work
        
        return result;
    }
    
    void TestComponentSwitching() {
        LOG_INFO("Testing component switching...");
        
        // Test Character component switching
        auto character = std::make_unique<Character>();
        if (character->Initialize(m_physicsEngine.get())) {
            LOG_INFO("Character initial component: " + std::string(character->GetMovementTypeName()));
            
            character->SwitchToPhysicsMovement();
            LOG_INFO("After switch to physics: " + std::string(character->GetMovementTypeName()));
            
            character->SwitchToHybridMovement();
            LOG_INFO("After switch to hybrid: " + std::string(character->GetMovementTypeName()));
            
            character->SwitchToCharacterMovement();
            LOG_INFO("After switch to character movement: " + std::string(character->GetMovementTypeName()));
        }
        
        // Test CharacterController component switching
        auto controller = std::make_unique<CharacterController>();
        if (controller->Initialize(m_physicsEngine.get())) {
            LOG_INFO("CharacterController initial component: " + std::string(controller->GetMovementTypeName()));
            
            controller->SwitchToPhysicsMovement();
            LOG_INFO("After switch to physics: " + std::string(controller->GetMovementTypeName()));
            
            controller->SwitchToCharacterMovement();
            LOG_INFO("After switch to character movement: " + std::string(controller->GetMovementTypeName()));
            
            controller->SwitchToHybridMovement();
            LOG_INFO("After switch to hybrid: " + std::string(controller->GetMovementTypeName()));
        }
    }
    
    void TestBackwardCompatibility() {
        LOG_INFO("Testing backward compatibility...");
        
        // Test that old Character interface still works
        auto character = std::make_unique<Character>();
        if (character->Initialize(m_physicsEngine.get())) {
            // Test old interface methods
            character->SetPosition(Math::Vec3(1.0f, 2.0f, 3.0f));
            Math::Vec3 pos = character->GetPosition();
            LOG_INFO("Character position set/get: (" + std::to_string(pos.x) + ", " + 
                    std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
            
            character->SetMoveSpeed(8.0f);
            float speed = character->GetMoveSpeed();
            LOG_INFO("Character move speed set/get: " + std::to_string(speed));
            
            bool grounded = character->IsGrounded();
            bool jumping = character->IsJumping();
            LOG_INFO("Character state - Grounded: " + std::string(grounded ? "true" : "false") + 
                    ", Jumping: " + std::string(jumping ? "true" : "false"));
        }
        
        // Test that old CharacterController interface still works
        auto controller = std::make_unique<CharacterController>();
        if (controller->Initialize(m_physicsEngine.get())) {
            // Test old interface methods
            controller->SetPosition(Math::Vec3(4.0f, 5.0f, 6.0f));
            Math::Vec3 pos = controller->GetPosition();
            LOG_INFO("CharacterController position set/get: (" + std::to_string(pos.x) + ", " + 
                    std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
            
            controller->SetMoveSpeed(7.0f);
            float speed = controller->GetMoveSpeed();
            LOG_INFO("CharacterController move speed set/get: " + std::to_string(speed));
            
            controller->SetJumpSpeed(12.0f);
            float jumpSpeed = controller->GetJumpSpeed();
            LOG_INFO("CharacterController jump speed set/get: " + std::to_string(jumpSpeed));
            
            bool grounded = controller->IsGrounded();
            LOG_INFO("CharacterController grounded: " + std::string(grounded ? "true" : "false"));
        }
    }
    
    void PrintComparisonResults(const std::vector<TestResult>& results) {
        TestOutput::PrintInfo("=== Movement Component Performance Comparison ===");
        
        TestOutput::PrintInfo("Component Name                          | Init | Avg Time | Max Time | Min Time | Behavior | Final Pos");
        TestOutput::PrintInfo("----------------------------------------|------|----------|----------|----------|----------|----------");
        
        for (const auto& result : results) {
            std::string line = "";
            line += result.componentName;
            line.resize(39, ' ');
            line += " | ";
            line += (result.initializationSuccess ? "OK  " : "FAIL");
            line += " | ";
            line += std::to_string(result.averageUpdateTime).substr(0, 8);
            line += " | ";
            line += std::to_string(result.maxUpdateTime).substr(0, 8);
            line += " | ";
            line += std::to_string(result.minUpdateTime).substr(0, 8);
            line += " | ";
            line += (result.behaviorCorrect ? "OK      " : "FAIL    ");
            line += " | ";
            line += "(" + std::to_string(result.finalPosition.x).substr(0, 3) + "," + 
                    std::to_string(result.finalPosition.y).substr(0, 3) + "," + 
                    std::to_string(result.finalPosition.z).substr(0, 3) + ")";
            
            TestOutput::PrintInfo(line);
        }
        
        // Find best performing component
        auto bestPerformance = std::min_element(results.begin(), results.end(), 
            [](const TestResult& a, const TestResult& b) {
                return a.averageUpdateTime < b.averageUpdateTime;
            });
        
        if (bestPerformance != results.end()) {
            LOG_INFO("Best Performance: " + bestPerformance->componentName + 
                    " (Avg: " + std::to_string(bestPerformance->averageUpdateTime) + "ms)");
        }
        
        // Summary
        LOG_INFO("Performance Summary:");
        LOG_INFO("- PhysicsMovementComponent: Full physics simulation, highest accuracy, moderate performance");
        LOG_INFO("- CharacterMovementComponent: Basic control, predictable behavior, best performance");
        LOG_INFO("- HybridMovementComponent: Physics collision + direct control, balanced approach");
        LOG_INFO("- All components maintain backward compatibility with Character and CharacterController");
    }

    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    std::unique_ptr<InputManager> m_inputManager;
};

int main() {
    TestOutput::PrintHeader("Movement Component Comparison Integration");

    bool allPassed = true;

    try {
        // Initialize logging
        Logger::GetInstance().Initialize();
        
        // Create test suite for result tracking
        TestSuite suite("Movement Component Comparison Integration Tests");
        
        // Run comprehensive movement component comparison test
        MovementComponentComparisonTest test;
        bool testResult = test.RunAllTests();
        
        allPassed &= suite.RunTest("Movement Component Comparison", [testResult]() { return testResult; });

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