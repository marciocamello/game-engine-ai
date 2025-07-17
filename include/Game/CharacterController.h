#pragma once

#include "Core/Math.h"
#include <vector>

namespace GameEngine {
    class PrimitiveRenderer;
    class InputManager;
    class PhysicsEngine;

    /**
     * @brief Hybrid physics character controller
     * 
     * Uses physics for collision detection only, with direct position control
     * for precise, deterministic movement. Implements ghost object/kinematic
     * body approach for collision queries without physics simulation.
     */
    class CharacterController {
    public:
        /**
         * @brief Movement state for the character
         */
        enum class MovementState {
            Grounded,
            Airborne,
            Sliding
        };

        /**
         * @brief Collision information from sweep tests
         */
        struct CollisionInfo {
            bool hasCollision = false;
            Math::Vec3 contactPoint{0.0f};
            Math::Vec3 contactNormal{0.0f};
            Math::Vec3 normal{0.0f};  // Alias for contactNormal for convenience
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

        CharacterController();
        ~CharacterController();

        bool Initialize(PhysicsEngine* physicsEngine);
        void Update(float deltaTime, InputManager* input, class ThirdPersonCameraSystem* camera = nullptr);
        void Render(PrimitiveRenderer* renderer);

        // Transform
        void SetPosition(const Math::Vec3& position);
        const Math::Vec3& GetPosition() const { return m_position; }
        
        void SetRotation(float yaw) { m_yaw = yaw; }
        float GetRotation() const { return m_yaw; }

        // Movement properties
        void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
        float GetMoveSpeed() const { return m_moveSpeed; }
        
        void SetJumpSpeed(float speed) { m_jumpSpeed = speed; }
        float GetJumpSpeed() const { return m_jumpSpeed; }

        // Character properties
        float GetHeight() const { return m_height; }
        float GetRadius() const { return m_radius; }
        
        // Slope and step settings
        void SetMaxSlopeAngle(float degrees) { m_maxSlopeAngle = degrees; }
        float GetMaxSlopeAngle() const { return m_maxSlopeAngle; }
        
        void SetMaxStepHeight(float height) { m_maxStepHeight = height; }
        float GetMaxStepHeight() const { return m_maxStepHeight; }

        // State queries
        MovementState GetMovementState() const { return m_movementState; }
        bool IsGrounded() const { return m_movementState == MovementState::Grounded; }
        const Math::Vec3& GetVelocity() const { return m_velocity; }

    private:
        // Core movement methods
        void HandleMovementInput(float deltaTime, InputManager* input, class ThirdPersonCameraSystem* camera);
        void UpdateMovement(float deltaTime);
        void ApplyGravity(float deltaTime);
        
        // Collision detection methods
        CollisionInfo SweepTest(const Math::Vec3& from, const Math::Vec3& to, float radius, float height);
        bool IsGroundedCheck();
        StepInfo CheckStepUp(const Math::Vec3& moveDirection, float moveDistance);
        bool CheckSlope(const Math::Vec3& normal);
        
        // Movement resolution methods
        Math::Vec3 ResolveMovement(const Math::Vec3& desiredMovement);
        Math::Vec3 ResolveCollision(const Math::Vec3& desiredMovement, const CollisionInfo& collision);
        Math::Vec3 SlideAlongSurface(const Math::Vec3& movement, const Math::Vec3& normal);
        
        // Ghost object management
        void CreateGhostObject();
        void DestroyGhostObject();
        void UpdateGhostObjectPosition();

        // Transform
        Math::Vec3 m_position{0.0f, 0.9f, 0.0f}; // Height/2 to sit on ground
        Math::Vec3 m_velocity{0.0f};
        float m_yaw = 0.0f;

        // Character properties (optimized for better gameplay)
        float m_height = 1.8f;          // Height of capsule
        float m_radius = 0.3f;          // Radius of capsule
        float m_moveSpeed = 6.0f;       // Movement speed (increased for better feel)
        float m_jumpSpeed = 10.0f;      // Jump velocity (increased for better jumping)
        float m_gravity = -20.0f;       // Gravity acceleration (stronger for snappier feel)

        // Movement constraints
        float m_maxSlopeAngle = 45.0f;  // Maximum walkable slope in degrees
        float m_maxStepHeight = 0.3f;   // Maximum step height
        float m_skinWidth = 0.02f;      // Collision skin width
        
        // State
        MovementState m_movementState = MovementState::Airborne;
        bool m_wasGrounded = false;
        float m_groundCheckDistance = 0.1f;
        
        // Input state
        Math::Vec3 m_inputDirection{0.0f};
        bool m_jumpRequested = false;

        // Physics
        PhysicsEngine* m_physicsEngine = nullptr;
        uint32_t m_ghostObjectId = 0;  // Ghost object for collision detection

        // Rendering
        Math::Vec4 m_color{0.8f, 0.2f, 0.2f, 1.0f}; // Red capsule to distinguish from Character
        
        // Performance tracking
        mutable int m_sweepTestCount = 0;
        mutable float m_lastFrameTime = 0.0f;
    };
}