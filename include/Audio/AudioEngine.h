#pragma once

#include "Core/Math.h"
#include <string>
#include <memory>
#include <unordered_map>

#ifdef GAMEENGINE_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace GameEngine {
    class AudioSource;
    class AudioListener;
    class AudioBufferPool;
    class AudioSourcePool;
    class Audio3DCalculator;

    enum class AudioFormat {
        WAV,
        OGG,
        MP3
    };

    struct AudioClip {
        std::string path;
        AudioFormat format;
        float duration = 0.0f;
        int sampleRate = 44100;
        int channels = 2;
        bool is3D = true;
        
#ifdef GAMEENGINE_HAS_OPENAL
        ALuint bufferId = 0;
#endif
    };

    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // Error handling and recovery
        bool IsAudioAvailable() const { return m_audioAvailable; }
        bool IsOpenALInitialized() const { return m_openALInitialized; }
        bool AttemptAudioRecovery();
        void HandleDeviceDisconnection();

        // Audio clip management
        std::shared_ptr<AudioClip> LoadAudioClip(const std::string& path);
        void UnloadAudioClip(const std::string& path);

        // Audio source management
        uint32_t CreateAudioSource();
        void DestroyAudioSource(uint32_t sourceId);
        void PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip);
        void StopAudioSource(uint32_t sourceId);
        void PauseAudioSource(uint32_t sourceId);
        void SetAudioSourcePosition(uint32_t sourceId, const Math::Vec3& position);
        void SetAudioSourceVolume(uint32_t sourceId, float volume);
        void SetAudioSourcePitch(uint32_t sourceId, float pitch);
        void SetAudioSourceLooping(uint32_t sourceId, bool looping);

        // Listener management
        void SetListenerPosition(const Math::Vec3& position);
        void SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up);
        void SetListenerVelocity(const Math::Vec3& velocity);

        // Global settings
        void SetMasterVolume(float volume);
        void SetMusicVolume(float volume);
        void SetSFXVolume(float volume);

        // Performance optimization controls
        void EnableBufferPooling(bool enabled) { m_bufferPoolingEnabled = enabled; }
        void EnableSourcePooling(bool enabled) { m_sourcePoolingEnabled = enabled; }
        void EnableOptimized3DAudio(bool enabled) { m_optimized3DAudioEnabled = enabled; }
        void SetBufferPoolSize(size_t maxSize);
        void SetSourcePoolSize(size_t minSize, size_t maxSize);
        void MarkAudioAsHot(const std::string& filepath);
        void UnmarkAudioAsHot(const std::string& filepath);
        
        // Performance statistics
        float GetBufferPoolHitRatio() const;
        float GetSourcePoolUtilization() const;
        size_t GetBufferPoolMemoryUsage() const;
        int GetAudio3DCalculationsPerSecond() const;

        // OpenAL error checking
        static bool CheckOpenALError(const std::string& operation);
        static std::string GetOpenALErrorString(ALenum error);

    private:
        bool InitializeOpenAL();
        void ShutdownOpenAL();
        
        // Legacy storage (kept for compatibility)
        std::unordered_map<std::string, std::shared_ptr<AudioClip>> m_audioClips;
        std::unordered_map<uint32_t, std::unique_ptr<AudioSource>> m_audioSources;
        std::unique_ptr<AudioListener> m_listener;
        
        // Performance optimization components
        std::unique_ptr<AudioBufferPool> m_bufferPool;
        std::unique_ptr<AudioSourcePool> m_sourcePool;
        std::unique_ptr<Audio3DCalculator> m_audio3DCalculator;
        
        // Performance settings
        bool m_bufferPoolingEnabled = true;
        bool m_sourcePoolingEnabled = true;
        bool m_optimized3DAudioEnabled = true;
        
        uint32_t m_nextSourceId = 1;
        float m_masterVolume = 1.0f;
        float m_musicVolume = 1.0f;
        float m_sfxVolume = 1.0f;
        
        // Error handling state
        bool m_audioAvailable = false;
        bool m_recoveryAttempted = false;
        int m_deviceDisconnectionCount = 0;
        
#ifdef GAMEENGINE_HAS_OPENAL
        ALCdevice* m_device = nullptr;
        ALCcontext* m_context = nullptr;
        bool m_openALInitialized = false;
        
        // Device monitoring
        bool CheckDeviceConnection();
        void LogDeviceInfo();
        std::string GetDeviceName() const;
#endif
    };

    class AudioSource {
    public:
        AudioSource(uint32_t id);
        ~AudioSource();

        void Play(std::shared_ptr<AudioClip> clip);
        void Stop();
        void Pause();
        void Resume();

        void SetPosition(const Math::Vec3& position);
        void SetVolume(float volume);
        void SetPitch(float pitch);
        void SetLooping(bool looping);

        bool IsPlaying() const { return m_isPlaying; }
        bool IsPaused() const { return m_isPaused; }
        bool IsStopped() const { return !m_isPlaying && !m_isPaused; }
        
        // Get current playback state from OpenAL
        bool GetOpenALPlayingState() const;
        
        // Get source ID
        uint32_t GetId() const { return m_id; }

    private:
        uint32_t m_id;
        Math::Vec3 m_position{0.0f};
        float m_volume = 1.0f;
        float m_pitch = 1.0f;
        bool m_looping = false;
        bool m_isPlaying = false;
        bool m_isPaused = false;
        std::shared_ptr<AudioClip> m_currentClip;
        
#ifdef GAMEENGINE_HAS_OPENAL
        ALuint m_sourceId = 0;
#endif
    };

    class AudioListener {
    public:
        AudioListener();
        ~AudioListener();

        void SetPosition(const Math::Vec3& position);
        void SetOrientation(const Math::Vec3& forward, const Math::Vec3& up);
        void SetVelocity(const Math::Vec3& velocity);

    private:
        Math::Vec3 m_position{0.0f};
        Math::Vec3 m_forward{0.0f, 0.0f, -1.0f};
        Math::Vec3 m_up{0.0f, 1.0f, 0.0f};
        Math::Vec3 m_velocity{0.0f};
    };
}