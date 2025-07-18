#pragma once

#include "Core/Math.h"
#include <memory>

namespace GameEngine {
    class InputManager;
    class PhysicsEngine;
    class ThirdPersonCameraSystem;

    /**
     * @brief Movement component types for third-person games
     */
    enum class MovementComponentType {
        CharacterMovement,  ///< Basic character movement with manual physics
        Physics,           ///< Full physics simulation for realistic movement
        Hybrid            ///< Physics collision detection with direct control
    };

    /**
     * @brief Base class for character movement components
     * 
     * Provides a common interface for different movement implementations.
     * Optimized for third-person action games like GTA.
     */
    class CharacterMovementComponent {
    public:
        /**
         * @brief Movement mode enumeration
         */
        enum class MovementMode {
            Walking,        ///< Standard ground-based movement
            Falling,        ///< Airborne movement with gravity
            Flying,         ///< Free-form movement without gravity
            Swimming,       ///< Water-based movement (future)
            Custom          ///< Custom movement mode
        };

        /**
         * @brief Movement configuration parameters
         */
        struct MovementConfig {
            float maxWalkSpeed = 6.0f;          ///< Maximum walking speed (m/s)
            float maxAcceleration = 20.0f;      ///< Maximum acceleration (m/s²)
            float brakingDeceleration = 20.0f;  ///< Braking deceleration (m/s²)
            float jumpZVelocity = 10.0f;        ///< Initial jump velocity (m/s)
            float gravityScale = 1.0f;          ///< Gravity multiplier
            float airControl = 0.2f;            ///< Air control factor (0-1)
            float groundFriction = 8.0f;        ///< Ground friction coefficient
            float maxStepHeight = 0.3f;         ///< Maximum step height (m)
            float maxSlopeAngle = 45.0f;        ///< Maximum walkable slope (degrees)
            bool canJump = true;                ///< Whether jumping is allowed
            bool canWalkOffLedges = true;       ///< Whether character can walk off edges
        };

        CharacterMovementComponent();
        virtual ~CharacterMovementComponent();

        // Component lifecycle
        virtual bool Initialize(PhysicsEngine* physicsEngine) = 0;
        virtual void Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera = nullptr) = 0;
        virtual void Shutdown() = 0;

        // Transform interface
        virtual void SetPosition(const Math::Vec3& position) = 0;
        virtual const Math::Vec3& GetPosition() const = 0;
        virtual void SetRotation(float yaw) = 0;
        virtual float GetRotation() const = 0;

        // Velocity interface
        virtual const Math::Vec3& GetVelocity() const = 0;
        virtual void SetVelocity(const Math::Vec3& velocity) = 0;
        virtual void AddVelocity(const Math::Vec3& deltaVelocity) = 0;

        // Movement state
        virtual MovementMode GetMovementMode() const { return m_movementMode; }
        virtual bool IsGrounded() const = 0;
        virtual bool IsJumping() const = 0;
        virtual bool IsFalling() const = 0;

        // Configuration
        virtual void SetMovementConfig(const MovementConfig& config) { m_config = config; }
        virtual const MovementConfig& GetMovementConfig() const { return m_config; }

        // Character properties
        virtual void SetCharacterSize(float radius, float height) {
            m_characterRadius = radius;
            m_characterHeight = height;
        }
        virtual float GetCharacterRadius() const { return m_characterRadius; }
        virtual float GetCharacterHeight() const { return m_characterHeight; }

        // Movement commands
        virtual void Jump() = 0;
        virtual void StopJumping() = 0;
        virtual void AddMovementInput(const Math::Vec3& worldDirection, float scaleValue = 1.0f) = 0;

        // Physics integration
        virtual void SetPhysicsEngine(PhysicsEngine* physicsEngine) { m_physicsEngine = physicsEngine; }
        virtual PhysicsEngine* GetPhysicsEngine() const { return m_physicsEngine; }

        // Component type identification
        virtual const char* GetComponentTypeName() const = 0;

    protected:
        // Common movement utilities
        Math::Vec3 ConstrainInputVector(const Math::Vec3& inputVector) const;
        Math::Vec3 ScaleInputAcceleration(const Math::Vec3& inputAcceleration) const;
        bool ShouldJumpOutOfWater() const { return false; } // Future water support
        
        // Configuration and state
        MovementConfig m_config;
        MovementMode m_movementMode = MovementMode::Walking;
        
        // Character properties
        float m_characterRadius = 0.3f;
        float m_characterHeight = 1.8f;
        
        // Physics reference
        PhysicsEngine* m_physicsEngine = nullptr;
        
        // Input accumulation
        Math::Vec3 m_pendingInputVector{0.0f};
        bool m_jumpRequested = false;
        
        // Timing
        float m_deltaTime = 0.0f;
    };

    // Forward declaration
    class MovementComponentFactory;
}