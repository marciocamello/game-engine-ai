# Design Document - Simple Testing System

## Overview

The Simple Testing System provides a lightweight, framework-independent approach to testing Game Engine Kiro components. The design prioritizes simplicity, cross-platform compatibility, and professional output formatting while maintaining full integration with the existing build system.

## Architecture

### Core Design Principles

1. **Zero External Dependencies**: Uses only standard C++ libraries and engine components
2. **Professional Output**: Consistent, text-based formatting without Unicode characters
3. **Simple Patterns**: Predictable test structure that's easy to learn and maintain
4. **Build Integration**: Seamless integration with existing CMake build system
5. **Cross-Platform**: Compatible with Windows, Linux, and macOS development environments

### System Architecture

```
Game Engine Kiro Testing System
├── Test Executables (Built by CMake)
│   ├── MathUnitTest.exe
│   ├── PhysicsUnitTest.exe
│   └── GraphicsUnitTest.exe
├── Test Source Files
│   ├── tests/unit/test_math.cpp
│   ├── tests/unit/test_physics.cpp
│   └── tests/unit/test_graphics.cpp
├── Shared Utilities
│   ├── Test Helper Functions (IsNearlyEqual, etc.)
│   └── Standard Output Formatting
└── Documentation
    ├── docs/coding-standards.md
    └── docs/testing-simple.md
```

## Components and Interfaces

### Test File Structure

Each test file follows a consistent pattern:

```cpp
// Standard includes
#include <iostream>
#include <cassert>
#include <cmath>
#include "Core/ComponentToTest.h"

using namespace GameEngine;

// Helper functions (shared across tests)
bool IsNearlyEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

// Individual test functions
bool TestComponentFeature() {
    std::cout << "Testing component feature..." << std::endl;

    // Setup
    ComponentType component;

    // Execution
    auto result = component.DoSomething();

    // Validation
    assert(result.IsValid());

    // Success output
    std::cout << "  [PASS] Component feature passed" << std::endl;
    return true;
}

// Main test runner
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Game Engine Kiro - Component Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    bool allPassed = true;

    try {
        allPassed &= TestComponentFeature();
        // Additional tests...

        std::cout << "========================================" << std::endl;
        if (allPassed) {
            std::cout << "[SUCCESS] ALL TESTS PASSED!" << std::endl;
            return 0;
        } else {
            std::cout << "[FAILED] SOME TESTS FAILED!" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << "[ERROR] TEST EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "[ERROR] UNKNOWN TEST ERROR!" << std::endl;
        return 1;
    }
}
```

### Output Formatting Standards

#### Standard Prefixes

| Prefix      | Usage                      | Example                                    |
| ----------- | -------------------------- | ------------------------------------------ |
| `[PASS]`    | Individual test success    | `[PASS] Vector operations passed`          |
| `[FAILED]`  | Individual test failure    | `[FAILED] Matrix multiplication failed`    |
| `[SUCCESS]` | Overall test suite success | `[SUCCESS] ALL TESTS PASSED!`              |
| `[ERROR]`   | Error conditions           | `[ERROR] TEST EXCEPTION: Division by zero` |
| `[WARNING]` | Warning conditions         | `[WARNING] Using fallback implementation`  |
| `[INFO]`    | Informational messages     | `[INFO] Testing with 1000 iterations`      |

#### Output Format Template

```
========================================
 Game Engine Kiro - [Component] Tests
========================================
Testing [feature description]...
  [PASS] [Feature] passed
Testing [another feature]...
  [PASS] [Another feature] passed
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

### CMake Integration

#### Test Executable Definition

```cmake
# Add unit test executable
add_executable(ComponentUnitTest
    tests/unit/test_component.cpp
)

# Apply coverage settings
apply_coverage_settings(ComponentUnitTest)

# Link with engine library
target_link_libraries(ComponentUnitTest PRIVATE GameEngineKiro)

# Add additional dependencies if needed
if(SpecialLibrary_FOUND)
    target_link_libraries(ComponentUnitTest PRIVATE SpecialLibrary::SpecialLibrary)
endif()
```

### Helper Utilities

#### Floating Point Comparison

```cpp
bool IsNearlyEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

bool IsNearlyEqual(const Math::Vec3& a, const Math::Vec3& b, float epsilon = 0.001f) {
    return IsNearlyEqual(a.x, b.x, epsilon) &&
           IsNearlyEqual(a.y, b.y, epsilon) &&
           IsNearlyEqual(a.z, b.z, epsilon);
}
```

#### Test Timing (Optional)

```cpp
#include <chrono>

class TestTimer {
public:
    TestTimer() : start(std::chrono::high_resolution_clock::now()) {}

    double ElapsedMs() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / 1000.0;
    }

private:
    std::chrono::high_resolution_clock::time_point start;
};
```

## Data Models

### Test Result Structure

```cpp
struct TestResult {
    std::string testName;
    bool passed;
    std::string errorMessage;
    double executionTimeMs;
};

struct TestSuiteResult {
    std::string suiteName;
    std::vector<TestResult> tests;
    int totalTests;
    int passedTests;
    int failedTests;
    double totalTimeMs;
};
```

### Test Configuration

```cpp
struct TestConfig {
    float floatEpsilon = 0.001f;
    bool enableTiming = false;
    bool verboseOutput = false;
    int maxIterations = 1000;
};
```

## Error Handling

### Exception Management

```cpp
try {
    // Test execution
    allPassed &= TestFunction();
} catch (const std::exception& e) {
    std::cout << "[ERROR] TEST EXCEPTION: " << e.what() << std::endl;
    return 1;
} catch (...) {
    std::cout << "[ERROR] UNKNOWN TEST ERROR!" << std::endl;
    return 1;
}
```

### Assertion Strategy

- Use `assert()` for immediate test failure with debugging information
- Provide custom assertion macros if needed for better error reporting
- Ensure assertions are enabled in debug builds but can be disabled in release

### Error Reporting

```cpp
void ReportTestFailure(const std::string& testName, const std::string& expected, const std::string& actual) {
    std::cout << "[FAILED] " << testName << std::endl;
    std::cout << "  Expected: " << expected << std::endl;
    std::cout << "  Actual: " << actual << std::endl;
}
```

## Testing Strategy

### Unit Testing Approach

1. **Math Components**: Test mathematical operations, transformations, and utilities
2. **Core Systems**: Test logging, resource management, and configuration
3. **Graphics Components**: Test shader compilation, texture loading, and rendering utilities
4. **Physics Components**: Test collision detection, rigid body operations, and utilities
5. **Input Systems**: Test input parsing, key mapping, and event handling

### Integration Testing Support

While this system focuses on unit testing, it can support integration testing by:

1. **Component Interaction**: Testing how multiple components work together
2. **System Initialization**: Testing engine startup and shutdown sequences
3. **Resource Loading**: Testing asset loading and management workflows
4. **Performance Validation**: Testing system performance under various conditions

### Test Organization

```
tests/
├── unit/
│   ├── test_math.cpp           # Mathematical operations
│   ├── test_logger.cpp         # Logging system
│   ├── test_resource.cpp       # Resource management
│   ├── test_input.cpp          # Input handling
│   └── test_graphics.cpp       # Graphics utilities
└── integration/
    ├── test_engine_startup.cpp # Engine initialization
    ├── test_asset_pipeline.cpp # Asset loading workflow
    └── test_rendering_flow.cpp # Rendering pipeline
```

## Implementation Guidelines

### Creating New Tests

1. **File Naming**: Use `test_[component].cpp` pattern
2. **Function Naming**: Use `Test[Component][Feature]()` pattern
3. **Output Formatting**: Follow established prefix standards
4. **Error Handling**: Include comprehensive exception handling
5. **Documentation**: Add comments explaining complex test logic

### Best Practices

1. **Keep Tests Simple**: Each test should focus on a single aspect
2. **Use Descriptive Names**: Test names should clearly indicate what's being tested
3. **Provide Context**: Include setup and cleanup in test functions
4. **Handle Edge Cases**: Test boundary conditions and error scenarios
5. **Maintain Consistency**: Follow established patterns and formatting

### Performance Considerations

1. **Fast Execution**: Tests should complete quickly for rapid development cycles
2. **Minimal Dependencies**: Avoid heavy external libraries or complex setups
3. **Efficient Assertions**: Use efficient comparison methods for complex data types
4. **Resource Cleanup**: Ensure proper cleanup to prevent resource leaks

This design provides a solid foundation for a simple, effective testing system that meets all requirements while maintaining the professional standards and cross-platform compatibility needed for Game Engine Kiro.
