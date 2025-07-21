# Game Engine Kiro - Project Structure

## Root Directory Organization

```
GameEngineKiro/
├── include/          # Public headers (interface definitions)
├── src/             # Implementation files (.cpp)
├── examples/        # Sample applications and demos
├── assets/          # Game assets (shaders, textures, models)
├── docs/            # Documentation and guides
├── tests/           # Integration tests
├── build/           # Generated build files (CMake output)
├── logs/            # Runtime log files
├── vcpkg/           # Dependency management (auto-generated)
├── vcpkg_installed/ # Installed packages (auto-generated)
└── .kiro/           # Kiro IDE configuration and specs
```

## Header Organization (include/)

Headers are organized by engine subsystem with clear separation of concerns:

```
include/
├── Core/            # Engine foundation
│   ├── Engine.h     # Main engine class
│   ├── Logger.h     # Logging system
│   └── Math.h       # Mathematics utilities
├── Graphics/        # Rendering system
│   ├── GraphicsRenderer.h    # Abstract renderer interface
│   ├── OpenGLRenderer.h      # OpenGL implementation
│   ├── Camera.h              # Camera base class
│   ├── PrimitiveRenderer.h   # Built-in shapes
│   ├── Shader.h              # Shader management
│   ├── Texture.h             # Texture handling
│   ├── Material.h            # Material system
│   └── Mesh.h                # Mesh data structures
├── Physics/         # Physics simulation
│   ├── PhysicsEngine.h       # Main physics interface
│   ├── BulletUtils.h         # Bullet Physics utilities
│   └── CollisionShapeFactory.h # Shape creation
├── Audio/           # Audio system
│   └── AudioEngine.h         # 3D spatial audio
├── Input/           # Input handling
│   └── InputManager.h        # Keyboard, mouse, gamepad
├── Resource/        # Asset management
│   └── ResourceManager.h     # Loading and caching
├── Game/            # Game-specific components
│   ├── Character.h           # Character controller
│   ├── ThirdPersonCamera.h   # Specialized camera
│   ├── ThirdPersonCameraSystem.h # Camera management
│   └── SpringArm.h           # Camera spring arm component
└── Scripting/       # Scripting integration
    └── ScriptingEngine.h     # Lua scripting support
```

## Source Organization (src/)

Implementation files mirror the header structure exactly:

```
src/
├── Core/            # Engine core implementation
├── Graphics/        # Rendering implementation
├── Physics/         # Physics implementation
├── Audio/           # Audio implementation
├── Input/           # Input implementation
├── Resource/        # Resource management implementation
├── Game/            # Game components implementation
└── Scripting/       # Scripting implementation
```

## Testing Structure (tests/)

```
tests/
├── integration/     # Integration tests (system interaction testing)
│   ├── test_bullet_integration.cpp
│   └── test_physics_queries.cpp
└── integration/     # Integration tests (system interaction)
    ├── test_bullet_integration.cpp
    ├── test_bullet_conversion.cpp
    ├── test_bullet_utils_simple.cpp
    └── test_collision_shape_factory_simple.cpp
```

## Asset Organization (assets/)

```
assets/
└── shaders/         # GLSL shader files
    ├── vertex/      # Vertex shaders
    ├── fragment/    # Fragment shaders
    └── compute/     # Compute shaders (future)
```

## Documentation Structure (docs/)

```
docs/
├── setup.md         # Installation and setup guide
├── quickstart.md    # Getting started tutorial
├── architecture.md  # Engine architecture overview
├── physics-strategy.md # Dual physics backend strategy
├── api-reference.md # Complete API documentation
└── ide.md          # IDE integration guide
```

## Naming Conventions

### Files and Directories

- **Headers**: PascalCase (e.g., `GraphicsRenderer.h`)
- **Source files**: Match header names (e.g., `GraphicsRenderer.cpp`)
- **Directories**: PascalCase for modules (e.g., `Graphics/`, `Physics/`)
- **Assets**: lowercase with underscores (e.g., `basic_vertex.glsl`)

### Code Conventions

- **Classes**: PascalCase (e.g., `GraphicsRenderer`, `PhysicsEngine`)
- **Methods**: PascalCase (e.g., `Initialize()`, `GetRenderer()`)
- **Member variables**: m\_ prefix with camelCase (e.g., `m_renderer`, `m_isRunning`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_RIGID_BODIES`)
- **Namespaces**: PascalCase (e.g., `GameEngine`, `Math`)

### CMake Conventions

- **Targets**: PascalCase (e.g., `GameEngineKiro`, `GameExample`)
- **Variables**: UPPER_SNAKE_CASE (e.g., `ENGINE_SOURCES`)
- **Options**: UPPER_SNAKE_CASE with descriptive names (e.g., `ENABLE_VULKAN`)

## Module Dependencies

### Dependency Hierarchy (top-to-bottom)

```
Game Layer (Character, Camera, etc.)
    ↓
Engine Systems (Graphics, Physics, Audio, Input)
    ↓
Core Foundation (Engine, Math, Logger)
    ↓
External Libraries (OpenGL, Bullet, GLFW, etc.)
```

### Interface Patterns

- **Abstract base classes** for major systems (e.g., `GraphicsRenderer`)
- **Factory methods** for platform-specific implementations
- **PIMPL idiom** for hiding implementation details
- **Resource management** through RAII and smart pointers

## Build Output Structure

```
build/
├── Release/         # Release build artifacts
│   ├── GameExample.exe      # Main executable
│   ├── GameEngineKiro.lib   # Engine static library
│   ├── assets/              # Copied game assets
│   └── *.exe                # Test executables
├── Debug/           # Debug build artifacts
└── CMakeFiles/      # CMake generated files
```

## Configuration Files

- **CMakeLists.txt**: Main build configuration
- **vcpkg.json**: Dependency manifest with feature flags
- **.clangd**: Language server configuration
- **.gitignore**: Version control exclusions
- **examples/config.json**: Runtime configuration example
