# Game Engine Kiro - Project Structure

## Root Directory Organization

```
GameEngineKiro/
├── include/          # Public headers (interface definitions)
├── src/             # Implementation files (.cpp)
├── examples/        # Sample applications and demos
├── assets/          # Game assets (shaders, textures, models)
├── docs/            # Documentation and guides
├── tests/           # Unit and integration tests
├── scripts/         # Build and development scripts
├── build/           # Generated build files (CMake output)
├── logs/            # Runtime log files
├── cache/           # Model cache and temporary files
├── coverage_reports/ # Code coverage analysis reports
├── vcpkg/           # Dependency management (auto-generated)
├── vcpkg_installed/ # Installed packages (auto-generated)
├── .kiro/           # Kiro IDE configuration and specs
├── .vscode/         # VS Code configuration
└── .git/            # Git version control
```

## Header Organization (include/)

Headers are organized by engine subsystem with clear separation of concerns:

```
include/
├── Animation/       # Animation system
├── Audio/           # Audio system
│   └── AudioEngine.h         # 3D spatial audio
├── Core/            # Engine foundation
│   ├── Engine.h     # Main engine class
│   ├── Logger.h     # Logging system
│   └── Math.h       # Mathematics utilities
├── Game/            # Game-specific components
│   ├── Character.h           # Character controller
│   ├── ThirdPersonCamera.h   # Specialized camera
│   ├── ThirdPersonCameraSystem.h # Camera management
│   └── SpringArm.h           # Camera spring arm component
├── Graphics/        # Rendering system
│   ├── GraphicsRenderer.h    # Abstract renderer interface
│   ├── OpenGLRenderer.h      # OpenGL implementation
│   ├── Camera.h              # Camera base class
│   ├── PrimitiveRenderer.h   # Built-in shapes
│   ├── Shader.h              # Shader management
│   ├── Texture.h             # Texture handling
│   ├── Material.h            # Material system
│   └── Mesh.h                # Mesh data structures
├── Input/           # Input handling
│   └── InputManager.h        # Keyboard, mouse, gamepad
├── Physics/         # Physics simulation
│   ├── PhysicsEngine.h       # Main physics interface
│   ├── BulletUtils.h         # Bullet Physics utilities
│   └── CollisionShapeFactory.h # Shape creation
├── Resource/        # Asset management
│   └── ResourceManager.h     # Loading and caching
└── Scripting/       # Scripting integration
    └── ScriptingEngine.h     # Lua scripting support
```

## Source Organization (src/)

Implementation files mirror the header structure exactly:

```
src/
├── Animation/       # Animation system implementation
├── Audio/           # Audio implementation
├── Core/            # Engine core implementation
├── Game/            # Game components implementation
├── Graphics/        # Rendering implementation
├── Input/           # Input implementation
├── Physics/         # Physics implementation
├── Resource/        # Resource management implementation
└── Scripting/       # Scripting implementation
```

## Testing Structure (tests/)

```
tests/
├── integration/     # Integration tests (system interaction testing)
│   ├── test_bullet_integration.cpp
│   ├── test_physics_queries.cpp
│   ├── test_model_loader_assimp.cpp
│   ├── test_audio_camera_integration.cpp
│   └── test_final_v1_validation.cpp
├── unit/           # Unit tests (individual component testing)
│   ├── test_math.cpp
│   ├── test_matrix.cpp
│   ├── test_quaternion.cpp
│   ├── test_audio_engine.cpp
│   └── test_resource_manager.cpp
├── templates/      # Test template files
└── TestUtils.h     # Shared testing utilities
```

## Asset Organization (assets/)

```
assets/
├── audio/           # Audio files and sound effects
├── GLTF/           # GLTF 3D model files
├── materials/      # Material definition files
├── meshes/         # 3D mesh files (OBJ, FBX, etc.)
├── shaders/        # GLSL shader files
│   ├── vertex/     # Vertex shaders
│   ├── fragment/   # Fragment shaders
│   └── compute/    # Compute shaders (future)
├── textures/       # Texture image files
└── README.md       # Asset documentation
```

## Scripts Structure (scripts/)

```
scripts/
├── build_unified.bat            # Main build script (ONLY permitted build command)
├── debug.bat                    # Debug launcher with multiple options
├── dev.bat                      # Development console (interactive)
├── monitor.bat                  # Log monitoring script
├── run_coverage_analysis.bat    # Coverage analysis runner
├── run_final_validation.bat     # Final validation tests
├── run_physics_tests.bat        # Physics-specific tests
├── run_tests.bat                # All tests runner (MANDATORY before task completion)
├── setup_dependencies.bat       # Dependency setup
└── README.md                    # Scripts documentation
```

## Documentation Structure (docs/)

```
docs/
├── 3d-model-loading.md           # 3D model loading system
├── advanced-shader-system.md     # Advanced shader features
├── animation-system.md           # Animation system documentation
├── api-reference.md              # Complete API documentation
├── architecture.md               # Engine architecture overview
├── audio-system.md               # Audio system documentation
├── coding-standards.md           # Code style and standards
├── coverage-setup.md             # Code coverage configuration
├── deterministic-physics.md      # Deterministic physics implementation
├── ide.md                        # IDE integration guide
├── model-hot-reloading.md        # Model hot-reloading system
├── nvidia-physx-integration.md   # PhysX integration guide
├── particle-effects.md           # Particle system documentation
├── physics-strategy.md           # Dual physics backend strategy
├── quickstart.md                 # Getting started tutorial
├── roadmap.md                    # Project roadmap and milestones
├── setup.md                      # Installation and setup guide
├── testing-*.md                  # Various testing documentation
└── validate-links.py             # Documentation link validator
```

## Naming Conventions

### CRITICAL RULE: English Only

- **ALL names, comments, and documentation MUST be in English**
- **NO Portuguese or other languages in any part of the codebase**

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
- **Comments**: English only, technical and professional
- **Variable names**: Descriptive English names

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
