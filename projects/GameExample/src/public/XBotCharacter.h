#pragma once

#include "Game/Character.h"
#include <memory>
#include <string>

namespace GameExample {

    /**
     * @brief XBot character implementation for GameExample project
     * 
     * Extends the base GameEngine::Character class with XBot-specific animations,
     * state machine, and behavior. All XBot assets are loaded from the project's
     * character-specific asset directory following engine naming conventions.
     */
    class XBotCharacter : public GameEngine::Character {
    public:
        XBotCharacter();
        ~XBotCharacter() = default;

        // Character lifecycle
        bool Initialize(GameEngine::PhysicsEngine* physicsEngine = nullptr);
        void Update(float deltaTime, GameEngine::InputManager* input, class GameEngine::ThirdPersonCameraSystem* camera = nullptr);
        void Render(GameEngine::PrimitiveRenderer* renderer);

        // Movement-based animation parameter synchronization
        void UpdateMovementAnimationParameters();
        void SynchronizeXBotAnimationWithMovement();

        // XBot-specific functionality
        void SetCrouching(bool crouching);
        bool IsCrouching() const { return m_isCrouching; }
        
        void TriggerCelebration();
        void TriggerHitReaction();
        void TriggerDeath();
        
        // Turn animations
        void TriggerLeftTurn();
        void TriggerRightTurn();

    protected:
        // Override base character virtual methods for XBot-specific implementation
        bool LoadCharacterAnimations() override;
        void SetupCharacterAnimationStateMachine() override;

    private:
        // XBot-specific asset loading
        bool LoadXBotModel();
        bool LoadXBotAnimations();
        void CreateXBotStateMachine();
        
        // XBot animation asset paths (following naming conventions)
        std::string GetXBotAssetPath(const std::string& assetName) const;
        std::string GetXBotAnimationPath(const std::string& animationName) const;
        
        // XBot state management
        bool m_isCrouching = false;
        bool m_isInCombat = false;
        bool m_isDead = false;
        
        // Animation state tracking
        std::string m_previousMovementState = "Idle";
        float m_celebrationTimer = 0.0f;
        float m_hitReactionTimer = 0.0f;
        
        // Movement-based animation parameters
        float m_currentSpeed = 0.0f;
        float m_previousSpeed = 0.0f;
        bool m_wasGrounded = true;
        bool m_wasJumping = false;
        bool m_wasFalling = false;
        
        // Animation parameter thresholds for XBot
        static constexpr float WALK_SPEED_THRESHOLD = 0.5f;
        static constexpr float RUN_SPEED_THRESHOLD = 3.0f;
        static constexpr float SPEED_CHANGE_SMOOTHING = 5.0f;
        
        // XBot-specific animation names (matching FBX files)
        static constexpr const char* ANIM_IDLE = "Idle";
        static constexpr const char* ANIM_WALK = "Walking";
        static constexpr const char* ANIM_RUN = "Running";
        static constexpr const char* ANIM_JUMP = "Jump";
        static constexpr const char* ANIM_CELEBRATE = "Celebrate";
        static constexpr const char* ANIM_HIT = "Hit";
        static constexpr const char* ANIM_DEATH = "Dying";
        static constexpr const char* ANIM_LEFT_TURN = "Left Turn";
        static constexpr const char* ANIM_RIGHT_TURN = "Right Turn";
        static constexpr const char* ANIM_CROUCH_WALK = "Crouched Walking";
    };

} // namespace GameExample