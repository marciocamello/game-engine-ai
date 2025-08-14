# Namespace Class Conflict Resolution - Migration Guide

This document provides a comprehensive guide for migrating code that uses the old conflicting class names to the new, unambiguous class names introduced in the namespace class conflict resolution refactoring.

## 🎯 Overview

The Game Engine Kiro project underwent a major refactoring to resolve critical namespace and class name conflicts that were causing compilation errors and symbol resolution issues. This migration guide documents all the changes made and provides guidance for updating existing code.

## 📋 Summary of Changes

### Class Name Changes

| Old Class Name                     | New Class Name                             | Location                                | Purpose                            |
| ---------------------------------- | ------------------------------------------ | --------------------------------------- | ---------------------------------- |
| `GameEngine::Animation::Animation` | `GameEngine::Animation::SkeletalAnimation` | `include/Animation/SkeletalAnimation.h` | Skeletal animation for gameplay    |
| `GameEngine::Graphics::Animation`  | `GameEngine::Graphics::GraphicsAnimation`  | `include/Graphics/GraphicsAnimation.h`  | Node-based animation for rendering |
| `GameEngine::Animation::Skeleton`  | `GameEngine::Animation::AnimationSkeleton` | `include/Animation/AnimationSkeleton.h` | Animation bone hierarchy           |
| `GameEngine::Graphics::Skeleton`   | `GameEngine::Graphics::RenderSkeleton`     | `include/Graphics/RenderSkeleton.h`     | Rendering bone hierarchy           |

### Header File Changes

| Old Header File                 | New Header File                         | Purpose                   |
| ------------------------------- | --------------------------------------- | ------------------------- |
| `include/Animation/Animation.h` | `include/Animation/SkeletalAnimation.h` | Skeletal animation system |
| `include/Graphics/Animation.h`  | `include/Graphics/GraphicsAnimation.h`  | Graphics animation system |
| `include/Animation/Skeleton.h`  | `include/Animation/AnimationSkeleton.h` | Animation skeleton system |
| `include/Graphics/Skeleton.h`   | `include/Graphics/RenderSkeleton.h`     | Render skeleton system    |

### Implementation File Changes

| Old Implementation File       | New Implementation File               |
| ----------------------------- | ------------------------------------- |
| `src/Animation/Animation.cpp` | `src/Animation/SkeletalAnimation.cpp` |
| `src/Graphics/Animation.cpp`  | `src/Graphics/GraphicsAnimation.cpp`  |
| `src/Animation/Skeleton.cpp`  | `src/Animation/AnimationSkeleton.cpp` |
| `src/Graphics/Skeleton.cpp`   | `src/Graphics/RenderSkeleton.cpp`     |

## 🔄 Migration Steps

### Step 1: Update Include Statements

Replace all old include statements with the new ones:

```cpp
// OLD - Replace these includes
#include "Animation/Animation.h"
#include "Graphics/Animation.h"
#include "Animation/Skeleton.h"
#include "Graphics/Skeleton.h"

// NEW - Use these includes instead
#include "Animation/SkeletalAnimation.h"
#include "Graphics/GraphicsAnimation.h"
#include "Animation/AnimationSkeleton.h"
#include "Graphics/RenderSkeleton.h"
```

### Step 2: Update Class Names in Code

Replace all class name references:

```cpp
// OLD - Animation system classes
GameEngine::Animation::Animation oldAnim;
GameEngine::Animation::Skeleton oldSkeleton;

// NEW - Animation system classes
GameEngine::Animation::SkeletalAnimation newAnim;
GameEngine::Animation::AnimationSkeleton newSkeleton;

// OLD - Graphics system classes
GameEngine::Graphics::Animation oldGraphicsAnim;
GameEngine::Graphics::Skeleton oldRenderSkeleton;

// NEW - Graphics system classes
GameEngine::Graphics::GraphicsAnimation newGraphicsAnim;
GameEngine::Graphics::RenderSkeleton newRenderSkeleton;
```

### Step 3: Update Type Declarations

Update all type declarations, function parameters, and return types:

```cpp
// OLD - Function parameters and return types
std::shared_ptr<GameEngine::Animation::Animation> LoadAnimation();
void ProcessSkeleton(std::shared_ptr<GameEngine::Animation::Skeleton> skeleton);

// NEW - Function parameters and return types
std::shared_ptr<GameEngine::Animation::SkeletalAnimation> LoadAnimation();
void ProcessSkeleton(std::shared_ptr<GameEngine::Animation::AnimationSkeleton> skeleton);
```

### Step 4: Update Member Variables

Update all member variable declarations:

```cpp
class AnimationController {
private:
    // OLD - Member variables
    std::shared_ptr<GameEngine::Animation::Animation> m_currentAnimation;
    std::shared_ptr<GameEngine::Animation::Skeleton> m_skeleton;

    // NEW - Member variables
    std::shared_ptr<GameEngine::Animation::SkeletalAnimation> m_currentAnimation;
    std::shared_ptr<GameEngine::Animation::AnimationSkeleton> m_skeleton;
};
```

### Step 5: Update Template Instantiations

Update template instantiations and type aliases:

```cpp
// OLD - Template instantiations
std::vector<std::shared_ptr<GameEngine::Animation::Animation>> animations;
std::unordered_map<std::string, GameEngine::Animation::Skeleton*> skeletons;

// NEW - Template instantiations
std::vector<std::shared_ptr<GameEngine::Animation::SkeletalAnimation>> animations;
std::unordered_map<std::string, GameEngine::Animation::AnimationSkeleton*> skeletons;

// OLD - Type aliases
using AnimationType = GameEngine::Animation::Animation;
using SkeletonType = GameEngine::Animation::Skeleton;

// NEW - Type aliases
using AnimationType = GameEngine::Animation::SkeletalAnimation;
using SkeletonType = GameEngine::Animation::AnimationSkeleton;
```

## 🧪 Testing Migration

### Unit Tests

Update all unit test files to use the new class names:

```cpp
// OLD - Test file: tests/unit/test_animation.cpp
#include "Animation/Animation.h"
bool TestAnimationCreation() {
    GameEngine::Animation::Animation anim("test");
    // ...
}

// NEW - Test file: tests/unit/test_animation.cpp
#include "Animation/SkeletalAnimation.h"
bool TestAnimationCreation() {
    GameEngine::Animation::SkeletalAnimation anim("test");
    // ...
}
```

### Integration Tests

Update integration tests that use the renamed classes:

```cpp
// OLD - Integration test
#include "Animation/Animation.h"
#include "Graphics/Animation.h"

// NEW - Integration test
#include "Animation/SkeletalAnimation.h"
#include "Graphics/GraphicsAnimation.h"
```

## 🔍 Common Migration Issues

### Issue 1: Ambiguous Symbol Resolution

**Problem**: Compiler errors about ambiguous symbols.

**Solution**: Use fully qualified names or update using statements:

```cpp
// OLD - Ambiguous
using namespace GameEngine::Animation;
Animation anim;  // Ambiguous - which Animation?

// NEW - Clear
using namespace GameEngine::Animation;
SkeletalAnimation anim;  // Clear - skeletal animation

// OR use fully qualified names
GameEngine::Animation::SkeletalAnimation skelAnim;
GameEngine::Graphics::GraphicsAnimation gfxAnim;
```

### Issue 2: Missing Include Files

**Problem**: Compiler errors about missing header files.

**Solution**: Update all include statements to use new header names:

```cpp
// OLD - Will cause compilation error
#include "Animation/Animation.h"  // File no longer exists

// NEW - Correct include
#include "Animation/SkeletalAnimation.h"  // New file location
```

### Issue 3: Linker Errors

**Problem**: Linker errors about undefined symbols.

**Solution**: Ensure all implementation files are updated and compiled:

```cpp
// Make sure these files exist and are compiled:
// src/Animation/SkeletalAnimation.cpp
// src/Graphics/GraphicsAnimation.cpp
// src/Animation/AnimationSkeleton.cpp
// src/Graphics/RenderSkeleton.cpp
```

## 📚 Updated Documentation

The following documentation files have been updated to reflect the new class names:

- `docs/animation-system.md` - Updated all Animation and Skeleton references
- `docs/3d-model-loading.md` - Updated graphics animation references
- This migration guide - Documents all changes

## 🔧 Build System Changes

### CMakeLists.txt Updates

The build system has been updated to:

1. Remove references to old `examples/` directory executables
2. Update source file references to new implementation files
3. Ensure all new files are included in the build

### Legacy Examples Cleanup

The legacy `examples/` directory has been cleaned up:

- All executable definitions removed from CMakeLists.txt
- Build conflicts with `projects/` directory resolved
- Old example files either moved to `projects/` or marked as deprecated

## ✅ Verification Checklist

Use this checklist to verify your migration is complete:

### Code Updates

- [ ] All `#include` statements updated to new header names
- [ ] All class name references updated
- [ ] All type declarations updated
- [ ] All member variables updated
- [ ] All function parameters and return types updated
- [ ] All template instantiations updated

### Build Verification

- [ ] Project compiles without errors
- [ ] All unit tests compile and pass
- [ ] All integration tests compile and pass
- [ ] No linker errors or undefined symbols
- [ ] No namespace conflicts remain

### Testing

- [ ] All existing functionality works correctly
- [ ] No regressions in performance
- [ ] All tests pass with new class names
- [ ] Animation system works correctly
- [ ] Graphics system works correctly

## 🚀 Benefits of Migration

After completing this migration, you will benefit from:

1. **No Namespace Conflicts**: Clear separation between animation and graphics systems
2. **Better Code Clarity**: Class names clearly indicate their purpose
3. **Improved Maintainability**: Easier to understand and modify code
4. **Compilation Speed**: Faster builds due to resolved conflicts
5. **Future-Proof**: Ready for additional animation and graphics features

## 🆘 Troubleshooting

### Common Compilation Errors

**Error**: `'Animation' is ambiguous`
**Solution**: Use fully qualified names or update to new class names

**Error**: `Cannot find 'Animation/Animation.h'`
**Solution**: Update include to `Animation/SkeletalAnimation.h`

**Error**: `Undefined reference to Animation::Animation()`
**Solution**: Update to `SkeletalAnimation::SkeletalAnimation()`

### Getting Help

If you encounter issues during migration:

1. Check this migration guide for common solutions
2. Verify all include statements are updated
3. Ensure all class names are updated consistently
4. Run a clean build to resolve any cached issues
5. Check that all new implementation files are being compiled

## 📝 Migration Script

For large codebases, consider using find-and-replace operations:

```bash
# Find and replace include statements
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/#include "Animation\/Animation.h"/#include "Animation\/SkeletalAnimation.h"/g'
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/#include "Graphics\/Animation.h"/#include "Graphics\/GraphicsAnimation.h"/g'
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/#include "Animation\/Skeleton.h"/#include "Animation\/AnimationSkeleton.h"/g'
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/#include "Graphics\/Skeleton.h"/#include "Graphics\/RenderSkeleton.h"/g'

# Find and replace class names (be careful with these - review changes)
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/GameEngine::Animation::Animation/GameEngine::Animation::SkeletalAnimation/g'
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/GameEngine::Graphics::Animation/GameEngine::Graphics::GraphicsAnimation/g'
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/GameEngine::Animation::Skeleton/GameEngine::Animation::AnimationSkeleton/g'
find . -name "*.cpp" -o -name "*.h" | xargs sed -i 's/GameEngine::Graphics::Skeleton/GameEngine::Graphics::RenderSkeleton/g'
```

**Note**: Always review automated changes carefully and test thoroughly after running any migration scripts.

---

This migration guide provides comprehensive information for updating code to use the new, conflict-free class names. Following these steps will ensure a smooth transition to the improved namespace structure.

**Game Engine Kiro** - Cleaner namespaces, better code organization.
