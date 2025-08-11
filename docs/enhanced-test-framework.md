# Enhanced Test Framework for Dual Architecture

This document describes the enhanced test framework implemented for Game Engine Kiro's dual architecture, supporting both engine tests and project tests with advanced categorization and configuration capabilities.

## Overview

The enhanced test framework provides:

- **Dual Architecture Support**: Separate test systems for engine and project tests
- **Test Categorization**: Unit, integration, and performance test categories
- **Configuration Management**: JSON-based test configuration with enable/disable controls
- **Automated Discovery**: Automatic test discovery in both engine and project directories
- **Enhanced Reporting**: Multiple output formats with detailed timing and error information
- **Cross-Platform Execution**: Windows-focused with extensible design

## Architecture

### Core Components

#### 1. Test Framework Interface (`engine/interfaces/TestFramework.h`)

Defines the core interfaces for the test system:

- `ITestDiscovery`: Test discovery and categorization
- `ITestExecutor`: Test execution and result collection
- `ITestFramework`: Overall framework management
- `TestConfig`: Configuration structure
- `TestExecutionResult`: Result tracking

#### 2. Test Framework Implementation (`engine/core/TestFramework.cpp`)

Provides default implementations:

- `DefaultTestDiscovery`: File-based test discovery
- `DefaultTestExecutor`: Process-based test execution
- `DefaultTestFramework`: Complete framework implementation

#### 3. Enhanced Test Runner (`engine/core/TestRunner.cpp`)

Command-line test runner with support for:

- Running all tests or by category
- Listing discovered tests
- Configuration-based filtering
- Multiple output formats

#### 4. Test Configuration Manager (`engine/core/TestConfigManager.cpp`)

Utility for managing test configuration:

- Enable/disable test categories
- Set output formats and verbosity
- Reset to default configuration
- Show current configuration

## Test Categories

### Unit Tests

- **Location**: `tests/unit/` and `projects/Tests/unit/`
- **Purpose**: Test individual components in isolation
- **Characteristics**: Fast execution, no external dependencies
- **Naming**: `test_*.cpp` files

### Integration Tests

- **Location**: `tests/integration/` and `projects/Tests/integration/`
- **Purpose**: Test component interactions and system integration
- **Characteristics**: May require external resources, longer execution
- **Naming**: `test_*.cpp` files

### Performance Tests

- **Location**: `tests/performance/` and `projects/Tests/performance/` (future)
- **Purpose**: Validate performance requirements and detect regressions
- **Characteristics**: Timing-sensitive, multiple iterations
- **Naming**: `test_*.cpp` files

## Configuration System

### Configuration File Location

- **Engine Tests**: Uses `projects/Tests/config/test_config.json`
- **Project Tests**: Same configuration file (unified system)

### Configuration Options

```json
{
  "enabledCategories": {
    "unit": true,
    "integration": true,
    "performance": true
  },
  "enablePerformanceTests": true,
  "enableIntegrationTests": true,
  "enableUnitTests": true,
  "verboseOutput": false,
  "showTimings": true,
  "outputFormat": "standard",
  "performanceTimeoutMs": 30000.0,
  "performanceIterations": 1000,
  "testDirectories": [
    "tests/unit",
    "tests/integration",
    "projects/Tests/unit",
    "projects/Tests/integration"
  ],
  "excludePatterns": ["**/temp_*", "**/debug_*", "**/.backup/**"]
}
```

## Usage

### Running Tests

#### Enhanced Test Runner

```bash
# Run all tests
.\scripts\run_enhanced_tests.bat

# Run specific category
.\scripts\run_enhanced_tests.bat unit
.\scripts\run_enhanced_tests.bat integration
.\scripts\run_enhanced_tests.bat performance

# List all discovered tests
.\scripts\run_enhanced_tests.bat list

# Show help
.\scripts\run_enhanced_tests.bat help
```

#### Direct Execution

```bash
# Using enhanced test runner directly
build\Release\EnhancedTestRunner.exe --all
build\Release\EnhancedTestRunner.exe --unit
build\Release\EnhancedTestRunner.exe --integration
build\Release\EnhancedTestRunner.exe --list
```

### Managing Configuration

#### Test Configuration Manager

```bash
# Show current configuration
.\scripts\test_config.bat --show

# Enable/disable categories
.\scripts\test_config.bat --enable unit
.\scripts\test_config.bat --disable performance

# Set output format
.\scripts\test_config.bat --format json
.\scripts\test_config.bat --format standard

# Enable verbose output
.\scripts\test_config.bat --verbose on

# Reset to defaults
.\scripts\test_config.bat --reset
```

## Test Discovery

### Engine Tests

- **Directory**: `tests/unit/` and `tests/integration/`
- **Pattern**: `test_*.cpp` files
- **Executable Naming**: Converts `test_something.cpp` to `SomethingTest.exe`
- **Build Integration**: Automatic CMake discovery and compilation

### Project Tests

- **Directory**: `projects/Tests/unit/` and `projects/Tests/integration/`
- **Pattern**: `test_*.cpp` files
- **Executable Naming**: Same as engine tests with `Project` prefix
- **Build Integration**: Separate CMake configuration with shared utilities

## Project Test Utilities

### ProjectTestUtils (`projects/Tests/utilities/ProjectTestUtils.h`)

Provides project-specific testing utilities:

- `GameTestFixture`: Setup/cleanup for game testing environments
- `GamePerformanceTest`: Performance validation for game components
- `GameAssetTest`: Asset loading and validation utilities
- Mock environment creation and management

### Usage Example

```cpp
#include "utilities/ProjectTestUtils.h"

bool TestGamePerformance() {
    GAME_TEST_FIXTURE_SETUP();

    auto gameLoop = []() {
        // Game loop logic here
    };

    EXPECT_GAME_PERFORMANCE("game loop", gameLoop, 16.67); // 60 FPS

    GAME_TEST_FIXTURE_CLEANUP();
    return true;
}
```

## Integration with Existing System

### Backward Compatibility

- **Existing Scripts**: `.\scripts\run_tests.bat` continues to work unchanged
- **Existing Tests**: All current tests continue to function normally
- **Build System**: Enhanced framework is additive, doesn't break existing builds

### Migration Path

1. **Phase 1**: Enhanced framework runs alongside existing system
2. **Phase 2**: Gradually migrate test execution to enhanced runner
3. **Phase 3**: Deprecate old test runner when fully validated

## Output Formats

### Standard Format

```
========================================
 Game Engine Kiro - Test Report
========================================

Summary by Category:
  unit: 25/25 passed
  integration: 18/18 passed

Overall: 43/43 passed (2.345s)

========================================
```

### JSON Format

```json
{
  "timestamp": 1640995200,
  "totalTests": 43,
  "testsPassed": 43,
  "testsFailed": 0,
  "totalExecutionTimeMs": 2345.67,
  "results": [
    {
      "name": "math",
      "category": "unit",
      "passed": true,
      "executionTimeMs": 12.34,
      "errorMessage": ""
    }
  ]
}
```

## Performance Considerations

### Test Execution

- **Parallel Execution**: Currently sequential, designed for future parallelization
- **Timeout Management**: Configurable timeouts prevent hanging tests
- **Resource Cleanup**: Automatic cleanup of temporary test resources

### Memory Management

- **RAII Design**: Automatic resource management in test fixtures
- **Leak Detection**: Framework designed to support memory leak detection
- **Isolation**: Tests run in separate processes to prevent interference

## Future Enhancements

### Planned Features

1. **Parallel Test Execution**: Run tests concurrently for faster execution
2. **Test Filtering**: Advanced filtering by tags, requirements, or custom criteria
3. **CI/CD Integration**: Enhanced reporting for continuous integration systems
4. **Visual Test Runner**: GUI application for interactive test management
5. **Test Coverage Integration**: Automatic code coverage reporting

### Extensibility Points

- **Custom Test Discoverers**: Plugin system for specialized test discovery
- **Custom Executors**: Support for different test execution environments
- **Custom Reporters**: Additional output formats and reporting destinations
- **Test Decorators**: Metadata and tagging system for advanced test management

## Troubleshooting

### Common Issues

#### Tests Not Discovered

- Verify test files follow `test_*.cpp` naming convention
- Check that test directories are included in configuration
- Ensure CMake has been run to generate build files

#### Configuration Not Loading

- Verify JSON syntax in `projects/Tests/config/test_config.json`
- Check file permissions and accessibility
- Use `--reset` option to restore default configuration

#### Test Execution Failures

- Check that all test executables are built successfully
- Verify test dependencies are available
- Review test output for specific error messages

### Debug Mode

Enable verbose output for detailed debugging information:

```bash
.\scripts\test_config.bat --verbose on
.\scripts\run_enhanced_tests.bat
```

## Requirements Mapping

This implementation addresses the following requirements from the modular project architecture specification:

- **Requirement 3.3**: Test categorization (unit, integration, performance) for both structures
- **Requirement 3.4**: Maintain existing engine test discovery system in tests/ directory
- **Requirement 3.5**: Create project test discovery system for projects/Tests/ directory

The enhanced test framework provides a comprehensive solution for managing tests in the dual architecture while maintaining backward compatibility and providing advanced features for future development.
