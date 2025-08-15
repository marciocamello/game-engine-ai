# Game Engine Kiro - Naming Conventions

## Overview

This document establishes comprehensive naming conventions for Game Engine Kiro to ensure consistency, readability, and maintainability across the entire codebase. These conventions are **MANDATORY** and must be followed by all contributors.

## Core Principles

### 1. English Only

- **ALL** names, comments, documentation, and identifiers MUST be in English
- **NO** Portuguese, Spanish, or any other language in code
- Technical terms should use standard English programming terminology

### 2. Descriptive and Clear

- Names should clearly indicate purpose and functionality
- Avoid abbreviations unless they are widely understood (e.g., `ID`, `URL`, `HTTP`)
- Use full words rather than shortened forms

### 3. Consistent Patterns

- Follow established patterns throughout the codebase
- Use the same naming style for similar concepts
- Maintain consistency within modules and across the engine

## File and Directory Naming

### Directory Structure

```
GameEngineKiro/
├── include/                    # PascalCase for modules
│   ├── Animation/             # ✅ Module directories
│   ├── Graphics/              # ✅ Clear, descriptive
│   ├── Physics/               # ✅ Single responsibility
│   └── Audio/                 # ✅ Standard terminology
├── src/                       # Mirror include structure
├── projects/                  # lowercase for top-level
│   ├── GameExample/           # PascalCase for project names
│   └── BasicExample/          # ✅ Descriptive project names
├── assets/                    # lowercase for resources
├── docs/                      # lowercase for documentation
└── tests/                     # lowercase for test directories
```

### File Naming Conventions

#### Header Files (.h)

```cpp
// ✅ CORRECT: PascalCase, descriptive
AnimationController.h
GraphicsRenderer.h
PhysicsEngine.h
ResourceManager.h
ThirdPersonCamera.h

// ❌ WRONG: Abbreviations, unclear purpose
AnimCtrl.h
GfxRend.h
PhysEng.h
ResMgr.h
TPCam.h
```

#### Source Files (.cpp)

```cpp
// ✅ CORRECT: Match header names exactly
AnimationController.cpp
GraphicsRenderer.cpp
PhysicsEngine.cpp
ResourceManager.cpp
ThirdPersonCamera.cpp
```

#### Test Files

```cpp
// ✅ CORRECT: test_ prefix, descriptive
test_animation_controller.cpp
test_graphics_renderer.cpp
test_physics_engine.cpp
test_resource_manager.cpp

// Test executables: PascalCase + Test suffix
AnimationControllerTest.exe
GraphicsRendererTest.exe
PhysicsEngineTest.exe
ResourceManagerTest.exe
```

## Code Naming Conventions

### Namespaces

#### Hierarchy Rules

```cpp
// ✅ CORRECT: Clear hierarchy, unique names
namespace GameEngine {
    namespace Animation {
        namespace SkeletalAnimation {
            class AnimationController { };
        }
        namespace BlendTrees {
            class BlendTreeController { };
        }
    }
    namespace Graphics {
        namespace OpenGL {
            class OpenGLRenderer { };
        }
        namespace Vulkan {
            class VulkanRenderer { };
        }
    }
}

// ❌ WRONG: Generic names, conflicts
namespace GameEngine {
    namespace Animation {
        namespace Animation {  // ❌ Duplicate name
            class Controller { };  // ❌ Too generic
        }
    }
    namespace Graphics {
        namespace Animation {  // ❌ Conflicts with above
            class Controller { };  // ❌ Same name, different namespace
        }
    }
}
```

#### Forbidden Namespace Names

```cpp
// ❌ FORBIDDEN: Too generic, cause conflicts
namespace Utils { }
namespace Common { }
namespace Base { }
namespace Helper { }
namespace Manager { }
namespace Handler { }
namespace Controller { }  // Without context
namespace System { }
namespace Core { }        // Unless it's THE core module
```

#### Required Namespace Patterns

```cpp
// ✅ CORRECT: Specific, descriptive, unique
namespace GameEngine::Animation::SkeletalAnimation { }
namespace GameEngine::Animation::BlendTrees { }
namespace GameEngine::Animation::InverseKinematics { }
namespace GameEngine::Graphics::OpenGL { }
namespace GameEngine::Graphics::Vulkan { }
namespace GameEngine::Physics::BulletPhysics { }
namespace GameEngine::Physics::PhysXPhysics { }
namespace GameEngine::Audio::OpenAL { }
namespace GameEngine::Resource::AssetLoading { }
```

### Class Names

#### Naming Rules

```cpp
// ✅ CORRECT: Unique, descriptive, clear purpose
class AnimationController { };        // Controls animations
class SkeletalAnimationPlayer { };    // Plays skeletal animations
class BlendTreeEvaluator { };         // Evaluates blend trees
class OpenGLRenderer { };             // OpenGL-specific renderer
class BulletPhysicsEngine { };        // Bullet Physics implementation
class ResourceManager { };            // Manages resources
class ThirdPersonCamera { };          // Third-person camera
class SpringArmComponent { };         // Spring arm for cameras

// ❌ WRONG: Generic, ambiguous, conflicts
class Animation { };                  // Too generic
class Controller { };                 // What does it control?
class Manager { };                    // What does it manage?
class Engine { };                     // Which engine?
class Player { };                     // Audio player? Animation player?
class Camera { };                     // What type of camera?
```

#### Class Naming Patterns

**Controllers and Managers:**

```cpp
// ✅ CORRECT: Specific purpose
class AnimationController { };        // Controls animation system
class ResourceManager { };            // Manages resources
class InputManager { };               // Manages input
class AudioManager { };               // Manages audio system

// ❌ WRONG: Generic without context
class Controller { };                 // Controls what?
class Manager { };                    // Manages what?
```

**Renderers and Engines:**

```cpp
// ✅ CORRECT: Technology-specific
class OpenGLRenderer { };             // OpenGL implementation
class VulkanRenderer { };             // Vulkan implementation
class BulletPhysicsEngine { };        // Bullet Physics implementation
class PhysXPhysicsEngine { };         // PhysX implementation

// ❌ WRONG: Generic base names
class Renderer { };                   // Which renderer?
class PhysicsEngine { };              // Which physics engine?
```

**Components and Systems:**

```cpp
// ✅ CORRECT: Clear functionality
class SpringArmComponent { };         // Spring arm component
class ThirdPersonCameraSystem { };    // Third-person camera system
class SkeletalMeshComponent { };       // Skeletal mesh component
class RigidBodyComponent { };          // Rigid body physics component

// ❌ WRONG: Ambiguous purpose
class Component { };                  // What kind of component?
class System { };                     // What kind of system?
```

### Method and Function Names

#### Method Naming Rules

```cpp
class AnimationController {
public:
    // ✅ CORRECT: PascalCase, descriptive verbs
    bool Initialize();
    void Update(float deltaTime);
    void Shutdown();

    void PlayAnimation(const std::string& name);
    void StopAnimation(const std::string& name);
    void PauseAnimation();
    void ResumeAnimation();

    void SetAnimationSpeed(float speed);
    float GetAnimationSpeed() const;

    void SetParameter(const std::string& name, float value);
    float GetParameter(const std::string& name) const;

    bool IsAnimationPlaying(const std::string& name) const;
    bool IsInitialized() const;

    // ❌ WRONG: Unclear, abbreviated, inconsistent
    bool Init();                      // Abbreviated
    void Upd(float dt);              // Abbreviated parameters
    void Stop();                     // Stop what?
    void Set(const std::string& n, float v);  // Unclear parameters
    bool IsPlaying();                // Playing what?
};
```

#### Function Naming Patterns

**Getters and Setters:**

```cpp
// ✅ CORRECT: Clear Get/Set pattern
void SetPosition(const Math::Vec3& position);
Math::Vec3 GetPosition() const;

void SetRotation(const Math::Quat& rotation);
Math::Quat GetRotation() const;

void SetScale(const Math::Vec3& scale);
Math::Vec3 GetScale() const;

// ❌ WRONG: Inconsistent patterns
void Position(const Math::Vec3& pos);     // Unclear if getter or setter
Math::Vec3 Pos() const;                   // Abbreviated
void SetRot(const Math::Quat& r);         // Abbreviated
```

**Boolean Methods:**

```cpp
// ✅ CORRECT: Clear boolean indicators
bool IsInitialized() const;
bool IsPlaying() const;
bool IsLoaded() const;
bool HasAnimation(const std::string& name) const;
bool CanPlay() const;
bool ShouldUpdate() const;

// ❌ WRONG: Unclear boolean nature
bool Initialized() const;             // Not clearly a boolean
bool Playing() const;                 // Could be a noun
bool Animation(const std::string& name) const;  // Unclear
```

### Variable Names

#### Member Variables

```cpp
class AnimationController {
private:
    // ✅ CORRECT: m_ prefix, descriptive camelCase
    std::shared_ptr<AnimationStateMachine> m_stateMachine;
    std::shared_ptr<Skeleton> m_skeleton;
    std::unordered_map<std::string, float> m_parameters;
    float m_playbackSpeed;
    bool m_isInitialized;
    bool m_isPaused;

    // Animation timing
    float m_currentTime;
    float m_deltaTime;
    float m_totalDuration;

    // State management
    std::string m_currentStateName;
    std::string m_previousStateName;
    float m_transitionTime;

    // ❌ WRONG: No prefix, unclear names
    AnimationStateMachine* stateMachine;  // No m_ prefix
    Skeleton* skel;                       // Abbreviated
    std::unordered_map<std::string, float> params;  // Abbreviated
    float speed;                          // Unclear what speed
    bool init;                            // Abbreviated
    bool paused;                          // No clear ownership
};
```

#### Local Variables

```cpp
void AnimationController::Update(float deltaTime) {
    // ✅ CORRECT: Descriptive camelCase
    float normalizedTime = m_currentTime / m_totalDuration;
    std::string currentStateName = m_stateMachine->GetCurrentStateName();
    bool shouldTransition = CheckTransitionConditions();

    for (const auto& animationPair : m_loadedAnimations) {
        const std::string& animationName = animationPair.first;
        const Animation& animation = animationPair.second;

        if (animation.IsPlaying()) {
            animation.Update(deltaTime);
        }
    }

    // ❌ WRONG: Unclear, abbreviated
    float t = m_currentTime / m_totalDuration;  // What is 't'?
    std::string state = m_stateMachine->GetCurrentStateName();  // Generic
    bool trans = CheckTransitionConditions();  // Abbreviated

    for (const auto& pair : m_loadedAnimations) {  // Generic names
        const std::string& name = pair.first;      // Too generic
        const Animation& anim = pair.second;       // Abbreviated
    }
}
```

#### Parameter Names

```cpp
// ✅ CORRECT: Descriptive parameter names
void LoadAnimation(const std::string& animationName, const std::string& filePath);
void SetBlendWeight(const std::string& stateName, float blendWeight);
void TransitionToState(const std::string& fromState, const std::string& toState, float transitionDuration);
bool PlayAnimationWithFade(const std::string& animationName, float fadeInTime, float fadeOutTime);

// ❌ WRONG: Unclear, abbreviated parameters
void LoadAnimation(const std::string& name, const std::string& path);  // Generic
void SetBlendWeight(const std::string& state, float weight);           // Generic
void TransitionToState(const std::string& from, const std::string& to, float time);  // Abbreviated
bool PlayAnimationWithFade(const std::string& anim, float in, float out);  // Unclear
```

### Constants and Enums

#### Constants

```cpp
// ✅ CORRECT: UPPER_SNAKE_CASE with descriptive names
namespace GameEngine::Animation {
    constexpr float DEFAULT_ANIMATION_SPEED = 1.0f;
    constexpr float MAX_BLEND_WEIGHT = 1.0f;
    constexpr float MIN_BLEND_WEIGHT = 0.0f;
    constexpr int MAX_ANIMATION_LAYERS = 8;
    constexpr int MAX_BONE_COUNT = 256;
    constexpr float DEFAULT_TRANSITION_DURATION = 0.3f;
    constexpr float ANIMATION_TIME_EPSILON = 0.001f;
}

// ❌ WRONG: Generic names, unclear purpose
constexpr float DEFAULT_SPEED = 1.0f;        // Speed of what?
constexpr float MAX_WEIGHT = 1.0f;           // Weight of what?
constexpr int MAX_COUNT = 256;               // Count of what?
constexpr float EPSILON = 0.001f;            // Epsilon for what?
```

#### Enumerations

```cpp
// ✅ CORRECT: Descriptive enum names and values
enum class AnimationState {
    Idle,
    Walking,
    Running,
    Jumping,
    Falling,
    Landing,
    Attacking,
    Defending,
    Dying,
    Celebrating
};

enum class BlendMode {
    Replace,
    Additive,
    Multiply,
    Overlay,
    Screen
};

enum class InterpolationType {
    Linear,
    Cubic,
    Hermite,
    Bezier,
    Step
};

// ❌ WRONG: Generic names, unclear values
enum class State {          // State of what?
    A, B, C, D             // Meaningless names
};

enum class Mode {           // Mode of what?
    Type1, Type2, Type3    // Unclear purpose
};
```

### Macros

#### Macro Naming Rules

```cpp
// ✅ CORRECT: Project prefix, descriptive names
#define GAMEENGINE_MAX_ANIMATION_BONES 256
#define GAMEENGINE_MAX_TEXTURE_SIZE 4096
#define GAMEENGINE_DEFAULT_FRAME_RATE 60.0f
#define GAMEENGINE_ANIMATION_EPSILON 0.001f
#define GAMEENGINE_VERSION_MAJOR 1
#define GAMEENGINE_VERSION_MINOR 0
#define GAMEENGINE_VERSION_PATCH 0

// Debug macros
#define GAMEENGINE_ASSERT(condition) assert(condition)
#define GAMEENGINE_LOG_ERROR(message) LOG_ERROR(message)
#define GAMEENGINE_LOG_WARNING(message) LOG_WARNING(message)

// ❌ WRONG: No prefix, generic names
#define MAX_BONES 256                    // Could conflict with other libraries
#define MAX_SIZE 4096                    // Too generic
#define EPSILON 0.001f                   // Common name, likely to conflict
#define ASSERT(condition) assert(condition)  // Conflicts with system macros
```

## Asset Naming Conventions

### 3D Models and Animations

```
projects/GameExample/assets/models/
├── characters/
│   ├── XBot.fbx                    # ✅ Character model
│   ├── XBot_Idle.fbx              # ✅ Character + animation type
│   ├── XBot_Walk.fbx              # ✅ Clear animation purpose
│   ├── XBot_Run.fbx               # ✅ Descriptive action
│   ├── XBot_Jump.fbx              # ✅ Single action
│   ├── XBot_Attack_Sword.fbx      # ✅ Action + weapon type
│   └── XBot_Death_Forward.fbx     # ✅ Action + direction
├── environment/
│   ├── Building_Medieval_House.fbx     # ✅ Category + style + type
│   ├── Tree_Oak_Large.fbx             # ✅ Type + species + size
│   └── Rock_Granite_Boulder.fbx       # ✅ Type + material + shape
└── props/
    ├── Weapon_Sword_Iron.fbx          # ✅ Category + type + material
    ├── Container_Chest_Wooden.fbx     # ✅ Category + type + material
    └── Tool_Hammer_Blacksmith.fbx     # ✅ Category + type + purpose
```

### Textures

```
projects/GameExample/assets/textures/
├── characters/
│   ├── XBot_Diffuse.png           # ✅ Character + map type
│   ├── XBot_Normal.png            # ✅ Character + map type
│   ├── XBot_Roughness.png         # ✅ Character + PBR map
│   └── XBot_Metallic.png          # ✅ Character + PBR map
├── environment/
│   ├── Grass_Diffuse_1024.png     # ✅ Material + type + resolution
│   ├── Stone_Normal_2048.png      # ✅ Material + type + resolution
│   └── Wood_Roughness_512.png     # ✅ Material + type + resolution
└── ui/
    ├── Button_Default.png          # ✅ UI element + state
    ├── Button_Hover.png            # ✅ UI element + state
    └── Icon_Health.png             # ✅ UI type + purpose
```

### Audio Files

```
projects/GameExample/assets/audio/
├── sfx/
│   ├── Footstep_Grass_01.wav      # ✅ Action + surface + variation
│   ├── Footstep_Stone_02.wav      # ✅ Action + surface + variation
│   ├── Sword_Hit_Metal.wav        # ✅ Object + action + material
│   └── Jump_Land_Soft.wav         # ✅ Action + result + intensity
├── music/
│   ├── BGM_Menu_Peaceful.ogg      # ✅ Type + location + mood
│   ├── BGM_Combat_Intense.ogg     # ✅ Type + situation + mood
│   └── BGM_Victory_Triumphant.ogg # ✅ Type + event + mood
└── voice/
    ├── XBot_Attack_Grunt_01.wav   # ✅ Character + action + type + variation
    ├── XBot_Death_Scream.wav      # ✅ Character + event + type
    └── XBot_Jump_Effort.wav       # ✅ Character + action + type
```

## Configuration and Data Files

### JSON Configuration Files

```
projects/GameExample/config/
├── engine_config.json             # ✅ Clear purpose
├── graphics_settings.json         # ✅ Specific system
├── audio_settings.json            # ✅ Specific system
├── input_bindings.json            # ✅ Specific functionality
└── game_balance.json              # ✅ Game-specific data
```

### Shader Files

```
assets/shaders/
├── vertex/
│   ├── basic_vertex.glsl          # ✅ Type + purpose
│   ├── skinned_vertex.glsl        # ✅ Type + feature
│   └── terrain_vertex.glsl        # ✅ Type + use case
├── fragment/
│   ├── pbr_fragment.glsl          # ✅ Technique + type
│   ├── unlit_fragment.glsl        # ✅ Technique + type
│   └── water_fragment.glsl        # ✅ Material + type
└── compute/
    ├── particle_update.glsl       # ✅ System + operation
    └── culling_compute.glsl       # ✅ Operation + type
```

## Documentation Naming

### Documentation Files

```
docs/
├── api-reference.md               # ✅ Clear purpose
├── architecture.md                # ✅ Technical content
├── coding-standards.md            # ✅ Development guidelines
├── engine-naming-conventions.md   # ✅ This document
├── project-development-guide.md   # ✅ Specific guide
├── animation-system.md            # ✅ System documentation
├── physics-strategy.md            # ✅ Technical strategy
└── quickstart.md                  # ✅ Getting started guide
```

### README Files

```
# ✅ CORRECT: Descriptive README content
README.md                          # Main project README
projects/GameExample/README.md     # Project-specific README
assets/README.md                   # Asset documentation
docs/README.md                     # Documentation index

# Content should be descriptive:
# "Game Engine Kiro - Animation System Documentation"
# "GameExample Project - XBot Character Demo"
# "Asset Guidelines and Organization"
```

## Version Control and Build

### Branch Naming

```bash
# ✅ CORRECT: Descriptive branch names
feature/animation-state-machine
feature/physics-integration
bugfix/memory-leak-resource-manager
hotfix/crash-on-startup
refactor/namespace-cleanup
docs/api-reference-update

# ❌ WRONG: Unclear, abbreviated
feature/anim
fix/bug
update/stuff
dev/work
```

### Commit Messages

```bash
# ✅ CORRECT: Clear, descriptive commits
"Add AnimationController class with state machine support"
"Fix memory leak in ResourceManager texture loading"
"Refactor Physics namespace to avoid conflicts"
"Update API documentation for Animation system"
"Implement XBotCharacter in GameExample project"

# ❌ WRONG: Unclear, abbreviated
"Add anim stuff"
"Fix bug"
"Update code"
"Work in progress"
"Changes"
```

## Validation and Enforcement

### Pre-Development Checklist

Before creating any new symbol, **ALWAYS** verify:

1. **[ ] Name uniqueness**: Search entire codebase for conflicts
2. **[ ] Namespace hierarchy**: Ensure proper namespace structure
3. **[ ] Descriptive naming**: Name clearly indicates purpose
4. **[ ] English only**: No non-English words or abbreviations
5. **[ ] Consistent patterns**: Follows established conventions
6. **[ ] No generic names**: Avoid `Manager`, `Controller`, `System` without context

### Automated Checks

```powershell
# Search for naming conflicts before creating new symbols
grep -r "class YourClassName" include/ src/
grep -r "namespace YourNamespace" include/ src/
grep -r "#define YOUR_MACRO" include/ src/

# Verify naming conventions
.\scripts\check_naming_conventions.bat
.\scripts\validate_symbol_uniqueness.bat
```

### Code Review Requirements

All code reviews **MUST** verify:

1. **Naming consistency** with established conventions
2. **Symbol uniqueness** across entire project
3. **Descriptive names** that clearly indicate purpose
4. **Proper namespace hierarchy** without conflicts
5. **English-only** naming throughout

## Common Violations and Corrections

### Namespace Violations

```cpp
// ❌ VIOLATION: Generic namespace names
namespace Utils {
    class StringHelper { };
}

// ✅ CORRECTION: Specific, descriptive namespace
namespace GameEngine::Core::StringUtilities {
    class StringFormatter { };
}
```

### Class Name Violations

```cpp
// ❌ VIOLATION: Generic class names
class Manager { };
class Controller { };
class System { };

// ✅ CORRECTION: Specific, descriptive class names
class ResourceManager { };
class AnimationController { };
class RenderingSystem { };
```

### Method Name Violations

```cpp
// ❌ VIOLATION: Unclear method names
void Update();              // Update what?
void Set(float value);      // Set what?
bool Is();                  // Is what?

// ✅ CORRECTION: Clear, descriptive method names
void UpdateAnimation(float deltaTime);
void SetAnimationSpeed(float speed);
bool IsAnimationPlaying();
```

### Variable Name Violations

```cpp
// ❌ VIOLATION: Unclear variable names
float t;                    // Time? Temperature? Threshold?
int n;                      // Number? Node? Name?
bool flag;                  // Flag for what?

// ✅ CORRECTION: Descriptive variable names
float animationTime;
int boneCount;
bool isInitialized;
```

## Summary

These naming conventions ensure:

1. **Consistency** across the entire Game Engine Kiro codebase
2. **Clarity** for developers reading and maintaining code
3. **Uniqueness** to prevent symbol conflicts and compilation errors
4. **Professionalism** through standard English terminology
5. **Maintainability** through descriptive, self-documenting names

**Remember**: Good naming is not just about following rules—it's about making code that is easy to read, understand, and maintain. When in doubt, choose the more descriptive name.

All contributors **MUST** follow these conventions. Violations will result in code review rejection and required refactoring.
