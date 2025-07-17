#pragma once

#include "Game/CharacterMovementComponent.h"

namespace GameEngine {
    /**
     * @brief Deterministic movement component
     * 
     * Provides precise, predictable character control without physics simulation.
     * Suitable for players, NPCs, and any character requiring exact positioning.
     * Uses direct position manipulation with manual collision detection.
     */
    class DeterministicMovementComponent : public CharacterMovementComponent {
    public:
        DeterministicMovementComponent();
        virtual ~DeterministicMovementComponent();

        // Component lifecycle
        bool Initialize(PhysicsEngine* physicsEngine) override;
        void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) override;
        void Shutdown() override;

        // Transform interface
        void SetPosition(const Math::Vec3& position) override { m_position = position; }
        const Math::Vec3& GetPosition() const override { return m_position; }
        void SetRotation(float yaw) override { m_yaw = yaw; }
        float GetRotation() const override { return m_yaw; }

        // Velocity interface
        const Math::Vec3& GetVelocity() const override { return m_velocity; }
        void SetVelocity(const Math::Vec3& velocity) override { m_velocity = velocity; }
        void AddVelocity(const Math::Vec3& deltaVelocity) override { m_velocity += deltaVelocity; }

        // Movement state
        bool IsGrounded() const override { return m_isGrounded; }
        bool IsJumping() const override { return m_isJumping; }
        bool IsFalling() const override { return !m_isGrounded && m_velocity.y < 0.0f; }

        // Movement commands
        void Jump() override;
        void StopJumping() override;
        void AddMovementInput(const Math::Vec3& worldDirection, float scaleValue = 1.0f) override;

        // Component identification
        const char* GetComponentTypeName() const override { return "DeterministicMovementComponent"; }

        // Deterministic-specific configuration
        void SetGroundLevel(float groundLevel) { m_groundLevel = groundLevel; }
        float GetGroundLevel() const { return m_groundLevel; }
        void SetGravity(float gravity) { m_gravity = gravity; }
        float GetGravity() const { return m_gravity; }

    private:
        void HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera);
        void UpdateMovement(float deltaTime);
        void ApplyGravity(float deltaTime);
        void CheckGroundCollision();
        void ProcessMovementInput();

        // Transform state
        Math::Vec3 m_position{0.0f, 0.9f, 0.0f};
        Math::Vec3 m_velocity{0.0f};
        float m_yaw = 0.0f;

        // Movement state
        bool m_isGrounded = true;
        bool m_isJumping = false;
        Math::Vec3 m_inputDirection{0.0f};

        // Deterministic physics parameters
        float m_gravity = -15.0f;           // Gravity acceleration (m/s²)
        float m_groundLevel = 0.9f;         // Ground level (character height/2)
        
        // Movement parameters
        float m_acceleration = 25.0f;       // Ground acceleration (increased for responsiveness)
        float m_airAcceleration = 8.0f;     // Air acceleration (increased)
        float m_friction = 15.0f;           // Ground friction (increased for less sliding)
        float m_airFriction = 2.0f;         // Air resistance (increased)
        
        // Smooth stopping parameters
        float m_brakingFriction = 25.0f;    // Extra friction when no input (smooth stopping)
        float m_minSpeedThreshold = 0.1f;   // Speed below which we stop completely
        
        // Input accumulation
        Math::Vec3 m_accumulatedInput{0.0f};
    };
}