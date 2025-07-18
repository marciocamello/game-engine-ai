#include "Game/Character.h"
#include "Game/CharacterController.h"
#include "Physics/PhysicsEngine.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"
#include <iostream>
#include <memory>
#include <cmath>

using namespace GameEngine;

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

    void RunBehaviorTests() {
        std::cout << "=== Simple Character Behavior Tests ===" << std::endl;
        
        int passedTests = 0;
        int totalTests = 0;
        
        if (TestCharacterInitialization()) passedTests++; totalTests++;
        if (TestCharacterProperties()) passedTests++; totalTests++;
        if (TestCharacterMovementComponents()) passedTests++; totalTests++;
        if (TestCharacterControllerCompatibility()) passedTests++; totalTests++;
        if (TestCharacterUpdate()) passedTests++; totalTests++;
        
        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Passed: " << passedTests << "/" << totalTests << " tests" << std::endl;
        std::cout << "Success Rate: " << (100.0 * passedTests / totalTests) << "%" << std::endl;
        
        if (passedTests == totalTests) {
            std::cout << "✅ All character behavior tests passed!" << std::endl;
        } else {
            std::cout << "❌ Some tests failed. Review the output above for details." << std::endl;
        }
    }

private:
    std::unique_ptr<PhysicsEngine> m_physicsEngine;
    std::unique_ptr<InputManager> m_inputManager;
    
    bool TestCharacterInitialization() {
        std::cout << "\nTesting Character Initialization..." << std::endl;
        
        try {
            auto character = std::make_unique<Character>();
            bool initSuccess = character->Initialize(m_physicsEngine.get());
            
            std::cout << "Character initialization: " << (initSuccess ? "SUCCESS" : "FAILED") << std::endl;
            
            // Test initial state
            Math::Vec3 initialPos = character->GetPosition();
            float initialSpeed = character->GetMoveSpeed();
            
            std::cout << "Initial position: (" << initialPos.x << ", " << initialPos.y << ", " << initialPos.z << ")" << std::endl;
            std::cout << "Initial move speed: " << initialSpeed << std::endl;
            
            bool success = initSuccess && initialSpeed > 0.0f;
            std::cout << "Character initialization test: " << (success ? "PASS" : "FAIL") << std::endl;
            
            return success;
        }
        catch (const std::exception& e) {
            std::cout << "Character initialization test FAILED with exception: " << e.what() << std::endl;
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
};

int main() {
    try {
        SimpleCharacterBehaviorTest test;
        test.RunBehaviorTests();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Character behavior test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}