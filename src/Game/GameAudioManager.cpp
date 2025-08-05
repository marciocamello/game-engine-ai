#include "Game/GameAudioManager.h"
#include "Audio/AudioEngine.h"
#include "Game/Character.h"
#include "Core/Logger.h"

namespace GameEngine {

    GameAudioManager::GameAudioManager() {
        // Initialize with default configuration
    }

    GameAudioManager::~GameAudioManager() {
        Cleanup();
    }

    bool GameAudioManager::Initialize(AudioEngine* audioEngine) {
        if (!audioEngine) {
            LOG_ERROR("GameAudioManager: AudioEngine is null");
            return false;
        }

        m_audioEngine = audioEngine;

        if (!m_audioEngine->IsAudioAvailable()) {
            LOG_WARNING("GameAudioManager: Audio not available, initializing in silent mode");
            m_initialized = true;
            return true; // Still return true to allow game to continue
        }

        // Load audio clips
        LoadAudioClips();

        // Create audio sources
        CreateAudioSources();

        // Start background music
        PlayBackgroundMusic();

        m_initialized = true;
        LOG_INFO("GameAudioManager initialized successfully");
        return true;
    }

    void GameAudioManager::Update(float deltaTime, const Character* character) {
        if (!m_initialized || !m_audioEngine || !m_audioEngine->IsAudioAvailable()) {
            return;
        }

        if (!character) {
            return;
        }

        // Update listener position to follow character
        UpdateListenerPosition(character);

        // Update footstep audio based on character movement
        UpdateFootstepAudio(deltaTime, character);

        // Check for jump state changes
        bool currentJumpingState = character->IsJumping();
        
        // Reduced logging for performance - only log actual jump events
        // Debug log removed to improve performance
        
        if (currentJumpingState && !m_previousJumpingState) {
            // Character just started jumping
            PlayJumpSound();
        }
        m_previousJumpingState = currentJumpingState;

        // Update previous character position and state for next frame
        m_previousCharacterPosition = character->GetPosition();
        m_previousGroundedState = character->IsGrounded();
        
        // Debug logging removed for performance optimization
    }

    void GameAudioManager::Cleanup() {
        if (!m_audioEngine || !m_initialized) {
            return;
        }

        // Stop all audio sources
        if (m_backgroundMusicSource != 0) {
            m_audioEngine->StopAudioSource(m_backgroundMusicSource);
            m_audioEngine->DestroyAudioSource(m_backgroundMusicSource);
            m_backgroundMusicSource = 0;
        }

        if (m_footstepSource != 0) {
            m_audioEngine->StopAudioSource(m_footstepSource);
            m_audioEngine->DestroyAudioSource(m_footstepSource);
            m_footstepSource = 0;
        }

        if (m_jumpSource != 0) {
            m_audioEngine->StopAudioSource(m_jumpSource);
            m_audioEngine->DestroyAudioSource(m_jumpSource);
            m_jumpSource = 0;
        }

        // Unload audio clips
        if (m_backgroundMusic) {
            m_audioEngine->UnloadAudioClip(m_config.backgroundMusicPath);
            m_backgroundMusic.reset();
        }

        if (m_footstepSound) {
            m_audioEngine->UnloadAudioClip(m_config.footstepSoundPath);
            m_footstepSound.reset();
        }

        if (m_jumpSound) {
            m_audioEngine->UnloadAudioClip(m_config.jumpSoundPath);
            m_jumpSound.reset();
        }

        m_backgroundMusicPlaying = false;
        m_initialized = false;
        m_audioEngine = nullptr;

        LOG_INFO("GameAudioManager cleaned up");
    }

    void GameAudioManager::PlayJumpSound() {
        if (!m_audioEngine || !m_audioEngine->IsAudioAvailable() || !m_jumpSound || m_jumpSource == 0) {
            return;
        }

        // Set jump sound position to character position (will be updated in UpdateListenerPosition)
        m_audioEngine->PlayAudioSource(m_jumpSource, m_jumpSound);
        LOG_INFO("GameAudioManager: Playing jump sound");
    }

    void GameAudioManager::SetWalkingState(bool isWalking) {
        m_isWalking = isWalking;
        if (!isWalking) {
            // Reset footstep timer when stopping
            m_footstepTimer = 0.0f;
        }
    }

    void GameAudioManager::PlayBackgroundMusic() {
        if (!m_audioEngine || !m_audioEngine->IsAudioAvailable() || !m_backgroundMusic || m_backgroundMusicSource == 0) {
            return;
        }

        if (m_backgroundMusicPlaying) {
            return; // Already playing
        }

        m_audioEngine->SetAudioSourceLooping(m_backgroundMusicSource, m_config.backgroundMusicLoop);
        m_audioEngine->SetAudioSourceVolume(m_backgroundMusicSource, m_config.backgroundMusicVolume);
        m_audioEngine->PlayAudioSource(m_backgroundMusicSource, m_backgroundMusic);
        m_backgroundMusicPlaying = true;

        LOG_INFO("GameAudioManager: Started background music");
    }

    void GameAudioManager::StopBackgroundMusic() {
        if (!m_audioEngine || m_backgroundMusicSource == 0) {
            return;
        }

        m_audioEngine->StopAudioSource(m_backgroundMusicSource);
        m_backgroundMusicPlaying = false;

        LOG_INFO("GameAudioManager: Stopped background music");
    }

    void GameAudioManager::SetAudioConfiguration(const AudioConfiguration& config) {
        m_config = config;
        
        if (m_initialized) {
            // Reload audio clips with new configuration
            Cleanup();
            Initialize(m_audioEngine);
        }
    }

    void GameAudioManager::SetMasterVolume(float volume) {
        if (m_audioEngine) {
            m_audioEngine->SetMasterVolume(volume);
        }
    }

    void GameAudioManager::SetMusicVolume(float volume) {
        if (m_audioEngine) {
            m_audioEngine->SetMusicVolume(volume);
        }
    }

    void GameAudioManager::SetSFXVolume(float volume) {
        if (m_audioEngine) {
            m_audioEngine->SetSFXVolume(volume);
        }
    }

    bool GameAudioManager::IsAudioAvailable() const {
        return m_audioEngine && m_audioEngine->IsAudioAvailable();
    }

    void GameAudioManager::LoadAudioClips() {
        if (!m_audioEngine) {
            return;
        }

        // Load background music
        m_backgroundMusic = m_audioEngine->LoadAudioClip(m_config.backgroundMusicPath);
        if (!m_backgroundMusic) {
            LOG_WARNING("GameAudioManager: Failed to load background music: " + m_config.backgroundMusicPath);
        } else {
            LOG_INFO("GameAudioManager: Loaded background music: " + m_config.backgroundMusicPath);
        }

        // Load footstep sound
        m_footstepSound = m_audioEngine->LoadAudioClip(m_config.footstepSoundPath);
        if (!m_footstepSound) {
            LOG_WARNING("GameAudioManager: Failed to load footstep sound: " + m_config.footstepSoundPath);
        } else {
            LOG_INFO("GameAudioManager: Loaded footstep sound: " + m_config.footstepSoundPath);
        }

        // Load jump sound
        m_jumpSound = m_audioEngine->LoadAudioClip(m_config.jumpSoundPath);
        if (!m_jumpSound) {
            LOG_WARNING("GameAudioManager: Failed to load jump sound: " + m_config.jumpSoundPath);
        } else {
            LOG_INFO("GameAudioManager: Loaded jump sound: " + m_config.jumpSoundPath);
        }
    }

    void GameAudioManager::CreateAudioSources() {
        if (!m_audioEngine) {
            return;
        }

        // Create background music source
        m_backgroundMusicSource = m_audioEngine->CreateAudioSource();
        if (m_backgroundMusicSource == 0) {
            LOG_ERROR("GameAudioManager: Failed to create background music source");
        }

        // Create footstep source
        m_footstepSource = m_audioEngine->CreateAudioSource();
        if (m_footstepSource == 0) {
            LOG_ERROR("GameAudioManager: Failed to create footstep source");
        } else {
            // Configure footstep source
            m_audioEngine->SetAudioSourceVolume(m_footstepSource, m_config.soundEffectVolume);
        }

        // Create jump source
        m_jumpSource = m_audioEngine->CreateAudioSource();
        if (m_jumpSource == 0) {
            LOG_ERROR("GameAudioManager: Failed to create jump source");
        } else {
            // Configure jump source
            m_audioEngine->SetAudioSourceVolume(m_jumpSource, m_config.soundEffectVolume);
        }

        LOG_INFO("GameAudioManager: Created audio sources");
    }

    void GameAudioManager::UpdateFootstepAudio(float deltaTime, const Character* character) {
        if (!character || !m_audioEngine || !m_footstepSound || m_footstepSource == 0) {
            return;
        }

        // Check if character is moving and grounded
        Math::Vec3 currentPosition = character->GetPosition();
        Math::Vec3 velocity = character->GetVelocity();
        bool isGrounded = character->IsGrounded();
        
        // Check movement using both velocity and position change
        float velocityMagnitude = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
        bool isMovingByVelocity = velocityMagnitude > 0.05f; // Lower threshold
        
        // Check movement by position change (for movement components that don't report velocity correctly)
        Math::Vec3 positionDelta = currentPosition - m_previousCharacterPosition;
        float positionChange = sqrt(positionDelta.x * positionDelta.x + positionDelta.z * positionDelta.z);
        bool isMovingByPosition = positionChange > 0.001f; // Very sensitive
        
        bool isMoving = isMovingByVelocity || isMovingByPosition;
        
        // Debug logging removed for performance optimization

        // Update walking state
        bool shouldPlayFootsteps = isGrounded && isMoving && !character->IsJumping();
        
        // If we were walking but now we should stop, reset the timer and stop any playing sound
        if (m_isWalking && !shouldPlayFootsteps) {
            m_audioEngine->StopAudioSource(m_footstepSource);
            m_footstepTimer = 0.0f;
            // Debug logging removed for performance
        }
        
        SetWalkingState(shouldPlayFootsteps);

        if (m_isWalking) {
            m_footstepTimer += deltaTime;

            if (m_footstepTimer >= m_config.footstepInterval) {
                // Play footstep sound at character position
                m_audioEngine->SetAudioSourcePosition(m_footstepSource, currentPosition);
                m_audioEngine->PlayAudioSource(m_footstepSource, m_footstepSound);
                m_footstepTimer = 0.0f;
                // Debug logging removed for performance
            }
        }
    }

    void GameAudioManager::UpdateListenerPosition(const Character* character) {
        if (!character || !m_audioEngine) {
            return;
        }

        // Set listener position to character position
        Math::Vec3 characterPosition = character->GetPosition();
        m_audioEngine->SetListenerPosition(characterPosition);

        // Set listener orientation (facing forward)
        Math::Vec3 forward(0.0f, 0.0f, -1.0f); // Default forward direction
        Math::Vec3 up(0.0f, 1.0f, 0.0f);       // Default up direction
        m_audioEngine->SetListenerOrientation(forward, up);

        // Update audio source positions for 3D audio
        if (m_jumpSource != 0) {
            m_audioEngine->SetAudioSourcePosition(m_jumpSource, characterPosition);
        }
    }

    void GameAudioManager::OnCharacterTypeChanged() {
        if (!m_audioEngine || !m_initialized) {
            return;
        }

        // Stop any currently playing footstep sounds to avoid duplicates
        if (m_footstepSource != 0) {
            m_audioEngine->StopAudioSource(m_footstepSource);
        }

        // Stop any currently playing jump sounds to avoid duplicates
        if (m_jumpSource != 0) {
            m_audioEngine->StopAudioSource(m_jumpSource);
        }

        // Reset walking state and timer - this ensures fresh start for new character type
        m_isWalking = false;
        m_footstepTimer = 0.0f;

        // Reset previous states for change detection - this ensures audio works for all character types
        m_previousCharacterPosition = Math::Vec3(0.0f);
        m_previousGroundedState = false;
        m_previousJumpingState = false;

        // Note: Background music continues playing - don't stop it
        // Audio sources remain active and ready for the new character type
        LOG_INFO("GameAudioManager: Character type changed, reset player audio state (background music continues, audio ready for new type)");
    }

}