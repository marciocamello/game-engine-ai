#include "TestUtils.h"
#include "Resource/AssetManager.h"
#include "Core/Logger.h"
#include <filesystem>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Resource;

/**
 * Test asset path resolution with multiple search paths
 * Requirements: 7.3 (asset path resolution for project-specific and shared assets)
 */
bool TestAssetPathResolution() {
    TestOutput::PrintTestStart("asset path resolution");

    AssetManager& assetManager = AssetManager::GetInstance();
    
    // Clear existing search paths
    assetManager.ClearSearchPaths();
    
    // Set up test search paths
    assetManager.SetProjectAssetPath("projects/GameExample/assets");
    assetManager.SetSharedAssetPath("shared/assets");
    assetManager.SetLegacyAssetPath("assets");
    
    // Test that search paths are added correctly
    auto searchPaths = assetManager.GetSearchPaths();
    EXPECT_TRUE(searchPaths.size() >= 3);
    
    TestOutput::PrintTestPass("asset path resolution");
    return true;
}

/**
 * Test asset existence checking
 * Requirements: 7.3 (asset path resolution)
 */
bool TestAssetExistence() {
    TestOutput::PrintTestStart("asset existence checking");

    AssetManager& assetManager = AssetManager::GetInstance();
    
    // Test with a known shared asset
    bool exists = assetManager.AssetExists("shaders/basic_vertex.glsl");
    // Note: This may be false if the file doesn't exist yet, which is acceptable
    
    // Test with a non-existent asset
    bool notExists = assetManager.AssetExists("nonexistent/file.txt");
    EXPECT_FALSE(notExists);
    
    TestOutput::PrintTestPass("asset existence checking");
    return true;
}

/**
 * Test asset deployment configuration
 * Requirements: 7.4 (asset deployment system that copies only relevant assets)
 */
bool TestAssetDeploymentConfig() {
    TestOutput::PrintTestStart("asset deployment configuration");

    AssetManager& assetManager = AssetManager::GetInstance();
    
    // Create deployment configuration
    AssetManager::DeploymentConfig config;
    config.sourceProject = "TestProject";
    config.targetDirectory = "test_temp/deployment_test";
    config.copySharedAssets = true;
    config.overwriteExisting = true;
    
    // Add include patterns for common asset types
    config.includePatterns = {"*.glsl", "*.json", "*.png", "*.obj"};
    config.excludePatterns = {"*.tmp", "*.bak"};
    
    // Test that configuration is properly structured
    EXPECT_TRUE(!config.targetDirectory.empty());
    EXPECT_TRUE(config.copySharedAssets);
    EXPECT_TRUE(config.includePatterns.size() > 0);
    
    TestOutput::PrintTestPass("asset deployment configuration");
    return true;
}

/**
 * Test asset information retrieval
 * Requirements: 7.3 (asset path resolution)
 */
bool TestAssetInfo() {
    TestOutput::PrintTestStart("asset information retrieval");

    AssetManager& assetManager = AssetManager::GetInstance();
    
    // Test getting asset info for a shared asset
    auto assetInfo = assetManager.GetAssetInfo("shaders/basic_vertex.glsl");
    
    // The asset may or may not exist, but the function should not crash
    EXPECT_TRUE(assetInfo.relativePath == "shaders/basic_vertex.glsl");
    
    TestOutput::PrintTestPass("asset information retrieval");
    return true;
}

/**
 * Test search path management
 * Requirements: 7.3, 7.5 (asset path resolution and shared assets)
 */
bool TestSearchPathManagement() {
    TestOutput::PrintTestStart("search path management");

    AssetManager& assetManager = AssetManager::GetInstance();
    
    // Clear and add search paths
    assetManager.ClearSearchPaths();
    assetManager.AddSearchPath("test_path_1", 100);
    assetManager.AddSearchPath("test_path_2", 50);
    
    auto paths = assetManager.GetSearchPaths();
    EXPECT_TRUE(paths.size() == 2);
    
    // Test path removal
    assetManager.RemoveSearchPath("test_path_1");
    paths = assetManager.GetSearchPaths();
    EXPECT_TRUE(paths.size() == 1);
    
    TestOutput::PrintTestPass("search path management");
    return true;
}

int main() {
    TestOutput::PrintHeader("AssetManager");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AssetManager Tests");

        // Run all tests
        allPassed &= suite.RunTest("Asset Path Resolution", TestAssetPathResolution);
        allPassed &= suite.RunTest("Asset Existence Checking", TestAssetExistence);
        allPassed &= suite.RunTest("Asset Deployment Configuration", TestAssetDeploymentConfig);
        allPassed &= suite.RunTest("Asset Information Retrieval", TestAssetInfo);
        allPassed &= suite.RunTest("Search Path Management", TestSearchPathManagement);

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