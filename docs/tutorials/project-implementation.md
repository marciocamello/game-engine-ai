# Tutorial: Project Implementation

## Step 4: Game Class Implementation

### Create Game Header

Create `include/MyGame.h`:

```cpp
#pragma once

#include "Core/Engine.h"
#include "Graphics/GraphicsRenderer.h"
#include "Physics/PhysicsEngine.h"
#include "Audio/AudioEngine.h"
#include "Input/InputManager.h"
#include "Game/ThirdPersonCamera.h"
#include "Player.h"
#include <memory>

class MyGame {
private:
    std::unique_ptr<GameEngine::Engine> m_engine;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<GameEngine::ThirdPersonCamera> m_camera;

    bool m_running = true;
    float m_deltaTime = 0.0f;

public:
    MyGame();
    ~MyGame();

    bool Initialize();
    void Run();
    void Update(float deltaTime);
    void Render();
    void Shutdown();

private:
    void HandleInput();
    void SetupScene();
    void LoadAssets();
};
```

### Create Game Implementation

Create `src/MyGame.cpp`:

```cpp
#include "MyGame.h"
#include "Core/Logger.h"
#include "Core/Math.h"
#include <iostream>

using namespace GameEngine;

MyGame::MyGame() {
    LOG_INFO("MyGame constructor");
}

MyGame::~MyGame() {
    LOG_INFO("MyGame destructor");
}

bool MyGame::Initialize() {
    LOG_INFO("Initializing MyGame");

    try {
        // Load engine configuration
        EngineConfig config = ConfigManager::LoadEngineConfig("config/engine.json");

        // Initialize module registry
        ModuleRegistry& registry = ModuleRegistry::GetInstance();

        // Register required modules
        registry.RegisterModule(std::make_unique<OpenGLGraphicsModule>());
        registry.RegisterModule(std::make_unique<BulletPhysicsModule>());
        registry.RegisterModule(std::make_unique<OpenALAudioModule>());
        registry.RegisterModule(std::make_unique<InputModule>());

        // Create and initialize engine
        m_engine = std::make_unique<Engine>();
        if (!m_engine->Initialize(config)) {
            LOG_ERROR("Failed to initialize engine");
            return false;
        }

        // Create player
        m_player = std::make_unique<Player>();
        if (!m_player->Initialize()) {
            LOG_ERROR("Failed to initialize player");
            return false;
        }

        // Create camera
        m_camera = std::make_unique<ThirdPersonCamera>();
        m_camera->SetTarget(m_player->GetPosition());
        m_camera->SetDistance(5.0f);
        m_camera->SetHeight(2.0f);

        // Setup scene
        SetupScene();
        LoadAssets();

        LOG_INFO("MyGame initialized successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during MyGame initialization: " + std::string(e.what()));
        return false;
    }
}

void MyGame::Run() {
    LOG_INFO("Starting game loop");

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        HandleInput();
        Update(m_deltaTime);
        Render();

        // Check for exit conditions
        if (InputManager::GetInstance().IsKeyPressed(GLFW_KEY_ESCAPE)) {
            m_running = false;
        }
    }

    Shutdown();
}

void MyGame::Update(float deltaTime) {
    // Update engine systems
    m_engine->Update(deltaTime);

    // Update player
    if (m_player) {
        m_player->Update(deltaTime);
    }

    // Update camera
    if (m_camera && m_player) {
        m_camera->SetTarget(m_player->GetPosition());
        m_camera->Update(deltaTime);
    }
}

void MyGame::Render() {
    // Begin frame
    auto* renderer = m_engine->GetRenderer();
    if (!renderer) return;

    renderer->BeginFrame();
    renderer->Clear();

    // Set camera
    if (m_camera) {
        renderer->SetViewMatrix(m_camera->GetViewMatrix());
        renderer->SetProjectionMatrix(m_camera->GetProjectionMatrix());
    }

    // Render player
    if (m_player) {
        m_player->Render(renderer);
    }

    // Render scene objects here...

    // End frame
    renderer->EndFrame();
}

void MyGame::HandleInput() {
    InputManager& input = InputManager::GetInstance();

    if (!m_player) return;

    // Player movement
    Math::Vec3 movement(0.0f);

    if (input.IsKeyPressed(GLFW_KEY_W)) {
        movement.z -= 1.0f;
    }
    if (input.IsKeyPressed(GLFW_KEY_S)) {
        movement.z += 1.0f;
    }
    if (input.IsKeyPressed(GLFW_KEY_A)) {
        movement.x -= 1.0f;
    }
    if (input.IsKeyPressed(GLFW_KEY_D)) {
        movement.x += 1.0f;
    }

    // Normalize movement
    if (Math::Length(movement) > 0.0f) {
        movement = Math::Normalize(movement);
    }

    m_player->SetMovementInput(movement);

    // Camera rotation with mouse
    auto mousePos = input.GetMousePosition();
    static auto lastMousePos = mousePos;

    auto mouseDelta = mousePos - lastMousePos;
    lastMousePos = mousePos;

    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        m_camera->Rotate(mouseDelta.x * 0.01f, mouseDelta.y * 0.01f);
    }
}

void MyGame::SetupScene() {
    LOG_INFO("Setting up scene");

    // Create ground plane
    auto* physics = m_engine->GetPhysicsEngine();
    if (physics) {
        // Add ground plane physics
        physics->CreateStaticPlane(Math::Vec3(0, 1, 0), 0.0f);
    }

    // Setup lighting
    auto* renderer = m_engine->GetRenderer();
    if (renderer) {
        // Add directional light
        renderer->SetDirectionalLight(
            Math::Vec3(-0.5f, -1.0f, -0.5f),  // Direction
            Math::Vec3(1.0f, 1.0f, 0.9f),     // Color
            0.8f                               // Intensity
        );

        // Add ambient light
        renderer->SetAmbientLight(Math::Vec3(0.2f, 0.2f, 0.3f));
    }
}

void MyGame::LoadAssets() {
    LOG_INFO("Loading assets");

    // Load player assets
    if (m_player) {
        m_player->LoadAssets();
    }

    // Load other game assets here...
}

void MyGame::Shutdown() {
    LOG_INFO("Shutting down MyGame");

    // Cleanup in reverse order
    m_camera.reset();
    m_player.reset();

    if (m_engine) {
        m_engine->Shutdown();
        m_engine.reset();
    }

    LOG_INFO("MyGame shutdown complete");
}
```
