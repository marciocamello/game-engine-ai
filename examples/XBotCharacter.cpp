#include "XBotCharacter.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationTransition.h"
#include "Core/Logger.h"

namespace GameExample {

    XBotCharacter::XBotCharacter() {
        LOG_INFO("XBotCharacter created");
    }

    XBotCharacter::~XBotCharacter() {
        LOG_INFO("XBotCharacter destroyed");
    }

    bool XBotCharacter::LoadCharacterAnimations() {
        if (!m_animationController || !m_xbotSkeleton) {
            LOG_ERROR("XBotCharacter: Animation controller or skeleton not initialized");
            return false;
        }

        // XBot-specific animation files
        std::vector<std::pair<std::string, std::string>> xbotAnimationFiles = {
            {"Idle", "assets/meshes/Idle.fbx"},
            {"Walking", "assets/meshes/Walking.fbx"},
            {"Running", "assets/meshes/Running.fbx"},
            {"Jump", "assets/meshes/Jump.fbx"},
            {"Attack", "assets/meshes/Attack.fbx"},
            {"Block", "assets/meshes/Block.fbx"},
            {"Hit", "assets/meshes/Hit.fbx"},
            {"Dying", "assets/meshes/Dying.fbx"},
            {"Celebrate", "assets/meshes/Celebrate.fbx"},
            {"LeftTurn", "assets/meshes/Left Turn.fbx"},
            {"RightTurn", "assets/meshes/Right Turn.fbx"},
            {"CrouchedWalking", "assets/meshes/Crouched Walking.fbx"}
        };

        int loadedAnimations = 0;
        for (const auto& animPair : xbotAnimationFiles) {
            if (LoadXBotAnimationFromFBX(animPair.second, animPair.first)) {
                loadedAnimations++;
            }
        }

        if (loadedAnimations == 0) {
            LOG_ERROR("XBotCharacter: No animations were loaded successfully");
            return false;
        }

        m_xbotAnimationsLoaded = true;
        LOG_INFO("XBotCharacter: Loaded " + std::to_string(loadedAnimations) + " animations for XBot character");
        return true;
    }

    void XBotCharacter::SetupCharacterAnimationStateMachine() {
        if (!m_animationController || !m_xbotAnimationsLoaded) {
            LOG_WARNING("XBotCharacter: Cannot setup state machine - controller or animations not ready");
            return;
        }

        CreateXBotStateMachine();
        SetupXBotAnimationParameters();
        
        LOG_INFO("XBotCharacter: Animation state machine configured for XBot");
    }

    bool XBotCharacter::LoadXBotAnimationFromFBX(const std::string& fbxPath, const std::string& animationName) {
        // Use the base class method for actual loading
        return LoadAnimationFromFBX(fbxPath, animationName);
    }

    void XBotCharacter::CreateXBotStateMachine() {
        // Create XBot-specific animation state machine
        auto stateMachine = std::make_shared<GameEngine::Animation::AnimationStateMachine>();
        
        // Create Idle state
        auto idleState = std::make_shared<GameEngine::Animation::AnimationState>("Idle", GameEngine::Animation::AnimationState::Type::Single);
        auto idleAnimation = m_animationController->GetAnimation("Idle");
        if (idleAnimation) {
            idleState->SetAnimation(idleAnimation);
            idleState->SetLooping(true);
            idleState->SetSpeed(1.0f);
        }
        
        // Create Walking state
        auto walkState = std::make_shared<GameEngine::Animation::AnimationState>("Walking", GameEngine::Animation::AnimationState::Type::Single);
        auto walkAnimation = m_animationController->GetAnimation("Walking");
        if (walkAnimation) {
            walkState->SetAnimation(walkAnimation);
            walkState->SetLooping(true);
            walkState->SetSpeed(1.0f);
        }
        
        // Add states to state machine
        stateMachine->AddState(idleState);
        stateMachine->AddState(walkState);
        
        // Set entry state
        stateMachine->SetEntryState("Idle");
        stateMachine->SetDefaultState("Idle");
        
        // Create transitions from Idle to Walking
        auto idleToWalkTransition = std::make_shared<GameEngine::Animation::AnimationTransition>("Idle", "Walking");
        idleToWalkTransition->SetDuration(0.2f); // 0.2 second blend time
        idleToWalkTransition->SetBlendMode(GameEngine::Animation::AnimationTransition::BlendMode::Linear);
        
        // Add condition: Speed > 0.5
        GameEngine::Animation::TransitionCondition speedCondition = GameEngine::Animation::TransitionCondition::FloatGreater("Speed", 0.5f);
        idleToWalkTransition->AddCondition(speedCondition);
        
        stateMachine->AddTransition("Idle", "Walking", idleToWalkTransition);
        
        // Create transitions from Walking to Idle
        auto walkToIdleTransition = std::make_shared<GameEngine::Animation::AnimationTransition>("Walking", "Idle");
        walkToIdleTransition->SetDuration(0.2f); // 0.2 second blend time
        walkToIdleTransition->SetBlendMode(GameEngine::Animation::AnimationTransition::BlendMode::Linear);
        
        // Add condition: Speed <= 0.5
        GameEngine::Animation::TransitionCondition stopCondition = GameEngine::Animation::TransitionCondition::FloatLess("Speed", 0.5f);
        walkToIdleTransition->AddCondition(stopCondition);
        
        stateMachine->AddTransition("Walking", "Idle", walkToIdleTransition);
        
        // Set the state machine in the animation controller
        m_animationController->SetStateMachine(stateMachine);
        
        // Start the state machine
        stateMachine->Start();
        
        LOG_INFO("XBotCharacter: Created state machine with Idle and Walking states");
        LOG_INFO("XBotCharacter:   - Idle -> Walking transition: Speed > 0.5");
        LOG_INFO("XBotCharacter:   - Walking -> Idle transition: Speed <= 0.5");
        LOG_INFO("XBotCharacter:   - Blend time: 0.2 seconds for smooth transitions");
    }

    void XBotCharacter::SetupXBotAnimationParameters() {
        if (!m_animationController) {
            return;
        }

        // Initialize XBot-specific animation parameters
        m_animationController->SetFloat("Speed", 0.0f);
        m_animationController->SetBool("IsGrounded", true);
        m_animationController->SetBool("IsJumping", false);
        m_animationController->SetBool("IsCrouching", false);
        m_animationController->SetTrigger("Attack");
        m_animationController->SetTrigger("Block");
        m_animationController->SetTrigger("Hit");
        m_animationController->SetTrigger("Die");
        m_animationController->SetTrigger("Celebrate");

        LOG_INFO("XBotCharacter: Animation parameters initialized for XBot");
    }

} // namespace GameExample