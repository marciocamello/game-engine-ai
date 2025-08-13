# Game Engine Kiro - Development Guidelines

## CRITICAL RULES - READ FIRST

**BEFORE ANY TASK**: Read critical-rules.md - Violation = Incomplete Task

## DANGEROUS COMMANDS - ABSOLUTELY PROHIBITED

### Windows Dangerous Commands

- **NEVER use**: `rmdir`, `rd`, `del`, `erase`, `format`, `diskpart`
- **NEVER use**: `Remove-Item` (except for build directory cleanup as specified)
- **NEVER use**: PowerShell deletion commands beyond specified exceptions
- **NEVER use**: Batch file deletion commands
- **ONLY PERMITTED cleanup**: `Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue`

### Cross-Platform Dangerous Commands

- **NEVER use**: `rm`, `rm -rf`, `unlink`, `shred`
- **NEVER use**: `mv` or `move` for deletion purposes
- **NEVER use**: Any command that could delete project files
- **NEVER use**: System-level commands that modify file permissions destructively

### Git Commands - Use with Extreme Caution

- **NEVER use**: `git reset --hard` without explicit approval
- **NEVER use**: `git clean -fd` without explicit approval
- **ALWAYS ask**: Before any destructive git operation

## Build System

### Primary Build Commands

```powershell
# Windows - ONLY permitted build command
.\scripts\build_unified.bat --tests

# Clean build - ONLY permitted cleanup command
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build_unified.bat --tests

# Development console with multiple options
.\scripts\dev.bat

# Run GameExample
build\projects\GameExample\Release\GameExample.exe

# Run BasicExample
build\projects\BasicExample\Release\BasicExample.exe

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

### Individual Test Development - RECOMMENDED FOR SPECS

**For spec development and individual task testing, compile and test one at a time:**

```powershell
# Compile only a specific test (much faster during development)
.\scripts\build_unified.bat --tests [TestName]

# Example: Compile only the AnimationStateMachine test
.\scripts\build_unified.bat --tests AnimationstatemachineTest

# Then run the specific test
.\build\Release\AnimationstatemachineTest.exe

# After all individual tests are working, run full test suite
.\scripts\run_tests.bat
```

**Benefits of Individual Test Development:**

- **Faster compilation**: Only builds what you need
- **Focused testing**: Test specific functionality in isolation
- **Quicker iteration**: Faster feedback loop during development
- **Easier debugging**: Isolate issues to specific components
- **Better workflow**: Complete one test at a time before moving to next

**Recommended Workflow for Specs:**

1. Implement feature code
2. Create test for that specific feature
3. Compile only that test: `.\scripts\build_unified.bat --tests [TestName]`
4. Run and debug that specific test until it passes
5. Move to next feature/test
6. When all individual tests pass, run full suite: `.\scripts\run_tests.bat`

## Test Standards - MANDATORY TO FOLLOW

**FOR COMPLETE TEST STANDARDS AND TEMPLATES**: See testing-standards.md

### Key Testing Rules

- **NEVER create fictitious tests just to pass**
- **ALWAYS follow the exact template structure in testing-standards.md**
- **ALWAYS run `.\scripts\run_tests.bat` before completing any task**
- **Focus on testing logic, algorithms, and data structures rather than OpenGL calls**

## Code Standards - MANDATORY

### CRITICAL RULE: Clean Code

- **ALWAYS check if class/method already exists before creating**
- **NEVER duplicate existing code**
- **NEVER add redundant code**
- **NEVER alter base classes without extreme necessity**

### CRITICAL RULE: Task-Driven Development

- **NEVER create anything not specified in current task**
- **If bugs/errors occur**: Fix them in place, do not recreate components
- **If tests fail**: Debug and fix the existing test, do not create new ones
- **If functions break**: Analyze and repair, do not rewrite from scratch
- **NEVER create "basic examples" when advanced examples already exist**
- **ALWAYS work within task scope**: Only implement what is explicitly requested

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

1. **Verify current build**: `.\scripts\build_unified.bat --tests`
2. **Execute all tests**: `.\scripts\run_tests.bat`
3. **Only start if everything is green**

### DURING TASK

1. **Frequent incremental builds**
2. **Test changes immediately**
3. **If something breaks: STOP and fix IMMEDIATELY**

### BEFORE COMPLETING TASK

1. **Complete build**: `.\scripts\build_unified.bat --tests`
2. **All tests**: `.\scripts\run_tests.bat`
3. **Check error logs**
4. **Create commit with task information**
5. **ONLY THEN mark as completed**

### CRITICAL RULE: Task Completion Standards

- **NEVER mark task completed if**:
  - Build is failing
  - Any tests are failing
  - Runtime errors exist
  - Performance regressions detected
- **If system breaks during task**: STOP and fix immediately
- **If unsure about reverting**: Ask developer for manual approval
- **ALWAYS commit after task completion** with descriptive message

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

### Communication Standards - MANDATORY

- **NEVER use complex summaries with icons at task completion**
- **Use simple, direct summaries**: State what was done, nothing more
- **NEVER use emoticons in analysis or technical communication**
- **Reserve emoticons ONLY for documentation creation**
- **Keep technical communication professional and concise**
- **NEVER use markdown formatting in summaries (no headers, bold, bullets)**
- **Use plain text checklists without complex formatting**
- **Avoid elaborate punctuation or visual elements in technical summaries**
- **Focus on factual information only in task completion reports**

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
- **Build failures**: `Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue` and `.\scripts\build_unified.bat --tests` (ONLY permitted commands)
- **Test failures**: Check output format and return types
- **Missing dependencies**: `.\scripts\setup_dependencies.bat`
- **GOLDEN RULE**: Broken build = Incomplete task until fixed
