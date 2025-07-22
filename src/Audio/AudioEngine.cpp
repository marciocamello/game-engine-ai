#include "Audio/AudioEngine.h"
#include "Audio/AudioLoader.h"
#include "Core/Logger.h"

#ifdef GAMEENGINE_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace GameEngine {
    AudioEngine::AudioEngine() {
    }

    AudioEngine::~AudioEngine() {
        Shutdown();
    }

    bool AudioEngine::Initialize() {
        if (!InitializeOpenAL()) {
            LOG_ERROR("Failed to initialize OpenAL, continuing without audio");
            return false;
        }
        
        m_listener = std::make_unique<AudioListener>();
        LOG_INFO("Audio Engine initialized successfully");
        return true;
    }

    void AudioEngine::Shutdown() {
        m_audioSources.clear();
        
        // Clean up all audio clips and their OpenAL buffers
#ifdef GAMEENGINE_HAS_OPENAL
        for (auto& pair : m_audioClips) {
            if (pair.second->bufferId != 0) {
                alDeleteBuffers(1, &pair.second->bufferId);
            }
        }
#endif
        m_audioClips.clear();
        m_listener.reset();
        
        ShutdownOpenAL();
        LOG_INFO("Audio Engine shutdown");
    }

    void AudioEngine::Update(float deltaTime) {
        // Update audio sources, apply 3D positioning, etc.
    }

    std::shared_ptr<AudioClip> AudioEngine::LoadAudioClip(const std::string& path) {
        // Check cache first
        auto it = m_audioClips.find(path);
        if (it != m_audioClips.end()) {
            return it->second;
        }

        auto clip = std::make_shared<AudioClip>();
        clip->path = path;

        // Determine format from file extension
        if (AudioLoader::IsWAVFile(path)) {
            clip->format = AudioFormat::WAV;
        } else if (AudioLoader::IsOGGFile(path)) {
            clip->format = AudioFormat::OGG;
        } else {
            LOG_ERROR("Unsupported audio format for file: " + path);
            return nullptr;
        }

        // Load audio data using AudioLoader
        AudioLoader loader;
        AudioData audioData;
        
        if (clip->format == AudioFormat::WAV) {
            audioData = loader.LoadWAV(path);
        } else if (clip->format == AudioFormat::OGG) {
            audioData = loader.LoadOGG(path);
        }

        if (!audioData.isValid) {
            LOG_ERROR("Failed to load audio file: " + path);
            return nullptr;
        }

        // Populate clip data
        clip->duration = audioData.duration;
        clip->sampleRate = audioData.sampleRate;
        clip->channels = audioData.channels;

#ifdef GAMEENGINE_HAS_OPENAL
        // Create OpenAL buffer if OpenAL is available
        if (m_openALInitialized) {
            clip->bufferId = loader.CreateOpenALBuffer(audioData);
            if (clip->bufferId == 0) {
                LOG_ERROR("Failed to create OpenAL buffer for: " + path);
                return nullptr;
            }
        }
#endif

        // Cache the loaded clip
        m_audioClips[path] = clip;
        return clip;
    }

    void AudioEngine::UnloadAudioClip(const std::string& path) {
        auto it = m_audioClips.find(path);
        if (it != m_audioClips.end()) {
#ifdef GAMEENGINE_HAS_OPENAL
            // Clean up OpenAL buffer
            if (it->second->bufferId != 0) {
                alDeleteBuffers(1, &it->second->bufferId);
                CheckOpenALError("Deleting audio buffer");
            }
#endif
            m_audioClips.erase(it);
            LOG_INFO("Unloaded audio clip: " + path);
        }
    }

    uint32_t AudioEngine::CreateAudioSource() {
        uint32_t id = m_nextSourceId++;
        m_audioSources[id] = std::make_unique<AudioSource>(id);
        return id;
    }

    void AudioEngine::DestroyAudioSource(uint32_t sourceId) {
        m_audioSources.erase(sourceId);
    }

    void AudioEngine::PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->Play(clip);
        }
    }

    void AudioEngine::StopAudioSource(uint32_t sourceId) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->Stop();
        }
    }

    void AudioEngine::PauseAudioSource(uint32_t sourceId) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->Pause();
        }
    }

    void AudioEngine::SetAudioSourcePosition(uint32_t sourceId, const Math::Vec3& position) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->SetPosition(position);
        }
    }

    void AudioEngine::SetAudioSourceVolume(uint32_t sourceId, float volume) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->SetVolume(volume);
        }
    }

    void AudioEngine::SetAudioSourcePitch(uint32_t sourceId, float pitch) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->SetPitch(pitch);
        }
    }

    void AudioEngine::SetAudioSourceLooping(uint32_t sourceId, bool looping) {
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            it->second->SetLooping(looping);
        }
    }

    void AudioEngine::SetListenerPosition(const Math::Vec3& position) {
        if (m_listener) {
            m_listener->SetPosition(position);
        }
    }

    void AudioEngine::SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up) {
        if (m_listener) {
            m_listener->SetOrientation(forward, up);
        }
    }

    void AudioEngine::SetListenerVelocity(const Math::Vec3& velocity) {
        if (m_listener) {
            m_listener->SetVelocity(velocity);
        }
    }

    void AudioEngine::SetMasterVolume(float volume) {
        m_masterVolume = Math::Clamp(volume, 0.0f, 1.0f);
    }

    void AudioEngine::SetMusicVolume(float volume) {
        m_musicVolume = Math::Clamp(volume, 0.0f, 1.0f);
    }

    void AudioEngine::SetSFXVolume(float volume) {
        m_sfxVolume = Math::Clamp(volume, 0.0f, 1.0f);
    }

    bool AudioEngine::InitializeOpenAL() {
#ifdef GAMEENGINE_HAS_OPENAL
        // Open the default audio device
        m_device = alcOpenDevice(nullptr);
        if (!m_device) {
            LOG_ERROR("Failed to open OpenAL device");
            return false;
        }

        // Create OpenAL context
        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context) {
            LOG_ERROR("Failed to create OpenAL context");
            alcCloseDevice(m_device);
            m_device = nullptr;
            return false;
        }

        // Make context current
        if (!alcMakeContextCurrent(m_context)) {
            LOG_ERROR("Failed to make OpenAL context current");
            alcDestroyContext(m_context);
            alcCloseDevice(m_device);
            m_context = nullptr;
            m_device = nullptr;
            return false;
        }

        // Check for errors
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            LOG_ERROR("OpenAL initialization error: {}", GetOpenALErrorString(error));
            ShutdownOpenAL();
            return false;
        }

        // Log OpenAL information
        const char* vendor = alGetString(AL_VENDOR);
        const char* version = alGetString(AL_VERSION);
        const char* renderer = alGetString(AL_RENDERER);
        
        LOG_INFO("OpenAL initialized successfully");
        LOG_INFO("  Vendor: {}", vendor ? vendor : "Unknown");
        LOG_INFO("  Version: {}", version ? version : "Unknown");
        LOG_INFO("  Renderer: {}", renderer ? renderer : "Unknown");

        // Set default listener properties
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        ALfloat orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f}; // forward, up
        alListenerfv(AL_ORIENTATION, orientation);

        CheckOpenALError("Setting default listener properties");

        m_openALInitialized = true;
        return true;
#else
        LOG_WARNING("OpenAL support not compiled in");
        return false;
#endif
    }

    void AudioEngine::ShutdownOpenAL() {
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_openALInitialized) {
            // Clean up context and device
            if (m_context) {
                alcMakeContextCurrent(nullptr);
                alcDestroyContext(m_context);
                m_context = nullptr;
            }

            if (m_device) {
                alcCloseDevice(m_device);
                m_device = nullptr;
            }

            m_openALInitialized = false;
            LOG_INFO("OpenAL shutdown complete");
        }
#endif
    }

    bool AudioEngine::CheckOpenALError(const std::string& operation) {
#ifdef GAMEENGINE_HAS_OPENAL
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            LOG_ERROR("OpenAL error in {}: {}", operation, GetOpenALErrorString(error));
            return false;
        }
        return true;
#else
        return true;
#endif
    }

    std::string AudioEngine::GetOpenALErrorString(ALenum error) {
#ifdef GAMEENGINE_HAS_OPENAL
        switch (error) {
            case AL_NO_ERROR:
                return "No error";
            case AL_INVALID_NAME:
                return "Invalid name";
            case AL_INVALID_ENUM:
                return "Invalid enum";
            case AL_INVALID_VALUE:
                return "Invalid value";
            case AL_INVALID_OPERATION:
                return "Invalid operation";
            case AL_OUT_OF_MEMORY:
                return "Out of memory";
            default:
                return "Unknown error (" + std::to_string(error) + ")";
        }
#else
        return "OpenAL not available";
#endif
    }

    // AudioSource implementation
    AudioSource::AudioSource(uint32_t id) : m_id(id) {
#ifdef GAMEENGINE_HAS_OPENAL
        // Generate OpenAL source
        alGenSources(1, &m_sourceId);
        if (!AudioEngine::CheckOpenALError("Creating audio source")) {
            m_sourceId = 0;
        } else {
            // Set default source properties
            alSourcef(m_sourceId, AL_PITCH, 1.0f);
            alSourcef(m_sourceId, AL_GAIN, 1.0f);
            alSource3f(m_sourceId, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(m_sourceId, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            alSourcei(m_sourceId, AL_LOOPING, AL_FALSE);
            AudioEngine::CheckOpenALError("Setting default source properties");
        }
#endif
    }

    AudioSource::~AudioSource() {
        Stop();
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alDeleteSources(1, &m_sourceId);
            AudioEngine::CheckOpenALError("Deleting audio source");
        }
#endif
    }

    void AudioSource::Play(std::shared_ptr<AudioClip> clip) {
        m_currentClip = clip;
        m_isPlaying = true;
        m_isPaused = false;
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0 && clip && clip->bufferId != 0) {
            alSourcei(m_sourceId, AL_BUFFER, clip->bufferId);
            alSourcePlay(m_sourceId);
            AudioEngine::CheckOpenALError("Playing audio source");
        }
#endif
    }

    void AudioSource::Stop() {
        m_isPlaying = false;
        m_isPaused = false;
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alSourceStop(m_sourceId);
            AudioEngine::CheckOpenALError("Stopping audio source");
        }
#endif
    }

    void AudioSource::Pause() {
        if (m_isPlaying) {
            m_isPaused = true;
            
#ifdef GAMEENGINE_HAS_OPENAL
            if (m_sourceId != 0) {
                alSourcePause(m_sourceId);
                AudioEngine::CheckOpenALError("Pausing audio source");
            }
#endif
        }
    }

    void AudioSource::Resume() {
        if (m_isPaused) {
            m_isPaused = false;
            
#ifdef GAMEENGINE_HAS_OPENAL
            if (m_sourceId != 0) {
                alSourcePlay(m_sourceId);
                AudioEngine::CheckOpenALError("Resuming audio source");
            }
#endif
        }
    }

    void AudioSource::SetPosition(const Math::Vec3& position) {
        m_position = position;
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alSource3f(m_sourceId, AL_POSITION, position.x, position.y, position.z);
            AudioEngine::CheckOpenALError("Setting audio source position");
        }
#endif
    }

    void AudioSource::SetVolume(float volume) {
        m_volume = Math::Clamp(volume, 0.0f, 1.0f);
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alSourcef(m_sourceId, AL_GAIN, m_volume);
            AudioEngine::CheckOpenALError("Setting audio source volume");
        }
#endif
    }

    void AudioSource::SetPitch(float pitch) {
        m_pitch = Math::Clamp(pitch, 0.1f, 2.0f); // Reasonable pitch range
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alSourcef(m_sourceId, AL_PITCH, m_pitch);
            AudioEngine::CheckOpenALError("Setting audio source pitch");
        }
#endif
    }

    void AudioSource::SetLooping(bool looping) {
        m_looping = looping;
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alSourcei(m_sourceId, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
            AudioEngine::CheckOpenALError("Setting audio source looping");
        }
#endif
    }

    // AudioListener implementation
    AudioListener::AudioListener() {
    }

    AudioListener::~AudioListener() {
    }

    void AudioListener::SetPosition(const Math::Vec3& position) {
        m_position = position;
        
#ifdef GAMEENGINE_HAS_OPENAL
        alListener3f(AL_POSITION, position.x, position.y, position.z);
        AudioEngine::CheckOpenALError("Setting listener position");
#endif
    }

    void AudioListener::SetOrientation(const Math::Vec3& forward, const Math::Vec3& up) {
        m_forward = forward;
        m_up = up;
        
#ifdef GAMEENGINE_HAS_OPENAL
        ALfloat orientation[] = {
            forward.x, forward.y, forward.z,
            up.x, up.y, up.z
        };
        alListenerfv(AL_ORIENTATION, orientation);
        AudioEngine::CheckOpenALError("Setting listener orientation");
#endif
    }

    void AudioListener::SetVelocity(const Math::Vec3& velocity) {
        m_velocity = velocity;
        
#ifdef GAMEENGINE_HAS_OPENAL
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        AudioEngine::CheckOpenALError("Setting listener velocity");
#endif
    }
}