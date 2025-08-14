# Game Engine Kiro - CRITICAL PROJECT RULES

## MANDATORY RULES - NEVER VIOLATE

### 1. ABSOLUTE PROHIBITIONS

#### Language and Code

- **ENGLISH ONLY**: All code, comments, logs, documentation, and messages MUST be in English
- **NEVER use emotions/emojis** in code, logs, comments, or system messages
- Emotions are permitted ONLY in documentation (docs/)
- Maintain technical and professional language throughout all code

#### Dangerous Commands

- **PROHIBITED to use deletion commands** beyond what is specified:
  - ONLY permitted command to clean build: `Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue` (PowerShell)
  - NEVER use: `rm`, `del`, `rmdir` or other deletion commands
  - NEVER use commands that could delete project files
- **PROHIBITED to use cmake commands directly**
  - ALWAYS use `.\scripts\build_unified.bat --tests` for builds
  - NEVER execute `cmake` or `cmake --build` manually

### 2. BUILD AND TESTS - GOLDEN RULE

#### Broken Build = Incomplete Task

- **If build breaks during a task, STOP EVERYTHING**
- **Fix ALL build errors before continuing**
- **Run all tests and ensure they pass**
- **NEVER leave a task "completed" with broken build**
- **NEVER ignore compilation errors or failing tests**

#### Priority Order

1. Fix broken build
2. Fix failing tests
3. Continue with original task
4. Complete task only when everything is working

### 3. CODE STRUCTURE

#### Base Classes

- **NEVER alter base classes without extreme necessity**
- **Always verify impact on entire project before modifying**
- **If altering base class, test ALL dependent components**

#### Directory Structure

- **ALWAYS follow structure defined in structure.md**
- **NEVER create code outside appropriate directories**
- **Headers in include/**, **implementation in src/**, **tests in tests/**

#### Clean Code

- **ALWAYS check if class/method already exists before creating**
- **NEVER duplicate existing code**
- **NEVER add redundant code just to make tests pass**
- **Reuse existing components whenever possible**

### 3.1. NAMESPACE AND SYMBOL CONFLICTS - ZERO TOLERANCE

#### Namespace Uniqueness - MANDATORY

- **EVERY namespace MUST be unique across the entire project**
- **NEVER create duplicate namespace names in different modules**
- **NEVER use generic namespace names like `Utils`, `Common`, `Base`**
- **ALWAYS use descriptive, module-specific namespace names**
- **Example**: `GameEngine::Animation::SkeletalAnimation` NOT `GameEngine::Animation::Animation`

#### Class Name Conflicts - FORBIDDEN

- **NEVER create classes with identical names in different namespaces**
- **Class names MUST be unique and descriptive of their purpose**
- **Example**: Use `SkeletalAnimation` and `GraphicsAnimation`, NOT two `Animation` classes
- **ALWAYS verify class name uniqueness before creating new classes**

#### Macro Conflicts - ZERO TOLERANCE

- **ALL macros MUST have unique names with proper prefixes**
- **ALWAYS use project-specific prefixes: `GAMEENGINE_`, `GE_`**
- **NEVER use generic macro names like `MAX`, `MIN`, `ASSERT`**
- **Example**: `GAMEENGINE_MAX_BONES` NOT `MAX_BONES`
- **ALWAYS check for existing macros before defining new ones**

#### Symbol Resolution Rules

- **NEVER rely on `using namespace` in headers**
- **ALWAYS use fully qualified names in headers**
- **NEVER create ambiguous symbol names**
- **ALWAYS verify symbol uniqueness across all modules**

#### Conflict Prevention Checklist

Before creating ANY new symbol (class, namespace, macro, function):

1. **Search entire codebase for existing names**
2. **Verify no conflicts in current or planned modules**
3. **Use descriptive, purpose-specific names**
4. **Add proper prefixes for macros and global symbols**
5. **Document the new symbol in appropriate headers**

#### Violation Consequences

- **Any namespace conflict** = IMMEDIATE task failure
- **Any class name conflict** = IMMEDIATE refactoring required
- **Any macro conflict** = IMMEDIATE renaming required
- **Ambiguous symbols** = IMMEDIATE resolution required

**This is the MINIMUM standard for ANY professional C++ project**

### 4. TEST STANDARDS

#### Valid Tests

- **Write correct tests, not just to pass**
- **When OpenGL/GLAD prevents testing, DO NOT create fictitious test**
- **Understand limitation or leave test for future offscreen structure**
- **NEVER create workarounds in tests just to "pass"**

#### Test Structure

- **ALWAYS use TestUtils.h and defined standards**
- **ALWAYS follow standardized output format**
- **ALWAYS include try/catch in main functions**
- **ALWAYS return correct exit codes (0/1)**

### 5. PLATFORM AND ENVIRONMENT

#### Windows Only

- **Project is EXCLUSIVELY Windows**
- **ALWAYS use Windows commands (cmd/PowerShell)**
- **NEVER assume Linux/Mac compatibility**
- **Use Windows path separators (\)**

#### Mandatory Scripts

- **Build**: `.\scripts\build_unified.bat --tests` (ONLY permitted command)
- **Tests**: `.\scripts\run_tests.bat`
- **Debug**: `.\scripts\debug.bat`
- **Logs**: `.\scripts\monitor.bat`

## MANDATORY WORKFLOW

### Before Starting Any Task

1. Verify current build is working
2. Execute `.\scripts\run_tests.bat` to confirm state
3. Only start task if everything is green

### During Task

1. Make frequent incremental builds
2. Test changes immediately
3. If something breaks, stop and fix IMMEDIATELY

### Before Completing Task

1. Complete build: `.\scripts\build_unified.bat --tests`
2. All tests: `.\scripts\run_tests.bat`
3. Check error logs
4. Confirm NOTHING is broken
5. ONLY THEN mark task as completed

## VIOLATION CONSEQUENCES

- **Broken build** = Task automatically INCOMPLETE
- **Failing test** = Task automatically INCOMPLETE
- **Code outside structure** = Mandatory refactoring
- **Code duplication** = Mandatory cleanup
- **Use of prohibited command** = Immediate reversal

## FINAL VERIFICATION

Before any commit or task completion:

```powershell
# 1. Clean build
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build_unified.bat --tests

# 2. All tests
.\scripts\run_tests.bat

# 3. Check logs
.\scripts\monitor.bat
```

**IF ANY STEP FAILS = TASK IS NOT READY**
