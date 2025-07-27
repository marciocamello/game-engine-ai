#pragma once

#include "Resource/ModelHotReloader.h"
#include "Core/Math.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>

namespace GameEngine {
    class Model;
    class ModelLoader;
    class ResourceManager;

    /**
     * @brief Development tools and utilities for 3D model workflow
     * 
     * Provides high-level development workflow support including:
     * - Automatic hot-reloading setup
     * - Model validation and optimization
     * - Development-time performance monitoring
     * - Asset pipeline integration
     */
    class ModelDevelopmentTools {
    public:
        /**
         * @brief Configuration for development workflow
         */
        struct DevelopmentConfig {
            bool enableHotReloading = true;
            bool enableValidation = true;
            bool enableOptimization = true;
            bool enablePerformanceMonitoring = true;
            bool autoWatchAssetDirectories = true;
            std::chrono::milliseconds hotReloadInterval = std::chrono::milliseconds(500);
            std::vector<std::string> assetDirectories = {"assets/meshes", "assets/models", "assets/GLTF"};
            std::vector<std::string> watchedExtensions = {"obj", "fbx", "gltf", "glb", "dae"};
        };

        /**
         * @brief Performance metrics for model operations
         */
        struct PerformanceMetrics {
            uint32_t totalModelsLoaded = 0;
            uint32_t totalReloads = 0;
            uint32_t validationFailures = 0;
            float averageLoadTimeMs = 0.0f;
            float averageReloadTimeMs = 0.0f;
            size_t totalMemoryUsage = 0;
            std::chrono::system_clock::time_point lastUpdate;
        };

        /**
         * @brief Model validation result
         */
        struct ValidationResult {
            bool isValid = false;
            std::vector<std::string> warnings;
            std::vector<std::string> errors;
            std::vector<std::string> suggestions;
            
            // Performance metrics
            uint32_t vertexCount = 0;
            uint32_t triangleCount = 0;
            uint32_t materialCount = 0;
            size_t memoryUsage = 0;
            
            // Quality metrics
            float meshComplexity = 0.0f;
            float textureMemoryUsage = 0.0f;
            bool hasOptimizedMeshes = false;
            bool hasValidBounds = false;
        };

    public:
        ModelDevelopmentTools();
        ~ModelDevelopmentTools();

        // Lifecycle
        bool Initialize(std::shared_ptr<ModelLoader> modelLoader, 
                       std::shared_ptr<ResourceManager> resourceManager = nullptr);
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // Configuration
        void SetConfig(const DevelopmentConfig& config);
        const DevelopmentConfig& GetConfig() const { return m_config; }

        // Hot-reloading workflow
        void EnableHotReloading();
        void DisableHotReloading();
        bool IsHotReloadingEnabled() const;
        
        void WatchModel(const std::string& modelPath, std::shared_ptr<Model> model);
        void WatchAssetDirectories();
        void SetReloadCallback(std::function<void(const std::string&, std::shared_ptr<Model>, bool)> callback);

        // Model validation and optimization
        ValidationResult ValidateModel(std::shared_ptr<Model> model);
        ValidationResult ValidateModelFile(const std::string& modelPath);
        void OptimizeModel(std::shared_ptr<Model> model);
        void OptimizeModelFile(const std::string& modelPath);

        // Batch operations
        void ValidateAllWatchedModels();
        void OptimizeAllWatchedModels();
        void ReloadAllWatchedModels();

        // Performance monitoring
        PerformanceMetrics GetPerformanceMetrics() const;
        void ResetPerformanceMetrics();
        void LogPerformanceReport() const;

        // Development utilities
        void PrintModelInfo(std::shared_ptr<Model> model) const;
        void PrintModelFileInfo(const std::string& modelPath) const;
        void PrintWatchedModelsStatus() const;
        void PrintAssetDirectoryStatus() const;

        // Asset pipeline integration
        bool ProcessAssetDirectory(const std::string& directoryPath, bool recursive = true);
        std::vector<std::string> FindProblematicModels(const std::string& directoryPath);
        void GenerateAssetReport(const std::string& outputPath);

    private:
        // Core components
        std::shared_ptr<ModelLoader> m_modelLoader;
        std::shared_ptr<ResourceManager> m_resourceManager;
        std::unique_ptr<ModelHotReloader> m_hotReloader;
        
        // Configuration
        DevelopmentConfig m_config;
        bool m_initialized = false;

        // Performance tracking
        mutable PerformanceMetrics m_metrics;
        mutable std::mutex m_metricsMutex;

        // Internal callbacks
        void OnModelReloaded(const std::string& modelPath, std::shared_ptr<Model> newModel, bool success);
        void UpdatePerformanceMetrics(const std::string& operation, float timeMs, bool success);

        // Validation helpers
        void ValidateMeshes(std::shared_ptr<Model> model, ValidationResult& result) const;
        void ValidateMaterials(std::shared_ptr<Model> model, ValidationResult& result) const;
        void ValidateBounds(std::shared_ptr<Model> model, ValidationResult& result) const;
        void ValidatePerformance(std::shared_ptr<Model> model, ValidationResult& result) const;

        // Optimization helpers
        void OptimizeMeshes(std::shared_ptr<Model> model) const;
        void OptimizeMaterials(std::shared_ptr<Model> model) const;
        void OptimizeBounds(std::shared_ptr<Model> model) const;

        // Utility methods
        std::vector<std::string> FindModelFiles(const std::string& directoryPath, bool recursive) const;
        bool IsAssetDirectory(const std::string& path) const;
        std::string FormatFileSize(size_t bytes) const;
        std::string FormatDuration(float milliseconds) const;
    };

    /**
     * @brief Global development tools instance for engine-wide use
     */
    class GlobalModelDevelopmentTools {
    public:
        static ModelDevelopmentTools& GetInstance();
        
    private:
        static std::unique_ptr<ModelDevelopmentTools> s_instance;
    };
}