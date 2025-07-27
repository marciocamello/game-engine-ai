#pragma once

#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <filesystem>

namespace GameEngine {
    class Model;
    class ModelLoader;

    /**
     * @brief Hot-reloading system for 3D models during development
     * 
     * Monitors model files for changes and automatically reloads them,
     * providing seamless development workflow with real-time updates.
     */
    class ModelHotReloader {
    public:
        /**
         * @brief Callback function type for model reload events
         * @param modelPath Path to the reloaded model
         * @param newModel Newly loaded model (nullptr if loading failed)
         * @param success Whether the reload was successful
         */
        using ReloadCallback = std::function<void(const std::string& modelPath, std::shared_ptr<Model> newModel, bool success)>;

        /**
         * @brief Configuration for hot-reloading behavior
         */
        struct HotReloadConfig {
            bool enabled = true;
            std::chrono::milliseconds pollInterval = std::chrono::milliseconds(500);
            bool validateOnReload = true;
            bool optimizeOnReload = true;
            bool clearCacheOnReload = true;
            bool logReloadEvents = true;
            std::vector<std::string> watchedExtensions = {"obj", "fbx", "gltf", "glb", "dae"};
            std::vector<std::string> ignoredDirectories = {"cache", "temp", ".git", ".kiro"};
        };

        /**
         * @brief Statistics for hot-reloading operations
         */
        struct HotReloadStats {
            uint32_t totalWatchedFiles = 0;
            uint32_t successfulReloads = 0;
            uint32_t failedReloads = 0;
            uint32_t totalReloadAttempts = 0;
            float averageReloadTimeMs = 0.0f;
            std::chrono::system_clock::time_point lastReloadTime;
        };

        /**
         * @brief Information about a watched model file
         */
        struct WatchedModel {
            std::string filePath;
            std::weak_ptr<Model> modelRef;
            std::chrono::system_clock::time_point lastModified;
            std::chrono::system_clock::time_point lastChecked;
            size_t fileSize = 0;
            bool isValid = true;
            uint32_t reloadCount = 0;
        };

    public:
        ModelHotReloader();
        ~ModelHotReloader();

        // Lifecycle
        bool Initialize(std::shared_ptr<ModelLoader> modelLoader);
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // File watching
        void WatchModel(const std::string& modelPath, std::shared_ptr<Model> model);
        void UnwatchModel(const std::string& modelPath);
        void WatchDirectory(const std::string& directoryPath, bool recursive = true);
        void UnwatchDirectory(const std::string& directoryPath);

        // Hot-reloading control
        void StartWatching();
        void StopWatching();
        bool IsWatching() const { return m_isWatching; }
        void TriggerReload(const std::string& modelPath);
        void ReloadAll();

        // Configuration
        void SetConfig(const HotReloadConfig& config);
        const HotReloadConfig& GetConfig() const { return m_config; }
        void SetReloadCallback(ReloadCallback callback);

        // Statistics and monitoring
        HotReloadStats GetStats() const;
        std::vector<WatchedModel> GetWatchedModels() const;
        void PrintWatchedModels() const;

        // Development utilities
        void ValidateWatchedModels();
        void OptimizeWatchedModels();
        void ClearCacheForWatchedModels();

    private:
        // Core components
        std::shared_ptr<ModelLoader> m_modelLoader;
        HotReloadConfig m_config;
        ReloadCallback m_reloadCallback;

        // File watching state
        std::unordered_map<std::string, WatchedModel> m_watchedModels;
        std::unordered_set<std::string> m_watchedDirectories;
        mutable std::mutex m_watchedModelsMutex;

        // Threading
        std::unique_ptr<std::thread> m_watchThread;
        std::atomic<bool> m_initialized{false};
        std::atomic<bool> m_isWatching{false};
        std::atomic<bool> m_shouldStop{false};

        // Statistics
        mutable HotReloadStats m_stats;
        mutable std::mutex m_statsMutex;

        // Internal methods
        void WatchThreadFunction();
        void CheckForChanges();
        bool HasFileChanged(const WatchedModel& watchedModel) const;
        void ReloadModel(const std::string& modelPath);
        void UpdateWatchedModel(const std::string& modelPath, const std::filesystem::file_time_type& modTime, size_t fileSize);
        void CleanupInvalidWatches();

        // File system utilities
        std::chrono::system_clock::time_point GetFileModificationTime(const std::string& path) const;
        size_t GetFileSize(const std::string& path) const;
        bool IsModelFile(const std::string& path) const;
        bool ShouldIgnoreDirectory(const std::string& path) const;
        std::vector<std::string> FindModelFilesInDirectory(const std::string& directoryPath, bool recursive) const;

        // Validation and optimization
        bool ValidateModel(std::shared_ptr<Model> model) const;
        void OptimizeModel(std::shared_ptr<Model> model) const;
        void LogReloadEvent(const std::string& modelPath, bool success, float reloadTimeMs) const;
    };

    /**
     * @brief Global hot-reloader instance for engine-wide use
     */
    class GlobalModelHotReloader {
    public:
        static ModelHotReloader& GetInstance();
        
    private:
        static std::unique_ptr<ModelHotReloader> s_instance;
    };
}