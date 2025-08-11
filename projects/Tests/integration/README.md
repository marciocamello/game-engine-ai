# Project Integration Tests

This directory contains integration tests for game projects (GameExample, BasicExample, etc.).

## Structure

```
integration/
├── GameExample/          # Integration tests for GameExample project
├── BasicExample/         # Integration tests for BasicExample project
└── README.md            # This file
```

## Test Naming Convention

- Test files should be named `test_[feature].cpp`
- Test executables will be named `Project[Feature]IntegrationTest.exe`
- Follow the same testing standards as engine integration tests

## Example Integration Test Structure

```cpp
#include <iostream>
#include "TestUtils.h"
#include "projects/GameExample/include/GameComponents.h"

using namespace GameEngine::Testing;

/**
 * Test gameplay integration
 * Requirements: X.X (requirement description)
 */
bool TestGameplayIntegration() {
    TestOutput::PrintTestStart("gameplay integration");

    // Test game-specific integration scenarios
    // Example: Character controller + camera + physics integration
    // Use EXPECT_* macros for assertions

    TestOutput::PrintTestPass("gameplay integration");
    return true;
}

int main() {
    TestOutput::PrintHeader("GameExample Integration");

    bool allPassed = true;

    try {
        TestSuite suite("GameExample Integration Tests");

        allPassed &= suite.RunTest("Gameplay Integration", TestGameplayIntegration);

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

- Engine integration tests remain in the root `tests/integration/` directory
- This directory is for testing game-specific integrations and workflows
- Currently no tests are implemented - this is a template structure for future use
