#include "../public/XBotCharacter.h"
#include "Core/Logger.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationImporter.h"
#include "Graphics/PrimitiveRenderer.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

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

        // Update movement-based animation parameters
        UpdateMovementAnimationParameters();

        // Synchronize XBot-specific animation parameters with movement
        SynchronizeXBotAnimationWithMovement();
    }

    bool XBotCharacter::LoadCharacterAnimations() {
        LOG_INFO("Loading XBot character animations");

        auto* controller = GetAnimationController();
        if (!controller) {
            LOG_ERROR("No animation controller available for XBot");
            return false;
        }

        // Get the skeleton from the controller
        auto skeleton = controller->GetSkeleton();
        if (!skeleton) {
            LOG_ERROR("No skeleton available in animation controller for XBot");
            return false;
        }

        // Load essential XBot animations using the base Character method
        std::vector<std::string> animationsToLoad = {
            ANIM_IDLE,
            ANIM_WALK,
            ANIM_RUN,
            ANIM_JUMP,
            ANIM_CELEBRATE,
            ANIM_HIT,
            ANIM_DEATH,
            ANIM_LEFT_TURN,
            ANIM_RIGHT_TURN,
            ANIM_CROUCH_WALK
        };

        int loadedCount = 0;
        
        // Load each animation file using the base Character method
        for (const std::string& animName : animationsToLoad) {
            std::string animPath = GetXBotAnimationPath(animName);
            LOG_INFO("Loading XBot animation: " + animName + " from " + animPath);
            
            if (LoadAnimationFromFBX(animPath, animName)) {
                loadedCount++;
                LOG_INFO("Successfully loaded XBot animation: " + animName);
            } else {
                LOG_WARNING("Failed to load XBot animation: " + animName + " from " + animPath);
            }
        }

        LOG_INFO("XBot animation loading complete: " + std::to_string(loadedCount) + "/" + 
                std::to_string(animationsToLoad.size()) + " animations loaded");

        return loadedCount > 0; // Return true if at least some animations were loaded
    }

    void XBotCharacter::SetupCharacterAnimationStateMachine() {
        LOG_INFO("Setting up XBot animation state machine");

        auto* controller = GetAnimationController();
        if (!controller) {
            LOG_ERROR("No animation controller available for XBot state machine setup");
            return;
        }

        // Create a simple state machine for XBot
        // For now, we'll use the controller's parameter system to manage states
        // A full state machine implementation would create AnimationStateMachine objects

        // Set up basic animation parameters for XBot
        controller->SetFloat("Speed", 0.0f);
        controller->SetBool("IsGrounded", true);
        controller->SetBool("IsJumping", false);
        controller->SetBool("IsFalling", false);
        controller->SetBool("IsIdle", true);
        controller->SetBool("IsWalking", false);
        controller->SetBool("IsRunning", false);
        controller->SetBool("IsCrouching", false);
        controller->SetBool("IsInCombat", false);
        controller->SetBool("IsDead", false);

        // Set normalized speed for blend trees
        controller->SetFloat("NormalizedSpeed", 0.0f);
        
        // Set directional parameters
        controller->SetFloat("ForwardDot", 1.0f);
        controller->SetFloat("RightDot", 0.0f);

        // Start with idle animation if available
        try {
            controller->Play(ANIM_IDLE, 0.0f);
            LOG_INFO("Started XBot idle animation");
        } catch (const std::exception& e) {
            LOG_WARNING("Could not start idle animation: " + std::string(e.what()));
        }

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

    void XBotCharacter::UpdateMovementAnimationParameters() {
        auto* movementComponent = GetMovementComponent();
        if (!movementComponent) {
            return;
        }

        // Get current movement state
        glm::vec3 velocity = movementComponent->GetVelocity();
        float horizontalSpeed = glm::length(glm::vec3(velocity.x, 0.0f, velocity.z));
        bool isGrounded = movementComponent->IsGrounded();
        bool isJumping = movementComponent->IsJumping();
        bool isFalling = !isGrounded && velocity.y < -0.1f;

        // Smooth speed changes to avoid animation jitter
        float targetSpeed = horizontalSpeed;
        m_currentSpeed = glm::mix(m_currentSpeed, targetSpeed, SPEED_CHANGE_SMOOTHING * 0.016f); // Assuming ~60 FPS

        // Store previous state for change detection
        m_previousSpeed = m_currentSpeed;
        m_wasGrounded = isGrounded;
        m_wasJumping = isJumping;
        m_wasFalling = isFalling;
    }

    void XBotCharacter::SynchronizeXBotAnimationWithMovement() {
        auto* controller = GetAnimationController();
        if (!controller) {
            return;
        }

        auto* movementComponent = GetMovementComponent();
        if (!movementComponent) {
            return;
        }

        // Get current movement state
        glm::vec3 velocity = movementComponent->GetVelocity();
        bool isGrounded = movementComponent->IsGrounded();
        bool isJumping = movementComponent->IsJumping();
        bool isFalling = !isGrounded && velocity.y < -0.1f;

        // Set basic movement parameters
        controller->SetFloat("Speed", m_currentSpeed);
        controller->SetBool("IsGrounded", isGrounded);
        controller->SetBool("IsJumping", isJumping);
        controller->SetBool("IsFalling", isFalling);

        // Set XBot-specific movement type parameters
        bool isIdle = m_currentSpeed < 0.1f && isGrounded;
        bool isWalking = m_currentSpeed >= WALK_SPEED_THRESHOLD && m_currentSpeed < RUN_SPEED_THRESHOLD && isGrounded;
        bool isRunning = m_currentSpeed >= RUN_SPEED_THRESHOLD && isGrounded;

        controller->SetBool("IsIdle", isIdle);
        controller->SetBool("IsWalking", isWalking);
        controller->SetBool("IsRunning", isRunning);

        // Set XBot-specific state parameters
        controller->SetBool("IsCrouching", m_isCrouching);
        controller->SetBool("IsInCombat", m_isInCombat);
        controller->SetBool("IsDead", m_isDead);

        // Ground detection for animation state changes
        if (!m_wasGrounded && isGrounded) {
            // Just landed
            controller->SetTrigger("OnLanded");
            LOG_DEBUG("XBot landed - triggering OnLanded event");
        }

        if (m_wasGrounded && !isGrounded) {
            // Just left ground
            controller->SetTrigger("OnLeftGround");
            LOG_DEBUG("XBot left ground - triggering OnLeftGround event");
        }

        // Speed-based state change detection
        std::string newMovementState = "Idle";
        if (isJumping) {
            newMovementState = "Jump";
        } else if (isFalling) {
            newMovementState = "Fall";
        } else if (m_isCrouching && isWalking) {
            newMovementState = "CrouchWalk";
        } else if (isRunning) {
            newMovementState = "Run";
        } else if (isWalking) {
            newMovementState = "Walk";
        }

        // Log state changes for debugging
        if (newMovementState != m_previousMovementState) {
            LOG_DEBUG("XBot movement state changed from " + m_previousMovementState + " to " + newMovementState + 
                     " (Speed: " + std::to_string(m_currentSpeed) + ")");
            m_previousMovementState = newMovementState;
        }

        // Set normalized speed parameter for blend trees (0-1 range)
        float normalizedSpeed = 0.0f;
        if (m_currentSpeed > 0.1f) {
            // Map speed to 0-1 range: 0 = idle, 0.5 = walk threshold, 1.0 = run threshold
            normalizedSpeed = glm::clamp(m_currentSpeed / RUN_SPEED_THRESHOLD, 0.0f, 1.0f);
        }
        controller->SetFloat("NormalizedSpeed", normalizedSpeed);

        // Set directional movement parameters for turning animations
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f); // Character forward direction
        glm::vec3 velocityDirection = glm::normalize(glm::vec3(velocity.x, 0.0f, velocity.z));
        
        if (glm::length(velocityDirection) > 0.1f) {
            float dotProduct = glm::dot(forward, velocityDirection);
            float crossProduct = glm::cross(forward, velocityDirection).y;
            
            controller->SetFloat("ForwardDot", dotProduct);
            controller->SetFloat("RightDot", crossProduct);
            
            // Trigger turn animations based on direction change
            if (crossProduct > 0.7f && m_currentSpeed > WALK_SPEED_THRESHOLD) {
                controller->SetTrigger("TurnRight");
            } else if (crossProduct < -0.7f && m_currentSpeed > WALK_SPEED_THRESHOLD) {
                controller->SetTrigger("TurnLeft");
            }
        } else {
            controller->SetFloat("ForwardDot", 1.0f);
            controller->SetFloat("RightDot", 0.0f);
        }
    }

    void XBotCharacter::Render(GameEngine::PrimitiveRenderer* renderer) {
        // Use the base Character render method which handles FBX models and animation
        Character::Render(renderer);
    }

} // namespace GameExample