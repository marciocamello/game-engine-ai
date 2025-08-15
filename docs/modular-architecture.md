# Game Engine Kiro - Modular Architecture Guide

## Overview

Game Engine Kiro has been restructured into a modular, plugin-based architecture that provides clear separation between engine modules, game projects, and tests. This guide explains the new architecture and how to work with it effectively.

## Architecture Benefits

- **Modular Design**: Engine components are now independent modules that can be loaded/unloaded dynamically
- **Project Separation**: Clear distinction between engine code and game projects
- **Scalable Structure**: Easy to add new modules and create new game projects
- **Dependency Management**: Explicit module dependencies with automatic resolution
- **Test Organization**: Separate test structures for engine and project testing

## Directory Structure

```
GameEngineKiro/
├── engine/                    # Core engine modules
│   ├── core/                 # Engine foundation (required)
│   ├── modules/              # Optional engine modules (plugins)
│   │   ├── graphics-opengl/  # OpenGL renderer module
│   │   ├── physics-bullet/   # Bullet Physics module
│   │   ├── audio-openal/     # OpenAL audio module
│   │   └── resource-assimp/  # Assimp model loading module
│   └── interfaces/           # Module interface definitions
├── projects/                 # Game projects and tests
│   ├── GameExample/          # Enhanced example game
│   ├── BasicExample/         # Simple example game
│   ├── Tests/               # All test suites
│   └── templates/           # Project templates
├── shared/                  # Shared resources and utilities
│   ├── assets/             # Common engine assets
│   └── configs/            # Default configurations
└── tools/                  # Development tools and scripts
```

## Module System

### Module Interface

All engine modules implement the `IEngineModule` interface:

```cpp
namespace GameEngine {
    class IEngineModule {
    public:
        virtual ~IEngineModule() = default;

        // Module lifecycle
        virtual bool Initialize(const ModuleConfig& config) = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Shutdown() = 0;

        // Module information
        virtual const char* GetName() const = 0;
        virtual const char* GetVersion() const = 0;
        virtual ModuleType GetType() const = 0;
        virtual std::vector<std::string> GetDependencies() const = 0;

        // Module state
        virtual bool IsInitialized() const = 0;
        virtual bool IsEnabled() const = 0;
        virtual void SetEnabled(bool enabled) = 0;
    };
}
```

### Module Types

- **Core Modules**: Essential engine components (Engine, Logger, Math)
- **Graphics Modules**: Rendering systems (OpenGL, Vulkan in future)
- **Physics Modules**: Physics engines (Bullet, PhysX in future)
- **Audio Modules**: Audio systems (OpenAL, FMOD in future)
- **Resource Modules**: Asset loading (Assimp, custom loaders)
- **Input Modules**: Input handling systems
- **Scripting Modules**: Scripting engines (Lua, Python in future)

### Module Registry

The `ModuleRegistry` manages all modules:

```cpp
// Get the singleton instance
ModuleRegistry& registry = ModuleRegistry::GetInstance();

// Register a module
registry.RegisterModule(std::make_unique<OpenGLGraphicsModule>());

// Get a module
IEngineModule* graphics = registry.GetModule("OpenGLGraphics");

// Initialize all modules
registry.InitializeModules(engineConfig);
```

## Project System

### Project Structure

Each game project follows a standardized structure:

```
projects/YourGame/
├── CMakeLists.txt           # Project build configuration
├── src/                     # Game-specific source code
│   ├── main.cpp            # Project entry point
│   ├── YourCharacter.cpp   # Project-specific character classes
│   ├── YourCharacter.h     # Project-specific headers
│   └── ...                 # Other project-specific code
├── assets/                  # Game-specific assets
│   ├── models/             # Project 3D models and animations
│   ├── textures/           # Project textures
│   ├── audio/              # Project audio files
│   └── shaders/            # Project-specific shaders
├── config/                  # Game configuration files
│   ├── config.json         # Main project configuration
│   ├── engine_config.json  # Engine module configuration
│   └── project_config.json # Project-specific settings
└── README.md               # Project documentation
```

### Critical Separation Rules

**NEVER put project-specific code in the base engine:**

- ❌ **WRONG**: `include/Game/XBotCharacter.h` (engine directory)
- ✅ **CORRECT**: `projects/GameExample/src/XBotCharacter.h` (project directory)

**NEVER put project-specific assets in shared locations:**

- ❌ **WRONG**: `assets/models/XBot.fbx` (shared engine assets)
- ✅ **CORRECT**: `projects/GameExample/assets/models/XBot.fbx` (project assets)

**Base engine provides generic interfaces only:**

- ✅ **CORRECT**: `include/Game/Character.h` (generic base class)
- ✅ **CORRECT**: `include/Animation/AnimationController.h` (generic animation system)

### Project Configuration

Projects use JSON configuration files to declare module dependencies:

```json
{
  "projectName": "YourGame",
  "projectVersion": "1.0.0",
  "requiredModules": ["Core", "OpenGLGraphics", "BulletPhysics", "OpenALAudio"],
  "optionalModules": ["AssimpResource", "LuaScripting"],
  "projectSettings": {
    "windowWidth": 1920,
    "windowHeight": 1080,
    "fullscreen": false
  },
  "assetPath": "assets/",
  "configPath": "config/"
}
```

## Configuration System

### Engine Configuration

Engine-wide settings are managed through `EngineConfig`:

```json
{
  "configVersion": "1.0",
  "engineVersion": "2.0.0",
  "modules": [
    {
      "name": "OpenGLGraphics",
      "version": "1.0.0",
      "enabled": true,
      "parameters": {
        "vsync": "true",
        "multisampling": "4"
      }
    },
    {
      "name": "BulletPhysics",
      "version": "1.0.0",
      "enabled": true,
      "parameters": {
        "gravity": "-9.81",
        "maxRigidBodies": "1000"
      }
    }
  ]
}
```

### Module Configuration

Individual modules can have specific configuration parameters:

```cpp
// Graphics module configuration
ModuleConfig graphicsConfig;
graphicsConfig.name = "OpenGLGraphics";
graphicsConfig.parameters["vsync"] = "true";
graphicsConfig.parameters["multisampling"] = "4";

// Physics module configuration
ModuleConfig physicsConfig;
physicsConfig.name = "BulletPhysics";
physicsConfig.parameters["gravity"] = "-9.81";
physicsConfig.parameters["maxRigidBodies"] = "1000";
```

## Build System

### Hierarchical CMake Structure

The build system uses a hierarchical approach:

1. **Root CMakeLists.txt**: Orchestrates the entire build
2. **Engine Module CMakeLists.txt**: Each module has its own build configuration
3. **Project CMakeLists.txt**: Each game project has its own build configuration

### Building Projects

```powershell
# Build entire solution
.\scripts\build_unified.bat --tests

# Build specific project (future feature)
.\scripts\build_project.bat GameExample

# Build specific module (future feature)
.\scripts\build_module.bat graphics-opengl
```

## Asset Management

### Shared Assets

Common engine assets are stored in `shared/assets/`:

```
shared/assets/
├── shaders/                 # Common shaders
├── textures/               # Default textures
├── models/                 # Basic models (cube, sphere, etc.)
└── audio/                  # System sounds
```

### Project Assets

Each project has its own asset directory:

```
projects/YourGame/assets/
├── models/                 # Game-specific 3D models
├── textures/              # Game textures
├── audio/                 # Game sounds and music
└── shaders/               # Custom shaders
```

### Asset Path Resolution

The engine automatically resolves asset paths following standardized naming conventions:

1. **Project-specific assets**: `projects/[ProjectName]/assets/characters/[CharacterName]/`
2. **Shared engine assets**: `assets/meshes/primitives/`, `assets/textures/defaults/`
3. **Fallback assets**: `assets/meshes/fallbacks/`, `assets/textures/fallbacks/`

**See [Asset Naming Conventions](asset-naming-conventions.md) for complete asset organization standards.**

## Testing Architecture

### Dual Test Structure

The testing system maintains separation:

#### Engine Tests

```
tests/
├── unit/                   # Engine unit tests
├── integration/           # Engine integration tests
└── TestUtils.h           # Shared testing utilities
```

#### Project Tests (Future)

```
projects/Tests/
├── CMakeLists.txt         # Project test build
├── unit/                  # Project unit tests
├── integration/          # Project integration tests
└── utilities/            # Project test utilities
```

### Running Tests

```powershell
# Run all engine tests
.\scripts\run_tests.bat

# Run specific test category (future)
.\scripts\run_tests.bat --unit
.\scripts\run_tests.bat --integration
```

## Error Handling

### Module Loading Errors

The system provides comprehensive error handling:

```cpp
// Module loading with error handling
try {
    if (!registry.InitializeModules(config)) {
        LOG_ERROR("Failed to initialize modules");
        // Handle graceful fallback
    }
} catch (const ModuleException& e) {
    LOG_ERROR("Module error: " + std::string(e.what()));
    // Attempt recovery or safe shutdown
}
```

### Common Error Scenarios

1. **Missing Dependencies**: Clear error messages with suggested solutions
2. **Version Conflicts**: Automatic resolution or user notification
3. **Configuration Errors**: Validation with helpful error messages
4. **Runtime Failures**: Module isolation to prevent cascading failures

## Performance Considerations

### Module Overhead

- Minimal runtime overhead for module system
- Compile-time dependency resolution where possible
- Efficient module lookup using hash maps
- Lazy loading of optional modules

### Memory Management

- RAII-based resource management
- Smart pointers for automatic cleanup
- Module-specific memory pools (future feature)
- Leak detection in debug builds

## Best Practices

### Module Development

1. **Single Responsibility**: Each module should have a clear, focused purpose
2. **Minimal Dependencies**: Reduce coupling between modules
3. **Error Handling**: Provide comprehensive error reporting
4. **Configuration**: Support runtime configuration changes
5. **Testing**: Include unit tests for all module functionality

### Project Development

1. **Declare Dependencies**: Explicitly list required and optional modules
2. **Asset Organization**: Keep project assets separate from engine assets
3. **Configuration Management**: Use JSON for project settings
4. **Documentation**: Document project-specific features and requirements

### Performance Optimization

1. **Profile Regularly**: Use Release builds for performance testing
2. **Minimize Allocations**: Use object pools and memory management
3. **Cache-Friendly Data**: Structure data for optimal cache usage
4. **Parallel Processing**: Design for future multi-threading support

## Troubleshooting

### Common Issues

1. **Module Not Found**: Check module registration and naming
2. **Dependency Conflicts**: Verify module compatibility
3. **Build Errors**: Ensure all dependencies are available
4. **Runtime Crashes**: Check module initialization order

### Debug Tools

```powershell
# Monitor logs in real-time
.\scripts\monitor.bat

# Debug build with additional logging
.\scripts\debug.bat

# Run with verbose output
.\scripts\run_tests.bat --verbose
```

### Getting Help

1. Check the logs in `logs/` directory
2. Review module documentation in `docs/`
3. Examine example projects in `projects/`
4. Use debug builds for detailed error information

## Future Enhancements

### Planned Features

1. **Hot Module Reloading**: Runtime module swapping for development
2. **Module Marketplace**: Community-contributed modules
3. **Visual Module Editor**: GUI for module configuration
4. **Performance Profiler**: Built-in performance analysis tools
5. **Multi-threading Support**: Parallel module execution
6. **Vulkan Graphics Module**: Modern graphics API support
7. **PhysX Physics Module**: Alternative physics engine
8. **Network Module**: Multiplayer networking support

### Roadmap

- **Phase 1**: Core modular architecture (Complete)
- **Phase 2**: Advanced module features (In Progress)
- **Phase 3**: Community and tooling support (Planned)
- **Phase 4**: Performance and optimization (Planned)

This modular architecture provides a solid foundation for scalable game development while maintaining the flexibility to adapt to future requirements.
