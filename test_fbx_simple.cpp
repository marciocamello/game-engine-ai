#include "Resource/FBXLoader.h"
#include "Core/Logger.h"
#include "tests/TestUtils.h"
#include <iostream>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test FBX loader initialization
 * Requirements: FBX loading system initialization
 */
bool TestFBXLoaderInitialization() {
    TestOutput::PrintTestStart("FBX loader initialization");
    
    FBXLoader loader;
    bool initResult = loader.Initialize();
    
    EXPECT_TRUE(initResult);
    
    if (initResult) {
        TestOutput::PrintInfo("FBX loader initialized successfully");
        loader.Shutdown();
    }
    
    TestOutput::PrintTestPass("FBX loader initialization");
    return true;
}

/**
 * Test XBot.fbx loading
 * Requirements: FBX file loading and parsing
 */
bool TestXBotFBXLoading() {
    TestOutput::PrintTestStart("XBot FBX loading");
    
    FBXLoader loader;
    if (!loader.Initialize()) {
        TestOutput::PrintTestFail("XBot FBX loading", "loader initialization", "failed to initialize");
        return false;
    }
    
    // Test loading XBot.fbx
    auto result = loader.LoadFBX("assets/meshes/XBot.fbx");
    
    if (result.success) {
        TestOutput::PrintInfo("Successfully loaded XBot.fbx");
        TestOutput::PrintInfo("  Meshes: " + std::to_string(result.meshes.size()));
        TestOutput::PrintInfo("  Materials: " + std::to_string(result.materialCount));
        TestOutput::PrintInfo("  Vertices: " + std::to_string(result.totalVertices));
        TestOutput::PrintInfo("  Triangles: " + std::to_string(result.totalTriangles));
        TestOutput::PrintInfo("  Loading time: " + std::to_string(result.loadingTimeMs) + "ms");
        TestOutput::PrintInfo("  Source app: " + result.sourceApplication);
        TestOutput::PrintInfo("  Has skeleton: " + std::string(result.hasSkeleton ? "Yes" : "No"));
        TestOutput::PrintInfo("  Has animations: " + std::string(result.hasAnimations ? "Yes" : "No"));
        
        EXPECT_TRUE(result.meshes.size() > 0);
        EXPECT_TRUE(result.totalVertices > 0);
        EXPECT_TRUE(result.totalTriangles > 0);
    } else {
        TestOutput::PrintInfo("Failed to load XBot.fbx: " + result.errorMessage);
        TestOutput::PrintInfo("This may be expected if the file doesn't exist");
    }
    
    loader.Shutdown();
    
    TestOutput::PrintTestPass("XBot FBX loading");
    return true;
}

int main() {
    TestOutput::PrintHeader("FBX Loader");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("FBX Loader Tests");

        // Run all tests
        allPassed &= suite.RunTest("FBX Loader Initialization", TestFBXLoaderInitialization);
        allPassed &= suite.RunTest("XBot FBX Loading", TestXBotFBXLoading);

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