#include "Game/Character.h"
#include "Game/MovementComponentFactory.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    Character::Character() 
        : m_color(0.3f, 0.5f, 1.0f, 1.0f) // Azul para Character
    {
    }

    Character::~Character() {
        // Movement component cleanup is handled automatically by unique_ptr
    }

    bool Character::Initialize(PhysicsEngine* physicsEngine) {
        m_physicsEngine = physicsEngine;
        
        // Initialize default movement component (DeterministicMovementComponent)
        InitializeDefaultMovementComponent(physicsEngine);
        
        if (!m_movementComponent) {
            LOG_ERROR("Failed to initialize movement component for Character");
            return false;
        }
        
        LOG_INFO("Character initialized with component-based movement system (" + 
                std::string(GetMovementTypeName()) + ")");
        return true;
    }

    void Character::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        if (m_movementComponent) {
            m_movementComponent->Update(deltaTime, input, camera);
        }
    }

    void Character::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Get color based on current movement component type
        Math::Vec4 currentColor = GetMovementTypeColor();
        
        // Draw character as a simple cube (easier to see movement)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(GetPosition(), cubeSize, currentColor);
    }

    // Transform delegation
    void Character::SetPosition(const Math::Vec3& position) {
        if (m_movementComponent) {
            m_movementComponent->SetPosition(position);
        }
    }

    const Math::Vec3& Character::GetPosition() const {
        if (m_movementComponent) {
            return m_movementComponent->GetPosition();
        }
        static Math::Vec3 defaultPos(0.0f, 0.9f, 0.0f);
        return defaultPos;
    }

    void Character::SetRotation(float yaw) {
        if (m_movementComponent) {
            m_movementComponent->SetRotation(yaw);
        }
    }

    float Character::GetRotation() const {
        if (m_movementComponent) {
            return m_movementComponent->GetRotation();
        }
        return 0.0f;
    }

    // Movement delegation
    void Character::SetMoveSpeed(float speed) {
        if (m_movementComponent) {
            auto config = m_movementComponent->GetMovementConfig();
            config.maxWalkSpeed = speed;
            m_movementComponent->SetMovementConfig(config);
        }
    }

    float Character::GetMoveSpeed() const {
        if (m_movementComponent) {
            return m_movementComponent->GetMovementConfig().maxWalkSpeed;
        }
        return 6.0f; // Default speed
    }

    const Math::Vec3& Character::GetVelocity() const {
        if (m_movementComponent) {
            return m_movementComponent->GetVelocity();
        }
        static Math::Vec3 defaultVel(0.0f);
        return defaultVel;
    }

    void Character::SetCharacterSize(float radius, float height) {
        m_radius = radius;
        m_height = height;
        if (m_movementComponent) {
            m_movementComponent->SetCharacterSize(radius, height);
        }
    }

    // Movement state queries
    bool Character::IsGrounded() const {
        if (m_movementComponent) {
            return m_movementComponent->IsGrounded();
        }
        return false;
    }

    bool Character::IsJumping() const {
        if (m_movementComponent) {
            return m_movementComponent->IsJumping();
        }
        return false;
    }

    bool Character::IsFalling() const {
        if (m_movementComponent) {
            return m_movementComponent->IsFalling();
        }
        return false;
    }

    // Movement component management
    void Character::SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component) {
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
            
            LOG_INFO("Character switched to " + std::string(GetMovementTypeName()) + 
                    " at position (" + std::to_string(currentPosition.x) + ", " + 
                    std::to_string(currentPosition.y) + ", " + std::to_string(currentPosition.z) + ")");
        }
    }

    void Character::SwitchToCharacterMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::CharacterMovement);
        SetMovementComponent(std::move(component));
    }

    void Character::SwitchToPhysicsMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Physics);
        SetMovementComponent(std::move(component));
    }

    void Character::SwitchToHybridMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        SetMovementComponent(std::move(component));
    }

    const char* Character::GetMovementTypeName() const {
        if (m_movementComponent) {
            return m_movementComponent->GetComponentTypeName();
        }
        return "NoMovementComponent";
    }

    Math::Vec4 Character::GetMovementTypeColor() const {
        if (!m_movementComponent) {
            return Math::Vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray for no component
        }
        
        const char* typeName = m_movementComponent->GetComponentTypeName();
        
        // Character colors (blue tones) - simplified to 3 components
        if (strcmp(typeName, "CharacterMovementComponent") == 0) {
            return Math::Vec4(0.2f, 0.4f, 1.0f, 1.0f); // Bright blue for basic movement
        }
        else if (strcmp(typeName, "HybridMovementComponent") == 0) {
            return Math::Vec4(0.0f, 0.8f, 1.0f, 1.0f); // Cyan for hybrid (recommended)
        }
        else if (strcmp(typeName, "PhysicsMovementComponent") == 0) {
            return Math::Vec4(0.0f, 0.2f, 0.8f, 1.0f); // Dark blue for physics
        }
        
        return Math::Vec4(0.2f, 0.6f, 1.0f, 1.0f); // Default blue
    }



    bool Character::HasFallen() const {
        return GetPosition().y < m_fallLimit;
    }

    void Character::ResetToSpawnPosition() {
        if (m_movementComponent) {
            // Reset position to spawn point
            m_movementComponent->SetPosition(m_spawnPosition);
            
            // Reset velocity to zero to stop any falling motion
            m_movementComponent->SetVelocity(Math::Vec3(0.0f));
            
            // Reset rotation to default
            m_movementComponent->SetRotation(0.0f);
            
            LOG_INFO("Character reset to spawn position: (" + 
                    std::to_string(m_spawnPosition.x) + ", " + 
                    std::to_string(m_spawnPosition.y) + ", " + 
                    std::to_string(m_spawnPosition.z) + ")");
        }
    }

    void Character::InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine) {
        // Use HybridMovementComponent by default for third-person games (best balance)
        m_movementComponent = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        if (m_movementComponent && physicsEngine) {
            m_movementComponent->Initialize(physicsEngine);
            m_movementComponent->SetCharacterSize(m_radius, m_height);
        }
    }
}