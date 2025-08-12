# Hierarchical Build System

Game Engine Kiro uses a hierarchical CMake build system that supports building individual components or the entire solution. This system provides flexibility for developers to build only what they need, improving build times and development efficiency.

## Build Options

### 1. Complete Build (Default)

Builds everything: engine, modules, projects, and tests.

```bash
.\scripts\build_hierarchical.bat
```

### 2. Engine Only

Builds only the engine library and modules, without projects or tests.

```bash
.\scripts\build_hierarchical.bat --engine-only
```

### 3. Projects Only

Builds only game projects (requires pre-built engine).

```bash
.\scripts\build_hierarchical.bat --projects-only
```

### 4. Tests Only

Builds only test suites (requires pre-built engine).

```bash
.\scripts\build_hierarchical.bat --tests-only
```

### 5. Specific Project

Builds only a specific project along with the required engine components.

```bash
.\scripts\build_hierarchical.bat --project GameExample
.\scripts\build_hierarchical.bat --project BasicExample
```

## Project Structure

The hierarchical build system organizes the project into distinct components:

```
GameEngineKiro/
├── engine/                    # Engine core and modules
│   ├── core/                 # Engine foundation (always built)
│   ├── modules/              # Optional engine modules
│   │   ├── graphics-opengl/  # OpenGL renderer module
│   │   ├── physics-bullet/   # Bullet Physics module
│   │   ├── audio-openal/     # OpenAL audio module
│   │   └── templates/        # Module templates
│   └── interfaces/           # Module interface definitions
├── projects/                 # Game projects and tests
│   ├── GameExample/          # Enhanced example game
│   ├── BasicExample/         # Simple example game
│   ├── Tests/               # Project-specific tests
│   └── templates/           # Project templates
└── scripts/                 # Build scripts
    └── build_hierarchical.bat # Hierarchical build script
```

## Module System

### Engine Modules

Engine modules are self-contained components that can be enabled or disabled:

- **graphics-opengl**: OpenGL rendering system
- **physics-bullet**: Bullet Physics integration
- **audio-openal**: OpenAL 3D audio system

Each module has its own CMakeLists.txt and can declare dependencies.

### Module Configuration

Modules can be enabled/disabled via CMake options:

```cmake
option(ENABLE_GRAPHICS_MODULE "Enable graphics module" ON)
option(ENABLE_PHYSICS_MODULE "Enable physics module" ON)
option(ENABLE_AUDIO_MODULE "Enable audio module" ON)
```

## Project System

### Game Projects

Game projects are independent applications that use the engine:

- **GameExample**: Full-featured example with all modules
- **BasicExample**: Minimal example with basic graphics

### Project Dependencies

Projects declare their module dependencies:

```cmake
# Required engine modules for this project
set(REQUIRED_ENGINE_MODULES
    graphics-opengl
    physics-bullet
    audio-openal
)

# Optional engine modules for this project
set(OPTIONAL_ENGINE_MODULES
    # Add optional modules here
)
```

## Creating New Components

### Creating a New Module

1. Create a new directory in `engine/modules/`
2. Copy the template from `engine/modules/templates/ModuleCMakeTemplate.cmake`
3. Customize the module configuration
4. Implement the module interface

### Creating a New Project

1. Create a new directory in `projects/`
2. Copy the template from `projects/templates/ProjectCMakeTemplate.cmake`
3. Customize the project configuration
4. Implement your game logic

## Build Targets

The hierarchical build system creates the following targets:

### Engine Targets

- `GameEngineKiro`: Main engine library
- `[ModuleName]`: Individual module libraries (when built standalone)

### Project Targets

- `GameExample`: Game example executable
- `BasicExample`: Basic example executable
- Custom project executables

### Test Targets

- Unit tests: `*Test.exe`
- Integration tests: `*IntegrationTest.exe`
- `EnhancedTestRunner`: Test execution framework
- `TestConfigManager`: Test configuration utility

## Build Configuration

### CMake Options

The build system supports various configuration options:

```cmake
# Build configuration options
option(BUILD_ENGINE_ONLY "Build only the engine library" OFF)
option(BUILD_PROJECTS_ONLY "Build only game projects" OFF)
option(BUILD_TESTS_ONLY "Build only test suites" OFF)
option(BUILD_SPECIFIC_PROJECT "Build a specific project only" "")

# Engine configuration
option(ENABLE_VULKAN "Enable Vulkan renderer" OFF)
option(ENABLE_OPENGL "Enable OpenGL renderer" ON)
option(ENABLE_DLSS "Enable NVIDIA DLSS support" OFF)
option(ENABLE_FSR "Enable AMD FSR support" OFF)

# Module configuration options
option(ENABLE_GRAPHICS_MODULE "Enable graphics module" ON)
option(ENABLE_PHYSICS_MODULE "Enable physics module" ON)
option(ENABLE_AUDIO_MODULE "Enable audio module" ON)
```

### Dependency Management

The system automatically handles dependencies:

1. **vcpkg Integration**: Automatic dependency resolution
2. **Module Dependencies**: Modules declare their required packages
3. **Project Dependencies**: Projects inherit engine dependencies
4. **Asset Management**: Automatic asset copying to build directories

## Development Workflow

### Typical Development Cycle

1. **Engine Development**: Use `--engine-only` for engine work
2. **Game Development**: Use `--project ProjectName` for game-specific work
3. **Testing**: Use `--tests-only` for test development
4. **Full Build**: Use default build for releases

### Performance Benefits

- **Faster Builds**: Only build what you need
- **Parallel Development**: Multiple developers can work on different components
- **Incremental Builds**: CMake handles dependency tracking
- **Selective Testing**: Run only relevant tests

## Troubleshooting

### Common Issues

1. **Missing Dependencies**: Run `.\scripts\setup_dependencies.bat`
2. **Build Conflicts**: Clean build directory: `Remove-Item -Recurse -Force build`
3. **Module Not Found**: Check module CMakeLists.txt and registration
4. **Project Link Errors**: Verify engine library is built first

### Debug Information

The build system provides detailed information:

```
-- === Build Configuration Summary ===
-- Engine Modules:
--   - audio-openal
--   - graphics-opengl
--   - physics-bullet
-- Game Projects:
--   - BasicExample
--   - GameExample
-- Build Options:
--   - Engine Only: OFF
--   - Projects Only: OFF
--   - Tests Only: OFF
--   - Specific Project: GameExample
-- ===================================
```

## Best Practices

1. **Use Specific Builds**: Use `--engine-only` or `--project` for faster iteration
2. **Module Independence**: Keep modules self-contained and well-defined
3. **Clean Dependencies**: Declare only necessary dependencies
4. **Test Early**: Use `--tests-only` for test-driven development
5. **Asset Organization**: Keep project assets separate from engine assets

## Future Enhancements

The hierarchical build system is designed to support:

- **Runtime Module Loading**: Dynamic module loading and unloading
- **Hot Module Swapping**: Replace modules without restarting
- **Distributed Builds**: Build components on different machines
- **Module Marketplace**: Share and distribute custom modules
