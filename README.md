# Game Engine Kiro

A modern 3D game engine built with AI assistance, designed for third-person action games and open-world experiences.

## 🤖 Built with AI

This game engine is being developed using cutting-edge AI development tools and methodologies:

- **KiroDev (AWS)**: AI-powered development assistant for architecture and code generation
- **Claude 4**: Advanced reasoning for complex system design and optimization
- **GPT-4.1**: Code analysis, debugging, and feature implementation
- **GitHub Copilot**: Real-time code completion and suggestions
- **VS Code**: Enhanced with AI extensions for intelligent development

The combination of human expertise and AI assistance allows for rapid prototyping, robust architecture, and innovative solutions in game engine development.

## ✨ Features

### Core Architecture

- **Modular Design**: Clean separation of concerns with well-defined interfaces
- **Modern C++20**: Leveraging latest language features for performance and safety
- **Cross-Platform**: Windows support with Linux/macOS planned
- **AI-Assisted Development**: Continuous improvement through AI code analysis

### Graphics & Rendering

- **OpenGL 4.6+**: Modern graphics pipeline with PBR shading
- **Third-Person Camera**: Professional over-the-shoulder camera system with SpringArm
- **Shader Management**: Hot-reloadable shaders with automatic compilation
- **Primitive Rendering**: Built-in shapes for rapid prototyping

### Physics & Simulation

- **🎯 Industry-Standard Raycast Movement**: Following the same approach as Unity, Unreal Engine, Godot, and Source Engine
- **Component-Based Movement System**: 3 specialized movement components for different use cases
  - **CharacterMovement**: Raycast-based deterministic movement (perfect for players, NPCs)
  - **HybridMovement**: Physics collision + direct control (recommended for third-person games)
  - **PhysicsMovement**: Full physics simulation (ideal for vehicles, ragdolls)
- **Bullet Physics Integration**: Solid, tested physics backend with comprehensive API
- **Fall Detection System**: Automatic character reset when falling off the world
- **Runtime Component Switching**: Change movement types dynamically during gameplay

### Input & Controls

- **Multi-Device Support**: Keyboard, mouse, and gamepad input
- **Action Mapping**: Flexible input binding system
- **Mouse Capture**: Professional FPS-style mouse handling for camera control
- **Fullscreen Toggle**: Seamless switching between windowed and fullscreen modes

### Audio System

- **3D Spatial Audio**: Positional audio with distance attenuation
- **OpenAL Integration**: Cross-platform audio backend
- **Resource Management**: Automatic loading and caching of audio assets

## 🚀 Quick Start

### Windows (Recommended)

```cmd
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
setup_dependencies.bat
build.bat
```

### Linux/macOS

```bash
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
chmod +x setup_dependencies.sh build.sh
./setup_dependencies.sh
./build.sh
```

## 📖 Documentation

- **[Setup Guide](docs/setup.md)** - Detailed installation instructions
- **[Quick Start](docs/quickstart.md)** - Get up and running in minutes
- **[Deterministic Physics](docs/deterministic-physics.md)** - Revolutionary component-based movement system
- **[Physics Strategy](docs/physics-strategy.md)** - Hybrid physics architecture and dual backend strategy
- **[API Reference](docs/api-reference.md)** - Complete API documentation
- **[Architecture](docs/architecture.md)** - Engine design and patterns

## 🎮 Controls

### Basic Controls

- **Mouse**: 360° camera rotation (captured by default)
- **WASD**: Character movement relative to camera
- **Space**: Jump
- **ESC**: Toggle mouse capture
- **F1**: Exit game
- **F2**: Test fall detection (teleport character high up)

### Movement Component Switching

- **1**: CharacterMovement (raycast-based, deterministic)
- **2**: PhysicsMovement (full physics simulation)
- **3**: HybridMovement (physics collision + direct control) - **RECOMMENDED**

### Advanced Controls

- **F11**: Toggle fullscreen
- **Mouse Wheel**: Zoom in/out (camera distance)

## 🏗️ Architecture

```
GameEngineKiro/
├── include/          # Public headers
│   ├── Core/        # Engine core systems
│   ├── Graphics/    # Rendering and graphics
│   ├── Physics/     # Physics simulation
│   ├── Audio/       # Audio system
│   ├── Input/       # Input handling
│   └── Game/        # Game-specific components
├── src/             # Implementation files
├── examples/        # Sample applications
├── assets/          # Game assets (shaders, textures)
├── docs/            # Documentation
└── vcpkg/           # Dependencies (auto-generated)
```

## 🔧 Dependencies

### Core Dependencies

- **CMake 3.20+**: Build system
- **C++20 Compiler**: MSVC 2019+, GCC 10+, or Clang 10+
- **GLFW3**: Window management and input
- **GLM**: OpenGL mathematics library
- **OpenGL 4.6+**: Graphics API

### Optional Dependencies

- **Assimp**: 3D model loading
- **OpenAL**: 3D audio
- **Bullet3**: Physics simulation
- **Lua**: Scripting support
- **nlohmann/json**: JSON parsing

All dependencies are automatically managed through vcpkg.

## 🎯 Roadmap

### Current (v1.0)

- [x] Core engine architecture
- [x] OpenGL rendering pipeline
- [x] Third-person camera system
- [x] Input management
- [x] Basic physics (Bullet)
- [x] Audio system
- [x] Resource management

### Next (v1.1)

- [ ] NVIDIA PhysX integration
- [ ] Advanced shader system
- [ ] 3D model loading (FBX, GLTF)
- [ ] Animation system
- [ ] Particle effects

### Future (v1.5+)

- [ ] Vulkan renderer
- [ ] DLSS/FSR support
- [ ] Networking
- [ ] Visual editor
- [ ] Console support

## 🤝 Contributing

We welcome contributions! This project demonstrates how AI-assisted development can accelerate game engine creation while maintaining code quality.

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **AI Development Tools**: This project showcases the power of AI-assisted development
- **Open Source Community**: Built on the shoulders of amazing open-source libraries
- **Game Development Community**: Inspired by modern game engine design patterns

---

**Game Engine Kiro** - Where AI meets game development. Built for the future of interactive entertainment.
