#pragma once

#include "Core/IEngineModule.h"
#include "Core/Math.h"
#include <memory>
#include <string>

namespace GameEngine {
    class AudioEngine;
    struct AudioClip;

    namespace Audio {
        class IAudioModule : public IEngineModule {
        public:
            virtual ~IAudioModule() = default;

            // Audio engine access
            virtual AudioEngine* GetAudioEngine() = 0;

            // Audio capabilities
            virtual bool SupportsFormat(const std::string& format) const = 0;
            virtual bool Supports3DAudio() const = 0;
            virtual bool SupportsStreaming() const = 0;

            // Audio settings
            virtual void SetMasterVolume(float volume) = 0;
            virtual void SetMusicVolume(float volume) = 0;
            virtual void SetSFXVolume(float volume) = 0;
            virtual float GetMasterVolume() const = 0;
            virtual float GetMusicVolume() const = 0;
            virtual float GetSFXVolume() const = 0;

            // Listener management
            virtual void SetListenerPosition(const Math::Vec3& position) = 0;
            virtual void SetListenerOrientation(const Math::Vec3& forward, const Math::Vec3& up) = 0;
            virtual void SetListenerVelocity(const Math::Vec3& velocity) = 0;

            // Audio source management
            virtual uint32_t CreateAudioSource() = 0;
            virtual void DestroyAudioSource(uint32_t sourceId) = 0;
            virtual void PlayAudioSource(uint32_t sourceId, std::shared_ptr<AudioClip> clip) = 0;
            virtual void StopAudioSource(uint32_t sourceId) = 0;
            virtual void PauseAudioSource(uint32_t sourceId) = 0;

            // Audio clip management
            virtual std::shared_ptr<AudioClip> LoadAudioClip(const std::string& path) = 0;
            virtual void UnloadAudioClip(const std::string& path) = 0;

            // Performance and diagnostics
            virtual bool IsAudioAvailable() const = 0;
            virtual std::string GetAudioBackendName() const = 0;
            virtual std::string GetAudioDeviceName() const = 0;
        };
    }
}