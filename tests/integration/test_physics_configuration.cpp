#include "Physics/PhysicsEngine.h"
#include "TestUtils.h"
#include "Core/Logger.h"
#include <iostream>
#include <cassert>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test default physics configuration
 * Requirements: Physics system configuration
 */
bool TestDefaultConfiguration() {
    TestOutput::PrintTestStart("default physics configuration");
    
    PhysicsEngine engine;
    PhysicsConfiguration defaultConfig = PhysicsConfiguration::Default();
    
    // Test default values
    EXPECT_NEARLY_EQUAL(defaultConfig.gravity.y, -9.81f);
    EXPECT_NEARLY_EQUAL(defaultConfig.timeStep, 1.0f / 60.0f);
    EXPECT_EQUAL(defaultConfig.maxSubSteps, 10);
    EXPECT_EQUAL(defaultConfig.solverIterations, 10);
    EXPECT_TRUE(defaultConfig.enableCCD);
    
    bool initialized = engine.Initialize(defaultConfig);
    EXPECT_TRUE(initialized);
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    EXPECT_NEARLY_EQUAL(storedConfig.gravity.y, -9.81f);
    EXPECT_NEARLY_EQUAL(storedConfig.timeStep, 1.0f / 60.0f);
    
    engine.Shutdown();
    
    TestOutput::PrintTestPass("default physics configuration");
    return true;
}

/**
 * Test character movement physics configuration
 * Requirements: Physics system configuration for character movement
 */
bool TestCharacterMovementConfiguration() {
    TestOutput::PrintTestStart("character movement physics configuration");
    
    PhysicsEngine engine;
    PhysicsConfiguration charConfig = PhysicsConfiguration::ForCharacterMovement();
    
    // Test character movement specific values
    EXPECT_EQUAL(charConfig.solverIterations, 15);
    EXPECT_NEARLY_EQUAL(charConfig.linearDamping, 0.1f);
    EXPECT_NEARLY_EQUAL(charConfig.angularDamping, 0.1f);
    
    bool initialized = engine.Initialize(charConfig);
    EXPECT_TRUE(initialized);
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    EXPECT_EQUAL(storedConfig.solverIterations, 15);
    EXPECT_NEARLY_EQUAL(storedConfig.linearDamping, 0.1f);
    
    engine.Shutdown();
    
    TestOutput::PrintTestPass("character movement physics configuration");
    return true;
}

/**
 * Test high precision physics configuration
 * Requirements: Physics system high precision configuration
 */
bool TestHighPrecisionConfiguration() {
    TestOutput::PrintTestStart("high precision physics configuration");
    
    PhysicsEngine engine;
    PhysicsConfiguration precisionConfig = PhysicsConfiguration::HighPrecision();
    
    // Test high precision specific values
    EXPECT_NEARLY_EQUAL(precisionConfig.timeStep, 1.0f / 120.0f);
    EXPECT_EQUAL(precisionConfig.maxSubSteps, 20);
    EXPECT_EQUAL(precisionConfig.solverIterations, 20);
    EXPECT_NEARLY_EQUAL(precisionConfig.contactBreakingThreshold, 0.01f);
    
    bool initialized = engine.Initialize(precisionConfig);
    EXPECT_TRUE(initialized);
    
    // Verify configuration was stored
    const PhysicsConfiguration& storedConfig = engine.GetConfiguration();
    EXPECT_NEARLY_EQUAL(storedConfig.timeStep, 1.0f / 120.0f);
    EXPECT_EQUAL(storedConfig.maxSubSteps, 20);
    
    engine.Shutdown();
    
    TestOutput::PrintTestPass("high precision physics configuration");
    return true;
}

/**
 * Test runtime parameter modification
 * Requirements: Physics system runtime configuration changes
 */
bool TestRuntimeParameterModification() {
    TestOutput::PrintTestStart("runtime parameter modification");
    
    PhysicsEngine engine;
    bool initialized = engine.Initialize();
    EXPECT_TRUE(initialized);
    
    // Test gravity modification
    Math::Vec3 newGravity(0.0f, -15.0f, 0.0f);
    engine.SetGravity(newGravity);
    EXPECT_NEARLY_EQUAL(engine.GetConfiguration().gravity.y, -15.0f);
    
    // Test timestep modification
    float newTimeStep = 1.0f / 30.0f;
    engine.SetTimeStep(newTimeStep);
    EXPECT_NEARLY_EQUAL(engine.GetConfiguration().timeStep, newTimeStep);
    
    // Test solver iterations modification
    int newIterations = 25;
    engine.SetSolverIterations(newIterations);
    EXPECT_EQUAL(engine.GetConfiguration().solverIterations, newIterations);
    
    // Test contact thresholds modification
    float newBreaking = 0.05f;
    float newProcessing = 0.025f;
    engine.SetContactThresholds(newBreaking, newProcessing);
    EXPECT_NEARLY_EQUAL(engine.GetConfiguration().contactBreakingThreshold, newBreaking);
    EXPECT_NEARLY_EQUAL(engine.GetConfiguration().contactProcessingThreshold, newProcessing);
    
    engine.Shutdown();
    
    TestOutput::PrintTestPass("runtime parameter modification");
    return true;
}

/**
 * Test world creation with configuration
 * Requirements: Physics world creation with custom configuration
 */
bool TestWorldCreationWithConfiguration() {
    TestOutput::PrintTestStart("world creation with configuration");
    
    PhysicsEngine engine;
    bool initialized = engine.Initialize();
    EXPECT_TRUE(initialized);
    
    // Create world with custom configuration
    PhysicsConfiguration customConfig = PhysicsConfiguration::ForCharacterMovement();
    customConfig.gravity = Math::Vec3(0.0f, -12.0f, 0.0f);
    
    auto world = engine.CreateWorld(customConfig);
    EXPECT_NOT_NULL(world);
    EXPECT_NEARLY_EQUAL(world->GetGravity().y, -12.0f);
    
    engine.SetActiveWorld(world);
    
    engine.Shutdown();
    
    TestOutput::PrintTestPass("world creation with configuration");
    return true;
}

int main() {
    TestOutput::PrintHeader("Physics Configuration Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Physics Configuration Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Default Configuration", TestDefaultConfiguration);
        allPassed &= suite.RunTest("Character Movement Configuration", TestCharacterMovementConfiguration);
        allPassed &= suite.RunTest("High Precision Configuration", TestHighPrecisionConfiguration);
        allPassed &= suite.RunTest("Runtime Parameter Modification", TestRuntimeParameterModification);
        allPassed &= suite.RunTest("World Creation with Configuration", TestWorldCreationWithConfiguration);

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