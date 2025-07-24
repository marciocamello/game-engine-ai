#include "Game/Character.h"
#include "Game/MovementComponentFactory.h"
#include "Game/ThirdPersonCameraSystem.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsEngine.h"
#include "Audio/AudioEngine.h"
#include "Core/Logger.h"

namespace GameEngine {
    Character::Character() 
        : m_color(0.3f, 0.5f, 1.0f, 1.0f) // Azul para Character
    {
    }

    Character::~Character() {
        // Movement component cleanup is handled automatically by unique_ptr
    }

    bool Character::Initialize(PhysicsEngine* physicsEngine, AudioEngine* audioEngine) {
        m_physicsEngine = physicsEngine;
        m_audioEngine = audioEngine;
        
        // Initialize default movement component (DeterministicMovementComponent)
        InitializeDefaultMovementComponent(physicsEngine);
        
        if (!m_movementComponent) {
            LOG_ERROR("Failed to initialize movement component for Character");
            return false;
        }
        
        // Initialize audio system
        InitializeAudio(audioEngine);
        
        LOG_INFO("Character initialized with component-based movement system (" + 
                std::string(GetMovementTypeName()) + ")");
        return true;
    }

    void Character::Update(float deltaTime, InputManager* input, ThirdPersonCameraSystem* camera) {
        if (m_movementComponent) {
            // Store previous state for audio event detection
            bool wasGrounded = m_wasGrounded;
            bool wasJumping = m_wasJumping;
            
            // Update movement
            m_movementComponent->Update(deltaTime, input, camera);
            
            // Update current state
            m_wasGrounded = IsGrounded();
            m_wasJumping = IsJumping();
            
            // Update audio based on movement state changes
            UpdateAudio(deltaTime);
            
            // Detect jump event (transition from grounded to jumping)
            if (wasGrounded && !wasJumping && m_wasJumping) {
                PlayJumpSound();
                LOG_DEBUG("Jump detected: wasGrounded=" + std::to_string(wasGrounded) + 
                         ", wasJumping=" + std::to_string(wasJumping) + 
                         ", isJumping=" + std::to_string(m_wasJumping));
            }
        }
    }

    void Character::Render(PrimitiveRenderer* renderer) {
        if (!renderer) return;

        // Get color based on current movement component type
        Math::Vec4 currentColor = GetMovementTypeColor();
        
        // Draw character as a simple cube (easier to see movement)
        Math::Vec3 cubeSize(m_radius * 2, m_height, m_radius * 2);
        renderer->DrawCube(GetPosition(), cubeSize, currentColor);
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

    void Character::InitializeDefaultMovementComponent(PhysicsEngine* physicsEngine) {
        // Use HybridMovementComponent by default for third-person games (best balance)
        m_movementComponent = MovementComponentFactory::CreateComponent(MovementComponentFactory::ComponentType::Hybrid);
        
        if (m_movementComponent && physicsEngine) {
            m_movementComponent->Initialize(physicsEngine);
            m_movementComponent->SetCharacterSize(m_radius, m_height);
        }
    }

    void Character::InitializeAudio(AudioEngine* audioEngine) {
        if (!audioEngine || !m_audioEnabled) {
            LOG_INFO("Character audio disabled or AudioEngine not available");
            return;
        }

        // Create audio sources for character sounds
        m_jumpAudioSource = audioEngine->CreateAudioSource();
        m_footstepAudioSource = audioEngine->CreateAudioSource();

        // Load audio clips
        m_jumpSound = audioEngine->LoadAudioClip("assets/audio/file_example_WAV_5MG.wav");
        m_footstepSound = audioEngine->LoadAudioClip("assets/audio/file_example_OOG_1MG.ogg");

        if (!m_jumpSound) {
            LOG_WARNING("Failed to load jump sound for Character");
        }
        if (!m_footstepSound) {
            LOG_WARNING("Failed to load footstep sound for Character");
        }

        // Configure audio sources for 3D positioning
        if (m_jumpAudioSource != 0) {
            audioEngine->SetAudioSourceVolume(m_jumpAudioSource, 0.7f);
            audioEngine->SetAudioSourcePitch(m_jumpAudioSource, 1.2f); // Slightly higher pitch for jump
        }
        
        if (m_footstepAudioSource != 0) {
            audioEngine->SetAudioSourceVolume(m_footstepAudioSource, 0.4f);
            audioEngine->SetAudioSourcePitch(m_footstepAudioSource, 0.9f); // Slightly lower pitch for footsteps
        }

        LOG_INFO("Character audio initialized with jump and footstep sounds");
    }

    void Character::UpdateAudio(float deltaTime) {
        if (!m_audioEngine || !m_audioEnabled) {
            return;
        }

        // Update 3D audio positions for all character audio sources
        Math::Vec3 characterPosition = GetPosition();
        
        if (m_jumpAudioSource != 0) {
            m_audioEngine->SetAudioSourcePosition(m_jumpAudioSource, characterPosition);
        }
        
        if (m_footstepAudioSource != 0) {
            m_audioEngine->SetAudioSourcePosition(m_footstepAudioSource, characterPosition);
        }

        // Update footstep audio
        UpdateFootsteps(deltaTime);
    }

    void Character::PlayJumpSound() {
        if (!m_audioEngine || !m_audioEnabled || m_jumpAudioSource == 0 || !m_jumpSound) {
            return;
        }

        // Stop any currently playing jump sound
        m_audioEngine->StopAudioSource(m_jumpAudioSource);
        
        // Play jump sound at character position
        m_audioEngine->PlayAudioSource(m_jumpAudioSource, m_jumpSound);
        
        LOG_DEBUG("Character played jump sound at position (" + 
                 std::to_string(GetPosition().x) + ", " + 
                 std::to_string(GetPosition().y) + ", " + 
                 std::to_string(GetPosition().z) + ")");
    }

    void Character::UpdateFootsteps(float deltaTime) {
        if (!m_audioEngine || !m_audioEnabled || m_footstepAudioSource == 0 || !m_footstepSound) {
            return;
        }

        // Only play footsteps when grounded and moving
        if (!IsGrounded() || IsJumping()) {
            return;
        }

        Math::Vec3 currentVelocity = GetVelocity();
        Math::Vec3 horizontalVelocity(currentVelocity.x, 0.0f, currentVelocity.z);
        float speed = glm::length(horizontalVelocity);

        // Only play footsteps if moving fast enough
        const float minSpeedForFootsteps = 1.0f; // m/s
        if (speed < minSpeedForFootsteps) {
            return;
        }

        // Update footstep timer
        m_footstepTimer += deltaTime;

        // Calculate dynamic footstep interval based on movement speed
        float dynamicInterval = m_footstepInterval * (6.0f / glm::max(speed, 1.0f)); // Faster movement = faster footsteps
        dynamicInterval = Math::Clamp(dynamicInterval, 0.2f, 1.0f); // Clamp between 0.2s and 1.0s

        // Check if enough time has passed and character has moved enough distance
        Math::Vec3 currentPosition = GetPosition();
        float distanceTraveled = glm::length(currentPosition - m_lastFootstepPosition);

        if (m_footstepTimer >= dynamicInterval && distanceTraveled >= m_footstepMinDistance) {
            // Play footstep sound
            m_audioEngine->StopAudioSource(m_footstepAudioSource); // Stop previous footstep if still playing
            m_audioEngine->PlayAudioSource(m_footstepAudioSource, m_footstepSound);
            
            // Reset timer and position
            m_footstepTimer = 0.0f;
            m_lastFootstepPosition = currentPosition;
            
            LOG_DEBUG("Character played footstep sound at speed " + std::to_string(speed) + " m/s");
        }
    }
}