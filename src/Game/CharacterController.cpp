#include "Game/CharacterController.h"
#include "Game/MovementComponentFactory.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    CharacterController::CharacterController() 
        : m_color(1.0f, 0.3f, 0.3f, 1.0f) // Vermelho para CharacterController
    {
    }

    CharacterController::~CharacterController() {
        // Movement component cleanup is handled automatically by unique_ptr
    }

    bool CharacterController::Initialize(PhysicsEngine* physicsEngine) {
        m_physicsEngine = physicsEngine;
        
        // Initialize default movement component (HybridMovementComponent)
        InitializeDefaultMovementComponent(physicsEngine);
        
        if (!m_movementComponent) {
            LOG_ERROR("Failed to initialize movement component for CharacterController");
            return false;
        }
        
        LOG_INFO("CharacterController initialized with component-based movement system (" + 
                std::string(GetMovementTypeName()) + ")");
        return true;
    }

    void CharacterController::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        if (m_movementComponent) {
            m_movementComponent->Update(deltaTime, input, camera);
        }
    }

    void CharacterController::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Get color based on current movement component type
        Math::Vec4 currentColor = GetMovementTypeColor();
        
        // Draw character as a capsule (different color from Character class)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(GetPosition(), cubeSize, currentColor);
    }

    // Transform delegation
    void CharacterController::SetPosition(const Math::Vec3& position) {
        if (m_movementComponent) {
            m_movementComponent->SetPosition(position);
        }
    }

    const Math::Vec3& CharacterController::GetPosition() const {
        if (m_movementComponent) {
            return m_movementComponent->GetPosition();
        }
        static Math::Vec3 defaultPos(0.0f, 0.9f, 0.0f);
        return defaultPos;
    }

    void CharacterController::SetRotation(float yaw) {
        if (m_movementComponent) {
            m_movementComponent->SetRotation(yaw);
        }
    }

    float CharacterController::GetRotation() const {
        if (m_movementComponent) {
            return m_movementComponent->GetRotation();
        }
        return 0.0f;
    }

    // Movement properties delegation
    void CharacterController::SetMoveSpeed(float speed) {
        if (m_movementComponent) {
            auto config = m_movementComponent->GetMovementConfig();
            config.maxWalkSpeed = speed;
            m_movementComponent->SetMovementConfig(config);
        }
    }

    float CharacterController::GetMoveSpeed() const {
        if (m_movementComponent) {
            return m_movementComponent->GetMovementConfig().maxWalkSpeed;
        }
        return 6.0f; // Default speed
    }

    void CharacterController::SetJumpSpeed(float speed) {
        if (m_movementComponent) {
            auto config = m_movementComponent->GetMovementConfig();
            config.jumpZVelocity = speed;
            m_movementComponent->SetMovementConfig(config);
        }
    }

    float CharacterController::GetJumpSpeed() const {
        if (m_movementComponent) {
            return m_movementComponent->GetMovementConfig().jumpZVelocity;
        }
        return 10.0f; // Default jump speed
    }

    void CharacterController::SetCharacterSize(float radius, float height) {
        m_radius = radius;
        m_height = height;
        if (m_movementComponent) {
            m_movementComponent->SetCharacterSize(radius, height);
        }
    }

    // Slope and step settings delegation
    void CharacterController::SetMaxSlopeAngle(float degrees) {
        if (m_movementComponent) {
            auto config = m_movementComponent->GetMovementConfig();
            config.maxSlopeAngle = degrees;
            m_movementComponent->SetMovementConfig(config);
        }
    }

    float CharacterController::GetMaxSlopeAngle() const {
        if (m_movementComponent) {
            return m_movementComponent->GetMovementConfig().maxSlopeAngle;
        }
        return 45.0f; // Default slope angle
    }

    void CharacterController::SetMaxStepHeight(float height) {
        if (m_movementComponent) {
            auto config = m_movementComponent->GetMovementConfig();
            config.maxStepHeight = height;
            m_movementComponent->SetMovementConfig(config);
        }
    }

    float CharacterController::GetMaxStepHeight() const {
        if (m_movementComponent) {
            return m_movementComponent->GetMovementConfig().maxStepHeight;
        }
        return 0.3f; // Default step height
    }

    // State queries delegation
    CharacterController::MovementState CharacterController::GetMovementState() const {
        if (m_movementComponent) {
            auto mode = m_movementComponent->GetMovementMode();
            switch (mode) {
                case CharacterMovementComponent::MovementMode::Walking:
                    return MovementState::Grounded;
                case CharacterMovementComponent::MovementMode::Falling:
                    return MovementState::Airborne;
                case CharacterMovementComponent::MovementMode::Flying:
                    return MovementState::Airborne;
                default:
                    return MovementState::Airborne;
            }
        }
        return MovementState::Airborne;
    }

    bool CharacterController::IsGrounded() const {
        if (m_movementComponent) {
            return m_movementComponent->IsGrounded();
        }
        return false;
    }

    const Math::Vec3& CharacterController::GetVelocity() const {
        if (m_movementComponent) {
            return m_movementComponent->GetVelocity();
        }
        static Math::Vec3 defaultVel(0.0f);
        return defaultVel;
    }

    // Movement component management
    void CharacterController::SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component) {
        if (component) {
            // Preserve current state
            Math::Vec3 currentPosition(0.0f, 0.9f, 0.0f);
            Math::Vec3 currentVelocity(0.0f);
            float currentRotation = 0.0f;
            
            if (m_movementComponent) {
                currentPosition = m_movementComponent->GetPosition();
                currentVelocity = m_movementComponent->GetVelocity();
                currentRotation = m_movementComponent->GetRotation();
                
                // Shutdown old component
                m_movementComponent->Shutdown();
            }
            
            // Set new component
            m_movementComponent = std::move(component);
            
            // Initialize new component
            if (m_physicsEngine) {
                m_movementComponent->Initialize(m_physicsEngine);
            }
            
            // Configure character size
            m_movementComponent->SetCharacterSize(m_radius, m_height);
            
            // Restore state to new component
            m_movementComponent->SetPosition(currentPosition);
            m_movementComponent->SetVelocity(currentVelocity);
            m_movementComponent->SetRotation(currentRotation);
            
            LOG_INFO("CharacterController switched to " + std::string(GetMovementTypeName()) + 
                    " at position (" + std::to_string(currentPosition.x) + ", " + 
                    std::to_string(currentPosition.y) + ", " + std::to_string(currentPosition.z) + ")");
        }
    }

    void CharacterController::SwitchToPhysicsMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Physics);
        SetMovementComponent(std::move(component));
    }

    void CharacterController::SwitchToCharacterMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::CharacterMovement);
        SetMovementComponent(std::move(component));
    }

    void CharacterController::SwitchToHybridMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        SetMovementComponent(std::move(component));
    }

    const char* CharacterController::GetMovementTypeName() const {
        if (m_movementComponent) {
            return m_movementComponent->GetComponentTypeName();
        }
        return "NoMovementComponent";
    }

    Math::Vec4 CharacterController::GetMovementTypeColor() const {
        if (!m_movementComponent) {
            return Math::Vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray for no component
        }
        
        const char* typeName = m_movementComponent->GetComponentTypeName();
        
        // CharacterController colors (red tones) - simplified to 3 components
        if (strcmp(typeName, "CharacterMovementComponent") == 0) {
            return Math::Vec4(1.0f, 0.2f, 0.4f, 1.0f); // Bright red for basic movement
        }
        else if (strcmp(typeName, "HybridMovementComponent") == 0) {
            return Math::Vec4(1.0f, 0.0f, 0.8f, 1.0f); // Magenta for hybrid
        }
        else if (strcmp(typeName, "PhysicsMovementComponent") == 0) {
            return Math::Vec4(0.8f, 0.0f, 0.2f, 1.0f); // Dark red for physics
        }
        
        return Math::Vec4(1.0f, 0.3f, 0.3f, 1.0f); // Default red
    }

    bool CharacterController::HasFallen() const {
        return GetPosition().y < m_fallLimit;
    }

    void CharacterController::ResetToSpawnPosition() {
        if (m_movementComponent) {
            // Reset position to spawn point
            m_movementComponent->SetPosition(m_spawnPosition);
            
            // Reset velocity to zero to stop any falling motion
            m_movementComponent->SetVelocity(Math::Vec3(0.0f));
            
            // Reset rotation to default
            m_movementComponent->SetRotation(0.0f);
            
            LOG_INFO("CharacterController reset to spawn position: (" + 
                    std::to_string(m_spawnPosition.x) + ", " + 
                    std::to_string(m_spawnPosition.y) + ", " + 
                    std::to_string(m_spawnPosition.z) + ")");
        }
    }

    void CharacterController::InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine) {
        // Use HybridMovementComponent by default for physics collision with direct control
        m_movementComponent = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        if (m_movementComponent && physicsEngine) {
            m_movementComponent->Initialize(physicsEngine);
            m_movementComponent->SetCharacterSize(m_radius, m_height);
        }
    }
}