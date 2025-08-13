# Resource Testing Best Practices Guide

## Table of Contents

1. [Overview](#overview)
2. [ResourceManager Testing Patterns](#resourcemanager-testing-patterns)
3. [Cache Validation Testing](#cache-validation-testing)
4. [Memory Management Testing](#memory-management-testing)
5. [Performance Testing Guidelines](#performance-testing-guidelines)
6. [Integration Testing Patterns](#integration-testing-patterns)
7. [Mock Resource Implementation](#mock-resource-implementation)
8. [Error Handling and Edge Cases](#error-handling-and-edge-cases)
9. [Best Practices Summary](#best-practices-summary)

## Overview

This guide provides comprehensive patterns and best practices for testing resource management functionality in Game Engine Kiro. It covers testing ResourceManager operations, cache behavior validation, memory management verification, and performance benchmarking for resource operations.

### Key Testing Principles

- **Context-Aware Testing**: Handle OpenGL context limitations gracefully
- **Mock Resource Usage**: Use lightweight mock resources for logic testing
- **Memory Tracking**: Validate memory usage and cleanup behavior
- **Performance Validation**: Ensure resource operations meet performance requirements
- **Cache Behavior**: Verify caching mechanisms work correctly
- **Error Resilience**: Test error conditions and recovery scenarios

### Related Documentation

**Core Testing Framework:**

- **[Testing Complete Guide](testing-complete-guide.md)**: Comprehensive testing instructions with resource examples
- **[Testing Standards](testing-standards.md)**: Coding standards for resource test implementation

**Context and Mocking:**

- **[OpenGL Context Limitations](testing-opengl-limitations.md)**: Essential for graphics resource testing
- **[Mock Resource Implementation](testing-mock-resources.md)**: Detailed mock resource patterns used in this guide

**Output and Quality:**

- **[Test Output Formatting](testing-output-formatting.md)**: Proper formatting for resource test output
- **[Code Examples Validation](testing-code-examples-validation.md)**: Validating resource-related examples

**API Documentation:**

- **[API Reference](api-reference.md)**: ResourceManager API and OpenGLContext utilities

**Advanced Topics:**

- **[Testing Complete Guide](testing-complete-guide.md)**: Strategic approach to resource testing

## ResourceManager Testing Patterns

### Basic ResourceManager Functionality

Test core ResourceManager operations using mock resources to avoid OpenGL dependencies:

```cpp
#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

// Mock resource for testing (no OpenGL dependencies)
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

bool TestResourceManagerInitialization() {
    TestOutput::PrintTestStart("resource manager initialization");

    ResourceManager manager;

    // Test initialization
    EXPECT_TRUE(manager.Initialize());

    // Test initial state
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(manager.GetMemoryUsage(), static_cast<size_t>(0));

    // Test statistics
    ResourceStats stats = manager.GetResourceStats();
    EXPECT_EQUAL(stats.totalResources, static_cast<size_t>(0));
    EXPECT_EQUAL(stats.totalMemoryUsage, static_cast<size_t>(0));

    TestOutput::PrintTestPass("resource manager initialization");
    return true;
}
```

bool TestBasicResourceLoading() {
TestOutput::PrintTestStart("basic resource loading");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load a mock resource
    auto resource = manager.Load<MockResource>("test/basic.dat");

    // Validate resource creation
    EXPECT_NOT_NULL(resource);
    EXPECT_STRING_EQUAL(resource->GetPath(), "assets/test/basic.dat");
    EXPECT_TRUE(resource->GetMemoryUsage() > 0);

    // Validate manager statistics
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));
    EXPECT_TRUE(manager.GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("basic resource loading");
    return true;

}

bool TestResourceUnloading() {
TestOutput::PrintTestStart("resource unloading");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load resources
    auto resource1 = manager.Load<MockResource>("test/unload1.dat");
    auto resource2 = manager.Load<MockResource>("test/unload2.dat");

    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(2));

    // Unload specific resource
    manager.Unload<MockResource>("test/unload1.dat");

    // Resource should still exist if shared_ptr is held
    EXPECT_NOT_NULL(resource1);

    // Clear reference and test cleanup
    resource1.reset();
    manager.UnloadUnused();

    // Should have one less resource
    EXPECT_TRUE(manager.GetResourceCount() <= 1);

    TestOutput::PrintTestPass("resource unloading");
    return true;

}

````

### Resource Type Validation

Test ResourceManager with different resource types:

```cpp
// Specialized mock resources for type testing
class MockTexture : public Resource {
public:
    MockTexture(const std::string& path) : Resource(path), m_width(256), m_height(256) {}
    size_t GetMemoryUsage() const override { return m_width * m_height * 4; }

private:
    int m_width, m_height;
};

class MockMesh : public Resource {
public:
    MockMesh(const std::string& path) : Resource(path), m_vertexCount(1000) {}
    size_t GetMemoryUsage() const override { return m_vertexCount * 32; } // 32 bytes per vertex

private:
    size_t m_vertexCount;
};

bool TestMultipleResourceTypes() {
    TestOutput::PrintTestStart("multiple resource types");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load different resource types
    auto texture = manager.Load<MockTexture>("textures/test.png");
    auto mesh = manager.Load<MockMesh>("meshes/test.obj");
    auto generic = manager.Load<MockResource>("data/test.dat");

    // Validate all resources loaded
    EXPECT_NOT_NULL(texture);
    EXPECT_NOT_NULL(mesh);
    EXPECT_NOT_NULL(generic);

    // Check resource count
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(3));

    // Test resource statistics by type
    ResourceStats stats = manager.GetResourceStats();
    EXPECT_EQUAL(stats.totalResources, static_cast<size_t>(3));
    EXPECT_TRUE(stats.resourcesByType.size() >= 3); // At least 3 different types

    TestOutput::PrintTestPass("multiple resource types");
    return true;
}
````

## Cache Validation Testing

### Cache Hit/Miss Testing

Verify that ResourceManager correctly caches and reuses resources:

```cpp
bool TestResourceCaching() {
    TestOutput::PrintTestStart("resource caching");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    const std::string resourcePath = "test/cache_test.dat";

    // First load - should be cache miss
    auto resource1 = manager.Load<MockResource>(resourcePath);
    EXPECT_NOT_NULL(resource1);

    // Second load - should be cache hit (same instance)
    auto resource2 = manager.Load<MockResource>(resourcePath);
    EXPECT_NOT_NULL(resource2);
    EXPECT_TRUE(resource1.get() == resource2.get());

    // Verify only one resource in manager
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));

    // Test with different path - should be different instance
    auto resource3 = manager.Load<MockResource>("test/different.dat");
    EXPECT_NOT_NULL(resource3);
    EXPECT_TRUE(resource1.get() != resource3.get());
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(2));

    TestOutput::PrintTestPass("resource caching");
    return true;
}

bool TestCacheInvalidation() {
    TestOutput::PrintTestStart("cache invalidation");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    const std::string resourcePath = "test/invalidation.dat";

    // Load resource
    auto resource1 = manager.Load<MockResource>(resourcePath);
    EXPECT_NOT_NULL(resource1);

    // Clear reference
    resource1.reset();

    // Clean up unused resources
    manager.UnloadUnused();

    // Load same resource again - should create new instance
    auto resource2 = manager.Load<MockResource>(resourcePath);
    EXPECT_NOT_NULL(resource2);

    // Should be different instance (cache was invalidated)
    EXPECT_TRUE(resource1.get() != resource2.get());

    TestOutput::PrintTestPass("cache invalidation");
    return true;
}

bool TestCacheAccessTimeTracking() {
    TestOutput::PrintTestStart("cache access time tracking");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load resource
    auto resource = manager.Load<MockResource>("test/access_time.dat");
    EXPECT_NOT_NULL(resource);

    auto initialAccessTime = resource->GetLastAccessTime();

    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Access resource again
    auto sameResource = manager.Load<MockResource>("test/access_time.dat");
    EXPECT_TRUE(resource.get() == sameResource.get());

    // Access time should be updated
    auto updatedAccessTime = resource->GetLastAccessTime();
    EXPECT_TRUE(updatedAccessTime > initialAccessTime);

    TestOutput::PrintTestPass("cache access time tracking");
    return true;
}
```

### Cache Performance Testing

Validate cache performance characteristics:

```cpp
bool TestCachePerformance() {
    TestOutput::PrintTestStart("cache performance");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    const std::string resourcePath = "test/performance.dat";

    // First load (cache miss) - measure time
    TestTimer timer;
    auto resource1 = manager.Load<MockResource>(resourcePath);
    double firstLoadTime = timer.ElapsedMs();

    EXPECT_NOT_NULL(resource1);

    // Second load (cache hit) - measure time
    timer.Restart();
    auto resource2 = manager.Load<MockResource>(resourcePath);
    double secondLoadTime = timer.ElapsedMs();

    EXPECT_NOT_NULL(resource2);
    EXPECT_TRUE(resource1.get() == resource2.get());

    // Cache hit should be significantly faster
    EXPECT_TRUE(secondLoadTime < firstLoadTime);

    TestOutput::PrintTiming("first load (cache miss)", firstLoadTime);
    TestOutput::PrintTiming("second load (cache hit)", secondLoadTime);

    TestOutput::PrintTestPass("cache performance");
    return true;
}
```

## Memory Management Testing

### Memory Usage Tracking

Test ResourceManager's memory tracking capabilities:

```cpp
bool TestMemoryUsageTracking() {
    TestOutput::PrintTestStart("memory usage tracking");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Initial memory should be zero
    EXPECT_EQUAL(manager.GetMemoryUsage(), static_cast<size_t>(0));

    // Load resources with known memory usage
    auto resource1 = manager.Load<MockResource>("test/memory1.dat");
    resource1->SetSize(1024); // 1KB

    auto resource2 = manager.Load<MockResource>("test/memory2.dat");
    resource2->SetSize(2048); // 2KB

    // Check total memory usage
    size_t totalMemory = manager.GetMemoryUsage();
    EXPECT_TRUE(totalMemory >= 3072); // At least 3KB (plus base Resource size)

    // Test detailed statistics
    ResourceStats stats = manager.GetResourceStats();
    EXPECT_EQUAL(stats.totalResources, static_cast<size_t>(2));
    EXPECT_TRUE(stats.totalMemoryUsage >= 3072);

    TestOutput::PrintTestPass("memory usage tracking");
    return true;
}
```

bool TestMemoryPressureHandling() {
TestOutput::PrintTestStart("memory pressure handling");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Set low memory threshold to trigger pressure handling
    manager.SetMemoryPressureThreshold(4096); // 4KB threshold

    // Load resources that exceed threshold
    std::vector<std::shared_ptr<MockResource>> resources;
    for (int i = 0; i < 10; ++i) {
        auto resource = manager.Load<MockResource>("test/pressure_" + std::to_string(i) + ".dat");
        resource->SetSize(1024); // 1KB each
        resources.push_back(resource);
    }

    size_t initialMemory = manager.GetMemoryUsage();
    EXPECT_TRUE(initialMemory > 4096); // Should exceed threshold

    // Manually trigger memory pressure check
    manager.CheckMemoryPressure();

    // Clear some references to allow cleanup
    resources.erase(resources.begin(), resources.begin() + 5);

    // Trigger LRU cleanup
    manager.UnloadLeastRecentlyUsed(2048); // Try to free 2KB

    size_t finalMemory = manager.GetMemoryUsage();

    // Memory should be reduced (though exact amount depends on cleanup)
    EXPECT_TRUE(finalMemory <= initialMemory);

    TestOutput::PrintTestPass("memory pressure handling");
    return true;

}

bool TestLRUCleanupBehavior() {
TestOutput::PrintTestStart("LRU cleanup behavior");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load resources with time gaps to establish access order
    auto resource1 = manager.Load<MockResource>("test/lru1.dat");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto resource2 = manager.Load<MockResource>("test/lru2.dat");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto resource3 = manager.Load<MockResource>("test/lru3.dat");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Access resource1 again to make it more recently used
    auto resource1_again = manager.Load<MockResource>("test/lru1.dat");
    EXPECT_TRUE(resource1.get() == resource1_again.get());

    size_t initialCount = manager.GetResourceCount();
    EXPECT_EQUAL(initialCount, static_cast<size_t>(3));

    // Clear references to allow cleanup
    resource1.reset();
    resource2.reset();
    resource3.reset();
    resource1_again.reset();

    // Trigger LRU cleanup
    manager.UnloadLeastRecentlyUsed(1024); // Target 1KB reduction

    size_t finalCount = manager.GetResourceCount();

    // Some resources should have been cleaned up
    EXPECT_TRUE(finalCount < initialCount);

    TestOutput::PrintTestPass("LRU cleanup behavior");
    return true;

}

````

### Memory Leak Detection

Test for memory leaks in resource operations:

```cpp
bool TestMemoryLeakPrevention() {
    TestOutput::PrintTestStart("memory leak prevention");

    // Test resource creation/destruction cycles
    bool memoryTest = MemoryTest::TestForMemoryLeaks(
        "resource lifecycle",
        []() {
            ResourceManager manager;
            manager.Initialize();

            // Create and destroy many resources
            for (int i = 0; i < 100; ++i) {
                auto resource = manager.Load<MockResource>("test/leak_" + std::to_string(i) + ".dat");
                // Resource automatically destroyed when shared_ptr goes out of scope
            }

            // Explicit cleanup
            manager.UnloadAll();
        }
    );

    EXPECT_TRUE(memoryTest);

    TestOutput::PrintTestPass("memory leak prevention");
    return true;
}
````

## Performance Testing Guidelines

### Resource Loading Performance

Benchmark resource loading operations:

```cpp
bool TestResourceLoadingPerformance() {
    TestOutput::PrintTestStart("resource loading performance");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test single resource loading performance
    bool singleLoadTest = PerformanceTest::ValidatePerformance(
        "single resource loading",
        [&]() {
            auto resource = manager.Load<MockResource>("test/perf_single.dat");
        },
        1.0, // 1ms threshold per operation
        1000 // 1000 iterations
    );

    EXPECT_TRUE(singleLoadTest);

    // Test batch resource loading performance
    bool batchLoadTest = PerformanceTest::ValidatePerformance(
        "batch resource loading",
        [&]() {
            std::vector<std::shared_ptr<MockResource>> resources;
            for (int i = 0; i < 10; ++i) {
                resources.push_back(manager.Load<MockResource>("test/batch_" + std::to_string(i) + ".dat"));
            }
        },
        5.0, // 5ms threshold for 10 resources
        100  // 100 iterations
    );

    EXPECT_TRUE(batchLoadTest);

    TestOutput::PrintTestPass("resource loading performance");
    return true;
}

bool TestCacheHitPerformance() {
    TestOutput::PrintTestStart("cache hit performance");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    const std::string resourcePath = "test/cache_perf.dat";

    // Pre-load resource to ensure it's cached
    auto preloadResource = manager.Load<MockResource>(resourcePath);

    // Test cache hit performance
    bool cacheHitTest = PerformanceTest::ValidatePerformance(
        "cache hit loading",
        [&]() {
            auto resource = manager.Load<MockResource>(resourcePath);
        },
        0.1, // 0.1ms threshold (should be very fast)
        10000 // 10000 iterations
    );

    EXPECT_TRUE(cacheHitTest);

    TestOutput::PrintTestPass("cache hit performance");
    return true;
}

bool TestMemoryOperationPerformance() {
    TestOutput::PrintTestStart("memory operation performance");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load many resources for testing
    std::vector<std::shared_ptr<MockResource>> resources;
    for (int i = 0; i < 1000; ++i) {
        resources.push_back(manager.Load<MockResource>("test/mem_perf_" + std::to_string(i) + ".dat"));
    }

    // Test memory usage calculation performance
    bool memoryStatsTest = PerformanceTest::ValidatePerformance(
        "memory usage calculation",
        [&]() {
            size_t usage = manager.GetMemoryUsage();
            (void)usage; // Suppress unused variable warning
        },
        1.0, // 1ms threshold
        1000 // 1000 iterations
    );

    EXPECT_TRUE(memoryStatsTest);

    // Test resource statistics performance
    bool resourceStatsTest = PerformanceTest::ValidatePerformance(
        "resource statistics calculation",
        [&]() {
            ResourceStats stats = manager.GetResourceStats();
            (void)stats; // Suppress unused variable warning
        },
        2.0, // 2ms threshold (more complex operation)
        500  // 500 iterations
    );

    EXPECT_TRUE(resourceStatsTest);

    TestOutput::PrintTestPass("memory operation performance");
    return true;
}
```

### Cleanup Performance

Test performance of cleanup operations:

```cpp
bool TestCleanupPerformance() {
    TestOutput::PrintTestStart("cleanup performance");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Load many resources
    std::vector<std::shared_ptr<MockResource>> resources;
    for (int i = 0; i < 1000; ++i) {
        resources.push_back(manager.Load<MockResource>("test/cleanup_" + std::to_string(i) + ".dat"));
    }

    // Clear references
    resources.clear();

    // Test unused resource cleanup performance
    TestTimer timer;
    manager.UnloadUnused();
    double cleanupTime = timer.ElapsedMs();

    TestOutput::PrintTiming("unused resource cleanup", cleanupTime);

    // Should complete within reasonable time
    EXPECT_TRUE(cleanupTime < 100.0); // 100ms threshold

    TestOutput::PrintTestPass("cleanup performance");
    return true;
}
```

## Integration Testing Patterns

### ResourceManager with Real Components

Test ResourceManager integration with actual engine components:

```cpp
bool TestResourceManagerIntegration() {
    TestOutput::PrintTestStart("resource manager integration");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test with context-aware resources (if OpenGL available)
    if (OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Testing with OpenGL context available");

        // Test real texture loading
        auto texture = manager.Load<Texture>("textures/test.png");
        EXPECT_NOT_NULL(texture);
        EXPECT_TRUE(texture->GetMemoryUsage() > 0);

        // Test real mesh loading
        auto mesh = manager.Load<Mesh>("meshes/cube.obj");
        EXPECT_NOT_NULL(mesh);
        EXPECT_TRUE(mesh->GetMemoryUsage() > 0);

    } else {
        TestOutput::PrintInfo("Testing without OpenGL context (using fallbacks)");

        // Resources should still be created (with default/fallback content)
        auto texture = manager.Load<Texture>("textures/test.png");
        EXPECT_NOT_NULL(texture);

        auto mesh = manager.Load<Mesh>("meshes/cube.obj");
        EXPECT_NOT_NULL(mesh);
    }

    // Test statistics work with real resources
    ResourceStats stats = manager.GetResourceStats();
    EXPECT_TRUE(stats.totalResources >= 2);
    EXPECT_TRUE(stats.totalMemoryUsage > 0);

    TestOutput::PrintTestPass("resource manager integration");
    return true;
}

bool TestResourceManagerWithEngine() {
    TestOutput::PrintTestStart("resource manager with engine");

    // Test ResourceManager as part of full engine initialization
    Engine engine;
    EXPECT_TRUE(engine.Initialize());

    // Get ResourceManager from engine
    ResourceManager* manager = engine.GetResourceManager();
    EXPECT_NOT_NULL(manager);

    // Test resource loading through engine
    auto resource = manager->Load<MockResource>("test/engine_integration.dat");
    EXPECT_NOT_NULL(resource);

    // Test engine shutdown cleans up resources
    engine.Shutdown();

    TestOutput::PrintTestPass("resource manager with engine");
    return true;
}
```

### Multi-threaded Resource Access

Test ResourceManager thread safety:

```cpp
bool TestThreadSafeResourceAccess() {
    TestOutput::PrintTestStart("thread-safe resource access");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    const int numThreads = 4;
    const int resourcesPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    // Launch multiple threads loading resources
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            try {
                for (int i = 0; i < resourcesPerThread; ++i) {
                    std::string path = "test/thread_" + std::to_string(t) + "_" + std::to_string(i) + ".dat";
                    auto resource = manager.Load<MockResource>(path);
                    if (resource != nullptr) {
                        successCount++;
                    }
                }
            } catch (...) {
                // Thread-safety failure
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // All resource loads should have succeeded
    EXPECT_EQUAL(successCount.load(), numThreads * resourcesPerThread);

    // Check final resource count
    size_t finalCount = manager.GetResourceCount();
    EXPECT_EQUAL(finalCount, static_cast<size_t>(numThreads * resourcesPerThread));

    TestOutput::PrintTestPass("thread-safe resource access");
    return true;
}
```

## Mock Resource Implementation

### Base Mock Resource Pattern

Standard mock resource implementation for testing:

```cpp
// Base mock resource class
class MockResource : public Resource {
public:
    MockResource(const std::string& path)
        : Resource(path), m_size(1024), m_loadSuccess(true) {}

    // Memory usage simulation
    size_t GetMemoryUsage() const override {
        return Resource::GetMemoryUsage() + m_size;
    }

    // Configurable properties for testing
    void SetSize(size_t size) { m_size = size; }
    void SetLoadSuccess(bool success) { m_loadSuccess = success; }

    // Simulate loading behavior
    bool LoadFromFile(const std::string& path) {
        if (!m_loadSuccess) {
            return false;
        }

        // Simulate loading time
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        return true;
    }

    // Create default resource when loading fails
    void CreateDefault() {
        m_size = 512; // Smaller default size
    }

private:
    size_t m_size;
    bool m_loadSuccess;
};

// Specialized mock resources
class MockTexture : public MockResource {
public:
    MockTexture(const std::string& path) : MockResource(path), m_width(256), m_height(256) {
        SetSize(m_width * m_height * 4); // RGBA
    }

    void SetDimensions(int width, int height) {
        m_width = width;
        m_height = height;
        SetSize(m_width * m_height * 4);
    }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    int m_width, m_height;
};

class MockMesh : public MockResource {
public:
    MockMesh(const std::string& path) : MockResource(path), m_vertexCount(1000), m_indexCount(3000) {
        SetSize(m_vertexCount * 32 + m_indexCount * 4); // 32 bytes per vertex, 4 per index
    }

    void SetGeometry(size_t vertices, size_t indices) {
        m_vertexCount = vertices;
        m_indexCount = indices;
        SetSize(m_vertexCount * 32 + m_indexCount * 4);
    }

    size_t GetVertexCount() const { return m_vertexCount; }
    size_t GetIndexCount() const { return m_indexCount; }

private:
    size_t m_vertexCount, m_indexCount;
};
```

### Mock Resource Usage Patterns

Examples of using mock resources in tests:

```cpp
bool TestMockResourceBehavior() {
    TestOutput::PrintTestStart("mock resource behavior");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test configurable mock resource
    auto resource = manager.Load<MockResource>("test/configurable.dat");
    EXPECT_NOT_NULL(resource);

    // Test size configuration
    size_t originalSize = resource->GetMemoryUsage();
    resource->SetSize(2048);
    size_t newSize = resource->GetMemoryUsage();
    EXPECT_TRUE(newSize > originalSize);

    // Test load failure simulation
    auto failingResource = std::make_shared<MockResource>("test/failing.dat");
    failingResource->SetLoadSuccess(false);

    // Should fall back to default
    EXPECT_FALSE(failingResource->LoadFromFile("test/failing.dat"));
    failingResource->CreateDefault();
    EXPECT_TRUE(failingResource->GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("mock resource behavior");
    return true;
}

bool TestSpecializedMockResources() {
    TestOutput::PrintTestStart("specialized mock resources");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test mock texture
    auto texture = manager.Load<MockTexture>("textures/mock.png");
    EXPECT_NOT_NULL(texture);
    EXPECT_EQUAL(texture->GetWidth(), 256);
    EXPECT_EQUAL(texture->GetHeight(), 256);

    // Test texture resizing
    texture->SetDimensions(512, 512);
    EXPECT_EQUAL(texture->GetWidth(), 512);
    EXPECT_EQUAL(texture->GetHeight(), 512);

    // Test mock mesh
    auto mesh = manager.Load<MockMesh>("meshes/mock.obj");
    EXPECT_NOT_NULL(mesh);
    EXPECT_EQUAL(mesh->GetVertexCount(), static_cast<size_t>(1000));
    EXPECT_EQUAL(mesh->GetIndexCount(), static_cast<size_t>(3000));

    // Test mesh geometry changes
    mesh->SetGeometry(2000, 6000);
    EXPECT_EQUAL(mesh->GetVertexCount(), static_cast<size_t>(2000));
    EXPECT_EQUAL(mesh->GetIndexCount(), static_cast<size_t>(6000));

    TestOutput::PrintTestPass("specialized mock resources");
    return true;
}
```

## Error Handling and Edge Cases

### Resource Loading Failures

Test error conditions and recovery:

```cpp
bool TestResourceLoadingFailures() {
    TestOutput::PrintTestStart("resource loading failures");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test loading non-existent resource
    auto missingResource = manager.Load<MockResource>("missing/nonexistent.dat");

    // Should still create resource (with default content)
    EXPECT_NOT_NULL(missingResource);
    EXPECT_TRUE(missingResource->GetMemoryUsage() > 0);

    // Test loading with invalid path
    auto invalidResource = manager.Load<MockResource>("");
    EXPECT_NOT_NULL(invalidResource);

    // Test resource statistics include failed loads
    ResourceStats stats = manager.GetResourceStats();
    EXPECT_TRUE(stats.totalResources >= 2);

    TestOutput::PrintTestPass("resource loading failures");
    return true;
}

bool TestMemoryExhaustionScenarios() {
    TestOutput::PrintTestStart("memory exhaustion scenarios");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Set very low memory threshold
    manager.SetMemoryPressureThreshold(1024); // 1KB

    // Try to load resources that exceed memory
    std::vector<std::shared_ptr<MockResource>> resources;

    try {
        for (int i = 0; i < 100; ++i) {
            auto resource = manager.Load<MockResource>("test/exhaust_" + std::to_string(i) + ".dat");
            resource->SetSize(1024); // 1KB each
            resources.push_back(resource);

            // Memory pressure should trigger cleanup
            if (i % 10 == 0) {
                manager.CheckMemoryPressure();
            }
        }

        // Should handle memory pressure gracefully
        EXPECT_TRUE(manager.GetResourceCount() > 0);

    } catch (const std::exception& e) {
        TestOutput::PrintError("Unexpected exception: " + std::string(e.what()));
        return false;
    }

    TestOutput::PrintTestPass("memory exhaustion scenarios");
    return true;
}

bool TestConcurrentAccessEdgeCases() {
    TestOutput::PrintTestStart("concurrent access edge cases");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    const std::string sharedPath = "test/concurrent.dat";
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<MockResource>> results(10);

    // Multiple threads loading same resource simultaneously
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&, i]() {
            results[i] = manager.Load<MockResource>(sharedPath);
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // All should get the same resource instance
    for (int i = 1; i < 10; ++i) {
        EXPECT_TRUE(results[0].get() == results[i].get());
    }

    // Should only have one resource in manager
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));

    TestOutput::PrintTestPass("concurrent access edge cases");
    return true;
}
```

### Resource Cleanup Edge Cases

Test cleanup behavior in edge cases:

```cpp
bool TestCleanupEdgeCases() {
    TestOutput::PrintTestStart("cleanup edge cases");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test cleanup with circular references (shouldn't happen with shared_ptr, but test anyway)
    auto resource1 = manager.Load<MockResource>("test/cleanup1.dat");
    auto resource2 = manager.Load<MockResource>("test/cleanup2.dat");

    // Clear references
    resource1.reset();
    resource2.reset();

    // Test multiple cleanup calls
    manager.UnloadUnused();
    manager.UnloadUnused(); // Should be safe to call multiple times

    // Test cleanup with active resources
    auto activeResource = manager.Load<MockResource>("test/active.dat");
    manager.UnloadUnused(); // Should not affect active resource

    EXPECT_NOT_NULL(activeResource);
    EXPECT_TRUE(manager.GetResourceCount() >= 1);

    // Test shutdown cleanup
    manager.UnloadAll();
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(0));

    // activeResource should still be valid (held by shared_ptr)
    EXPECT_NOT_NULL(activeResource);

    TestOutput::PrintTestPass("cleanup edge cases");
    return true;
}
```

## Best Practices Summary

### Testing Checklist

When writing resource tests, ensure you:

1. **Use Mock Resources**: Avoid OpenGL dependencies in unit tests
2. **Test Cache Behavior**: Verify cache hits, misses, and invalidation
3. **Validate Memory Tracking**: Check memory usage calculations and cleanup
4. **Test Performance**: Benchmark critical operations with thresholds
5. **Handle Edge Cases**: Test error conditions and recovery scenarios
6. **Verify Thread Safety**: Test concurrent access patterns
7. **Check Integration**: Test with real engine components when possible
8. **Follow Output Standards**: Use consistent TestOutput formatting

### Common Patterns

```cpp
// Standard test structure for resource testing
bool TestResourceFeature() {
    TestOutput::PrintTestStart("resource feature");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    // Test setup
    auto resource = manager.Load<MockResource>("test/feature.dat");
    EXPECT_NOT_NULL(resource);

    // Test execution
    // ... perform operations ...

    // Test validation
    // ... check results ...

    TestOutput::PrintTestPass("resource feature");
    return true;
}

// Performance test pattern
bool TestResourcePerformance() {
    TestOutput::PrintTestStart("resource performance");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    bool performanceTest = PerformanceTest::ValidatePerformance(
        "operation name",
        [&]() {
            // Operation to benchmark
        },
        thresholdMs,
        iterations
    );

    EXPECT_TRUE(performanceTest);

    TestOutput::PrintTestPass("resource performance");
    return true;
}

// Integration test pattern
bool TestResourceIntegration() {
    TestOutput::PrintTestStart("resource integration");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("resource integration");
        return true;
    }

    // Full integration test with real resources
    // ...

    TestOutput::PrintTestPass("resource integration");
    return true;
}
```

### Key Principles

1. **Isolation**: Each test should be independent and not affect others
2. **Repeatability**: Tests should produce consistent results across runs
3. **Clarity**: Test names and structure should clearly indicate what's being tested
4. **Coverage**: Test normal cases, edge cases, and error conditions
5. **Performance**: Include performance validation for critical operations
6. **Maintainability**: Use helper functions and patterns to reduce code duplication

### Example Complete Test File

```cpp
#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Mock resource implementation
class MockResource : public Resource {
public:
    MockResource(const std::string& path) : Resource(path), m_size(1024) {}
    size_t GetMemoryUsage() const override { return Resource::GetMemoryUsage() + m_size; }
    void SetSize(size_t size) { m_size = size; }
private:
    size_t m_size;
};

bool TestResourceBasics() {
    TestOutput::PrintTestStart("resource basics");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    auto resource = manager.Load<MockResource>("test.dat");
    EXPECT_NOT_NULL(resource);
    EXPECT_TRUE(resource->GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("resource basics");
    return true;
}

bool TestResourceCaching() {
    TestOutput::PrintTestStart("resource caching");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    auto resource1 = manager.Load<MockResource>("cache_test.dat");
    auto resource2 = manager.Load<MockResource>("cache_test.dat");

    EXPECT_TRUE(resource1.get() == resource2.get());
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));

    TestOutput::PrintTestPass("resource caching");
    return true;
}

bool TestMemoryManagement() {
    TestOutput::PrintTestStart("memory management");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    auto resource = manager.Load<MockResource>("memory_test.dat");
    resource->SetSize(2048);

    EXPECT_TRUE(manager.GetMemoryUsage() >= 2048);

    resource.reset();
    manager.UnloadUnused();

    EXPECT_EQUAL(manager.GetMemoryUsage(), static_cast<size_t>(0));

    TestOutput::PrintTestPass("memory management");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Testing");
    Logger::GetInstance().Initialize();

    TestSuite suite("Resource Tests");
    bool allPassed = true;

    allPassed &= suite.RunTest("Resource Basics", TestResourceBasics);
    allPassed &= suite.RunTest("Resource Caching", TestResourceCaching);
    allPassed &= suite.RunTest("Memory Management", TestMemoryManagement);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);

    return allPassed ? 0 : 1;
}
```

This guide provides comprehensive patterns for testing all aspects of resource management in Game Engine Kiro, ensuring robust and reliable resource handling across all scenarios.
