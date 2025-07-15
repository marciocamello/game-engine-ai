#include "Game/Character.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Core/Logger.h"

namespace GameEngine {
    Character::Character() {
    }

    Character::~Character() {
    }

    bool Character::Initialize() {
        LOG_INFO("Character initialized");
        return true;
    }

    void Character::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        HandleMovementInput(deltaTime, input, camera);
        UpdatePhysics(deltaTime);
    }

    void Character::HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        Math::Vec3 inputDirection(0.0f);
        
        // Get input values
        float forwardInput = 0.0f;
        float rightInput = 0.0f;
        
        if (input->IsKeyDown(KeyCode::W)) forwardInput += 1.0f;
        if (input->IsKeyDown(KeyCode::S)) forwardInput -= 1.0f;
        if (input->IsKeyDown(KeyCode::A)) rightInput -= 1.0f;
        if (input->IsKeyDown(KeyCode::D)) rightInput += 1.0f;
        
        if (camera && (forwardInput != 0.0f || rightInput != 0.0f)) {
            // Use camera system to get movement direction
            inputDirection = camera->GetMovementDirection(forwardInput, rightInput);
            
            // Update character rotation to face movement direction
            if (glm::length(inputDirection) > 0.0f) {
                m_yaw = atan2(inputDirection.x, inputDirection.z) * 180.0f / glm::pi<float>();
            }
        } else if (forwardInput != 0.0f || rightInput != 0.0f) {
            // Fallback to world-space movement
            inputDirection.x = rightInput;
            inputDirection.z = -forwardInput;  // Z is forward in our coordinate system
            
            if (glm::length(inputDirection) > 0.0f) {
                inputDirection = glm::normalize(inputDirection);
                m_yaw = atan2(inputDirection.x, inputDirection.z) * 180.0f / glm::pi<float>();
            }
        }

        // Apply movement
        if (glm::length(inputDirection) > 0.0f) {
            Math::Vec3 movement = inputDirection * m_moveSpeed * deltaTime;
            m_position += movement;
        }

        // Handle jumping
        if (input->IsKeyPressed(KeyCode::Space) && m_isGrounded) {
            m_velocity.y = m_jumpSpeed;
            m_isGrounded = false;
            m_isJumping = true;
            LOG_INFO("Character jumping!");
        }
    }

    void Character::UpdatePhysics(float deltaTime) {
        // Apply gravity
        if (!m_isGrounded) {
            m_velocity.y += m_gravity * deltaTime;
        }

        // Apply velocity
        m_position += m_velocity * deltaTime;

        // Simple ground collision (y = 0) - character should sit on ground
        float groundLevel = m_height * 0.5f;  // Half height to sit properly on ground
        if (m_position.y <= groundLevel) {
            m_position.y = groundLevel;
            m_velocity.y = 0.0f;
            m_isGrounded = true;
            m_isJumping = false;
        }

        // Keep character in bounds (simple world boundaries)
        float worldSize = 50.0f;
        m_position.x = Math::Clamp(m_position.x, -worldSize, worldSize);
        m_position.z = Math::Clamp(m_position.z, -worldSize, worldSize);
    }

    void Character::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Draw character as a simple cube (easier to see movement)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(m_position, cubeSize, m_color);
    }
}