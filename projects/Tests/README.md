# Project Tests

This directory contains the testing infrastructure for game projects (GameExample, BasicExample, etc.).

## Important Note

**Engine tests remain in the root `tests/` directory.** This directory is specifically for testing game project components and logic.

## Directory Structure

```
projects/Tests/
├── CMakeLists.txt           # Project test build configuration
├── unit/                    # Unit tests for game projects
│   ├── GameExample/        # GameExample unit tests (future)
│   ├── BasicExample/       # BasicExample unit tests (future)
│   └── README.md           # Unit test documentation
├── integration/            # Integration tests for game projects
│   ├── GameExample/        # GameExample integration tests (future)
│   ├── BasicExample/       # BasicExample integration tests (future)
│   └── README.md           # Integration test documentation
├── config/                 # Test configuration files
│   └── test_config.json    # Project test configuration
└── README.md              # This file
```

## Dual Test Architecture

The Game Engine Kiro project uses a dual test architecture:

### Engine Tests (`tests/` directory)

- **Location**: Root `tests/` directory
- **Purpose**: Testing engine core, modules, and plugins
- **Structure**: `tests/unit/` and `tests/integration/`
- **Execution**: `.\scripts\run_tests.bat`
- **Status**: Active with 31 tests (13 unit + 18 integration)

### Project Tests (`projects/Tests/` directory)

- **Location**: `projects/Tests/` directory
- **Purpose**: Testing game-specific logic and components
- **Structure**: `unit/` and `integration/` subdirectories
- **Execution**: Future implementation
- **Status**: Template structure only (no tests implemented yet)

## Current Status

This directory provides the template structure for future project testing. Currently:

- ✅ Directory structure created
- ✅ CMakeLists.txt template ready
- ✅ Uses existing TestUtils from engine tests (no duplication)
- ✅ Test configuration system ready
- ❌ No actual tests implemented yet

## Future Implementation

When game projects need testing, developers can:

1. Create test files in `unit/[ProjectName]/` or `integration/[ProjectName]/`
2. Follow the same testing standards as engine tests
3. Use the existing `TestUtils.h` from the engine tests (no duplication)
4. Configure tests in `config/test_config.json`

## Testing Standards

Project tests should follow the same standards as engine tests:

- Use the exact template structure from `testing-standards.md`
- Include proper error handling with try/catch blocks
- Use standardized output format
- Return correct exit codes (0 for pass, 1 for fail)
- Include requirement references in test comments

## Build Integration

The project test system integrates with the main build system:

- Tests are automatically discovered by CMake
- Individual test executables are created
- Tests can be run independently or as a suite
- Build configuration supports conditional compilation

## Notes

- Engine functionality should be tested in the root `tests/` directory
- Only game-specific logic should be tested here
- This structure supports future expansion as projects grow
- Uses the same TestUtils.h as engine tests to avoid code duplication
