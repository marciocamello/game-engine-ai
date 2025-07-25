#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestBasicResourceManager() {
    TestOutput::PrintTestStart("Basic Resource Manager");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Test initial state
    EXPECT_EQUAL(resourceManager.GetResourceCount(), 0);
    EXPECT_EQUAL(resourceManager.GetMemoryUsage(), 0);
    
    // Test statistics methods don't crash
    ResourceStats stats = resourceManager.GetResourceStats();
    EXPECT_EQUAL(stats.totalResources, 0);
    EXPECT_EQUAL(stats.totalMemoryUsage, 0);
    
    // Test logging methods don't crash
    resourceManager.LogResourceUsage();
    resourceManager.LogDetailedResourceInfo();
    
    // Test memory pressure methods
    resourceManager.SetMemoryPressureThreshold(1024 * 1024); // 1MB
    resourceManager.CheckMemoryPressure(); // Should not crash
    resourceManager.UnloadLeastRecentlyUsed(0); // Should not crash with no resources
    
    TestOutput::PrintTestPass("Basic Resource Manager");
    return true;
}

bool TestMemoryThresholdSettings() {
    TestOutput::PrintTestStart("Memory Threshold Settings");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Test setting different thresholds
    resourceManager.SetMemoryPressureThreshold(512 * 1024 * 1024); // 512MB
    resourceManager.SetMemoryPressureThreshold(1024 * 1024 * 1024); // 1GB
    resourceManager.SetMemoryPressureThreshold(100 * 1024 * 1024); // 100MB
    
    // Test memory pressure check with different thresholds
    resourceManager.CheckMemoryPressure();
    
    TestOutput::PrintTestPass("Memory Threshold Settings");
    return true;
}

bool TestResourceStatsStructure() {
    TestOutput::PrintTestStart("Resource Stats Structure");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    ResourceStats stats = resourceManager.GetResourceStats();
    
    // Test initial stats
    EXPECT_EQUAL(stats.totalResources, 0);
    EXPECT_EQUAL(stats.totalMemoryUsage, 0);
    EXPECT_EQUAL(stats.expiredReferences, 0);
    EXPECT_TRUE(stats.resourcesByType.empty());
    EXPECT_TRUE(stats.memoryByType.empty());
    
    TestOutput::PrintTestPass("Resource Stats Structure");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Manager Basic Integration");

    bool allPassed = true;

    try {
        // Initialize logger
        Logger::GetInstance().Initialize();
        
        // Create test suite for result tracking
        TestSuite suite("Resource Manager Basic Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Basic Resource Manager", TestBasicResourceManager);
        allPassed &= suite.RunTest("Memory Threshold Settings", TestMemoryThresholdSettings);
        allPassed &= suite.RunTest("Resource Stats Structure", TestResourceStatsStructure);

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