#pragma once

#include "Game/CharacterMovementComponent.h"

namespace GameEngine {
    /**
     * @brief Hybrid movement component
     * 
     * Combines physics collision detection with direct position control.
     * Uses ghost objects/kinematic bodies for collision queries without
     * physics simulation. Provides precise control with realistic collision.
     */
    class HybridMovementComponent : public CharacterMovementComponent {
    public:
        /**
         * @brief Collision information from sweep tests
         */
        struct CollisionInfo {
            bool hasCollision = false;
            Math::Vec3 contactPoint{0.0f};
            Math::Vec3 contactNormal{0.0f};
            Math::Vec3 normal{0.0f};  // Alias for contactNormal
            float penetrationDepth = 0.0f;
            float distance = 0.0f;
            uint32_t hitBodyId = 0;
        };

        /**
         * @brief Step-up detection result
         */
        struct StepInfo {
            bool canStepUp = false;
            float stepHeight = 0.0f;
            Math::Vec3 stepPosition{0.0f};
        };

        HybridMovementComponent();
        virtual ~HybridMovementComponent();

        // Component lifecycle
        bool Initialize(PhysicsEngine* physicsEngine) override;
        void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) override;
        void Shutdown() override;

        // Transform interface
        void SetPosition(const Math::Vec3& position) override;
        const Math::Vec3& GetPosition() const override { return m_position; }
        void SetRotation(float yaw) override { m_yaw = yaw; }
        float GetRotation() const override { return m_yaw; }

        // Velocity interface
        const Math::Vec3& GetVelocity() const override { return m_velocity; }
        void SetVelocity(const Math::Vec3& velocity) override { m_velocity = velocity; }
        void AddVelocity(const Math::Vec3& deltaVelocity) override { m_velocity += deltaVelocity; }

        // Movement state
        bool IsGrounded() const override;
        bool IsJumping() const override { return m_isJumping; }
        bool IsFalling() const override { return !IsGrounded() && m_velocity.y < 0.0f; }

        // Movement commands
        void Jump() override;
        void StopJumping() override;
        void AddMovementInput(const Math::Vec3& worldDirection, float scaleValue = 1.0f) override;

        // Component identification
        const char* GetComponentTypeName() const override { return "HybridMovementComponent"; }

        // Hybrid-specific configuration
        void SetSkinWidth(float width) { m_skinWidth = width; }
        float GetSkinWidth() const { return m_skinWidth; }
        void SetGroundCheckDistance(float distance) { m_groundCheckDistance = distance; }
        float GetGroundCheckDistance() const { return m_groundCheckDistance; }

    private:
        void HandleMovementInput(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera);
        void UpdateMovement(float deltaTime);
        void ApplyGravity(float deltaTime);
        
        // Collision detection methods
        CollisionInfo SweepTest(const Math::Vec3& from, const Math::Vec3& to, float radius, float height);
        bool IsGroundedCheck() const;
        StepInfo CheckStepUp(const Math::Vec3& moveDirection, float moveDistance);
        bool CheckSlope(const Math::Vec3& normal);
        void CheckGroundCollision();
        
        // Movement resolution methods
        Math::Vec3 ResolveMovement(const Math::Vec3& desiredMovement);
        Math::Vec3 ResolveCollision(const Math::Vec3& desiredMovement, const CollisionInfo& collision);
        Math::Vec3 SlideAlongSurface(const Math::Vec3& movement, const Math::Vec3& normal);
        
        // Ghost object management
        void CreateGhostObject();
        void DestroyGhostObject();
        void UpdateGhostObjectPosition();

        // Transform state
        Math::Vec3 m_position{0.0f, 0.9f, 0.0f};
        Math::Vec3 m_velocity{0.0f};
        float m_yaw = 0.0f;

        // Movement state
        bool m_isGrounded = false;
        bool m_isJumping = false;
        Math::Vec3 m_inputDirection{0.0f};

        // Hybrid physics parameters
        float m_gravity = -20.0f;               // Stronger gravity for snappier feel
        float m_skinWidth = 0.02f;              // Collision skin width
        float m_groundCheckDistance = 0.1f;     // Ground detection distance
        
        // Ghost object for collision detection
        uint32_t m_ghostObjectId = 0;
        
        // Input accumulation
        Math::Vec3 m_accumulatedInput{0.0f};
        
        // Performance tracking
        mutable int m_sweepTestCount = 0;
        mutable float m_lastFrameTime = 0.0f;
    };
}