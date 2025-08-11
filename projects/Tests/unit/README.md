# Project Unit Tests

This directory contains unit tests for game project components.

## Structure

```
unit/
├── GameExample/          # GameExample unit tests
│   ├── test_game_logic.cpp
│   ├── test_player_controller.cpp
│   └── test_game_state.cpp
├── BasicExample/         # BasicExample unit tests
│   ├── test_basic_logic.cpp
│   └── test_basic_rendering.cpp
└── README.md            # This file
```

## Test Standards

All project unit tests must follow the same standards as engine tests:

- Use the exact template structure from `testing-standards.md`
- Include proper error handling with try/catch blocks
- Use standardized output format with `TestUtils.h`
- Return correct exit codes (0 for pass, 1 for fail)
- Include requirement references in test comments

## Example Test Structure

```cpp
#include "../../TestUtils.h"
#include "GameExample/GameLogic.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test game logic initialization
 * Requirements: Game.1.1 (game initialization)
 */
bool TestGameLogicInitialization() {
    TestOutput::PrintTestStart("game logic initialization");

    // Test implementation here
    GameLogic logic;
    EXPECT_TRUE(logic.Initialize());

    TestOutput::PrintTestPass("game logic initialization");
    return true;
}

int main() {
    TestOutput::PrintHeader("GameLogic");

    bool allPassed = true;

    try {
        TestSuite suite("GameLogic Tests");

        allPassed &= suite.RunTest("Game Logic Initialization", TestGameLogicInitialization);

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

## Current Status

- ✅ Directory structure ready
- ✅ Test discovery system implemented
- ✅ CMake integration ready
- ❌ No actual tests implemented yet

## Future Implementation

When game projects need unit testing:

1. Create subdirectory for the project (e.g., `GameExample/`)
2. Add test files following the naming convention `test_*.cpp`
3. Follow the same testing standards as engine tests
4. Tests will be automatically discovered and included in builds
