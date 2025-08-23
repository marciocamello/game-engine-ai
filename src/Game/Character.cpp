#include "Game/Character.h"
#include "Game/MovementComponentFactory.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/Model.h"
#include "Resource/ModelLoader.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationImporter.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/AnimationTransition.h"

#include "Core/Logger.h"

namespace GameEngine {
    Character::Character() 
        : m_color(0.3f, 0.5f, 1.0f, 1.0f) // Blue for Character
    {
        // Initialize model loader
        m_modelLoader = std::make_unique<ModelLoader>();
        
        // Initialize animation system components
        m_animationController = std::make_unique<Animation::AnimationController>();
        m_animationImporter = std::make_unique<Animation::AnimationImporter>();
    }

    Character::~Character() {
        // Shutdown animation system
        ShutdownAnimationSystem();
        
        // Movement component cleanup is handled automatically by unique_ptr
    }

    bool Character::Initialize(PhysicsEngine* physicsEngine) {
        m_physicsEngine = physicsEngine;
        
        // Initialize model loader
        if (!m_modelLoader->Initialize()) {
            LOG_WARNING("Failed to initialize model loader - FBX models will not be available");
        }
        
        // Initialize default movement component (HybridMovementComponent)
        InitializeDefaultMovementComponent(physicsEngine);
        
        if (!m_movementComponent) {
            LOG_ERROR("Failed to initialize movement component for Character");
            return false;
        }
        
        // Initialize animation system
        if (!InitializeAnimationSystem()) {
            LOG_WARNING("Failed to initialize animation system - animations will not be available");
        }
        
        LOG_INFO("Character initialized with component-based movement system (" + 
                std::string(GetMovementTypeName()) + ")");
        return true;
    }

    void Character::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        if (m_movementComponent) {
            // Update movement
            m_movementComponent->Update(deltaTime, input, camera);
        }
        
        // Update animation system
        UpdateAnimationState(deltaTime);
    }

    void Character::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Get color based on current movement component type
        Math::Vec4 currentColor = GetMovementTypeColor();
        
        if (IsUsingFBXModel()) {
            // Render FBX model meshes with rotation and offset
            Math::Vec3 basePosition = GetPosition();
            Math::Vec3 scale(m_modelScale, m_modelScale, m_modelScale);
            
            // Create rotation quaternion from yaw angle
            float yawRadians = GetRotation() * Math::DEG_TO_RAD;
            Math::Quat rotation = Math::Quat(cos(yawRadians * 0.5f), 0.0f, sin(yawRadians * 0.5f), 0.0f);
            
            // Apply model offset to position
            // The offset is applied in world space for simplicity
            Math::Vec3 offsetPosition = basePosition + m_modelOffset;
            
            // Get bone matrices for skinning if animation system is available
            std::vector<Math::Mat4> boneMatrices;
            if (m_animationSystemInitialized && m_animationController) {
                m_animationController->Evaluate(boneMatrices);
                LOG_DEBUG("Evaluated " + std::to_string(boneMatrices.size()) + " bone matrices for skinning");
            }
            
            // Render all meshes from the FBX model
            auto meshes = m_fbxModel->GetMeshes();
            for (const auto& mesh : meshes) {
                if (mesh) {
                    // Use skinned rendering if bone matrices are available
                    if (!boneMatrices.empty()) {
                        renderer->DrawSkinnedMesh(mesh, offsetPosition, rotation, scale, boneMatrices, currentColor);
                        LOG_DEBUG("Rendered skinned mesh with " + std::to_string(boneMatrices.size()) + " bone matrices");
                    } else {
                        // Fallback to regular mesh rendering
                        renderer->DrawMesh(mesh, offsetPosition, rotation, scale, currentColor);
                        LOG_DEBUG("Rendered mesh without skinning (no bone matrices available)");
                    }
                }
            }
            
            LOG_DEBUG("Rendered FBX model with " + std::to_string(meshes.size()) + " meshes at rotation " + std::to_string(GetRotation()) + " degrees, offset (" + 
                     std::to_string(m_modelOffset.x) + ", " + std::to_string(m_modelOffset.y) + ", " + std::to_string(m_modelOffset.z) + ")");
        } else {
            // Draw character as a capsule (fallback when no FBX model) - matches physics collision shape
            renderer->DrawCapsule(GetPosition(), m_radius, m_height, currentColor);
        }

        // Render debug collision capsule if enabled
        if (m_showDebugCapsule) {
            Math::Vec4 debugColor(1.0f, 0.0f, 0.0f, 0.5f); // Semi-transparent red
            renderer->DrawCapsule(GetPosition(), m_radius, m_height, debugColor);
        }
    }

    // Transform delegation
    void Character::SetPosition(const Math::Vec3& position) {
        if (m_movementComponent) {
            m_movementComponent->SetPosition(position);
        }
    }

    const Math::Vec3& Character::GetPosition() const {
        if (m_movementComponent) {
            return m_movementComponent->GetPosition();
        }
        static Math::Vec3 defaultPos(0.0f, 0.9f, 0.0f);
        return defaultPos;
    }

    void Character::SetRotation(float yaw) {
        if (m_movementComponent) {
            m_movementComponent->SetRotation(yaw);
        }
    }

    float Character::GetRotation() const {
        if (m_movementComponent) {
            return m_movementComponent->GetRotation();
        }
        return 0.0f;
    }

    // Movement delegation
    void Character::SetMoveSpeed(float speed) {
        if (m_movementComponent) {
            auto config = m_movementComponent->GetMovementConfig();
            config.maxWalkSpeed = speed;
            m_movementComponent->SetMovementConfig(config);
        }
    }

    float Character::GetMoveSpeed() const {
        if (m_movementComponent) {
            return m_movementComponent->GetMovementConfig().maxWalkSpeed;
        }
        return 6.0f; // Default speed
    }

    const Math::Vec3& Character::GetVelocity() const {
        if (m_movementComponent) {
            return m_movementComponent->GetVelocity();
        }
        static Math::Vec3 defaultVel(0.0f);
        return defaultVel;
    }

    void Character::SetCharacterSize(float radius, float height) {
        m_radius = radius;
        m_height = height;
        if (m_movementComponent) {
            m_movementComponent->SetCharacterSize(radius, height);
        }
    }

    // Movement state queries
    bool Character::IsGrounded() const {
        if (m_movementComponent) {
            return m_movementComponent->IsGrounded();
        }
        return false;
    }

    bool Character::IsJumping() const {
        if (m_movementComponent) {
            return m_movementComponent->IsJumping();
        }
        return false;
    }

    bool Character::IsFalling() const {
        if (m_movementComponent) {
            return m_movementComponent->IsFalling();
        }
        return false;
    }

    // Movement component management
    void Character::SetMovementComponent(std::unique_ptr<CharacterMovementComponent> component) {
        if (component) {
            // Preserve current state
            Math::Vec3 currentPosition(0.0f, 0.9f, 0.0f);
            Math::Vec3 currentVelocity(0.0f);
            float currentRotation = 0.0f;
            
            if (m_movementComponent) {
                currentPosition = m_movementComponent->GetPosition();
                currentVelocity = m_movementComponent->GetVelocity();
                currentRotation = m_movementComponent->GetRotation();
                
                // Shutdown old component
                m_movementComponent->Shutdown();
            }
            
            // Set new component
            m_movementComponent = std::move(component);
            
            // Initialize new component
            if (m_physicsEngine) {
                m_movementComponent->Initialize(m_physicsEngine);
            }
            
            // Configure character size
            m_movementComponent->SetCharacterSize(m_radius, m_height);
            
            // Restore state to new component
            m_movementComponent->SetPosition(currentPosition);
            m_movementComponent->SetVelocity(currentVelocity);
            m_movementComponent->SetRotation(currentRotation);
            
            LOG_INFO("Character switched to " + std::string(GetMovementTypeName()) + 
                    " at position (" + std::to_string(currentPosition.x) + ", " + 
                    std::to_string(currentPosition.y) + ", " + std::to_string(currentPosition.z) + ")");
        }
    }

    void Character::SwitchToCharacterMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::CharacterMovement);
        SetMovementComponent(std::move(component));
    }

    void Character::SwitchToPhysicsMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Physics);
        SetMovementComponent(std::move(component));
    }

    void Character::SwitchToHybridMovement() {
        auto component = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        SetMovementComponent(std::move(component));
    }

    const char* Character::GetMovementTypeName() const {
        if (m_movementComponent) {
            return m_movementComponent->GetComponentTypeName();
        }
        return "NoMovementComponent";
    }

    Math::Vec4 Character::GetMovementTypeColor() const {
        if (!m_movementComponent) {
            return Math::Vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray for no component
        }
        
        const char* typeName = m_movementComponent->GetComponentTypeName();
        
        // Character colors (blue tones) - simplified to 3 components
        if (strcmp(typeName, "CharacterMovementComponent") == 0) {
            return Math::Vec4(0.2f, 0.4f, 1.0f, 1.0f); // Bright blue for basic movement
        }
        else if (strcmp(typeName, "HybridMovementComponent") == 0) {
            return Math::Vec4(0.0f, 0.8f, 1.0f, 1.0f); // Cyan for hybrid (recommended)
        }
        else if (strcmp(typeName, "PhysicsMovementComponent") == 0) {
            return Math::Vec4(0.0f, 0.2f, 0.8f, 1.0f); // Dark blue for physics
        }
        
        return Math::Vec4(0.2f, 0.6f, 1.0f, 1.0f); // Default blue
    }



    bool Character::HasFallen() const {
        return GetPosition().y < m_fallLimit;
    }

    void Character::ResetToSpawnPosition() {
        if (m_movementComponent) {
            // Reset position to spawn point
            m_movementComponent->SetPosition(m_spawnPosition);
            
            // Reset velocity to zero to stop any falling motion
            m_movementComponent->SetVelocity(Math::Vec3(0.0f));
            
            // Reset rotation to default
            m_movementComponent->SetRotation(0.0f);
            
            LOG_INFO("Character reset to spawn position: (" + 
                    std::to_string(m_spawnPosition.x) + ", " + 
                    std::to_string(m_spawnPosition.y) + ", " + 
                    std::to_string(m_spawnPosition.z) + ")");
        }
    }

    bool Character::LoadFBXModel(const std::string& fbxPath) {
        if (!m_modelLoader || !m_modelLoader->IsInitialized()) {
            LOG_ERROR("Model loader not initialized, cannot load FBX model: " + fbxPath);
            return false;
        }

        LOG_INFO("Loading FBX model: " + fbxPath);
        
        try {
            // Load the FBX model
            auto loadResult = m_modelLoader->LoadModel(fbxPath);
            
            if (!loadResult.success) {
                LOG_ERROR("Failed to load FBX model '" + fbxPath + "': " + loadResult.errorMessage);
                return false;
            }
            
            // Create a Model resource from the loaded meshes
            m_fbxModel = std::make_shared<Model>(fbxPath);
            m_fbxModel->SetMeshes(loadResult.meshes);
            
            // Mixamo models come in standard game character size, so use appropriate scale
            m_modelScale = 1.0f; // Start with 1:1 scale for Mixamo models
            
            // Set default model offset to center FBX model within physics capsule
            // Physics capsule: radius=0.3f, height=1.8f, centered at character position
            // Most FBX models need downward offset to align feet with capsule bottom
            m_modelOffset = ModelOffsetConfiguration::CenteredInCapsule().offset;
            
            m_useFBXModel = true;
            
            LOG_INFO("Successfully loaded FBX model '" + fbxPath + "' with " + 
                    std::to_string(loadResult.meshes.size()) + " meshes, " +
                    std::to_string(loadResult.totalVertices) + " vertices, " +
                    std::to_string(loadResult.totalTriangles) + " triangles");
            
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while loading FBX model '" + fbxPath + "': " + std::string(e.what()));
            m_fbxModel.reset();
            m_useFBXModel = false;
            return false;
        }
    }

    void Character::InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine) {
        // Use HybridMovementComponent by default for third-person games (best balance)
        m_movementComponent = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        if (m_movementComponent && physicsEngine) {
            m_movementComponent->Initialize(physicsEngine);
            m_movementComponent->SetCharacterSize(m_radius, m_height);
        }
    }

    // Animation system implementation
    bool Character::InitializeAnimationSystem() {
        if (!m_animationController || !m_animationImporter) {
            LOG_ERROR("Animation system components not initialized");
            return false;
        }

        // Load Xbot skeleton and animations
        if (!LoadXbotAnimations()) {
            LOG_WARNING("Failed to load Xbot animations - using fallback rendering");
            return false;
        }

        m_animationSystemInitialized = true;
        LOG_INFO("Animation system initialized successfully");
        return true;
    }

    void Character::ShutdownAnimationSystem() {
        if (m_animationController) {
            m_animationController->Shutdown();
        }
        
        m_xbotSkeleton.reset();
        m_animationSystemInitialized = false;
        
        LOG_INFO("Animation system shutdown complete");
    }

    bool Character::LoadXbotAnimations() {
        if (!m_animationImporter) {
            LOG_ERROR("Animation importer not available");
            return false;
        }

        // Import Xbot skeleton from the main FBX file
        LOG_INFO("Loading Xbot skeleton from XBot.fbx");
        auto skeletonResult = m_animationImporter->ImportFromFile("assets/meshes/XBot.fbx");
        
        if (!skeletonResult.success || !skeletonResult.skeleton) {
            LOG_ERROR("Failed to load Xbot skeleton: " + skeletonResult.errorMessage);
            return false;
        }

        m_xbotSkeleton = skeletonResult.skeleton;
        
        // Initialize animation controller with skeleton
        if (!m_animationController->Initialize(m_xbotSkeleton)) {
            LOG_ERROR("Failed to initialize animation controller with Xbot skeleton");
            return false;
        }

        // Load character-specific animations (to be overridden by derived classes)
        if (!LoadCharacterAnimations()) {
            LOG_WARNING("No character-specific animations loaded - using base character");
        }

        // Setup character-specific animation state machine (to be overridden by derived classes)
        SetupCharacterAnimationStateMachine();

        return true;
    }

    bool Character::LoadAnimationFromFBX(const std::string& fbxPath, const std::string& animationName) {
        if (!m_animationImporter || !m_xbotSkeleton) {
            return false;
        }

        try {
            // Import animations from FBX file using existing skeleton
            auto animations = m_animationImporter->ImportAnimationsFromFile(fbxPath, m_xbotSkeleton);
            
            if (animations.empty()) {
                LOG_WARNING("No animations found in " + fbxPath);
                return false;
            }

            // Use the first animation found (most FBX files contain one animation)
            auto animation = animations[0];
            if (!animation) {
                LOG_WARNING("Invalid animation in " + fbxPath);
                return false;
            }

            // Set animation name and add to controller
            animation->SetName(animationName);
            m_animationController->AddAnimation(animationName, animation);
            
            LOG_INFO("Loaded animation '" + animationName + "' from " + fbxPath + 
                    " (duration: " + std::to_string(animation->GetDuration()) + "s)");
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception loading animation from " + fbxPath + ": " + std::string(e.what()));
            return false;
        }
    }

    bool Character::LoadCharacterAnimations() {
        // Base implementation - no specific animations
        // Derived classes should override this method to load their specific animations
        LOG_INFO("Base Character class - no specific animations to load");
        return true;
    }

    void Character::SetupCharacterAnimationStateMachine() {
        if (!m_animationController) {
            return;
        }

        // Base implementation - create minimal state machine
        // Derived classes should override this method to create their specific state machines
        LOG_INFO("Base Character class - using minimal animation state machine");
        
        // Initialize basic animation parameters
        m_animationController->SetFloat("Speed", 0.0f);
        m_animationController->SetBool("IsGrounded", true);
        m_animationController->SetBool("IsJumping", false);
        m_animationController->SetBool("IsCrouching", false);
    }

    void Character::UpdateAnimationState(float deltaTime) {
        if (!m_animationSystemInitialized || !m_animationController) {
            return;
        }

        // Update animation controller
        m_animationController->Update(deltaTime);

        // Synchronize animation parameters with movement state
        SynchronizeAnimationWithMovement();
    }

    void Character::SynchronizeAnimationWithMovement() {
        if (!m_animationController || !m_movementComponent) {
            return;
        }

        // Get current movement state
        Math::Vec3 velocity = m_movementComponent->GetVelocity();
        float speed = glm::length(Math::Vec3(velocity.x, 0.0f, velocity.z)); // Horizontal speed only
        bool isGrounded = m_movementComponent->IsGrounded();
        bool isJumping = m_movementComponent->IsJumping();

        // Update animation parameters - the state machine will handle transitions automatically
        m_animationController->SetFloat("Speed", speed);
        m_animationController->SetBool("IsGrounded", isGrounded);
        m_animationController->SetBool("IsJumping", isJumping);

        // Get current state from state machine for tracking
        auto stateMachine = m_animationController->GetStateMachine();
        if (stateMachine) {
            std::string currentState = stateMachine->GetCurrentStateName();
            if (currentState != m_currentAnimationState) {
                m_currentAnimationState = currentState;
                LOG_DEBUG("Animation state changed to: " + currentState + " (Speed: " + std::to_string(speed) + ")");
            }
        }

        // Store previous state for next frame
        m_lastMovementSpeed = speed;
        m_wasGrounded = isGrounded;
        m_wasJumping = isJumping;
    }

    void Character::UpdateMovementAnimationParameters() {
        if (!m_animationController || !m_movementComponent) {
            return;
        }

        // This method can be extended for more complex parameter updates
        // Currently handled by SynchronizeAnimationWithMovement()
    }

    // Animation control methods
    void Character::PlayAnimation(const std::string& animationName, float fadeTime) {
        if (m_animationController) {
            m_animationController->Play(animationName, fadeTime);
        }
    }

    void Character::StopAnimation(const std::string& animationName, float fadeTime) {
        if (m_animationController) {
            m_animationController->Stop(animationName, fadeTime);
        }
    }

    void Character::SetAnimationSpeed(float speed) {
        if (m_animationController) {
            m_animationController->SetPlaybackSpeed(speed);
        }
    }

    float Character::GetAnimationSpeed() const {
        if (m_animationController) {
            return m_animationController->GetPlaybackSpeed();
        }
        return 1.0f;
    }

    void Character::SetAnimationParameter(const std::string& name, float value) {
        if (m_animationController) {
            m_animationController->SetFloat(name, value);
        }
    }

    void Character::SetAnimationParameter(const std::string& name, int value) {
        if (m_animationController) {
            m_animationController->SetInt(name, value);
        }
    }

    void Character::SetAnimationParameter(const std::string& name, bool value) {
        if (m_animationController) {
            m_animationController->SetBool(name, value);
        }
    }

    void Character::TriggerAnimationEvent(const std::string& name) {
        if (m_animationController) {
            m_animationController->SetTrigger(name);
        }
    }

}