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
            bodyDesc.friction = 0.7f; // Good friction for walking
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
            // Apply force for movement instead of directly modifying position
            // Use stronger force scaling for better responsiveness
            Math::Vec3 movementForce = inputDirection * m_moveSpeed * 500.0f; // Increased force scaling
            m_physicsEngine->ApplyForce(m_rigidBodyId, movementForce);
        } else if (glm::length(inputDirection) > 0.0f) {
            // Fallback to direct position modification if no physics
            Math::Vec3 movement = inputDirection * m_moveSpeed * deltaTime;
            m_position += movement;
        }

        // Handle jumping
        if (input->IsKeyPressed(KeyCode::Space) && m_isGrounded) {
            if (m_physicsEngine && m_rigidBodyId != 0) {
                // Apply upward impulse for jumping
                // Use mass-based impulse calculation: impulse = mass * desired_velocity
                Math::Vec3 jumpImpulse(0.0f, 70.0f * m_jumpSpeed, 0.0f); // 70kg mass * jump speed
                m_physicsEngine->ApplyImpulse(m_rigidBodyId, jumpImpulse);
            } else {
                // Fallback to direct velocity modification
                m_velocity.y = m_jumpSpeed;
            }
            m_isGrounded = false;
            m_isJumping = true;
            LOG_INFO("Character jumping!");
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
            
            // Check if character is grounded using physics engine
            m_isGrounded = m_physicsEngine->IsRigidBodyGrounded(m_rigidBodyId, 0.1f);
            
            // Update jumping state based on grounded status
            if (m_isGrounded && m_isJumping) {
                m_isJumping = false;
            }
        } else {
            // Fallback to manual physics calculations when no physics engine is available
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

            // Note: Removed hardcoded world boundaries - physics collision should handle boundaries instead
        }
    }

    void Character::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Draw character as a simple cube (easier to see movement)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(m_position, cubeSize, m_color);
    }
}