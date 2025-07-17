#include "Game/CharacterController.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    CharacterController::CharacterController() {
    }

    CharacterController::~CharacterController() {
        DestroyGhostObject();
    }

    bool CharacterController::Initialize(PhysicsEngine* physicsEngine) {
        m_physicsEngine = physicsEngine;
        
        if (!m_physicsEngine) {
            LOG_ERROR("CharacterController requires a physics engine");
            return false;
        }
        
        CreateGhostObject();
        
        LOG_INFO("CharacterController initialized with hybrid physics approach");
        return true;
    }

    void CharacterController::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        m_lastFrameTime = deltaTime;
        m_sweepTestCount = 0;
        
        HandleMovementInput(deltaTime, input, camera);
        UpdateMovement(deltaTime);
        UpdateGhostObjectPosition();
    }

    void CharacterController::HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        m_inputDirection = Math::Vec3(0.0f);
        m_jumpRequested = false;
        
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
    }

    void CharacterController::UpdateMovement(float deltaTime) {
        // HYBRID APPROACH: Funciona exatamente como Character original, mas com física para colisão
        
        // 1. Movimento horizontal (controle direto como Character original)
        Math::Vec3 horizontalMovement(0.0f);
        if (glm::length(m_inputDirection) > 0.0f) {
            horizontalMovement = m_inputDirection * m_moveSpeed * deltaTime;
        }
        
        // 2. Pulo (controle direto como Character original)
        if (m_jumpRequested && IsGrounded()) {
            m_velocity.y = m_jumpSpeed;
            LOG_DEBUG("CharacterController jumping");
        }
        
        // 3. Gravidade (exatamente como Character original)
        if (!IsGrounded()) {
            m_velocity.y += m_gravity * deltaTime;
        }
        
        // 4. Movimento vertical
        Math::Vec3 verticalMovement(0.0f, m_velocity.y * deltaTime, 0.0f);
        
        // 5. Aplicar movimento horizontal (com detecção de colisão por física)
        if (glm::length(horizontalMovement) > 0.001f) {
            // Tentar mover horizontalmente - se colidir, deslizar
            Math::Vec3 newHorizontalPos = m_position + horizontalMovement;
            
            if (m_physicsEngine) {
                // Usar física apenas para detectar colisão
                CollisionInfo collision = SweepTest(m_position, newHorizontalPos, m_radius, m_height);
                if (!collision.hasCollision) {
                    m_position += horizontalMovement; // Movimento livre
                } else {
                    // Deslizar ao longo da superfície
                    Math::Vec3 slideMovement = SlideAlongSurface(horizontalMovement, collision.normal);
                    m_position += slideMovement * 0.8f; // Reduzir um pouco para evitar penetração
                }
            } else {
                // Fallback - mover diretamente como Character original
                m_position += horizontalMovement;
            }
        }
        
        // 6. Aplicar movimento vertical (simples como Character original)
        m_position += verticalMovement;
        
        // 7. Verificar colisão com chão (simples)
        if (m_position.y < 0.9f) { // Mesmo nível do chão que Character original
            m_position.y = 0.9f;
            m_velocity.y = 0.0f;
        }
        
        // 8. Atualizar estado de chão (simples como Character original)
        if (m_position.y <= 0.9f) {
            m_movementState = MovementState::Grounded;
        } else {
            m_movementState = MovementState::Airborne;
        }
    }

    void CharacterController::ApplyGravity(float deltaTime) {
        if (!IsGrounded()) {
            m_velocity.y += m_gravity * deltaTime;
        }
    }

    Math::Vec3 CharacterController::ResolveMovement(const Math::Vec3& desiredMovement) {
        // Simplified collision resolution - just like original Character but with physics collision check
        CollisionInfo collision = SweepTest(m_position, m_position + desiredMovement, m_radius, m_height);
        m_sweepTestCount++;
        
        if (!collision.hasCollision) {
            // No collision - move freely
            return desiredMovement;
        }
        
        // Simple collision response - slide along surface
        return SlideAlongSurface(desiredMovement, collision.normal) * 0.9f;
    }

    CharacterController::CollisionInfo CharacterController::SweepTest(const Math::Vec3& from, const Math::Vec3& to, float radius, float height) {
        CollisionInfo result;
        
        if (!m_physicsEngine) {
            return result;
        }
        
        PhysicsEngine::SweepHit hit = m_physicsEngine->SweepCapsule(from, to, radius, height);
        
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

    bool CharacterController::IsGroundedCheck() {
        if (!m_physicsEngine) {
            // Fallback - simple ground check like original Character
            return m_position.y <= 0.9f; // Same as original Character ground level
        }
        
        // Simple raycast downward from character bottom
        Math::Vec3 rayStart = m_position; // Character center
        Math::Vec3 rayDirection(0.0f, -1.0f, 0.0f);
        float rayDistance = (m_height * 0.5f) + 0.1f; // From center to slightly below feet
        
        RaycastHit hit = m_physicsEngine->Raycast(rayStart, rayDirection, rayDistance);
        
        if (hit.hasHit) {
            // Simple check - if we hit something below us, we're grounded
            return hit.normal.y > 0.5f; // Surface is somewhat horizontal
        }
        
        // If no physics hit, use simple Y position check like original Character
        return m_position.y <= 0.9f;
    }

    CharacterController::StepInfo CharacterController::CheckStepUp(const Math::Vec3& moveDirection, float moveDistance) {
        StepInfo result;
        
        if (!m_physicsEngine) {
            return result;
        }
        
        // Cast a ray forward at step height to check for obstacles
        Math::Vec3 stepCheckStart = m_position + Math::Vec3(0.0f, m_maxStepHeight, 0.0f);
        Math::Vec3 stepCheckEnd = stepCheckStart + moveDirection * (m_radius + moveDistance);
        
        RaycastHit forwardHit = m_physicsEngine->Raycast(stepCheckStart, moveDirection, m_radius + moveDistance);
        
        if (!forwardHit.hasHit) {
            // No obstacle at step height, check if there's ground to step onto
            Math::Vec3 groundCheckStart = stepCheckEnd;
            Math::Vec3 groundCheckDirection(0.0f, -1.0f, 0.0f);
            
            RaycastHit groundHit = m_physicsEngine->Raycast(groundCheckStart, groundCheckDirection, m_maxStepHeight + m_skinWidth);
            
            if (groundHit.hasHit) {
                float stepHeight = m_maxStepHeight - groundHit.distance;
                if (stepHeight > 0.01f && stepHeight <= m_maxStepHeight) {
                    result.canStepUp = true;
                    result.stepHeight = stepHeight;
                    result.stepPosition = groundHit.point;
                }
            }
        }
        
        return result;
    }

    bool CharacterController::CheckSlope(const Math::Vec3& normal) {
        float slopeAngle = glm::degrees(acos(normal.y));
        return slopeAngle <= m_maxSlopeAngle;
    }

    Math::Vec3 CharacterController::SlideAlongSurface(const Math::Vec3& movement, const Math::Vec3& normal) {
        // Project movement onto the surface plane
        return movement - normal * glm::dot(movement, normal);
    }

    void CharacterController::CreateGhostObject() {
        if (!m_physicsEngine || m_ghostObjectId != 0) {
            return;
        }
        
        CollisionShape shape;
        shape.type = CollisionShape::Capsule;
        shape.dimensions = Math::Vec3(m_radius, m_height, m_radius);
        
        m_ghostObjectId = m_physicsEngine->CreateGhostObject(shape, m_position);
        
        if (m_ghostObjectId == 0) {
            LOG_ERROR("Failed to create ghost object for CharacterController");
        } else {
            LOG_DEBUG("Created ghost object for CharacterController with ID: " + std::to_string(m_ghostObjectId));
        }
    }

    void CharacterController::DestroyGhostObject() {
        if (m_physicsEngine && m_ghostObjectId != 0) {
            m_physicsEngine->DestroyGhostObject(m_ghostObjectId);
            m_ghostObjectId = 0;
        }
    }

    void CharacterController::UpdateGhostObjectPosition() {
        if (m_physicsEngine && m_ghostObjectId != 0) {
            Math::Quat rotation = Math::Quat(cos(glm::radians(m_yaw) * 0.5f), 0.0f, sin(glm::radians(m_yaw) * 0.5f), 0.0f);
            m_physicsEngine->SetGhostObjectTransform(m_ghostObjectId, m_position, rotation);
        }
    }

    void CharacterController::SetPosition(const Math::Vec3& position) {
        m_position = position;
        UpdateGhostObjectPosition();
    }

    void CharacterController::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Draw character as a capsule (different color from Character class)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(m_position, cubeSize, m_color);
        
        // Debug info - draw sweep test count
        if (m_sweepTestCount > 0) {
            LOG_DEBUG("CharacterController performed " + std::to_string(m_sweepTestCount) + 
                     " sweep tests in " + std::to_string(m_lastFrameTime * 1000.0f) + "ms");
        }
    }
}