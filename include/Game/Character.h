#pragma once

#include "Core/Math.h"
#include "Game/CharacterMovementComponent.h"
#include <memory>

namespace GameEngine {
    class PrimitiveRenderer;
    class InputManager;
    class PhysicsEngine;

    /**
     * @brief Character class with component-based movement system
     * 
     * Uses CharacterMovementComponent for movement logic, allowing runtime
     * switching between different movement types (Physics, Deterministic, Hybrid).
     * Maintains backward compatibility with existing Character interface.
     */
    class Character {
    public:
        Character();
        ~Character();

        bool Initialize(PhysicsEngine* physicsEngine = nullptr);
        void Update(float deltaTime, InputManager* input, class ThirdPersonCameraSystem* camera = nullptr);
        void Render(PrimitiveRenderer* renderer);

        // Transform (delegated to movement component)
        void SetPosition(const Math::Vec3& position);
        const Math::Vec3& GetPosition() const;
        
        void SetRotation(float yaw);
        float GetRotation() const;

        // Movement (delegated to movement component)
        void SetMoveSpeed(float speed);
        float GetMoveSpeed() const;
        const Math::Vec3& GetVelocity() const;

        // Character properties
        float GetHeight() const { return m_height; }
        float GetRadius() const { return m_radius; }
        void SetCharacterSize(float radius, float height);

        // Movement state queries
        bool IsGrounded() const;
        bool IsJumping() const;
        bool IsFalling() const;

        // Movement component management
        void SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component);
        CharacterMovementComponent* GetMovementComponent() const { return m_movementComponent.get(); }
        
        // Convenience methods for switching movement types
        void SwitchToPhysicsMovement();
        void SwitchToDeterministicMovement();
        void SwitchToHybridMovement();
        
        // Get current movement type name
        const char* GetMovementTypeName() const;
        
        // Get color based on movement type
        Math::Vec4 GetMovementTypeColor() const;

    private:
        void InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine);

        // Character properties (human proportions)
        float m_height = 1.8f;  // Height of a person
        float m_radius = 0.3f;  // Radius of capsule

        // Movement component (handles all movement logic)
        std::unique_ptr<CharacterMovementComponent> m_movementComponent;
        
        // Physics engine reference (for component switching)
        PhysicsEngine* m_physicsEngine = nullptr;

        // Rendering
        Math::Vec4 m_color{0.2f, 0.6f, 1.0f, 1.0f}; // Blue capsule
    };
}