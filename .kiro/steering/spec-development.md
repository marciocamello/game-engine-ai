# Game Engine Kiro - Spec Development Guidelines

## MANDATORY SPEC DEVELOPMENT WORKFLOW

### Individual Test Development - REQUIRED FOR SPECS

**ALWAYS use individual test compilation during spec development for maximum efficiency:**

```powershell
# RECOMMENDED: Compile only the test you're working on
.\scripts\build_unified.bat --tests [TestName]

# Example: AnimationStateMachine development
.\scripts\build_unified.bat --tests AnimationstatemachineTest

# Run individual test
.\build\Release\AnimationstatemachineTest.exe

# ONLY after all individual tests pass, run full suite
.\scripts\run_tests.bat
```

### Speed Benefits

| Approach        | Compilation Time | Development Efficiency |
| --------------- | ---------------- | ---------------------- |
| Full test suite | 2-5 minutes      | Slow iteration         |
| Individual test | 10-30 seconds    | Fast iteration         |

### Test Name Conversion - MANDATORY RULES

Convert test file names to build target names:

**Pattern**: `test_[component_name].cpp` → `[ComponentName]Test`

**Examples**:

- `test_animation_state_machine.cpp` → `AnimationstatemachineTest`
- `test_physics_engine.cpp` → `PhysicsengineTest`
- `test_resource_manager.cpp` → `ResourcemanagerTest`

**Conversion Steps**:

1. Remove `test_` prefix
2. Remove `.cpp` extension
3. Convert snake_case to PascalCase
4. Add `Test` suffix

### Spec Implementation Workflow - MANDATORY

1. **Implement Core Classes**

   - Create headers in `include/[Module]/`
   - Create implementations in `src/[Module]/`

2. **Create Individual Test**

   - File: `tests/unit/test_[component].cpp`
   - Follow exact template from `testing-standards.md`

3. **Individual Development Cycle**

   ```powershell
   # Fast compile (30 seconds vs 3 minutes)
   .\scripts\build_unified.bat --tests [TestName]

   # Quick test
   .\build\Release\[TestName].exe

   # Fix issues and repeat until passing
   ```

4. **Move to Next Component**

   - Complete one test at a time
   - Don't move to next until current test passes

5. **Final Integration**
   ```powershell
   # MANDATORY before marking task complete
   .\scripts\run_tests.bat
   ```

### Task Completion Requirements - MANDATORY

**NEVER mark a spec task as complete unless ALL of these pass:**

1. **Individual test passes**: `.\build\Release\[TestName].exe` returns 0
2. **Individual compilation succeeds**: No errors or warnings
3. **Full build succeeds**: `.\scripts\build_unified.bat --tests` succeeds
4. **All tests pass**: `.\scripts\run_tests.bat` shows all green
5. **No regressions**: Existing functionality still works

### Benefits of Individual Test Development

- **10x Faster Compilation**: 30 seconds vs 5 minutes
- **Focused Debugging**: Issues isolated to specific components
- **Better Workflow**: Complete one feature at a time
- **Immediate Feedback**: Know instantly if changes work
- **Easier Troubleshooting**: Clear error isolation

### Common Spec Development Patterns

#### For Complex Features (like AnimationStateMachine)

1. **Break into Components**:

   - `AnimationStateMachine` class test
   - `AnimationTransition` class test
   - Integration test

2. **Test Dependencies First**:

   - Test base classes before derived classes
   - Test utilities before main classes

3. **Integration Last**:
   - Unit tests first
   - Integration tests after all units pass

#### For Bug Fixes During Spec Development

1. **Reproduce in Individual Test**: Create focused test case
2. **Fix and Verify**: Use individual compilation for quick verification
3. **Regression Test**: Run full suite to ensure no regressions

### CRITICAL RULES FOR SPEC DEVELOPMENT

- **ALWAYS use individual test compilation during active development**
- **NEVER compile all tests while iterating on a single component**
- **ALWAYS run full test suite before marking task complete**
- **NEVER mark task complete with failing individual tests**
- **ALWAYS follow exact test templates**

### Example: Successful Spec Development

```powershell
# 1. Implement AnimationStateMachine classes
# 2. Create test_animation_state_machine.cpp
# 3. Individual development cycle:

.\scripts\build_unified.bat --tests AnimationstatemachineTest
.\build\Release\AnimationstatemachineTest.exe

# Fix issues, repeat until:
# [SUCCESS] ALL TESTS PASSED!

# 4. Final validation:
.\scripts\run_tests.bat

# 5. Mark task complete only if all tests pass
```

### Troubleshooting Individual Tests

#### Test Not Found Error

```
Error: Test 'MyTest' not found
```

**Solution**: Check test name conversion rules above

#### Compilation Errors

```
Error: Cannot find header
```

**Solution**: Verify include paths in test file

#### Linking Errors

```
Error: Unresolved external symbol
```

**Solution**: Ensure implementation files exist and are complete

### Integration with Kiro Spec System

When working on spec tasks:

1. **Read requirements and design documents first**
2. **Implement using individual test development**
3. **Verify against specific requirements mentioned in tasks**
4. **Only mark complete when all validation passes**

This workflow ensures maximum development efficiency while maintaining code quality and test coverage.
