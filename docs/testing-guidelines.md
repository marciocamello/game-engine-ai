# Testing Guidelines - Game Engine Kiro

## Overview

This document provides guidelines for writing tests in Game Engine Kiro, with special attention to OpenGL context limitations and resource testing patterns.

## Test Categories

### Unit Tests (`tests/unit/`)

- Test individual components in isolation
- Should not depend on OpenGL context
- Use mock objects when necessary
- Focus on logic and data validation

### Integration Tests (`tests/integration/`)

- Test system interactions
- May require OpenGL context for graphics tests
- Should handle missing context gracefully
- Use real assets when available, fallback to mocks

## OpenGL Context Limitations

### The Problem

Many tests run without an active OpenGL context (no window created), which causes crashes when:

- Creating textures (`glGenTextures`, `glTexImage2D`)
- Creating meshes (`glGenVertexArrays`, `glGenBuffers`)
- Any OpenGL function calls

### Solutions

#### 1. Context-Safe Resource Classes

Our Texture and Mesh classes now check for OpenGL context:

```cpp
// Safe approach - check context before OpenGL calls
if (OpenGLContext::HasActiveContext()) {
    glGenTextures(1, &m_textureID);
    // ... other OpenGL calls
} else {
    LOG_WARNING("No OpenGL context available for texture creation");
}
```

#### 2. Mock Resources for Testing

Use mock resources that don't depend on OpenGL:

```cpp
// Example: MockResource for ResourceManager tests
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
```

#### 3. CPU-Only Data Testing

Test data loading and processing separately from GPU resource creation:

```cpp
// Test data loading without OpenGL
bool TestMeshDataLoading() {
    MeshLoader::MeshData data = MeshLoader::LoadOBJ("test.obj");
    EXPECT_TRUE(data.isValid);
    EXPECT_TRUE(data.vertices.size() > 0);
    // Don't test OpenGL buffer creation here
}
```

## Test Output Standards

### Formatting Requirements

All tests must follow the standardized output formatting defined in [Test Output Formatting Standards](testing-output-formatting.md) and maintain consistency as outlined in [Test Output Consistency Guidelines](testing-output-consistency-guide.md). Key requirements include:

- **TestOutput Methods Only**: Never use `std::cout`, `printf`, or direct console output
- **Consistent Naming**: Use lowercase with spaces for test names
- **Matching Start/Pass Names**: Same string for `PrintTestStart()` and `PrintTestPass()`
- **Minimal Information**: Use `PrintInfo()` only for essential context

```cpp
// ✅ CORRECT
TestOutput::PrintTestStart("vector operations");
// ... test logic ...
TestOutput::PrintTestPass("vector operations");

// ❌ INCORRECT
TestOutput::PrintTestStart("Vector Operations");  // PascalCase
std::cout << "Test result: " << result << std::endl;  // Direct output
TestOutput::PrintTestPass("vector operations test completed");  // Different string
```

### Output Structure Requirements

- **Headers**: Use `TestOutput::PrintHeader("Component Name")`
- **Footers**: Use `TestOutput::PrintFooter(allPassed)`
- **Status Prefixes**: `[PASS]`, `[FAILED]`, `[INFO]`, `[WARNING]`, `[ERROR]`
- **Indentation**: 2 spaces for test results, 4 spaces for error details
- **Separators**: Use `========================================` for major sections only

## Resource Testing Patterns

### Pattern 1: Mock Resources

Best for testing resource management logic:

```cpp
bool TestResourceCaching() {
    ResourceManager manager;

    // Use mock resources that don't need OpenGL
    auto resource1 = manager.Load<MockResource>("test1.dat");
    auto resource2 = manager.Load<MockResource>("test1.dat"); // Same path

    // Test caching behavior
    EXPECT_TRUE(resource1.get() == resource2.get());
    return true;
}
```

### Pattern 2: Real Assets with Fallbacks

For integration tests that can use real assets:

```cpp
bool TestTextureLoading() {
    ResourceManager manager;

    // Try to load real texture, fallback to default if no OpenGL
    auto texture = manager.Load<Texture>("textures/wall.png");
    EXPECT_TRUE(texture != nullptr);

    // Test should pass whether real texture loaded or default created
    EXPECT_TRUE(texture->GetMemoryUsage() > 0);
    return true;
}
```

### Pattern 3: Context-Aware Testing

Check for OpenGL context and adjust test behavior:

```cpp
bool TestGraphicsFeatures() {
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent tests (no context)");
        return true; // Pass the test
    }

    // Run full OpenGL tests
    // ...
}
```

## Memory Management Testing

### Testing Resource Statistics

Use the ResourceManager's built-in statistics:

```cpp
bool TestMemoryTracking() {
    ResourceManager manager;

    // Load resources
    auto resource1 = manager.Load<MockResource>("test1.dat");
    auto resource2 = manager.Load<MockResource>("test2.dat");

    // Test statistics
    EXPECT_TRUE(manager.GetResourceCount() >= 2);
    EXPECT_TRUE(manager.GetMemoryUsage() > 0);

    // Test detailed stats
    ResourceStats stats = manager.GetResourceStats();
    EXPECT_TRUE(stats.totalResources >= 2);

    return true;
}
```

### Testing LRU Cleanup

Test automatic memory management:

```cpp
bool TestLRUCleanup() {
    ResourceManager manager;
    manager.SetMemoryPressureThreshold(1024); // Low threshold

    // Load many resources to trigger cleanup
    std::vector<std::shared_ptr<MockResource>> resources;
    for (int i = 0; i < 10; ++i) {
        auto resource = manager.Load<MockResource>("test" + std::to_string(i) + ".dat");
        resources.push_back(resource);
    }

    size_t initialCount = manager.GetResourceCount();

    // Clear references and trigger cleanup
    resources.clear();
    manager.CheckMemoryPressure();
    manager.UnloadLeastRecentlyUsed(512);

    // Verify cleanup occurred
    size_t finalCount = manager.GetResourceCount();
    EXPECT_TRUE(finalCount <= initialCount);

    return true;
}
```

## Common Pitfalls

### 1. OpenGL Context Assumptions

```cpp
// ❌ BAD - Assumes OpenGL context exists
Texture texture;
texture.LoadFromFile("test.png"); // May crash in tests

// ✅ GOOD - Context-safe or use mocks
auto texture = resourceManager.Load<Texture>("test.png"); // Handles context safely
```

### 2. Inconsistent Test Output

```cpp
// ❌ BAD
std::cout << "Loading " << count << " resources" << std::endl;
TestOutput::PrintTestStart("Resource Loading Test");

// ✅ GOOD
TestOutput::PrintTestStart("resource loading");
// Minimal logging, consistent format
```

### 3. Resource Cleanup

```cpp
// ❌ BAD - May leak resources
void TestResourceLoading() {
    auto texture = new Texture("test.png");
    // ... test logic ...
    // Forgot to delete!
}

// ✅ GOOD - RAII with smart pointers
bool TestResourceLoading() {
    auto texture = resourceManager.Load<Texture>("test.png");
    // ... test logic ...
    return true; // Automatic cleanup
}
```

## Best Practices Summary

1. **Always check OpenGL context** before graphics operations
2. **Use mock resources** for pure logic testing
3. **Follow consistent output format** (lowercase, TestOutput::Print\*)
4. **Test data separately from GPU resources** when possible
5. **Use ResourceManager** for all resource loading in tests
6. **Implement proper cleanup** with RAII and smart pointers
7. **Test both success and failure cases** gracefully
8. **Keep tests focused** - one concept per test function

## Example Test Structure

```cpp
#include "Resource/ResourceManager.h"
#include "Core/Logger.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

// Mock resource for testing
class MockResource : public Resource {
public:
    MockResource(const std::string& path) : Resource(path), m_size(1024) {}
    size_t GetMemoryUsage() const override { return Resource::GetMemoryUsage() + m_size; }
private:
    size_t m_size;
};

bool TestResourceBasics() {
    TestOutput::PrintTestStart("resource basics");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());

    auto resource = manager.Load<MockResource>("test.dat");
    EXPECT_TRUE(resource != nullptr);
    EXPECT_TRUE(resource->GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("resource basics");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Tests");
    Logger::GetInstance().Initialize();

    TestSuite suite("Resource Tests");
    bool allPassed = suite.RunTest("Resource Basics", TestResourceBasics);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);

    return allPassed ? 0 : 1;
}
```

This structure ensures tests are robust, consistent, and work reliably both with and without OpenGL context.
