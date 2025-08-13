# Game Engine Kiro - Complete Testing Guide

## Overview

Game Engine Kiro uses a lightweight, framework-independent testing system designed for simplicity, professional output formatting, and seamless integration with the engine's architecture. This comprehensive guide covers everything you need to know about testing in the engine.

## Table of Contents

1. [Testing Philosophy](#testing-philosophy)
2. [Test Types and Structure](#test-types-and-structure)
3. [Writing Tests](#writing-tests)
4. [Output Formatting Standards](#output-formatting-standards)
5. [OpenGL Context Handling](#opengl-context-handling)
6. [Resource Testing Patterns](#resource-testing-patterns)
7. [Performance Testing](#performance-testing)
8. [Running Tests](#running-tests)
9. [Best Practices](#best-practices)
10. [Troubleshooting](#troubleshooting)

## Testing Philosophy

### Core Principles

- **Framework-Independent**: No external dependencies like GoogleTest or Catch2
- **Professional Output**: Consistent, text-based formatting compatible across all platforms
- **Simple Patterns**: Predictable test structure that's easy to learn and maintain
- **Build Integration**: Seamless integration with existing CMake build system
- **Context-Aware**: Handles OpenGL context limitations gracefully for headless testing
- **Resource-Focused**: Comprehensive patterns for testing resource management systems

### Design Goals

- **Simplicity**: Easy to write, read, and maintain
- **Reliability**: Consistent behavior across different environments
- **Performance**: Fast compilation and execution
- **Debugging**: Clear output for troubleshooting issues

## Test Types and Structure

### Directory Organization

```
tests/
├── unit/                    # Unit tests for individual components
│   ├── test_math.cpp       # Math operations, vectors, matrices
│   ├── test_logger.cpp     # Logging system functionality
│   └── test_*.cpp          # Other component tests
├── integration/             # System interaction testing
│   ├── test_physics_integration.cpp
│   ├── test_audio_integration.cpp
│   └── test_*.cpp          # System integration tests
└── TestUtils.h             # Shared testing utilities
```

### Test Categories

| Test Type             | Purpose                      | Location                | OpenGL Context  |
| --------------------- | ---------------------------- | ----------------------- | --------------- |
| **Unit Tests**        | Individual component testing | `tests/unit/`           | Not required    |
| **Integration Tests** | System interaction testing   | `tests/integration/`    | May be required |
| **Performance Tests** | Benchmarking and validation  | Embedded in other tests | Context-aware   |

## Writing Tests

### Basic Test Template

```cpp
#include "TestUtils.h"
#include "Path/To/ComponentHeader.h"
// Add other necessary includes

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test description
 * Requirements: X.X, Y.Y (requirement description)
 */
bool TestFunctionName() {
    TestOutput::PrintTestStart("test description");

    // Test implementation here
    // Use EXPECT_* macros for assertions:
    // EXPECT_TRUE(condition)
    // EXPECT_FALSE(condition)
    // EXPECT_EQUAL(expected, actual)
    // EXPECT_NOT_EQUAL(expected, actual)
    // EXPECT_NEARLY_EQUAL(expected, actual)
    // EXPECT_VEC3_NEARLY_EQUAL(expected, actual)

    TestOutput::PrintTestPass("test description");
    return true;
}

int main() {
    TestOutput::PrintHeader("ComponentName");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ComponentName Tests");

        // Run all tests
        allPassed &= suite.RunTest("Test Description", TestFunctionName);
        // Add more tests here

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
```

### Assertion Macros

```cpp
// Boolean assertions
EXPECT_TRUE(condition)                    // Boolean true
EXPECT_FALSE(condition)                   // Boolean false

// Equality assertions
EXPECT_EQUAL(expected, actual)            // Exact equality
EXPECT_NOT_EQUAL(expected, actual)        // Inequality

// Floating-point assertions
EXPECT_NEARLY_EQUAL(expected, actual)     // Float comparison with epsilon

// Vector assertions
EXPECT_VEC3_NEARLY_EQUAL(expected, actual) // Vector3 comparison
EXPECT_VEC4_NEARLY_EQUAL(expected, actual) // Vector4 comparison
EXPECT_QUAT_NEARLY_EQUAL(expected, actual) // Quaternion comparison
```

## Output Formatting Standards

### Required Methods

All tests MUST use the `TestOutput` class methods exclusively. Direct `std::cout` usage is prohibited.

```cpp
// Test lifecycle
TestOutput::PrintHeader("ComponentName");           // Suite header
TestOutput::PrintTestStart("test description");     // Individual test start
TestOutput::PrintTestPass("test description");      // Test success
TestOutput::PrintTestFail("test description");      // Test failure
TestOutput::PrintFooter(allPassed);                 // Suite footer

// Information and diagnostics
TestOutput::PrintInfo("informational message");     // General information
TestOutput::PrintWarning("warning message");        // Warning conditions
TestOutput::PrintError("error message");            // Error conditions

// Performance reporting
TestOutput::PrintTiming("operation", timeMs, iterations);
```

### Naming Conventions

- **Test Names**: lowercase with spaces (e.g., "vector operations")
- **Consistency**: Same string for `PrintTestStart()` and `PrintTestPass()`
- **Descriptive**: Clear but concise descriptions

```cpp
// ✅ CORRECT
TestOutput::PrintTestStart("quaternion normalization");
// ... test logic ...
TestOutput::PrintTestPass("quaternion normalization");

// ❌ INCORRECT - Different strings
TestOutput::PrintTestStart("quaternion normalization");
TestOutput::PrintTestPass("quaternion normalize");
```

## OpenGL Context Handling

Many tests run without an active OpenGL context, which causes crashes when creating textures, meshes, or making OpenGL function calls. The engine provides several strategies to handle this:

### Context Detection

```cpp
bool TestGraphicsFeature() {
    TestOutput::PrintTestStart("graphics feature");

    // Always check context before OpenGL operations
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("graphics feature");
        return true;
    }

    // Safe to use OpenGL here
    // ... OpenGL operations ...

    TestOutput::PrintTestPass("graphics feature");
    return true;
}
```

### Mock Resources

For testing logic without OpenGL dependencies:

```cpp
bool TestResourceLogic() {
    TestOutput::PrintTestStart("resource logic");

    // Use mock resources for logic testing
    auto mockTexture = std::make_shared<MockTexture>("test.png");
    EXPECT_TRUE(mockTexture->IsValid());
    EXPECT_EQUAL(mockTexture->GetWidth(), 256);
    EXPECT_EQUAL(mockTexture->GetHeight(), 256);

    TestOutput::PrintTestPass("resource logic");
    return true;
}
```

### Graceful Degradation

```cpp
bool TestWithFallback() {
    TestOutput::PrintTestStart("feature with fallback");

    if (OpenGLContext::HasActiveContext()) {
        // Test with real OpenGL resources
        auto texture = CreateTexture("test.png");
        EXPECT_TRUE(texture != nullptr);
    } else {
        // Test logic without OpenGL
        TestOutput::PrintInfo("Testing logic only (no OpenGL context)");
        auto mockTexture = CreateMockTexture("test.png");
        EXPECT_TRUE(mockTexture->IsValid());
    }

    TestOutput::PrintTestPass("feature with fallback");
    return true;
}
```

## Resource Testing Patterns

### Memory Management Testing

```cpp
bool TestResourceMemoryManagement() {
    TestOutput::PrintTestStart("resource memory management");

    size_t initialMemory = GetMemoryUsage();

    {
        // Create resources in scope
        auto resource1 = LoadResource("test1.dat");
        auto resource2 = LoadResource("test2.dat");

        EXPECT_TRUE(GetMemoryUsage() > initialMemory);
    } // Resources should be cleaned up here

    // Force cleanup and check memory
    ForceGarbageCollection();
    EXPECT_NEARLY_EQUAL(GetMemoryUsage(), initialMemory);

    TestOutput::PrintTestPass("resource memory management");
    return true;
}
```

### Caching and Reuse Testing

```cpp
bool TestResourceCaching() {
    TestOutput::PrintTestStart("resource caching");

    // Load same resource twice
    auto resource1 = LoadResource("test.dat");
    auto resource2 = LoadResource("test.dat");

    // Should return same instance (cached)
    EXPECT_EQUAL(resource1.get(), resource2.get());
    EXPECT_EQUAL(GetResourceCacheSize(), 1);

    TestOutput::PrintTestPass("resource caching");
    return true;
}
```

## Performance Testing

### Timing Measurements

```cpp
bool TestPerformance() {
    TestOutput::PrintTestStart("performance measurement");

    const int iterations = 1000;
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // Operation to benchmark
        Math::Vec3 result = Math::Vec3(1.0f) + Math::Vec3(2.0f);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    float timeMs = duration.count() / 1000.0f;

    TestOutput::PrintTiming("vector addition", timeMs, iterations);

    // Verify performance threshold
    float avgTimePerOp = timeMs / iterations;
    EXPECT_TRUE(avgTimePerOp < 0.001f); // Less than 1 microsecond per operation

    TestOutput::PrintTestPass("performance measurement");
    return true;
}
```

## Running Tests

### Build and Execute

```bash
# Build all tests
.\scripts\build_unified.bat --tests

# Run all tests
.\scripts\run_tests.bat

# Run individual test
.\build\Release\ComponentNameTest.exe
```

### Individual Test Development (Recommended for Specs)

For faster development and debugging, compile and test one component at a time:

```bash
# Compile only a specific test (much faster)
.\scripts\build_unified.bat --tests [TestName]

# Example: Compile only AnimationStateMachine test
.\scripts\build_unified.bat --tests AnimationstatemachineTest

# Run the specific test
.\build\Release\AnimationstatemachineTest.exe

# After all individual tests work, run full suite
.\scripts\run_tests.bat
```

**Benefits:**

- **Faster compilation**: Seconds instead of minutes
- **Focused debugging**: Isolate issues to specific components
- **Quicker iteration**: Immediate feedback on changes
- **Better workflow**: Complete one feature at a time

**Test Name Conversion:**

- `test_animation_state_machine.cpp` → `AnimationstatemachineTest`
- `test_physics_engine.cpp` → `PhysicsengineTest`
- `test_resource_manager.cpp` → `ResourcemanagerTest`

### Test Discovery

Tests are automatically discovered by CMake from:

- `tests/unit/test_*.cpp` → Unit tests
- `tests/integration/test_*.cpp` → Integration tests

### Expected Output

```
=== ComponentName Tests ===
[START] vector operations
[PASS]  vector operations
[START] matrix multiplication
[PASS]  matrix multiplication
=== Test Summary ===
Tests Run: 2
Passed: 2
Failed: 0
Success Rate: 100%
=== ComponentName Tests Complete ===
```

## Best Practices

### Do's

- ✅ Use TestOutput methods exclusively
- ✅ Follow naming conventions consistently
- ✅ Handle OpenGL context limitations
- ✅ Test both success and failure cases
- ✅ Include performance validation where appropriate
- ✅ Use descriptive test names
- ✅ Clean up resources properly

### Don'ts

- ❌ Use `std::cout`, `printf`, or direct console output
- ❌ Create tests that require specific hardware
- ❌ Ignore OpenGL context availability
- ❌ Create tests with external dependencies
- ❌ Use different strings for start/pass methods
- ❌ Create overly complex test logic

### Error Handling

```cpp
bool TestWithErrorHandling() {
    TestOutput::PrintTestStart("error handling");

    try {
        // Test operation that might throw
        auto result = RiskyOperation();
        EXPECT_TRUE(result.IsValid());
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception caught: " + std::string(e.what()));
        TestOutput::PrintTestFail("error handling");
        return false;
    }

    TestOutput::PrintTestPass("error handling");
    return true;
}
```

## Troubleshooting

### Common Issues

#### Build Failures

```bash
# Clean and rebuild
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build_unified.bat --tests
```

#### OpenGL Context Errors

- Check if test properly detects context availability
- Use mock resources for logic-only testing
- Ensure graceful degradation when context is missing

#### Memory Leaks

- Verify proper resource cleanup in destructors
- Use RAII patterns for automatic cleanup
- Check that shared_ptr references are released

#### Inconsistent Output

- Verify all tests use TestOutput methods
- Check that test names match between start/pass calls
- Ensure proper exception handling

### Debugging Tests

```cpp
// Add debug information
TestOutput::PrintInfo("Debug: variable value = " + std::to_string(value));

// Use conditional compilation for debug builds
#ifdef _DEBUG
    TestOutput::PrintInfo("Debug build - additional validation enabled");
#endif
```

## Integration with Build System

### CMake Integration

Tests are automatically discovered and built by CMake. No manual configuration required for new tests that follow naming conventions.

### Continuous Integration

All tests run automatically as part of the build process:

```bash
# This command builds AND runs tests
.\scripts\build_unified.bat --tests
```

### Test Results

- **Exit Code 0**: All tests passed
- **Exit Code 1**: One or more tests failed
- **Detailed Output**: Individual test results and summary statistics

---

This complete guide provides everything needed to write, run, and maintain tests in Game Engine Kiro. For specific implementation examples, refer to existing tests in the `tests/` directory.
