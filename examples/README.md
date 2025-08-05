# Game Engine Kiro - Examples

This directory contains example applications that demonstrate the capabilities of Game Engine Kiro. The examples are organized into two main categories: basic and enhanced demonstrations.

## Example Structure

### Basic Example (`basic_example.cpp` → `BasicExample.exe`)

**Purpose**: Clean demonstration of core character movement mechanics without distractions.

**Features Demonstrated**:

- Essential character movement (WASD controls)
- Character jumping with physics simulation
- Third-person camera system with mouse control
- Movement component switching (CharacterMovement, PhysicsMovement, HybridMovement)
- Basic physics collision detection
- Simple ground plane rendering

**Target Audience**:

- Developers learning the engine fundamentals
- Understanding core movement mechanics
- Getting started with character controllers

**Controls**:

- `WASD` - Move character
- `Space` - Jump
- `Mouse` - Look around (third-person camera)
- `ESC` - Toggle mouse capture
- `1/2/3` - Switch movement components
- `F1` - Exit application

### Enhanced Example (`main.cpp` → `GameExample.exe`)

**Purpose**: Comprehensive showcase of all engine capabilities and systems.

**Features Demonstrated**:

- **Physics System**: Collision detection, rigid bodies, movement components
- **Rendering System**: Primitives, meshes, textures, shaders, professional grid
- **Audio System**: 3D spatial audio, background music, sound effects
- **Resource System**: Model loading, texture loading, resource management
- **Input System**: Keyboard, mouse, responsive controls with feedback
- **Camera System**: Third-person camera, smooth movement, collision

**Advanced Features**:

- FBX T-Poser character model with fallback to capsule
- GameAudioManager with background music and sound effects
- Professional grid system with dark background
- Environment objects with different material properties
- Performance monitoring and resource management
- Comprehensive debug controls and system status

**Target Audience**:

- Developers evaluating engine capabilities
- Showcasing complete feature set
- Performance and stability testing
- Professional demonstrations

**Controls**:

- `WASD` - Move character (with footstep audio)
- `Space` - Jump (with sound effect)
- `Mouse` - Look around (third-person camera)
- `1/2/3` - Switch movement components
- `F1` - Exit application
- `F2` - Test fall detection system
- `F3` - Toggle debug capsule visualization
- `F4` - Show comprehensive system status
- `F5` - Show performance report
- `F6` - Show asset validation status
- `F7` - Force resource cleanup
- `F11` - Toggle fullscreen
- `ESC` - Toggle mouse capture

## Building and Running

### Prerequisites

- Windows 10/11 with Visual Studio 2019+
- CMake 3.16+
- vcpkg dependency manager

### Build Instructions

```powershell
# Build both examples (ONLY permitted build command)
.\scripts\build.bat

# Run basic example
.\build\Release\BasicExample.exe

# Run enhanced example
.\build\Release\GameExample.exe
```

### Development Workflow

```powershell
# Development console with multiple options
.\scripts\dev.bat

# Run with logging
.\scripts\start.bat

# Monitor logs in real-time
.\scripts\monitor.bat

# Debug session
.\scripts\debug.bat
```

## Example Comparison

| Feature                | Basic Example | Enhanced Example |
| ---------------------- | ------------- | ---------------- |
| Character Movement     | ✓             | ✓                |
| Physics Simulation     | ✓             | ✓                |
| Third-Person Camera    | ✓             | ✓                |
| Audio System           | ✗             | ✓                |
| FBX Model Loading      | ✗             | ✓                |
| Environment Objects    | ✗             | ✓                |
| Professional Grid      | ✗             | ✓                |
| Performance Monitoring | ✗             | ✓                |
| Resource Management    | ✗             | ✓                |
| Debug Visualization    | ✗             | ✓                |
| Comprehensive Logging  | ✗             | ✓                |

## Asset Requirements

### Basic Example

- No external assets required
- Uses built-in primitive rendering
- Fallback systems handle missing resources

### Enhanced Example

- **Audio**: `assets/audio/` directory with background music and sound effects
- **Models**: `assets/meshes/XBot.fbx` for character model
- **Textures**: `assets/textures/wall.jpg` for environment objects
- **Fallbacks**: Graceful degradation when assets are missing

## Development Notes

### Code Organization

- **Basic Example**: Minimal code focusing on core functionality
- **Enhanced Example**: Comprehensive implementation with all systems
- **Shared Dependencies**: Both examples use the same engine library

### Performance Targets

- **Basic Example**: Lightweight, 60+ FPS on modest hardware
- **Enhanced Example**: Full feature set, 60+ FPS with all systems active

### Testing

Both examples are automatically built and can be tested using:

```powershell
# Run all tests to ensure examples build correctly
.\scripts\run_tests.bat
```

## Other Examples

The examples directory also contains specialized test applications:

- `character_controller_test.cpp` - Character controller testing
- `physics_debug_example.cpp` - Physics system debugging
- `model_hot_reload_example.cpp` - Model hot-reloading demonstration
- `test_fbx_*.cpp` - FBX loading and animation tests
- `debug_*.cpp` - Various debugging utilities

These are primarily for development and testing purposes.

## Troubleshooting

### Common Issues

1. **Build Failures**: Use `.\scripts\build.bat` (never cmake directly)
2. **Missing Assets**: Enhanced example will use fallbacks
3. **Audio Issues**: Enhanced example continues without audio if OpenAL fails
4. **Performance Issues**: Check F5 performance report in enhanced example

### Getting Help

- Check logs in `logs/` directory
- Use `.\scripts\monitor.bat` for real-time log monitoring
- Enable debug visualization with F3 in enhanced example
- Review comprehensive system status with F4 in enhanced example

## Contributing

When adding new examples:

1. Follow the established naming convention
2. Add appropriate CMake targets
3. Update this README with new example information
4. Ensure examples build with `.\scripts\build.bat`
5. Test examples run correctly after build
