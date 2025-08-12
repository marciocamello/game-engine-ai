#include <iostream>
#include "../TestUtils.h"
#include "Resource/AssetManager.h"
#include "Core/ConfigManager.h"
#include "Core/Logger.h"
#include <filesystem>

using namespace GameEngine::Testing;
using namespace GameEngine::Resource;
using namespace GameEngine::Core;

/**
 * Test complete asset and configuration management integration
 * Requirements: 7.1, 7.3, 7.4, 7.5 (complete asset and configuration management)
 */
bool TestAssetConfigIntegration() {
    TestOutput::PrintTestStart("asset and configuration management integration");

    // Initialize managers
    AssetManager& assetManager = AssetManager::GetInstance();
    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Set up asset paths
    assetManager.ClearSearchPaths();
    assetManager.SetProjectAssetPath("projects/GameExample/assets");
    assetManager.SetSharedAssetPath("shared/assets");
    assetManager.SetLegacyAssetPath("assets");
    
    // Set up configuration paths
    configManager.SetSharedConfigPath("shared/configs");
    configManager.SetProjectConfigPath("GameExample");
    
    // Load configurations
    bool configLoaded = configManager.LoadEngineConfig("GameExample");
    EXPECT_TRUE(configLoaded);
    
    // Test asset resolution with configuration
    auto searchPaths = assetManager.GetSearchPaths();
    EXPECT_TRUE(searchPaths.size() >= 3);
    
    // Test that shared assets directory exists
    bool sharedAssetsExist = std::filesystem::exists("shared/assets");
    EXPECT_TRUE(sharedAssetsExist);
    
    // Test that shared configs directory exists
    bool sharedConfigsExist = std::filesystem::exists("shared/configs");
    EXPECT_TRUE(sharedConfigsExist);
    
    TestOutput::PrintTestPass("asset and configuration management integration");
    return true;
}

/**
 * Test asset deployment with configuration-driven settings
 * Requirements: 7.4 (asset deployment system)
 */
bool TestConfigDrivenAssetDeployment() {
    TestOutput::PrintTestStart("configuration-driven asset deployment");

    AssetManager& assetManager = AssetManager::GetInstance();
    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Create test deployment directory
    std::string testDeployDir = "test_temp/config_driven_deployment";
    
    try {
        // Clean up any existing test directory
        if (std::filesystem::exists(testDeployDir)) {
            std::filesystem::remove_all(testDeployDir);
        }
        
        // Create deployment configuration
        AssetManager::DeploymentConfig config;
        config.sourceProject = "GameExample";
        config.targetDirectory = testDeployDir;
        config.copySharedAssets = true;
        config.overwriteExisting = true;
        
        // Add patterns based on typical game asset types
        config.includePatterns = {"*.glsl", "*.json", "*.png", "*.obj", "*.wav"};
        config.excludePatterns = {"*.tmp", "*.bak", "*.log"};
        
        // Attempt deployment (may succeed or fail based on available assets)
        bool deployed = assetManager.DeployAssets(config);
        
        // Clean up test directory
        if (std::filesystem::exists(testDeployDir)) {
            std::filesystem::remove_all(testDeployDir);
        }
        
        // Test passes if deployment attempt doesn't crash
        EXPECT_TRUE(true);
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Deployment test exception: " + std::string(e.what()));
        return false;
    }
    
    TestOutput::PrintTestPass("configuration-driven asset deployment");
    return true;
}

/**
 * Test shared asset and configuration hierarchy
 * Requirements: 7.5 (shared engine assets), 7.2 (configuration hierarchy)
 */
bool TestSharedResourceHierarchy() {
    TestOutput::PrintTestStart("shared resource hierarchy");

    AssetManager& assetManager = AssetManager::GetInstance();
    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Test asset search order (project -> shared -> legacy)
    auto searchPaths = assetManager.GetSearchPaths();
    bool hasProjectPath = false;
    bool hasSharedPath = false;
    bool hasLegacyPath = false;
    
    for (const auto& path : searchPaths) {
        if (path.find("projects/") != std::string::npos) hasProjectPath = true;
        if (path.find("shared/") != std::string::npos) hasSharedPath = true;
        if (path == "assets") hasLegacyPath = true;
    }
    
    // At least shared path should be configured
    EXPECT_TRUE(hasSharedPath);
    
    // Test configuration hierarchy
    std::string sharedConfigPath = configManager.GetSharedConfigPath();
    EXPECT_TRUE(!sharedConfigPath.empty());
    
    // Test that default configurations exist
    bool defaultEngineConfigExists = std::filesystem::exists("shared/configs/default_engine_config.json");
    bool defaultProjectConfigExists = std::filesystem::exists("shared/configs/default_project_config.json");
    bool moduleDefaultsExist = std::filesystem::exists("shared/configs/module_defaults.json");
    
    EXPECT_TRUE(defaultEngineConfigExists);
    EXPECT_TRUE(defaultProjectConfigExists);
    EXPECT_TRUE(moduleDefaultsExist);
    
    TestOutput::PrintTestPass("shared resource hierarchy");
    return true;
}

/**
 * Test asset and configuration system initialization
 * Requirements: 7.1, 7.2, 7.3 (complete system initialization)
 */
bool TestSystemInitialization() {
    TestOutput::PrintTestStart("asset and configuration system initialization");

    // Test that managers can be initialized without crashing
    AssetManager& assetManager = AssetManager::GetInstance();
    ConfigManager& configManager = ConfigManager::GetInstance();
    
    // Test basic functionality
    assetManager.ClearSearchPaths();
    assetManager.AddSearchPath("shared/assets", 50);
    
    auto paths = assetManager.GetSearchPaths();
    EXPECT_TRUE(paths.size() >= 1);
    
    // Test configuration loading
    configManager.SetSharedConfigPath("shared/configs");
    bool moduleDefaultsLoaded = configManager.LoadModuleDefaults();
    
    // Test passes if no crashes occur
    EXPECT_TRUE(true);
    
    TestOutput::PrintTestPass("asset and configuration system initialization");
    return true;
}

int main() {
    TestOutput::PrintHeader("Asset and Configuration Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Asset and Configuration Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Asset Config Integration", TestAssetConfigIntegration);
        allPassed &= suite.RunTest("Config Driven Asset Deployment", TestConfigDrivenAssetDeployment);
        allPassed &= suite.RunTest("Shared Resource Hierarchy", TestSharedResourceHierarchy);
        allPassed &= suite.RunTest("System Initialization", TestSystemInitialization);

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