# Game Engine Kiro - Comprehensive Testing Guide

## Table of Contents

1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Test Creation](#test-creation)
4. [Test Execution](#test-execution)
5. [Best Practices](#best-practices)
6. [Common Patterns](#common-patterns)
7. [Troubleshooting](#troubleshooting)
8. [Adding New Tests](#adding-new-tests)
9. [Framework Integration](#framework-integration)
10. [Advanced Topics](#advanced-topics)

## Overview

Game Engine Kiro uses a lightweight, framework-independent testing system designed for simplicity, cross-platform compatibility, and professional output formatting. This guide provides comprehensive instructions for creating, executing, and maintaining tests within the engine.

### Testing Philosophy

- **Framework-Independent**: No external dependencies like GoogleTest or Catch2
- **Professional Output**: Consistent, text-based formatting compatible across all platforms
- **Simple Patterns**: Predictable test structure that's easy to learn and maintain
- **Build Integration**: Seamless integration with existing CMake build system
- **Cross-Platform**: Compatible with Windows, Linux, and macOS development environments

### Test Types

| Test Type             | Location             | Purpose                                 | Examples                                    |
| --------------------- | -------------------- | --------------------------------------- | ------------------------------------------- |
| **Unit Tests**        | `tests/unit/`        | Individual component testing            | Math operations, Logger functionality       |
| **Integration Tests** | `tests/integration/` | System interaction testing              | Physics integration, Component interactions |
| **Performance Tests** | `tests/performance/` | Benchmarking and performance validation | Math operation speed, Memory usage          |

## Getting Started

### Prerequisites

- CMake 3.16+
- C++20 compatible compiler (MSVC 2019+, GCC 10+, or Clang 10+)
- Game Engine Kiro development environment set up

### Quick Start

1. **Build the project** to ensure all tests are compiled:

   ```cmd
   .\scripts\build.bat
   ```

2. **Run an existing test** to verify the system works:

   ```cmd
   .\build\Release\MathTest.exe
   ```

3. **Examine test output** to understand the format:
   ```
   ========================================
    Game Engine Kiro - Math Tests
   ========================================
   Testing vector operations...
     [PASS] Vector operations passed
   Testing cross product operations...
     [PASS] Cross product operations passed
   ========================================
   [SUCCESS] ALL TESTS PASSED!
   ========================================
   ```

## Test Creation

### Basic Test Structure

Every test file follows this standard pattern:

```cpp
#include <iostream>
#include <cassert>
#include <cmath>
#include "TestUtils.h"
#include "Core/ComponentToTest.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test description and requirements reference
 * Requirements: X.X, Y.Y (Description of what's being validated)
 */
bool TestSpecificFeature() {
    TestOutput::PrintTestStart("specific feature");

    // Setup - Initialize test data
    ComponentType component;

    // Execution - Perform the operation being tested
    auto result = component.DoSomething();

    // Validation - Check results using assertions
    EXPECT_TRUE(result.IsValid());
    EXPECT_NEARLY_EQUAL(result.GetValue(), expectedValue);

    TestOutput::PrintTestPass("specific feature");
    return true;
}

int main() {
    TestOutput::PrintHeader("Component");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Component Tests");

        // Run all tests
        allPassed &= suite.RunTest("Specific Feature", TestSpecificFeature);
        // Add more tests here...

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

### Using Test Templates

The engine provides templates for different test types in `tests/templates/`:

#### Unit Test Template

```cmd
# Copy the template
copy tests\templates\unit_test_template.cpp tests\unit\test_mycomponent.cpp

# Edit the file to replace placeholders:
# [COMPONENT] -> MYCOMPONENT
# [Component] -> MyComponent
# Add: #include "Core/MyComponent.h"
```

#### Integration Test Template

```cmd
# Copy the template
copy tests\templates\integration_test_template.cpp tests\integration\test_mysystem_integration.cpp

# Edit the file to replace placeholders and add necessary headers
```

#### Performance Test Template

```cmd
# Create performance directory if needed
mkdir tests\performance

# Copy the template
copy tests\templates\performance_test_template.cpp tests\performance\test_mycomponent_performance.cpp
```

### Test Utilities

The `TestUtils.h` header provides comprehensive utilities for test creation:

#### Floating-Point Comparisons

```cpp
// Basic floating-point comparison
EXPECT_NEARLY_EQUAL(actualValue, expectedValue);

// Custom epsilon tolerance
EXPECT_NEARLY_EQUAL_EPSILON(actualValue, expectedValue, 0.0001f);

// Vector comparisons
EXPECT_NEAR_VEC3(actualVector, expectedVector);
EXPECT_NEAR_VEC4(actualVector4, expectedVector4);

// Quaternion comparisons
EXPECT_NEAR_QUAT(actualQuat, expectedQuat);

// Matrix comparisons
EXPECT_MATRIX_EQUAL(actualMatrix, expectedMatrix);
```

#### Standard Assertions

```cpp
// Boolean assertions
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Pointer assertions
EXPECT_NULL(pointer);
EXPECT_NOT_NULL(pointer);

// Equality assertions
EXPECT_EQUAL(actual, expected);
EXPECT_NOT_EQUAL(actual, expected);

// Range assertions
EXPECT_IN_RANGE(value, minValue, maxValue);

// String assertions
EXPECT_STRING_EQUAL(actualString, expectedString);
```

#### Performance Testing

```cpp
// Simple performance validation
bool performanceTest = PerformanceTest::ValidatePerformance(
    "operation name",
    [&]() {
        // Operation to test
        component.ExpensiveOperation();
    },
    1.0, // Threshold in milliseconds
    1000 // Number of iterations
);

// Manual timing
TestTimer timer;
// ... perform operations ...
double elapsed = timer.ElapsedMs();
TestOutput::PrintTiming("operation", elapsed, iterations);
```

#### Output Formatting

```cpp
// Test progress messages
TestOutput::PrintTestStart("feature name");
TestOutput::PrintTestPass("feature name");
TestOutput::PrintTestFail("feature name");

// Informational messages
TestOutput::PrintInfo("Additional information");
TestOutput::PrintWarning("Warning message");
TestOutput::PrintError("Error message");

// Performance timing
TestOutput::PrintTiming("operation", timeMs, iterations);
```

## Test Execution

### Building Tests

Tests are automatically built with the main project:

```cmd
# Build all tests
.\scripts\build.bat

# Build specific configuration
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Running Individual Tests

```cmd
# Run unit tests
.\build\Release\MathTest.exe
.\build\Release\QuaternionTest.exe
.\build\Release\MatrixTest.exe

# Run integration tests
.\build\Release\BulletIntegrationTest.exe
.\build\Release\BulletConversionTest.exe
.\build\Release\PhysicsQueriesTest.exe

# Run performance tests (if created)
.\build\Release\MathPerformanceTest.exe
```

### Running Multiple Tests

```cmd
# Run all unit tests (example batch script)
for %%f in (build\Release\*Test.exe) do (
    echo Running %%f...
    %%f
    if errorlevel 1 echo FAILED: %%f
)
```

### Test Output Interpretation

#### Successful Test Output

```
========================================
 Game Engine Kiro - Math Tests
========================================
Testing vector operations...
  [PASS] Vector operations passed
Testing cross product operations...
  [PASS] Cross product operations passed
  [INFO] Test Summary:
  [INFO]   Total: 2
  [INFO]   Passed: 2
  [INFO]   Failed: 0
  [INFO]   Total Time: 1.234ms
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

#### Failed Test Output

```
========================================
 Game Engine Kiro - Math Tests
========================================
Testing vector operations...
  [FAILED] Vector operations failed
    Expected: (1.000, 2.000, 3.000)
    Actual: (1.001, 2.000, 3.000)
    Location: test_math.cpp:45
========================================
[FAILED] SOME TESTS FAILED!
========================================
```

#### Performance Test Output

```
Testing math performance...
  [INFO] vector addition completed in 0.123ms (1000 iterations, 0.000123ms per iteration)
  [PASS] vector addition passed
  [INFO] dot product completed in 0.089ms (1000 iterations, 0.000089ms per iteration)
  [PASS] dot product passed
```

## Best Practices

### Test Design Principles

1. **Test One Thing**: Each test function should focus on a single aspect or feature
2. **Clear Names**: Use descriptive names that explain what is being tested
3. **Independent Tests**: Tests should not depend on each other's execution order
4. **Comprehensive Coverage**: Test normal cases, edge cases, and error conditions
5. **Performance Awareness**: Consider the performance impact of test code itself

### Code Organization

```cpp
// Group related tests logically
bool TestVectorBasicOperations() { /* ... */ }
bool TestVectorEdgeCases() { /* ... */ }
bool TestVectorPerformance() { /* ... */ }

// Use consistent naming patterns
bool TestMatrixMultiplication() { /* ... */ }
bool TestMatrixInversion() { /* ... */ }
bool TestMatrixTransformation() { /* ... */ }
```

### Error Handling

```cpp
bool TestWithErrorHandling() {
    TestOutput::PrintTestStart("error handling");

    try {
        // Test code that might throw
        component.RiskyOperation();

        // If we get here, test what should happen
        EXPECT_TRUE(component.IsInValidState());

    } catch (const ExpectedException& e) {
        // Expected exception - test passed
        TestOutput::PrintInfo("Caught expected exception: " + std::string(e.what()));
    } catch (const std::exception& e) {
        // Unexpected exception - test failed
        TestOutput::PrintError("Unexpected exception: " + std::string(e.what()));
        return false;
    }

    TestOutput::PrintTestPass("error handling");
    return true;
}
```

### Platform-Specific Testing

```cpp
bool TestPlatformSpecificFeature() {
    TestOutput::PrintTestStart("platform-specific feature");

#ifdef _WIN32
    // Windows-specific test code
    TestOutput::PrintInfo("Running Windows-specific validation");
    EXPECT_TRUE(WindowsSpecificFunction());
#elif defined(__linux__)
    // Linux-specific test code
    TestOutput::PrintInfo("Running Linux-specific validation");
    EXPECT_TRUE(LinuxSpecificFunction());
#elif defined(__APPLE__)
    // macOS-specific test code
    TestOutput::PrintInfo("Running macOS-specific validation");
    EXPECT_TRUE(MacOSSpecificFunction());
#else
    TestOutput::PrintWarning("Platform-specific test skipped on unknown platform");
#endif

    TestOutput::PrintTestPass("platform-specific feature");
    return true;
}
```

## Common Patterns

### Math Component Testing

```cpp
bool TestMathOperations() {
    TestOutput::PrintTestStart("math operations");

    // Vector operations
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 result = a + b;
    Math::Vec3 expected(5.0f, 7.0f, 9.0f);

    EXPECT_NEAR_VEC3(result, expected);

    // Floating-point operations with epsilon
    float dotProduct = glm::dot(a, b);
    float expectedDot = 32.0f; // 1*4 + 2*5 + 3*6

    EXPECT_NEARLY_EQUAL(dotProduct, expectedDot);

    TestOutput::PrintTestPass("math operations");
    return true;
}
```

### Physics Integration Testing

```cpp
bool TestPhysicsIntegration() {
    TestOutput::PrintTestStart("physics integration");

    // Initialize physics world
    btDefaultCollisionConfiguration* config = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(config);
    btDbvtBroadphase* broadphase = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

    btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(
        dispatcher, broadphase, solver, config);

    // Test world initialization
    EXPECT_NOT_NULL(world);
    EXPECT_EQUAL(world->getNumCollisionObjects(), 0);

    // Test gravity setting
    world->setGravity(btVector3(0, -9.81f, 0));
    btVector3 gravity = world->getGravity();

    EXPECT_NEARLY_EQUAL(gravity.getY(), -9.81f);

    // Cleanup
    delete world;
    delete solver;
    delete broadphase;
    delete dispatcher;
    delete config;

    TestOutput::PrintTestPass("physics integration");
    return true;
}
```

### Performance Testing Pattern

```cpp
bool TestPerformanceCriticalOperation() {
    TestOutput::PrintTestStart("performance critical operation");

    // Setup test data
    std::vector<Math::Vec3> vectors;
    for (int i = 0; i < 10000; ++i) {
        vectors.emplace_back(i * 0.1f, i * 0.2f, i * 0.3f);
    }

    // Test performance with threshold
    bool performanceTest = PerformanceTest::ValidatePerformance(
        "vector normalization",
        [&]() {
            for (auto& vec : vectors) {
                vec = glm::normalize(vec);
            }
        },
        10.0, // 10ms threshold
        1     // Single iteration (already testing 10k operations)
    );

    EXPECT_TRUE(performanceTest);

    TestOutput::PrintTestPass("performance critical operation");
    return true;
}
```

### Memory Testing Pattern

```cpp
bool TestMemoryUsage() {
    TestOutput::PrintTestStart("memory usage");

    // Test for memory leaks
    bool memoryTest = MemoryTest::TestForMemoryLeaks(
        "component creation/destruction",
        []() {
            for (int i = 0; i < 1000; ++i) {
                auto component = std::make_unique<TestComponent>();
                component->Initialize();
                // component automatically destroyed
            }
        }
    );

    EXPECT_TRUE(memoryTest);

    TestOutput::PrintTestPass("memory usage");
    return true;
}
```

## Troubleshooting

### Common Issues and Solutions

#### Build Errors

**Issue**: Test fails to compile with missing header errors

```
error: 'TestComponent' was not declared in this scope
```

**Solution**:

1. Ensure the component header is included:
   ```cpp
   #include "Core/TestComponent.h"
   ```
2. Check that the component exists in the engine library
3. Verify the include path is correct

**Issue**: Linking errors with undefined references

```
undefined reference to `TestComponent::Initialize()'
```

**Solution**:

1. Ensure the component is properly linked in CMakeLists.txt
2. Check that the component implementation exists
3. Verify the component is part of the GameEngineKiro library

#### Test Execution Errors

**Issue**: Test crashes with assertion failure

```
Assertion failed: result.IsValid(), file test_component.cpp, line 45
```

**Solution**:

1. Use the enhanced assertion macros for better error reporting:
   ```cpp
   EXPECT_TRUE(result.IsValid()); // Instead of assert()
   ```
2. Add debug output to understand the failure:
   ```cpp
   TestOutput::PrintInfo("Result state: " + result.GetStateString());
   ```

**Issue**: Floating-point comparison failures

```
[FAILED] Vector comparison failed
  Expected: (1.000, 2.000, 3.000)
  Actual: (1.000001, 2.000000, 3.000000)
```

**Solution**:

1. Use appropriate epsilon tolerance:
   ```cpp
   EXPECT_NEAR_VEC3_EPSILON(actual, expected, 0.0001f);
   ```
2. Consider if the precision requirement is realistic

#### Performance Test Issues

**Issue**: Performance tests fail inconsistently

```
[FAILED] Operation performance failed (too slow)
  Expected: < 1.000ms per iteration
  Actual: 1.234ms per iteration
```

**Solution**:

1. Run performance tests multiple times and use average:
   ```cpp
   double avgTime = PerformanceTest::MeasureAverageTime(operation, 100);
   ```
2. Adjust thresholds based on actual hardware capabilities
3. Consider system load and other factors affecting performance

#### Platform-Specific Issues

**Issue**: Tests pass on Windows but fail on Linux

```
[ERROR] Platform-specific function not available
```

**Solution**:

1. Add platform-specific conditional compilation:
   ```cpp
   #ifdef __linux__
       // Linux-specific implementation
   #endif
   ```
2. Use platform-agnostic alternatives where possible
3. Skip platform-specific tests when not applicable

### Debugging Test Failures

#### Using Visual Studio Debugger

1. **Set Breakpoints**: Place breakpoints in test functions
2. **Debug Configuration**: Use Debug build for better debugging experience
3. **Watch Variables**: Monitor test variables and component state
4. **Call Stack**: Examine the call stack when assertions fail

#### Adding Debug Output

```cpp
bool TestWithDebugOutput() {
    TestOutput::PrintTestStart("debug example");

    ComponentType component;

    // Add debug output
    TestOutput::PrintInfo("Component initialized");
    TestOutput::PrintInfo("Component state: " + component.GetStateString());

    auto result = component.DoSomething();

    // More debug output
    TestOutput::PrintInfo("Operation result: " + result.ToString());

    if (!result.IsValid()) {
        TestOutput::PrintError("Result validation failed");
        TestOutput::PrintError("Error details: " + result.GetErrorMessage());
        return false;
    }

    TestOutput::PrintTestPass("debug example");
    return true;
}
```

#### Isolating Test Failures

```cpp
// Temporarily disable other tests to isolate the failing one
int main() {
    TestOutput::PrintHeader("Component");

    bool allPassed = true;

    try {
        TestSuite suite("Component Tests");

        // Comment out other tests to isolate the failing one
        // allPassed &= suite.RunTest("Working Test", TestWorkingFeature);
        allPassed &= suite.RunTest("Failing Test", TestFailingFeature);
        // allPassed &= suite.RunTest("Another Test", TestAnotherFeature);

        suite.PrintSummary();
        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    }
}
```

### Getting Help

1. **Review Existing Tests**: Look at similar tests in `tests/unit/` and `tests/integration/`
2. **Check Documentation**: Refer to `docs/testing-standards.md` for coding standards
3. **Examine Templates**: Use templates in `tests/templates/` as starting points
4. **Test Utilities**: Review `tests/TestUtils.h` for available helper functions

## Adding New Tests

### Step-by-Step Process

#### 1. Determine Test Type

- **Unit Test**: Testing individual component functionality
- **Integration Test**: Testing system interactions
- **Performance Test**: Benchmarking and performance validation

#### 2. Create Test File

```cmd
# For unit tests
copy tests\templates\unit_test_template.cpp tests\unit\test_newcomponent.cpp

# For integration tests
copy tests\templates\integration_test_template.cpp tests\integration\test_newsystem_integration.cpp

# For performance tests
copy tests\templates\performance_test_template.cpp tests\performance\test_newcomponent_performance.cpp
```

#### 3. Customize Template

Replace placeholders in the template:

```cpp
// Replace [COMPONENT] with NEWCOMPONENT
// Replace [Component] with NewComponent
// Add appropriate includes:
#include "Core/NewComponent.h"
```

#### 4. Implement Test Functions

```cpp
bool TestNewComponentFeature() {
    TestOutput::PrintTestStart("new component feature");

    // Setup
    NewComponent component;

    // Execution
    auto result = component.NewFeature();

    // Validation
    EXPECT_TRUE(result.IsValid());
    EXPECT_EQUAL(result.GetValue(), expectedValue);

    TestOutput::PrintTestPass("new component feature");
    return true;
}
```

#### 5. Update Main Function

```cpp
int main() {
    TestOutput::PrintHeader("NewComponent");

    bool allPassed = true;

    try {
        TestSuite suite("NewComponent Tests");

        // Add your test here
        allPassed &= suite.RunTest("New Feature", TestNewComponentFeature);

        suite.PrintSummary();
        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    }
}
```

#### 6. Build and Test

```cmd
# Build the project (tests are auto-discovered)
.\scripts\build.bat

# Run your new test
.\build\Release\NewcomponentTest.exe
```

### CMake Integration

Tests are automatically discovered by the CMake system. The `discover_and_add_tests` function in CMakeLists.txt will:

1. Find all `test_*.cpp` files in the appropriate directories
2. Create appropriately named executables
3. Link with the engine library
4. Apply coverage settings
5. Add platform-specific dependencies

### Manual CMake Configuration

If you need custom configuration for your test:

```cmake
# Add custom test executable
add_executable(CustomTest
    tests/unit/test_custom.cpp
)

# Apply standard settings
apply_coverage_settings(CustomTest)
target_link_libraries(CustomTest PRIVATE GameEngineKiro)
target_include_directories(CustomTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Add custom dependencies if needed
if(SpecialLibrary_FOUND)
    target_link_libraries(CustomTest PRIVATE SpecialLibrary::SpecialLibrary)
endif()
```

## Framework Integration

### CMake Build System

The testing system integrates seamlessly with the CMake build system:

#### Automatic Test Discovery

```cmake
# Automatically discovers and adds test files
discover_and_add_tests("tests/unit" "unit")
discover_and_add_tests("tests/integration" "integration")
discover_and_add_tests("tests/performance" "performance")
```

#### Helper Functions

```cmake
# Create unit test with standard configuration
add_unit_test(TestName tests/unit/test_component.cpp)

# Create integration test with standard configuration
add_integration_test(TestName tests/integration/test_system.cpp)

# Create performance test with standard configuration
add_performance_test(TestName tests/performance/test_performance.cpp)
```

#### Coverage Support

```cmake
# Apply coverage settings to test
apply_coverage_settings(TestExecutable)
```

### Continuous Integration

The testing system is designed to work with CI/CD pipelines:

#### Exit Codes

- **0**: All tests passed
- **1**: One or more tests failed or error occurred

#### Output Format

- Consistent, parseable output format
- Clear success/failure indicators
- Detailed error reporting with file and line information

#### Example CI Script

```cmd
@echo off
setlocal enabledelayedexpansion

echo Building tests...
call .\scripts\build.bat
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo Running unit tests...
set FAILED_TESTS=

for %%f in (build\Release\*Test.exe) do (
    echo Running %%f...
    %%f
    if errorlevel 1 (
        echo FAILED: %%f
        set FAILED_TESTS=!FAILED_TESTS! %%f
    )
)

if defined FAILED_TESTS (
    echo.
    echo Failed tests: %FAILED_TESTS%
    exit /b 1
) else (
    echo.
    echo All tests passed!
    exit /b 0
)
```

### IDE Integration

#### Visual Studio

1. **Project Generation**: Tests appear as separate projects in the solution
2. **Debugging**: Full debugging support with breakpoints and watch variables
3. **Test Explorer**: Tests can be run individually from the IDE
4. **IntelliSense**: Full code completion and error checking

#### VS Code

1. **CMake Tools**: Automatic test discovery and building
2. **C++ Extension**: Code completion and error checking
3. **Debugging**: Integrated debugging support
4. **Terminal Integration**: Easy test execution from integrated terminal

#### CLion

1. **CMake Integration**: Automatic project configuration
2. **Test Runner**: Built-in test execution and results display
3. **Debugging**: Full debugging capabilities
4. **Code Analysis**: Static analysis and code quality checks

## Advanced Topics

### Custom Assertion Macros

You can create custom assertion macros for domain-specific testing:

```cpp
// Custom macro for testing game objects
#define EXPECT_GAME_OBJECT_VALID(obj) \
    do { \
        if (!(obj).IsValid()) { \
            GameEngine::Testing::AssertionReporter::ReportFailure(__func__, \
                #obj ".IsValid()", "valid game object", "invalid game object", \
                __FILE__, __LINE__); \
            return false; \
        } \
        if ((obj).GetId() == 0) { \
            GameEngine::Testing::AssertionReporter::ReportFailure(__func__, \
                #obj ".GetId() != 0", "non-zero ID", "zero ID", \
                __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

// Usage
bool TestGameObject() {
    GameObject obj = CreateGameObject();
    EXPECT_GAME_OBJECT_VALID(obj);
    return true;
}
```

### Test Data Management

For tests requiring external data files:

```cpp
class TestDataManager {
public:
    static std::string GetTestDataPath(const std::string& filename) {
        return "tests/data/" + filename;
    }

    static bool LoadTestMesh(const std::string& filename, Mesh& outMesh) {
        std::string fullPath = GetTestDataPath(filename);
        return MeshLoader::Load(fullPath, outMesh);
    }
};

bool TestMeshLoading() {
    TestOutput::PrintTestStart("mesh loading");

    Mesh testMesh;
    bool loaded = TestDataManager::LoadTestMesh("test_cube.obj", testMesh);

    EXPECT_TRUE(loaded);
    EXPECT_TRUE(testMesh.GetVertexCount() > 0);

    TestOutput::PrintTestPass("mesh loading");
    return true;
}
```

### Parameterized Tests

For testing multiple similar scenarios:

```cpp
struct VectorTestCase {
    Math::Vec3 input1;
    Math::Vec3 input2;
    Math::Vec3 expected;
    std::string description;
};

bool TestVectorAdditionParameterized() {
    TestOutput::PrintTestStart("vector addition (parameterized)");

    std::vector<VectorTestCase> testCases = {
        {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}, {5.0f, 7.0f, 9.0f}, "positive values"},
        {{-1.0f, -2.0f, -3.0f}, {1.0f, 2.0f, 3.0f}, {0.0f, 0.0f, 0.0f}, "negative + positive"},
        {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, "zero vector"},
    };

    for (const auto& testCase : testCases) {
        TestOutput::PrintInfo("Testing: " + testCase.description);

        Math::Vec3 result = testCase.input1 + testCase.input2;

        if (!FloatComparison::IsNearlyEqual(result, testCase.expected)) {
            TestOutput::PrintError("Failed case: " + testCase.description);
            TestOutput::PrintError("Expected: " + StringUtils::FormatVec3(testCase.expected));
            TestOutput::PrintError("Actual: " + StringUtils::FormatVec3(result));
            return false;
        }
    }

    TestOutput::PrintTestPass("vector addition (parameterized)");
    return true;
}
```

### Mock Objects and Stubs

For testing components with dependencies:

```cpp
class MockRenderer : public GraphicsRenderer {
public:
    mutable int renderCallCount = 0;
    mutable std::vector<RenderCommand> capturedCommands;

    void Render(const RenderCommand& command) const override {
        renderCallCount++;
        capturedCommands.push_back(command);
    }

    bool Initialize() override { return true; }
    void Shutdown() override {}
};

bool TestRenderingSystem() {
    TestOutput::PrintTestStart("rendering system with mock");

    MockRenderer mockRenderer;
    RenderingSystem system(&mockRenderer);

    // Test rendering
    system.RenderFrame();

    EXPECT_EQUAL(mockRenderer.renderCallCount, 1);
    EXPECT_TRUE(mockRenderer.capturedCommands.size() > 0);

    TestOutput::PrintTestPass("rendering system with mock");
    return true;
}
```

### Test Fixtures and Setup/Teardown

For tests requiring complex setup:

```cpp
class PhysicsTestFixture {
public:
    btDiscreteDynamicsWorld* world;
    btDefaultCollisionConfiguration* config;
    btCollisionDispatcher* dispatcher;
    btDbvtBroadphase* broadphase;
    btSequentialImpulseConstraintSolver* solver;

    bool SetUp() {
        config = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(config);
        broadphase = new btDbvtBroadphase();
        solver = new btSequentialImpulseConstraintSolver();

        world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
        world->setGravity(btVector3(0, -9.81f, 0));

        return world != nullptr;
    }

    void TearDown() {
        delete world;
        delete solver;
        delete broadphase;
        delete dispatcher;
        delete config;
    }
};

bool TestPhysicsWithFixture() {
    TestOutput::PrintTestStart("physics with fixture");

    PhysicsTestFixture fixture;
    if (!fixture.SetUp()) {
        TestOutput::PrintError("Failed to set up physics fixture");
        return false;
    }

    // Test code using fixture.world
    EXPECT_NOT_NULL(fixture.world);
    EXPECT_EQUAL(fixture.world->getNumCollisionObjects(), 0);

    fixture.TearDown();

    TestOutput::PrintTestPass("physics with fixture");
    return true;
}
```

This comprehensive testing guide provides everything needed to create, execute, and maintain tests within the Game Engine Kiro testing system. The framework-independent approach ensures simplicity while maintaining professional standards and cross-platform compatibility.
