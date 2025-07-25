#include "TestUtils.h"
#include "Resource/AsyncModelLoader.h"
#include "Core/Logger.h"
#include <chrono>
#include <thread>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestPriorityLoading() {
    TestOutput::PrintTestStart("priority loading");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test priority configuration
    loader.SetDefaultPriority(AsyncModelLoader::TaskPriority::High);
    EXPECT_TRUE(static_cast<int>(loader.GetDefaultPriority()) == static_cast<int>(AsyncModelLoader::TaskPriority::High));

    loader.Shutdown();

    TestOutput::PrintTestPass("priority loading");
    return true;
}

bool TestMemoryManagement() {
    TestOutput::PrintTestStart("memory management");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test memory limit configuration
    size_t memoryLimit = 100 * 1024 * 1024; // 100MB
    loader.SetMemoryLimit(memoryLimit);
    EXPECT_EQUAL(loader.GetMemoryLimit(), memoryLimit);

    // Test memory management methods
    loader.FreeMemoryIfNeeded(); // Should not crash

    loader.Shutdown();

    TestOutput::PrintTestPass("memory management");
    return true;
}

bool TestQueueManagement() {
    TestOutput::PrintTestStart("queue management");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test queue queries
    auto queuedTasks = loader.GetQueuedTasks();
    EXPECT_EQUAL(queuedTasks.size(), static_cast<size_t>(0));

    // Test dependency resolution (with non-existent file)
    EXPECT_TRUE(loader.HasDependenciesResolved("nonexistent.obj"));

    // Test queue processing
    loader.ProcessTaskQueue(); // Should not crash

    loader.Shutdown();

    TestOutput::PrintTestPass("queue management");
    return true;
}

bool TestConcurrentLoadingStats() {
    TestOutput::PrintTestStart("concurrent loading statistics");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test enhanced statistics
    auto stats = loader.GetLoadingStats();
    EXPECT_EQUAL(stats.queuedLoads, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.currentMemoryUsage, static_cast<size_t>(0));
    EXPECT_EQUAL(stats.peakMemoryUsage, static_cast<size_t>(0));

    loader.Shutdown();

    TestOutput::PrintTestPass("concurrent loading statistics");
    return true;
}

bool TestDependencyLoading() {
    TestOutput::PrintTestStart("dependency loading");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test loading with dependencies (will fail but test the interface)
    try {
        std::vector<std::string> dependencies = {"dependency1.obj", "dependency2.obj"};
        auto future = loader.LoadModelAsync("main.obj", ModelLoader::LoadingFlags::None, 
                                          AsyncModelLoader::TaskPriority::Normal, dependencies);
        
        // Wait briefly then cancel to avoid long wait
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        loader.CancelLoad("main.obj");
        
    } catch (const std::exception& e) {
        TestOutput::PrintInfo("Expected exception for dependency loading: " + std::string(e.what()));
    }

    loader.Shutdown();

    TestOutput::PrintTestPass("dependency loading");
    return true;
}

bool TestBatchLoadingWithPriority() {
    TestOutput::PrintTestStart("batch loading with priority");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test batch loading with priority
    std::vector<std::string> filepaths = {"model1.obj", "model2.obj", "model3.obj"};
    
    try {
        auto future = loader.LoadModelsAsync(filepaths, ModelLoader::LoadingFlags::None, 
                                           AsyncModelLoader::TaskPriority::High);
        
        // Wait briefly then cancel to avoid long wait
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        loader.CancelAllLoads();
        
    } catch (const std::exception& e) {
        TestOutput::PrintInfo("Expected exception for batch loading: " + std::string(e.what()));
    }

    loader.Shutdown();

    TestOutput::PrintTestPass("batch loading with priority");
    return true;
}

bool TestResourceCleanup() {
    TestOutput::PrintTestStart("resource cleanup");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test cleanup methods
    loader.CleanupCompletedTasks(); // Should not crash
    loader.FreeMemoryIfNeeded(); // Should not crash

    loader.Shutdown();

    TestOutput::PrintTestPass("resource cleanup");
    return true;
}

int main() {
    TestOutput::PrintHeader("Concurrent Model Loading Unit Tests");

    TestSuite suite("ConcurrentModelLoading");
    bool allPassed = true;

    try {
        allPassed &= suite.RunTest("Priority Loading", TestPriorityLoading);
        allPassed &= suite.RunTest("Memory Management", TestMemoryManagement);
        allPassed &= suite.RunTest("Queue Management", TestQueueManagement);
        allPassed &= suite.RunTest("Concurrent Loading Statistics", TestConcurrentLoadingStats);
        allPassed &= suite.RunTest("Dependency Loading", TestDependencyLoading);
        allPassed &= suite.RunTest("Batch Loading with Priority", TestBatchLoadingWithPriority);
        allPassed &= suite.RunTest("Resource Cleanup", TestResourceCleanup);

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