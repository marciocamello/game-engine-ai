# Project Integration Tests

This directory contains integration tests for game project components and their interactions with the engine.

## Structure

```
integration/
├── GameExample/          # GameExample integration tests
│   ├── test_game_engine_integration.cpp
│   ├── test_physics_gameplay.cpp
│   └── test_audio_gameplay.cpp
├── BasicExample/         # BasicExample integration tests
│   ├── test_basic_engine_integration.cpp
│   └── test_basic_rendering_pipeline.cpp
└── README.md            # This file
```

## Test Standards

All project integration tests must follow the same standards as engine tests:

- Use the exact template structure from `testing-standards.md`
- Include proper error handling with try/catch blocks
- Use standardized output format with `TestUtils.h`
- Return correct exit codes (0 for pass, 1 for fail)
- Include requirement references in test comments
- Properly clean up resources after testing

## Example Test Structure

```cpp
#include "../../TestUtils.h"
#include "GameExample/GameEngine.h"
#include "Core/Engine.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test game engine integration
 * Requirements: Game.2.1 (engine integration)
 */
bool TestGameEngineIntegration() {
    TestOutput::PrintTestStart("game engine integration");

    // Initialize engine
    Engine engine;
    EXPECT_TRUE(engine.Initialize());

    // Initialize game
    GameExample::GameEngine gameEngine;
    EXPECT_TRUE(gameEngine.Initialize(&engine));

    // Test integration
    EXPECT_TRUE(gameEngine.IsEngineConnected());

    // Cleanup
    gameEngine.Shutdown();
    engine.Shutdown();

    TestOutput::PrintTestPass("game engine integration");
    return true;
}

int main() {
    TestOutput::PrintHeader("GameEngine Integration");

    bool allPassed = true;

    try {
        TestSuite suite("GameEngine Integration Tests");

        allPassed &= suite.RunTest("Game Engine Integration", TestGameEngineIntegration);

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

## Integration Test Focus Areas

Project integration tests should focus on:

1. **Engine Integration**: How game components interact with engine systems
2. **Cross-System Interactions**: Physics + gameplay, audio + gameplay, etc.
3. **Resource Management**: How games load and manage assets
4. **Performance Integration**: How game logic affects engine performance
5. **State Management**: Game state transitions and engine state

## Current Status

- ✅ Directory structure ready
- ✅ Test discovery system implemented
- ✅ CMake integration ready
- ❌ No actual tests implemented yet

## Future Implementation

When game projects need integration testing:

1. Create subdirectory for the project (e.g., `GameExample/`)
2. Add test files following the naming convention `test_*.cpp`
3. Focus on testing interactions between game and engine
4. Include proper resource cleanup and error handling
5. Tests will be automatically discovered and included in builds
