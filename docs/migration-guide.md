# Migration Guide - Modular Architecture

## Overview

This guide helps developers migrate from the previous monolithic structure to the new modular architecture of Game Engine Kiro. The migration has been designed to maintain backward compatibility while providing new modular capabilities.

## What Changed

### Directory Structure Changes

#### Before (Monolithic)

```
GameEngineKiro/
├── include/          # All headers mixed together
├── src/             # All implementation files mixed
├── examples/        # Examples mixed with tests
├── assets/          # All assets in one location
└── tests/           # Limited test structure
```

#### After (Modular)

```
GameEngineKiro/
├── engine/                    # Engine modules
│   ├── core/                 # Core engine components
│   ├── modules/              # Plugin modules
│   └── interfaces/           # Module interfaces
├── projects/                 # Game projects
│   ├── GameExample/          # Migrated examples
│   ├── BasicExample/
│   └── Tests/               # Dedicated test project
├── shared/                  # Shared resources
│   ├── assets/             # Common assets
│   └── configs/            # Default configurations
└── include/                 # Legacy headers (maintained)
```

### Code Structure Changes

#### Module System Introduction

**Before**: Direct instantiation of engine components

```cpp
// Old approach
Engine engine;
GraphicsRenderer* renderer = new OpenGLRenderer();
PhysicsEngine* physics = new BulletPhysicsEngine();
AudioEngine* audio = new OpenALAudioEngine();
```

**After**: Module-based initialization

```cpp
// New approach
ModuleRegistry& registry = ModuleRegistry::GetInstance();
registry.RegisterModule(std::make_unique<OpenGLGraphicsModule>());
registry.RegisterModule(std::make_unique<BulletPhysicsModule>());
registry.RegisterModule(std::make_unique<OpenALAudioModule>());

Engine engine;
engine.Initialize(engineConfig);
```

#### Configuration System Changes

**Before**: Hardcoded configuration

```cpp
// Old approach
renderer->SetVSync(true);
physics->SetGravity(Vector3(0, -9.81f, 0));
```

**After**: JSON-based configuration

```json
{
  "modules": [
    {
      "name": "OpenGLGraphics",
      "parameters": {
        "vsync": "true",
        "multisampling": "4"
      }
    },
    {
      "name": "BulletPhysics",
      "parameters": {
        "gravity": "-9.81"
      }
    }
  ]
}
```

## Migration Steps

### Step 1: Update Build Process

#### Old Build Command

```powershell
# This no longer works
cmake --build build --config Release
```

#### New Build Command

```powershell
# Use the unified build script
.\scripts\build_unified.bat --tests
```

**Action Required**: Update any custom build scripts or CI/CD pipelines to use the new build system.

### Step 2: Update Include Paths

#### Legacy Support

The old include structure is maintained for backward compatibility:

```cpp
// These still work
#include "Core/Engine.h"
#include "Graphics/GraphicsRenderer.h"
#include "Physics/PhysicsEngine.h"
```

#### New Module Includes

For new development, use module-specific includes:

```cpp
// New module includes
#include "engine/interfaces/IEngineModule.h"
#include "engine/core/ModuleRegistry.h"
#include "engine/modules/graphics-opengl/OpenGLGraphicsModule.h"
```

**Action Required**: Gradually migrate to new include paths in new code.

### Step 3: Migrate Existing Projects

#### Creating a New Project Structure

1. **Create Project Directory**

```powershell
mkdir projects\YourGame
cd projects\YourGame
```

2. **Copy Existing Code**

```powershell
# Copy your existing game code
xcopy ..\..\examples\your_game_files src\ /E /I
```

3. **Create Project Configuration**
   Create `projects/YourGame/config/project.json`:

```json
{
  "projectName": "YourGame",
  "projectVersion": "1.0.0",
  "requiredModules": ["Core", "OpenGLGraphics", "BulletPhysics", "OpenALAudio"],
  "optionalModules": ["AssimpResource"],
  "projectSettings": {
    "windowWidth": 1920,
    "windowHeight": 1080,
    "fullscreen": false
  }
}
```

4. **Create Project CMakeLists.txt**
   Create `projects/YourGame/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(YourGame)

# Find required modules
find_package(GameEngineKiro REQUIRED)

# Add executable
add_executable(YourGame
    src/main.cpp
    src/YourGameLogic.cpp
    # Add other source files
)

# Link engine modules
target_link_libraries(YourGame
    GameEngineKiro::Core
    GameEngineKiro::Graphics
    GameEngineKiro::Physics
    GameEngineKiro::Audio
)

# Copy assets
file(COPY assets/ DESTINATION ${CMAKE_BINARY_DIR}/assets/)
```

### Step 4: Update Engine Initialization

#### Old Initialization Pattern

```cpp
int main() {
    Engine engine;

    // Manual subsystem initialization
    if (!engine.InitializeGraphics()) {
        return -1;
    }
    if (!engine.InitializePhysics()) {
        return -1;
    }
    if (!engine.InitializeAudio()) {
        return -1;
    }

    engine.Run();
    return 0;
}
```

#### New Initialization Pattern

```cpp
int main() {
    try {
        // Load engine configuration
        EngineConfig config = ConfigManager::LoadEngineConfig("config/engine.json");

        // Initialize module registry
        ModuleRegistry& registry = ModuleRegistry::GetInstance();

        // Register required modules (or load from config)
        registry.RegisterModule(std::make_unique<OpenGLGraphicsModule>());
        registry.RegisterModule(std::make_unique<BulletPhysicsModule>());
        registry.RegisterModule(std::make_unique<OpenALAudioModule>());

        // Initialize engine with module system
        Engine engine;
        if (!engine.Initialize(config)) {
            LOG_ERROR("Failed to initialize engine");
            return -1;
        }

        engine.Run();
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Engine error: " + std::string(e.what()));
        return -1;
    }
}
```

### Step 5: Update Asset Management

#### Old Asset Loading

```cpp
// Direct file paths
Texture* texture = resourceManager->LoadTexture("assets/textures/player.png");
Model* model = resourceManager->LoadModel("assets/models/character.fbx");
```

#### New Asset Loading

```cpp
// Asset path resolution (checks project assets first, then shared)
Texture* texture = resourceManager->LoadTexture("textures/player.png");
Model* model = resourceManager->LoadModel("models/character.fbx");

// Explicit paths still work
Texture* sharedTexture = resourceManager->LoadTexture("shared/textures/default.png");
```

**Action Required**: Move project-specific assets to `projects/YourGame/assets/` directory.

### Step 6: Update Test Structure

#### Old Test Location

```
tests/
├── test_your_feature.cpp    # Mixed with engine tests
└── TestUtils.h
```

#### New Test Structure

```
projects/Tests/              # Future project tests
├── unit/
│   └── YourGame/           # Project-specific tests
└── integration/
    └── YourGame/

tests/                      # Engine tests only
├── unit/                   # Engine unit tests
└── integration/           # Engine integration tests
```

**Action Required**: Keep engine tests in `tests/`, move project-specific tests to `projects/Tests/` (when implemented).

## Compatibility Matrix

### Backward Compatibility

| Feature               | Old Code Works | New Code Required | Notes                              |
| --------------------- | -------------- | ----------------- | ---------------------------------- |
| Include paths         | ✅ Yes         | ❌ No             | Legacy includes maintained         |
| Engine initialization | ⚠️ Partial     | ✅ Yes            | Old pattern works but deprecated   |
| Asset loading         | ✅ Yes         | ❌ No             | Path resolution enhanced           |
| Build system          | ❌ No          | ✅ Yes            | Must use new build scripts         |
| Configuration         | ⚠️ Partial     | ✅ Yes            | Hardcoded config works but limited |

### Migration Timeline

#### Immediate (Required)

- Update build commands to use `.\scripts\build_unified.bat --tests`
- Update CI/CD pipelines to use new build system
- Test existing functionality with new build system

#### Short Term (Recommended)

- Migrate projects to new directory structure
- Adopt JSON-based configuration
- Update asset organization

#### Long Term (Optional)

- Migrate to new include paths
- Adopt module-based initialization
- Implement project-specific testing

## Common Migration Issues

### Build Errors

**Issue**: CMake configuration errors

```
CMake Error: Could not find a package configuration file
```

**Solution**: Ensure you're using the unified build script:

```powershell
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build_unified.bat --tests
```

### Include Path Errors

**Issue**: Header files not found

```cpp
fatal error: 'Graphics/Renderer.h' file not found
```

**Solution**: Use legacy include paths or migrate to new structure:

```cpp
// Legacy (works)
#include "Graphics/GraphicsRenderer.h"

// New (preferred)
#include "engine/interfaces/IGraphicsModule.h"
```

### Asset Loading Failures

**Issue**: Assets not found after migration

```
ERROR: Could not load texture: textures/player.png
```

**Solution**: Check asset directory structure:

```
projects/YourGame/
├── assets/
│   ├── textures/
│   │   └── player.png      # Move here
│   └── models/
└── src/
```

### Module Registration Errors

**Issue**: Module not found at runtime

```
ERROR: Module 'OpenGLGraphics' not registered
```

**Solution**: Ensure modules are registered before engine initialization:

```cpp
ModuleRegistry& registry = ModuleRegistry::GetInstance();
registry.RegisterModule(std::make_unique<OpenGLGraphicsModule>());
// Register other modules...

Engine engine;
engine.Initialize(config);
```

## Testing Migration

### Verify Migration Success

1. **Build Test**

```powershell
.\scripts\build_unified.bat --tests
```

2. **Run All Tests**

```powershell
.\scripts\run_tests.bat
```

3. **Run Your Application**

```powershell
build\projects\YourGame\Release\YourGame.exe
```

4. **Check Logs**

```powershell
.\scripts\monitor.bat
```

### Validation Checklist

- [ ] Project builds successfully with new build system
- [ ] All existing functionality works as expected
- [ ] Assets load correctly from new locations
- [ ] Configuration system works with JSON files
- [ ] Module system initializes without errors
- [ ] Performance is comparable to previous version
- [ ] All tests pass

## Getting Help

### Resources

1. **Documentation**: Check `docs/modular-architecture.md` for detailed architecture information
2. **Examples**: Look at `projects/GameExample/` and `projects/BasicExample/` for reference implementations
3. **Logs**: Monitor `logs/` directory for detailed error information
4. **Debug Mode**: Use `.\scripts\debug.bat` for additional debugging information

### Common Support Scenarios

#### "My project won't build"

1. Verify you're using `.\scripts\build_unified.bat --tests`
2. Check that all dependencies are available
3. Review CMakeLists.txt for correct module references

#### "Assets aren't loading"

1. Check asset directory structure
2. Verify asset paths in code
3. Review asset path resolution in logs

#### "Performance is worse"

1. Ensure you're using Release builds for performance testing
2. Check module configuration for optimization settings
3. Profile specific bottlenecks

#### "Tests are failing"

1. Run `.\scripts\run_tests.bat` to see detailed test output
2. Check if tests need updates for new architecture
3. Verify test data and asset paths

## Migration Support

For complex migration scenarios or issues not covered in this guide:

1. **Review Architecture Documentation**: `docs/modular-architecture.md`
2. **Check Example Projects**: `projects/GameExample/` and `projects/BasicExample/`
3. **Examine Test Cases**: `tests/unit/` and `tests/integration/` for usage patterns
4. **Debug with Logging**: Use `.\scripts\monitor.bat` for real-time log monitoring

The modular architecture is designed to be backward compatible while providing new capabilities. Most existing code should continue to work with minimal changes, allowing for gradual migration to the new system.
