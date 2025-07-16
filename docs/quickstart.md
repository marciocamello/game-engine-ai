# Quick Start Guide

Get up and running with Game Engine Kiro in minutes!

## 🚀 One-Minute Setup

### Windows

```cmd
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
setup_dependencies.bat && build.bat
```

### Linux/macOS

```bash
git clone https://github.com/yourusername/GameEngineKiro.git
cd GameEngineKiro
chmod +x *.sh && ./setup_dependencies.sh && ./build.sh
```

That's it! Your game engine is ready to use.

## 🎮 First Run

After building, run the example:

```bash
# Windows
cd build\Release
GameExample.exe

# Linux/macOS
cd build
./GameExample
```

### Controls

- **Mouse**: Look around (360° rotation)
- **WASD**: Move character
- **Mouse Wheel**: Zoom in/out
- **Space**: Jump
- **ESC**: Toggle mouse capture
- **F11**: Toggle fullscreen
- **F1**: Exit

## 🏗️ What You Get

After setup, you have a complete game engine with:

✅ **3D Rendering Pipeline** - Modern OpenGL with PBR shaders  
✅ **Third-Person Camera** - Professional over-the-shoulder system  
✅ **Physics Simulation** - Bullet Physics with character controller  
✅ **Input Management** - Keyboard, mouse, and gamepad support  
✅ **Audio System** - 3D spatial audio with OpenAL  
✅ **Resource Management** - Automatic asset loading and caching  
✅ **Cross-Platform** - Windows, Linux, macOS support

## 📁 Project Structure

```
GameEngineKiro/
├── include/          # Engine headers
│   ├── Core/        # Core systems (Engine, Logger, Math)
│   ├── Graphics/    # Rendering (Camera, Renderer, Shaders)
│   ├── Physics/     # Physics simulation
│   ├── Audio/       # Audio system
│   ├── Input/       # Input handling
│   └── Game/        # Game components (Character, Camera)
├── src/             # Implementation files
├── examples/        # Sample game (main.cpp)
├── assets/          # Shaders and resources
├── build/           # Compiled output
└── docs/            # Documentation
```

## 🎯 Your First Modification

Let's modify the character color to see the engine in action:

### 1. Open the Character File

Edit `src/Game/Character.cpp` and find this line:

```cpp
Math::Vec4 m_color{0.2f, 0.6f, 1.0f, 1.0f}; // Blue character
```

### 2. Change the Color

```cpp
Math::Vec4 m_color{1.0f, 0.2f, 0.2f, 1.0f}; // Red character
```

### 3. Rebuild and Run

```cmd
build.bat  # Windows
./build.sh # Linux/macOS
```

Your character is now red! 🔴

## 🎨 Customize the Camera

Want to adjust the camera? Edit `src/Game/ThirdPersonCameraSystem.cpp`:

```cpp
// Make camera closer
m_springArm.SetLength(2.0f);  // Default: 3.5f

// Adjust shoulder offset
m_springArm.SetShoulderOffset(1.0f, 0.3f);  // Default: 2.2f, 0.8f

// Change mouse sensitivity
m_mouseSensitivity = 1.5f;  // Default: 2.0f
```

## 🔧 Add Your Game Logic

The main game loop is in `examples/main.cpp`. Here's where to add your code:

```cpp
void Update(float deltaTime) {
    // Your game logic here

    // Example: Rotate character automatically
    static float rotation = 0.0f;
    rotation += deltaTime * 45.0f; // 45 degrees per second
    m_character->SetRotation(rotation);

    // Update character and camera
    m_character->Update(deltaTime, m_engine.GetInput(), m_camera.get());
    m_camera->Update(deltaTime, m_engine.GetInput());
}
```

## 🎵 Add Sound Effects

```cpp
// In your game initialization
auto* audio = m_engine.GetAudio();
auto jumpSound = audio->LoadAudioClip("assets/sounds/jump.wav");

// In your update loop (when player jumps)
if (input->IsKeyPressed(KeyCode::Space)) {
    uint32_t source = audio->CreateAudioSource();
    audio->PlayAudioSource(source, jumpSound);
}
```

## 🏃‍♂️ Performance Tips

### Debug vs Release

- **Debug builds**: Slower but easier to debug
- **Release builds**: Optimized for performance

```bash
# Build in Release mode (default)
cmake --build . --config Release

# Build in Debug mode
cmake --build . --config Debug
```

### Graphics Settings

Adjust these in `src/Core/Engine.cpp`:

```cpp
RenderSettings renderSettings;
renderSettings.windowWidth = 1920;   // Lower for better performance
renderSettings.windowHeight = 1080;
renderSettings.fullscreen = true;    // Better performance
renderSettings.vsync = true;         // Smooth but may reduce FPS
renderSettings.msaaSamples = 4;      // Lower for better performance
```

## 🐛 Common Issues

### Game Won't Start

```bash
# Check if all dependencies are installed
./setup_dependencies.sh  # Linux/macOS
setup_dependencies.bat   # Windows

# Rebuild from scratch
rm -rf build/            # Linux/macOS
rmdir /s /q build        # Windows
./build.sh               # Linux/macOS
build.bat                # Windows
```

### Low FPS

- Update graphics drivers
- Try windowed mode instead of fullscreen
- Reduce MSAA samples in render settings
- Check if using integrated graphics instead of dedicated GPU

### Mouse Not Working

- Press ESC to toggle mouse capture
- Make sure the game window has focus
- Try fullscreen mode (F11)

## 📚 Next Steps

Now that you have the engine running:

1. **Read the [Architecture Guide](architecture.md)** - Understand how the engine works
2. **Check the [API Reference](api-reference.md)** - Learn about available classes and functions
3. **Explore [Physics Strategy](physics-strategy.md)** - Understand the dual physics backend
4. **Study the Examples** - Look at `examples/main.cpp` for implementation patterns

## 🎮 Game Development Tips

### Start Small

- Begin with simple mechanics
- Add complexity gradually
- Test frequently

### Use the Engine's Strengths

- Third-person character games
- Physics-based interactions
- 3D environments with spatial audio

### Performance Considerations

- Profile early and often
- Use the physics system efficiently
- Optimize graphics settings for your target hardware

## 🤝 Getting Help

- **Documentation**: Check the `docs/` folder
- **Examples**: Study `examples/main.cpp`
- **Issues**: Create GitHub issues for bugs
- **Community**: Join discussions in GitHub Discussions

---

**Ready to build amazing games!** 🎮✨

_Next: [Architecture Overview](architecture.md)_
