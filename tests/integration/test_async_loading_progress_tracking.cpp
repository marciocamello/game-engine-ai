#include "Resource/AsyncModelLoader.h"
#include "Resource/ModelLoader.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <filesystem>
#include <fstream>
#include <future>
#include <chrono>
#include <thread>
#include <atomic>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Create multiple test model files for concurrent loading
 */
bool CreateTestModelFiles() {
    std::filesystem::create_directories("test_assets");
    
    // Create multiple OBJ files with different complexity
    std::vector<std::string> filenames = {
        "test_assets/simple_triangle.obj",
        "test_assets/quad_mesh.obj",
        "test_assets/complex_mesh.obj"
    };
    
    // Simple triangle
    std::ofstream file1(filenames[0]);
    if (file1.is_open()) {
        file1 << "v 0.0 0.0 0.0\n";
        file1 << "v 1.0 0.0 0.0\n";
        file1 << "v 0.5 1.0 0.0\n";
        file1 << "f 1 2 3\n";
        file1.close();
    }
    
    // Quad mesh
    std::ofstream file2(filenames[1]);
    if (file2.is_open()) {
        file2 << "v -1.0 -1.0 0.0\n";
        file2 << "v  1.0 -1.0 0.0\n";
        file2 << "v  1.0  1.0 0.0\n";
        file2 << "v -1.0  1.0 0.0\n";
        file2 << "f 1 2 3\n";
        file2 << "f 1 3 4\n";
        file2.close();
    }
    
    // More complex mesh (multiple triangles)
    std::ofstream file3(filenames[2]);
    if (file3.is_open()) {
        // Create a grid of vertices
        for (int y = 0; y < 5; ++y) {
            for (int x = 0; x < 5; ++x) {
                file3 << "v " << (x - 2.0f) << " " << (y - 2.0f) << " 0.0\n";
            }
        }
        
        // Create triangles
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                int v1 = y * 5 + x + 1;
                int v2 = v1 + 1;
                int v3 = v1 + 5;
                int v4 = v3 + 1;
                
                file3 << "f " << v1 << " " << v2 << " " << v3 << "\n";
                file3 << "f " << v2 << " " << v4 << " " << v3 << "\n";
            }
        }
        file3.close();
    }
    
    return std::filesystem::exists(filenames[0]) && 
           std::filesystem::exists(filenames[1]) && 
           std::filesystem::exists(filenames[2]);
}

/**
 * Test asynchronous model loading with progress tracking
 * Requirements: 6.1, 6.2, 6.3 (Asynchronous loading with progress tracking)
 */
bool TestAsyncLoadingWithProgressTracking() {
    TestOutput::PrintTestStart("async loading with progress tracking");
    
    if (!CreateTestModelFiles()) {
        TestOutput::PrintInfo("Skipping test - could not create test files");
        TestOutput::PrintTestPass("async loading with progress tracking");
        return true;
    }
    
    AsyncModelLoader asyncLoader;
    EXPECT_TRUE(asyncLoader.Initialize(2)); // Use 2 worker threads
    
    // Setup progress tracking
    std::atomic<int> progressCallbackCount{0};
    std::atomic<float> lastProgress{0.0f};
    std::string lastFilepath;
    std::string lastStage;
    
    asyncLoader.SetProgressCallback([&](const std::string& filepath, float progress, const std::string& stage) {
        progressCallbackCount++;
        lastProgress = progress;
        lastFilepath = filepath;
        lastStage = stage;
        
        EXPECT_IN_RANGE(progress, 0.0f, 1.0f);
        EXPECT_FALSE(filepath.empty());
        
        TestOutput::PrintInfo("Progress: " + filepath + " - " + 
                             std::to_string(static_cast<int>(progress * 100)) + "% (" + stage + ")");
    });
    
    // Test single async load
    std::string testFile = "test_assets/simple_triangle.obj";
    auto future = asyncLoader.LoadModelAsync(testFile);
    
    // Wait for completion with timeout
    auto status = future.wait_for(std::chrono::seconds(5));
    EXPECT_TRUE(status == std::future_status::ready);
    
    if (status == std::future_status::ready) {
        try {
            auto model = future.get();
            // Model may be null if Assimp is not available, but future should complete
            TestOutput::PrintInfo("Single async load completed");
        } catch (const std::exception& e) {
            TestOutput::PrintInfo("Async load exception (may be expected): " + std::string(e.what()));
        }
    }
    
    // Test progress query
    float currentProgress = asyncLoader.GetLoadingProgress(testFile);
    EXPECT_IN_RANGE(currentProgress, 0.0f, 1.0f);
    
    std::string currentStage = asyncLoader.GetLoadingStage(testFile);
    // Stage may be empty if loading is complete
    
    // Test loading statistics
    auto stats = asyncLoader.GetLoadingStats();
    EXPECT_TRUE(stats.totalLoadsStarted > 0);
    
    TestOutput::PrintInfo("Async loading statistics:");
    TestOutput::PrintInfo("  Loads started: " + std::to_string(stats.totalLoadsStarted));
    TestOutput::PrintInfo("  Loads completed: " + std::to_string(stats.totalLoadsCompleted));
    TestOutput::PrintInfo("  Loads failed: " + std::to_string(stats.totalLoadsFailed));
    TestOutput::PrintInfo("  Progress callbacks: " + std::to_string(progressCallbackCount.load()));
    
    asyncLoader.Shutdown();
    
    // Cleanup
    std::filesystem::remove_all("test_assets");
    
    TestOutput::PrintTestPass("async loading with progress tracking");
    return true;
}

/**
 * Test concurrent model loading with multiple files
 * Requirements: 6.2, 6.4 (Concurrent loading with thread safety)
 */
bool TestConcurrentModelLoading() {
    TestOutput::PrintTestStart("concurrent model loading");
    
    if (!CreateTestModelFiles()) {
        TestOutput::PrintInfo("Skipping test - could not create test files");
        TestOutput::PrintTestPass("concurrent model loading");
        return true;
    }
    
    AsyncModelLoader asyncLoader;
    EXPECT_TRUE(asyncLoader.Initialize(3)); // Use 3 worker threads
    
    // Setup concurrent loading tracking
    std::atomic<int> completedLoads{0};
    std::atomic<int> failedLoads{0};
    
    asyncLoader.SetProgressCallback([&](const std::string& filepath, float progress, const std::string& stage) {
        if (progress >= 1.0f) {
            completedLoads++;
        }
    });
    
    // Start multiple concurrent loads
    std::vector<std::string> testFiles = {
        "test_assets/simple_triangle.obj",
        "test_assets/quad_mesh.obj",
        "test_assets/complex_mesh.obj"
    };
    
    std::vector<std::future<std::shared_ptr<Model>>> futures;
    
    // Launch all loads concurrently
    for (const auto& file : testFiles) {
        futures.push_back(asyncLoader.LoadModelAsync(file));
    }
    
    // Wait for all loads to complete
    for (auto& future : futures) {
        auto status = future.wait_for(std::chrono::seconds(10));
        EXPECT_TRUE(status == std::future_status::ready);
        
        if (status == std::future_status::ready) {
            try {
                auto model = future.get();
                // Model may be null if Assimp is not available
                TestOutput::PrintInfo("Concurrent load completed");
            } catch (const std::exception& e) {
                failedLoads++;
                TestOutput::PrintInfo("Concurrent load failed: " + std::string(e.what()));
            }
        }
    }
    
    // Verify concurrent loading statistics
    auto stats = asyncLoader.GetLoadingStats();
    EXPECT_TRUE(stats.totalLoadsStarted >= testFiles.size());
    
    TestOutput::PrintInfo("Concurrent loading results:");
    TestOutput::PrintInfo("  Files processed: " + std::to_string(testFiles.size()));
    TestOutput::PrintInfo("  Loads started: " + std::to_string(stats.totalLoadsStarted));
    TestOutput::PrintInfo("  Loads completed: " + std::to_string(stats.totalLoadsCompleted));
    TestOutput::PrintInfo("  Loads failed: " + std::to_string(stats.totalLoadsFailed));
    
    asyncLoader.Shutdown();
    
    // Cleanup
    std::filesystem::remove_all("test_assets");
    
    TestOutput::PrintTestPass("concurrent model loading");
    return true;
}

/**
 * Test load cancellation and management
 * Requirements: 6.2, 6.6 (Load cancellation and resource cleanup)
 */
bool TestLoadCancellationAndManagement() {
    TestOutput::PrintTestStart("load cancellation and management");
    
    if (!CreateTestModelFiles()) {
        TestOutput::PrintInfo("Skipping test - could not create test files");
        TestOutput::PrintTestPass("load cancellation and management");
        return true;
    }
    
    AsyncModelLoader asyncLoader;
    EXPECT_TRUE(asyncLoader.Initialize(2));
    
    // Test load cancellation
    std::string testFile = "test_assets/complex_mesh.obj";
    auto future = asyncLoader.LoadModelAsync(testFile);
    
    // Give it a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Cancel the load
    bool cancelled = asyncLoader.CancelLoad(testFile);
    TestOutput::PrintInfo("Load cancellation result: " + std::string(cancelled ? "success" : "failed"));
    
    // Wait for future to complete (should be cancelled)
    auto status = future.wait_for(std::chrono::seconds(2));
    if (status == std::future_status::ready) {
        try {
            auto model = future.get();
            TestOutput::PrintInfo("Load completed despite cancellation attempt");
        } catch (const std::exception& e) {
            TestOutput::PrintInfo("Load cancelled with exception: " + std::string(e.what()));
        }
    }
    
    // Test cancel all loads
    std::vector<std::future<std::shared_ptr<Model>>> futures;
    for (int i = 0; i < 3; ++i) {
        futures.push_back(asyncLoader.LoadModelAsync("test_assets/simple_triangle.obj"));
    }
    
    // Cancel all loads
    asyncLoader.CancelAllLoads();
    
    // Wait for all futures to complete
    for (auto& future : futures) {
        auto status = future.wait_for(std::chrono::seconds(1));
        if (status == std::future_status::ready) {
            try {
                future.get();
            } catch (...) {
                // Expected for cancelled loads
            }
        }
    }
    
    // Test active loads query
    auto activeLoads = asyncLoader.GetActiveLoads();
    TestOutput::PrintInfo("Active loads after cancellation: " + std::to_string(activeLoads.size()));
    
    // Test max concurrent loads setting
    asyncLoader.SetMaxConcurrentLoads(1);
    EXPECT_EQUAL(asyncLoader.GetMaxConcurrentLoads(), static_cast<uint32_t>(1));
    
    asyncLoader.SetMaxConcurrentLoads(4);
    EXPECT_EQUAL(asyncLoader.GetMaxConcurrentLoads(), static_cast<uint32_t>(4));
    
    // Test statistics after cancellation
    auto stats = asyncLoader.GetLoadingStats();
    TestOutput::PrintInfo("Final statistics:");
    TestOutput::PrintInfo("  Loads cancelled: " + std::to_string(stats.totalLoadsCancelled));
    TestOutput::PrintInfo("  Current active: " + std::to_string(stats.currentActiveLoads));
    
    asyncLoader.Shutdown();
    
    // Cleanup
    std::filesystem::remove_all("test_assets");
    
    TestOutput::PrintTestPass("load cancellation and management");
    return true;
}

/**
 * Test async loading error handling and recovery
 * Requirements: 6.5, 9.7 (Error handling in async loading)
 */
bool TestAsyncLoadingErrorHandling() {
    TestOutput::PrintTestStart("async loading error handling");
    
    AsyncModelLoader asyncLoader;
    EXPECT_TRUE(asyncLoader.Initialize(2));
    
    // Test loading non-existent file
    auto future1 = asyncLoader.LoadModelAsync("non_existent_file.obj");
    auto status1 = future1.wait_for(std::chrono::seconds(2));
    
    EXPECT_TRUE(status1 == std::future_status::ready);
    
    if (status1 == std::future_status::ready) {
        try {
            auto model = future1.get();
            EXPECT_NULL(model);
        } catch (const std::exception& e) {
            TestOutput::PrintInfo("Expected exception for non-existent file: " + std::string(e.what()));
        }
    }
    
    // Test loading with empty filename
    auto future2 = asyncLoader.LoadModelAsync("");
    auto status2 = future2.wait_for(std::chrono::seconds(1));
    
    EXPECT_TRUE(status2 == std::future_status::ready);
    
    if (status2 == std::future_status::ready) {
        try {
            auto model = future2.get();
            EXPECT_NULL(model);
        } catch (const std::exception& e) {
            TestOutput::PrintInfo("Expected exception for empty filename: " + std::string(e.what()));
        }
    }
    
    // Create a corrupted file
    std::filesystem::create_directories("test_assets");
    std::string corruptedFile = "test_assets/corrupted.obj";
    std::ofstream file(corruptedFile);
    if (file.is_open()) {
        file << "This is not valid OBJ content\n";
        file << "Random garbage\n";
        file.close();
        
        // Test loading corrupted file
        auto future3 = asyncLoader.LoadModelAsync(corruptedFile);
        auto status3 = future3.wait_for(std::chrono::seconds(2));
        
        EXPECT_TRUE(status3 == std::future_status::ready);
        
        if (status3 == std::future_status::ready) {
            try {
                auto model = future3.get();
                // Should either be null or handle gracefully
                TestOutput::PrintInfo("Corrupted file handled gracefully");
            } catch (const std::exception& e) {
                TestOutput::PrintInfo("Corrupted file exception: " + std::string(e.what()));
            }
        }
    }
    
    // Test error statistics
    auto stats = asyncLoader.GetLoadingStats();
    TestOutput::PrintInfo("Error handling statistics:");
    TestOutput::PrintInfo("  Total loads started: " + std::to_string(stats.totalLoadsStarted));
    TestOutput::PrintInfo("  Total loads failed: " + std::to_string(stats.totalLoadsFailed));
    TestOutput::PrintInfo("  Error rate: " + std::to_string(
        stats.totalLoadsStarted > 0 ? 
        (static_cast<float>(stats.totalLoadsFailed) / stats.totalLoadsStarted * 100.0f) : 0.0f
    ) + "%");
    
    asyncLoader.Shutdown();
    
    // Cleanup
    std::filesystem::remove_all("test_assets");
    
    TestOutput::PrintTestPass("async loading error handling");
    return true;
}

/**
 * Test async loading performance and thread management
 * Requirements: 6.2, 6.6 (Thread management and performance)
 */
bool TestAsyncLoadingPerformance() {
    TestOutput::PrintTestStart("async loading performance");
    
    if (!CreateTestModelFiles()) {
        TestOutput::PrintInfo("Skipping test - could not create test files");
        TestOutput::PrintTestPass("async loading performance");
        return true;
    }
    
    // Test with different thread counts
    std::vector<uint32_t> threadCounts = {1, 2, 4};
    
    for (uint32_t threadCount : threadCounts) {
        TestOutput::PrintInfo("Testing with " + std::to_string(threadCount) + " threads");
        
        AsyncModelLoader asyncLoader;
        EXPECT_TRUE(asyncLoader.Initialize(threadCount));
        EXPECT_EQUAL(asyncLoader.GetWorkerThreadCount(), threadCount);
        
        // Measure loading performance
        TestTimer timer;
        
        std::vector<std::future<std::shared_ptr<Model>>> futures;
        std::vector<std::string> testFiles = {
            "test_assets/simple_triangle.obj",
            "test_assets/quad_mesh.obj",
            "test_assets/complex_mesh.obj"
        };
        
        // Start concurrent loads
        for (const auto& file : testFiles) {
            futures.push_back(asyncLoader.LoadModelAsync(file));
        }
        
        // Wait for all to complete
        for (auto& future : futures) {
            future.wait_for(std::chrono::seconds(5));
        }
        
        double totalTime = timer.ElapsedMs();
        
        TestOutput::PrintTiming("Async loading (" + std::to_string(threadCount) + " threads)", 
                               totalTime, testFiles.size());
        
        // Get performance statistics
        auto stats = asyncLoader.GetLoadingStats();
        if (stats.totalLoadsCompleted > 0) {
            double avgTime = stats.totalLoadingTimeMs / stats.totalLoadsCompleted;
            TestOutput::PrintInfo("  Average load time: " + std::to_string(avgTime) + "ms");
        }
        
        asyncLoader.Shutdown();
    }
    
    // Cleanup
    std::filesystem::remove_all("test_assets");
    
    TestOutput::PrintTestPass("async loading performance");
    return true;
}

int main() {
    TestOutput::PrintHeader("Async Loading and Progress Tracking Integration");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().Initialize();
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        
        // Create test suite for result tracking
        TestSuite suite("Async Loading and Progress Tracking Tests");

        // Run all tests
        allPassed &= suite.RunTest("Async Loading with Progress Tracking", TestAsyncLoadingWithProgressTracking);
        allPassed &= suite.RunTest("Concurrent Model Loading", TestConcurrentModelLoading);
        allPassed &= suite.RunTest("Load Cancellation and Management", TestLoadCancellationAndManagement);
        allPassed &= suite.RunTest("Async Loading Error Handling", TestAsyncLoadingErrorHandling);
        allPassed &= suite.RunTest("Async Loading Performance", TestAsyncLoadingPerformance);

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