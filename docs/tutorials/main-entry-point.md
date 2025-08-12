# Tutorial: Main Entry Point

## Step 6: Main Entry Point

### Create Main File

Create `src/main.cpp`:

```cpp
#include "MyGame.h"
#include "Core/Logger.h"
#include <iostream>
#include <exception>

int main() {
    try {
        // Initialize logging
        GameEngine::Logger::Initialize("logs/MyThirdPersonGame.log");

        LOG_INFO("=== Starting My Third Person Game ===");
        LOG_INFO("Engine: Game Engine Kiro v2.0.0");
        LOG_INFO("Project: MyThirdPersonGame v1.0.0");

        // Create and initialize game
        MyGame game;

        if (!game.Initialize()) {
            LOG_ERROR("Failed to initialize game");
            std::cerr << "Failed to initialize game. Check logs for details." << std::endl;
            return -1;
        }

        // Run the game
        LOG_INFO("Starting game loop");
        game.Run();

        LOG_INFO("=== Game Ended Successfully ===");
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Unhandled exception: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        LOG_ERROR("Unknown exception occurred");
        std::cerr << "Fatal error: Unknown exception" << std::endl;
        return -1;
    }
}
```

## Step 7: Building and Running

### Build the Project

```powershell
# Navigate to root directory
cd ../..

# Build the entire solution (including your new project)
.\scripts\build_unified.bat --tests
```

### Run Your Game

```powershell
# Run your game
build\projects\MyThirdPersonGame\Release\MyThirdPersonGame.exe
```

### Monitor Logs

```powershell
# In another terminal, monitor logs
.\scripts\monitor.bat
```

## Step 8: Testing and Validation

### Basic Functionality Test

1. **Window Creation**: Verify the game window opens correctly
2. **Player Movement**: Test WASD movement controls
3. **Camera Control**: Test right-click mouse camera rotation
4. **Physics**: Verify player collision with ground
5. **Jumping**: Test spacebar jumping (if implemented)

### Debug Information

Add debug output to verify systems are working:

```cpp
// In MyGame::Update()
void MyGame::Update(float deltaTime) {
    // Existing update code...

    // Debug output (remove in final version)
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;

    if (debugTimer >= 1.0f) { // Every second
        LOG_INFO("Player Position: (" +
                 std::to_string(m_player->GetPosition().x) + ", " +
                 std::to_string(m_player->GetPosition().y) + ", " +
                 std::to_string(m_player->GetPosition().z) + ")");
        debugTimer = 0.0f;
    }
}
```

## Step 9: Adding Assets

### Basic Assets Structure

Create basic assets for your game:

```
assets/
├── models/
│   └── character.fbx          # Player character model
├── textures/
│   ├── player_diffuse.png     # Player texture
│   ├── ground_diffuse.png     # Ground texture
│   └── skybox/                # Skybox textures
├── materials/
│   ├── player.json            # Player material definition
│   └── ground.json            # Ground material definition
└── audio/
    ├── footsteps.wav          # Footstep sounds
    └── jump.wav               # Jump sound
```

### Sample Material File

Create `assets/materials/player.json`:

```json
{
  "name": "PlayerMaterial",
  "type": "PBR",
  "properties": {
    "diffuseColor": [0.8, 0.8, 0.8, 1.0],
    "specularColor": [0.2, 0.2, 0.2, 1.0],
    "shininess": 32.0,
    "metallic": 0.0,
    "roughness": 0.8
  },
  "textures": {
    "diffuse": "textures/player_diffuse.png",
    "normal": "textures/player_normal.png",
    "specular": "textures/player_specular.png"
  }
}
```

## Step 10: Next Steps

### Enhancements to Consider

1. **Animation System**: Add character animations
2. **Audio Integration**: Add footstep sounds and ambient audio
3. **UI System**: Add health bars, menus, and HUD elements
4. **Level Loading**: Create multiple levels or scenes
5. **Save System**: Implement game state saving/loading
6. **Particle Effects**: Add visual effects for actions
7. **AI Enemies**: Create enemy characters with AI behavior

### Performance Optimization

1. **Profiling**: Use Release builds for performance testing
2. **Asset Optimization**: Optimize textures and models
3. **Culling**: Implement frustum culling for better performance
4. **LOD System**: Add level-of-detail for distant objects

### Project Organization

1. **Code Structure**: Organize code into logical modules
2. **Asset Management**: Create asset loading and caching systems
3. **Configuration**: Use JSON files for game settings
4. **Documentation**: Document your game's features and systems

This tutorial provides a solid foundation for creating new game projects with Game Engine Kiro's modular architecture. The modular design allows you to easily extend functionality and maintain clean separation between different game systems.
