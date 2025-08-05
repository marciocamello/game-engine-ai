#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <vector>

namespace GameEngine {
    
    struct WatchedFile {
        std::string filepath;
        std::filesystem::file_time_type lastWriteTime;
        bool needsReload = false;
    };

    class ShaderHotReloader {
    public:
        // Lifecycle
        bool Initialize();
        void Shutdown();
        void Update();

        // File watching
        void WatchShaderDirectory(const std::string& directory);
        void WatchShaderFile(const std::string& filepath);
        void UnwatchShaderFile(const std::string& filepath);

        // Callbacks
        void SetReloadCallback(std::function<void(const std::string&)> callback);
        void SetErrorCallback(std::function<void(const std::string&, const std::string&)> callback);

        // Manual reload
        void ReloadShader(const std::string& filepath);
        void ReloadAllShaders();

        // Configuration
        void SetEnabled(bool enabled);
        bool IsEnabled() const { return m_enabled; }
        void SetCheckInterval(float intervalSeconds);
        float GetCheckInterval() const { return m_checkInterval; }

        // Status and debugging
        size_t GetWatchedFileCount() const { return m_watchedFiles.size(); }
        std::vector<std::string> GetWatchedFiles() const;
        bool IsFileWatched(const std::string& filepath) const;

    private:
        std::unordered_map<std::string, WatchedFile> m_watchedFiles;
        std::function<void(const std::string&)> m_reloadCallback;
        std::function<void(const std::string&, const std::string&)> m_errorCallback;

        bool m_enabled = false;
        bool m_initialized = false;
        float m_checkInterval = 0.5f;
        float m_timeSinceLastCheck = 0.0f;

        void CheckFileChanges();
        bool HasFileChanged(const WatchedFile& file);
        void UpdateFileTimestamp(const std::string& filepath);
        void ProcessDirectoryRecursively(const std::string& directory);
        bool IsShaderFile(const std::string& filepath) const;
    };
}