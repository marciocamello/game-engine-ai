#include "Game/Character.h"
#include "Game/CharacterController.h"
#include "Physics/PhysicsEngine.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <iostream>
#include <memory>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * @brief Simple character behavior test
 * 
 * Tests basic character functionality:
 * - Character initialization
 * - Position and movement properties
 * - Movement component switching
 * - Basic API compatibility
 */
class SimpleCharacterBehaviorTest {
public:
    SimpleCharacterBehaviorTest() {
        m_physicsEngine = std::make_unique<PhysicsEngine>();
        if (!m_physicsEngine->Initialize()) {
            LOG_ERROR("Failed to initialize physics engine for character test");
        }
        
        m_inputManager = std::make_unique<InputManager>();
        if (!m_inputManager->Initialize(nullptr)) {
            LOG_ERROR("Failed to initialize input manager for character test");
        }
    }

    ~SimpleCharacterBehaviorTest() {
        if (m_inputManager) {
            m_inputManager->Shutdown();
        }
        if (m_physicsEngine) {
            m_physicsEngine->Shutdown();
        }
    }

    bool RunBehaviorTests() {
        TestOutput::PrintInfo("Starting Simple Character Behavior Tests");
        
        bool allPassed = true;
        allPassed &= TestCharacterInitialization();
        allPassed &= TestCharacterProperties();
        allPassed &= TestCharacterMovementComponents();
        allPassed &= TestCharacterControllerCompatibility();
        allPassed &= TestCharacterUpdate();
        allPassed &= TestModelOffsetSystem();
        
        TestOutput::PrintInfo("Character Behavior Tests Complete");
        return allPassed;
    }

private:
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    std::unique_ptr<InputManager> m_inputManager;
    
    bool TestCharacterInitialization() {
        TestOutput::PrintTestStart("character initialization");
        
        try {
            auto character = std::make_unique<Character>();
            bool initSuccess = character->Initialize(m_physicsEngine.get());
            
            EXPECT_TRUE(initSuccess);
            
            // Test initial state
            Math::Vec3 initialPos = character->GetPosition();
            float initialSpeed = character->GetMoveSpeed();
            
            TestOutput::PrintInfo("Initial position: (" + std::to_string(initialPos.x) + ", " + 
                                 std::to_string(initialPos.y) + ", " + std::to_string(initialPos.z) + ")");
            TestOutput::PrintInfo("Initial move speed: " + std::to_string(initialSpeed));
            
            EXPECT_TRUE(initialSpeed > 0.0f);
            
            TestOutput::PrintTestPass("character initialization");
            return true;
        }
        catch (const std::exception& e) {
            TestOutput::PrintError("Character initialization exception: " + std::string(e.what()));
            return false;
        }
    }
    
    bool TestCharacterProperties() {
        std::cout << "\nTesting Character Properties..." << std::endl;
        
        try {
            auto character = std::make_unique<Character>();
            character->Initialize(m_physicsEngine.get());
            
            // Test position setting/getting
            Math::Vec3 testPosition(5.0f, 2.0f, 3.0f);
            character->SetPosition(testPosition);
            Math::Vec3 retrievedPosition = character->GetPosition();
            
            bool positionCorrect = (std::abs(retrievedPosition.x - testPosition.x) < 0.1f) &&
                                  (std::abs(retrievedPosition.y - testPosition.y) < 0.1f) &&
                                  (std::abs(retrievedPosition.z - testPosition.z) < 0.1f);
            
            std::cout << "Set position: (" << testPosition.x << ", " << testPosition.y << ", " << testPosition.z << ")" << std::endl;
            std::cout << "Got position: (" << retrievedPosition.x << ", " << retrievedPosition.y << ", " << retrievedPosition.z << ")" << std::endl;
            
            // Test move speed setting/getting
            float testSpeed = 8.5f;
            character->SetMoveSpeed(testSpeed);
            float retrievedSpeed = character->GetMoveSpeed();
            
            bool speedCorrect = std::abs(retrievedSpeed - testSpeed) < 0.1f;
            
            std::cout << "Set move speed: " << testSpeed << std::endl;
            std::cout << "Got move speed: " << retrievedSpeed << std::endl;
            
            // Test character dimensions
            float height = character->GetHeight();
            float radius = character->GetRadius();
            
            bool dimensionsValid = (height > 0.0f) && (radius > 0.0f);
            
            std::cout << "Character height: " << height << std::endl;
            std::cout << "Character radius: " << radius << std::endl;
            
            bool success = positionCorrect && speedCorrect && dimensionsValid;
            std::cout << "Character properties test: " << (success ? "PASS" : "FAIL") << std::endl;
            
            return success;
        }
        catch (const std::exception& e) {
            std::cout << "Character properties test FAILED with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool TestCharacterMovementComponents() {
        std::cout << "\nTesting Character Movement Components..." << std::endl;
        
        try {
            auto character = std::make_unique<Character>();
            character->Initialize(m_physicsEngine.get());
            
            // Test getting current movement type
            const char* initialType = character->GetMovementTypeName();
            std::cout << "Initial movement type: " << initialType << std::endl;
            
            // Test movement component switching
            character->SwitchToCharacterMovement();
            const char* characterType = character->GetMovementTypeName();
            std::cout << "After switch to character movement: " << characterType << std::endl;
            
            character->SwitchToPhysicsMovement();
            const char* physicsType = character->GetMovementTypeName();
            std::cout << "After switch to physics movement: " << physicsType << std::endl;
            
            character->SwitchToHybridMovement();
            const char* hybridType = character->GetMovementTypeName();
            std::cout << "After switch to hybrid movement: " << hybridType << std::endl;
            
            // Test movement type colors
            Math::Vec4 color = character->GetMovementTypeColor();
            std::cout << "Movement type color: (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;
            
            // Verify that switching actually changes the type name
            bool switchingWorks = (std::string(characterType) != std::string(physicsType)) &&
                                 (std::string(physicsType) != std::string(hybridType));
            
            bool success = switchingWorks && (initialType != nullptr) && (characterType != nullptr) && 
                          (physicsType != nullptr) && (hybridType != nullptr);
            
            std::cout << "Movement component switching test: " << (success ? "PASS" : "FAIL") << std::endl;
            
            return success;
        }
        catch (const std::exception& e) {
            std::cout << "Movement components test FAILED with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool TestCharacterControllerCompatibility() {
        std::cout << "\nTesting CharacterController Compatibility..." << std::endl;
        
        try {
            auto controller = std::make_unique<CharacterController>();
            bool initSuccess = controller->Initialize(m_physicsEngine.get());
            
            std::cout << "CharacterController initialization: " << (initSuccess ? "SUCCESS" : "FAILED") << std::endl;
            
            if (!initSuccess) {
                std::cout << "CharacterController compatibility test: FAIL" << std::endl;
                return false;
            }
            
            // Test basic properties
            controller->SetPosition(Math::Vec3(1.0f, 2.0f, 3.0f));
            Math::Vec3 pos = controller->GetPosition();
            
            controller->SetMoveSpeed(7.0f);
            float speed = controller->GetMoveSpeed();
            
            // Test movement type switching
            controller->SwitchToCharacterMovement();
            const char* characterType = controller->GetMovementTypeName();
            
            controller->SwitchToPhysicsMovement();
            const char* physicsType = controller->GetMovementTypeName();
            
            controller->SwitchToHybridMovement();
            const char* hybridType = controller->GetMovementTypeName();
            
            std::cout << "CharacterController position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            std::cout << "CharacterController speed: " << speed << std::endl;
            std::cout << "Movement types: " << characterType << ", " << physicsType << ", " << hybridType << std::endl;
            
            bool success = initSuccess && (speed > 0.0f) && 
                          (characterType != nullptr) && (physicsType != nullptr) && (hybridType != nullptr);
            
            std::cout << "CharacterController compatibility test: " << (success ? "PASS" : "FAIL") << std::endl;
            
            return success;
        }
        catch (const std::exception& e) {
            std::cout << "CharacterController compatibility test FAILED with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool TestCharacterUpdate() {
        std::cout << "\nTesting Character Update..." << std::endl;
        
        try {
            auto character = std::make_unique<Character>();
            character->Initialize(m_physicsEngine.get());
            
            Math::Vec3 initialPos = character->GetPosition();
            std::cout << "Initial position: (" << initialPos.x << ", " << initialPos.y << ", " << initialPos.z << ")" << std::endl;
            
            // Test character update (basic functionality)
            float deltaTime = 1.0f / 60.0f;
            
            // Update character multiple times
            for (int i = 0; i < 60; ++i) {
                character->Update(deltaTime, m_inputManager.get());
            }
            
            Math::Vec3 finalPos = character->GetPosition();
            std::cout << "Final position after updates: (" << finalPos.x << ", " << finalPos.y << ", " << finalPos.z << ")" << std::endl;
            
            // Test movement state queries
            bool isGrounded = character->IsGrounded();
            bool isJumping = character->IsJumping();
            bool isFalling = character->IsFalling();
            
            std::cout << "Movement state - Grounded: " << (isGrounded ? "true" : "false") 
                      << ", Jumping: " << (isJumping ? "true" : "false")
                      << ", Falling: " << (isFalling ? "true" : "false") << std::endl;
            
            // Test fall detection
            character->SetFallLimit(-5.0f);
            float fallLimit = character->GetFallLimit();
            bool hasFallen = character->HasFallen();
            
            std::cout << "Fall limit: " << fallLimit << ", Has fallen: " << (hasFallen ? "true" : "false") << std::endl;
            
            // Test spawn position
            Math::Vec3 spawnPos = character->GetSpawnPosition();
            character->SetSpawnPosition(Math::Vec3(10.0f, 5.0f, 0.0f));
            Math::Vec3 newSpawnPos = character->GetSpawnPosition();
            
            std::cout << "Original spawn: (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << std::endl;
            std::cout << "New spawn: (" << newSpawnPos.x << ", " << newSpawnPos.y << ", " << newSpawnPos.z << ")" << std::endl;
            
            bool success = (std::abs(fallLimit - (-5.0f)) < 0.1f) && 
                          (std::abs(newSpawnPos.x - 10.0f) < 0.1f);
            
            std::cout << "Character update test: " << (success ? "PASS" : "FAIL") << std::endl;
            
            return success;
        }
        catch (const std::exception& e) {
            std::cout << "Character update test FAILED with exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool TestModelOffsetSystem() {
        std::cout << "\nTesting Model Offset System..." << std::endl;
        
        try {
            auto character = std::make_unique<Character>();
            character->Initialize(m_physicsEngine.get());
            
            // Test default model offset
            Math::Vec3 defaultOffset = character->GetModelOffset();
            std::cout << "Default model offset: (" << defaultOffset.x << ", " << defaultOffset.y << ", " << defaultOffset.z << ")" << std::endl;
            
            // Test setting custom model offset
            Math::Vec3 testOffset(1.0f, -0.5f, 0.2f);
            character->SetModelOffset(testOffset);
            Math::Vec3 retrievedOffset = character->GetModelOffset();
            
            bool offsetCorrect = (std::abs(retrievedOffset.x - testOffset.x) < 0.001f) &&
                                (std::abs(retrievedOffset.y - testOffset.y) < 0.001f) &&
                                (std::abs(retrievedOffset.z - testOffset.z) < 0.001f);
            
            std::cout << "Set offset: (" << testOffset.x << ", " << testOffset.y << ", " << testOffset.z << ")" << std::endl;
            std::cout << "Got offset: (" << retrievedOffset.x << ", " << retrievedOffset.y << ", " << retrievedOffset.z << ")" << std::endl;
            
            // Test model offset configuration structure
            auto centeredConfig = ModelOffsetConfiguration::CenteredInCapsule();
            character->SetModelOffsetConfiguration(centeredConfig);
            Math::Vec3 centeredOffset = character->GetModelOffset();
            
            std::cout << "Centered in capsule offset: (" << centeredOffset.x << ", " << centeredOffset.y << ", " << centeredOffset.z << ")" << std::endl;
            
            // Test default configuration
            auto defaultConfig = ModelOffsetConfiguration::Default();
            character->SetModelOffsetConfiguration(defaultConfig);
            Math::Vec3 defaultConfigOffset = character->GetModelOffset();
            
            bool defaultConfigCorrect = (std::abs(defaultConfigOffset.x) < 0.001f) &&
                                       (std::abs(defaultConfigOffset.y) < 0.001f) &&
                                       (std::abs(defaultConfigOffset.z) < 0.001f);
            
            std::cout << "Default config offset: (" << defaultConfigOffset.x << ", " << defaultConfigOffset.y << ", " << defaultConfigOffset.z << ")" << std::endl;
            
            // Test custom configuration
            Math::Vec3 customOffsetValue(2.0f, -1.0f, 0.5f);
            auto customConfig = ModelOffsetConfiguration::Custom(customOffsetValue);
            character->SetModelOffsetConfiguration(customConfig);
            Math::Vec3 customConfigOffset = character->GetModelOffset();
            
            bool customConfigCorrect = (std::abs(customConfigOffset.x - customOffsetValue.x) < 0.001f) &&
                                      (std::abs(customConfigOffset.y - customOffsetValue.y) < 0.001f) &&
                                      (std::abs(customConfigOffset.z - customOffsetValue.z) < 0.001f);
            
            std::cout << "Custom config offset: (" << customConfigOffset.x << ", " << customConfigOffset.y << ", " << customConfigOffset.z << ")" << std::endl;
            
            // Test getting configuration
            auto retrievedConfig = character->GetModelOffsetConfiguration();
            bool configRetrievalCorrect = (std::abs(retrievedConfig.offset.x - customOffsetValue.x) < 0.001f) &&
                                         (std::abs(retrievedConfig.offset.y - customOffsetValue.y) < 0.001f) &&
                                         (std::abs(retrievedConfig.offset.z - customOffsetValue.z) < 0.001f);
            
            // Test that model offset works with different movement components
            character->SwitchToCharacterMovement();
            Math::Vec3 offsetWithCharacterMovement = character->GetModelOffset();
            
            character->SwitchToPhysicsMovement();
            Math::Vec3 offsetWithPhysicsMovement = character->GetModelOffset();
            
            character->SwitchToHybridMovement();
            Math::Vec3 offsetWithHybridMovement = character->GetModelOffset();
            
            bool offsetPersistsAcrossComponents = 
                (std::abs(offsetWithCharacterMovement.x - customOffsetValue.x) < 0.001f) &&
                (std::abs(offsetWithPhysicsMovement.x - customOffsetValue.x) < 0.001f) &&
                (std::abs(offsetWithHybridMovement.x - customOffsetValue.x) < 0.001f);
            
            std::cout << "Offset with CharacterMovement: (" << offsetWithCharacterMovement.x << ", " << offsetWithCharacterMovement.y << ", " << offsetWithCharacterMovement.z << ")" << std::endl;
            std::cout << "Offset with PhysicsMovement: (" << offsetWithPhysicsMovement.x << ", " << offsetWithPhysicsMovement.y << ", " << offsetWithPhysicsMovement.z << ")" << std::endl;
            std::cout << "Offset with HybridMovement: (" << offsetWithHybridMovement.x << ", " << offsetWithHybridMovement.y << ", " << offsetWithHybridMovement.z << ")" << std::endl;
            
            bool success = offsetCorrect && defaultConfigCorrect && customConfigCorrect && 
                          configRetrievalCorrect && offsetPersistsAcrossComponents;
            
            std::cout << "Model offset system test: " << (success ? "PASS" : "FAIL") << std::endl;
            
            return success;
        }
        catch (const std::exception& e) {
            std::cout << "Model offset system test FAILED with exception: " << e.what() << std::endl;
            return false;
        }
    }
};

int main() {
    TestOutput::PrintHeader("Character Behavior Simple Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Character Behavior Simple Integration Tests");
        
        SimpleCharacterBehaviorTest test;
        bool testResult = test.RunBehaviorTests();
        
        allPassed &= suite.RunTest("Character Behavior Tests", [testResult]() { return testResult; });

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