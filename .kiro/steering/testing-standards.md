# Game Engine Kiro - Testing Standards

## MANDATORY TEST STRUCTURE - FOLLOW EXACTLY

### Unit Test Template

All unit tests MUST follow this exact structure:

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

// Add more test functions following the same pattern

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

### Integration Test Template

All integration tests MUST follow this exact structure:

```cpp
#include <iostream>
#include "../TestUtils.h"
#include "Path/To/ComponentHeaders.h"
// Add other necessary includes

using namespace GameEngine::Testing;

/**
 * Test description
 * Requirements: X.X, Y.Y (requirement description)
 */
bool TestFunctionName() {
    TestOutput::PrintTestStart("test description");

    // Test implementation here
    // Use EXPECT_* macros for assertions
    // Include proper cleanup for resources

    TestOutput::PrintTestPass("test description");
    return true;
}

// Add more test functions following the same pattern

int main() {
    TestOutput::PrintHeader("Integration Test Name");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Integration Test Name Tests");

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

## CRITICAL TESTING RULES

### File Naming Convention

- Unit tests: `tests/unit/test_[component_name].cpp`
- Integration tests: `tests/integration/test_[feature_name].cpp`
- Test executables: `[ComponentName]Test.exe`

### Include Structure

- **Unit tests**: `#include "TestUtils.h"` (relative path)
- **Integration tests**: `#include "../TestUtils.h"` (relative path from integration folder)
- **ALWAYS include**: Component headers being tested
- **ALWAYS use**: `using namespace GameEngine;` and `using namespace GameEngine::Testing;`

### Test Function Requirements

- **MUST return**: `bool` (true for pass, false for fail)
- **MUST start with**: `TestOutput::PrintTestStart("description")`
- **MUST end with**: `TestOutput::PrintTestPass("description")` and `return true`
- **MUST include**: Requirements comment with specific requirement numbers
- **MUST use**: EXPECT\_\* macros for all assertions

### Main Function Requirements

- **MUST use**: `TestOutput::PrintHeader("ComponentName")`
- **MUST create**: `TestSuite suite("ComponentName Tests")`
- **MUST use**: `allPassed &= suite.RunTest("Description", TestFunction)`
- **MUST call**: `suite.PrintSummary()`
- **MUST call**: `TestOutput::PrintFooter(allPassed)`
- **MUST include**: Exception handling with try/catch blocks
- **MUST return**: `allPassed ? 0 : 1`

### Assertion Macros Available

```cpp
EXPECT_TRUE(condition)                    // Boolean true
EXPECT_FALSE(condition)                   // Boolean false
EXPECT_EQUAL(expected, actual)            // Exact equality
EXPECT_NOT_EQUAL(expected, actual)        // Inequality
EXPECT_NEARLY_EQUAL(expected, actual)     // Float comparison with epsilon
EXPECT_VEC3_NEARLY_EQUAL(expected, actual) // Vector3 comparison
EXPECT_VEC4_NEARLY_EQUAL(expected, actual) // Vector4 comparison
EXPECT_QUAT_NEARLY_EQUAL(expected, actual) // Quaternion comparison
```

### OpenGL Testing Strategy

- **ALWAYS try**: Mathematical and CPU-based tests first
- **IF OpenGL context required**: Use CPU/math alternatives when possible
- **IF OpenGL context errors**: Leave for future offscreen rendering structure
- **FOCUS on**: Testing logic, algorithms, and data structures rather than OpenGL calls
- **NEVER create**: Fictitious tests just to pass

### Test Output Standards

- **NEVER use**: Emotions/emojis in test messages
- **ALWAYS use**: Technical and professional language
- **ALWAYS include**: Proper error messages with expected vs actual values
- **ALWAYS use**: `TestOutput::PrintTestFail(testName, expected, actual)` for detailed failures

### Memory Management in Tests

- **ALWAYS cleanup**: Allocated resources in tests
- **USE RAII**: When possible for automatic cleanup
- **INCLUDE**: Proper exception safety in resource management
- **TEST**: Memory leaks and proper destruction

### Performance Testing Guidelines

- **USE**: Release builds for performance testing
- **MEASURE**: Actual performance metrics when relevant
- **AVOID**: Performance tests that depend on system load
- **FOCUS**: On algorithmic performance rather than absolute timing

## EXAMPLES FROM EXISTING TESTS

### Good Unit Test Example (Math)

```cpp
bool TestVectorOperations() {
    TestOutput::PrintTestStart("vector operations");

    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);
    Math::Vec3 result = a + b;
    Math::Vec3 expected(5.0f, 7.0f, 9.0f);

    EXPECT_VEC3_NEARLY_EQUAL(result, expected);

    TestOutput::PrintTestPass("vector operations");
    return true;
}
```

### Good Integration Test Example (Physics)

```cpp
bool TestBasicWorldInitialization() {
    TestOutput::PrintTestStart("basic Bullet Physics world initialization");

    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(/*...*/);

    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
    btVector3 gravity = dynamicsWorld->getGravity();

    EXPECT_NEARLY_EQUAL(gravity.getY(), -9.81f);

    // Proper cleanup
    delete dynamicsWorld;
    delete collisionConfiguration;

    TestOutput::PrintTestPass("basic Bullet Physics world initialization");
    return true;
}
```

## VIOLATIONS = BROKEN BUILD

- **Wrong include paths**: Build will fail
- **Missing namespaces**: Compilation errors
- **Wrong main structure**: Test runner won't work
- **Missing TestOutput calls**: Inconsistent output format
- **Wrong return types**: Test framework errors
- **Missing exception handling**: Crashes on errors

## INDIVIDUAL TEST DEVELOPMENT - RECOMMENDED WORKFLOW

### For Spec Development and Task Implementation

**RECOMMENDED**: Develop and test one component at a time for faster iteration:

```powershell
# 1. Compile only the specific test you're working on
.\scripts\build_unified.bat --tests [TestName]

# Example: Working on AnimationStateMachine
.\scripts\build_unified.bat --tests AnimationstatemachineTest

# 2. Run only that specific test
.\build\Release\AnimationstatemachineTest.exe

# 3. Debug and fix until it passes
# 4. Move to next test
# 5. When all individual tests pass, run full suite
.\scripts\run_tests.bat
```

### Benefits of Individual Test Development

- **Faster compilation**: Only builds what you need (seconds vs minutes)
- **Focused debugging**: Isolate issues to specific components
- **Quicker iteration**: Immediate feedback on changes
- **Better workflow**: Complete one feature at a time
- **Easier troubleshooting**: Clear error isolation

### Test Name Conversion Rules

File name `tests/unit/test_animation_state_machine.cpp` becomes test name `AnimationstatemachineTest`:

- Remove `test_` prefix
- Remove `.cpp` extension
- Convert to PascalCase
- Add `Test` suffix

Examples:

- `test_physics_engine.cpp` → `PhysicsengineTest`
- `test_resource_manager.cpp` → `ResourcemanagerTest`
- `test_shader_compiler.cpp` → `ShadercompilerTest`

### Individual Test Workflow for Specs

1. **Implement feature code** (headers and source)
2. **Create test file** following naming convention
3. **Write test functions** using templates above
4. **Compile individual test**: `.\scripts\build_unified.bat --tests [TestName]`
5. **Run and debug**: `.\build\Release\[TestName].exe`
6. **Fix issues** and repeat steps 4-5 until test passes
7. **Move to next feature/test**
8. **Final validation**: `.\scripts\run_tests.bat` (all tests)

## BEFORE COMPLETING ANY TASK WITH TESTS

1. **Individual tests pass**: Each test compiled and run individually
2. **Build successfully**: `.\scripts\build_unified.bat --tests`
3. **All tests pass**: `.\scripts\run_tests.bat`
4. **New tests follow template exactly**
5. **Output format matches existing tests**
6. **No compilation warnings for test files**
