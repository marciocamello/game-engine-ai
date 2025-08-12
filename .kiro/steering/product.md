# Game Engine Kiro - Product Overview

Game Engine Kiro is a modern 3D game engine built with AI assistance, specifically designed for third-person action games and open-world experiences.

## Core Vision

- **AI-Assisted Development**: Built using cutting-edge AI development tools (KiroDev, Claude 4, GPT-4.1, GitHub Copilot)
- **Third-Person Focus**: Specialized for over-the-shoulder camera systems and character-based gameplay
- **Dual Physics Strategy**: Supports both Bullet Physics and NVIDIA PhysX for maximum flexibility
- **Modern C++20**: Leverages latest language features for performance and safety

## Key Features

### Modular Plugin System

- **Dynamic Module Loading**: Runtime module management without engine restart
- **Module Registry**: Centralized discovery and dependency resolution
- **Hot-Swappable Modules**: Replace graphics, physics, or audio backends on-the-fly
- **Configuration-Driven**: JSON-based module configuration and behavior control

### Core Engine Features

- **Windows-Focused**: Optimized for Windows development
- **OpenGL 4.6+ Rendering**: Modern graphics pipeline with PBR shading
- **Professional Camera System**: Third-person camera with SpringArm component
- **Dual Physics Backends**: Intelligent selection between Bullet Physics and PhysX
- **3D Spatial Audio**: OpenAL integration with positional audio
- **Hot-Reloadable Shaders**: Automatic compilation and reloading
- **Resource Management**: Automatic loading, caching, and lifetime management

## Target Audience

- **Indie Developers**: Easy-to-use engine with comprehensive documentation
- **Educational Use**: Clean architecture for learning game engine development
- **Prototyping**: Built-in primitives and rapid development tools
- **Third-Person Games**: Specialized systems for character-based gameplay

## Development Philosophy

- **Performance-First**: Cache-friendly data structures and minimal allocations
- **RAII and Smart Pointers**: Automatic memory management
- **Template Metaprogramming**: Compile-time optimizations
- **Thread-Safe Design**: Ready for future parallelization
