# Tutorial: Creating a New Game Project

## Overview

This tutorial walks you through creating a new game project using Game Engine Kiro's modular architecture. We'll create a simple third-person game project from scratch.

## Prerequisites

- Game Engine Kiro installed and built successfully
- Basic understanding of C++ and CMake
- Familiarity with the engine's modular architecture

## Step 1: Project Setup

### Create Project Directory

```powershell
# Navigate to projects directory
cd projects

# Create your new project
mkdir MyThirdPersonGame
cd MyThirdPersonGame

# Create directory structure
mkdir src
mkdir include
mkdir assets
mkdir config
```

### Project Structure

Your project should follow this structure:

```
projects/MyThirdPersonGame/
├── CMakeLists.txt           # Build configuration
├── src/                     # Source files
│   ├── main.cpp            # Entry point
│   ├── MyGame.cpp          # Game logic
│   └── Player.cpp          # Player controller
├── include/                 # Header files
│   ├── MyGame.h            # Game class
│   └── Player.h            # Player class
├── assets/                  # Game assets
│   ├── models/             # 3D models
│   ├── textures/           # Textures
│   └── audio/              # Sound files
├── config/                  # Configuration files
│   ├── project.json        # Project configuration
│   └── engine.json         # Engine configuration
└── README.md               # Project documentation
```

## Step 2: Project Configuration

### Create Project Configuration

Create `config/project.json`:

```json
{
  "projectName": "MyThirdPersonGame",
  "projectVersion": "1.0.0",
  "description": "A third-person action game built with Game Engine Kiro",
  "requiredModules": [
    "Core",
    "OpenGLGraphics",
    "BulletPhysics",
    "OpenALAudio",
    "InputManager"
  ],
  "optionalModules": ["AssimpResource", "LuaScripting"],
  "projectSettings": {
    "windowTitle": "My Third Person Game",
    "windowWidth": 1920,
    "windowHeight": 1080,
    "fullscreen": false,
    "vsync": true
  },
  "assetPath": "assets/",
  "configPath": "config/"
}
```

### Create Engine Configuration

Create `config/engine.json`:

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
        "multisampling": "4",
        "shadowQuality": "high"
      }
    },
    {
      "name": "BulletPhysics",
      "version": "1.0.0",
      "enabled": true,
      "parameters": {
        "gravity": "-9.81",
        "maxRigidBodies": "1000",
        "debugDraw": "false"
      }
    },
    {
      "name": "OpenALAudio",
      "version": "1.0.0",
      "enabled": true,
      "parameters": {
        "maxSources": "32",
        "dopplerFactor": "1.0"
      }
    }
  ]
}
```

## Step 3: CMake Configuration

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyThirdPersonGame)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Game Engine Kiro
find_path(ENGINE_ROOT_DIR
    NAMES "engine/core/include/Core/Engine.h"
    PATHS ${CMAKE_SOURCE_DIR}/../..
    REQUIRED
)

# Include engine directories
include_directories(
    ${ENGINE_ROOT_DIR}/include
    ${ENGINE_ROOT_DIR}/engine/core/include
    ${ENGINE_ROOT_DIR}/engine/interfaces
    include/
)

# Source files
set(SOURCES
    src/main.cpp
    src/MyGame.cpp
    src/Player.cpp
)

# Header files
set(HEADERS
    include/MyGame.h
    include/Player.h
)

# Create executable
add_executable(MyThirdPersonGame
    ${SOURCES}
    ${HEADERS}
)

# Link libraries
target_link_libraries(MyThirdPersonGame
    GameEngineKiro
    # Add other required libraries
)

# Copy assets to build directory
add_custom_command(TARGET MyThirdPersonGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:MyThirdPersonGame>/assets
)

# Copy config to build directory
add_custom_command(TARGET MyThirdPersonGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/config
    $<TARGET_FILE_DIR:MyThirdPersonGame>/config
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(MyThirdPersonGame PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(MyThirdPersonGame PRIVATE /W4)
endif()
```
