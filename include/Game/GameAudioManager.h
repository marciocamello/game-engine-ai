#pragma once

#include "Core/Math.h"
#include <memory>
#include <string>

namespace GameEngine {
    class AudioEngine;
    class Character;
    struct AudioClip;

    /**
     * @brief Audio configuration structure for easy customization
     */
    struct AudioConfiguration {
        // Background Music
        std::string backgroundMusicPath = "assets/audio/file_example_WAV_5MG.wav";
        float backgroundMusicVolume = 0.3f;
        bool backgroundMusicLoop = true;

        // Sound Effects
        std::string jumpSoundPath = "assets/audio/cartoon-jump.wav";
        std::string footstepSoundPath = "assets/audio/concrete-footsteps.wav";
        float soundEffectVolume = 0.7f;

        // 3D Audio Settings
        float maxAudioDistance = 50.0f;
        float referenceDistance = 1.0f;
        float rolloffFactor = 1.0f;

        // Footstep timing
        float footstepInterval = 0.2f; // Time between footsteps in seconds (very responsive)
    };

    /**
     * @brief GameAudioManager component for managing game audio
     * 
     * Handles background music, footstep sounds, jump sounds, and other game audio.
     * Provides centralized audio management with proper resource cleanup.
     */
    class GameAudioManager {
    public:
        GameAudioManager();
        ~GameAudioManager();

        bool Initialize(AudioEngine* audioEngine);
        void Update(float deltaTime, const Character* character);
        void Cleanup();

        // Audio control
        void PlayJumpSound();
        void SetWalkingState(bool isWalking);
        void PlayBackgroundMusic();
        void StopBackgroundMusic();

        // Configuration
        void SetAudioConfiguration(const AudioConfiguration& config);
        const AudioConfiguration& GetAudioConfiguration() const { return m_config; }

        // Volume controls
        void SetMasterVolume(float volume);
        void SetMusicVolume(float volume);
        void SetSFXVolume(float volume);

        // State queries
        bool IsAudioAvailable() const;
        bool IsBackgroundMusicPlaying() const { return m_backgroundMusicPlaying; }

        // Character type switching support
        void OnCharacterTypeChanged();

    private:
        void LoadAudioClips();
        void CreateAudioSources();
        void UpdateFootstepAudio(float deltaTime, const Character* character);
        void UpdateListenerPosition(const Character* character);

        // Audio engine reference
        AudioEngine* m_audioEngine = nullptr;

        // Audio configuration
        AudioConfiguration m_config;

        // Audio Sources
        uint32_t m_backgroundMusicSource = 0;
        uint32_t m_footstepSource = 0;
        uint32_t m_jumpSource = 0;

        // Audio Clips
        std::shared_ptr<AudioClip> m_backgroundMusic;
        std::shared_ptr<AudioClip> m_footstepSound;
        std::shared_ptr<AudioClip> m_jumpSound;

        // State Management
        bool m_isWalking = false;
        bool m_backgroundMusicPlaying = false;
        float m_footstepTimer = 0.0f;
        bool m_initialized = false;

        // Previous character state for change detection
        Math::Vec3 m_previousCharacterPosition{0.0f};
        bool m_previousGroundedState = false;
        bool m_previousJumpingState = false;
    };
}