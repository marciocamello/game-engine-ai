#pragma once

#include "Core/Math.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace GameEngine {
    class AudioSource;
    class AudioListener;

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
    };

    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

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

    private:
        std::unordered_map<std::string, std::shared_ptr<AudioClip>> m_audioClips;
        std::unordered_map<uint32_t, std::unique_ptr<AudioSource>> m_audioSources;
        std::unique_ptr<AudioListener> m_listener;
        
        uint32_t m_nextSourceId = 1;
        float m_masterVolume = 1.0f;
        float m_musicVolume = 1.0f;
        float m_sfxVolume = 1.0f;
    };

    class AudioSource {
    public:
        AudioSource(uint32_t id);
        ~AudioSource();

        void Play(std::shared_ptr<AudioClip> clip);
        void Stop();
        void Pause();
        void Resume();

        void SetPosition(const Math::Vec3& position) { m_position = position; }
        void SetVolume(float volume) { m_volume = volume; }
        void SetPitch(float pitch) { m_pitch = pitch; }
        void SetLooping(bool looping) { m_looping = looping; }

        bool IsPlaying() const { return m_isPlaying; }
        bool IsPaused() const { return m_isPaused; }

    private:
        uint32_t m_id;
        Math::Vec3 m_position{0.0f};
        float m_volume = 1.0f;
        float m_pitch = 1.0f;
        bool m_looping = false;
        bool m_isPlaying = false;
        bool m_isPaused = false;
        std::shared_ptr<AudioClip> m_currentClip;
    };

    class AudioListener {
    public:
        AudioListener();
        ~AudioListener();

        void SetPosition(const Math::Vec3& position) { m_position = position; }
        void SetOrientation(const Math::Vec3& forward, const Math::Vec3& up) { m_forward = forward; m_up = up; }
        void SetVelocity(const Math::Vec3& velocity) { m_velocity = velocity; }

    private:
        Math::Vec3 m_position{0.0f};
        Math::Vec3 m_forward{0.0f, 0.0f, -1.0f};
        Math::Vec3 m_up{0.0f, 1.0f, 0.0f};
        Math::Vec3 m_velocity{0.0f};
    };
}