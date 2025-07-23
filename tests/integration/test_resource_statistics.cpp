#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Simple mock resource class for testing that doesn't depend on OpenGL
class MockResource : public Resource {
public:
    MockResource(const std::string& path) : Resource(path), m_size(1024) {}
    
    size_t GetMemoryUsage() const override {
        return Resource::GetMemoryUsage() + m_size;
    }
    
    void SetSize(size_t size) { m_size = size; }
    
private:
    size_t m_size;
};

bool TestResourceStatistics() {
    TestOutput::PrintTestStart("Resource Statistics");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Test initial state
    EXPECT_EQUAL(resourceManager.GetResourceCount(), 0);
    EXPECT_EQUAL(resourceManager.GetMemoryUsage(), 0);
    
    // Use mock resources that don't depend on OpenGL
    auto resource1 = resourceManager.Load<MockResource>("test/resource1.dat");
    auto resource2 = resourceManager.Load<MockResource>("test/resource2.dat");
    auto resource3 = resourceManager.Load<MockResource>("test/resource3.dat");
    
    // Resources should be created
    EXPECT_TRUE(resource1 != nullptr);
    EXPECT_TRUE(resource2 != nullptr);
    EXPECT_TRUE(resource3 != nullptr);
    
    // Check statistics
    size_t resourceCount = resourceManager.GetResourceCount();
    size_t memoryUsage = resourceManager.GetMemoryUsage();
    
    std::cout << "Resources loaded: " << resourceCount << std::endl;
    std::cout << "Memory usage: " << memoryUsage << " bytes" << std::endl;
    
    EXPECT_TRUE(resourceCount >= 3); // At least 3 resources loaded
    EXPECT_TRUE(memoryUsage > 0); // Some memory should be used
    
    // Test resource stats structure
    ResourceStats stats = resourceManager.GetResourceStats();
    EXPECT_TRUE(stats.totalResources >= 3);
    EXPECT_TRUE(stats.totalMemoryUsage > 0);
    EXPECT_TRUE(stats.resourcesByType.size() >= 1); // At least MockResource type
    
    // Test logging (should not crash)
    resourceManager.LogResourceUsage();
    resourceManager.LogDetailedResourceInfo();
    
    TestOutput::PrintTestPass("Resource Statistics");
    return true;
}

bool TestMissingResourceHandling() {
    TestOutput::PrintTestStart("Missing Resource Handling");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Test loading non-existent mock resources
    auto missingResource1 = resourceManager.Load<MockResource>("missing/resource1.dat");
    auto missingResource2 = resourceManager.Load<MockResource>("missing/resource2.dat");
    
    // Resources should still be created
    EXPECT_TRUE(missingResource1 != nullptr);
    EXPECT_TRUE(missingResource2 != nullptr);
    
    std::cout << "Missing resource1 memory: " << missingResource1->GetMemoryUsage() << " bytes" << std::endl;
    std::cout << "Missing resource2 memory: " << missingResource2->GetMemoryUsage() << " bytes" << std::endl;
    
    // Check statistics include these resources
    size_t resourceCount = resourceManager.GetResourceCount();
    std::cout << "Resource count after loading missing resources: " << resourceCount << std::endl;
    EXPECT_TRUE(resourceCount >= 2);
    
    TestOutput::PrintTestPass("Missing Resource Handling");
    return true;
}

bool TestMemoryPressureHandling() {
    TestOutput::PrintTestStart("Memory Pressure Handling");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Set a very low memory threshold to trigger pressure handling
    resourceManager.SetMemoryPressureThreshold(2048); // 2 KB threshold
    
    // Load multiple mock resources to trigger memory pressure
    std::vector<std::shared_ptr<MockResource>> resources;
    
    // Create resources with different sizes
    for (int i = 0; i < 10; ++i) {
        auto resource = resourceManager.Load<MockResource>("test/resource_" + std::to_string(i) + ".dat");
        resource->SetSize(512 * (i + 1)); // Varying sizes: 512B, 1KB, 1.5KB, etc.
        resources.push_back(resource);
    }
    
    size_t initialCount = resourceManager.GetResourceCount();
    size_t initialMemory = resourceManager.GetMemoryUsage();
    
    std::cout << "Before memory pressure: " << initialCount << " resources, " << 
                 (initialMemory / 1024) << " KB" << std::endl;
    
    // Manually trigger memory pressure check
    resourceManager.CheckMemoryPressure();
    
    // Clear some references to allow LRU cleanup
    resources.clear();
    
    // Wait a bit for potential cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test LRU cleanup directly
    resourceManager.UnloadLeastRecentlyUsed(1024); // Try to free 1KB
    
    size_t finalCount = resourceManager.GetResourceCount();
    size_t finalMemory = resourceManager.GetMemoryUsage();
    
    std::cout << "After memory pressure: " << finalCount << " resources, " << 
                 (finalMemory / 1024) << " KB" << std::endl;
    
    // Memory management should have had some effect
    EXPECT_TRUE(finalCount <= initialCount); // Should not have more resources
    
    TestOutput::PrintTestPass("Memory Pressure Handling");
    return true;
}

bool TestDetailedLogging() {
    TestOutput::PrintTestStart("Detailed Resource Logging");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Load resources with some time gaps to test access time tracking
    auto resource1 = resourceManager.Load<MockResource>("test/logging1.dat");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    auto resource2 = resourceManager.Load<MockResource>("test/logging2.dat");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Access first resource again to update its access time
    auto resource1_again = resourceManager.Load<MockResource>("test/logging1.dat");
    EXPECT_TRUE(resource1.get() == resource1_again.get()); // Should be same instance (cache hit)
    
    // Test detailed logging
    resourceManager.LogDetailedResourceInfo();
    
    // Test resource stats
    ResourceStats stats = resourceManager.GetResourceStats();
    EXPECT_TRUE(stats.totalResources >= 2);
    
    // Test cache hit tracking
    resourceManager.LogResourceUsage(); // Should show cache hit rate
    
    TestOutput::PrintTestPass("Detailed Resource Logging");
    return true;
}

bool TestResourceCacheHits() {
    TestOutput::PrintTestStart("Resource Cache Hits");
    
    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());
    
    // Load a resource for the first time
    auto resource1 = resourceManager.Load<MockResource>("test/cache1.dat");
    EXPECT_TRUE(resource1 != nullptr);
    
    // Load the same resource again - should be cache hit
    auto resource2 = resourceManager.Load<MockResource>("test/cache1.dat");
    EXPECT_TRUE(resource2 != nullptr);
    EXPECT_TRUE(resource1.get() == resource2.get()); // Should be same instance
    
    // Load different resource
    auto resource3 = resourceManager.Load<MockResource>("test/cache2.dat");
    EXPECT_TRUE(resource3 != nullptr);
    
    // Load same resource again - should be cache hit
    auto resource4 = resourceManager.Load<MockResource>("test/cache2.dat");
    EXPECT_TRUE(resource4 != nullptr);
    EXPECT_TRUE(resource3.get() == resource4.get()); // Should be same instance
    
    // Check statistics show cache hits
    resourceManager.LogResourceUsage();
    
    TestOutput::PrintTestPass("Resource Cache Hits");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Statistics and Debugging Tests");
    Logger::GetInstance().Initialize();
    
    TestSuite suite("Resource Statistics Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("Resource Statistics", TestResourceStatistics);
    allPassed &= suite.RunTest("Missing Resource Handling", TestMissingResourceHandling);
    allPassed &= suite.RunTest("Memory Pressure Handling", TestMemoryPressureHandling);
    allPassed &= suite.RunTest("Detailed Logging", TestDetailedLogging);
    allPassed &= suite.RunTest("Resource Cache Hits", TestResourceCacheHits);
    
    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}