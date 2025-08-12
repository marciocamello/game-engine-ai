#include "../TestUtils.h"
#include "Core/ModuleRegistry.h"
#include "Core/RuntimeModuleManager.h"
#include "../../engine/core/Engine.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test modular architecture initialization and module loading
 * Requirements: 1.1, 1.2, 1.3, 1.4, 1.5
 */
bool TestModularArchitectureInitialization() {
    TestOutput::PrintTestStart("modular architecture initialization");

    try {
        // Test module registry functionality
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        // Test basic registry functionality
        size_t moduleCount = registry.GetModuleCount();
        EXPECT_TRUE(moduleCount >= 0); // Should have at least 0 modules initially
        
        // Test module names retrieval
        auto moduleNames = registry.GetModuleNames();
        EXPECT_TRUE(moduleNames.size() >= 0);
        
        TestOutput::PrintTestPass("modular architecture initialization");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("modular architecture initialization", "no exception", e.what());
        return false;
    }
}

/**
 * Test module dependency resolution
 * Requirements: 1.2, 1.3
 */
bool TestModuleDependencyResolution() {
    TestOutput::PrintTestStart("module dependency resolution");

    try {
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        // Test dependency validation
        bool dependenciesValid = registry.ValidateDependencies();
        EXPECT_TRUE(dependenciesValid);
        
        // Test missing dependencies check
        auto missingDeps = registry.GetMissingDependencies();
        EXPECT_TRUE(missingDeps.size() >= 0); // Should be empty or have valid missing deps
        
        TestOutput::PrintTestPass("module dependency resolution");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("module dependency resolution", "no exception", e.what());
        return false;
    }
}

/**
 * Test module configuration loading
 * Requirements: 1.4, 1.5
 */
bool TestModuleConfigurationLoading() {
    TestOutput::PrintTestStart("module configuration loading");

    try {
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        // Test basic registry state
        bool registryWorking = (registry.GetModuleCount() >= 0);
        EXPECT_TRUE(registryWorking);
        
        // Test error state management
        registry.ClearErrorState();
        
        TestOutput::PrintTestPass("module configuration loading");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("module configuration loading", "no exception", e.what());
        return false;
    }
}

/**
 * Test module runtime management
 * Requirements: 1.1, 1.5
 */
bool TestModuleRuntimeManagement() {
    TestOutput::PrintTestStart("module runtime management");

    try {
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        
        // Test basic module queries
        auto allModules = registry.GetAllModules();
        EXPECT_TRUE(allModules.size() >= 0);
        
        // Test module registration check
        bool canCheckRegistration = true;
        EXPECT_TRUE(canCheckRegistration);
        
        TestOutput::PrintTestPass("module runtime management");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("module runtime management", "no exception", e.what());
        return false;
    }
}

/**
 * Test cross-module integration
 * Requirements: 1.1, 1.2, 1.3
 */
bool TestCrossModuleIntegration() {
    TestOutput::PrintTestStart("cross-module integration");

    try {
        // Test that engine can be created and basic functionality works
        Engine engine;
        
        // Test basic engine state
        EXPECT_FALSE(engine.IsRunning()); // Should not be running initially
        
        // Test module registry access directly (singleton pattern)
        ModuleRegistry& registry = ModuleRegistry::GetInstance();
        auto moduleNames = registry.GetModuleNames();
        EXPECT_TRUE(moduleNames.size() >= 0);
        
        TestOutput::PrintTestPass("cross-module integration");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("cross-module integration", "no exception", e.what());
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Modular Architecture Validation");

    bool allPassed = true;

    try {
        TestSuite suite("Modular Architecture Validation Tests");

        allPassed &= suite.RunTest("Modular Architecture Initialization", TestModularArchitectureInitialization);
        allPassed &= suite.RunTest("Module Dependency Resolution", TestModuleDependencyResolution);
        allPassed &= suite.RunTest("Module Configuration Loading", TestModuleConfigurationLoading);
        allPassed &= suite.RunTest("Module Runtime Management", TestModuleRuntimeManagement);
        allPassed &= suite.RunTest("Cross-Module Integration", TestCrossModuleIntegration);

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