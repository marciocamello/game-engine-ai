#pragma once

#include "Core/Math.h"
#include "Game/CharacterMovementComponent.h"
#include <memory>
#include <string>

namespace GameEngine {
    class PrimitiveRenderer;
    class InputManager;
    class PhysicsEngine;
    class Model;
    class ModelLoader;

    namespace Animation {
        class AnimationController;
        class AnimationSkeleton;
        class SkeletalAnimation;
        class AnimationImporter;
    }

    /**
     * @brief Configuration structure for character model offset system
     * 
     * Provides easy adjustment of FBX model positioning within the physics capsule.
     * The offset is applied in local space relative to the character's position.
     */
    struct ModelOffsetConfiguration {
        Math::Vec3 offset{0.0f, 0.0f, 0.0f};  // Model offset in local space
        
        // Predefined offset configurations for common scenarios
        static ModelOffsetConfiguration CenteredInCapsule() {
            // Center the model within a standard capsule (radius=0.3f, height=1.8f)
            // For Mixamo models at 0.01f scale (they export huge), we need to offset downward 
            // to align feet with capsule bottom. At 0.01f scale, model is ~0.018 units tall.
            return ModelOffsetConfiguration{Math::Vec3(0.0f, -0.89f, 0.0f)};
        }
        
        static ModelOffsetConfiguration Default() {
            return ModelOffsetConfiguration{Math::Vec3(0.0f, 0.0f, 0.0f)};
        }
        
        static ModelOffsetConfiguration Custom(const Math::Vec3& customOffset) {
            return ModelOffsetConfiguration{customOffset};
        }
    };

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

        // FBX Model support
        bool LoadFBXModel(const std::string& fbxPath);
        void SetUseFBXModel(bool useFBX) { m_useFBXModel = useFBX; }
        bool IsUsingFBXModel() const { return m_useFBXModel && m_fbxModel != nullptr; }
        void SetModelScale(float scale) { m_modelScale = scale; }

        // Model offset system
        void SetModelOffset(const Math::Vec3& offset) { m_modelOffset = offset; }
        const Math::Vec3& GetModelOffset() const { return m_modelOffset; }
        void SetModelOffsetConfiguration(const ModelOffsetConfiguration& config) { m_modelOffset = config.offset; }
        ModelOffsetConfiguration GetModelOffsetConfiguration() const { return ModelOffsetConfiguration{m_modelOffset}; }

        // Debug visualization
        void SetShowDebugCapsule(bool show) { m_showDebugCapsule = show; }
        bool IsShowingDebugCapsule() const { return m_showDebugCapsule; }

        // Animation system integration
        bool InitializeAnimationSystem();
        void ShutdownAnimationSystem();
        bool LoadXbotAnimations();
        Animation::AnimationController* GetAnimationController() const { return m_animationController.get(); }
        bool HasAnimationController() const { return m_animationController != nullptr; }
        
        // Animation state synchronization with movement
        void UpdateAnimationState(float deltaTime);
        void SynchronizeAnimationWithMovement();
        
        // Animation control
        void PlayAnimation(const std::string& animationName, float fadeTime = 0.3f);
        void StopAnimation(const std::string& animationName, float fadeTime = 0.3f);
        void SetAnimationSpeed(float speed);
        float GetAnimationSpeed() const;
        
        // Animation parameters for state machine
        void SetAnimationParameter(const std::string& name, float value);
        void SetAnimationParameter(const std::string& name, int value);
        void SetAnimationParameter(const std::string& name, bool value);
        void TriggerAnimationEvent(const std::string& name);

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

        // FBX Model rendering
        std::shared_ptr<Model> m_fbxModel;
        std::unique_ptr<ModelLoader> m_modelLoader;
        bool m_useFBXModel = false;
        float m_modelScale = 1.0f;
        Math::Vec3 m_modelOffset{0.0f, 0.0f, 0.0f}; // Offset for centering FBX model within physics capsule


        // Fall detection and reset system
        float m_fallLimit = -10.0f;  // Y position below which character is considered fallen
        Math::Vec3 m_spawnPosition{0.0f, 1.0f, 0.0f};  // Default spawn position

        // Debug visualization
        bool m_showDebugCapsule = false;

        // Animation system
        std::unique_ptr<Animation::AnimationController> m_animationController;
        std::unique_ptr<Animation::AnimationImporter> m_animationImporter;
        std::shared_ptr<Animation::AnimationSkeleton> m_xbotSkeleton;
        bool m_animationSystemInitialized = false;
        
        // Animation state tracking
        std::string m_currentAnimationState = "Idle";
        float m_lastMovementSpeed = 0.0f;
        bool m_wasGrounded = true;
        bool m_wasJumping = false;

        // Animation asset loading
        bool LoadAnimationFromFBX(const std::string& fbxPath, const std::string& animationName);
        void SetupAnimationStateMachine();
        void UpdateMovementAnimationParameters();

    };
}