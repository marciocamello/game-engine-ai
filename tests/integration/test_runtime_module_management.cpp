#include <iostream>
#include "../TestUtils.h"
#include "Core/RuntimeModuleManager.h"
#include "Core/DynamicModuleLoader.h"
#include "Core/ModuleRegistry.h"
#include "Core/ModuleConfigLoader.h"
#include "Core/Engine.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test runtime module manager initialization
 * Requirements: 2.6, 2.7
 */
bool TestRuntimeModuleManagerInitialization() {
    TestOutput::PrintTestStart("runtime module manager initialization");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    
    // Test initialization
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.IsInitialized());
    
    // Test double initialization (should succeed)
    EXPECT_TRUE(manager.Initialize());
    
    // Test shutdown
    manager.Shutdown();
    EXPECT_FALSE(manager.IsInitialized());

    TestOutput::PrintTestPass("runtime module manager initialization");
    return true;
}

/**
 * Test dynamic module discovery
 * Requirements: 2.6
 */
bool TestModuleDiscovery() {
    TestOutput::PrintTestStart("module discovery");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    
    // Test module discovery
    EXPECT_TRUE(manager.RefreshModuleList());
    
    auto availableModules = manager.GetAvailableModules();
    EXPECT_TRUE(availableModules.size() > 0);
    
    // Check that built-in modules are discovered
    bool foundGraphics = false;
    bool foundPhysics = false;
    bool foundAudio = false;
    
    for (const auto& module : availableModules) {
        if (module.name == "OpenGLGraphics") foundGraphics = true;
        if (module.name == "BulletPhysics") foundPhysics = true;
        if (module.name == "OpenALAudio") foundAudio = true;
    }
    
    EXPECT_TRUE(foundGraphics);
    EXPECT_TRUE(foundPhysics);
    EXPECT_TRUE(foundAudio);
    
    manager.Shutdown();

    TestOutput::PrintTestPass("module discovery");
    return true;
}

/**
 * Test runtime module loading and unloading
 * Requirements: 2.6
 */
bool TestRuntimeModuleLoading() {
    TestOutput::PrintTestStart("runtime module loading and unloading");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    const std::string testModuleName = "OpenGLGraphics";
    
    // Initially module should not be loaded
    EXPECT_FALSE(manager.IsModuleLoaded(testModuleName));
    
    // Load the module
    EXPECT_TRUE(manager.LoadModule(testModuleName));
    EXPECT_TRUE(manager.IsModuleLoaded(testModuleName));
    
    // Check that module appears in loaded modules list
    auto loadedModules = manager.GetLoadedModules();
    bool foundInLoaded = false;
    for (const auto& module : loadedModules) {
        if (module.name == testModuleName) {
            foundInLoaded = true;
            EXPECT_TRUE(module.isLoaded);
            break;
        }
    }
    EXPECT_TRUE(foundInLoaded);
    
    // Try to load again (should succeed but not duplicate)
    EXPECT_TRUE(manager.LoadModule(testModuleName));
    
    // Unload the module
    EXPECT_TRUE(manager.UnloadModule(testModuleName));
    EXPECT_FALSE(manager.IsModuleLoaded(testModuleName));
    
    // Try to unload again (should fail gracefully)
    EXPECT_FALSE(manager.UnloadModule(testModuleName));
    
    manager.Shutdown();

    TestOutput::PrintTestPass("runtime module loading and unloading");
    return true;
}

/**
 * Test module enable/disable functionality
 * Requirements: 2.7
 */
bool TestModuleEnableDisable() {
    TestOutput::PrintTestStart("module enable/disable functionality");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    const std::string testModuleName = "OpenGLGraphics";
    
    // Load the module first
    EXPECT_TRUE(manager.LoadModule(testModuleName));
    EXPECT_TRUE(manager.IsModuleLoaded(testModuleName));
    
    // Module should be enabled by default
    EXPECT_TRUE(manager.IsModuleEnabled(testModuleName));
    
    // Disable the module
    EXPECT_TRUE(manager.DisableModule(testModuleName));
    EXPECT_FALSE(manager.IsModuleEnabled(testModuleName));
    EXPECT_TRUE(manager.IsModuleLoaded(testModuleName)); // Still loaded, just disabled
    
    // Enable the module again
    EXPECT_TRUE(manager.EnableModule(testModuleName));
    EXPECT_TRUE(manager.IsModuleEnabled(testModuleName));
    
    // Check enabled modules list
    auto enabledModules = manager.GetEnabledModules();
    bool foundInEnabled = false;
    for (const auto& module : enabledModules) {
        if (module.name == testModuleName) {
            foundInEnabled = true;
            EXPECT_TRUE(module.isEnabled);
            break;
        }
    }
    EXPECT_TRUE(foundInEnabled);
    
    // Clean up
    EXPECT_TRUE(manager.UnloadModule(testModuleName));
    manager.Shutdown();

    TestOutput::PrintTestPass("module enable/disable functionality");
    return true;
}

/**
 * Test module reloading functionality
 * Requirements: 2.6
 */
bool TestModuleReloading() {
    TestOutput::PrintTestStart("module reloading functionality");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    const std::string testModuleName = "OpenGLGraphics";
    
    // Load the module first
    EXPECT_TRUE(manager.LoadModule(testModuleName));
    EXPECT_TRUE(manager.IsModuleLoaded(testModuleName));
    
    // Reload the module
    EXPECT_TRUE(manager.ReloadModule(testModuleName));
    EXPECT_TRUE(manager.IsModuleLoaded(testModuleName));
    EXPECT_TRUE(manager.IsModuleEnabled(testModuleName));
    
    // Clean up
    EXPECT_TRUE(manager.UnloadModule(testModuleName));
    manager.Shutdown();

    TestOutput::PrintTestPass("module reloading functionality");
    return true;
}

/**
 * Test batch module operations
 * Requirements: 2.6, 2.7
 */
bool TestBatchModuleOperations() {
    TestOutput::PrintTestStart("batch module operations");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    std::vector<std::string> testModules = {"OpenGLGraphics", "BulletPhysics"};
    
    // Load multiple modules
    EXPECT_TRUE(manager.LoadModules(testModules));
    
    for (const std::string& moduleName : testModules) {
        EXPECT_TRUE(manager.IsModuleLoaded(moduleName));
        EXPECT_TRUE(manager.IsModuleEnabled(moduleName));
    }
    
    // Disable multiple modules
    EXPECT_TRUE(manager.DisableModules(testModules));
    
    for (const std::string& moduleName : testModules) {
        EXPECT_TRUE(manager.IsModuleLoaded(moduleName));
        EXPECT_FALSE(manager.IsModuleEnabled(moduleName));
    }
    
    // Enable multiple modules
    EXPECT_TRUE(manager.EnableModules(testModules));
    
    for (const std::string& moduleName : testModules) {
        EXPECT_TRUE(manager.IsModuleLoaded(moduleName));
        EXPECT_TRUE(manager.IsModuleEnabled(moduleName));
    }
    
    // Unload multiple modules
    EXPECT_TRUE(manager.UnloadModules(testModules));
    
    for (const std::string& moduleName : testModules) {
        EXPECT_FALSE(manager.IsModuleLoaded(moduleName));
    }
    
    manager.Shutdown();

    TestOutput::PrintTestPass("batch module operations");
    return true;
}

/**
 * Test hot-swap functionality
 * Requirements: 2.6
 */
bool TestHotSwapFunctionality() {
    TestOutput::PrintTestStart("hot-swap functionality");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    // Test hot-swap enable/disable
    EXPECT_TRUE(manager.EnableHotSwap(true));
    EXPECT_TRUE(manager.IsHotSwapEnabled());
    
    EXPECT_TRUE(manager.EnableHotSwap(false));
    EXPECT_FALSE(manager.IsHotSwapEnabled());
    
    // Note: Actual hot-swapping of modules would require external module files
    // For now, we just test the API availability
    
    manager.Shutdown();

    TestOutput::PrintTestPass("hot-swap functionality");
    return true;
}

/**
 * Test module dependency management
 * Requirements: 2.6
 */
bool TestModuleDependencyManagement() {
    TestOutput::PrintTestStart("module dependency management");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    const std::string testModuleName = "OpenGLGraphics";
    
    // Load the module
    EXPECT_TRUE(manager.LoadModule(testModuleName));
    
    // Test dependency queries
    auto dependencies = manager.GetModuleDependencies(testModuleName);
    // OpenGL Graphics module should have no dependencies
    EXPECT_TRUE(dependencies.empty());
    
    auto dependents = manager.GetDependentModules(testModuleName);
    // Initially no modules should depend on graphics
    EXPECT_TRUE(dependents.empty());
    
    // Test if module can be unloaded (should be true since no dependents)
    EXPECT_TRUE(manager.CanUnloadModule(testModuleName));
    
    // Test load order calculation
    std::vector<std::string> moduleList = {testModuleName};
    auto loadOrder = manager.GetLoadOrder(moduleList);
    EXPECT_EQUAL(loadOrder.size(), 1);
    EXPECT_EQUAL(loadOrder[0], testModuleName);
    
    // Clean up
    EXPECT_TRUE(manager.UnloadModule(testModuleName));
    manager.Shutdown();

    TestOutput::PrintTestPass("module dependency management");
    return true;
}

/**
 * Test configuration management
 * Requirements: 2.7
 */
bool TestConfigurationManagement() {
    TestOutput::PrintTestStart("configuration management");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    // Load some modules
    EXPECT_TRUE(manager.LoadModule("OpenGLGraphics"));
    EXPECT_TRUE(manager.LoadModule("BulletPhysics"));
    
    // Get current configuration
    EngineConfig currentConfig = manager.GetCurrentConfiguration();
    EXPECT_TRUE(currentConfig.modules.size() >= 2);
    
    // Find our loaded modules in the config
    bool foundGraphics = false;
    bool foundPhysics = false;
    
    for (const auto& moduleConfig : currentConfig.modules) {
        if (moduleConfig.name == "OpenGLGraphics") {
            foundGraphics = true;
            EXPECT_TRUE(moduleConfig.enabled);
        }
        if (moduleConfig.name == "BulletPhysics") {
            foundPhysics = true;
            EXPECT_TRUE(moduleConfig.enabled);
        }
    }
    
    EXPECT_TRUE(foundGraphics);
    EXPECT_TRUE(foundPhysics);
    
    // Test configuration save/load (using temporary file)
    const std::string tempConfigPath = "test_runtime_config.json";
    EXPECT_TRUE(manager.SaveModuleConfiguration(tempConfigPath));
    
    // Unload modules
    EXPECT_TRUE(manager.UnloadModule("OpenGLGraphics"));
    EXPECT_TRUE(manager.UnloadModule("BulletPhysics"));
    
    // Load configuration back
    EXPECT_TRUE(manager.LoadModuleConfiguration(tempConfigPath));
    
    // Modules should be loaded again
    EXPECT_TRUE(manager.IsModuleLoaded("OpenGLGraphics"));
    EXPECT_TRUE(manager.IsModuleLoaded("BulletPhysics"));
    
    // Clean up
    EXPECT_TRUE(manager.UnloadModule("OpenGLGraphics"));
    EXPECT_TRUE(manager.UnloadModule("BulletPhysics"));
    
    // Remove temporary config file
    std::remove(tempConfigPath.c_str());
    
    manager.Shutdown();

    TestOutput::PrintTestPass("configuration management");
    return true;
}

/**
 * Test event system
 * Requirements: 2.6, 2.7
 */
bool TestEventSystem() {
    TestOutput::PrintTestStart("event system");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    // Register event callback
    std::vector<ModuleEventData> receivedEvents;
    manager.RegisterEventCallback([&receivedEvents](const ModuleEventData& eventData) {
        receivedEvents.push_back(eventData);
    });
    
    // Perform operations that should generate events
    EXPECT_TRUE(manager.LoadModule("OpenGLGraphics"));
    EXPECT_TRUE(manager.DisableModule("OpenGLGraphics"));
    EXPECT_TRUE(manager.EnableModule("OpenGLGraphics"));
    EXPECT_TRUE(manager.UnloadModule("OpenGLGraphics"));
    
    // Check that events were received
    EXPECT_TRUE(receivedEvents.size() >= 4);
    
    // Check event history
    auto recentEvents = manager.GetRecentEvents(10);
    EXPECT_TRUE(recentEvents.size() >= 4);
    
    manager.Shutdown();

    TestOutput::PrintTestPass("event system");
    return true;
}

/**
 * Test engine integration with runtime module management
 * Requirements: 2.6, 2.7
 */
bool TestEngineIntegration() {
    TestOutput::PrintTestStart("engine integration with runtime module management");

    // Note: This test is simplified since full engine initialization requires OpenGL context
    // We'll test the API availability and basic functionality
    
    Engine engine;
    
    // Test that engine exposes runtime module management methods
    auto availableModules = engine.GetAvailableModules();
    auto loadedModules = engine.GetLoadedModules();
    auto enabledModules = engine.GetEnabledModules();
    
    // These should return empty lists since engine is not initialized
    EXPECT_TRUE(availableModules.empty());
    EXPECT_TRUE(loadedModules.empty());
    EXPECT_TRUE(enabledModules.empty());
    
    // Test hot-swap API
    EXPECT_FALSE(engine.IsHotSwapEnabled());
    
    TestOutput::PrintTestPass("engine integration with runtime module management");
    return true;
}

/**
 * Test error handling and edge cases
 * Requirements: 2.6, 2.7
 */
bool TestErrorHandling() {
    TestOutput::PrintTestStart("error handling and edge cases");

    RuntimeModuleManager& manager = RuntimeModuleManager::GetInstance();
    
    // Test operations without initialization
    EXPECT_FALSE(manager.LoadModule("NonExistentModule"));
    EXPECT_TRUE(manager.HasErrors());
    EXPECT_FALSE(manager.GetLastError().empty());
    
    manager.ClearErrors();
    EXPECT_FALSE(manager.HasErrors());
    EXPECT_TRUE(manager.GetLastError().empty());
    
    // Initialize and test with non-existent module
    EXPECT_TRUE(manager.Initialize());
    EXPECT_TRUE(manager.RefreshModuleList());
    
    EXPECT_FALSE(manager.LoadModule("NonExistentModule"));
    EXPECT_TRUE(manager.HasErrors());
    
    // Test operations on non-loaded modules
    EXPECT_FALSE(manager.UnloadModule("NonExistentModule"));
    EXPECT_FALSE(manager.EnableModule("NonExistentModule"));
    EXPECT_FALSE(manager.DisableModule("NonExistentModule"));
    
    manager.Shutdown();

    TestOutput::PrintTestPass("error handling and edge cases");
    return true;
}

int main() {
    TestOutput::PrintHeader("Runtime Module Management Integration Tests");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Runtime Module Management Tests");

        // Run all tests
        allPassed &= suite.RunTest("Runtime Module Manager Initialization", TestRuntimeModuleManagerInitialization);
        allPassed &= suite.RunTest("Module Discovery", TestModuleDiscovery);
        allPassed &= suite.RunTest("Runtime Module Loading", TestRuntimeModuleLoading);
        allPassed &= suite.RunTest("Module Enable/Disable", TestModuleEnableDisable);
        allPassed &= suite.RunTest("Module Reloading", TestModuleReloading);
        allPassed &= suite.RunTest("Batch Module Operations", TestBatchModuleOperations);
        allPassed &= suite.RunTest("Hot-Swap Functionality", TestHotSwapFunctionality);
        allPassed &= suite.RunTest("Module Dependency Management", TestModuleDependencyManagement);
        allPassed &= suite.RunTest("Configuration Management", TestConfigurationManagement);
        allPassed &= suite.RunTest("Event System", TestEventSystem);
        allPassed &= suite.RunTest("Engine Integration", TestEngineIntegration);
        allPassed &= suite.RunTest("Error Handling", TestErrorHandling);

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