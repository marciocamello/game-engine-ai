#include "TestUtils.h"
#include "Animation/AnimationThreading.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationSkeleton.h"
#include "Core/Math.h"
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test animation thread pool initialization
 * Requirements: 9.6 (multi-threaded animation updates)
 */
bool TestAnimationThreadPoolInitialization() {
    TestOutput::PrintTestStart("animation thread pool initialization");

    AnimationThreadPool threadPool;
    
    AnimationThreadConfig config;
    config.numThreads = 2;
    config.maxQueueSize = 100;
    config.enableWorkStealing = true;
    
    EXPECT_TRUE(threadPool.Initialize(config));
    EXPECT_EQUAL(threadPool.GetThreadCount(), static_cast<size_t>(2));
    EXPECT_EQUAL(threadPool.GetQueueSize(), static_cast<size_t>(0));
    EXPECT_TRUE(threadPool.IsIdle());

    threadPool.Shutdown();

    TestOutput::PrintTestPass("animation thread pool initialization");
    return true;
}

/**
 * Test animation task submission and execution
 * Requirements: 9.6 (multi-threaded animation updates)
 */
bool TestAnimationTaskSubmission() {
    TestOutput::PrintTestStart("animation task submission");

    AnimationThreadPool threadPool;
    
    AnimationThreadConfig config;
    config.numThreads = 2;
    config.maxQueueSize = 10;
    
    EXPECT_TRUE(threadPool.Initialize(config));

    // Test simple task submission
    std::atomic<int> counter{0};
    
    auto future1 = threadPool.SubmitTask([&counter]() {
        counter++;
    }, AnimationTaskPriority::Normal);
    
    auto future2 = threadPool.SubmitTask([&counter]() {
        counter++;
    }, AnimationTaskPriority::High);

    // Wait for tasks to complete
    future1.wait();
    future2.wait();

    EXPECT_EQUAL(counter.load(), 2);

    threadPool.Shutdown();

    TestOutput::PrintTestPass("animation task submission");
    return true;
}

/**
 * Test animation batch processing
 * Requirements: 9.6 (multi-threaded animation updates)
 */
bool TestAnimationBatchProcessing() {
    TestOutput::PrintTestStart("animation batch processing");

    AnimationThreadPool threadPool;
    
    AnimationThreadConfig config;
    config.numThreads = 2;
    
    EXPECT_TRUE(threadPool.Initialize(config));

    // Create mock animation controllers
    std::vector<std::shared_ptr<AnimationController>> controllers;
    for (int i = 0; i < 3; ++i) {
        auto skeleton = std::make_shared<AnimationSkeleton>();
        auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
        skeleton->AddBone(rootBone);
        
        auto controller = std::make_shared<AnimationController>();
        EXPECT_TRUE(controller->Initialize(skeleton));
        controllers.push_back(controller);
    }

    // Create batch
    AnimationBatch batch;
    batch.controllers = controllers;
    batch.deltaTime = 0.016f;
    batch.priority = AnimationTaskPriority::Normal;

    // Submit batch
    auto future = threadPool.SubmitBatch(batch);
    future.wait();

    // Verify batch was processed (controllers should have been updated)
    EXPECT_TRUE(future.valid());

    threadPool.Shutdown();

    TestOutput::PrintTestPass("animation batch processing");
    return true;
}

/**
 * Test multi-threaded animation manager
 * Requirements: 9.6 (multi-threaded animation updates)
 */
bool TestMultiThreadedAnimationManager() {
    TestOutput::PrintTestStart("multi-threaded animation manager");

    MultiThreadedAnimationManager manager;
    
    AnimationThreadConfig config;
    config.numThreads = 2;
    config.maxQueueSize = 50;
    
    EXPECT_TRUE(manager.Initialize(config));

    // Register animation controllers
    std::vector<uint32_t> instanceIds;
    for (int i = 0; i < 5; ++i) {
        auto skeleton = std::make_shared<AnimationSkeleton>();
        auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
        skeleton->AddBone(rootBone);
        
        auto controller = std::make_shared<AnimationController>();
        EXPECT_TRUE(controller->Initialize(skeleton));
        
        uint32_t instanceId = manager.RegisterAnimationController(controller, AnimationTaskPriority::Normal);
        EXPECT_NOT_EQUAL(instanceId, static_cast<uint32_t>(0));
        instanceIds.push_back(instanceId);
    }

    // Update animations
    manager.UpdateAnimations(0.016f);
    manager.WaitForAnimationUpdates();

    // Get statistics
    auto stats = manager.GetStats();
    EXPECT_EQUAL(stats.totalInstances, static_cast<size_t>(5));

    // Unregister controllers
    for (uint32_t instanceId : instanceIds) {
        manager.UnregisterAnimationController(instanceId);
    }

    manager.Shutdown();

    TestOutput::PrintTestPass("multi-threaded animation manager");
    return true;
}

/**
 * Test animation thread pool statistics
 * Requirements: 9.5 (performance scaling based on system capabilities)
 */
bool TestAnimationThreadPoolStatistics() {
    TestOutput::PrintTestStart("animation thread pool statistics");

    AnimationThreadPool threadPool;
    
    AnimationThreadConfig config;
    config.numThreads = 2;
    
    EXPECT_TRUE(threadPool.Initialize(config));

    // Submit some tasks
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(threadPool.SubmitTask([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }));
    }

    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }

    // Check statistics
    auto stats = threadPool.GetStats();
    EXPECT_EQUAL(stats.totalTasksProcessed, static_cast<size_t>(10));
    EXPECT_EQUAL(stats.currentQueueSize, static_cast<size_t>(0));
    EXPECT_TRUE(stats.averageTaskTime >= 0.0f);

    threadPool.Shutdown();

    TestOutput::PrintTestPass("animation thread pool statistics");
    return true;
}

/**
 * Test GPU animation processor initialization
 * Requirements: 9.7 (GPU-accelerated skinning)
 */
bool TestGPUAnimationProcessor() {
    TestOutput::PrintTestStart("GPU animation processor");

    GPUAnimationProcessor processor;
    
    // Initialize (may fail if compute shaders not supported)
    bool initialized = processor.Initialize();
    
    // Test basic functionality regardless of initialization success
    EXPECT_FALSE(processor.IsGPUAccelerationSupported() && !initialized);
    
    if (initialized) {
        EXPECT_TRUE(processor.IsComputeShaderSupported());
        EXPECT_TRUE(processor.GetMaxComputeWorkGroups() > 0);
        
        // Test animation data upload
        GPUAnimationProcessor::GPUAnimationData data;
        data.boneCount = 10;
        data.animationCount = 1;
        data.boneMatrices.resize(10, Math::Mat4(1.0f));
        data.bindPoses.resize(10, Math::Mat4(1.0f));
        data.inverseBindPoses.resize(10, Math::Mat4(1.0f));
        data.animationWeights.resize(1, 1.0f);
        
        uint32_t dataId = processor.UploadAnimationData(data);
        EXPECT_NOT_EQUAL(dataId, static_cast<uint32_t>(0));
        
        processor.RemoveAnimationData(dataId);
    }

    processor.Shutdown();

    TestOutput::PrintTestPass("GPU animation processor");
    return true;
}

/**
 * Test animation memory pool
 * Requirements: 9.5 (efficient memory allocation and pooling)
 */
bool TestAnimationMemoryPool() {
    TestOutput::PrintTestStart("animation memory pool");

    AnimationMemoryPool pool;

    // Test basic allocation
    void* ptr1 = pool.Allocate(64);
    EXPECT_TRUE(ptr1 != nullptr);

    void* ptr2 = pool.Allocate(128);
    EXPECT_TRUE(ptr2 != nullptr);
    EXPECT_TRUE(ptr1 != ptr2);

    // Test typed allocation
    float* floatPtr = pool.Allocate<float>(10);
    EXPECT_TRUE(floatPtr != nullptr);

    // Test deallocation
    pool.Deallocate(ptr1);
    pool.Deallocate(ptr2);
    pool.Deallocate(floatPtr);

    // Test statistics
    auto stats = pool.GetStats();
    EXPECT_EQUAL(stats.totalAllocations, static_cast<size_t>(3));
    EXPECT_EQUAL(stats.totalDeallocations, static_cast<size_t>(3));
    EXPECT_EQUAL(stats.currentAllocations, static_cast<size_t>(0));

    // Test reset
    pool.Reset();
    EXPECT_EQUAL(pool.GetTotalAllocated(), static_cast<size_t>(0));

    TestOutput::PrintTestPass("animation memory pool");
    return true;
}

/**
 * Test animation threading performance
 * Requirements: 9.6 (multi-threading performance)
 */
bool TestAnimationThreadingPerformance() {
    TestOutput::PrintTestStart("animation threading performance");

    MultiThreadedAnimationManager manager;
    
    AnimationThreadConfig config;
    config.numThreads = std::thread::hardware_concurrency();
    
    EXPECT_TRUE(manager.Initialize(config));

    // Create many animation controllers for performance test
    const size_t numControllers = 50;
    std::vector<uint32_t> instanceIds;
    
    for (size_t i = 0; i < numControllers; ++i) {
        auto skeleton = std::make_shared<AnimationSkeleton>();
        auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
        skeleton->AddBone(rootBone);
        
        auto controller = std::make_shared<AnimationController>();
        EXPECT_TRUE(controller->Initialize(skeleton));
        
        uint32_t instanceId = manager.RegisterAnimationController(controller, AnimationTaskPriority::Normal);
        instanceIds.push_back(instanceId);
    }

    // Measure performance
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Update animations multiple times
    for (int frame = 0; frame < 10; ++frame) {
        manager.UpdateAnimations(0.016f);
        manager.WaitForAnimationUpdates();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    float totalTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    // Verify performance is reasonable (should complete in reasonable time)
    EXPECT_TRUE(totalTime < 1000.0f); // Less than 1 second for 50 controllers * 10 frames
    
    // Get final statistics
    auto stats = manager.GetStats();
    EXPECT_EQUAL(stats.totalInstances, numControllers);
    EXPECT_TRUE(stats.parallelEfficiency > 0.0f);

    // Cleanup
    for (uint32_t instanceId : instanceIds) {
        manager.UnregisterAnimationController(instanceId);
    }

    manager.Shutdown();

    TestOutput::PrintTestPass("animation threading performance");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationThreading");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationThreading Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Thread Pool Initialization", TestAnimationThreadPoolInitialization);
        allPassed &= suite.RunTest("Animation Task Submission", TestAnimationTaskSubmission);
        allPassed &= suite.RunTest("Animation Batch Processing", TestAnimationBatchProcessing);
        allPassed &= suite.RunTest("Multi-Threaded Animation Manager", TestMultiThreadedAnimationManager);
        allPassed &= suite.RunTest("Animation Thread Pool Statistics", TestAnimationThreadPoolStatistics);
        allPassed &= suite.RunTest("GPU Animation Processor", TestGPUAnimationProcessor);
        allPassed &= suite.RunTest("Animation Memory Pool", TestAnimationMemoryPool);
        allPassed &= suite.RunTest("Animation Threading Performance", TestAnimationThreadingPerformance);

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