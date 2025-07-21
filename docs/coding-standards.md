# Game Engine Kiro - Coding Standards

## Overview

This document establishes consistent coding standards for Game Engine Kiro, ensuring maintainable and professional code across all components.

## Logging and Output Standards

### No Icons or Emoticons

**RULE: Never use Unicode icons, emoticons, or special characters in code output.**

**Rationale:**

- Cross-platform compatibility issues
- Console encoding problems
- Professional appearance
- Accessibility concerns
- CI/CD system compatibility

### Standard Output Prefixes

Use consistent text-based prefixes for all output:

#### Test Output

```cpp
// CORRECT
std::cout << "[PASS] Vector operations test passed" << std::endl;
std::cout << "[FAILED] Some tests failed" << std::endl;
std::cout << "[SUCCESS] All tests completed successfully" << std::endl;
std::cout << "[ERROR] Test exception occurred" << std::endl;
std::cout << "[WARNING] Potential memory leak detected" << std::endl;
std::cout << "[INFO] Memory usage within acceptable range" << std::endl;

// INCORRECT - Never use these
std::cout << "✅ Test passed" << std::endl;           // Unicode icons
std::cout << "❌ Test failed" << std::endl;           // Unicode icons
std::cout << "🎉 Success!" << std::endl;             // Emoticons
```

#### Logger Output

```cpp
// CORRECT
Logger::GetInstance().Info("Physics engine initialized successfully");
Logger::GetInstance().Warning("Texture not found, using default");
Logger::GetInstance().Error("Failed to load shader file");
Logger::GetInstance().Debug("Rendering 1024 triangles this frame");

// INCORRECT - Never use these
Logger::GetInstance().Info("🚀 Engine started!");    // Emoticons
Logger::GetInstance().Error("❌ Shader failed");     // Unicode icons
```

#### Console Output

```cpp
// CORRECT
std::cout << "Game Engine Kiro - Build System" << std::endl;
std::cout << "Build Completed Successfully!" << std::endl;
std::cout << "Configuration loaded from file" << std::endl;

// INCORRECT
std::cout << "🎮 Game Engine Kiro" << std::endl;     // Emoticons
std::cout << "✅ Build Success!" << std::endl;       // Unicode icons
```

### Standard Prefixes Reference

| Prefix      | Usage             | Example                                      |
| ----------- | ----------------- | -------------------------------------------- |
| `[PASS]`    | Test passed       | `[PASS] Vector operations test passed`       |
| `[FAILED]`  | Test failed       | `[FAILED] Matrix multiplication test failed` |
| `[SUCCESS]` | Overall success   | `[SUCCESS] All tests completed successfully` |
| `[ERROR]`   | Error condition   | `[ERROR] Failed to initialize graphics`      |
| `[WARNING]` | Warning condition | `[WARNING] Using fallback renderer`          |
| `[INFO]`    | Informational     | `[INFO] Loading 15 textures`                 |
| `[DEBUG]`   | Debug information | `[DEBUG] Frame time: 16.7ms`                 |

## Test Writing Standards

### Simple Unit Test Pattern

Based on our working `test_math.cpp` example:

```cpp
#include <iostream>
#include <cassert>
#include <cmath>
#include "Core/Math.h"

using namespace GameEngine;

// Helper function for floating point comparison
bool IsNearlyEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

// Test function - returns bool, uses assert for validation
bool TestVectorOperations() {
    std::cout << "Testing vector operations..." << std::endl;

    // Test setup
    Math::Vec3 a(1.0f, 2.0f, 3.0f);
    Math::Vec3 b(4.0f, 5.0f, 6.0f);

    // Test execution
    Math::Vec3 result = a + b;

    // Validation with assert
    assert(IsNearlyEqual(result.x, 5.0f));
    assert(IsNearlyEqual(result.y, 7.0f));
    assert(IsNearlyEqual(result.z, 9.0f));

    // Success output
    std::cout << "  [PASS] Vector operations passed" << std::endl;
    return true;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Game Engine Kiro - Math Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    bool allPassed = true;

    try {
        allPassed &= TestVectorOperations();
        // Add more tests here

        std::cout << "========================================" << std::endl;
        if (allPassed) {
            std::cout << "[SUCCESS] ALL TESTS PASSED!" << std::endl;
            return 0;
        } else {
            std::cout << "[FAILED] SOME TESTS FAILED!" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << "[ERROR] TEST EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "[ERROR] UNKNOWN TEST ERROR!" << std::endl;
        return 1;
    }
}
```

### CMakeLists.txt Integration

```cmake
# Add unit test executable
add_executable(MathUnitTest
    tests/unit/test_math.cpp
)
apply_coverage_settings(MathUnitTest)

# Link with engine library
target_link_libraries(MathUnitTest PRIVATE GameEngineKiro)
```

## File Organization Standards

### Test Files

- **Unit tests**: `tests/unit/test_[component].cpp`
- **Integration tests**: `tests/integration/test_[system]_[type].cpp`
- **Visual tests**: Embedded in engine via `TestRunner`

### Naming Conventions

- **Test functions**: `TestComponentFeature()` (e.g., `TestVectorOperations()`)
- **Test files**: `test_component.cpp` (e.g., `test_math.cpp`)
- **Test executables**: `ComponentUnitTest` (e.g., `MathUnitTest`)

## Documentation Standards

### No Icons in Documentation

**RULE: Documentation should use clear text headers and bullet points instead of Unicode icons.**

```markdown
<!-- CORRECT -->

## Quick Start

### Running Tests

- Build the project
- Execute test binaries
- Review output

<!-- INCORRECT -->

## 🚀 Quick Start

### 🧪 Running Tests

- ✅ Build the project
- 🎯 Execute test binaries
- 📊 Review output
```

## Benefits of This Standard

1. **Cross-Platform Compatibility**: Works on all systems and terminals
2. **Professional Appearance**: Clean, consistent output
3. **CI/CD Friendly**: No encoding issues in automated systems
4. **Accessibility**: Screen readers can properly interpret text
5. **Maintainability**: Easy to search and filter log output
6. **Consistency**: Uniform appearance across all components

## Enforcement

- **Code Reviews**: Check for Unicode characters in output
- **Build Scripts**: Can add checks for forbidden characters
- **Documentation**: Update all existing docs to follow this standard
- **Examples**: All example code should demonstrate proper usage

This standard ensures our engine maintains a professional, consistent, and compatible codebase across all platforms and development environments.
