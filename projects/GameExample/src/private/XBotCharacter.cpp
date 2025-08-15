#include "../public/XBotCharacter.h"
#include "Core/Logger.h"
#include <GLFW/glfw3.h>

namespace GameExample {

    XBotCharacter::XBotCharacter() {
        LOG_INFO("Creating XBotCharacter instance");
    }

    bool XBotCharacter::Initialize(GameEngine::PhysicsEngine* physicsEngine) {
        LOG_INFO("Initializing XBotCharacter");

        // Initialize base character first
        if (!Character::Initialize(physicsEngine)) {
            LOG_ERROR("Failed to initialize base Character for XBotCharacter");
            return false;
        }

        // Load XBot-specific model
        if (!LoadXBotModel()) {
            LOG_ERROR("Failed to load XBot model");
            return false;
        }

        // Configure model for XBot (Mixamo models need scaling and offset)
        SetModelScale(0.01f);  // Mixamo models export very large
        SetModelOffsetConfiguration(GameEngine::ModelOffsetConfiguration::CenteredInCapsule());
        SetUseFBXModel(true);

        LOG_INFO("XBotCharacter initialized successfully");
        return true;
    }

    void XBotCharacter::Update(float deltaTime, GameEngine::InputManager* input, GameEngine::ThirdPersonCameraSystem* camera) {
        // Update base character
        Character::Update(deltaTime, input, camera);

        // Update XBot-specific timers
        if (m_celebrationTimer > 0.0f) {
            m_celebrationTimer -= deltaTime;
        }
        
        if (m_hitReactionTimer > 0.0f) {
            m_hitReactionTimer -= deltaTime;
        }

        // XBot-specific input handling will be implemented later
        // For now, just log that we're updating XBot character
        if (input) {
            // Placeholder for XBot-specific input handling
            LOG_INFO("XBot character input handling - placeholder");
        }
    }

    bool XBotCharacter::LoadCharacterAnimations() {
        LOG_INFO("Loading XBot character animations");

        // For now, just log that we're loading XBot-specific animations
        // The actual animation loading will be implemented when the animation system is complete
        LOG_INFO("XBot animation loading - placeholder implementation");
        LOG_INFO("Will load animations from: " + GetXBotAnimationPath("Idle"));
        LOG_INFO("Will load animations from: " + GetXBotAnimationPath("Walking"));

        return true;
    }

    void XBotCharacter::SetupCharacterAnimationStateMachine() {
        LOG_INFO("Setting up XBot animation state machine");

        // For now, just log that we're setting up XBot-specific state machine
        // The actual state machine setup will be implemented when the animation system is complete
        LOG_INFO("XBot state machine setup - placeholder implementation");
        LOG_INFO("Will create states for: Idle, Walk, Run, Jump, etc.");

        LOG_INFO("XBot animation state machine setup complete");
    }

    bool XBotCharacter::LoadXBotModel() {
        LOG_INFO("Loading XBot character model");

        std::string modelPath = GetXBotAssetPath("XBotCharacter.fbx");
        
        if (!LoadFBXModel(modelPath)) {
            LOG_ERROR("Failed to load XBot model from: " + modelPath);
            return false;
        }

        LOG_INFO("XBot model loaded successfully from: " + modelPath);
        return true;
    }

    bool XBotCharacter::LoadXBotAnimations() {
        // This method is called by LoadCharacterAnimations()
        // Kept separate for potential future use
        return LoadCharacterAnimations();
    }

    void XBotCharacter::CreateXBotStateMachine() {
        auto* controller = GetAnimationController();
        if (!controller) {
            return;
        }

        // Note: This is a simplified state machine setup
        // In a full implementation, we would create AnimationStateMachine,
        // AnimationState, and AnimationTransition objects and configure them
        // For now, we'll use the controller's parameter system

        LOG_INFO("XBot state machine created with basic parameter setup");
    }

    std::string XBotCharacter::GetXBotAssetPath(const std::string& assetName) const {
        // Following asset naming conventions: characters/XBotCharacter/
        return "projects/GameExample/assets/characters/XBotCharacter/" + assetName;
    }

    std::string XBotCharacter::GetXBotAnimationPath(const std::string& animationName) const {
        // Following asset naming conventions: characters/XBotCharacter/animations/
        return "projects/GameExample/assets/characters/XBotCharacter/animations/" + animationName + ".fbx";
    }

    void XBotCharacter::SetCrouching(bool crouching) {
        if (m_isCrouching != crouching) {
            m_isCrouching = crouching;
            LOG_INFO("XBot crouching state changed to: " + std::string(m_isCrouching ? "true" : "false"));
        }
    }

    void XBotCharacter::TriggerCelebration() {
        if (m_celebrationTimer <= 0.0f) {
            m_celebrationTimer = 3.0f; // Prevent rapid celebration triggering
            LOG_INFO("XBot celebration triggered");
        }
    }

    void XBotCharacter::TriggerHitReaction() {
        if (m_hitReactionTimer <= 0.0f && !m_isDead) {
            m_hitReactionTimer = 1.0f; // Prevent rapid hit reactions
            LOG_INFO("XBot hit reaction triggered");
        }
    }

    void XBotCharacter::TriggerDeath() {
        if (!m_isDead) {
            m_isDead = true;
            LOG_INFO("XBot death triggered");
        }
    }

    void XBotCharacter::TriggerLeftTurn() {
        LOG_INFO("XBot left turn triggered");
    }

    void XBotCharacter::TriggerRightTurn() {
        LOG_INFO("XBot right turn triggered");
    }

} // namespace GameExample