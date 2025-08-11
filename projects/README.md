# Game Engine Kiro - Example Projects

This directory contains example projects that demonstrate different aspects of the Game Engine Kiro.

## Project Structure

Each project is self-contained with its own CMakeLists.txt, source files, and configuration.

### GameExample

- **Purpose**: Comprehensive demonstration of all engine features
- **Features**: Advanced character movement, 3D audio, FBX models, physics debugging
- **Target Audience**: Developers wanting to see full engine capabilities

### BasicExample

- **Purpose**: Clean demonstration of core movement mechanics
- **Features**: WASD movement, jumping, third-person camera
- **Target Audience**: Developers learning engine fundamentals

## Building Projects

### Option 1: Build from Engine Root (Recommended)

The main engine CMakeLists.txt includes these projects automatically:

```bash
.\scripts\build.bat
```

### Option 2: Build Individual Projects

Each project can be built independently:

```bash
cd projects/GameExample
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

**Note**: The engine library must be built first before building individual projects.

## Adding New Projects

1. Create a new directory under `projects/`
2. Add a `CMakeLists.txt` file following the existing patterns
3. Create a `src/` directory for source files
4. Add project-specific assets in an `assets/` directory if needed
5. Update the main engine CMakeLists.txt to include the new project

## Asset Management

- **Shared Assets**: Located in the main `assets/` directory, automatically copied to all projects
- **Project Assets**: Located in each project's `assets/` directory for project-specific resources
- **Configuration**: Each project can have its own configuration files in a `config/` directory
