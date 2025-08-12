# Setup Guide

Complete installation and setup instructions for Game Engine Kiro.

## System Requirements

### Minimum Requirements

- **OS**: Windows 10/11, Ubuntu 20.04+, macOS 11+
- **CPU**: Intel i5-4590 / AMD FX 8350 equivalent
- **Memory**: 8 GB RAM
- **Graphics**: OpenGL 4.6 compatible GPU
- **Storage**: 2 GB available space

### Recommended Requirements

- **OS**: Windows 11, Ubuntu 22.04+, macOS 12+
- **CPU**: Intel i7-8700K / AMD Ryzen 5 3600 equivalent
- **Memory**: 16 GB RAM
- **Graphics**: NVIDIA GTX 1060 / AMD RX 580 or better
- **Storage**: 4 GB available space (SSD recommended)

## Prerequisites

### All Platforms

- **Git**: Version control system
- **CMake 3.20+**: Build system generator
- **C++20 Compiler**: See platform-specific requirements below

### Windows

- **Visual Studio 2019+** (Community Edition is free)
  - Include "Desktop development with C++" workload
  - Or **Build Tools for Visual Studio 2019+**
- **Git for Windows**
- **CMake** (can be installed via Visual Studio Installer)

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y libgl1-mesa-dev  # OpenGL development files
```

### macOS

- **Xcode 12+** or **Xcode Command Line Tools**
- **Homebrew** (recommended package manager)

```bash
xcode-select --install
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install cmake git
```

## Automatic Setup (Recommended)

### Windows

```cmd
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
setup_dependencies.bat
.\scripts\build_unified.bat --tests
```

### Linux/macOS

```bash
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
setup_dependencies.bat
.\scripts\build_unified.bat --tests
```

## Manual Setup

If you prefer to install dependencies manually or the automatic setup fails:

### Windows (vcpkg)

```cmd
# Clone and bootstrap vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# Install dependencies
vcpkg install glfw3:x64-windows glm:x64-windows glad:x64-windows
vcpkg install assimp:x64-windows openal-soft:x64-windows bullet3:x64-windows
vcpkg install lua:x64-windows nlohmann-json:x64-windows fmt:x64-windows stb:x64-windows

# Return to project directory
cd ..

# Build project
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt install -y libglfw3-dev libglm-dev libgl1-mesa-dev
sudo apt install -y libassimp-dev libopenal-dev libbullet-dev
sudo apt install -y liblua5.4-dev nlohmann-json3-dev libfmt-dev libstb-dev

# Build project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### macOS (Homebrew)

```bash
# Install dependencies
brew install glfw glm assimp openal-soft bullet lua nlohmann-json fmt

# Build project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

## Build Configuration Options

You can customize the build with these CMake options:

```bash
# Enable Vulkan support (experimental)
cmake .. -DENABLE_VULKAN=ON

# Disable OpenGL support
cmake .. -DENABLE_OPENGL=OFF

# Use system libraries instead of vcpkg (Linux/macOS)
cmake .. -DUSE_SYSTEM_LIBS=ON

# Build in Debug mode
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Enable all optional features
cmake .. -DENABLE_ALL_FEATURES=ON
```

## Verification

After successful compilation, you should have:

```
build/
├── Release/                    # Windows
│   ├── GameExample.exe
│   ├── GameEngineKiro.lib
│   └── assets/
└── GameExample                 # Linux/macOS
```

### Running the Example

```bash
# Windows
cd build\Release
GameExample.exe

# Linux/macOS
cd build
./GameExample
```

## Troubleshooting

### Common Issues

#### "GLFW3 not found"

- **Windows**: Run `setup_dependencies.bat` or install via vcpkg
- **Linux**: `sudo apt install libglfw3-dev`
- **macOS**: `brew install glfw`

#### "OpenGL not found"

- **Windows**: Update graphics drivers
- **Linux**: `sudo apt install libgl1-mesa-dev`
- **macOS**: OpenGL is included with the system

#### "C++20 features not supported"

- Ensure you're using a compatible compiler:
  - **Windows**: Visual Studio 2019 16.11+ or MSVC 19.29+
  - **Linux**: GCC 10+ or Clang 10+
  - **macOS**: Xcode 12+ or Clang 10+

#### vcpkg Integration Issues

```cmd
# Windows - run as administrator if needed
vcpkg integrate install

# Clean vcpkg cache if corrupted
vcpkg remove --outdated
```

#### CMake Configuration Fails

```bash
# Clear CMake cache
rm -rf build/
mkdir build && cd build

# Verbose output for debugging
cmake .. --debug-output
```

### Performance Issues

#### Low FPS

- Update graphics drivers
- Check if integrated graphics is being used instead of dedicated GPU
- Reduce window resolution for testing

#### High Memory Usage

- This is normal during development builds
- Release builds will have optimized memory usage
- Consider reducing texture quality for older systems

## Development Environment

### Recommended IDEs

- **Visual Studio 2019+** (Windows)
- **Visual Studio Code** with C++ extensions
- **CLion** (JetBrains)
- **Qt Creator**

### Useful Extensions (VS Code)

- C/C++ (Microsoft)
- CMake Tools
- GitLens
- Shader languages support

## Next Steps

1. Read the [Quick Start Guide](quickstart.md)
2. Explore the [Architecture Documentation](architecture.md)
3. Check out the [API Reference](api-reference.md)
4. Start building your game!

## Getting Help

If you encounter issues:

1. Check this troubleshooting section
2. Search existing GitHub issues
3. Create a new issue with:
   - Your operating system and version
   - Compiler version
   - Complete error messages
   - Steps to reproduce

---

**Happy coding!** 🚀
