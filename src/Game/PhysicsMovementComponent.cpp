#include "Game/PhysicsMovementComponent.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    PhysicsMovementComponent::PhysicsMovementComponent() {
        m_movementMode = MovementMode::Walking;
    }

    PhysicsMovementComponent::~PhysicsMovementComponent() {
        DestroyRigidBody();
    }

    bool PhysicsMovementComponent::Initialize(PhysicsEngine* physicsEngine) {
        if (!physicsEngine) {
            LOG_ERROR("PhysicsMovementComponent requires a physics engine");
            return false;
        }

        m_physicsEngine = physicsEngine;
        CreateRigidBody();

        LOG_INFO("PhysicsMovementComponent initialized with full physics simulation");
        return true;
    }

    void PhysicsMovementComponent::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        m_deltaTime = deltaTime;
        
        // Clear accumulated forces/impulses from previous frame
        m_accumulatedForces = Math::Vec3(0.0f);
        m_accumulatedImpulses = Math::Vec3(0.0f);
        
        // Handle input and accumulate movement forces
        HandleMovementInput(deltaTime, input, camera);
        
        // Apply accumulated forces and impulses to physics body
        ApplyMovementForces();
        
        // Update state from physics simulation
        UpdatePhysicsState();
    }

    void PhysicsMovementComponent::Shutdown() {
        DestroyRigidBody();
        m_physicsEngine = nullptr;
    }

    void PhysicsMovementComponent::SetPosition(const Math::Vec3& position) {
        m_position = position;
        if (m_physicsEngine && m_rigidBodyId != 0) {
            Math::Quat rotation = Math::Quat(cos(glm::radians(m_yaw) * 0.5f), 0.0f, sin(glm::radians(m_yaw) * 0.5f), 0.0f);
            m_physicsEngine->SetRigidBodyTransform(m_rigidBodyId, m_position, rotation);
        }
    }

    void PhysicsMovementComponent::SetRotation(float yaw) {
        m_yaw = yaw;
        if (m_physicsEngine && m_rigidBodyId != 0) {
            Math::Quat rotation = Math::Quat(cos(glm::radians(m_yaw) * 0.5f), 0.0f, sin(glm::radians(m_yaw) * 0.5f), 0.0f);
            m_physicsEngine->SetRigidBodyTransform(m_rigidBodyId, m_position, rotation);
        }
    }

    void PhysicsMovementComponent::SetVelocity(const Math::Vec3& velocity) {
        m_velocity = velocity;
        // Note: Cannot directly set velocity in Bullet Physics
        // This would require applying impulses to achieve desired velocity
    }

    void PhysicsMovementComponent::AddVelocity(const Math::Vec3& deltaVelocity) {
        // Convert velocity change to impulse (impulse = mass * deltaVelocity)
        Math::Vec3 impulse = deltaVelocity * m_mass;
        m_accumulatedImpulses += impulse;
    }

    bool PhysicsMovementComponent::IsGrounded() const {
        if (!m_physicsEngine || m_rigidBodyId == 0) {
            return false;
        }
        
        return m_physicsEngine->IsRigidBodyGrounded(m_rigidBodyId, 0.1f);
    }

    void PhysicsMovementComponent::Jump() {
        if (!m_config.canJump || !IsGrounded()) {
            return;
        }

        // Apply upward impulse for jumping
        Math::Vec3 jumpImpulse(0.0f, m_mass * m_config.jumpZVelocity, 0.0f);
        m_accumulatedImpulses += jumpImpulse;
        m_isJumping = true;
        
        LOG_DEBUG("PhysicsMovementComponent jumping with impulse: " + std::to_string(jumpImpulse.y));
    }

    void PhysicsMovementComponent::StopJumping() {
        // Stop jumping logic - currently not needed for physics-based jumping
    }

    void PhysicsMovementComponent::AddMovementInput(const Math::Vec3& worldDirection, float scaleValue) {
        Math::Vec3 constrainedInput = ConstrainInputVector(worldDirection * scaleValue);
        m_pendingInputVector += constrainedInput;
    }

    void PhysicsMovementComponent::SetMass(float mass) {
        m_mass = mass;
        // Note: Would need to recreate rigid body to change mass in Bullet Physics
    }

    void PhysicsMovementComponent::SetFriction(float friction) {
        m_friction = friction;
        // Apply to existing rigid body if available
    }

    void PhysicsMovementComponent::SetRestitution(float restitution) {
        m_restitution = restitution;
        // Apply to existing rigid body if available
    }

    void PhysicsMovementComponent::SetLinearDamping(float damping) {
        m_linearDamping = damping;
        if (m_physicsEngine && m_rigidBodyId != 0) {
            m_physicsEngine->SetLinearDamping(m_rigidBodyId, damping);
        }
    }

    void PhysicsMovementComponent::SetAngularDamping(float damping) {
        m_angularDamping = damping;
        if (m_physicsEngine && m_rigidBodyId != 0) {
            m_physicsEngine->SetAngularDamping(m_rigidBodyId, damping);
        }
    }

    void PhysicsMovementComponent::HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
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
            Jump();
        }

        // Convert input to movement forces
        if (glm::length(m_inputDirection) > 0.0f) {
            // Calculate desired acceleration based on movement mode
            float maxAccel = IsGrounded() ? m_config.maxAcceleration : m_config.maxAcceleration * m_config.airControl;
            
            // Apply force for movement
            Math::Vec3 movementForce = m_inputDirection * maxAccel * m_mass;
            m_accumulatedForces += movementForce;
        } else if (IsGrounded()) {
            // Apply braking force when no input (for elegant stopping)
            Math::Vec3 horizontalVelocity(m_velocity.x, 0.0f, m_velocity.z);
            float currentSpeed = glm::length(horizontalVelocity);
            
            if (currentSpeed > 0.1f) { // Only brake if moving fast enough
                // Apply counter-force proportional to current velocity
                Math::Vec3 brakingForce = -glm::normalize(horizontalVelocity) * m_config.brakingDeceleration * m_mass;
                m_accumulatedForces += brakingForce;
            }
        }
    }

    void PhysicsMovementComponent::UpdatePhysicsState() {
        if (!m_physicsEngine || m_rigidBodyId == 0) {
            return;
        }

        // Get current transform from physics engine
        Math::Vec3 physicsPosition;
        Math::Quat physicsRotation;
        if (m_physicsEngine->GetRigidBodyTransform(m_rigidBodyId, physicsPosition, physicsRotation)) {
            m_position = physicsPosition;
        }

        // Get current velocity from physics engine
        Math::Vec3 physicsVelocity;
        Math::Vec3 physicsAngularVelocity;
        if (m_physicsEngine->GetRigidBodyVelocity(m_rigidBodyId, physicsVelocity, physicsAngularVelocity)) {
            m_velocity = physicsVelocity;
        }

        // Update movement mode based on physics state
        bool wasGrounded = m_movementMode == MovementMode::Walking;
        bool isGrounded = IsGrounded();
        
        if (isGrounded && !wasGrounded) {
            m_movementMode = MovementMode::Walking;
            m_isJumping = false;
            LOG_DEBUG("PhysicsMovementComponent: Landed (physics-detected ground contact)");
        } else if (!isGrounded && wasGrounded) {
            m_movementMode = MovementMode::Falling;
            LOG_DEBUG("PhysicsMovementComponent: Became airborne (physics collision detection)");
        }
    }

    void PhysicsMovementComponent::ApplyMovementForces() {
        if (!m_physicsEngine || m_rigidBodyId == 0) {
            return;
        }

        // Apply accumulated forces
        if (glm::length(m_accumulatedForces) > 0.001f) {
            m_physicsEngine->ApplyForce(m_rigidBodyId, m_accumulatedForces);
        }

        // Apply accumulated impulses
        if (glm::length(m_accumulatedImpulses) > 0.001f) {
            m_physicsEngine->ApplyImpulse(m_rigidBodyId, m_accumulatedImpulses);
        }
    }

    void PhysicsMovementComponent::CreateRigidBody() {
        if (!m_physicsEngine || m_rigidBodyId != 0) {
            return;
        }

        // Create character rigid body using capsule shape
        RigidBody bodyDesc;
        bodyDesc.position = m_position;
        bodyDesc.rotation = Math::Quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        bodyDesc.velocity = m_velocity;
        bodyDesc.mass = m_mass;
        bodyDesc.restitution = m_restitution;
        bodyDesc.friction = m_friction;
        bodyDesc.isStatic = false;
        bodyDesc.isKinematic = false;
        
        CollisionShape shape;
        shape.type = CollisionShape::Capsule;
        shape.dimensions = Math::Vec3(m_characterRadius, m_characterHeight, m_characterRadius);
        
        m_rigidBodyId = m_physicsEngine->CreateRigidBody(bodyDesc, shape);
        
        if (m_rigidBodyId == 0) {
            LOG_ERROR("Failed to create rigid body for PhysicsMovementComponent");
            return;
        }
        
        // Set angular constraints to keep character upright
        Math::Vec3 angularFactor(0.0f, 1.0f, 0.0f); // Only allow Y-axis rotation (yaw)
        m_physicsEngine->SetAngularFactor(m_rigidBodyId, angularFactor);
        
        // Set damping for character-like movement
        m_physicsEngine->SetLinearDamping(m_rigidBodyId, m_linearDamping);
        m_physicsEngine->SetAngularDamping(m_rigidBodyId, m_angularDamping);
        
        LOG_INFO("PhysicsMovementComponent created rigid body with ID: " + std::to_string(m_rigidBodyId));
    }

    void PhysicsMovementComponent::DestroyRigidBody() {
        if (m_physicsEngine && m_rigidBodyId != 0) {
            m_physicsEngine->DestroyRigidBody(m_rigidBodyId);
            m_rigidBodyId = 0;
        }
    }
}