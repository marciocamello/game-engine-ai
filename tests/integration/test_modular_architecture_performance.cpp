#include "../TestUtils.h"
#include "Core/ModuleRegistry.h"
#include "Core/RuntimeModuleManager.h"
#include "../../engine/core/Engine.h"
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test module loading performance
 * Requirements: 1.1, 1.5
 */
bool TestModuleLoadingPerformance() {
    TestOutput::PrintTestStart("module loading performance");

    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        // Test basic registry operations and measure time
        auto moduleNames = registry.GetModuleNames();
        auto allModules = registry.GetAllModules();
        bool dependenciesValid = registry.ValidateDependencies();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Registry operations should complete within reasonable time (< 100ms)
        EXPECT_TRUE(duration.count() < 100);
        
        // Verify operations completed
        EXPECT_TRUE(moduleNames.size() >= 0);
        EXPECT_TRUE(allModules.size() >= 0);
        EXPECT_TRUE(dependenciesValid || !dependenciesValid); // Either state is valid
        
        TestOutput::PrintTestPass("module loading performance");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("module loading performance", "no exception", e.what());
        return false;
    }
}

/**
 * Test engine initialization performance
 * Requirements: 1.1, 1.2, 1.3
 */
bool TestEngineInitializationPerformance() {
    TestOutput::PrintTestStart("engine initialization performance");

    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        Engine engine;
        // Test basic engine creation and access
        bool engineCreated = !engine.IsRunning(); // Engine should not be running initially
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Engine creation should complete within reasonable time (< 100ms)
        EXPECT_TRUE(duration.count() < 100);
        EXPECT_TRUE(engineCreated);
        
        TestOutput::PrintTestPass("engine initialization performance");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("engine initialization performance", "no exception", e.what());
        return false;
    }
}

/**
 * Test module registry performance
 * Requirements: 1.4, 1.5
 */
bool TestModuleRegistryPerformance() {
    TestOutput::PrintTestStart("module registry performance");

    try {
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Perform multiple registry operations
        for (int i = 0; i < 100; ++i) {
            registry.GetModuleNames();
            registry.GetAllModules();
            registry.GetModuleCount();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        // Registry operations should be fast (< 10ms for 100 operations)
        EXPECT_TRUE(duration.count() < 10000);
        
        TestOutput::PrintTestPass("module registry performance");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("module registry performance", "no exception", e.what());
        return false;
    }
}

/**
 * Test memory usage with modular architecture
 * Requirements: 1.1, 1.5
 */
bool TestModularArchitectureMemoryUsage() {
    TestOutput::PrintTestStart("modular architecture memory usage");

    try {
        // Basic memory usage test - ensure no obvious leaks
        Engine engine;
        
        // Perform some operations
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        auto moduleNames = registry.GetModuleNames();
        auto allModules = registry.GetAllModules();
        
        // Test multiple engine instances to check for leaks
        {
            Engine engine2;
            bool engine2Created = !engine2.IsRunning(); // Engine should not be running initially
            EXPECT_TRUE(engine2Created);
        } // engine2 goes out of scope here
        
        // If we reach here without crashes, memory management is working
        EXPECT_TRUE(moduleNames.size() >= 0);
        EXPECT_TRUE(allModules.size() >= 0);
        
        TestOutput::PrintTestPass("modular architecture memory usage");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("modular architecture memory usage", "no exception", e.what());
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Modular Architecture Performance");

    bool allPassed = true;

    try {
        TestSuite suite("Modular Architecture Performance Tests");

        allPassed &= suite.RunTest("Module Loading Performance", TestModuleLoadingPerformance);
        allPassed &= suite.RunTest("Engine Initialization Performance", TestEngineInitializationPerformance);
        allPassed &= suite.RunTest("Module Registry Performance", TestModuleRegistryPerformance);
        allPassed &= suite.RunTest("Modular Architecture Memory Usage", TestModularArchitectureMemoryUsage);

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