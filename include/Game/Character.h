#pragma once

#include "Core/Math.h"
#include "Game/CharacterMovementComponent.h"
#include <memory>

namespace GameEngine {
    class PrimitiveRenderer;
    class InputManager;
    class PhysicsEngine;
    class AudioEngine;
    struct AudioClip;

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

        bool Initialize(PhysicsEngine* physicsEngine = nullptr, AudioEngine* audioEngine = nullptr);
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

        // Audio settings
        void SetAudioEnabled(bool enabled) { m_audioEnabled = enabled; }
        bool IsAudioEnabled() const { return m_audioEnabled; }
        void SetFootstepInterval(float interval) { m_footstepInterval = interval; }
        float GetFootstepInterval() const { return m_footstepInterval; }

    private:
        void InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine);
        void InitializeAudio(AudioEngine* audioEngine);
        void UpdateAudio(float deltaTime);
        void PlayJumpSound();
        void UpdateFootsteps(float deltaTime);

        // Character properties (human proportions)
        float m_height = 1.8f;  // Height of a person
        float m_radius = 0.3f;  // Radius of capsule

        // Movement component (handles all movement logic)
        std::unique_ptr<CharacterMovementComponent> m_movementComponent;
        
        // Physics engine reference (for component switching)
        PhysicsEngine* m_physicsEngine = nullptr;

        // Rendering
        Math::Vec4 m_color{0.2f, 0.6f, 1.0f, 1.0f}; // Blue capsule

        // Fall detection and reset system
        float m_fallLimit = -10.0f;  // Y position below which character is considered fallen
        Math::Vec3 m_spawnPosition{0.0f, 1.0f, 0.0f};  // Default spawn position

        // Audio system integration
        AudioEngine* m_audioEngine = nullptr;
        bool m_audioEnabled = true;
        
        // Audio sources for character sounds
        uint32_t m_jumpAudioSource = 0;
        uint32_t m_footstepAudioSource = 0;
        
        // Audio clips
        std::shared_ptr<AudioClip> m_jumpSound;
        std::shared_ptr<AudioClip> m_footstepSound;
        
        // Audio state tracking
        bool m_wasGrounded = true;
        bool m_wasJumping = false;
        float m_footstepTimer = 0.0f;
        float m_footstepInterval = 0.5f;  // Time between footsteps when walking
        Math::Vec3 m_lastFootstepPosition{0.0f};
        float m_footstepMinDistance = 1.0f;  // Minimum distance to travel before next footstep
    };
}