#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <iostream>
#include <cassert>

using namespace GameEngine;

void TestDefaultConfiguration() {
    std::cout << "Testing default physics configuration..." << std::endl;
    
    PhysicsEngine engine;
    PhysicsConfiguration defaultConfig = PhysicsConfiguration::Default();
    
    // Test default values
    assert(defaultConfig.gravity.y == -9.81f);
    assert(defaultConfig.timeStep == 1.0f / 60.0f);
    assert(defaultConfig.maxSubSteps == 10);
    assert(defaultConfig.solverIterations == 10);
    assert(defaultConfig.enableCCD == true);
    
    bool initialized = engine.Initialize(defaultConfig);
    assert(initialized);
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    assert(storedConfig.gravity.y == -9.81f);
    assert(storedConfig.timeStep == 1.0f / 60.0f);
    
    engine.Shutdown();
    std::cout << "Default configuration test passed!" << std::endl;
}

void TestCharacterMovementConfiguration() {
    std::cout << "Testing character movement physics configuration..." << std::endl;
    
    PhysicsEngine engine;
    PhysicsConfiguration charConfig = PhysicsConfiguration::ForCharacterMovement();
    
    // Test character movement specific values
    assert(charConfig.solverIterations == 15); // Higher for stability
    assert(charConfig.linearDamping == 0.1f);
    assert(charConfig.angularDamping == 0.1f);
    
    bool initialized = engine.Initialize(charConfig);
    assert(initialized);
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    assert(storedConfig.solverIterations == 15);
    assert(storedConfig.linearDamping == 0.1f);
    
    engine.Shutdown();
    std::cout << "Character movement configuration test passed!" << std::endl;
}

void TestHighPrecisionConfiguration() {
    std::cout << "Testing high precision physics configuration..." << std::endl;
    
    PhysicsEngine engine;
    PhysicsConfiguration precisionConfig = PhysicsConfiguration::HighPrecision();
    
    // Test high precision specific values
    assert(precisionConfig.timeStep == 1.0f / 120.0f); // Higher frequency
    assert(precisionConfig.maxSubSteps == 20);
    assert(precisionConfig.solverIterations == 20);
    assert(precisionConfig.contactBreakingThreshold == 0.01f);
    
    bool initialized = engine.Initialize(precisionConfig);
    assert(initialized);
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    assert(storedConfig.timeStep == 1.0f / 120.0f);
    assert(storedConfig.maxSubSteps == 20);
    
    engine.Shutdown();
    std::cout << "High precision configuration test passed!" << std::endl;
}

void TestRuntimeParameterModification() {
    std::cout << "Testing runtime parameter modification..." << std::endl;
    
    PhysicsEngine engine;
    bool initialized = engine.Initialize();
    assert(initialized);
    
    // Test gravity modification
    Math::Vec3 newGravity(0.0f, -15.0f, 0.0f);
    engine.SetGravity(newGravity);
    assert(engine.GetConfiguration().gravity.y == -15.0f);
    
    // Test timestep modification
    float newTimeStep = 1.0f / 30.0f;
    engine.SetTimeStep(newTimeStep);
    assert(engine.GetConfiguration().timeStep == newTimeStep);
    
    // Test solver iterations modification
    int newIterations = 25;
    engine.SetSolverIterations(newIterations);
    assert(engine.GetConfiguration().solverIterations == newIterations);
    
    // Test contact thresholds modification
    float newBreaking = 0.05f;
    float newProcessing = 0.025f;
    engine.SetContactThresholds(newBreaking, newProcessing);
    assert(engine.GetConfiguration().contactBreakingThreshold == newBreaking);
    assert(engine.GetConfiguration().contactProcessingThreshold == newProcessing);
    
    engine.Shutdown();
    std::cout << "Runtime parameter modification test passed!" << std::endl;
}

void TestWorldCreationWithConfiguration() {
    std::cout << "Testing world creation with configuration..." << std::endl;
    
    PhysicsEngine engine;
    bool initialized = engine.Initialize();
    assert(initialized);
    
    // Create world with custom configuration
    PhysicsConfiguration customConfig = PhysicsConfiguration::ForCharacterMovement();
    customConfig.gravity = Math::Vec3(0.0f, -12.0f, 0.0f);
    
    auto world = engine.CreateWorld(customConfig);
    assert(world != nullptr);
    assert(world->GetGravity().y == -12.0f);
    
    engine.SetActiveWorld(world);
    
    engine.Shutdown();
    std::cout << "World creation with configuration test passed!" << std::endl;
}

int main() {
    try {
        std::cout << "Starting Physics Configuration Tests..." << std::endl;
        
        TestDefaultConfiguration();
        TestCharacterMovementConfiguration();
        TestHighPrecisionConfiguration();
        TestRuntimeParameterModification();
        TestWorldCreationWithConfiguration();
        
        std::cout << "All physics configuration tests passed!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}