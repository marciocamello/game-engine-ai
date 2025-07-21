# Game Engine Kiro - Technical Stack

## Build System

- **CMake 3.16+**: Primary build system with modern CMake practices
- **vcpkg**: Dependency management with manifest mode (vcpkg.json)
- **C++20 Standard**: Required with MSVC 2019+, GCC 10+, or Clang 10+

## Core Dependencies

- **GLFW3**: Window management and input handling
- **GLM**: OpenGL mathematics library with experimental features enabled
- **GLAD**: OpenGL function loader
- **OpenGL 4.6+**: Graphics API
- **Bullet3**: Primary physics engine (with PhysX planned)
- **OpenAL**: 3D spatial audio system
- **nlohmann/json**: JSON parsing and configuration
- **fmt**: String formatting library

## Optional Dependencies

- **Assimp**: 3D model loading (FBX, GLTF support)
- **Lua**: Scripting engine integration

- **STB**: Image loading utilities

## Development Tools

- **clangd**: Language server with compile_commands.json generation
- **Visual Studio**: Primary IDE on Windows
- **VS Code**: Alternative development environment

## Common Commands

### Initial Setup

```cmd
# Windows setup
git clone <repository>
setup_dependencies.bat
build.bat
```

### Build Commands

```cmd
# Release build
build.bat

# Debug build (manual)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug

# Clean build
rmdir /s /q build
```

### Development Workflow

```cmd
# Development console with multiple options
dev.bat

# Run with logging
start.bat

# Monitor logs in real-time
monitor.bat

# Debug session
debug.bat
```

### Testing

```cmd
# Run integration tests
build\Release\BulletIntegrationTest.exe
build\Release\BulletConversionTest.exe

# Run integration tests
build\Release\BulletIntegrationTest.exe
build\Release\BulletConversionTest.exe
```

## Compiler Configuration

- **MSVC**: `/W4` warning level, `_CRT_SECURE_NO_WARNINGS` defined
- **GCC/Clang**: `-Wall -Wextra -Wpedantic` warnings enabled
- **All Platforms**: `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for clangd support

## Platform-Specific Libraries

- **Windows**: `winmm` for multimedia
- **Linux**: `pthread`, `dl` for threading and dynamic loading
- **macOS**: Cocoa, IOKit, CoreVideo frameworks

## CMake Features

- **Automatic vcpkg detection**: Uses vcpkg toolchain when available
- **Optional dependency handling**: Graceful fallback when dependencies missing
- **Asset copying**: Automatic asset deployment to build directory
- **Multiple executables**: Engine library + example game + tests
