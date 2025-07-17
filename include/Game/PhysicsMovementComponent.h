#pragma once

#include "Game/CharacterMovementComponent.h"

namespace GameEngine {
    /**
     * @brief Physics-based movement component
     * 
     * Uses full physics simulation for character movement. Suitable for
     * vehicles, ragdolls, and objects that need realistic physics behavior.
     * Movement is achieved through forces and impulses applied to rigid bodies.
     */
    class PhysicsMovementComponent : public CharacterMovementComponent {
    public:
        PhysicsMovementComponent();
        virtual ~PhysicsMovementComponent();

        // Component lifecycle
        bool Initialize(PhysicsEngine* physicsEngine) override;
        void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) override;
        void Shutdown() override;

        // Transform interface
        void SetPosition(const Math::Vec3& position) override;
        const Math::Vec3& GetPosition() const override { return m_position; }
        void SetRotation(float yaw) override;
        float GetRotation() const override { return m_yaw; }

        // Velocity interface
        const Math::Vec3& GetVelocity() const override { return m_velocity; }
        void SetVelocity(const Math::Vec3& velocity) override;
        void AddVelocity(const Math::Vec3& deltaVelocity) override;

        // Movement state
        bool IsGrounded() const override;
        bool IsJumping() const override { return m_isJumping; }
        bool IsFalling() const override { return !IsGrounded() && m_velocity.y < 0.0f; }

        // Movement commands
        void Jump() override;
        void StopJumping() override;
        void AddMovementInput(const Math::Vec3& worldDirection, float scaleValue = 1.0f) override;

        // Component identification
        const char* GetComponentTypeName() const override { return "PhysicsMovementComponent"; }

        // Physics-specific configuration
        void SetMass(float mass);
        float GetMass() const { return m_mass; }
        void SetFriction(float friction);
        void SetRestitution(float restitution);
        void SetLinearDamping(float damping);
        void SetAngularDamping(float damping);

    private:
        void HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera);
        void UpdatePhysicsState();
        void ApplyMovementForces();
        void CreateRigidBody();
        void DestroyRigidBody();

        // Transform state (synced from physics)
        Math::Vec3 m_position{0.0f, 0.9f, 0.0f};
        Math::Vec3 m_velocity{0.0f};
        float m_yaw = 0.0f;

        // Movement state
        bool m_isGrounded = false;
        bool m_isJumping = false;
        Math::Vec3 m_inputDirection{0.0f};

        // Physics properties
        float m_mass = 70.0f;           // Average human mass (kg)
        float m_friction = 1.5f;        // Higher friction for better control
        float m_restitution = 0.0f;     // No bouncing for characters
        float m_linearDamping = 1.2f;   // Higher damping for smoother stopping
        float m_angularDamping = 0.95f; // Very high angular damping for stability

        // Physics body
        uint32_t m_rigidBodyId = 0;

        // Force accumulation
        Math::Vec3 m_accumulatedForces{0.0f};
        Math::Vec3 m_accumulatedImpulses{0.0f};
    };
}