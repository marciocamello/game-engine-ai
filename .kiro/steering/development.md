# Game Engine Kiro - Development Guidelines

## CRITICAL RULES - READ FIRST

**BEFORE ANY TASK**: Read critical-rules.md - Violation = Incomplete Task

## Build System

### Primary Build Commands

```powershell
# Windows - ONLY permitted build command
.\scripts\build.bat

# Clean build - ONLY permitted cleanup command
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build.bat

# Development console with multiple options
.\scripts\dev.bat

# Run with logging
.\scripts\start.bat

# Monitor logs in real-time
.\scripts\monitor.bat

# Debug session
.\scripts\debug.bat
```

### Test Execution

```powershell
# Run all tests after build - MANDATORY before completing task
.\scripts\run_tests.bat

# Individual test execution
.\build\Release\[TestName].exe

# Tests are automatically discovered by CMake from:
# - tests/unit/test_*.cpp -> Unit tests
# - tests/integration/test_*.cpp -> Integration tests
```

## Test Standards - MANDATORY TO FOLLOW

### CRITICAL RULE: Correct Tests

- **NEVER create fictitious tests just to pass**
- **If OpenGL/GLAD prevents testing, leave for future offscreen structure**
- **Write valid tests, not workarounds**

### Test Output Format

All tests MUST use the standardized TestOutput methods:

```cpp
#include "TestUtils.h"

// Test structure
bool TestFunctionName() {
    TestOutput::PrintTestStart("Test description");

    // Test logic here
    if (condition) {
        TestOutput::PrintTestPass("Test description");
        return true;
    } else {
        TestOutput::PrintTestFail("Test description", "expected_value", "actual_value");
        return false;
    }
}

// Main function structure
int main() {
    TestOutput::PrintHeader("Test Suite Name");

    TestSuite suite("Test Suite Name");

    try {
        suite.RunTest("Test Name", TestFunctionName);
        // Add more tests...

        suite.PrintSummary();
        return suite.AllTestsPassed() ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception: " + std::string(e.what()));
        return 1;
    }
}
```

### Test Output Standards - MANDATORY

- Use `TestOutput::PrintInfo()` for informational messages
- Use `TestOutput::PrintTestPass()` for successful tests
- Use `TestOutput::PrintTestFail()` for failed tests:
  - `PrintTestFail(testName)` - simple failure message
  - `PrintTestFail(testName, expected, actual)` - detailed failure with expected vs actual values
- Use `TestOutput::PrintError()` for errors and exceptions
- **ALWAYS include try/catch blocks in main functions**
- **ALWAYS return correct exit codes (0 for success, 1 for failure)**
- **NEVER use emotions/emojis in test messages**

### Test File Naming

- Unit tests: `tests/unit/test_[component_name].cpp`
- Integration tests: `tests/integration/test_[feature_name].cpp`
- Test executables: `[ComponentName]Test.exe`

## Code Standards - MANDATORY

### CRITICAL RULE: Clean Code

- **ALWAYS check if class/method already exists before creating**
- **NEVER duplicate existing code**
- **NEVER add redundant code**
- **NEVER alter base classes without extreme necessity**

### Include Paths

```cpp
// Correct include for TestUtils
#include "../TestUtils.h"  // For integration tests in tests/integration/
#include "TestUtils.h"     // For unit tests in tests/unit/

// Engine includes
#include "Core/Engine.h"
#include "Graphics/Renderer.h"
// etc.
```

### Function Return Types

- Test functions MUST return `bool` (true for pass, false for fail)
- Use consistent error handling with try/catch blocks
- Provide meaningful error messages

## Development Workflow - MANDATORY FLOW

### BEFORE STARTING ANY TASK

1. **Verify current build**: `.\scripts\build.bat`
2. **Execute all tests**: `.\scripts\run_tests.bat`
3. **Only start if everything is green**

### DURING TASK

1. **Frequent incremental builds**
2. **Test changes immediately**
3. **If something breaks: STOP and fix IMMEDIATELY**

### BEFORE COMPLETING TASK

1. **Complete build**: `.\scripts\build.bat`
2. **All tests**: `.\scripts\run_tests.bat`
3. **Check error logs**
4. **ONLY THEN mark as completed**

### Adding New Tests

1. Create test file in appropriate directory (`tests/unit/` or `tests/integration/`)
2. Follow naming convention: `test_[component].cpp`
3. Use TestUtils.h for standardized output
4. CMake will automatically discover and compile the test
5. Test will be included in `.\scripts\run_tests.bat` execution

### Code Quality - MANDATORY RULES

- **ALWAYS follow defined directory structure**
- **NEVER create code outside appropriate directories**
- **Headers in include/, implementation in src/, tests in tests/**
- **NEVER use emotions/emojis in code or logs**
- **Follow existing patterns without duplication**
- **Use proper error handling**
- **Technical and professional logging only**

### Performance Considerations

- Profile code changes
- Use Release builds for performance testing
- Monitor memory usage
- Optimize critical paths

## Debugging

### CRITICAL RULE: Permitted Commands

- **PROHIBITED to use cmake directly**
- **ALWAYS use project scripts**

### Debug Builds

```powershell
# ONLY permitted command for debug
.\scripts\debug.bat
```

### Logging - MANDATORY STANDARDS

- **ALWAYS use engine logging system**: `LOG_INFO()`, `LOG_ERROR()`, etc.
- **NEVER use emotions/emojis in logs**
- **Maintain technical and professional language**
- **Check logs in logs/ directory**
- **Use .\scripts\monitor.bat for real-time monitoring**

### Common Issues - MANDATORY SOLUTIONS

- **Include path problems**: Check TestUtils.h path
- **Build failures**: `Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue` and `.\scripts\build.bat` (ONLY permitted commands)
- **Test failures**: Check output format and return types
- **Missing dependencies**: `.\scripts\setup_dependencies.bat`
- **GOLDEN RULE**: Broken build = Incomplete task until fixed
