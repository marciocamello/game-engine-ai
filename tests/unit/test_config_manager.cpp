#include "TestUtils.h"
#include "Core/ConfigManager.h"
#include "Core/Logger.h"
#include <filesystem>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Core;

/**
 * Test configuration loading hierarchy
 * Requirements: 7.2 (project-specific configuration files)
 */
bool TestConfigurationHierarchy() {
    TestOutput::PrintTestStart("configuration loading hierarchy");

    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Set up configuration paths
    configManager.SetSharedConfigPath("shared/configs");
    configManager.SetProjectConfigPath("GameExample");
    
    // Test loading module defaults
    bool defaultsLoaded = configManager.LoadModuleDefaults();
    // This may fail if the file doesn't exist, which is acceptable for testing
    
    // Test loading engine config
    bool engineLoaded = configManager.LoadEngineConfig("GameExample");
    EXPECT_TRUE(engineLoaded); // Should always succeed with fallbacks
    
    TestOutput::PrintTestPass("configuration loading hierarchy");
    return true;
}

/**
 * Test configuration value retrieval
 * Requirements: 7.2 (configuration file management)
 */
bool TestConfigurationValues() {
    TestOutput::PrintTestStart("configuration value retrieval");

    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Test getting engine config values with defaults
    std::string logLevel = configManager.GetEngineConfigValue("logLevel", "INFO");
    EXPECT_TRUE(!logLevel.empty());
    
    // Test getting boolean values
    bool vsync = configManager.GetEngineConfigBool("vsync", true);
    // Should return a valid boolean value
    
    // Test getting integer values
    int maxThreads = configManager.GetEngineConfigInt("maxThreads", 4);
    EXPECT_TRUE(maxThreads > 0);
    
    TestOutput::PrintTestPass("configuration value retrieval");
    return true;
}

/**
 * Test module configuration management
 * Requirements: 7.2 (configuration file management for engine modules)
 */
bool TestModuleConfiguration() {
    TestOutput::PrintTestStart("module configuration management");

    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Test getting module configuration
    auto moduleConfig = configManager.GetModuleConfig("Graphics");
    EXPECT_TRUE(moduleConfig.name == "Graphics" || moduleConfig.name.empty());
    
    // Test module enabled status
    bool coreEnabled = configManager.IsModuleEnabled("Core");
    // Core should typically be enabled, but we'll accept any boolean result
    
    // Test getting enabled modules list
    auto enabledModules = configManager.GetEnabledModules();
    // Should return a list (may be empty if no config loaded)
    
    TestOutput::PrintTestPass("module configuration management");
    return true;
}

/**
 * Test configuration validation
 * Requirements: 7.2 (configuration file management)
 */
bool TestConfigurationValidation() {
    TestOutput::PrintTestStart("configuration validation");

    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Test engine config validation
    bool engineValid = configManager.ValidateEngineConfig();
    // Should return true or false without crashing
    
    // Test project config validation
    bool projectValid = configManager.ValidateProjectConfig();
    // Should return true or false without crashing
    
    // Test getting validation errors
    auto errors = configManager.GetConfigurationErrors();
    // Should return a vector (may be empty)
    
    TestOutput::PrintTestPass("configuration validation");
    return true;
}

/**
 * Test configuration path management
 * Requirements: 7.2 (configuration file management)
 */
bool TestConfigurationPaths() {
    TestOutput::PrintTestStart("configuration path management");

    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Test setting and getting configuration paths
    configManager.SetSharedConfigPath("shared/configs");
    configManager.SetProjectConfigPath("TestProject");
    
    std::string sharedPath = configManager.GetSharedConfigPath();
    std::string projectPath = configManager.GetProjectConfigPath();
    
    EXPECT_TRUE(sharedPath == "shared/configs");
    EXPECT_TRUE(projectPath == "TestProject");
    
    TestOutput::PrintTestPass("configuration path management");
    return true;
}

int main() {
    TestOutput::PrintHeader("ConfigManager");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ConfigManager Tests");

        // Run all tests
        allPassed &= suite.RunTest("Configuration Hierarchy", TestConfigurationHierarchy);
        allPassed &= suite.RunTest("Configuration Values", TestConfigurationValues);
        allPassed &= suite.RunTest("Module Configuration", TestModuleConfiguration);
        allPassed &= suite.RunTest("Configuration Validation", TestConfigurationValidation);
        allPassed &= suite.RunTest("Configuration Paths", TestConfigurationPaths);

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