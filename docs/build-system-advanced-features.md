# Build System Advanced Features

This document describes the advanced features available in the Game Engine Kiro build system and how to enable/disable them.

## Overview

The build system includes several advanced features that are designed to be optional and can be enabled or disabled based on your needs. All features are designed to work without any advanced configuration, but can be customized for specific workflows.

## Available Advanced Features

### 1. vcpkg Binary Cache

**Description**: Caches compiled dependencies to speed up subsequent builds.

**Default**: Automatically enabled when possible, disabled in IDE environments for compatibility.

**Manual Control**:

```batch
# Disable cache for current build
.\scripts\build_unified.bat --no-cache --engine

# Permanently disable cache (set environment variable)
set DISABLE_VCPKG_CACHE=1

# Clean cache
.\scripts\build_unified.bat --clean-cache --engine
```

**Configuration**:

- Cache directory: `%USERPROFILE%\.vcpkg-cache`
- Cache mode: Read-only for safety
- Automatic fallback when cache is unavailable

### 2. Ninja Generator Support

**Description**: Uses Ninja build system for faster compilation when available.

**Default**: Automatically selected when Ninja is available and Visual Studio environment is detected.

**Manual Control**:

```batch
# Force Ninja usage
.\scripts\build_unified.bat --ninja --engine

# Prefer Visual Studio generator
set GAMEENGINE_PREFER_VS=1
.\scripts\build_unified.bat --engine
```

**Requirements**:

- Ninja installed and in PATH
- Visual Studio Developer Command Prompt environment
- Installation: `winget install Ninja-build.Ninja`

### 3. CMakePresets Integration

**Description**: Uses modern CMakePresets.json for build configuration.

**Default**: Automatically enabled when CMakePresets.json exists.

**Features**:

- Automatic preset selection based on available tools
- Separate build directories for different configurations
- IDE integration support

**Manual Override**: Not recommended - presets are designed to work automatically.

### 4. Build Performance Monitoring

**Description**: Tracks build times and provides performance reports.

**Default**: Always enabled.

**Features**:

- Build duration tracking
- Performance comparison with previous builds
- Compilation statistics
- Cache effectiveness reporting

**Output**: Displayed at end of each build, logged to `build_diagnostics.log`.

### 5. Granular Cleaning Options

**Description**: Clean specific parts of the build without full rebuild.

**Default**: Always available.

**Options**:

```batch
# Clean specific components
.\scripts\build_unified.bat --clean-engine --engine
.\scripts\build_unified.bat --clean-tests --tests
.\scripts\build_unified.bat --clean-projects --projects
.\scripts\build_unified.bat --clean-cache --engine

# Clean everything
.\scripts\build_unified.bat --clean-all --all
```

### 6. Intelligent Configuration Detection

**Description**: Automatically detects configuration changes and invalidates cache when needed.

**Default**: Always enabled.

**Features**:

- Detects build type changes (Debug/Release)
- Detects specific test/project changes
- Automatic cache invalidation
- Build signature tracking

**Manual Override**: Use `--clean-cache` to force cache invalidation.

## Environment Variables

### Global Configuration

```batch
# Disable vcpkg cache globally
set DISABLE_VCPKG_CACHE=1

# Prefer Visual Studio generator over Ninja
set GAMEENGINE_PREFER_VS=1

# IDE environment detection (automatically set by IDEs)
set KIRO_IDE_SESSION=1
set VSCODE_PID=12345
```

### Temporary Configuration

```batch
# Disable cache for single build
set DISABLE_VCPKG_CACHE=1 && .\scripts\build_unified.bat --engine

# Force Ninja for single build
.\scripts\build_unified.bat --ninja --engine
```

## IDE Integration

### Visual Studio Code

**Automatic Features**:

- Cache disabled for compatibility
- CMakePresets integration
- compile_commands.json generation

**Configuration**: `.vscode/settings.json`, `.vscode/tasks.json`

### Visual Studio

**Automatic Features**:

- Solution file generation
- Multi-configuration support
- IntelliSense integration

**Configuration**: CMakePresets.json handles VS integration

### clangd Language Server

**Automatic Features**:

- compile_commands.json generation
- Header search path detection

**Configuration**: `.clangd` file for custom settings

## Troubleshooting

### Cache Issues

```batch
# Clear all caches and rebuild
.\scripts\build_unified.bat --clean-cache --clean-all --all

# Disable cache if causing problems
.\scripts\build_unified.bat --no-cache --engine
```

### Generator Issues

```batch
# Force Visual Studio generator
set GAMEENGINE_PREFER_VS=1
.\scripts\build_unified.bat --engine

# Check Ninja availability
ninja --version
```

### Performance Issues

```batch
# Use individual test compilation for development
.\scripts\build_unified.bat --tests MathTest

# Monitor build performance
.\scripts\monitor.bat
```

## Best Practices

### Development Workflow

1. **Individual Test Development**: Use `--tests TestName` for faster iteration
2. **Cache Management**: Let the system manage cache automatically
3. **Generator Selection**: Use default selection unless specific needs
4. **Cleaning**: Use granular cleaning options instead of full rebuilds

### CI/CD Integration

1. **Disable Cache**: Use `--no-cache` for reproducible builds
2. **Force Clean**: Use `--clean-all` for clean CI builds
3. **Specific Builds**: Use component-specific builds for faster CI

### IDE Usage

1. **Let IDE Detection Work**: Don't override IDE-specific settings
2. **Use CMakePresets**: Modern IDEs work better with presets
3. **Check compile_commands.json**: Ensure it's generated for language servers

## Advanced Customization

### Custom Cache Directory

```batch
# Set custom cache directory (not recommended)
set VCPKG_CACHE_DIR=D:\custom-cache
.\scripts\build_unified.bat --engine
```

### Custom CMake Arguments

The build system accepts additional CMake arguments through internal mechanisms. For custom builds, modify the CMakeLists.txt or use CMakePresets.json.

### Build System Extension

The build system is designed to be extended through:

- Additional cleaning functions in build_unified.bat
- Custom presets in CMakePresets.json
- Environment variable detection
- Performance monitoring hooks

## Migration Guide

### From Old Build System

1. **No Changes Required**: All existing commands work unchanged
2. **Optional Improvements**: Enable Ninja for faster builds
3. **Cache Benefits**: Automatic dependency caching
4. **Better IDE Support**: Improved integration with modern IDEs

### Disabling Advanced Features

```batch
# Minimal build system (disable all advanced features)
set DISABLE_VCPKG_CACHE=1
set GAMEENGINE_PREFER_VS=1
.\scripts\build_unified.bat --no-cache --engine
```

This provides a build experience similar to the original system while maintaining all compatibility.
