# Game Engine Kiro - Development Guidelines

## Build System

### Primary Build Commands

```cmd
# Windows - Primary build command
.\scripts\build.bat

# Clean build (when needed)
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

```cmd
# Run all tests after build
.\scripts\run_tests.bat

# Individual test execution
.\build\Release\[TestName].exe

# Tests are automatically discovered by CMake from:
# - tests/unit/test_*.cpp -> Unit tests
# - tests/integration/test_*.cpp -> Integration tests
```

## Test Standards

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
        TestOutput::PrintTestFail("Test description", "Expected vs Actual");
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
        return suite.AllTestsPassed() ? 1 : 0;

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Exception: " + std::string(e.what()));
        return 1;
    }
}
```

### Test Output Standards

- Use `TestOutput::PrintInfo()` for informational messages
- Use `TestOutput::PrintTestPass()` for successful tests
- Use `TestOutput::PrintTestFail()` for failed tests with details
- Use `TestOutput::PrintError()` for errors
- Always include try/catch blocks in main functions
- Return proper exit codes (0 for success, 1 for failure)

### Test File Naming

- Unit tests: `tests/unit/test_[component_name].cpp`
- Integration tests: `tests/integration/test_[feature_name].cpp`
- Test executables: `[ComponentName]Test.exe`

## Code Standards

### Include Paths

```cpp
// Correct include for TestUtils
#include "TestUtils.h"  // From tests/ directory

// Engine includes
#include "Core/Engine.h"
#include "Graphics/Renderer.h"
// etc.
```

### Function Return Types

- Test functions MUST return `bool` (true for pass, false for fail)
- Use consistent error handling with try/catch blocks
- Provide meaningful error messages

## Development Workflow

### Adding New Tests

1. Create test file in appropriate directory (`tests/unit/` or `tests/integration/`)
2. Follow naming convention: `test_[component].cpp`
3. Use TestUtils.h for standardized output
4. CMake will automatically discover and compile the test
5. Test will be included in `scripts\run_tests.bat` execution

### Code Quality

- Follow existing code patterns
- Use proper error handling
- Include comprehensive logging
- Write tests for new functionality
- Maintain consistent formatting

### Performance Considerations

- Profile code changes
- Use Release builds for performance testing
- Monitor memory usage
- Optimize critical paths

## Debugging

### Debug Builds

```cmd
# Manual debug build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```

### Logging

- Use engine logging system: `LOG_INFO()`, `LOG_ERROR()`, etc.
- Check logs in `logs/` directory
- Use `scripts\monitor.bat` for real-time log monitoring

### Common Issues

- Include path problems: Check TestUtils.h path
- Build failures: Clean build directory and rebuild
- Test failures: Check output format and return types
- Missing dependencies: Run `scripts\setup_dependencies.bat`
