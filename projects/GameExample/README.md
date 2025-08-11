# GameExample

This is the comprehensive example project that demonstrates all engine capabilities including:

- Advanced character movement and physics
- Third-person camera system with SpringArm
- 3D spatial audio integration
- FBX model loading and rendering
- Environment objects and scene management
- Professional grid rendering
- Material and texture systems
- Physics debugging visualization

## Building

From the GameExample directory:

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Running

After building, the executable will be in the build directory. The program will automatically copy required assets and configuration files.

## Configuration

- `config/config.json` - Main application configuration
- `config/engine_config.json` - Engine-specific settings

## Assets

This project uses shared assets from the main engine assets directory, plus any project-specific assets in the local assets folder.
