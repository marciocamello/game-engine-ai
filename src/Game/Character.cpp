#include "Game/Character.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    Character::Character() {
    }

    Character::~Character() {
        // Clean up physics rigid body if it exists
        if (m_physicsEngine && m_rigidBodyId != 0) {
            m_physicsEngine->DestroyRigidBody(m_rigidBodyId);
            m_rigidBodyId = 0;
        }
    }

    bool Character::Initialize(PhysicsEngine* physicsEngine) {
        m_physicsEngine = physicsEngine;
        
        if (m_physicsEngine) {
            // Create character rigid body using capsule shape
            RigidBody bodyDesc;
            bodyDesc.position = m_position;
            bodyDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
            bodyDesc.velocity = m_velocity;
            bodyDesc.mass = 70.0f; // Average human mass in kg
            bodyDesc.restitution = 0.0f; // No bouncing for character
            bodyDesc.friction = 1.0f; // High friction to prevent sliding like a car
            bodyDesc.isStatic = false;
            bodyDesc.isKinematic = false;
            
            CollisionShape shape;
            shape.type = CollisionShape::Capsule;
            shape.dimensions = Math::Vec3(m_radius, m_height, m_radius); // radius, height, radius
            
            m_rigidBodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
            
            if (m_rigidBodyId == 0) {
                LOG_ERROR("Failed to create character rigid body");
                return false;
            }
            
            // Set angular constraints to keep character upright (prevent rotation around X and Z axes)
            Math::Vec3 angularFactor(0.0f, 1.0f, 0.0f); // Only allow Y-axis rotation (yaw)
            m_physicsEngine->SetAngularFactor(m_rigidBodyId, angularFactor);
            
            // Set damping to reduce sliding and make movement more character-like
            m_physicsEngine->SetLinearDamping(m_rigidBodyId, 0.8f); // High linear damping to stop sliding
            m_physicsEngine->SetAngularDamping(m_rigidBodyId, 0.9f); // High angular damping for stability
            
            LOG_INFO("Character initialized with physics (rigid body ID: " + std::to_string(m_rigidBodyId) + ")");
        } else {
            LOG_INFO("Character initialized without physics");
        }
        
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

        // Apply movement through physics engine if available
        if (m_physicsEngine && m_rigidBodyId != 0 && glm::length(inputDirection) > 0.0f) {
            // Apply force for movement - much more controlled for character-like movement
            Math::Vec3 movementForce = inputDirection * m_moveSpeed * 300.0f; // Increased for better responsiveness
            m_physicsEngine->ApplyForce(m_rigidBodyId, movementForce);
        } else if (glm::length(inputDirection) > 0.0f) {
            // Fallback to direct position modification if no physics
            Math::Vec3 movement = inputDirection * m_moveSpeed * deltaTime;
            m_position += movement;
        }

        // Handle jumping
        if (input->IsKeyPressed(KeyCode::Space)) {
            LOG_DEBUG("Space key pressed! Grounded state: " + std::string(m_isGrounded ? "true" : "false"));
            
            if (m_isGrounded) {
                if (m_physicsEngine && m_rigidBodyId != 0) {
                    // Apply upward impulse for jumping
                    // Use mass-based impulse calculation: impulse = mass * desired_velocity
                    Math::Vec3 jumpImpulse(0.0f, 70.0f * m_jumpSpeed, 0.0f); // 70kg mass * jump speed
                    m_physicsEngine->ApplyImpulse(m_rigidBodyId, jumpImpulse);
                    LOG_INFO("Character jumping with impulse: " + std::to_string(jumpImpulse.y));
                } else {
                    // Fallback to direct velocity modification
                    m_velocity.y = m_jumpSpeed;
                    LOG_INFO("Character jumping (fallback mode)");
                }
                m_isGrounded = false;
                m_isJumping = true;
            } else {
                LOG_DEBUG("Cannot jump - character not grounded");
            }
        }
    }

    void Character::UpdatePhysics(float deltaTime) {
        if (m_physicsEngine && m_rigidBodyId != 0) {
            // Query rigid body state from physics engine instead of manual calculations
            Math::Vec3 physicsPosition;
            Math::Quat physicsRotation;
            Math::Vec3 physicsVelocity;
            Math::Vec3 physicsAngularVelocity;
            
            // Get current transform from physics engine
            if (m_physicsEngine->GetRigidBodyTransform(m_rigidBodyId, physicsPosition, physicsRotation)) {
                m_position = physicsPosition;
            }
            
            // Get current velocity from physics engine
            if (m_physicsEngine->GetRigidBodyVelocity(m_rigidBodyId, physicsVelocity, physicsAngularVelocity)) {
                m_velocity = physicsVelocity;
            }
            
            // Check if character is grounded using physics-based collision detection
            // This uses raycasting to detect ground contact instead of hardcoded y-position checks
            bool wasGrounded = m_isGrounded;
            m_isGrounded = m_physicsEngine->IsRigidBodyGrounded(m_rigidBodyId, 0.1f);
            
            // Update jumping state based on physics-detected grounded status
            if (m_isGrounded && m_isJumping) {
                m_isJumping = false;
                LOG_DEBUG("Character landed (physics-detected ground contact)");
            }
            
            // Log ground state changes for debugging
            if (wasGrounded != m_isGrounded) {
                if (m_isGrounded) {
                    LOG_DEBUG("Character became grounded (physics collision detection)");
                } else {
                    LOG_DEBUG("Character became airborne (physics collision detection)");
                }
            }
        } else {
            // Without physics engine, character cannot function properly
            // All collision detection now requires physics engine
            LOG_WARNING("Character physics update called without physics engine - character will not behave correctly");
            
            // Set character as not grounded since we can't detect ground without physics
            // This prevents jumping when no physics engine is available
            m_isGrounded = false;
            m_isJumping = true; // Always consider jumping state when no physics
            
            // Apply basic gravity but no collision detection
            m_velocity.y += m_gravity * deltaTime;
            m_position += m_velocity * deltaTime;
            
            // Note: All manual ground collision code removed
            // Physics engine is now required for proper character behavior
        }
    }

    void Character::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Draw character as a simple cube (easier to see movement)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(m_position, cubeSize, m_color);
    }
}