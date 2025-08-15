# Game Engine Kiro - Project Development Guide

## Overview

Game Engine Kiro is designed as a modular game engine that supports multiple independent game projects. This guide explains how to properly develop game projects while maintaining clean separation from the base engine.

## Core Principle: Engine vs Project Separation

### The Golden Rule

**NEVER put project-specific code or assets in the base engine directories.**

The base engine provides generic, reusable components. Game projects extend and customize these components within their own project directories.

### Directory Structure Rules

```
GameEngineKiro/
├── include/                 # ✅ BASE ENGINE: Generic interfaces only
├── src/                     # ✅ BASE ENGINE: Generic implementations only
├── assets/                  # ✅ BASE ENGINE: Shared/fallback assets only
├── projects/                # ✅ PROJECTS: All project-specific code
│   ├── GameExample/         # ✅ PROJECT: XBot character example
│   ├── YourGame/            # ✅ PROJECT: Your custom game
│   └── AnotherGame/         # ✅ PROJECT: Another independent game
└── examples/                # ⚠️  TEMPORARY: Legacy examples (being migrated)
```

## Project Development Workflow

### 1. Creating a New Project

```powershell
# Create project directory structure
mkdir projects\YourGame
mkdir projects\YourGame\src
mkdir projects\YourGame\assets
mkdir projects\YourGame\assets\models
mkdir projects\YourGame\assets\textures
mkdir projects\YourGame\assets\audio
mkdir projects\YourGame\config
```

### 2. Project Structure Template

```
projects/YourGame/
├── CMakeLists.txt           # Build configuration
├── README.md               # Project documentation
├── src/                    # ALL project-specific code goes here
│   ├── main.cpp           # Project entry point (stays in src/)
│   ├── public/            # Public headers (.h files)
│   │   ├── YourCharacter.h    # Custom character class header
│   │   ├── YourGameLogic.h    # Game-specific logic header
│   │   └── ...                # Other public headers
│   ├── private/           # Private implementations (.cpp files)
│   │   ├── YourCharacter.cpp  # Custom character implementation
│   │   ├── YourGameLogic.cpp  # Game-specific implementation
│   │   └── ...                # Other implementation files
├── assets/                # ALL project-specific assets
│   ├── models/           # Project 3D models and animations
│   │   ├── YourCharacter.fbx
│   │   ├── Idle.fbx
│   │   ├── Walk.fbx
│   │   └── ...
│   ├── textures/         # Project textures
│   ├── audio/            # Project audio files
│   └── shaders/          # Project-specific shaders
└── config/               # Project configuration
    ├── config.json       # Main project settings
    ├── engine_config.json # Engine module configuration
    └── project_config.json # Project-specific settings
```

### 3. Extending Base Engine Classes

#### Example: Custom Character Class

**Base Engine (Generic Interface):**

```cpp
// include/Game/Character.h - BASE ENGINE
namespace GameEngine {
    class Character {
    public:
        Character();
        virtual ~Character() = default;

        // Generic interface - no specific assets
        virtual bool Initialize() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Render() = 0;

    protected:
        // Generic animation interface
        virtual void LoadCharacterAnimations() = 0;
        virtual void SetupCharacterAnimationStateMachine() = 0;

        // Base functionality
        std::shared_ptr<AnimationController> m_animationController;
        // ... other base members
    };
}
```

**Project Implementation (Specific Character):**

```cpp
// projects/GameExample/src/public/XBotCharacter.h - PROJECT SPECIFIC
#include "Game/Character.h"

namespace GameExample {
    class XBotCharacter : public GameEngine::Character {
    public:
        XBotCharacter();
        ~XBotCharacter() override = default;

        bool Initialize() override;
        void Update(float deltaTime) override;
        void Render() override;

    protected:
        void LoadCharacterAnimations() override;
        void SetupCharacterAnimationStateMachine() override;

    private:
        // XBot-specific data and methods
        void LoadXBotAssets();
        void SetupXBotStateMachine();
    };
}
```

```cpp
// projects/GameExample/src/private/XBotCharacter.cpp - PROJECT SPECIFIC
#include "XBotCharacter.h"
#include "Resource/ResourceManager.h"

namespace GameExample {
    void XBotCharacter::LoadCharacterAnimations() {
        // Load XBot-specific animations from project assets
        auto& resourceManager = GameEngine::ResourceManager::GetInstance();

        // Project-specific asset paths
        resourceManager.LoadAnimation("xbot_idle", "projects/GameExample/assets/models/Idle.fbx");
        resourceManager.LoadAnimation("xbot_walk", "projects/GameExample/assets/models/Walking.fbx");
        // ... load other XBot animations
    }

    void XBotCharacter::SetupCharacterAnimationStateMachine() {
        // Create XBot-specific state machine
        auto stateMachine = std::make_shared<AnimationStateMachine>();

        // XBot-specific states and transitions
        auto idleState = std::make_shared<AnimationState>("Idle");
        idleState->SetAnimation(resourceManager.GetAnimation("xbot_idle"));

        auto walkState = std::make_shared<AnimationState>("Walk");
        walkState->SetAnimation(resourceManager.GetAnimation("xbot_walk"));

        // XBot-specific logic
        stateMachine->AddState(idleState);
        stateMachine->AddState(walkState);
        // ... setup XBot transitions

        m_animationController->SetStateMachine(stateMachine);
    }
}
```

## Asset Management for Projects

### Project Asset Paths

Each project manages its own assets:

```cpp
// ✅ CORRECT: Project-specific asset loading following naming conventions
resourceManager.LoadModel("projects/GameExample/assets/characters/XBotCharacter/meshes/XBotCharacter.fbx");
resourceManager.LoadTexture("projects/GameExample/assets/characters/XBotCharacter/textures/XBotCharacter_Diffuse.png");
resourceManager.LoadAnimation("projects/GameExample/assets/characters/XBotCharacter/animations/locomotion/Idle.fbx");
resourceManager.LoadAudio("projects/GameExample/assets/characters/XBotCharacter/audio/footsteps/concrete.wav");

// ❌ WRONG: Don't put project assets in shared locations
resourceManager.LoadModel("assets/meshes/XBot.fbx");  // This is for shared engine assets only
```

### Asset Organization Standards

**MANDATORY**: All projects must follow the standardized asset naming conventions defined in [Asset Naming Conventions](asset-naming-conventions.md).

```
projects/YourGame/assets/
├── characters/                  # Character-specific assets
│   └── [CharacterName]/        # Individual character folder
│       ├── meshes/             # Character model files
│       │   └── [CharacterName].fbx
│       ├── animations/         # Character animations
│       │   ├── locomotion/     # Movement animations
│       │   │   ├── Idle.fbx
│       │   │   ├── Walk.fbx
│       │   │   ├── Run.fbx
│       │   │   └── Jump.fbx
│       │   ├── combat/         # Combat animations
│       │   └── interaction/    # Interaction animations
│       ├── textures/           # Character textures
│       │   ├── [CharacterName]_Diffuse.png
│       │   ├── [CharacterName]_Normal.png
│       │   └── [CharacterName]_Roughness.png
│       ├── materials/          # Character materials
│       └── audio/              # Character-specific audio
├── environment/                 # Environment assets
│   ├── levels/                 # Level-specific assets
│   └── props/                  # Environment props
├── vfx/                        # Visual effects
├── audio/                      # Project audio assets
│   ├── music/                  # Background music
│   ├── sfx/                    # Sound effects
│   └── voice/                  # Voice acting
├── ui/                         # User interface assets
├── input/                      # Input-related assets
└── data/                       # Game data files
```

**See [Asset Naming Conventions](asset-naming-conventions.md) for complete structure and naming rules.**

## Configuration Management

### Project Configuration Files

#### config/config.json

```json
{
  "projectName": "YourGame",
  "projectVersion": "1.0.0",
  "windowTitle": "Your Game Title",
  "windowWidth": 1920,
  "windowHeight": 1080,
  "fullscreen": false,
  "assetPath": "projects/YourGame/assets/",
  "configPath": "projects/YourGame/config/"
}
```

#### config/engine_config.json

```json
{
  "requiredModules": [
    "Core",
    "OpenGLGraphics",
    "BulletPhysics",
    "OpenALAudio",
    "AssimpResource"
  ],
  "optionalModules": ["LuaScripting"],
  "moduleSettings": {
    "OpenGLGraphics": {
      "vsync": true,
      "multisampling": 4
    },
    "BulletPhysics": {
      "gravity": -9.81,
      "maxRigidBodies": 1000
    }
  }
}
```

## Build System Integration

### Project CMakeLists.txt Template

```cmake
# projects/YourGame/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(YourGame)

# Find the engine
find_package(GameEngineKiro REQUIRED)

# Project sources
set(PROJECT_SOURCES
    src/main.cpp
    src/YourCharacter.cpp
    src/YourGameLogic.cpp
    # Add other project sources
)

set(PROJECT_HEADERS
    src/YourCharacter.h
    src/YourGameLogic.h
    # Add other project headers
)

# Create executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Link with engine
target_link_libraries(${PROJECT_NAME} PRIVATE GameEngineKiro::Engine)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src/)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/projects/${PROJECT_NAME}/assets
)

# Copy config files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/projects/${PROJECT_NAME}/config
)
```

## Common Mistakes to Avoid

### ❌ Wrong Approaches

1. **Putting project code in engine directories:**

   ```cpp
   // ❌ WRONG: Don't create project-specific classes in engine
   include/Game/XBotCharacter.h     // This pollutes the engine
   src/Game/XBotCharacter.cpp       // This violates separation
   ```

2. **Hardcoding project assets in base classes:**

   ```cpp
   // ❌ WRONG: Don't hardcode specific assets in base Character class
   class Character {
       void LoadAnimations() {
           LoadAnimation("XBot_Idle.fbx");  // This ties engine to specific project
       }
   };
   ```

3. **Mixing project assets with engine assets:**
   ```
   assets/models/XBot.fbx           # ❌ WRONG: Project-specific asset in shared location
   assets/textures/XBot_Diffuse.png # ❌ WRONG: Project-specific texture in shared location
   ```

### ✅ Correct Approaches

1. **Project-specific code in project directories:**

   ```cpp
   // ✅ CORRECT: Project-specific classes in project directory
   projects/GameExample/src/public/XBotCharacter.h
   projects/GameExample/src/private/XBotCharacter.cpp
   ```

2. **Virtual interfaces in base classes:**

   ```cpp
   // ✅ CORRECT: Generic virtual interface in base Character class
   class Character {
   protected:
       virtual void LoadCharacterAnimations() = 0;  // Let projects implement
   };
   ```

3. **Project assets in project directories:**
   ```
   projects/GameExample/assets/models/XBot.fbx           # ✅ CORRECT
   projects/GameExample/assets/textures/XBot_Diffuse.png # ✅ CORRECT
   ```

## Testing Project Code

### Project-Specific Tests

Create tests for your project code:

```
projects/YourGame/
├── tests/                  # Project-specific tests
│   ├── unit/              # Unit tests for project classes
│   │   ├── test_your_character.cpp
│   │   └── test_your_game_logic.cpp
│   └── integration/       # Integration tests
│       └── test_your_game_integration.cpp
└── ...
```

### Test Implementation Example

```cpp
// projects/GameExample/tests/unit/test_xbot_character.cpp
#include "TestUtils.h"
#include "../src/public/XBotCharacter.h"

using namespace GameExample;
using namespace GameEngine::Testing;

bool TestXBotCharacterInitialization() {
    TestOutput::PrintTestStart("XBot character initialization");

    XBotCharacter xbot;
    EXPECT_TRUE(xbot.Initialize());

    TestOutput::PrintTestPass("XBot character initialization");
    return true;
}

int main() {
    TestOutput::PrintHeader("XBotCharacter Tests");

    bool allPassed = true;
    TestSuite suite("XBotCharacter Tests");

    allPassed &= suite.RunTest("XBot Character Initialization", TestXBotCharacterInitialization);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    return allPassed ? 0 : 1;
}
```

## Documentation Standards

### Project README Template

````markdown
# Your Game Project

## Overview

Brief description of your game project.

## Features

- List of game-specific features
- Character types and abilities
- Gameplay mechanics

## Assets

### Character Models

- YourCharacter.fbx - Main character model with animations
- Idle.fbx - Idle animation
- Walk.fbx - Walking animation
- Run.fbx - Running animation

### Textures

- YourCharacter_Diffuse.png - Character diffuse texture
- YourCharacter_Normal.png - Character normal map

## Building

```powershell
# Build the project
.\scripts\build_unified.bat --project YourGame

# Run the project
build\projects\YourGame\Release\YourGame.exe
```
````

## Configuration

Edit `config/config.json` to customize game settings.

## Controls

- WASD - Movement
- Space - Jump
- Mouse - Look around

```

## Best Practices Summary

1. **Separation of Concerns**: Keep engine generic, projects specific
2. **Asset Organization**: Each project manages its own assets
3. **Virtual Interfaces**: Use virtual methods for project customization
4. **Configuration Management**: Use JSON for project settings
5. **Testing**: Create project-specific tests
6. **Documentation**: Document project-specific features and assets
7. **Build Integration**: Use CMake for project build configuration

This approach ensures that:
- The engine remains reusable across multiple projects
- Projects can be developed independently
- Code is maintainable and scalable
- Multiple developers can work on different projects simultaneously
- The engine can evolve without breaking existing projects
```
