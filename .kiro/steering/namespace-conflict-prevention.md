# Namespace and Symbol Conflict Prevention - MANDATORY RULES

## 🚨 ZERO TOLERANCE POLICY

**Game Engine Kiro enforces ZERO TOLERANCE for namespace, class, macro, or symbol conflicts. This is the MINIMUM standard for ANY professional C++ project.**

## MANDATORY PRE-DEVELOPMENT VERIFICATION

### Before Creating ANY New Symbol

**EVERY developer MUST perform these checks before creating any new class, namespace, macro, function, or global symbol:**

```powershell
# 1. Search for existing class names
grep -r "class YourClassName" include/ src/
grep -r "YourClassName" include/ src/ tests/

# 2. Search for existing namespace names
grep -r "namespace YourNamespace" include/ src/
grep -r "YourNamespace::" include/ src/

# 3. Search for existing macro names
grep -r "#define YOUR_MACRO" include/ src/
grep -r "YOUR_MACRO" include/ src/

# 4. Search for existing function names
grep -r "YourFunctionName" include/ src/
```

**IF ANY RESULTS FOUND = CHOOSE DIFFERENT NAME**

## NAMESPACE RULES - MANDATORY

### Uniqueness Requirements

- **EVERY namespace MUST be globally unique across entire project**
- **NO duplicate namespace names allowed anywhere**
- **NO generic namespace names allowed**

### Forbidden Namespace Names

```cpp
// FORBIDDEN - Generic names
namespace Utils { }        // TOO GENERIC
namespace Common { }       // TOO GENERIC
namespace Base { }         // TOO GENERIC
namespace Helper { }       // TOO GENERIC
namespace Manager { }      // TOO GENERIC

// FORBIDDEN - Duplicate purposes
namespace GameEngine::Animation::Animation { }  // DUPLICATE NAME
namespace GameEngine::Graphics::Animation { }   // CONFLICT WITH ABOVE
```

### Required Namespace Patterns

```cpp
// CORRECT - Unique, descriptive, hierarchical
namespace GameEngine::Animation::SkeletalAnimation { }
namespace GameEngine::Graphics::GraphicsAnimation { }
namespace GameEngine::Animation::AnimationSkeleton { }
namespace GameEngine::Graphics::RenderSkeleton { }

// CORRECT - Clear purpose hierarchy
namespace GameEngine::Physics::BulletPhysics { }
namespace GameEngine::Physics::PhysXPhysics { }
namespace GameEngine::Audio::OpenALAudio { }
namespace GameEngine::Input::GLFWInput { }
```

## CLASS NAME RULES - MANDATORY

### Global Uniqueness Requirement

- **EVERY class name MUST be globally unique**
- **NO two classes can have the same name, even in different namespaces**
- **Class names MUST clearly indicate their specific purpose**

### Forbidden Class Patterns

```cpp
// FORBIDDEN - Duplicate class names
namespace GameEngine::Animation {
    class Animation { };  // FORBIDDEN
}
namespace GameEngine::Graphics {
    class Animation { };  // CONFLICT - SAME NAME
}

// FORBIDDEN - Generic class names
class Manager { };        // TOO GENERIC
class Handler { };        // TOO GENERIC
class Controller { };     // TOO GENERIC (without context)
class Processor { };      // TOO GENERIC
```

### Required Class Patterns

```cpp
// CORRECT - Unique, descriptive names
namespace GameEngine::Animation {
    class SkeletalAnimation { };      // UNIQUE, DESCRIPTIVE
    class AnimationSkeleton { };      // UNIQUE, DESCRIPTIVE
    class AnimationStateMachine { };  // UNIQUE, DESCRIPTIVE
}

namespace GameEngine::Graphics {
    class GraphicsAnimation { };      // UNIQUE, DESCRIPTIVE
    class RenderSkeleton { };         // UNIQUE, DESCRIPTIVE
    class GraphicsRenderer { };       // UNIQUE, DESCRIPTIVE
}
```

## MACRO RULES - MANDATORY

### Prefix Requirements

- **ALL macros MUST have project-specific prefix**
- **NEVER use generic macro names**
- **ALWAYS use `GAMEENGINE_` or `GE_` prefix**

### Forbidden Macro Patterns

```cpp
// FORBIDDEN - No prefix, generic names
#define MAX_SIZE 1024           // FORBIDDEN
#define MIN_VALUE 0             // FORBIDDEN
#define ASSERT(x) assert(x)     // FORBIDDEN
#define ERROR -1                // FORBIDDEN
```

### Required Macro Patterns

```cpp
// CORRECT - Project prefix, descriptive names
#define GAMEENGINE_MAX_ANIMATION_BONES 256
#define GAMEENGINE_MAX_TEXTURE_SIZE 4096
#define GE_ASSERT(x) assert(x)
#define GAMEENGINE_VERSION_MAJOR 1
#define GAMEENGINE_AUDIO_SAMPLE_RATE 44100
```

## SYMBOL RESOLUTION RULES - MANDATORY

### Header File Rules

```cpp
// FORBIDDEN - Using directives in headers
#include "SomeHeader.h"
using namespace GameEngine;          // FORBIDDEN IN HEADERS
using namespace std;                 // FORBIDDEN IN HEADERS

class MyClass {
    Animation* anim;                 // AMBIGUOUS - WHICH Animation?
};
```

```cpp
// CORRECT - Fully qualified names in headers
#include "Animation/SkeletalAnimation.h"

class MyClass {
    GameEngine::Animation::SkeletalAnimation* anim;  // CLEAR, UNAMBIGUOUS
};
```

### Implementation File Rules

```cpp
// ACCEPTABLE - Using directives in .cpp files only
#include "MyClass.h"
using namespace GameEngine::Animation;  // OK in .cpp files

void MyClass::Update() {
    SkeletalAnimation anim;  // OK - namespace specified above
}
```

## CONFLICT DETECTION WORKFLOW - MANDATORY

### Pre-Implementation Checklist

**BEFORE writing ANY code:**

1. **[ ] Search entire codebase for name conflicts**
2. **[ ] Verify namespace uniqueness**
3. **[ ] Check macro name availability**
4. **[ ] Ensure class name is globally unique**
5. **[ ] Verify no ambiguous symbols will be created**

### Implementation Verification

**DURING implementation:**

1. **[ ] Use fully qualified names in headers**
2. **[ ] Avoid using directives in headers**
3. **[ ] Test compilation with all modules**
4. **[ ] Verify no linker conflicts**

### Post-Implementation Validation

**AFTER implementation:**

1. **[ ] Full project compilation succeeds**
2. **[ ] All tests pass**
3. **[ ] No symbol resolution warnings**
4. **[ ] No namespace conflicts detected**

## VIOLATION CONSEQUENCES - IMMEDIATE ACTION

### Automatic Task Failure

- **ANY namespace conflict** = IMMEDIATE task failure
- **ANY class name conflict** = IMMEDIATE task failure
- **ANY macro conflict** = IMMEDIATE task failure
- **ANY symbol ambiguity** = IMMEDIATE task failure

### Required Actions

1. **STOP all development immediately**
2. **Identify and resolve ALL conflicts**
3. **Rename conflicting symbols**
4. **Update all references**
5. **Verify full project compilation**
6. **Run complete test suite**
7. **ONLY THEN continue with original task**

## PROFESSIONAL STANDARDS

### Why This Matters

- **Symbol conflicts cause compilation failures**
- **Ambiguous names lead to maintenance nightmares**
- **Generic names make code unreadable**
- **Namespace conflicts break modular design**
- **This is BASIC professional C++ development**

### Industry Standards

- **ALL professional C++ projects enforce these rules**
- **NO exceptions for "quick fixes" or "temporary code"**
- **Symbol uniqueness is NON-NEGOTIABLE**
- **Proper namespacing is FUNDAMENTAL**

## ENFORCEMENT

### Automated Checks

```powershell
# Run before every commit
.\scripts\check_symbol_conflicts.bat

# Verify namespace uniqueness
.\scripts\verify_namespaces.bat

# Check macro conflicts
.\scripts\check_macro_conflicts.bat
```

### Manual Verification

- **Code reviews MUST verify symbol uniqueness**
- **Pull requests MUST pass conflict detection**
- **NO code merges without verification**

---

**REMEMBER: This is the MINIMUM standard for professional C++ development. Conflicts are NOT acceptable under ANY circumstances.**
