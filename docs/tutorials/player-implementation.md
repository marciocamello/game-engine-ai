# Tutorial: Player Implementation

## Step 5: Player Class Implementation

### Create Player Header

Create `include/Player.h`:

```cpp
#pragma once

#include "Core/Math.h"
#include "Graphics/GraphicsRenderer.h"
#include "Physics/PhysicsEngine.h"
#include "Resource/ResourceManager.h"
#include <memory>

class Player {
private:
    GameEngine::Math::Vec3 m_position{0.0f, 1.0f, 0.0f};
    GameEngine::Math::Vec3 m_rotation{0.0f};
    GameEngine::Math::Vec3 m_velocity{0.0f};
    GameEngine::Math::Vec3 m_movementInput{0.0f};

    float m_speed = 5.0f;
    float m_jumpForce = 10.0f;
    bool m_isGrounded = false;

    // Rendering
    std::shared_ptr<GameEngine::Mesh> m_mesh;
    std::shared_ptr<GameEngine::Material> m_material;

    // Physics
    void* m_rigidBody = nullptr;  // Bullet physics body

public:
    Player();
    ~Player();

    bool Initialize();
    void Update(float deltaTime);
    void Render(GameEngine::GraphicsRenderer* renderer);
    void Shutdown();

    // Input handling
    void SetMovementInput(const GameEngine::Math::Vec3& input);
    void Jump();

    // Getters
    const GameEngine::Math::Vec3& GetPosition() const { return m_position; }
    const GameEngine::Math::Vec3& GetRotation() const { return m_rotation; }
    bool IsGrounded() const { return m_isGrounded; }

    // Asset loading
    bool LoadAssets();

private:
    void UpdatePhysics(float deltaTime);
    void UpdateMovement(float deltaTime);
    void CheckGrounded();
};
```

### Create Player Implementation

Create `src/Player.cpp`:

```cpp
#include "Player.h"
#include "Core/Logger.h"
#include "Core/Engine.h"
#include "Physics/BulletUtils.h"
#include <algorithm>

using namespace GameEngine;
using namespace GameEngine::Math;

Player::Player() {
    LOG_INFO("Player constructor");
}

Player::~Player() {
    Shutdown();
    LOG_INFO("Player destructor");
}

bool Player::Initialize() {
    LOG_INFO("Initializing Player");

    try {
        // Create physics body
        auto* physics = Engine::GetInstance().GetPhysicsEngine();
        if (physics) {
            // Create capsule collision shape for player
            auto* shape = physics->CreateCapsuleShape(0.5f, 1.8f);

            // Create rigid body
            m_rigidBody = physics->CreateRigidBody(
                shape,
                m_position,
                Quaternion::Identity(),
                1.0f  // mass
            );

            if (!m_rigidBody) {
                LOG_ERROR("Failed to create player physics body");
                return false;
            }

            // Configure physics properties
            physics->SetRigidBodyFriction(m_rigidBody, 0.8f);
            physics->SetRigidBodyRestitution(m_rigidBody, 0.0f);
            physics->SetRigidBodyAngularFactor(m_rigidBody, Vec3(0, 1, 0)); // Only Y rotation
        }

        // Load assets
        if (!LoadAssets()) {
            LOG_ERROR("Failed to load player assets");
            return false;
        }

        LOG_INFO("Player initialized successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during Player initialization: " + std::string(e.what()));
        return false;
    }
}

void Player::Update(float deltaTime) {
    // Update physics
    UpdatePhysics(deltaTime);

    // Update movement
    UpdateMovement(deltaTime);

    // Check if grounded
    CheckGrounded();

    // Update position from physics body
    if (m_rigidBody) {
        auto* physics = Engine::GetInstance().GetPhysicsEngine();
        if (physics) {
            m_position = physics->GetRigidBodyPosition(m_rigidBody);
            // Keep rotation for rendering but don't get it from physics
            // since we're constraining angular movement
        }
    }
}

void Player::Render(GraphicsRenderer* renderer) {
    if (!renderer || !m_mesh || !m_material) {
        return;
    }

    // Create transform matrix
    Mat4 transform = Mat4::Identity();
    transform = Mat4::Translate(transform, m_position);
    transform = Mat4::RotateY(transform, m_rotation.y);

    // Render the player mesh
    renderer->RenderMesh(m_mesh.get(), m_material.get(), transform);
}

void Player::SetMovementInput(const Vec3& input) {
    m_movementInput = input;
}

void Player::Jump() {
    if (!m_isGrounded || !m_rigidBody) {
        return;
    }

    auto* physics = Engine::GetInstance().GetPhysicsEngine();
    if (physics) {
        // Apply upward impulse
        Vec3 jumpImpulse(0.0f, m_jumpForce, 0.0f);
        physics->ApplyImpulse(m_rigidBody, jumpImpulse);

        m_isGrounded = false;
        LOG_INFO("Player jumped");
    }
}

bool Player::LoadAssets() {
    LOG_INFO("Loading player assets");

    auto* resourceManager = Engine::GetInstance().GetResourceManager();
    if (!resourceManager) {
        LOG_ERROR("ResourceManager not available");
        return false;
    }

    try {
        // Load player mesh (try to load a character model, fallback to cube)
        m_mesh = resourceManager->LoadMesh("models/character.fbx");
        if (!m_mesh) {
            LOG_WARNING("Character model not found, using default cube");
            m_mesh = resourceManager->GetDefaultCube();
        }

        // Load player material
        m_material = resourceManager->LoadMaterial("materials/player.json");
        if (!m_material) {
            LOG_WARNING("Player material not found, using default");
            m_material = resourceManager->GetDefaultMaterial();
        }

        // Load textures if needed
        auto texture = resourceManager->LoadTexture("textures/player_diffuse.png");
        if (texture && m_material) {
            m_material->SetDiffuseTexture(texture);
        }

        LOG_INFO("Player assets loaded successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception loading player assets: " + std::string(e.what()));
        return false;
    }
}

void Player::UpdatePhysics(float deltaTime) {
    if (!m_rigidBody) {
        return;
    }

    auto* physics = Engine::GetInstance().GetPhysicsEngine();
    if (!physics) {
        return;
    }

    // Get current velocity
    Vec3 currentVelocity = physics->GetRigidBodyVelocity(m_rigidBody);

    // Calculate desired movement velocity
    Vec3 desiredVelocity = m_movementInput * m_speed;

    // Only modify horizontal velocity, keep vertical velocity from physics
    desiredVelocity.y = currentVelocity.y;

    // Apply movement force
    Vec3 velocityDifference = desiredVelocity - currentVelocity;
    velocityDifference.y = 0.0f; // Don't interfere with gravity/jumping

    if (Length(velocityDifference) > 0.01f) {
        Vec3 force = velocityDifference * 10.0f; // Adjust force multiplier as needed
        physics->ApplyForce(m_rigidBody, force);
    }
}

void Player::UpdateMovement(float deltaTime) {
    // Update rotation based on movement direction
    if (Length(m_movementInput) > 0.01f) {
        float targetRotation = atan2f(m_movementInput.x, -m_movementInput.z);

        // Smooth rotation interpolation
        float rotationDifference = targetRotation - m_rotation.y;

        // Handle angle wrapping
        while (rotationDifference > M_PI) rotationDifference -= 2.0f * M_PI;
        while (rotationDifference < -M_PI) rotationDifference += 2.0f * M_PI;

        // Interpolate rotation
        float rotationSpeed = 10.0f; // Adjust as needed
        m_rotation.y += rotationDifference * rotationSpeed * deltaTime;
    }
}

void Player::CheckGrounded() {
    if (!m_rigidBody) {
        return;
    }

    auto* physics = Engine::GetInstance().GetPhysicsEngine();
    if (!physics) {
        return;
    }

    // Perform a raycast downward to check if grounded
    Vec3 rayStart = m_position;
    Vec3 rayEnd = m_position + Vec3(0.0f, -1.1f, 0.0f); // Slightly below player

    RaycastResult result;
    if (physics->Raycast(rayStart, rayEnd, result)) {
        m_isGrounded = (result.distance < 1.05f); // Small tolerance
    } else {
        m_isGrounded = false;
    }
}

void Player::Shutdown() {
    LOG_INFO("Shutting down Player");

    // Clean up physics body
    if (m_rigidBody) {
        auto* physics = Engine::GetInstance().GetPhysicsEngine();
        if (physics) {
            physics->DestroyRigidBody(m_rigidBody);
        }
        m_rigidBody = nullptr;
    }

    // Clear resources
    m_mesh.reset();
    m_material.reset();

    LOG_INFO("Player shutdown complete");
}
```
