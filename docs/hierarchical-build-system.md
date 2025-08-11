# Hierarchical CMake Build System

## Overview

The Game Engine Kiro now features a hierarchical CMake build system that supports modular engine architecture and flexible project management. This system allows for building individual components, entire solutions, or specific projects based on your development needs.

## Build System Architecture

### Root Level (CMakeLists.txt)

The root CMakeLists.txt orchestrates the entire build process:

- **Engine Discovery**: Automatically discovers engine modules in `engine/modules/`
- **Project Discovery**: Automatically discovers game projects in `projects/`
- **Conditional Building**: Supports building specific components or the entire solution
- **Module Management**: Handles module dependencies and registration

### Engine Modules

Engine modules are located in `engine/modules/` and each has its own CMakeLists.txt:

```
engine/modules/
├── audio-openal/CMakeLists.txt      # OpenAL audio module
├── graphics-opengl/CMakeLists.txt   # OpenGL graphics module
├── physics-bullet/CMakeLists.txt    # Bullet Physics module
└── templates/ModuleTemplate.cmake   # Template for new modules
```

### Game Projects

Game projects are located in `projects/` and each has its own CMakeLists.txt:

```
projects/
├── GameExample/CMakeLists.txt       # Enhanced example game
├── BasicExample/CMakeLists.txt      # Simple example game
└── templates/ProjectTemplate.cmake  # Template for new projects
```

## Build Options

### CMake Configuration Options

| Option                   | Description                               | Default |
| ------------------------ | ----------------------------------------- | ------- |
| `BUILD_ENGINE_ONLY`      | Build only the engine library and modules | OFF     |
| `BUILD_PROJECTS_ONLY`    | Build only game projects                  | OFF     |
| `BUILD_TESTS_ONLY`       | Build only test suites                    | OFF     |
| `BUILD_SPECIFIC_PROJECT` | Build a specific project only             | ""      |
| `ENABLE_GRAPHICS_MODULE` | Enable graphics module                    | ON      |
| `ENABLE_PHYSICS_MODULE`  | Enable physics module                     | ON      |
| `ENABLE_AUDIO_MODULE`    | Enable audio module                       | ON      |

### Module Configuration Options

Individual modules can be enabled or disabled:

```cmake
# Enable/disable specific modules
cmake -DENABLE_GRAPHICS_MODULE=ON \
      -DENABLE_PHYSICS_MODULE=OFF \
      -DENABLE_AUDIO_MODULE=ON \
      ..
```

## Usage Examples

### Building Everything (Default)

```bash
# Windows
.\scripts\build.bat

# Or using the new hierarchical script
.\scripts\build_hierarchical.bat
```

### Building Only the Engine

```bash
# Windows
.\scripts\build_hierarchical.bat --engine-only

# Or with CMake directly
cmake -DBUILD_ENGINE_ONLY=ON ..
cmake --build . --config Release
```

### Building Only Projects

```bash
# Windows (requires pre-built engine)
.\scripts\build_hierarchical.bat --projects-only

# Or with CMake directly
cmake -DBUILD_PROJECTS_ONLY=ON ..
cmake --build . --config Release
```

### Building a Specific Project

```bash
# Windows
.\scripts\build_hierarchical.bat --project GameExample

# Or with CMake directly
cmake -DBUILD_SPECIFIC_PROJECT=GameExample ..
cmake --build . --config Release
```

### Building Only Tests

```bash
# Windows
.\scripts\build_hierarchical.bat --tests-only

# Or with CMake directly
cmake -DBUILD_TESTS_ONLY=ON ..
cmake --build . --config Release
```

## Module Development

### Creating a New Engine Module

1. Create a new directory in `engine/modules/`:

   ```
   engine/modules/your-module-name/
   ```

2. Copy the module template:

   ```bash
   copy engine\modules\templates\ModuleTemplate.cmake engine\modules\your-module-name\CMakeLists.txt
   ```

3. Customize the CMakeLists.txt:

   ```cmake
   set(MODULE_NAME "your-module-name")
   set(MODULE_VERSION "1.0.0")
   set(MODULE_TYPE "YourModuleType")

   set(MODULE_SOURCES
       YourModule.cpp
   )

   set(MODULE_HEADERS
       YourModule.h
   )
   ```

4. The module will be automatically discovered and included in the build.

### Module Structure

Each module CMakeLists.txt should:

- Define module metadata (name, version, type)
- List source and header files
- Specify required and optional dependencies
- Configure the module library
- Register with the parent engine target

## Project Development

### Creating a New Game Project

1. Create a new directory in `projects/`:

   ```
   projects/YourProjectName/
   ```

2. Copy the project template:

   ```bash
   copy projects\templates\ProjectTemplate.cmake projects\YourProjectName\CMakeLists.txt
   ```

3. Customize the CMakeLists.txt:

   ```cmake
   set(PROJECT_NAME "YourProjectName")
   set(PROJECT_VERSION "1.0.0")

   set(REQUIRED_ENGINE_MODULES
       graphics-opengl
       physics-bullet
   )

   set(PROJECT_SOURCES
       src/main.cpp
   )
   ```

4. The project will be automatically discovered and included in the build.

### Project Structure

Each project CMakeLists.txt should:

- Define project metadata (name, version, type)
- Specify required and optional engine modules
- List source and header files
- Configure the project executable
- Handle asset and configuration copying

## Module Dependencies

### Declaring Module Dependencies

Projects declare their module dependencies:

```cmake
# Required modules (build will fail if missing)
set(REQUIRED_ENGINE_MODULES
    graphics-opengl
    physics-bullet
)

# Optional modules (used if available)
set(OPTIONAL_ENGINE_MODULES
    audio-openal
    scripting-lua
)
```

### Module Validation

The build system validates that required modules are available and provides warnings for missing optional modules.

## Build Performance

### Incremental Builds

The hierarchical system supports efficient incremental builds:

- **Module Changes**: Only affected modules and dependent projects rebuild
- **Project Changes**: Only the specific project rebuilds
- **Engine Core Changes**: All dependent modules and projects rebuild

### Parallel Builds

CMake automatically handles parallel compilation of independent modules and projects.

## Integration with Existing Workflow

### Backward Compatibility

The new system maintains backward compatibility:

- Existing `.\scripts\build.bat` continues to work
- All existing projects build without modification
- Test execution remains unchanged

### Migration Path

Existing projects can gradually adopt the new module dependency system:

1. Update project CMakeLists.txt to declare module dependencies
2. Move module-specific code to appropriate module directories
3. Update build scripts to use new hierarchical options

## Troubleshooting

### Common Issues

1. **Module Not Found**: Ensure the module directory exists and has a CMakeLists.txt
2. **Dependency Errors**: Check that required packages are installed via vcpkg
3. **Build Order Issues**: The system automatically handles build order based on dependencies

### Debug Information

Enable verbose output to see module discovery and dependency resolution:

```bash
cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..
```

## Future Enhancements

### Planned Features

- **Dynamic Module Loading**: Runtime module discovery and loading
- **Module Marketplace**: Centralized module repository
- **Cross-Platform Templates**: Linux and macOS support
- **Module Versioning**: Semantic versioning and compatibility checking

### Extension Points

The system is designed to be extensible:

- Custom module types
- Project-specific build rules
- Advanced dependency management
- Plugin architecture support
