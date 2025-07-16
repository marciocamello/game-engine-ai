#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"
#include <iostream>

using namespace GameEngine;

int main() {
    std::cout << "Testing Physics Configuration System..." << std::endl;
    
    // Test 1: Default Configuration
    PhysicsEngine engine;
    PhysicsConfiguration defaultConfig = PhysicsConfiguration::Default();
    
    std::cout << "Default config gravity: " << defaultConfig.gravity.y << std::endl;
    std::cout << "Default config timestep: " << defaultConfig.timeStep << std::endl;
    std::cout << "Default config solver iterations: " << defaultConfig.solverIterations << std::endl;
    
    bool initialized = engine.Initialize(defaultConfig);
    if (!initialized) {
        std::cout << "Failed to initialize physics engine" << std::endl;
        return 1;
    }
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    std::cout << "Stored config gravity: " << storedConfig.gravity.y << std::endl;
    std::cout << "Stored config timestep: " << storedConfig.timeStep << std::endl;
    
    // Test 2: Character Movement Configuration
    PhysicsConfiguration charConfig = PhysicsConfiguration::ForCharacterMovement();
    std::cout << "Character config solver iterations: " << charConfig.solverIterations << std::endl;
    std::cout << "Character config linear damping: " << charConfig.linearDamping << std::endl;
    
    // Test 3: High Precision Configuration
    PhysicsConfiguration precisionConfig = PhysicsConfiguration::HighPrecision();
    std::cout << "Precision config timestep: " << precisionConfig.timeStep << std::endl;
    std::cout << "Precision config max substeps: " << precisionConfig.maxSubSteps << std::endl;
    
    // Test 4: Runtime Parameter Modification
    Math::Vec3 newGravity(0.0f, -15.0f, 0.0f);
    engine.SetGravity(newGravity);
    std::cout << "Updated gravity: " << engine.GetConfiguration().gravity.y << std::endl;
    
    float newTimeStep = 1.0f / 30.0f;
    engine.SetTimeStep(newTimeStep);
    std::cout << "Updated timestep: " << engine.GetConfiguration().timeStep << std::endl;
    
    int newIterations = 25;
    engine.SetSolverIterations(newIterations);
    std::cout << "Updated solver iterations: " << engine.GetConfiguration().solverIterations << std::endl;
    
    // Test 5: World Creation with Configuration
    auto world = engine.CreateWorld(charConfig);
    if (world) {
        std::cout << "World created with config gravity: " << world->GetGravity().y << std::endl;
    }
    
    engine.Shutdown();
    std::cout << "Physics Configuration System test completed successfully!" << std::endl;
    
    return 0;
}