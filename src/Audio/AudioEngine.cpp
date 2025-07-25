#include "Audio/AudioEngine.h"
#include "Audio/AudioLoader.h"
#include "Audio/AudioBufferPool.h"
#include "Audio/AudioSourcePool.h"
#include "Audio/Audio3DCalculator.h"
#include "Core/Logger.h"

#ifdef GAMEENGINE_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace GameEngine {
    AudioEngine::AudioEngine() {
        // Initialize performance optimization components
        m_bufferPool = std::make_unique<AudioBufferPool>();
        m_sourcePool = std::make_unique<AudioSourcePool>();
        m_audio3DCalculator = std::make_unique<Audio3DCalculator>();
    }

    AudioEngine::~AudioEngine() {
        Shutdown();
    }

    bool AudioEngine::Initialize() {
        LOG_INFO("Initializing Audio Engine...");
        
        if (!InitializeOpenAL()) {
            LOG_WARNING("Failed to initialize OpenAL, continuing in silent mode");
            LOG_INFO("Audio functionality will be disabled but the engine will continue normally");
            m_audioAvailable = false;
            m_openALInitialized = false;
            
            // Create a dummy listener for API compatibility
            m_listener = std::make_unique<AudioListener>();
            return true; // Return true to allow engine to continue without audio
        }
        
        // Initialize performance optimization components now that OpenAL is ready
        if (m_sourcePool) {
            m_sourcePool->Initialize();
        }
        
        m_listener = std::make_unique<AudioListener>();
        m_audioAvailable = true;
        LOG_INFO("Audio Engine initialized successfully with OpenAL support");
        return true;
    }

    void AudioEngine::Shutdown() {
        // Clear performance optimization components first
        if (m_sourcePool) {
            m_sourcePool->Clear();
        }
        if (m_bufferPool) {
            m_bufferPool->Clear();
        }
        
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
        
        // Reset performance components
        m_bufferPool.reset();
        m_sourcePool.reset();
        m_audio3DCalculator.reset();
        
        ShutdownOpenAL();
        
        // Mark audio as unavailable after shutdown
        m_audioAvailable = false;
        
        LOG_INFO("Audio Engine shutdown");
    }

    void AudioEngine::Update(float deltaTime) {
        if (!m_audioAvailable) {
            return; // Skip audio updates if audio is not available
        }

#ifdef GAMEENGINE_HAS_OPENAL
        // Check for device disconnection
        if (m_openALInitialized && !CheckDeviceConnection()) {
            LOG_WARNING("Audio device disconnection detected");
            HandleDeviceDisconnection();
            return;
        }
#endif

        // Update performance optimization components
        if (m_sourcePoolingEnabled && m_sourcePool) {
            m_sourcePool->Update();
        }
        
        if (m_bufferPoolingEnabled && m_bufferPool) {
            // Periodic cleanup of unused buffers (every 30 seconds)
            static float cleanupTimer = 0.0f;
            cleanupTimer += deltaTime;
            if (cleanupTimer >= 30.0f) {
                m_bufferPool->CleanupUnusedBuffers();
                cleanupTimer = 0.0f;
            }
        }

        // Update audio sources, check for completed sounds, etc.
        for (auto it = m_audioSources.begin(); it != m_audioSources.end();) {
            auto& source = it->second;
            if (source && source->IsPlaying()) {
                // Check if source is still actually playing (OpenAL state)
                if (!source->GetOpenALPlayingState()) {
                    // Source finished playing, clean up
                    source->Stop();
                }
            }
            ++it;
        }
    }

    std::shared_ptr<AudioClip> AudioEngine::LoadAudioClip(const std::string& path) {
        LOG_DEBUG("Loading audio clip: " + path);
        
        // Use buffer pool if enabled
        if (m_bufferPoolingEnabled && m_bufferPool) {
            auto clip = m_bufferPool->GetBuffer(path);
            if (clip) {
                // Also cache in legacy storage for compatibility
                m_audioClips[path] = clip;
                return clip;
            }
        }
        
        // Check legacy cache
        auto it = m_audioClips.find(path);
        if (it != m_audioClips.end()) {
            LOG_DEBUG("Audio clip found in legacy cache: " + path);
            return it->second;
        }

        auto clip = std::make_shared<AudioClip>();
        clip->path = path;

        try {
            // Load audio data using unified AudioLoader interface
            AudioLoader loader;
            AudioData audioData = loader.LoadAudio(path);
            
            if (!audioData.isValid) {
                LOG_ERROR("Failed to load audio file (invalid data): " + path);
                LOG_INFO("Audio clip loading failed, but engine will continue without this sound");
                return nullptr;
            }

            // Determine format from loaded data or file extension as fallback
            if (AudioLoader::IsWAVFile(path)) {
                clip->format = AudioFormat::WAV;
            } else if (AudioLoader::IsOGGFile(path)) {
                clip->format = AudioFormat::OGG;
            } else {
                // Format was detected by content analysis
                clip->format = AudioFormat::OGG; // Default assumption for successful load
            }

            // Populate clip data
            clip->duration = audioData.duration;
            clip->sampleRate = audioData.sampleRate;
            clip->channels = audioData.channels;

#ifdef GAMEENGINE_HAS_OPENAL
            // Create OpenAL buffer if OpenAL is available
            if (m_openALInitialized && m_audioAvailable) {
                clip->bufferId = loader.CreateOpenALBuffer(audioData);
                if (clip->bufferId == 0) {
                    LOG_ERROR("Failed to create OpenAL buffer for: " + path);
                    LOG_WARNING("Audio clip will be cached but cannot be played");
                    // Still cache the clip for API consistency
                }
            } else {
                LOG_DEBUG("OpenAL not available, audio clip loaded but cannot be played: " + path);
            }
#endif

            // Cache the loaded clip even if OpenAL buffer creation failed
            m_audioClips[path] = clip;
            LOG_INFO("Successfully loaded audio clip: " + path);
            return clip;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while loading audio clip '" + path + "': " + e.what());
            return nullptr;
        } catch (...) {
            LOG_ERROR("Unknown exception while loading audio clip: " + path);
            return nullptr;
        }
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
        // Use source pool if enabled
        if (m_sourcePoolingEnabled && m_sourcePool) {
            uint32_t pooledId = m_sourcePool->AcquireSource();
            if (pooledId != 0) {
                LOG_DEBUG("Acquired pooled audio source with ID: " + std::to_string(pooledId));
                return pooledId;
            }
        }
        
        // Fallback to legacy creation
        uint32_t id = m_nextSourceId++;
        
        try {
            m_audioSources[id] = std::make_unique<AudioSource>(id);
            LOG_DEBUG("Created legacy audio source with ID: " + std::to_string(id));
            return id;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create audio source: " + std::string(e.what()));
            return 0; // Return invalid ID
        } catch (...) {
            LOG_ERROR("Unknown exception while creating audio source");
            return 0; // Return invalid ID
        }
    }

    void AudioEngine::DestroyAudioSource(uint32_t sourceId) {
        // Try to release to source pool first
        if (m_sourcePoolingEnabled && m_sourcePool && m_sourcePool->IsSourceActive(sourceId)) {
            m_sourcePool->ReleaseSource(sourceId);
            LOG_DEBUG("Released audio source to pool: " + std::to_string(sourceId));
            return;
        }
        
        // Fallback to legacy destruction
        m_audioSources.erase(sourceId);
    }

    void AudioEngine::PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip) {
        if (!m_audioAvailable) {
            LOG_DEBUG("Audio not available, skipping playback for source ID: " + std::to_string(sourceId));
            return;
        }
        
        if (!clip) {
            LOG_WARNING("Attempted to play null audio clip on source ID: " + std::to_string(sourceId));
            return;
        }
        
        AudioSource* audioSource = nullptr;
        
        // First try to find in legacy sources
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // If not found in legacy sources, try the source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (!audioSource) {
            LOG_WARNING("Attempted to play audio on non-existent source ID: " + std::to_string(sourceId));
            return;
        }
        
        try {
            audioSource->Play(clip);
            LOG_DEBUG("Playing audio clip '" + clip->path + "' on source ID: " + std::to_string(sourceId));
        } catch (const std::exception& e) {
            LOG_ERROR("Exception while playing audio source " + std::to_string(sourceId) + ": " + e.what());
        } catch (...) {
            LOG_ERROR("Unknown exception while playing audio source: " + std::to_string(sourceId));
        }
    }

    void AudioEngine::StopAudioSource(uint32_t sourceId) {
        AudioSource* audioSource = nullptr;
        
        // Try legacy sources first
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // Try source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (audioSource) {
            audioSource->Stop();
        }
    }

    void AudioEngine::PauseAudioSource(uint32_t sourceId) {
        AudioSource* audioSource = nullptr;
        
        // Try legacy sources first
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // Try source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (audioSource) {
            audioSource->Pause();
        }
    }

    void AudioEngine::SetAudioSourcePosition(uint32_t sourceId, const Math::Vec3& position) {
        AudioSource* audioSource = nullptr;
        
        // Try legacy sources first
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // Try source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (audioSource) {
            audioSource->SetPosition(position);
        }
    }

    void AudioEngine::SetAudioSourceVolume(uint32_t sourceId, float volume) {
        AudioSource* audioSource = nullptr;
        
        // Try legacy sources first
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // Try source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (audioSource) {
            audioSource->SetVolume(volume);
        }
    }

    void AudioEngine::SetAudioSourcePitch(uint32_t sourceId, float pitch) {
        AudioSource* audioSource = nullptr;
        
        // Try legacy sources first
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // Try source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (audioSource) {
            audioSource->SetPitch(pitch);
        }
    }

    void AudioEngine::SetAudioSourceLooping(uint32_t sourceId, bool looping) {
        AudioSource* audioSource = nullptr;
        
        // Try legacy sources first
        auto it = m_audioSources.find(sourceId);
        if (it != m_audioSources.end()) {
            audioSource = it->second.get();
        }
        // Try source pool
        else if (m_sourcePoolingEnabled && m_sourcePool) {
            audioSource = m_sourcePool->GetSource(sourceId);
        }
        
        if (audioSource) {
            audioSource->SetLooping(looping);
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
        LOG_INFO("Attempting to initialize OpenAL...");
        
        // Try to enumerate available devices first
        if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT")) {
            const char* devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
            if (devices) {
                LOG_INFO("Available audio devices:");
                const char* device = devices;
                while (*device) {
                    LOG_INFO("  - " + std::string(device));
                    device += strlen(device) + 1;
                }
            }
        }

        // Open the default audio device with error handling
        m_device = alcOpenDevice(nullptr);
        if (!m_device) {
            LOG_ERROR("Failed to open default OpenAL device");
            
            // Try to get more specific error information
            ALCenum error = alcGetError(nullptr);
            if (error != ALC_NO_ERROR) {
                LOG_ERROR("OpenAL device error: " + std::to_string(error));
            }
            
            LOG_INFO("Attempting to continue without audio support");
            return false;
        }

        // Log device information
        LogDeviceInfo();

        // Create OpenAL context with error handling
        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context) {
            LOG_ERROR("Failed to create OpenAL context");
            ALCenum error = alcGetError(m_device);
            if (error != ALC_NO_ERROR) {
                LOG_ERROR("OpenAL context creation error: " + std::to_string(error));
            }
            
            alcCloseDevice(m_device);
            m_device = nullptr;
            return false;
        }

        // Make context current with error handling
        if (!alcMakeContextCurrent(m_context)) {
            LOG_ERROR("Failed to make OpenAL context current");
            ALCenum error = alcGetError(m_device);
            if (error != ALC_NO_ERROR) {
                LOG_ERROR("OpenAL context activation error: " + std::to_string(error));
            }
            
            alcDestroyContext(m_context);
            alcCloseDevice(m_device);
            m_context = nullptr;
            m_device = nullptr;
            return false;
        }

        // Check for OpenAL errors after context setup
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::string errorMsg = "OpenAL initialization error: " + GetOpenALErrorString(error);
            LOG_ERROR(errorMsg);
            ShutdownOpenAL();
            return false;
        }

        // Log OpenAL information with error checking
        const char* vendor = alGetString(AL_VENDOR);
        const char* version = alGetString(AL_VERSION);
        const char* renderer = alGetString(AL_RENDERER);
        const char* extensions = alGetString(AL_EXTENSIONS);
        
        LOG_INFO("OpenAL initialized successfully");
        LOG_INFO("  Vendor: " + std::string(vendor ? vendor : "Unknown"));
        LOG_INFO("  Version: " + std::string(version ? version : "Unknown"));
        LOG_INFO("  Renderer: " + std::string(renderer ? renderer : "Unknown"));
        
        if (extensions) {
            LOG_DEBUG("  Extensions: " + std::string(extensions));
        }

        // Set default listener properties with error checking
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        if (!CheckOpenALError("Setting listener position")) {
            LOG_WARNING("Failed to set default listener position");
        }
        
        alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        if (!CheckOpenALError("Setting listener velocity")) {
            LOG_WARNING("Failed to set default listener velocity");
        }
        
        ALfloat orientation[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f}; // forward, up
        alListenerfv(AL_ORIENTATION, orientation);
        if (!CheckOpenALError("Setting listener orientation")) {
            LOG_WARNING("Failed to set default listener orientation");
        }

        m_openALInitialized = true;
        m_recoveryAttempted = false;
        LOG_INFO("OpenAL initialization completed successfully");
        return true;
#else
        LOG_WARNING("OpenAL support not compiled in - audio will be disabled");
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
            LOG_ERROR("OpenAL error in " + operation + ": " + GetOpenALErrorString(error));
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
                return "Invalid name parameter";
            case AL_INVALID_ENUM:
                return "Invalid enum parameter";
            case AL_INVALID_VALUE:
                return "Invalid value parameter";
            case AL_INVALID_OPERATION:
                return "Invalid operation";
            case AL_OUT_OF_MEMORY:
                return "Out of memory";
            default:
                return "Unknown OpenAL error (code: " + std::to_string(error) + ")";
        }
#else
        return "OpenAL not available";
#endif
    }

    bool AudioEngine::AttemptAudioRecovery() {
        if (m_recoveryAttempted) {
            LOG_WARNING("Audio recovery already attempted, skipping");
            return false;
        }
        
        LOG_INFO("Attempting audio system recovery...");
        m_recoveryAttempted = true;
        
        // Shutdown current OpenAL state
        ShutdownOpenAL();
        
        // Clear all audio sources and clips
        m_audioSources.clear();
        
        // Don't clear cached clips as they might be reusable
        // Just clear their OpenAL buffer IDs
#ifdef GAMEENGINE_HAS_OPENAL
        for (auto& pair : m_audioClips) {
            if (pair.second->bufferId != 0) {
                // Buffer is already invalid due to context destruction
                pair.second->bufferId = 0;
            }
        }
#endif
        
        // Try to reinitialize OpenAL
        if (InitializeOpenAL()) {
            LOG_INFO("Audio recovery successful - OpenAL reinitialized");
            m_audioAvailable = true;
            
            // Recreate listener
            m_listener = std::make_unique<AudioListener>();
            
            // Reload audio clips' OpenAL buffers
            AudioLoader loader;
            int reloadedCount = 0;
            for (auto& pair : m_audioClips) {
                auto& clip = pair.second;
                if (clip->bufferId == 0) {
                    // Try to reload the audio data and create buffer
                    AudioData audioData = loader.LoadAudio(clip->path);
                    if (audioData.isValid) {
                        clip->bufferId = loader.CreateOpenALBuffer(audioData);
                        if (clip->bufferId != 0) {
                            reloadedCount++;
                        }
                    }
                }
            }
            
            LOG_INFO("Reloaded " + std::to_string(reloadedCount) + " audio clips after recovery");
            return true;
        } else {
            LOG_ERROR("Audio recovery failed - continuing in silent mode");
            m_audioAvailable = false;
            return false;
        }
    }

    void AudioEngine::HandleDeviceDisconnection() {
        m_deviceDisconnectionCount++;
        LOG_WARNING("Audio device disconnection #" + std::to_string(m_deviceDisconnectionCount) + " detected");
        
        m_audioAvailable = false;
        m_openALInitialized = false;
        
        // Stop all currently playing sources
        for (auto& pair : m_audioSources) {
            if (pair.second && pair.second->IsPlaying()) {
                pair.second->Stop();
            }
        }
        
        // Attempt recovery if we haven't tried too many times
        if (m_deviceDisconnectionCount <= 3) {
            LOG_INFO("Attempting automatic audio recovery...");
            if (AttemptAudioRecovery()) {
                LOG_INFO("Audio device recovery successful");
            } else {
                LOG_WARNING("Audio device recovery failed, continuing without audio");
            }
        } else {
            LOG_ERROR("Too many device disconnections (" + std::to_string(m_deviceDisconnectionCount) + 
                     "), disabling automatic recovery");
        }
    }

#ifdef GAMEENGINE_HAS_OPENAL
    bool AudioEngine::CheckDeviceConnection() {
        if (!m_device || !m_context) {
            return false;
        }
        
        // Check if the device is still connected
        ALCint connected = ALC_TRUE;
        if (alcIsExtensionPresent(m_device, "ALC_EXT_disconnect")) {
            // ALC_CONNECTED is defined in the disconnect extension
            const ALCenum ALC_CONNECTED = 0x313;
            alcGetIntegerv(m_device, ALC_CONNECTED, 1, &connected);
            ALCenum error = alcGetError(m_device);
            if (error != ALC_NO_ERROR) {
                LOG_WARNING("Error checking device connection: " + std::to_string(error));
                return false;
            }
        }
        
        return connected == ALC_TRUE;
    }

    void AudioEngine::LogDeviceInfo() {
        if (!m_device) {
            return;
        }
        
        const char* deviceName = alcGetString(m_device, ALC_DEVICE_SPECIFIER);
        if (deviceName) {
            LOG_INFO("Using audio device: " + std::string(deviceName));
        }
        
        // Get device capabilities
        ALCint majorVersion = 0, minorVersion = 0;
        alcGetIntegerv(m_device, ALC_MAJOR_VERSION, 1, &majorVersion);
        alcGetIntegerv(m_device, ALC_MINOR_VERSION, 1, &minorVersion);
        
        if (majorVersion > 0 || minorVersion > 0) {
            LOG_INFO("Device ALC version: " + std::to_string(majorVersion) + "." + std::to_string(minorVersion));
        }
        
        // Check for important extensions
        if (alcIsExtensionPresent(m_device, "ALC_EXT_disconnect")) {
            LOG_DEBUG("Device supports disconnect detection");
        }
        
        if (alcIsExtensionPresent(m_device, "ALC_ENUMERATE_ALL_EXT")) {
            LOG_DEBUG("Device supports device enumeration");
        }
    }

    std::string AudioEngine::GetDeviceName() const {
        if (!m_device) {
            return "No device";
        }
        
        const char* deviceName = alcGetString(m_device, ALC_DEVICE_SPECIFIER);
        return deviceName ? std::string(deviceName) : "Unknown device";
    }
#endif

    // AudioSource implementation
    AudioSource::AudioSource(uint32_t id) : m_id(id) {
        LOG_DEBUG("Creating audio source with ID: " + std::to_string(id));
        
#ifdef GAMEENGINE_HAS_OPENAL
        // Generate OpenAL source
        alGenSources(1, &m_sourceId);
        if (!AudioEngine::CheckOpenALError("Creating OpenAL source")) {
            LOG_ERROR("Failed to create OpenAL source for ID: " + std::to_string(id));
            m_sourceId = 0;
            return;
        }
        
        if (m_sourceId == 0) {
            LOG_ERROR("OpenAL returned invalid source ID for audio source: " + std::to_string(id));
            return;
        }
        
        // Set default source properties with error checking
        alSourcef(m_sourceId, AL_PITCH, 1.0f);
        if (!AudioEngine::CheckOpenALError("Setting source pitch")) {
            LOG_WARNING("Failed to set default pitch for source " + std::to_string(id));
        }
        
        alSourcef(m_sourceId, AL_GAIN, 1.0f);
        if (!AudioEngine::CheckOpenALError("Setting source gain")) {
            LOG_WARNING("Failed to set default gain for source " + std::to_string(id));
        }
        
        alSource3f(m_sourceId, AL_POSITION, 0.0f, 0.0f, 0.0f);
        if (!AudioEngine::CheckOpenALError("Setting source position")) {
            LOG_WARNING("Failed to set default position for source " + std::to_string(id));
        }
        
        alSource3f(m_sourceId, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        if (!AudioEngine::CheckOpenALError("Setting source velocity")) {
            LOG_WARNING("Failed to set default velocity for source " + std::to_string(id));
        }
        
        alSourcei(m_sourceId, AL_LOOPING, AL_FALSE);
        if (!AudioEngine::CheckOpenALError("Setting source looping")) {
            LOG_WARNING("Failed to set default looping for source " + std::to_string(id));
        }
        
        LOG_DEBUG("Successfully created OpenAL source " + std::to_string(m_sourceId) + 
                 " for audio source ID: " + std::to_string(id));
#else
        LOG_DEBUG("Created audio source " + std::to_string(id) + " (OpenAL not available)");
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
        if (!clip) {
            LOG_WARNING("Attempted to play null audio clip on source " + std::to_string(m_id));
            return;
        }
        
        LOG_DEBUG("Playing audio clip '" + clip->path + "' on source " + std::to_string(m_id));
        
        // Stop current playback if any
        if (m_isPlaying) {
            Stop();
        }
        
        m_currentClip = clip;
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId == 0) {
            LOG_ERROR("Cannot play audio - OpenAL source not initialized for source " + std::to_string(m_id));
            return;
        }
        
        if (clip->bufferId == 0) {
            LOG_WARNING("Cannot play audio clip '" + clip->path + "' - no OpenAL buffer available");
            return;
        }
        
        // Detach any previous buffer
        alSourcei(m_sourceId, AL_BUFFER, 0);
        if (!AudioEngine::CheckOpenALError("Detaching previous buffer")) {
            LOG_WARNING("Failed to detach previous buffer from source " + std::to_string(m_id));
        }
        
        // Attach new buffer
        alSourcei(m_sourceId, AL_BUFFER, clip->bufferId);
        if (!AudioEngine::CheckOpenALError("Attaching audio buffer")) {
            LOG_ERROR("Failed to attach audio buffer for clip '" + clip->path + "'");
            return;
        }
        
        // Start playback
        alSourcePlay(m_sourceId);
        if (AudioEngine::CheckOpenALError("Starting audio playback")) {
            m_isPlaying = true;
            m_isPaused = false;
            LOG_DEBUG("Successfully started playback of '" + clip->path + "' on source " + std::to_string(m_id));
        } else {
            LOG_ERROR("Failed to start playback of '" + clip->path + "' on source " + std::to_string(m_id));
            // Detach buffer on failure
            alSourcei(m_sourceId, AL_BUFFER, 0);
        }
#else
        // Fallback for when OpenAL is not available
        m_isPlaying = true;
        m_isPaused = false;
        LOG_DEBUG("Simulating playback of '" + clip->path + "' (OpenAL not available)");
#endif
    }

    void AudioSource::Stop() {
        m_isPlaying = false;
        m_isPaused = false;
        
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            alSourceStop(m_sourceId);
            // Detach buffer to free resources
            alSourcei(m_sourceId, AL_BUFFER, 0);
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
        if (m_isPaused && m_isPlaying) {
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

    bool AudioSource::GetOpenALPlayingState() const {
#ifdef GAMEENGINE_HAS_OPENAL
        if (m_sourceId != 0) {
            ALint state;
            alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
            return state == AL_PLAYING;
        }
#endif
        return false;
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

    // Performance optimization methods
    void AudioEngine::SetBufferPoolSize(size_t maxSize) {
        if (m_bufferPool) {
            m_bufferPool->SetMaxPoolSize(maxSize);
            LOG_INFO("AudioEngine buffer pool max size set to: " + std::to_string(maxSize));
        }
    }

    void AudioEngine::SetSourcePoolSize(size_t minSize, size_t maxSize) {
        if (m_sourcePool) {
            m_sourcePool->SetPoolSize(minSize, maxSize);
            LOG_INFO("AudioEngine source pool size set to min: " + std::to_string(minSize) + 
                    ", max: " + std::to_string(maxSize));
        }
    }

    void AudioEngine::MarkAudioAsHot(const std::string& filepath) {
        if (m_bufferPool) {
            m_bufferPool->MarkAsHot(filepath);
            LOG_DEBUG("AudioEngine marked audio as hot: " + filepath);
        }
    }

    void AudioEngine::UnmarkAudioAsHot(const std::string& filepath) {
        if (m_bufferPool) {
            m_bufferPool->UnmarkAsHot(filepath);
            LOG_DEBUG("AudioEngine unmarked audio as hot: " + filepath);
        }
    }

    float AudioEngine::GetBufferPoolHitRatio() const {
        if (m_bufferPool) {
            return m_bufferPool->GetHitRatio();
        }
        return 0.0f;
    }

    float AudioEngine::GetSourcePoolUtilization() const {
        if (m_sourcePool) {
            return m_sourcePool->GetPoolUtilization();
        }
        return 0.0f;
    }

    size_t AudioEngine::GetBufferPoolMemoryUsage() const {
        if (m_bufferPool) {
            return m_bufferPool->GetMemoryUsage();
        }
        return 0;
    }

    int AudioEngine::GetAudio3DCalculationsPerSecond() const {
        if (m_audio3DCalculator) {
            return m_audio3DCalculator->GetCalculationsPerSecond();
        }
        return 0;
    }
}