#include "Game/DeterministicMovementComponent.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    DeterministicMovementComponent::DeterministicMovementComponent() {
        m_movementMode = MovementMode::Walking;
    }

    DeterministicMovementComponent::~DeterministicMovementComponent() {
    }

    bool DeterministicMovementComponent::Initialize(PhysicsEngine* physicsEngine) {
        // DeterministicMovementComponent doesn't require physics engine but can use it for collision queries
        m_physicsEngine = physicsEngine;
        
        LOG_INFO("DeterministicMovementComponent initialized with precise character control");
        return true;
    }

    void DeterministicMovementComponent::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        m_deltaTime = deltaTime;
        
        // Clear accumulated input from previous frame
        m_accumulatedInput = Math::Vec3(0.0f);
        m_jumpRequested = false;
        
        // Handle input and accumulate movement
        HandleMovementInput(deltaTime, input, camera);
        
        // Process accumulated input and update movement
        ProcessMovementInput();
        
        // Update position and physics
        UpdateMovement(deltaTime);
    }

    void DeterministicMovementComponent::Shutdown() {
        // No cleanup needed for deterministic movement
        m_physicsEngine = nullptr;
    }

    void DeterministicMovementComponent::Jump() {
        LOG_INFO("DeterministicMovementComponent::Jump() - canJump: " + std::to_string(m_config.canJump) + ", isGrounded: " + std::to_string(m_isGrounded) + ", Y: " + std::to_string(m_position.y));
        
        if (!m_config.canJump) {
            LOG_INFO("DeterministicMovementComponent: Jump disabled in config");
            return;
        }
        
        if (!m_isGrounded) {
            LOG_INFO("DeterministicMovementComponent: Cannot jump - not grounded (Y=" + std::to_string(m_position.y) + ")");
            return;
        }

        m_velocity.y = m_config.jumpZVelocity;
        m_isGrounded = false;
        m_isJumping = true;
        m_movementMode = MovementMode::Falling;
        
        LOG_INFO("DeterministicMovementComponent jumping with velocity: " + std::to_string(m_config.jumpZVelocity));
    }

    void DeterministicMovementComponent::StopJumping() {
        m_jumpRequested = false;
    }

    void DeterministicMovementComponent::AddMovementInput(const Math::Vec3& worldDirection, float scaleValue) {
        Math::Vec3 constrainedInput = ConstrainInputVector(worldDirection * scaleValue);
        m_accumulatedInput += constrainedInput;
    }

    void DeterministicMovementComponent::HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        m_inputDirection = Math::Vec3(0.0f);
        
        // Get input values
        float forwardInput = 0.0f;
        float rightInput = 0.0f;
        
        if (input->IsKeyDown(KeyCode::W)) forwardInput += 1.0f;
        if (input->IsKeyDown(KeyCode::S)) forwardInput -= 1.0f;
        if (input->IsKeyDown(KeyCode::A)) rightInput -= 1.0f;
        if (input->IsKeyDown(KeyCode::D)) rightInput += 1.0f;
        
        if (camera && (forwardInput != 0.0f || rightInput != 0.0f)) {
            // Use camera system to get movement direction
            m_inputDirection = camera->GetMovementDirection(forwardInput, rightInput);
            
            // Update character rotation to face movement direction
            if (glm::length(m_inputDirection) > 0.0f) {
                m_yaw = atan2(m_inputDirection.x, m_inputDirection.z) * 180.0f / glm::pi<float>();
            }
        } else if (forwardInput != 0.0f || rightInput != 0.0f) {
            // Fallback to world-space movement
            m_inputDirection.x = rightInput;
            m_inputDirection.z = -forwardInput;  // Z is forward in our coordinate system
            
            if (glm::length(m_inputDirection) > 0.0f) {
                m_inputDirection = glm::normalize(m_inputDirection);
                m_yaw = atan2(m_inputDirection.x, m_inputDirection.z) * 180.0f / glm::pi<float>();
            }
        }

        // Handle jumping
        if (input->IsKeyPressed(KeyCode::Space)) {
            m_jumpRequested = true;
        }

        // Add movement input to accumulator
        if (glm::length(m_inputDirection) > 0.0f) {
            AddMovementInput(m_inputDirection, 1.0f);
        }
    }

    void DeterministicMovementComponent::UpdateMovement(float deltaTime) {
        // Apply gravity
        ApplyGravity(deltaTime);
        
        // Apply velocity to position
        m_position += m_velocity * deltaTime;
        
        // Check ground collision
        CheckGroundCollision();
        
        // Apply friction/damping
        if (m_isGrounded) {
            // Ground friction with smooth stopping
            Math::Vec3 horizontalVelocity(m_velocity.x, 0.0f, m_velocity.z);
            float currentSpeed = glm::length(horizontalVelocity);
            
            if (currentSpeed > m_minSpeedThreshold) {
                // Use different friction based on whether we have input
                float effectiveFriction = (glm::length(m_accumulatedInput) > 0.0f) ? m_friction : m_brakingFriction;
                float frictionForce = effectiveFriction * deltaTime;
                
                if (currentSpeed > frictionForce) {
                    Math::Vec3 frictionVector = -glm::normalize(horizontalVelocity) * frictionForce;
                    m_velocity.x += frictionVector.x;
                    m_velocity.z += frictionVector.z;
                } else {
                    // Gradual stop instead of instant
                    float stopFactor = 1.0f - (frictionForce / currentSpeed);
                    m_velocity.x *= stopFactor;
                    m_velocity.z *= stopFactor;
                }
            } else {
                // Stop completely when speed is very low
                m_velocity.x = 0.0f;
                m_velocity.z = 0.0f;
            }
        } else {
            // Air resistance
            Math::Vec3 horizontalVelocity(m_velocity.x, 0.0f, m_velocity.z);
            float currentSpeed = glm::length(horizontalVelocity);
            
            if (currentSpeed > 0.0f) {
                float airResistance = m_airFriction * deltaTime;
                if (currentSpeed > airResistance) {
                    Math::Vec3 resistanceForce = -glm::normalize(horizontalVelocity) * airResistance;
                    m_velocity.x += resistanceForce.x;
                    m_velocity.z += resistanceForce.z;
                } else {
                    float stopFactor = 1.0f - (airResistance / currentSpeed);
                    m_velocity.x *= stopFactor;
                    m_velocity.z *= stopFactor;
                }
            }
        }
    }

    void DeterministicMovementComponent::ApplyGravity(float deltaTime) {
        if (!m_isGrounded) {
            m_velocity.y += m_gravity * m_config.gravityScale * deltaTime;
        }
    }

    void DeterministicMovementComponent::CheckGroundCollision() {
        // Simple ground collision check for basic movement (no physics)
        // Ground plane is 100x100 units centered at origin (X: -50 to 50, Z: -50 to 50)
        float groundLevel = 0.0f;
        float characterCenterHeight = groundLevel + (m_characterHeight * 0.5f); // 0.9f for 1.8f height
        
        // Check if character is within ground plane bounds
        bool withinGroundBounds = (m_position.x >= -50.0f && m_position.x <= 50.0f && 
                                  m_position.z >= -50.0f && m_position.z <= 50.0f);
        
        if (withinGroundBounds && m_position.y <= characterCenterHeight && m_velocity.y <= 0.0f) {
            // Character hit or is on ground and falling/stationary, and within ground bounds
            m_position.y = characterCenterHeight; // Position character on ground
            m_velocity.y = 0.0f; // Stop vertical movement
            
            if (!m_isGrounded) {
                // Just landed
                m_isGrounded = true;
                m_isJumping = false;
                m_movementMode = MovementMode::Walking;
                LOG_INFO("DeterministicMovementComponent: Landed on ground at Y=" + std::to_string(m_position.y));
            }
        } else if (!withinGroundBounds || m_position.y > characterCenterHeight + 0.1f) {
            // Character is outside ground bounds OR clearly airborne
            if (m_isGrounded) {
                // Just left ground or ground area
                m_isGrounded = false;
                m_movementMode = MovementMode::Falling;
                LOG_INFO("DeterministicMovementComponent: Became airborne at Y=" + std::to_string(m_position.y) + 
                        " (withinBounds: " + std::to_string(withinGroundBounds) + ")");
            }
        }
        // If between characterCenterHeight and characterCenterHeight + 0.1f AND within bounds, maintain current state (hysteresis)
    }

    void DeterministicMovementComponent::ProcessMovementInput() {
        // Handle jumping first (independent of movement input)
        if (m_jumpRequested) {
            LOG_INFO("DeterministicMovementComponent: Jump requested, calling Jump()");
            Jump();
        }
        
        // Process movement input if available
        if (glm::length(m_accumulatedInput) >= 0.001f) {
            // Normalize accumulated input
            Math::Vec3 inputDirection = glm::normalize(m_accumulatedInput);
            
            // Calculate acceleration based on movement mode
            float acceleration = m_isGrounded ? m_acceleration : m_airAcceleration;
            
            // Scale acceleration based on movement configuration
            if (!m_isGrounded) {
                acceleration *= m_config.airControl;
            }
            
            // Apply acceleration to velocity
            Math::Vec3 accelerationVector = inputDirection * acceleration * m_deltaTime;
            
            // Limit maximum speed
            Math::Vec3 newHorizontalVelocity = Math::Vec3(m_velocity.x, 0.0f, m_velocity.z) + Math::Vec3(accelerationVector.x, 0.0f, accelerationVector.z);
            float currentSpeed = glm::length(newHorizontalVelocity);
            
            if (currentSpeed > m_config.maxWalkSpeed) {
                newHorizontalVelocity = glm::normalize(newHorizontalVelocity) * m_config.maxWalkSpeed;
            }
            
            m_velocity.x = newHorizontalVelocity.x;
            m_velocity.z = newHorizontalVelocity.z;
        }
    }
}