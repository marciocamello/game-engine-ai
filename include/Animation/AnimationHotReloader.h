#pragma once

#include "Animation/AnimationSerialization.h"
#include "Animation/SkeletalAnimation.h"
#include "Animation/AnimationStateMachine.h"
#include "Animation/BlendTree.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <vector>
#include <memory>

namespace GameEngine {
namespace Animation {

    /**
     * Watched animation file information
     */
    struct WatchedAnimationFile {
        std::string filepath;
        std::string assetType; // "skeletal_animation", "state_machine", "blend_tree"
        std::filesystem::file_time_type lastWriteTime;
        bool needsReload = false;
        bool isValid = true;
        std::string lastError;
    };

    /**
     * Animation asset validation result
     */
    struct AnimationValidationResult {
        bool isValid = false;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        size_t fileSize = 0;
        std::string assetType;
        std::string version;
    };

    /**
     * Animation hot-reloading system for development workflow
     */
    class AnimationHotReloader {
    public:
        // Callback types
        using ReloadCallback = std::function<void(const std::string& filepath, const std::string& assetType)>;
        using ErrorCallback = std::function<void(const std::string& filepath, const std::string& error)>;
        using ValidationCallback = std::function<void(const std::string& filepath, const AnimationValidationResult& result)>;

        // Lifecycle
        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // File watching
        void WatchAnimationDirectory(const std::string& directory);
        void WatchAnimationFile(const std::string& filepath);
        void UnwatchAnimationFile(const std::string& filepath);
        void ClearWatchedFiles();

        // Callbacks
        void SetReloadCallback(ReloadCallback callback) { m_reloadCallback = callback; }
        void SetErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }
        void SetValidationCallback(ValidationCallback callback) { m_validationCallback = callback; }

        // Manual operations
        void ReloadAnimation(const std::string& filepath);
        void ReloadAllAnimations();
        void ValidateAnimation(const std::string& filepath);
        void ValidateAllAnimations();

        // Configuration
        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }
        
        void SetCheckInterval(float intervalSeconds) { m_checkInterval = intervalSeconds; }
        float GetCheckInterval() const { return m_checkInterval; }
        
        void SetAutoValidation(bool enabled) { m_autoValidation = enabled; }
        bool IsAutoValidationEnabled() const { return m_autoValidation; }

        // Asset optimization
        void OptimizeAnimation(const std::string& filepath);
        void OptimizeAllAnimations();
        void SetOptimizationEnabled(bool enabled) { m_optimizationEnabled = enabled; }
        bool IsOptimizationEnabled() const { return m_optimizationEnabled; }

        // Status and debugging
        size_t GetWatchedFileCount() const { return m_watchedFiles.size(); }
        std::vector<std::string> GetWatchedFiles() const;
        std::vector<std::string> GetInvalidFiles() const;
        bool IsFileWatched(const std::string& filepath) const;
        bool IsFileValid(const std::string& filepath) const;
        std::string GetFileError(const std::string& filepath) const;

        // Asset information
        std::string GetAssetType(const std::string& filepath) const;
        AnimationValidationResult GetValidationResult(const std::string& filepath) const;

        // Development tools
        void GenerateAssetReport(const std::string& outputPath) const;
        void ExportAssetStatistics(const std::string& outputPath) const;

    private:
        std::unordered_map<std::string, WatchedAnimationFile> m_watchedFiles;
        std::unordered_map<std::string, AnimationValidationResult> m_validationResults;
        
        ReloadCallback m_reloadCallback;
        ErrorCallback m_errorCallback;
        ValidationCallback m_validationCallback;

        bool m_enabled = false;
        bool m_initialized = false;
        bool m_autoValidation = true;
        bool m_optimizationEnabled = false;
        
        float m_checkInterval = 1.0f; // Check every second for animations (less frequent than shaders)
        float m_timeSinceLastCheck = 0.0f;

        // File monitoring
        void CheckFileChanges();
        bool HasFileChanged(const WatchedAnimationFile& file);
        void UpdateFileTimestamp(const std::string& filepath);
        void ProcessDirectoryRecursively(const std::string& directory);
        bool IsAnimationFile(const std::string& filepath) const;
        std::string DetectAssetType(const std::string& filepath) const;

        // Asset processing
        AnimationValidationResult ValidateAnimationFile(const std::string& filepath);
        bool OptimizeAnimationFile(const std::string& filepath);
        void ProcessReloadedFile(const std::string& filepath);

        // Utility methods
        std::string GetFileExtension(const std::string& filepath) const;
        std::string GetRelativePath(const std::string& filepath) const;
        void LogReloadEvent(const std::string& filepath, const std::string& assetType);
        void LogValidationResult(const std::string& filepath, const AnimationValidationResult& result);
    };

    /**
     * Animation development workflow manager
     */
    class AnimationDevelopmentWorkflow {
    public:
        // Lifecycle
        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // Workflow configuration
        void SetProjectDirectory(const std::string& directory);
        void SetOutputDirectory(const std::string& directory);
        void SetSourceDirectory(const std::string& directory);

        // Asset pipeline integration
        void RegisterAssetImporter(const std::string& extension, 
                                 std::function<bool(const std::string&, const std::string&)> importer);
        void ImportAsset(const std::string& sourcePath, const std::string& outputPath);
        void ImportAllAssets();

        // Development tools
        void StartLivePreview();
        void StopLivePreview();
        bool IsLivePreviewActive() const { return m_livePreviewActive; }

        void EnableAssetWatching(bool enabled);
        bool IsAssetWatchingEnabled() const { return m_assetWatchingEnabled; }

        // Asset validation and optimization
        void RunAssetValidation();
        void RunAssetOptimization();
        void GenerateAssetReport();

        // Statistics and monitoring
        struct WorkflowStatistics {
            size_t totalAssets = 0;
            size_t validAssets = 0;
            size_t invalidAssets = 0;
            size_t optimizedAssets = 0;
            size_t reloadedAssets = 0;
            float totalProcessingTime = 0.0f;
            std::string lastUpdate;
        };

        WorkflowStatistics GetStatistics() const { return m_statistics; }
        void ResetStatistics();

    private:
        AnimationHotReloader m_hotReloader;
        
        std::string m_projectDirectory;
        std::string m_outputDirectory;
        std::string m_sourceDirectory;
        
        std::unordered_map<std::string, std::function<bool(const std::string&, const std::string&)>> m_assetImporters;
        
        bool m_initialized = false;
        bool m_livePreviewActive = false;
        bool m_assetWatchingEnabled = true;
        
        WorkflowStatistics m_statistics;

        // Callback handlers
        void OnAnimationReloaded(const std::string& filepath, const std::string& assetType);
        void OnAnimationError(const std::string& filepath, const std::string& error);
        void OnAnimationValidated(const std::string& filepath, const AnimationValidationResult& result);

        // Asset processing
        void ProcessAssetChange(const std::string& filepath);
        void UpdateStatistics();
        std::string GetCurrentTimeString() const;
    };

} // namespace Animation
} // namespace GameEngine