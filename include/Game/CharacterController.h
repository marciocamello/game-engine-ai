#pragma once

#include "Core/Math.h"
#include "Game/CharacterMovementComponent.h"
#include <memory>

namespace GameEngine {
    class PrimitiveRenderer;
    class InputManager;
    class PhysicsEngine;

    /**
     * @brief Character controller with component-based movement system
     * 
     * Uses HybridMovementComponent by default for physics collision detection
     * with direct position control. Maintains backward compatibility with
     * existing CharacterController interface.
     */
    class CharacterController {
    public:
        /**
         * @brief Movement state for the character (for backward compatibility)
         */
        enum class MovementState {
            Grounded,
            Airborne,
            Sliding
        };

        CharacterController();
        ~CharacterController();

        bool Initialize(PhysicsEngine* physicsEngine);
        void Update(float deltaTime, InputManager* input, class ThirdPersonCameraSystem* camera = nullptr);
        void Render(PrimitiveRenderer* renderer);

        // Transform (delegated to movement component)
        void SetPosition(const Math::Vec3& position);
        const Math::Vec3& GetPosition() const;
        
        void SetRotation(float yaw);
        float GetRotation() const;

        // Movement properties (delegated to movement component)
        void SetMoveSpeed(float speed);
        float GetMoveSpeed() const;
        
        void SetJumpSpeed(float speed);
        float GetJumpSpeed() const;

        // Character properties
        float GetHeight() const { return m_height; }
        float GetRadius() const { return m_radius; }
        void SetCharacterSize(float radius, float height);
        
        // Slope and step settings (delegated to movement component)
        void SetMaxSlopeAngle(float degrees);
        float GetMaxSlopeAngle() const;
        
        void SetMaxStepHeight(float height);
        float GetMaxStepHeight() const;

        // State queries (delegated to movement component)
        MovementState GetMovementState() const;
        bool IsGrounded() const;
        const Math::Vec3& GetVelocity() const;

        // Movement component management
        void SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component);
        CharacterMovementComponent* GetMovementComponent() const { return m_movementComponent.get(); }
        
        // Convenience methods for switching movement types (3 components only)
        void SwitchToCharacterMovement();  // Basic movement with manual physics
        void SwitchToPhysicsMovement();    // Full physics simulation
        void SwitchToHybridMovement();     // Physics collision + direct control (recommended)
        
        // Get current movement type name
        const char* GetMovementTypeName() const;
        
        // Get color based on movement type
        Math::Vec4 GetMovementTypeColor() const;

        // Fall detection and reset
        void SetFallLimit(float fallY) { m_fallLimit = fallY; }
        float GetFallLimit() const { return m_fallLimit; }
        bool HasFallen() const;
        void ResetToSpawnPosition();
        void SetSpawnPosition(const Math::Vec3& position) { m_spawnPosition = position; }
        const Math::Vec3& GetSpawnPosition() const { return m_spawnPosition; }

    private:
        void InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine);

        // Character properties
        float m_height = 1.8f;          // Height of capsule
        float m_radius = 0.3f;          // Radius of capsule

        // Movement component (handles all movement logic)
        std::unique_ptr<CharacterMovementComponent> m_movementComponent;
        
        // Physics engine reference (for component switching)
        PhysicsEngine* m_physicsEngine = nullptr;

        // Rendering
        Math::Vec4 m_color{0.8f, 0.2f, 0.2f, 1.0f}; // Red capsule to distinguish from Character

        // Fall detection and reset system
        float m_fallLimit = -10.0f;  // Y position below which character is considered fallen
        Math::Vec3 m_spawnPosition{0.0f, 1.0f, 0.0f};  // Default spawn position
    };
}