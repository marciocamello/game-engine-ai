# Game Engine Kiro - Test Output Formatting Standards

## Overview

This document establishes comprehensive standards for test output formatting in Game Engine Kiro. Consistent output formatting ensures professional presentation, easier debugging, and maintainable test code across all engine components.

## Core Principles

1. **Consistency**: All tests must use identical formatting patterns
2. **Readability**: Output should be clear and easy to parse both visually and programmatically
3. **Professional Appearance**: Output should reflect the quality of the engine
4. **Windows Compatibility**: Use only ASCII characters for maximum compatibility
5. **Minimal Noise**: Avoid excessive output that clutters test results

## Related Documentation

**Implementation Guides:**

- **[Testing Guide](testing-guide.md)**: Comprehensive guide with proper output formatting examples
- **[Testing Guidelines](testing-guidelines.md)**: Guidelines that reference these formatting standards
- **[Testing Standards](testing-standards.md)**: Coding standards that enforce these formatting rules

**Consistency and Quality:**

- **[Test Output Consistency](testing-output-consistency-guide.md)**: Detailed consistency guidelines
- **[Code Examples Validation](testing-code-examples-validation.md)**: Validating output formatting in examples

**Specialized Testing:**

- **[OpenGL Context Limitations](testing-opengl-limitations.md)**: Context-aware output formatting
- **[Resource Testing Patterns](testing-resource-patterns.md)**: Resource test output formatting
- **[Mock Resource Implementation](testing-mock-resources.md)**: Mock resource test output

**Migration and Strategy:**

- **[Testing Migration](testing-migration.md)**: Migrating tests to new output standards
- **[Testing Strategy](testing-strategy.md)**: Strategic approach to consistent output

## TestOutput Method Usage Patterns

### Required Methods

All tests MUST use the `TestOutput` class methods exclusively. Direct `std::cout` usage is prohibited in test files.

#### Basic Test Flow Pattern

```cpp
bool TestFeatureName() {
    TestOutput::PrintTestStart("feature name");

    // Test implementation
    // ... test logic ...

    TestOutput::PrintTestPass("feature name");
    return true;
}
```

#### Complete Method Reference

| Method                                       | Usage                      | Example                                                         |
| -------------------------------------------- | -------------------------- | --------------------------------------------------------------- |
| `PrintHeader(name)`                          | Test suite header          | `TestOutput::PrintHeader("Math");`                              |
| `PrintFooter(passed)`                        | Test suite footer          | `TestOutput::PrintFooter(allPassed);`                           |
| `PrintTestStart(name)`                       | Start of individual test   | `TestOutput::PrintTestStart("vector operations");`              |
| `PrintTestPass(name)`                        | Successful test completion | `TestOutput::PrintTestPass("vector operations");`               |
| `PrintTestFail(name)`                        | Failed test (basic)        | `TestOutput::PrintTestFail("vector operations");`               |
| `PrintTestFail(name, expected, actual)`      | Failed test with details   | `TestOutput::PrintTestFail("vector operations", "1.0", "1.1");` |
| `PrintInfo(message)`                         | Informational output       | `TestOutput::PrintInfo("Using fallback implementation");`       |
| `PrintWarning(message)`                      | Warning conditions         | `TestOutput::PrintWarning("OpenGL context not available");`     |
| `PrintError(message)`                        | Error conditions           | `TestOutput::PrintError("Exception: " + e.what());`             |
| `PrintTiming(operation, timeMs, iterations)` | Performance results        | `TestOutput::PrintTiming("vector addition", 1.23, 1000);`       |

### Naming Conventions

#### Test Names in Output

- **Format**: lowercase with spaces
- **Style**: descriptive but concise
- **Examples**:
  - ✅ `"vector operations"`
  - ✅ `"matrix multiplication"`
  - ✅ `"resource loading"`
  - ❌ `"Vector Operations"` (PascalCase)
  - ❌ `"vectorOps"` (camelCase)
  - ❌ `"test_vector_operations"` (underscores)

#### Consistency Rule

The same string MUST be used for both `PrintTestStart()` and `PrintTestPass()`:

```cpp
// ✅ CORRECT
TestOutput::PrintTestStart("quaternion normalization");
// ... test logic ...
TestOutput::PrintTestPass("quaternion normalization");

// ❌ INCORRECT - Different strings
TestOutput::PrintTestStart("quaternion normalization");
// ... test logic ...
TestOutput::PrintTestPass("Quaternion normalization test passed");
```

## Standard Output Structure

### Test Suite Format

```
========================================
 Game Engine Kiro - [Component] Tests
========================================
Testing [feature 1]...
  [PASS] [feature 1] passed
Testing [feature 2]...
  [PASS] [feature 2] passed
  [INFO] Test Summary:
  [INFO]   Total: 2
  [INFO]   Passed: 2
  [INFO]   Failed: 0
  [INFO]   Total Time: 1.234ms
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

### Individual Test Format

```cpp
bool TestExample() {
    TestOutput::PrintTestStart("example feature");

    // Setup phase - minimal output
    Component component;

    // Execution phase - no output unless necessary
    auto result = component.DoSomething();

    // Validation phase - use assertions (no direct output)
    EXPECT_TRUE(result.IsValid());
    EXPECT_NEARLY_EQUAL(result.GetValue(), 1.0f);

    TestOutput::PrintTestPass("example feature");
    return true;
}
```

### Error Reporting Format

```cpp
bool TestWithErrorHandling() {
    TestOutput::PrintTestStart("error handling");

    try {
        // Test implementation
        Component component;
        auto result = component.RiskyOperation();

        EXPECT_TRUE(result.IsValid());

    } catch (const std::exception& e) {
        TestOutput::PrintError("Unexpected exception: " + std::string(e.what()));
        return false;
    }

    TestOutput::PrintTestPass("error handling");
    return true;
}
```

## Status Prefixes and Formatting

### Required Status Prefixes

| Prefix      | Usage              | Color (if supported) | Format                         |
| ----------- | ------------------ | -------------------- | ------------------------------ |
| `[PASS]`    | Test success       | Green                | `  [PASS] test name passed`    |
| `[FAILED]`  | Test failure       | Red                  | `  [FAILED] test name failed`  |
| `[SUCCESS]` | Suite success      | Green                | `[SUCCESS] ALL TESTS PASSED!`  |
| `[ERROR]`   | Error conditions   | Red                  | `  [ERROR] error message`      |
| `[WARNING]` | Warning conditions | Yellow               | `  [WARNING] warning message`  |
| `[INFO]`    | Information        | Blue                 | `  [INFO] information message` |

### Indentation Rules

- **Main headers**: No indentation
- **Test progress**: No indentation (`Testing feature...`)
- **Test results**: 2-space indentation (`  [PASS] feature passed`)
- **Additional info**: 2-space indentation (`  [INFO] message`)
- **Error details**: 4-space indentation (`    Expected: value`)

### Separator Usage

- **Major sections**: `========================================` (40 characters)
- **No other separators**: Avoid custom separators or decorative elements

## Performance Testing Output

### Timing Output Format

```cpp
bool TestPerformance() {
    TestOutput::PrintTestStart("performance test");

    // Use PerformanceTest utility
    bool result = PerformanceTest::ValidatePerformance(
        "vector operations",
        [&]() {
            // Performance test code
        },
        1.0, // 1ms threshold
        1000 // iterations
    );

    EXPECT_TRUE(result);

    TestOutput::PrintTestPass("performance test");
    return result;
}
```

**Expected Output:**

```
Testing performance test...
  [INFO] vector operations completed in 0.856ms (1000 iterations, 0.000856ms per iteration)
  [PASS] performance test passed
```

### Performance Failure Format

```
Testing performance test...
  [INFO] vector operations completed in 1.234ms (1000 iterations, 0.001234ms per iteration)
  [FAILED] vector operations failed
    Expected: < 1.000ms per iteration
    Actual: 1.234ms per iteration
```

## Information Output Guidelines

### When to Use PrintInfo()

Use `PrintInfo()` sparingly and only for:

1. **Context-dependent behavior**:

   ```cpp
   TestOutput::PrintInfo("Skipping OpenGL tests (no context available)");
   ```

2. **Performance metrics**:

   ```cpp
   TestOutput::PrintInfo("Processed 1000 vertices in 2.3ms");
   ```

3. **Test configuration**:
   ```cpp
   TestOutput::PrintInfo("Using Bullet Physics backend");
   ```

### When NOT to Use PrintInfo()

Avoid `PrintInfo()` for:

- ❌ Test progress updates (`"Loading test data..."`)
- ❌ Verbose debugging (`"Setting up component..."`)
- ❌ Redundant information (`"Test completed successfully"`)
- ❌ Implementation details (`"Calling Initialize() method"`)

### Warning and Error Guidelines

#### PrintWarning() Usage

```cpp
// ✅ CORRECT - Non-fatal issues that affect test behavior
TestOutput::PrintWarning("OpenGL context not available, using mock resources");
TestOutput::PrintWarning("Performance threshold adjusted for debug build");

// ❌ INCORRECT - Use PrintInfo() instead
TestOutput::PrintWarning("Test data loaded successfully");
```

#### PrintError() Usage

```cpp
// ✅ CORRECT - Actual error conditions
TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
TestOutput::PrintError("Failed to initialize required component");

// ❌ INCORRECT - Use PrintTestFail() instead
TestOutput::PrintError("Vector comparison failed");
```

## Test Result Reporting Structure

### Basic Success Pattern

```cpp
bool TestBasicFeature() {
    TestOutput::PrintTestStart("basic feature");

    // Test implementation with assertions
    EXPECT_TRUE(condition);
    EXPECT_NEARLY_EQUAL(actual, expected);

    TestOutput::PrintTestPass("basic feature");
    return true;
}
```

### Failure with Details Pattern

```cpp
bool TestComplexFeature() {
    TestOutput::PrintTestStart("complex feature");

    Component component;
    auto result = component.ComplexOperation();

    if (!result.IsValid()) {
        TestOutput::PrintTestFail("complex feature",
                                 "valid result",
                                 "invalid result: " + result.GetErrorMessage());
        return false;
    }

    TestOutput::PrintTestPass("complex feature");
    return true;
}
```

### Exception Handling Pattern

```cpp
bool TestWithExceptions() {
    TestOutput::PrintTestStart("exception handling");

    try {
        // Test code that might throw
        Component component;
        component.RiskyOperation();

        TestOutput::PrintTestPass("exception handling");
        return true;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Unexpected exception: " + std::string(e.what()));
        return false;
    }
}
```

## Main Function Structure

### Standard Template

```cpp
int main() {
    TestOutput::PrintHeader("Component Name");

    bool allPassed = true;

    try {
        TestSuite suite("Component Tests");

        // Run all tests
        allPassed &= suite.RunTest("Feature 1", TestFeature1);
        allPassed &= suite.RunTest("Feature 2", TestFeature2);
        allPassed &= suite.RunTest("Feature 3", TestFeature3);

        // Print summary
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

### Exception Handling Requirements

All test main functions MUST include:

1. **Standard exception catch**: `catch (const std::exception& e)`
2. **Unknown exception catch**: `catch (...)`
3. **Proper error reporting**: Use `TestOutput::PrintError()`
4. **Correct exit codes**: Return 0 for success, 1 for failure

## Output Consistency Across Test Types

### Unit Tests

```cpp
// File: tests/unit/test_math.cpp
int main() {
    TestOutput::PrintHeader("Math");
    // ... test implementation
}
```

**Expected Output:**

```
========================================
 Game Engine Kiro - Math Tests
========================================
```

### Integration Tests

```cpp
// File: tests/integration/test_physics_integration.cpp
int main() {
    TestOutput::PrintHeader("Physics Integration");
    // ... test implementation
}
```

**Expected Output:**

```
========================================
 Game Engine Kiro - Physics Integration Tests
========================================
```

### Performance Tests

```cpp
// File: tests/performance/test_math_performance.cpp
int main() {
    TestOutput::PrintHeader("Math Performance");
    // ... test implementation
}
```

**Expected Output:**

```
========================================
 Game Engine Kiro - Math Performance Tests
========================================
```

## Common Anti-Patterns to Avoid

### Direct Console Output

```cpp
// ❌ NEVER DO THIS
std::cout << "Testing feature..." << std::endl;
printf("Test result: %d\n", result);
std::cerr << "Error occurred" << std::endl;

// ✅ ALWAYS DO THIS
TestOutput::PrintTestStart("feature");
TestOutput::PrintInfo("Test result: " + std::to_string(result));
TestOutput::PrintError("Error occurred");
```

### Inconsistent Naming

```cpp
// ❌ INCONSISTENT
TestOutput::PrintTestStart("vector operations");
TestOutput::PrintTestPass("Vector Operations Test");

// ✅ CONSISTENT
TestOutput::PrintTestStart("vector operations");
TestOutput::PrintTestPass("vector operations");
```

### Excessive Information Output

```cpp
// ❌ TOO VERBOSE
TestOutput::PrintInfo("Initializing component...");
TestOutput::PrintInfo("Setting up test data...");
TestOutput::PrintInfo("Running test logic...");
TestOutput::PrintInfo("Validating results...");
TestOutput::PrintInfo("Cleaning up...");

// ✅ MINIMAL AND FOCUSED
TestOutput::PrintTestStart("component functionality");
// Test logic without excessive output
TestOutput::PrintTestPass("component functionality");
```

### Custom Formatting

```cpp
// ❌ CUSTOM FORMATTING
std::cout << "*** RUNNING TEST ***" << std::endl;
std::cout << ">>> Feature test passed <<<" << std::endl;

// ✅ STANDARD FORMATTING
TestOutput::PrintTestStart("feature test");
TestOutput::PrintTestPass("feature test");
```

## Validation and Quality Assurance

### Automated Checks

Future tooling should validate:

1. **No direct console output**: Scan for `std::cout`, `printf`, `std::cerr`
2. **Consistent naming**: Verify matching strings in Start/Pass calls
3. **Proper indentation**: Check output formatting compliance
4. **Required methods**: Ensure all tests use TestOutput methods

### Manual Review Checklist

- [ ] Uses only `TestOutput::Print*` methods
- [ ] Consistent test names in Start/Pass calls
- [ ] Proper lowercase naming convention
- [ ] Minimal use of `PrintInfo()`
- [ ] Appropriate error handling with `PrintError()`
- [ ] Standard main function structure
- [ ] Proper exception handling

### Testing the Output Format

To verify output formatting compliance:

```cmd
# Run a test and check output format
.\build\Release\MathTest.exe > test_output.txt

# Verify format matches standards
# - Check for proper headers/footers
# - Verify status prefixes
# - Confirm indentation
# - Validate no direct console output
```

## Migration Guide for Existing Tests

### Step 1: Replace Direct Console Output

```cpp
// Before
std::cout << "Testing feature..." << std::endl;

// After
TestOutput::PrintTestStart("feature");
```

### Step 2: Standardize Test Names

```cpp
// Before
TestOutput::PrintTestStart("Vector Operations");
TestOutput::PrintTestPass("Vector operations test completed");

// After
TestOutput::PrintTestStart("vector operations");
TestOutput::PrintTestPass("vector operations");
```

### Step 3: Reduce Information Output

```cpp
// Before
TestOutput::PrintInfo("Setting up test data");
TestOutput::PrintInfo("Running calculations");
TestOutput::PrintInfo("Validating results");

// After
TestOutput::PrintTestStart("calculations");
// Test logic without excessive output
TestOutput::PrintTestPass("calculations");
```

### Step 4: Update Exception Handling

```cpp
// Before
try {
    // test code
} catch (...) {
    std::cout << "Error!" << std::endl;
}

// After
try {
    // test code
} catch (const std::exception& e) {
    TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
    return 1;
} catch (...) {
    TestOutput::PrintError("UNKNOWN TEST ERROR!");
    return 1;
}
```

## Summary

This document establishes the definitive standards for test output formatting in Game Engine Kiro. Adherence to these standards ensures:

- **Professional presentation** of test results
- **Consistent experience** across all test types
- **Easy debugging** with clear, structured output
- **Maintainable test code** with standardized patterns
- **Windows compatibility** with ASCII-only output

All new tests MUST follow these standards, and existing tests should be migrated to comply with these guidelines during regular maintenance cycles.

## Complete Examples

### Basic Unit Test Example

```cpp
// File: tests/unit/test_vector.cpp
#include "TestUtils.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestVectorAddition() {
    TestOutput::PrintTestStart("vector addition");

    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 result = a + b;
    Math::Vec3 expected(5.0f, 7.0f, 9.0f);

    EXPECT_NEAR_VEC3(result, expected);

    TestOutput::PrintTestPass("vector addition");
    return true;
}

bool TestVectorNormalization() {
    TestOutput::PrintTestStart("vector normalization");

    Math::Vec3 vector(3.0f, 4.0f, 0.0f);
    Math::Vec3 normalized = glm::normalize(vector);

    EXPECT_NEARLY_EQUAL(glm::length(normalized), 1.0f);
    EXPECT_NEARLY_EQUAL(normalized.x, 0.6f);
    EXPECT_NEARLY_EQUAL(normalized.y, 0.8f);
    EXPECT_NEARLY_EQUAL(normalized.z, 0.0f);

    TestOutput::PrintTestPass("vector normalization");
    return true;
}

int main() {
    TestOutput::PrintHeader("Vector");

    bool allPassed = true;

    try {
        TestSuite suite("Vector Tests");

        allPassed &= suite.RunTest("Vector Addition", TestVectorAddition);
        allPassed &= suite.RunTest("Vector Normalization", TestVectorNormalization);

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

**Expected Output:**

```
========================================
 Game Engine Kiro - Vector Tests
========================================
Testing vector addition...
  [PASS] vector addition passed
Testing vector normalization...
  [PASS] vector normalization passed
  [INFO] Test Summary:
  [INFO]   Total: 2
  [INFO]   Passed: 2
  [INFO]   Failed: 0
  [INFO]   Total Time: 0.123ms
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

### Integration Test with Context Awareness

```cpp
// File: tests/integration/test_resource_loading.cpp
#include "TestUtils.h"
#include "Resource/ResourceManager.h"
#include "Graphics/OpenGLContext.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestResourceManagerInitialization() {
    TestOutput::PrintTestStart("resource manager initialization");

    ResourceManager manager;
    EXPECT_TRUE(manager.Initialize());
    EXPECT_EQUAL(manager.GetResourceCount(), 0);
    EXPECT_EQUAL(manager.GetMemoryUsage(), 0);

    TestOutput::PrintTestPass("resource manager initialization");
    return true;
}

bool TestTextureLoadingWithContext() {
    TestOutput::PrintTestStart("texture loading with context");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintWarning("OpenGL context not available, using mock resources");
    }

    ResourceManager manager;
    manager.Initialize();

    auto texture = manager.Load<Texture>("textures/test.png");
    EXPECT_NOT_NULL(texture);
    EXPECT_TRUE(texture->GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("texture loading with context");
    return true;
}

bool TestResourceCaching() {
    TestOutput::PrintTestStart("resource caching");

    ResourceManager manager;
    manager.Initialize();

    // Load same resource twice
    auto texture1 = manager.Load<Texture>("textures/test.png");
    auto texture2 = manager.Load<Texture>("textures/test.png");

    // Should be the same instance (cached)
    EXPECT_TRUE(texture1.get() == texture2.get());
    EXPECT_EQUAL(manager.GetResourceCount(), 1);

    TestOutput::PrintTestPass("resource caching");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Loading Integration");

    bool allPassed = true;

    try {
        TestSuite suite("Resource Loading Integration Tests");

        allPassed &= suite.RunTest("Resource Manager Initialization", TestResourceManagerInitialization);
        allPassed &= suite.RunTest("Texture Loading", TestTextureLoadingWithContext);
        allPassed &= suite.RunTest("Resource Caching", TestResourceCaching);

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

**Expected Output:**

```
========================================
 Game Engine Kiro - Resource Loading Integration Tests
========================================
Testing resource manager initialization...
  [PASS] resource manager initialization passed
Testing texture loading with context...
  [WARNING] OpenGL context not available, using mock resources
  [PASS] texture loading with context passed
Testing resource caching...
  [PASS] resource caching passed
  [INFO] Test Summary:
  [INFO]   Total: 3
  [INFO]   Passed: 3
  [INFO]   Failed: 0
  [INFO]   Total Time: 2.456ms
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

### Performance Test Example

```cpp
// File: tests/performance/test_math_performance.cpp
#include "TestUtils.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestVectorOperationPerformance() {
    TestOutput::PrintTestStart("vector operation performance");

    // Test vector addition performance
    bool additionTest = PerformanceTest::ValidatePerformance(
        "vector addition",
        []() {
            Math::Vec3 a(1.0f, 2.0f, 3.0f);
            Math::Vec3 b(4.0f, 5.0f, 6.0f);
            volatile Math::Vec3 result = a + b; // volatile to prevent optimization
        },
        0.001, // 1 microsecond threshold
        10000  // 10k iterations
    );

    EXPECT_TRUE(additionTest);

    // Test vector normalization performance
    bool normalizationTest = PerformanceTest::ValidatePerformance(
        "vector normalization",
        []() {
            Math::Vec3 vector(3.0f, 4.0f, 5.0f);
            volatile Math::Vec3 result = glm::normalize(vector);
        },
        0.01,  // 10 microseconds threshold
        1000   // 1k iterations
    );

    EXPECT_TRUE(normalizationTest);

    TestOutput::PrintTestPass("vector operation performance");
    return additionTest && normalizationTest;
}

bool TestMatrixMultiplicationPerformance() {
    TestOutput::PrintTestStart("matrix multiplication performance");

    bool multiplicationTest = PerformanceTest::ValidatePerformance(
        "matrix multiplication",
        []() {
            Math::Mat4 a = glm::mat4(1.0f);
            Math::Mat4 b = glm::rotate(glm::mat4(1.0f), 0.1f, Math::Vec3(0, 1, 0));
            volatile Math::Mat4 result = a * b;
        },
        0.1,   // 100 microseconds threshold
        1000   // 1k iterations
    );

    EXPECT_TRUE(multiplicationTest);

    TestOutput::PrintTestPass("matrix multiplication performance");
    return multiplicationTest;
}

int main() {
    TestOutput::PrintHeader("Math Performance");

    bool allPassed = true;

    try {
        TestSuite suite("Math Performance Tests");

        allPassed &= suite.RunTest("Vector Operations", TestVectorOperationPerformance);
        allPassed &= suite.RunTest("Matrix Multiplication", TestMatrixMultiplicationPerformance);

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

**Expected Output:**

```
========================================
 Game Engine Kiro - Math Performance Tests
========================================
Testing vector operation performance...
  [INFO] vector addition completed in 8.234ms (10000 iterations, 0.000823ms per iteration)
  [PASS] vector addition passed
  [INFO] vector normalization completed in 9.876ms (1000 iterations, 0.009876ms per iteration)
  [PASS] vector normalization passed
  [PASS] vector operation performance passed
Testing matrix multiplication performance...
  [INFO] matrix multiplication completed in 45.123ms (1000 iterations, 0.045123ms per iteration)
  [PASS] matrix multiplication passed
  [PASS] matrix multiplication performance passed
  [INFO] Test Summary:
  [INFO]   Total: 2
  [INFO]   Passed: 2
  [INFO]   Failed: 0
  [INFO]   Total Time: 63.233ms
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

### Test with Failure Example

```cpp
// File: tests/unit/test_failure_example.cpp
#include "TestUtils.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestVectorComparison() {
    TestOutput::PrintTestStart("vector comparison");

    Math::Vec3 actual(1.0f, 2.0f, 3.001f);  // Slightly off
    Math::Vec3 expected(1.0f, 2.0f, 3.0f);

    EXPECT_NEAR_VEC3(actual, expected);  // This will fail

    TestOutput::PrintTestPass("vector comparison");
    return true;
}

bool TestDivisionByZero() {
    TestOutput::PrintTestStart("division by zero handling");

    try {
        float result = 10.0f / 0.0f;  // This should be handled

        if (std::isinf(result)) {
            TestOutput::PrintInfo("Division by zero correctly produces infinity");
        } else {
            TestOutput::PrintTestFail("division by zero handling", "infinity", std::to_string(result));
            return false;
        }

    } catch (const std::exception& e) {
        TestOutput::PrintError("Unexpected exception: " + std::string(e.what()));
        return false;
    }

    TestOutput::PrintTestPass("division by zero handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Failure Example");

    bool allPassed = true;

    try {
        TestSuite suite("Failure Example Tests");

        allPassed &= suite.RunTest("Vector Comparison", TestVectorComparison);
        allPassed &= suite.RunTest("Division by Zero", TestDivisionByZero);

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

**Expected Output (with failure):**

```
========================================
 Game Engine Kiro - Failure Example Tests
========================================
Testing vector comparison...
  [FAILED] TestVectorComparison failed
    Vec3 comparison failed
    Expected: (1.000, 2.000, 3.000)
    Actual: (1.000, 2.000, 3.001)
    Location: test_failure_example.cpp:12
Testing division by zero handling...
  [INFO] Division by zero correctly produces infinity
  [PASS] division by zero handling passed
  [INFO] Test Summary:
  [INFO]   Total: 2
  [INFO]   Passed: 1
  [INFO]   Failed: 1
  [INFO]   Total Time: 0.234ms
========================================
[FAILED] SOME TESTS FAILED!
========================================
```

### Exception Handling Example

```cpp
// File: tests/unit/test_exception_handling.cpp
#include "TestUtils.h"
#include <stdexcept>

using namespace GameEngine::Testing;

bool TestExpectedException() {
    TestOutput::PrintTestStart("expected exception");

    try {
        throw std::runtime_error("Expected test exception");

        // Should not reach here
        TestOutput::PrintTestFail("expected exception", "exception thrown", "no exception");
        return false;

    } catch (const std::runtime_error& e) {
        TestOutput::PrintInfo("Caught expected exception: " + std::string(e.what()));
    } catch (const std::exception& e) {
        TestOutput::PrintError("Unexpected exception type: " + std::string(e.what()));
        return false;
    }

    TestOutput::PrintTestPass("expected exception");
    return true;
}

bool TestUnexpectedException() {
    TestOutput::PrintTestStart("unexpected exception handling");

    try {
        // This should not throw
        int result = 2 + 2;
        EXPECT_EQUAL(result, 4);

    } catch (const std::exception& e) {
        TestOutput::PrintError("Unexpected exception: " + std::string(e.what()));
        return false;
    }

    TestOutput::PrintTestPass("unexpected exception handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Exception Handling");

    bool allPassed = true;

    try {
        TestSuite suite("Exception Handling Tests");

        allPassed &= suite.RunTest("Expected Exception", TestExpectedException);
        allPassed &= suite.RunTest("Unexpected Exception", TestUnexpectedException);

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

**Expected Output:**

```
========================================
 Game Engine Kiro - Exception Handling Tests
========================================
Testing expected exception...
  [INFO] Caught expected exception: Expected test exception
  [PASS] expected exception passed
Testing unexpected exception handling...
  [PASS] unexpected exception handling passed
  [INFO] Test Summary:
  [INFO]   Total: 2
  [INFO]   Passed: 2
  [INFO]   Failed: 0
  [INFO]   Total Time: 0.089ms
========================================
[SUCCESS] ALL TESTS PASSED!
========================================
```

These examples demonstrate the complete range of test output formatting patterns that should be used throughout the Game Engine Kiro testing system. Each example follows the established standards and provides clear, consistent, and professional output formatting.
