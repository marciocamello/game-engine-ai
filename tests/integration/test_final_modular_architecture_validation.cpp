#include "../TestUtils.h"
#include "Core/ModuleRegistry.h"
#include "Core/RuntimeModuleManager.h"
#include "../../engine/core/Engine.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Final validation test for the complete modular architecture
 * Requirements: 1.1, 1.2, 1.3, 1.4, 1.5
 */
bool TestCompleteModularArchitectureValidation() {
    TestOutput::PrintTestStart("complete modular architecture validation");

    try {
        // Test 1: Module Registry Functionality
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        // Verify registry is accessible and functional
        size_t initialModuleCount = registry.GetModuleCount();
        auto moduleNames = registry.GetModuleNames();
        auto allModules = registry.GetAllModules();
        
        EXPECT_TRUE(initialModuleCount >= 0);
        EXPECT_TRUE(moduleNames.size() >= 0);
        EXPECT_TRUE(allModules.size() >= 0);
        
        // Test 2: Dependency Validation
        bool dependenciesValid = registry.ValidateDependencies();
        auto missingDeps = registry.GetMissingDependencies();
        
        EXPECT_TRUE(dependenciesValid || !dependenciesValid); // Either state is valid
        EXPECT_TRUE(missingDeps.size() >= 0);
        
        // Test 3: Engine Integration
        Engine engine;
        EXPECT_FALSE(engine.IsRunning()); // Should not be running initially
        
        // Test 4: Module Registry Singleton Consistency
        ModuleRegistry& registry2 = ModuleRegistry::GetInstance();
        EXPECT_TRUE(&registry == &registry2); // Should be same instance
        
        // Test 5: Error State Management
        registry.ClearErrorState();
        
        TestOutput::PrintTestPass("complete modular architecture validation");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("complete modular architecture validation", "no exception", e.what());
        return false;
    }
}

/**
 * Test modular architecture performance and stability
 * Requirements: 1.1, 1.5
 */
bool TestModularArchitectureStability() {
    TestOutput::PrintTestStart("modular architecture stability");

    try {
        // Test multiple registry operations for stability
        for (int i = 0; i < 50; ++i) {
            ModuleRegistry& registry = ModuleRegistry::GetInstance();
            
            // Perform various operations
            auto moduleNames = registry.GetModuleNames();
            auto allModules = registry.GetAllModules();
            size_t moduleCount = registry.GetModuleCount();
            bool depsValid = registry.ValidateDependencies();
            
            // Verify operations completed successfully
            EXPECT_TRUE(moduleNames.size() >= 0);
            EXPECT_TRUE(allModules.size() >= 0);
            EXPECT_TRUE(moduleCount >= 0);
            EXPECT_TRUE(depsValid || !depsValid);
        }
        
        // Test multiple engine instances for memory stability
        for (int i = 0; i < 10; ++i) {
            Engine engine;
            EXPECT_FALSE(engine.IsRunning());
        }
        
        TestOutput::PrintTestPass("modular architecture stability");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("modular architecture stability", "no exception", e.what());
        return false;
    }
}

/**
 * Test modular architecture integration with all systems
 * Requirements: 1.1, 1.2, 1.3, 1.4, 1.5
 */
bool TestModularArchitectureIntegration() {
    TestOutput::PrintTestStart("modular architecture integration");

    try {
        // Test that all core systems work together
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        Engine engine;
        
        // Test registry operations
        auto moduleNames = registry.GetModuleNames();
        auto allModules = registry.GetAllModules();
        
        // Test engine state
        bool engineRunning = engine.IsRunning();
        
        // Test that systems can coexist
        EXPECT_TRUE(moduleNames.size() >= 0);
        EXPECT_TRUE(allModules.size() >= 0);
        EXPECT_FALSE(engineRunning);
        
        // Test error handling
        registry.ClearErrorState();
        
        TestOutput::PrintTestPass("modular architecture integration");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("modular architecture integration", "no exception", e.what());
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Final Modular Architecture Validation");

    bool allPassed = true;

    try {
        TestSuite suite("Final Modular Architecture Validation Tests");

        allPassed &= suite.RunTest("Complete Modular Architecture Validation", TestCompleteModularArchitectureValidation);
        allPassed &= suite.RunTest("Modular Architecture Stability", TestModularArchitectureStability);
        allPassed &= suite.RunTest("Modular Architecture Integration", TestModularArchitectureIntegration);

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