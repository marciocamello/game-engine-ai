# Project Unit Tests

This directory contains unit tests for game projects (GameExample, BasicExample, etc.).

## Structure

```
unit/
├── GameExample/          # Unit tests for GameExample project
├── BasicExample/         # Unit tests for BasicExample project
└── README.md            # This file
```

## Test Naming Convention

- Test files should be named `test_[component].cpp`
- Test executables will be named `Project[Component]Test.exe`
- Follow the same testing standards as engine tests

## Example Test Structure

```cpp
#include "TestUtils.h"
#include "projects/GameExample/include/GameSpecificComponent.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test game specific feature
 * Requirements: X.X (requirement description)
 */
bool TestGameSpecificFeature() {
    TestOutput::PrintTestStart("game specific feature");

    // Test implementation here
    // Use EXPECT_* macros for assertions:
    // EXPECT_TRUE(condition)
    // EXPECT_EQUAL(expected, actual)

    TestOutput::PrintTestPass("game specific feature");
    return true;
}

int main() {
    TestOutput::PrintHeader("GameExample Component");

    bool allPassed = true;

    try {
        TestSuite suite("GameExample Component Tests");

        allPassed &= suite.RunTest("Game Specific Feature", TestGameSpecificFeature);

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

## Notes

- Engine tests remain in the root `tests/` directory
- This directory is for testing game-specific logic and components
- Currently no tests are implemented - this is a template structure for future use
