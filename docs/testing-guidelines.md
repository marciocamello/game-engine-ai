# Testing Guidelines - Game Engine Kiro

## Overview

This document provides guidelines for writing tests in Game Engine Kiro, with special attention to OpenGL context limitations and resource testing patterns. This guide works in conjunction with other testing documentation to provide comprehensive testing guidance.

### Related Documentation

**Essential Reading:**

- **[Testing Guide](testing-guide.md)**: Comprehensive testing instructions and examples
- **[Testing Standards](testing-standards.md)**: Coding standards and conventions for test code
- **[Test Output Formatting](testing-output-formatting.md)**: Complete formatting standards

**Context-Aware Testing:**

- **[OpenGL Context Limitations](testing-opengl-limitations.md)**: Detailed guide for handling OpenGL context issues
- **[Resource Testing Patterns](testing-resource-patterns.md)**: Best practices for testing resource management
- **[Mock Resource Implementation](testing-mock-resources.md)**: Patterns for creating and using mock resources

**Quality Assurance:**

- **[Test Output Consistency](testing-output-consistency-guide.md)**: Consistency guidelines across test types
- **[Code Examples Validation](testing-code-examples-validation.md)**: Keeping documentation examples current
- **[API Reference](api-reference.md)**: Complete API documentation with usage examples

**See Also:**

- **[Testing Strategy](testing-strategy.md)**: Overall testing methodology
- **[Testing Migration](testing-migration.md)**: Updating existing tests

## Test Categories

### Unit Tests (`tests/unit/`)

- Test individual components in isolation
- Should not depend on OpenGL context
- Use mock objects when necessary
- Focus on logic and data validation
- Follow patterns in [Mock Resource Implementation Guide](testing-mock-resources.md)

### Integration Tests (`tests/integration/`)

- Test system interactions
- May require OpenGL context for graphics tests
- Should handle missing context gracefully using [OpenGL Context Limitations](testing-opengl-limitations.md) patterns
- Use real assets when available, fallback to mocks
- Apply [Resource Testing Patterns](testing-resource-patterns.md) for resource management testing

### Performance Tests (`tests/performance/`)

- Benchmark critical operations and validate performance thresholds
- Use consistent timing patterns from [Test Output Formatting](testing-output-formatting.md)
- Handle context limitations gracefully
- Focus on realistic performance scenarios

## OpenGL Context Limitations

Many tests run without an active OpenGL context (no window created), which causes crashes when creating textures, meshes, or making any OpenGL function calls. Game Engine Kiro provides comprehensive solutions for this challenge.

### Quick Reference

For immediate solutions, use these patterns:

```cpp
// Always check context before OpenGL operations
if (!OpenGLContext::HasActiveContext()) {
    TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
    TestOutput::PrintTestPass("graphics feature");
    return true;
}

// Use mock resources for logic testing
auto mockResource = std::make_shared<MockResource>("test.dat");
EXPECT_NOT_NULL(mockResource);
EXPECT_TRUE(mockResource->GetMemoryUsage() > 0);
```

### Comprehensive Solutions

The engine provides multiple strategies for handling OpenGL context limitations:

1. **Context Detection**: Use `OpenGLContext::HasActiveContext()` to check availability
2. **Mock Resources**: Lightweight implementations that don't require OpenGL
3. **Context-Aware Classes**: Resource classes that handle missing context gracefully
4. **Separate Testing**: Test data processing separately from GPU operations
5. **Fallback Behavior**: Graceful degradation when context is unavailable

For complete implementation details, examples, and troubleshooting, see [OpenGL Context Limitations in Testing](testing-opengl-limitations.md).

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

Game Engine Kiro provides comprehensive patterns for testing resource management systems, including memory tracking, cache validation, and performance testing.

### Quick Reference Patterns

```cpp
// Pattern 1: Mock Resources for Logic Testing
bool TestResourceCaching() {
    ResourceManager manager;
    auto resource1 = manager.Load<MockResource>("test.dat");
    auto resource2 = manager.Load<MockResource>("test.dat"); // Same path
    EXPECT_TRUE(resource1.get() == resource2.get()); // Should be cached
    return true;
}

// Pattern 2: Context-Aware Integration Testing
bool TestTextureLoading() {
    ResourceManager manager;
    auto texture = manager.Load<Texture>("textures/test.png");
    EXPECT_NOT_NULL(texture);
    EXPECT_TRUE(texture->GetMemoryUsage() > 0);
    return true; // Works with or without OpenGL context
}

// Pattern 3: Memory Management Testing
bool TestMemoryTracking() {
    ResourceManager manager;
    auto resource = manager.Load<MockResource>("test.dat");
    EXPECT_TRUE(manager.GetMemoryUsage() > 0);
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));
    return true;
}
```

### Comprehensive Testing Patterns

The engine supports multiple resource testing approaches:

1. **Mock Resource Testing**: Lightweight resources for logic validation
2. **Cache Behavior Testing**: Verify resource caching and reuse
3. **Memory Management Testing**: Track memory usage and cleanup
4. **Performance Testing**: Benchmark resource operations
5. **Integration Testing**: Test with real engine systems
6. **Error Handling Testing**: Validate failure scenarios

For detailed implementations, examples, and best practices, see [Resource Testing Best Practices Guide](testing-resource-patterns.md).

## Memory Management Testing

Resource memory management testing is critical for ensuring proper resource lifecycle and preventing memory leaks.

### Quick Reference

```cpp
// Test memory usage tracking
bool TestMemoryTracking() {
    ResourceManager manager;
    auto resource = manager.Load<MockResource>("test.dat");
    EXPECT_TRUE(manager.GetMemoryUsage() > 0);
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));
    return true;
}

// Test resource cleanup
bool TestResourceCleanup() {
    ResourceManager manager;
    {
        auto resource = manager.Load<MockResource>("test.dat");
        EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(1));
    } // resource goes out of scope
    manager.UnloadUnused();
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(0));
    return true;
}
```

### Advanced Memory Testing

The ResourceManager provides comprehensive memory management features:

1. **Memory Usage Tracking**: Monitor total memory consumption
2. **Resource Statistics**: Detailed breakdown by type and usage
3. **LRU Cleanup**: Automatic cleanup of least recently used resources
4. **Memory Pressure Handling**: Configurable thresholds and cleanup strategies
5. **Leak Detection**: Validation of proper resource cleanup

For detailed memory testing patterns, performance validation, and troubleshooting, see [Resource Testing Best Practices Guide](testing-resource-patterns.md).

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
