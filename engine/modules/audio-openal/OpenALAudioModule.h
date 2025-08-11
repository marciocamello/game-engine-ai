#pragma once

#include "../../interfaces/IAudioModule.h"
#include "Audio/AudioEngine.h"
#include <memory>

namespace GameEngine {
    namespace Audio {
        class OpenALAudioModule : public IAudioModule {
        public:
            OpenALAudioModule();
            virtual ~OpenALAudioModule();

            // IEngineModule interface
            bool Initialize(const ModuleConfig& config) override;
            void Update(float deltaTime) override;
            void Shutdown() override;

            const char* GetName() const override;
            const char* GetVersion() const override;
            ModuleType GetType() const override;
            std::vector<std::string> GetDependencies() const override;

            bool IsInitialized() const override;
            bool IsEnabled() const override;
            void SetEnabled(bool enabled) override;

            // IAudioModule interface
            AudioEngine* GetAudioEngine() override;

            bool SupportsFormat(const std::string& format) const override;
            bool Supports3DAudio() const override;
            bool SupportsStreaming() const override;

            void SetMasterVolume(float volume) override;
            void SetMusicVolume(float volume) override;
            void SetSFXVolume(float volume) override;
            float GetMasterVolume() const override;
            float GetMusicVolume() const override;
            float GetSFXVolume() const override;

            void SetListenerPosition(const Math::Vec3& position) override;
            void SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up) override;
            void SetListenerVelocity(const Math::Vec3& velocity) override;

            uint32_t CreateAudioSource() override;
            void DestroyAudioSource(uint32_t sourceId) override;
            void PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip) override;
            void StopAudioSource(uint32_t sourceId) override;
            void PauseAudioSource(uint32_t sourceId) override;

            std::shared_ptr<AudioClip> LoadAudioClip(const std::string& path) override;
            void UnloadAudioClip(const std::string& path) override;

            bool IsAudioAvailable() const override;
            std::string GetAudioBackendName() const override;
            std::string GetAudioDeviceName() const override;

        private:
            std::unique_ptr<AudioEngine> m_audioEngine;
            bool m_initialized = false;
            bool m_enabled = true;
            ModuleConfig m_config;

            // Configuration helpers
            void ApplyConfiguration(const ModuleConfig& config);
            bool GetConfigBool(const std::string& key, bool defaultValue) const;
            float GetConfigFloat(const std::string& key, float defaultValue) const;
            std::string GetConfigString(const std::string& key, const std::string& defaultValue) const;
        };
    }
}