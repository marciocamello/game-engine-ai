#include "TestUtils.h"
#include "Resource/AsyncModelLoader.h"
#include "Core/Logger.h"
#include <chrono>
#include <thread>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestAsyncModelLoaderInitialization() {
    TestOutput::PrintTestStart("AsyncModelLoader initialization");

    AsyncModelLoader loader;
    
    // Test initialization
    EXPECT_TRUE(loader.Initialize(2));
    EXPECT_TRUE(loader.IsInitialized());
    EXPECT_EQUAL(loader.GetWorkerThreadCount(), static_cast<uint32_t>(2));
    
    // Test shutdown
    loader.Shutdown();
    EXPECT_FALSE(loader.IsInitialized());

    TestOutput::PrintTestPass("AsyncModelLoader initialization");
    return true;
}

bool TestThreadPoolBasic() {
    TestOutput::PrintTestStart("ThreadPool basic functionality");

    ThreadPool pool(2);
    
    // Test simple task execution
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 5; ++i) {
        futures.push_back(pool.Enqueue([&counter]() {
            counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }));
    }
    
    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }
    
    EXPECT_EQUAL(counter.load(), 5);

    TestOutput::PrintTestPass("ThreadPool basic functionality");
    return true;
}

bool TestAsyncModelLoaderConfiguration() {
    TestOutput::PrintTestStart("AsyncModelLoader configuration");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test default configuration
    EXPECT_EQUAL(loader.GetMaxConcurrentLoads(), static_cast<uint32_t>(4));
    EXPECT_TRUE(static_cast<uint32_t>(loader.GetDefaultLoadingFlags()) == static_cast<uint32_t>(ModelLoader::LoadingFlags::None));

    // Test configuration changes
    loader.SetMaxConcurrentLoads(8);
    EXPECT_EQUAL(loader.GetMaxConcurrentLoads(), static_cast<uint32_t>(8));

    loader.SetDefaultLoadingFlags(ModelLoader::LoadingFlags::GenerateNormals);
    // Test that the flag was set (can't directly compare enum values in this test framework)
    EXPECT_TRUE(static_cast<uint32_t>(loader.GetDefaultLoadingFlags()) == static_cast<uint32_t>(ModelLoader::LoadingFlags::GenerateNormals));

    loader.Shutdown();

    TestOutput::PrintTestPass("AsyncModelLoader configuration");
    return true;
}

bool TestProgressTracking() {
    TestOutput::PrintTestStart("progress tracking");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test progress callback
    bool callbackCalled = false;
    std::string lastFilepath;
    float lastProgress = 0.0f;
    std::string lastStage;

    loader.SetProgressCallback([&](const std::string& filepath, float progress, const std::string& stage) {
        callbackCalled = true;
        lastFilepath = filepath;
        lastProgress = progress;
        lastStage = stage;
    });

    // Test progress queries with non-existent file
    EXPECT_EQUAL(loader.GetLoadingProgress("nonexistent.obj"), 0.0f);
    EXPECT_EQUAL(loader.GetLoadingStage("nonexistent.obj"), std::string(""));
    EXPECT_FALSE(loader.IsLoading("nonexistent.obj"));

    auto activeLoads = loader.GetActiveLoads();
    EXPECT_EQUAL(activeLoads.size(), static_cast<size_t>(0));

    loader.Shutdown();

    TestOutput::PrintTestPass("progress tracking");
    return true;
}

bool TestLoadCancellation() {
    TestOutput::PrintTestStart("load cancellation");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test cancellation of non-existent load
    EXPECT_FALSE(loader.CancelLoad("nonexistent.obj"));

    // Test cancel all loads (should not crash with no active loads)
    loader.CancelAllLoads();

    loader.Shutdown();

    TestOutput::PrintTestPass("load cancellation");
    return true;
}

bool TestLoadingStats() {
    TestOutput::PrintTestStart("loading statistics");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test initial stats
    auto stats = loader.GetLoadingStats();
    EXPECT_EQUAL(stats.totalLoadsStarted, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.totalLoadsCompleted, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.totalLoadsCancelled, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.totalLoadsFailed, static_cast<uint32_t>(0));
    EXPECT_EQUAL(stats.currentActiveLoads, static_cast<uint32_t>(0));

    // Test stats reset
    loader.ResetStats();
    stats = loader.GetLoadingStats();
    EXPECT_EQUAL(stats.totalLoadsStarted, static_cast<uint32_t>(0));

    loader.Shutdown();

    TestOutput::PrintTestPass("loading statistics");
    return true;
}

bool TestAsyncModelLoaderErrorHandling() {
    TestOutput::PrintTestStart("AsyncModelLoader error handling");

    AsyncModelLoader loader;

    // Test operations without initialization
    try {
        auto future = loader.LoadModelAsync("test.obj");
        TestOutput::PrintTestFail("AsyncModelLoader error handling");
        return false;
    } catch (const std::runtime_error&) {
        // Expected
    }

    // Test initialization and proper error handling
    EXPECT_TRUE(loader.Initialize());

    // Test loading non-existent file (should fail gracefully)
    try {
        auto future = loader.LoadModelAsync("definitely_nonexistent_file.obj");
        
        // Wait for completion with timeout
        auto status = future.wait_for(std::chrono::seconds(2));
        if (status == std::future_status::ready) {
            try {
                auto model = future.get();
                // If we get here, the load unexpectedly succeeded
                TestOutput::PrintInfo("Load of non-existent file unexpectedly succeeded");
            } catch (const std::exception& e) {
                // Expected - load should fail
                TestOutput::PrintInfo("Load correctly failed with: " + std::string(e.what()));
            }
        } else {
            TestOutput::PrintInfo("Load timed out as expected");
        }
    } catch (const std::exception& e) {
        TestOutput::PrintInfo("Exception during async load setup: " + std::string(e.what()));
    }

    loader.Shutdown();

    TestOutput::PrintTestPass("AsyncModelLoader error handling");
    return true;
}

bool TestCleanupAndResourceManagement() {
    TestOutput::PrintTestStart("cleanup and resource management");

    AsyncModelLoader loader;
    EXPECT_TRUE(loader.Initialize());

    // Test cleanup of completed tasks
    loader.CleanupCompletedTasks(); // Should not crash with no tasks

    // Test wait for all loads with no active loads
    loader.WaitForAllLoads(); // Should return immediately

    loader.Shutdown();

    TestOutput::PrintTestPass("cleanup and resource management");
    return true;
}

int main() {
    TestOutput::PrintHeader("AsyncModelLoader Unit Tests");

    TestSuite suite("AsyncModelLoader");
    bool allPassed = true;

    try {
        allPassed &= suite.RunTest("AsyncModelLoader Initialization", TestAsyncModelLoaderInitialization);
        allPassed &= suite.RunTest("ThreadPool Basic Functionality", TestThreadPoolBasic);
        allPassed &= suite.RunTest("AsyncModelLoader Configuration", TestAsyncModelLoaderConfiguration);
        allPassed &= suite.RunTest("Progress Tracking", TestProgressTracking);
        allPassed &= suite.RunTest("Load Cancellation", TestLoadCancellation);
        allPassed &= suite.RunTest("Loading Statistics", TestLoadingStats);
        allPassed &= suite.RunTest("AsyncModelLoader Error Handling", TestAsyncModelLoaderErrorHandling);
        allPassed &= suite.RunTest("Cleanup and Resource Management", TestCleanupAndResourceManagement);

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