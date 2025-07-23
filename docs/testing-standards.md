# Game Engine Kiro - Testing Standards

## Overview

This document establishes comprehensive coding standards for the Game Engine Kiro testing system. These standards ensure consistent, professional output formatting and maintainable test code across all engine components.

### Related Documentation

**This document works in conjunction with other testing documentation:**

**Foundation Documents:**

- **[Testing Guide](testing-guide.md)**: Comprehensive testing instructions and examples
- **[Testing Guidelines](testing-guidelines.md)**: High-level guidelines and best practices

**Output and Formatting:**

- **[Test Output Formatting](testing-output-formatting.md)**: Detailed formatting standards
- **[Test Output Consistency](testing-output-consistency-guide.md)**: Consistency guidelines across test types

**Specialized Testing:**

- **[OpenGL Context Limitations](testing-opengl-limitations.md)**: Handling OpenGL context issues
- **[Resource Testing Patterns](testing-resource-patterns.md)**: Resource management testing patterns
- **[Mock Resource Implementation](testing-mock-resources.md)**: Mock resource creation and usage

**Documentation Quality:**

- **[Code Examples Validation](testing-code-examples-validation.md)**: Keeping examples current and accurate
- **[API Reference](api-reference.md)**: Complete API documentation with examples

**Migration and Strategy:**

- **[Testing Migration](testing-migration.md)**: Updating existing tests to new standards
- **[Testing Strategy](testing-strategy.md)**: Overall testing approach and methodology

## Test File Organization

### File Naming Convention

- **Unit Tests**: `test_[component].cpp` (e.g., `test_math.cpp`, `test_logger.cpp`)
- **Integration Tests**: `test_[system]_[feature].cpp` (e.g., `test_physics_integration.cpp`)
- **Performance Tests**: `test_[component]_performance.cpp`

### Directory Structure

```
tests/
├── unit/                    # Unit tests for individual components
│   ├── test_math.cpp
│   ├── test_logger.cpp
│   └── test_resource.cpp
├── integration/             # Integration tests for system interactions
│   ├── test_physics_integration.cpp
│   └── test_rendering_pipeline.cpp
└── performance/             # Performance and benchmark tests
    ├── test_math_performance.cpp
    └── test_physics_performance.cpp
```

## Code Structure Standards

### Test Function Naming

- **Pattern**: `Test[Component][Feature]()`
- **Examples**: `TestVectorOperations()`, `TestMatrixMultiplication()`, `TestLoggerFileOutput()`

### Test File Template

```cpp
#include <iostream>
#include <cassert>
#include <cmath>
#include <chrono>
#include "Core/ComponentToTest.h"

using namespace GameEngine;

// Helper functions (if needed)
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
            std::cout << "========================================" << std::endl;
            return 0;
        } else {
            std::cout << "[FAILED] SOME TESTS FAILED!" << std::endl;
            std::cout << "========================================" << std::endl;
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

## Output Formatting Standards

**IMPORTANT**: All test output formatting must follow the comprehensive standards defined in [Test Output Formatting Standards](testing-output-formatting.md). This section provides a summary of key requirements.

### Core Requirements

1. **Use TestOutput Methods Only**: Never use `std::cout`, `printf`, or direct console output
2. **Consistent Test Names**: Use lowercase with spaces, same string for Start/Pass calls
3. **Standard Status Prefixes**: `[PASS]`, `[FAILED]`, `[SUCCESS]`, `[ERROR]`, `[WARNING]`, `[INFO]`
4. **Proper Indentation**: 2 spaces for test results, 4 spaces for error details
5. **ASCII Only**: No Unicode characters for maximum compatibility

### Required Pattern

```cpp
bool TestFeatureName() {
    TestOutput::PrintTestStart("feature name");

    // Test implementation with assertions
    EXPECT_TRUE(condition);

    TestOutput::PrintTestPass("feature name");
    return true;
}
```

### Standard Output Structure

```
========================================
 Game Engine Kiro - Component Tests
========================================
Testing feature name...
  [PASS] feature name passed
  [INFO] Test Summary:
  [INFO]   Total: 1
  [INFO]   Passed: 1
  [INFO]   Failed: 0
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

### Context-Aware Testing Standards

Tests must handle OpenGL context limitations gracefully:

```cpp
bool TestGraphicsFeature() {
    TestOutput::PrintTestStart("graphics feature");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("graphics feature");
        return true;
    }

    // OpenGL-dependent test logic
    TestOutput::PrintTestPass("graphics feature");
    return true;
}
```

### Resource Testing Standards

Use mock resources for consistent testing:

```cpp
bool TestResourceFeature() {
    TestOutput::PrintTestStart("resource feature");

    ResourceManager manager;
    auto resource = manager.Load<MockResource>("test.dat");
    EXPECT_NOT_NULL(resource);
    EXPECT_TRUE(resource->GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("resource feature");
    return true;
}
```

### Migration Requirements

Existing tests must be updated to:

- Replace all `std::cout` with `TestOutput::Print*` methods
- Use consistent lowercase naming for test names
- Remove excessive `PrintInfo()` calls
- Follow standard main function exception handling pattern
- Add OpenGL context awareness where needed
- Use mock resources for resource-dependent tests

For complete formatting guidelines and examples, see [Test Output Formatting Standards](testing-output-formatting.md) and [Test Output Consistency Guidelines](testing-output-consistency-guide.md).

## Error Handling Standards

### Exception Handling Pattern

```cpp
try {
    allPassed &= TestFunction();
} catch (const std::exception& e) {
    std::cout << "[ERROR] TEST EXCEPTION: " << e.what() << std::endl;
    return 1;
} catch (...) {
    std::cout << "[ERROR] UNKNOWN TEST ERROR!" << std::endl;
    return 1;
}
```

### Assertion Guidelines

1. **Use Standard Assert**: Prefer `assert()` for immediate failure with debugging info
2. **Custom Assertions**: Create helper functions for complex validations
3. **Error Context**: Provide meaningful error messages with expected vs actual values

### Error Reporting Format

```cpp
void ReportTestFailure(const std::string& testName, const std::string& expected, const std::string& actual) {
    std::cout << "[FAILED] " << testName << std::endl;
    std::cout << "  Expected: " << expected << std::endl;
    std::cout << "  Actual: " << actual << std::endl;
}
```

## Performance Testing Standards

### Timing Measurements

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

### Performance Test Format

```cpp
bool TestPerformance() {
    std::cout << "Testing performance..." << std::endl;

    TestTimer timer;

    // Perform operations
    for (int i = 0; i < 10000; ++i) {
        // Test operation
    }

    double elapsed = timer.ElapsedMs();
    std::cout << "  [INFO] Completed 10000 operations in " << elapsed << "ms" << std::endl;

    // Validate performance threshold
    if (elapsed < 100.0) {
        std::cout << "  [PASS] Performance test passed" << std::endl;
        return true;
    } else {
        std::cout << "  [FAILED] Performance test failed (too slow)" << std::endl;
        return false;
    }
}
```

## Documentation Standards

### Test Documentation Requirements

1. **File Headers**: Include purpose and component being tested
2. **Function Comments**: Describe what each test validates
3. **Complex Logic**: Comment non-obvious test logic
4. **Requirements Traceability**: Reference requirements in comments

### Comment Format

```cpp
/**
 * Tests vector addition operations for correctness and edge cases.
 * Requirements: 6.1, 6.2 (Math operations validation)
 */
bool TestVectorAddition() {
    // Implementation
}
```

## Windows Platform Considerations

### Windows-Specific Code

```cpp
#ifdef _WIN32
    // Windows-specific test code
    #include <windows.h>
    // Use Windows APIs as needed
#endif
```

### Encoding and Output

1. **Use UTF-8**: Ensure proper encoding for all text output
2. **Avoid Platform-Specific Characters**: Stick to ASCII for compatibility
3. **Line Endings**: Let CMake handle platform-specific line endings

## Build Integration Standards

### CMake Test Target Pattern

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

### Test Discovery Pattern

Tests should be automatically discoverable through CMake configuration without manual registration.

## Quality Assurance

### Code Review Checklist

- [ ] Follows naming conventions
- [ ] Uses standard output formatting
- [ ] Includes proper error handling
- [ ] Has appropriate test coverage
- [ ] Includes performance considerations
- [ ] Windows compatible
- [ ] Properly documented

### Automated Checks

1. **Output Format Validation**: Automated checks for consistent prefixes
2. **Performance Regression**: Automated performance threshold validation
3. **Windows Testing**: Validation on Windows platform

## Maintenance Guidelines

### Adding New Tests

1. Follow the established template
2. Use consistent naming patterns
3. Include in CMake configuration
4. Add to documentation
5. Validate Windows compatibility

### Updating Existing Tests

1. Maintain backward compatibility
2. Update documentation
3. Validate performance impact
4. Test on all platforms

This document serves as the authoritative guide for all testing code in Game Engine Kiro. Adherence to these standards ensures maintainable, professional, and reliable test code.
