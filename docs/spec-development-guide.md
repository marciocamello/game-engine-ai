# Game Engine Kiro - Spec Development Guide

## Overview

This guide covers best practices for developing features using the Kiro spec system, with emphasis on efficient testing and development workflows.

## Spec Development Workflow

### 1. Requirements and Design Phase

Follow the standard spec workflow:

1. Create requirements document
2. Create design document
3. Create implementation tasks
4. Get user approval for each phase

### 2. Implementation Phase - Recommended Approach

**Use individual test compilation for faster development:**

#### Step-by-Step Process

1. **Implement Core Classes**

   ```cpp
   // Create headers in include/[Module]/
   // Create implementations in src/[Module]/
   ```

2. **Create Individual Test**

   ```cpp
   // Create test file: tests/unit/test_[component_name].cpp
   // Follow the exact template from testing-standards.md
   ```

3. **Compile Individual Test** (Fast!)

   ```powershell
   # Much faster than compiling all tests
   .\scripts\build_unified.bat --tests [TestName]

   # Example for AnimationStateMachine
   .\scripts\build_unified.bat --tests AnimationstatemachineTest
   ```

4. **Run and Debug Individual Test**

   ```powershell
   .\build\Release\AnimationstatemachineTest.exe
   ```

5. **Iterate Quickly**

   - Fix issues in code
   - Recompile only your test
   - Run only your test
   - Repeat until passing

6. **Move to Next Component**

   - Repeat process for each major component
   - Keep tests isolated and focused

7. **Final Integration**
   ```powershell
   # When all individual tests pass, run full suite
   .\scripts\run_tests.bat
   ```

## Test Name Conversion Rules

Convert file names to test names for compilation:

| File Name                          | Test Name                   |
| ---------------------------------- | --------------------------- |
| `test_animation_state_machine.cpp` | `AnimationstatemachineTest` |
| `test_physics_engine.cpp`          | `PhysicsengineTest`         |
| `test_resource_manager.cpp`        | `ResourcemanagerTest`       |
| `test_shader_compiler.cpp`         | `ShadercompilerTest`        |

**Conversion Rules:**

1. Remove `test_` prefix
2. Remove `.cpp` extension
3. Convert underscores to PascalCase
4. Add `Test` suffix

## Benefits of Individual Test Development

### Speed Comparison

| Approach        | Compilation Time | Feedback Loop |
| --------------- | ---------------- | ------------- |
| Full test suite | 2-5 minutes      | Slow          |
| Individual test | 10-30 seconds    | Fast          |

### Development Benefits

- **Faster Iteration**: Immediate feedback on changes
- **Focused Debugging**: Issues isolated to specific components
- **Better Workflow**: Complete one feature at a time
- **Easier Troubleshooting**: Clear error isolation
- **Reduced Context Switching**: Stay focused on one component

## Example: AnimationStateMachine Development

### 1. Implementation

```cpp
// include/Animation/AnimationStateMachine.h
// src/Animation/AnimationStateMachine.cpp
// include/Animation/AnimationTransition.h
// src/Animation/AnimationTransition.cpp
```

### 2. Test Creation

```cpp
// tests/unit/test_animation_state_machine.cpp
// Follow exact template structure
```

### 3. Individual Development Cycle

```powershell
# Fast compile (30 seconds vs 3 minutes)
.\scripts\build_unified.bat --tests AnimationstatemachineTest

# Quick test
.\build\Release\AnimationstatemachineTest.exe

# Fix issues, repeat until passing
```

### 4. Integration

```powershell
# Final validation with all tests
.\scripts\run_tests.bat
```

## Common Patterns

### For Complex Features

1. **Break into Components**: Create separate tests for major classes
2. **Test Dependencies First**: Test base classes before derived classes
3. **Integration Last**: Create integration tests after unit tests pass

### For Bug Fixes

1. **Reproduce in Individual Test**: Create focused test case
2. **Fix and Verify**: Use individual compilation for quick verification
3. **Regression Test**: Run full suite to ensure no regressions

## Best Practices

### Do's ✅

- **Use individual test compilation during development**
- **Run full test suite before task completion**
- **Follow exact test templates**
- **Test one component at a time**
- **Keep tests focused and isolated**

### Don'ts ❌

- **Don't compile all tests during active development**
- **Don't skip individual testing**
- **Don't create tests that depend on each other**
- **Don't ignore compilation warnings**
- **Don't mark tasks complete with failing tests**

## Troubleshooting

### Common Issues

1. **Test Name Not Found**

   ```
   Error: Test 'MyTest' not found
   ```

   **Solution**: Check test name conversion rules above

2. **Compilation Errors**

   ```
   Error: Cannot find header file
   ```

   **Solution**: Check include paths in test file

3. **Linking Errors**
   ```
   Error: Unresolved external symbol
   ```
   **Solution**: Ensure implementation files are created

### Debug Commands

```powershell
# Clean build if issues persist
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build_unified.bat --tests [TestName]

# Check logs
.\scripts\monitor.bat

# Full rebuild
.\scripts\build_unified.bat --tests
```

## Integration with Spec System

### Task Completion Checklist

- [ ] Individual tests created and passing
- [ ] Individual compilation successful
- [ ] Full test suite passing
- [ ] No compilation warnings
- [ ] Code follows project standards
- [ ] Documentation updated if needed

### Marking Tasks Complete

Only mark spec tasks as complete when:

1. **Individual tests pass**: `.\build\Release\[TestName].exe` returns 0
2. **Full build succeeds**: `.\scripts\build_unified.bat --tests` succeeds
3. **All tests pass**: `.\scripts\run_tests.bat` shows all green
4. **No regressions**: Existing functionality still works

## Conclusion

Individual test development significantly improves the spec development experience by providing faster feedback loops and better debugging capabilities. Use this approach for all spec implementations to maximize productivity and code quality.
