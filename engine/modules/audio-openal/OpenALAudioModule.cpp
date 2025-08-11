#include "OpenALAudioModule.h"
#include "Core/Logger.h"
#include <algorithm>

namespace GameEngine {
    namespace Audio {
        OpenALAudioModule::OpenALAudioModule() {
            LOG_DEBUG("Creating OpenAL Audio Module");
        }

        OpenALAudioModule::~OpenALAudioModule() {
            if (m_initialized) {
                Shutdown();
            }
        }

        bool OpenALAudioModule::Initialize(const ModuleConfig& config) {
            LOG_INFO("Initializing OpenAL Audio Module");
            
            if (m_initialized) {
                LOG_WARNING("OpenAL Audio Module already initialized");
                return true;
            }

            m_config = config;

            // Create the audio engine
            m_audioEngine = std::make_unique<AudioEngine>();
            if (!m_audioEngine) {
                LOG_ERROR("Failed to create AudioEngine instance");
                return false;
            }

            // Initialize the audio engine
            if (!m_audioEngine->Initialize()) {
                LOG_ERROR("Failed to initialize AudioEngine");
                m_audioEngine.reset();
                return false;
            }

            // Apply module configuration
            ApplyConfiguration(config);

            m_initialized = true;
            LOG_INFO("OpenAL Audio Module initialized successfully");
            return true;
        }

        void OpenALAudioModule::Update(float deltaTime) {
            if (!m_initialized || !m_enabled) {
                return;
            }

            if (m_audioEngine) {
                m_audioEngine->Update(deltaTime);
            }
        }

        void OpenALAudioModule::Shutdown() {
            if (!m_initialized) {
                return;
            }

            LOG_INFO("Shutting down OpenAL Audio Module");

            if (m_audioEngine) {
                m_audioEngine->Shutdown();
                m_audioEngine.reset();
            }

            m_initialized = false;
            LOG_INFO("OpenAL Audio Module shutdown complete");
        }

        const char* OpenALAudioModule::GetName() const {
            return "OpenALAudioModule";
        }

        const char* OpenALAudioModule::GetVersion() const {
            return "1.0.0";
        }

        ModuleType OpenALAudioModule::GetType() const {
            return ModuleType::Audio;
        }

        std::vector<std::string> OpenALAudioModule::GetDependencies() const {
            // Audio module has no dependencies on other engine modules
            // Core functionality (logging, math) is available without a module
            return {};
        }

        bool OpenALAudioModule::IsInitialized() const {
            return m_initialized;
        }

        bool OpenALAudioModule::IsEnabled() const {
            return m_enabled;
        }

        void OpenALAudioModule::SetEnabled(bool enabled) {
            m_enabled = enabled;
            LOG_INFO("OpenAL Audio Module " + std::string(enabled ? "enabled" : "disabled"));
        }

        AudioEngine* OpenALAudioModule::GetAudioEngine() {
            return m_audioEngine.get();
        }

        bool OpenALAudioModule::SupportsFormat(const std::string& format) const {
            // Convert to lowercase for comparison
            std::string lowerFormat = format;
            std::transform(lowerFormat.begin(), lowerFormat.end(), lowerFormat.begin(), ::tolower);
            
            // OpenAL with our current implementation supports these formats
            return lowerFormat == "wav" || lowerFormat == "ogg" || lowerFormat == "mp3";
        }

        bool OpenALAudioModule::Supports3DAudio() const {
            return true; // OpenAL supports 3D audio
        }

        bool OpenALAudioModule::SupportsStreaming() const {
            return false; // Current implementation doesn't support streaming
        }

        void OpenALAudioModule::SetMasterVolume(float volume) {
            if (m_audioEngine) {
                m_audioEngine->SetMasterVolume(volume);
            }
        }

        void OpenALAudioModule::SetMusicVolume(float volume) {
            if (m_audioEngine) {
                m_audioEngine->SetMusicVolume(volume);
            }
        }

        void OpenALAudioModule::SetSFXVolume(float volume) {
            if (m_audioEngine) {
                m_audioEngine->SetSFXVolume(volume);
            }
        }

        float OpenALAudioModule::GetMasterVolume() const {
            // AudioEngine doesn't expose getters, so we'll track it in config
            return GetConfigFloat("masterVolume", 1.0f);
        }

        float OpenALAudioModule::GetMusicVolume() const {
            return GetConfigFloat("musicVolume", 1.0f);
        }

        float OpenALAudioModule::GetSFXVolume() const {
            return GetConfigFloat("sfxVolume", 1.0f);
        }

        void OpenALAudioModule::SetListenerPosition(const Math::Vec3& position) {
            if (m_audioEngine) {
                m_audioEngine->SetListenerPosition(position);
            }
        }

        void OpenALAudioModule::SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up) {
            if (m_audioEngine) {
                m_audioEngine->SetListenerOrientation(forward, up);
            }
        }

        void OpenALAudioModule::SetListenerVelocity(const Math::Vec3& velocity) {
            if (m_audioEngine) {
                m_audioEngine->SetListenerVelocity(velocity);
            }
        }

        uint32_t OpenALAudioModule::CreateAudioSource() {
            if (m_audioEngine) {
                return m_audioEngine->CreateAudioSource();
            }
            return 0;
        }

        void OpenALAudioModule::DestroyAudioSource(uint32_t sourceId) {
            if (m_audioEngine) {
                m_audioEngine->DestroyAudioSource(sourceId);
            }
        }

        void OpenALAudioModule::PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip) {
            if (m_audioEngine) {
                m_audioEngine->PlayAudioSource(sourceId, clip);
            }
        }

        void OpenALAudioModule::StopAudioSource(uint32_t sourceId) {
            if (m_audioEngine) {
                m_audioEngine->StopAudioSource(sourceId);
            }
        }

        void OpenALAudioModule::PauseAudioSource(uint32_t sourceId) {
            if (m_audioEngine) {
                m_audioEngine->PauseAudioSource(sourceId);
            }
        }

        std::shared_ptr<AudioClip> OpenALAudioModule::LoadAudioClip(const std::string& path) {
            if (m_audioEngine) {
                return m_audioEngine->LoadAudioClip(path);
            }
            return nullptr;
        }

        void OpenALAudioModule::UnloadAudioClip(const std::string& path) {
            if (m_audioEngine) {
                m_audioEngine->UnloadAudioClip(path);
            }
        }

        bool OpenALAudioModule::IsAudioAvailable() const {
            if (m_audioEngine) {
                return m_audioEngine->IsAudioAvailable();
            }
            return false;
        }

        std::string OpenALAudioModule::GetAudioBackendName() const {
            return "OpenAL";
        }

        std::string OpenALAudioModule::GetAudioDeviceName() const {
            if (m_audioEngine && m_audioEngine->IsAudioAvailable()) {
                return "OpenAL Device"; // Generic name since GetDeviceName is private
            }
            return "No audio device";
        }

        void OpenALAudioModule::ApplyConfiguration(const ModuleConfig& config) {
            // Apply volume settings from configuration
            float masterVolume = GetConfigFloat("masterVolume", 1.0f);
            float musicVolume = GetConfigFloat("musicVolume", 1.0f);
            float sfxVolume = GetConfigFloat("sfxVolume", 1.0f);

            SetMasterVolume(masterVolume);
            SetMusicVolume(musicVolume);
            SetSFXVolume(sfxVolume);

            // Apply performance settings
            bool bufferPooling = GetConfigBool("enableBufferPooling", true);
            bool sourcePooling = GetConfigBool("enableSourcePooling", true);
            bool optimized3D = GetConfigBool("enableOptimized3DAudio", true);

            if (m_audioEngine) {
                m_audioEngine->EnableBufferPooling(bufferPooling);
                m_audioEngine->EnableSourcePooling(sourcePooling);
                m_audioEngine->EnableOptimized3DAudio(optimized3D);
            }

            LOG_INFO("Applied OpenAL Audio Module configuration");
        }

        bool OpenALAudioModule::GetConfigBool(const std::string& key, bool defaultValue) const {
            auto it = m_config.parameters.find(key);
            if (it != m_config.parameters.end()) {
                const std::string& value = it->second;
                return value == "true" || value == "1" || value == "yes";
            }
            return defaultValue;
        }

        float OpenALAudioModule::GetConfigFloat(const std::string& key, float defaultValue) const {
            auto it = m_config.parameters.find(key);
            if (it != m_config.parameters.end()) {
                try {
                    return std::stof(it->second);
                } catch (const std::exception&) {
                    LOG_WARNING("Invalid float value for config key '" + key + "': " + it->second);
                }
            }
            return defaultValue;
        }

        std::string OpenALAudioModule::GetConfigString(const std::string& key, const std::string& defaultValue) const {
            auto it = m_config.parameters.find(key);
            if (it != m_config.parameters.end()) {
                return it->second;
            }
            return defaultValue;
        }
    }
}