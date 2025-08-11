# Dual Test Architecture Documentation

Game Engine Kiro uses a dual test architecture to separate engine testing from project testing.

## Architecture Overview

### Engine Tests (`tests/` directory)

- **Purpose**: Testing engine core, modules, and plugins
- **Location**: Root `tests/` directory
- **Structure**:
  - `tests/unit/` - Unit tests for engine components
  - `tests/integration/` - Integration tests for engine modules
  - `tests/TestUtils.h` - Shared testing utilities
- **Execution**: `.\scripts\run_tests.bat`
- **Status**: Active with 31 tests (13 unit + 18 integration)

### Project Tests (`projects/Tests/` directory)

- **Purpose**: Testing game-specific logic and components
- **Location**: `projects/Tests/` directory
- **Structure**:
  - `projects/Tests/unit/` - Unit tests for game projects
  - `projects/Tests/integration/` - Integration tests for game projects
  - `projects/Tests/config/` - Test configuration files
- **Execution**: Future implementation (separate from engine tests)
- **Status**: Template structure only (no tests implemented yet)

## Key Principles

### No Code Duplication

- Both test structures use the same `TestUtils.h` from the engine tests
- No separate ProjectTestUtils to avoid duplication
- Same testing standards and macros across both structures

### Clear Separation of Concerns

- **Engine tests**: Test engine functionality, modules, physics, graphics, audio
- **Project tests**: Test game-specific logic, gameplay mechanics, project integrations

### Independent Execution

- Engine tests run independently and are part of the main build validation
- Project tests (when implemented) will run independently for game validation
- Both can be executed separately or together

## Usage Guidelines

### When to Use Engine Tests (`tests/`)

- Testing core engine functionality
- Testing engine modules (graphics, physics, audio)
- Testing engine utilities and math libraries
- Testing resource management and loading
- Testing engine integrations (Bullet Physics, OpenAL, etc.)

### When to Use Project Tests (`projects/Tests/`)

- Testing game-specific components and logic
- Testing gameplay mechanics and systems
- Testing project-specific integrations
- Testing game configuration and settings
- Testing custom game behaviors and AI

## Implementation Status

### Current State

- ✅ Engine tests: Fully implemented and active
- ✅ Project test structure: Template created
- ❌ Project tests: No actual tests implemented yet

### Future Implementation

When game projects need testing:

1. Create test files in appropriate `projects/Tests/unit/[ProjectName]/` or `projects/Tests/integration/[ProjectName]/`
2. Use the same `TestUtils.h` and testing standards
3. Follow the same template structure as engine tests
4. Configure tests in `projects/Tests/config/test_config.json`

## Benefits

### Maintainability

- Clear separation prevents test pollution
- Engine tests remain focused on engine functionality
- Project tests can grow independently

### Scalability

- New game projects can add their own test suites
- Engine tests don't become cluttered with game-specific logic
- Independent execution allows for targeted testing

### Development Workflow

- Engine developers can run engine tests quickly
- Game developers can focus on project-specific testing
- CI/CD can run appropriate test suites based on changes

## Technical Details

### Build Integration

- Engine tests: Integrated with main CMakeLists.txt
- Project tests: Separate CMakeLists.txt in `projects/Tests/`
- Both use the same build standards and dependencies

### Test Discovery

- Engine tests: Automatic discovery via existing CMake configuration
- Project tests: Automatic discovery via `projects/Tests/CMakeLists.txt`
- Both follow the same naming conventions (`test_*.cpp`)

### Configuration

- Engine tests: Use existing test configuration
- Project tests: Use `projects/Tests/config/test_config.json`
- Both support enabling/disabling test categories
