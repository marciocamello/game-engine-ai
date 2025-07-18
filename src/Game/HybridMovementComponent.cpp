#include "Game/HybridMovementComponent.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    HybridMovementComponent::HybridMovementComponent() {
        m_movementMode = MovementMode::Walking;
    }

    HybridMovementComponent::~HybridMovementComponent() {
        DestroyGhostObject();
    }

    bool HybridMovementComponent::Initialize(PhysicsEngine* physicsEngine) {
        if (!physicsEngine) {
            LOG_ERROR("HybridMovementComponent requires a physics engine for collision detection");
            return false;
        }

        m_physicsEngine = physicsEngine;
        CreateGhostObject();

        LOG_INFO("HybridMovementComponent initialized with hybrid physics approach");
        return true;
    }

    void HybridMovementComponent::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        m_deltaTime = deltaTime;
        m_lastFrameTime = deltaTime;
        m_sweepTestCount = 0;
        
        // Clear accumulated input from previous frame
        m_accumulatedInput = Math::Vec3(0.0f);
        m_jumpRequested = false;
        
        // Handle input and accumulate movement
        HandleMovementInput(deltaTime, input, camera);
        
        // Update movement with collision detection
        UpdateMovement(deltaTime);
        
        // Update ghost object position for collision detection
        UpdateGhostObjectPosition();
    }

    void HybridMovementComponent::Shutdown() {
        DestroyGhostObject();
        m_physicsEngine = nullptr;
    }

    void HybridMovementComponent::SetPosition(const Math::Vec3& position) {
        m_position = position;
        UpdateGhostObjectPosition();
    }

    bool HybridMovementComponent::IsGrounded() const {
        return IsGroundedCheck();
    }

    void HybridMovementComponent::Jump() {
        if (!m_config.canJump || !IsGrounded()) {
            return;
        }

        m_velocity.y = m_config.jumpZVelocity;
        m_isJumping = true;
        m_movementMode = MovementMode::Falling;
        
        LOG_DEBUG("HybridMovementComponent jumping with velocity: " + std::to_string(m_config.jumpZVelocity));
    }

    void HybridMovementComponent::StopJumping() {
        m_jumpRequested = false;
    }

    void HybridMovementComponent::AddMovementInput(const Math::Vec3& worldDirection, float scaleValue) {
        Math::Vec3 constrainedInput = ConstrainInputVector(worldDirection * scaleValue);
        m_accumulatedInput += constrainedInput;
    }

    void HybridMovementComponent::HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
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

    void HybridMovementComponent::UpdateMovement(float deltaTime) {
        // 1. Handle jumping
        if (m_jumpRequested && IsGrounded()) {
            Jump();
        }
        
        // 2. Apply gravity
        ApplyGravity(deltaTime);
        
        // 3. Process horizontal movement with collision detection
        Math::Vec3 horizontalMovement(0.0f);
        if (glm::length(m_accumulatedInput) > 0.0f) {
            Math::Vec3 inputDirection = glm::normalize(m_accumulatedInput);
            horizontalMovement = inputDirection * m_config.maxWalkSpeed * deltaTime;
        }
        
        // 4. Apply horizontal movement with collision resolution
        if (glm::length(horizontalMovement) > 0.001f) {
            Math::Vec3 resolvedMovement = ResolveMovement(horizontalMovement);
            m_position += resolvedMovement;
        }
        
        // 5. Apply vertical movement
        Math::Vec3 verticalMovement(0.0f, m_velocity.y * deltaTime, 0.0f);
        m_position += verticalMovement;
        
        // 6. Physics-based ground collision detection
        CheckGroundCollision();
        
        // 7. Update movement mode based on grounded state
        bool wasGrounded = (m_movementMode == MovementMode::Walking);
        bool isGrounded = IsGrounded();
        
        if (isGrounded && !wasGrounded) {
            m_movementMode = MovementMode::Walking;
            m_isJumping = false;
            LOG_DEBUG("HybridMovementComponent: Landed");
        } else if (!isGrounded && wasGrounded) {
            m_movementMode = MovementMode::Falling;
            LOG_DEBUG("HybridMovementComponent: Became airborne");
        }
    }

    void HybridMovementComponent::ApplyGravity(float deltaTime) {
        if (!IsGrounded()) {
            m_velocity.y += m_gravity * m_config.gravityScale * deltaTime;
        }
    }

    HybridMovementComponent::CollisionInfo HybridMovementComponent::SweepTest(const Math::Vec3& from, const Math::Vec3& to, float radius, float height) {
        CollisionInfo result;
        
        if (!m_physicsEngine) {
            return result;
        }
        
        PhysicsEngine::SweepHit hit = m_physicsEngine->SweepCapsule(from, to, radius, height);
        m_sweepTestCount++;
        
        if (hit.hasHit) {
            result.hasCollision = true;
            result.contactPoint = hit.point;
            result.contactNormal = hit.normal;
            result.normal = hit.normal;  // Set the alias field
            result.penetrationDepth = 0.0f; // Sweep tests don't provide penetration depth
            result.hitBodyId = hit.bodyId;
            result.distance = hit.distance;
        }
        
        return result;
    }

    bool HybridMovementComponent::IsGroundedCheck() const {
        if (!m_physicsEngine) {
            // Fallback - simple ground check (but allow falling)
            return false; // Let character fall if no physics engine
        }
        
        // Simple raycast downward from character center
        Math::Vec3 rayStart = m_position;
        Math::Vec3 rayDirection(0.0f, -1.0f, 0.0f);
        float rayDistance = (m_characterHeight * 0.5f) + m_groundCheckDistance;
        
        RaycastHit hit = m_physicsEngine->Raycast(rayStart, rayDirection, rayDistance);
        
        if (hit.hasHit) {
            // Check if surface is walkable (not too steep)
            bool isWalkable = hit.normal.y > 0.5f; // Surface is somewhat horizontal
            
            // Also check if we're close enough to the ground
            float distanceToGround = hit.distance - (m_characterHeight * 0.5f);
            bool isCloseToGround = distanceToGround <= m_groundCheckDistance;
            
            return isWalkable && isCloseToGround;
        }
        
        // If no physics hit, character is not grounded (allow falling)
        return false;
    }

    HybridMovementComponent::StepInfo HybridMovementComponent::CheckStepUp(const Math::Vec3& moveDirection, float moveDistance) {
        StepInfo result;
        
        if (!m_physicsEngine) {
            return result;
        }
        
        // Cast a ray forward at step height to check for obstacles
        Math::Vec3 stepCheckStart = m_position + Math::Vec3(0.0f, m_config.maxStepHeight, 0.0f);
        Math::Vec3 stepCheckEnd = stepCheckStart + moveDirection * (m_characterRadius + moveDistance);
        
        RaycastHit forwardHit = m_physicsEngine->Raycast(stepCheckStart, moveDirection, m_characterRadius + moveDistance);
        
        if (!forwardHit.hasHit) {
            // No obstacle at step height, check if there's ground to step onto
            Math::Vec3 groundCheckStart = stepCheckEnd;
            Math::Vec3 groundCheckDirection(0.0f, -1.0f, 0.0f);
            
            RaycastHit groundHit = m_physicsEngine->Raycast(groundCheckStart, groundCheckDirection, m_config.maxStepHeight + m_skinWidth);
            
            if (groundHit.hasHit) {
                float stepHeight = m_config.maxStepHeight - groundHit.distance;
                if (stepHeight > 0.01f && stepHeight <= m_config.maxStepHeight) {
                    result.canStepUp = true;
                    result.stepHeight = stepHeight;
                    result.stepPosition = groundHit.point;
                }
            }
        }
        
        return result;
    }

    bool HybridMovementComponent::CheckSlope(const Math::Vec3& normal) {
        float slopeAngle = glm::degrees(acos(normal.y));
        return slopeAngle <= m_config.maxSlopeAngle;
    }

    Math::Vec3 HybridMovementComponent::ResolveMovement(const Math::Vec3& desiredMovement) {
        // Simple collision resolution - just like original CharacterController
        CollisionInfo collision = SweepTest(m_position, m_position + desiredMovement, m_characterRadius, m_characterHeight);
        
        if (!collision.hasCollision) {
            // No collision - move freely
            return desiredMovement;
        }
        
        // Simple collision response - slide along surface
        return SlideAlongSurface(desiredMovement, collision.normal) * 0.9f;
    }

    Math::Vec3 HybridMovementComponent::ResolveCollision(const Math::Vec3& desiredMovement, const CollisionInfo& collision) {
        // Slide along the collision surface
        return SlideAlongSurface(desiredMovement, collision.normal);
    }

    Math::Vec3 HybridMovementComponent::SlideAlongSurface(const Math::Vec3& movement, const Math::Vec3& normal) {
        // Project movement onto the surface plane
        return movement - normal * glm::dot(movement, normal);
    }

    void HybridMovementComponent::CreateGhostObject() {
        if (!m_physicsEngine || m_ghostObjectId != 0) {
            return;
        }
        
        CollisionShape shape;
        shape.type = CollisionShape::Capsule;
        shape.dimensions = Math::Vec3(m_characterRadius, m_characterHeight, m_characterRadius);
        
        m_ghostObjectId = m_physicsEngine->CreateGhostObject(shape, m_position);
        
        if (m_ghostObjectId == 0) {
            LOG_ERROR("Failed to create ghost object for HybridMovementComponent");
        } else {
            LOG_DEBUG("Created ghost object for HybridMovementComponent with ID: " + std::to_string(m_ghostObjectId));
        }
    }

    void HybridMovementComponent::DestroyGhostObject() {
        if (m_physicsEngine && m_ghostObjectId != 0) {
            m_physicsEngine->DestroyGhostObject(m_ghostObjectId);
            m_ghostObjectId = 0;
        }
    }

    void HybridMovementComponent::UpdateGhostObjectPosition() {
        if (m_physicsEngine && m_ghostObjectId != 0) {
            Math::Quat rotation = Math::Quat(cos(glm::radians(m_yaw) * 0.5f), 0.0f, sin(glm::radians(m_yaw) * 0.5f), 0.0f);
            m_physicsEngine->SetGhostObjectTransform(m_ghostObjectId, m_position, rotation);
        }
    }

    void HybridMovementComponent::CheckGroundCollision() {
        // Hybrid approach: Use physics raycast for detection but natural falling like deterministic
        // This prevents teleporting/snapping and maintains smooth movement
        
        if (m_physicsEngine) {
            // Cast a ray downward from character center
            Math::Vec3 rayOrigin = m_position;
            Math::Vec3 rayDirection = Math::Vec3(0.0f, -1.0f, 0.0f);
            float maxDistance = m_characterHeight + 0.5f;
            
            RaycastHit hit = m_physicsEngine->Raycast(rayOrigin, rayDirection, maxDistance);
            
            if (hit.hasHit) {
                // Calculate ground level
                float groundLevel = hit.point.y + (m_characterHeight * 0.5f);
                
                // Natural ground collision - only stop falling when actually touching ground
                if (m_position.y <= groundLevel && m_velocity.y <= 0.0f) {
                    // Character naturally reached ground level while falling
                    m_position.y = groundLevel; // Correct position to exact ground level
                    m_velocity.y = 0.0f; // Stop vertical movement
                    
                    if (!m_isGrounded) {
                        // Just landed naturally
                        m_isGrounded = true;
                        m_isJumping = false;
                        m_movementMode = MovementMode::Walking;
                        LOG_INFO("HybridMovementComponent: Landed naturally on ground at Y=" + std::to_string(groundLevel));
                    }
                } else if (m_position.y > groundLevel + 0.1f) {
                    // Character is clearly airborne
                    if (m_isGrounded) {
                        // Just left ground
                        m_isGrounded = false;
                        m_movementMode = MovementMode::Falling;
                        LOG_INFO("HybridMovementComponent: Became airborne at Y=" + std::to_string(m_position.y));
                    }
                }
            } else {
                // No ground detected below - character is falling (like when outside grid bounds)
                if (m_isGrounded) {
                    // Just left ground area
                    m_isGrounded = false;
                    m_movementMode = MovementMode::Falling;
                    LOG_INFO("HybridMovementComponent: No ground detected - falling");
                }
            }
        } else {
            // Fallback: Use same logic as DeterministicMovementComponent
            float groundLevel = 0.0f;
            float characterCenterHeight = groundLevel + (m_characterHeight * 0.5f);
            
            // Check if character is within ground plane bounds
            bool withinGroundBounds = (m_position.x >= -50.0f && m_position.x <= 50.0f && 
                                      m_position.z >= -50.0f && m_position.z <= 50.0f);
            
            if (withinGroundBounds && m_position.y <= characterCenterHeight && m_velocity.y <= 0.0f) {
                m_position.y = characterCenterHeight;
                m_velocity.y = 0.0f;
                
                if (!m_isGrounded) {
                    m_isGrounded = true;
                    m_isJumping = false;
                    m_movementMode = MovementMode::Walking;
                    LOG_INFO("HybridMovementComponent: Landed on ground (fallback)");
                }
            } else if (!withinGroundBounds || m_position.y > characterCenterHeight + 0.1f) {
                if (m_isGrounded) {
                    m_isGrounded = false;
                    m_movementMode = MovementMode::Falling;
                    LOG_INFO("HybridMovementComponent: Became airborne (fallback)");
                }
            }
        }
    }
}