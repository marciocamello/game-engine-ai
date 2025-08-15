---
inclusion: always
---

# Naming Conventions - Always Applied

## MANDATORY Rules - Zero Tolerance

### English Only

- ALL code, comments, variables, functions, classes MUST be in English
- NO Portuguese, Spanish, or other languages in any code element
- Technical terms use standard English programming terminology

### Symbol Uniqueness - CRITICAL

- EVERY class name MUST be globally unique across entire project
- EVERY namespace MUST be unique and descriptive
- EVERY macro MUST have GAMEENGINE* or GE* prefix
- NO duplicate symbols anywhere in codebase

### Naming Patterns - MANDATORY

#### Classes

```cpp
// ✅ CORRECT: Unique, descriptive
class AnimationController { };
class SkeletalAnimationPlayer { };
class OpenGLRenderer { };
class BulletPhysicsEngine { };

// ❌ FORBIDDEN: Generic, conflicts
class Controller { };
class Animation { };  // Too generic
class Manager { };    // Without context
```

#### Namespaces

```cpp
// ✅ CORRECT: Specific hierarchy
namespace GameEngine::Animation::SkeletalAnimation { }
namespace GameEngine::Graphics::OpenGL { }

// ❌ FORBIDDEN: Generic names
namespace Utils { }
namespace Common { }
namespace Base { }
```

#### Variables

```cpp
// ✅ CORRECT: Descriptive
float animationSpeed;
bool isInitialized;
std::string currentStateName;

// ❌ FORBIDDEN: Unclear
float speed;    // Speed of what?
bool flag;      // Flag for what?
std::string name;  // Name of what?
```

### Pre-Development Verification - MANDATORY

Before creating ANY new symbol:

1. Search entire codebase for conflicts: `grep -r "ClassName" include/ src/`
2. Verify namespace uniqueness
3. Check macro name availability
4. Ensure descriptive, English-only naming

### Violation = Task Failure

- Any naming convention violation = IMMEDIATE task failure
- Any symbol conflict = IMMEDIATE refactoring required
- Any non-English naming = IMMEDIATE correction required

This is the MINIMUM standard for professional C++ development.
