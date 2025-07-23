# Test Output Consistency Guidelines

## Overview

This guide provides specific guidelines for maintaining consistent test output formatting across different test types in Game Engine Kiro. It complements the [Test Output Formatting Standards](testing-output-formatting.md) by focusing on consistency patterns and common scenarios.

## Consistency Across Test Types

### Naming Consistency

#### Component Names in Headers

Use consistent component names across all test types:

```cpp
// Unit test
TestOutput::PrintHeader("Math");

// Integration test
TestOutput::PrintHeader("Math Integration");

// Performance test
TestOutput::PrintHeader("Math Performance");
```

#### Feature Names in Tests

Use the same base feature name across different test types:

```cpp
// Unit test
TestOutput::PrintTestStart("vector addition");

// Integration test
TestOutput::PrintTestStart("vector addition integration");

// Performance test
TestOutput::PrintTestStart("vector addition performance");
```

### Output Volume Consistency

#### Unit Tests - Minimal Output

```cpp
bool TestVectorAddition() {
    TestOutput::PrintTestStart("vector addition");

    // Test logic with assertions only
    Math::Vec3 result = a + b;
    EXPECT_NEAR_VEC3(result, expected);

    TestOutput::PrintTestPass("vector addition");
    return true;
}
```

#### Integration Tests - Context-Aware Output

```cpp
bool TestPhysicsIntegration() {
    TestOutput::PrintTestStart("physics integration");

    if (!PhysicsEngine::IsAvailable()) {
        TestOutput::PrintWarning("Physics engine not available, using mock");
    }

    // Test logic
    EXPECT_TRUE(world->Initialize());

    TestOutput::PrintTestPass("physics integration");
    return true;
}
```

#### Performance Tests - Timing Output

```cpp
bool TestVectorPerformance() {
    TestOutput::PrintTestStart("vector performance");

    bool result = PerformanceTest::ValidatePerformance(
        "vector operations", testFunction, 1.0, 1000);

    EXPECT_TRUE(result);

    TestOutput::PrintTestPass("vector performance");
    return result;
}
```

## Error Reporting Consistency

### Assertion Failures

All test types should use the same assertion macros and error reporting:

```cpp
// Consistent across all test types
EXPECT_TRUE(condition);
EXPECT_NEARLY_EQUAL(actual, expected);
EXPECT_NEAR_VEC3(actualVec, expectedVec);
```

### Exception Handling

Use identical exception handling patterns:

```cpp
// Standard pattern for all test types
try {
    // Test implementation

} catch (const std::exception& e) {
    TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
    return 1;
} catch (...) {
    TestOutput::PrintError("UNKNOWN TEST ERROR!");
    return 1;
}
```

### Custom Error Messages

Format custom error messages consistently:

```cpp
// Unit test error
TestOutput::PrintTestFail("vector addition", "valid result", "null result");

// Integration test error
TestOutput::PrintTestFail("physics integration", "initialized world", "failed initialization");

// Performance test error
TestOutput::PrintTestFail("vector performance", "< 1.0ms", "1.23ms");
```

## Information Output Guidelines

### Context Information

#### When to Use PrintInfo()

Use `PrintInfo()` for essential context that affects test behavior:

```cpp
// ✅ GOOD - Affects test behavior
TestOutput::PrintInfo("Using fallback renderer (OpenGL unavailable)");
TestOutput::PrintInfo("Testing with 10000 iterations");
TestOutput::PrintInfo("Skipping GPU tests (no context)");

// ❌ BAD - Unnecessary noise
TestOutput::PrintInfo("Starting test execution");
TestOutput::PrintInfo("Test completed successfully");
TestOutput::PrintInfo("Cleaning up resources");
```

#### Information Consistency Across Types

Use similar information patterns for similar situations:

```cpp
// Unit test - minimal context
if (!OpenGLContext::HasActiveContext()) {
    TestOutput::PrintInfo("Skipping OpenGL-dependent validation");
}

// Integration test - more context
if (!OpenGLContext::HasActiveContext()) {
    TestOutput::PrintWarning("OpenGL context unavailable, using mock resources");
}

// Performance test - performance context
if (isDebugBuild) {
    TestOutput::PrintWarning("Performance thresholds adjusted for debug build");
}
```

### Warning Consistency

Use warnings for non-fatal issues that may affect results:

```cpp
// Consistent warning patterns
TestOutput::PrintWarning("OpenGL context not available");
TestOutput::PrintWarning("Using fallback implementation");
TestOutput::PrintWarning("Performance threshold adjusted");
TestOutput::PrintWarning("Test data not found, using defaults");
```

## Test Suite Structure Consistency

### Main Function Pattern

All test types must use the identical main function structure:

```cpp
int main() {
    TestOutput::PrintHeader("[Component] [Type]");

    bool allPassed = true;

    try {
        TestSuite suite("[Component] [Type] Tests");

        // Test execution
        allPassed &= suite.RunTest("Test Name", TestFunction);

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

### TestSuite Usage

Use consistent TestSuite naming patterns:

```cpp
// Unit tests
TestSuite suite("Math Tests");
TestSuite suite("Physics Tests");
TestSuite suite("Resource Tests");

// Integration tests
TestSuite suite("Math Integration Tests");
TestSuite suite("Physics Integration Tests");
TestSuite suite("Resource Integration Tests");

// Performance tests
TestSuite suite("Math Performance Tests");
TestSuite suite("Physics Performance Tests");
TestSuite suite("Resource Performance Tests");
```

## Windows Platform Consistency

### Windows Platform Detection

Use consistent patterns for Windows-specific behavior:

```cpp
bool TestWindowsFeature() {
    TestOutput::PrintTestStart("windows feature");

#ifdef _WIN32
    TestOutput::PrintInfo("Running Windows-specific validation");
    // Windows test code
    EXPECT_TRUE(WindowsSpecificFunction());
#else
    TestOutput::PrintWarning("Windows-specific test skipped on non-Windows platform");
    TestOutput::PrintTestPass("windows feature");
    return true;
#endif

    TestOutput::PrintTestPass("windows feature");
    return true;
}
```

### Build Configuration

Handle build configurations consistently:

```cpp
bool TestPerformanceFeature() {
    TestOutput::PrintTestStart("performance feature");

#ifdef _DEBUG
    TestOutput::PrintWarning("Debug build detected, adjusting thresholds");
    double threshold = 10.0; // More lenient for debug
#else
    double threshold = 1.0;  // Strict for release
#endif

    bool result = PerformanceTest::ValidatePerformance(
        "operation", testFunc, threshold, 1000);

    EXPECT_TRUE(result);

    TestOutput::PrintTestPass("performance feature");
    return result;
}
```

## File Organization Consistency

### File Naming

Use consistent file naming patterns:

```
tests/unit/test_math.cpp
tests/unit/test_physics.cpp
tests/unit/test_resource.cpp

tests/integration/test_math_integration.cpp
tests/integration/test_physics_integration.cpp
tests/integration/test_resource_integration.cpp

tests/performance/test_math_performance.cpp
tests/performance/test_physics_performance.cpp
tests/performance/test_resource_performance.cpp
```

### Header Includes

Use consistent include patterns:

```cpp
// Standard includes for all test types
#include "TestUtils.h"
#include "Core/ComponentToTest.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
```

### Function Organization

Organize test functions consistently:

```cpp
// 1. Helper functions (if needed)
bool IsValidResult(const Result& result) { /* ... */ }

// 2. Test functions in logical order
bool TestBasicFunctionality() { /* ... */ }
bool TestEdgeCases() { /* ... */ }
bool TestErrorHandling() { /* ... */ }

// 3. Main function
int main() { /* ... */ }
```

## Documentation Consistency

### Function Comments

Use consistent comment patterns:

```cpp
/**
 * Test [feature description]
 * Requirements: X.X, Y.Y ([requirement description])
 */
bool TestFeatureName() {
    // Implementation
}
```

### Test Descriptions

Write consistent test descriptions:

```cpp
// ✅ GOOD - Clear and consistent
bool TestVectorAddition() { /* ... */ }
bool TestVectorSubtraction() { /* ... */ }
bool TestVectorMultiplication() { /* ... */ }

// ❌ BAD - Inconsistent naming
bool TestVectorAdd() { /* ... */ }
bool TestSubtractVectors() { /* ... */ }
bool TestMultiply() { /* ... */ }
```

## Quality Assurance Checklist

### Pre-Commit Checklist

- [ ] Uses only `TestOutput::Print*` methods
- [ ] Consistent test naming (lowercase with spaces)
- [ ] Matching strings in `PrintTestStart()` and `PrintTestPass()`
- [ ] Minimal use of `PrintInfo()` (only for essential context)
- [ ] Standard main function structure
- [ ] Proper exception handling
- [ ] Consistent file naming and organization
- [ ] Appropriate test type classification

### Review Checklist

- [ ] Output format matches established patterns
- [ ] Error messages are clear and helpful
- [ ] Information output is justified and minimal
- [ ] Platform-specific code is handled consistently
- [ ] Performance tests include proper timing output
- [ ] Integration tests handle missing dependencies gracefully
- [ ] All tests follow the same structural patterns

### Automated Validation

Future tooling should check for:

1. **No direct console output**: Scan for `std::cout`, `printf`, `std::cerr`
2. **Consistent naming**: Verify matching Start/Pass strings
3. **Proper indentation**: Check output formatting
4. **Required patterns**: Ensure standard main function structure
5. **Information usage**: Flag excessive `PrintInfo()` usage
6. **Error handling**: Verify exception handling patterns

## Migration Strategy

### Updating Existing Tests

1. **Replace direct output**: Convert all `std::cout` to `TestOutput::Print*`
2. **Standardize naming**: Update test names to lowercase with spaces
3. **Reduce noise**: Remove unnecessary `PrintInfo()` calls
4. **Update structure**: Ensure main function follows standard pattern
5. **Validate consistency**: Check against this guide

### Gradual Migration

- **Phase 1**: Update critical test files first
- **Phase 2**: Update by component (Math, Physics, etc.)
- **Phase 3**: Update remaining files
- **Phase 4**: Implement automated validation

This guide ensures that all tests in Game Engine Kiro maintain consistent, professional output formatting regardless of test type or component being tested.
