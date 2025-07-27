#include "Resource/ModelDevelopmentTools.h"
#include "Resource/ModelLoader.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace GameEngine {

// Global instance implementation
std::unique_ptr<ModelDevelopmentTools> GlobalModelDevelopmentTools::s_instance = nullptr;

ModelDevelopmentTools& GlobalModelDevelopmentTools::GetInstance() {
    if (!s_instance) {
        s_instance = std::make_unique<ModelDevelopmentTools>();
    }
    return *s_instance;
}

ModelDevelopmentTools::ModelDevelopmentTools() {
    // Initialize default configuration
    m_config.enableHotReloading = true;
    m_config.enableValidation = true;
    m_config.enableOptimization = true;
    m_config.enablePerformanceMonitoring = true;
    m_config.autoWatchAssetDirectories = true;
    m_config.hotReloadInterval = std::chrono::milliseconds(500);
    m_config.assetDirectories = {"assets/meshes", "assets/models", "assets/GLTF"};
    m_config.watchedExtensions = {"obj", "fbx", "gltf", "glb", "dae", "3ds", "blend"};
}

ModelDevelopmentTools::~ModelDevelopmentTools() {
    Shutdown();
}

bool ModelDevelopmentTools::Initialize(std::shared_ptr<ModelLoader> modelLoader, 
                                      std::shared_ptr<ResourceManager> resourceManager) {
    if (m_initialized) {
        Logger::GetInstance().Warning("ModelDevelopmentTools already initialized");
        return true;
    }

    if (!modelLoader) {
        Logger::GetInstance().Error("ModelDevelopmentTools::Initialize: ModelLoader is null");
        return false;
    }

    m_modelLoader = modelLoader;
    m_resourceManager = resourceManager;

    // Initialize hot-reloader if enabled
    if (m_config.enableHotReloading) {
        m_hotReloader = std::make_unique<ModelHotReloader>();
        if (!m_hotReloader->Initialize(modelLoader)) {
            Logger::GetInstance().Error("Failed to initialize ModelHotReloader");
            return false;
        }

        // Configure hot-reloader
        ModelHotReloader::HotReloadConfig hotReloadConfig;
        hotReloadConfig.enabled = true;
        hotReloadConfig.pollInterval = m_config.hotReloadInterval;
        hotReloadConfig.validateOnReload = m_config.enableValidation;
        hotReloadConfig.optimizeOnReload = m_config.enableOptimization;
        hotReloadConfig.logReloadEvents = true;
        hotReloadConfig.watchedExtensions = m_config.watchedExtensions;
        
        m_hotReloader->SetConfig(hotReloadConfig);
        m_hotReloader->SetReloadCallback(
            [this](const std::string& path, std::shared_ptr<Model> model, bool success) {
                OnModelReloaded(path, model, success);
            });
    }

    // Initialize performance metrics
    {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        m_metrics = PerformanceMetrics{};
        m_metrics.lastUpdate = std::chrono::system_clock::now();
    }

    m_initialized = true;

    Logger::GetInstance().Info("ModelDevelopmentTools initialized successfully");

    // Auto-watch asset directories if enabled
    if (m_config.autoWatchAssetDirectories) {
        WatchAssetDirectories();
    }

    return true;
}

void ModelDevelopmentTools::Shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_hotReloader) {
        m_hotReloader->Shutdown();
        m_hotReloader.reset();
    }

    m_modelLoader.reset();
    m_resourceManager.reset();
    m_initialized = false;

    Logger::GetInstance().Info("ModelDevelopmentTools shutdown complete");
}

void ModelDevelopmentTools::SetConfig(const DevelopmentConfig& config) {
    m_config = config;

    // Update hot-reloader configuration if it exists
    if (m_hotReloader) {
        ModelHotReloader::HotReloadConfig hotReloadConfig;
        hotReloadConfig.enabled = m_config.enableHotReloading;
        hotReloadConfig.pollInterval = m_config.hotReloadInterval;
        hotReloadConfig.validateOnReload = m_config.enableValidation;
        hotReloadConfig.optimizeOnReload = m_config.enableOptimization;
        hotReloadConfig.watchedExtensions = m_config.watchedExtensions;
        
        m_hotReloader->SetConfig(hotReloadConfig);
    }

    Logger::GetInstance().Info("ModelDevelopmentTools configuration updated");
}

void ModelDevelopmentTools::EnableHotReloading() {
    if (!m_initialized) {
        Logger::GetInstance().Error("ModelDevelopmentTools not initialized");
        return;
    }

    if (!m_hotReloader) {
        Logger::GetInstance().Error("Hot-reloader not available");
        return;
    }

    m_hotReloader->StartWatching();
    Logger::GetInstance().Info("Model hot-reloading enabled");
}

void ModelDevelopmentTools::DisableHotReloading() {
    if (m_hotReloader) {
        m_hotReloader->StopWatching();
        Logger::GetInstance().Info("Model hot-reloading disabled");
    }
}

bool ModelDevelopmentTools::IsHotReloadingEnabled() const {
    return m_hotReloader && m_hotReloader->IsWatching();
}

void ModelDevelopmentTools::WatchModel(const std::string& modelPath, std::shared_ptr<Model> model) {
    if (!m_initialized || !m_hotReloader) {
        Logger::GetInstance().Error("ModelDevelopmentTools or hot-reloader not initialized");
        return;
    }

    m_hotReloader->WatchModel(modelPath, model);
    
    // Start watching if not already started
    if (!m_hotReloader->IsWatching()) {
        m_hotReloader->StartWatching();
    }

    Logger::GetInstance().Info("Now watching model for changes: " + modelPath);
}

void ModelDevelopmentTools::WatchAssetDirectories() {
    if (!m_initialized || !m_hotReloader) {
        Logger::GetInstance().Warning("Cannot watch asset directories - hot-reloader not available");
        return;
    }

    for (const auto& directory : m_config.assetDirectories) {
        if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
            m_hotReloader->WatchDirectory(directory, true);
            Logger::GetInstance().Info("Watching asset directory: " + directory);
        } else {
            Logger::GetInstance().Warning("Asset directory does not exist: " + directory);
        }
    }
}

void ModelDevelopmentTools::SetReloadCallback(std::function<void(const std::string&, std::shared_ptr<Model>, bool)> callback) {
    if (m_hotReloader) {
        m_hotReloader->SetReloadCallback(callback);
    }
}

ModelDevelopmentTools::ValidationResult ModelDevelopmentTools::ValidateModel(std::shared_ptr<Model> model) {
    ValidationResult result;
    
    if (!model) {
        result.errors.push_back("Model is null");
        return result;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Validate meshes
        ValidateMeshes(model, result);
        
        // Validate materials
        ValidateMaterials(model, result);
        
        // Validate bounds
        ValidateBounds(model, result);
        
        // Validate performance characteristics
        ValidatePerformance(model, result);

        // Overall validation result
        result.isValid = result.errors.empty();

        // Generate suggestions based on findings
        if (result.triangleCount > 100000) {
            result.suggestions.push_back("Consider using LOD (Level of Detail) for high-poly models");
        }
        
        if (result.materialCount > 10) {
            result.suggestions.push_back("Consider consolidating materials to reduce draw calls");
        }
        
        if (!result.hasOptimizedMeshes) {
            result.suggestions.push_back("Consider optimizing meshes for better rendering performance");
        }

    } catch (const std::exception& e) {
        result.errors.push_back("Validation exception: " + std::string(e.what()));
        result.isValid = false;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    float validationTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    // Update performance metrics
    if (m_config.enablePerformanceMonitoring) {
        UpdatePerformanceMetrics("validation", validationTime, result.isValid);
        if (!result.isValid) {
            std::lock_guard<std::mutex> lock(m_metricsMutex);
            m_metrics.validationFailures++;
        }
    }

    return result;
}

ModelDevelopmentTools::ValidationResult ModelDevelopmentTools::ValidateModelFile(const std::string& modelPath) {
    ValidationResult result;

    if (!m_modelLoader) {
        result.errors.push_back("ModelLoader not available");
        return result;
    }

    try {
        auto model = m_modelLoader->LoadModelAsResource(modelPath);
        if (!model) {
            result.errors.push_back("Failed to load model from file: " + modelPath);
            return result;
        }

        result = ValidateModel(model);
        
    } catch (const std::exception& e) {
        result.errors.push_back("Exception loading model for validation: " + std::string(e.what()));
        result.isValid = false;
    }

    return result;
}

void ModelDevelopmentTools::OptimizeModel(std::shared_ptr<Model> model) {
    if (!model) {
        Logger::GetInstance().Warning("Cannot optimize null model");
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Optimize meshes
        OptimizeMeshes(model);
        
        // Optimize materials
        OptimizeMaterials(model);
        
        // Optimize bounds
        OptimizeBounds(model);

        auto endTime = std::chrono::high_resolution_clock::now();
        float optimizationTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

        Logger::GetInstance().Info("Model optimization completed in " + 
                                  FormatDuration(optimizationTime));

        // Update performance metrics
        if (m_config.enablePerformanceMonitoring) {
            UpdatePerformanceMetrics("optimization", optimizationTime, true);
        }

    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Model optimization failed: " + std::string(e.what()));
    }
}

void ModelDevelopmentTools::OptimizeModelFile(const std::string& modelPath) {
    if (!m_modelLoader) {
        Logger::GetInstance().Error("ModelLoader not available");
        return;
    }

    try {
        auto model = m_modelLoader->LoadModelAsResource(modelPath);
        if (!model) {
            Logger::GetInstance().Error("Failed to load model for optimization: " + modelPath);
            return;
        }

        OptimizeModel(model);
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Exception optimizing model file: " + std::string(e.what()));
    }
}

void ModelDevelopmentTools::ValidateAllWatchedModels() {
    if (!m_hotReloader) {
        Logger::GetInstance().Warning("Hot-reloader not available");
        return;
    }

    auto watchedModels = m_hotReloader->GetWatchedModels();
    Logger::GetInstance().Info("Validating " + std::to_string(watchedModels.size()) + " watched models");

    uint32_t validModels = 0;
    uint32_t invalidModels = 0;

    for (const auto& watchedModel : watchedModels) {
        if (!watchedModel.modelRef.expired()) {
            auto model = watchedModel.modelRef.lock();
            if (model) {
                auto result = ValidateModel(model);
                if (result.isValid) {
                    validModels++;
                } else {
                    invalidModels++;
                    Logger::GetInstance().Warning("Validation failed for: " + watchedModel.filePath);
                    for (const auto& error : result.errors) {
                        Logger::GetInstance().Warning("  Error: " + error);
                    }
                }
            }
        }
    }

    Logger::GetInstance().Info("Validation complete: " + std::to_string(validModels) + 
                              " valid, " + std::to_string(invalidModels) + " invalid");
}

void ModelDevelopmentTools::OptimizeAllWatchedModels() {
    if (!m_hotReloader) {
        Logger::GetInstance().Warning("Hot-reloader not available");
        return;
    }

    auto watchedModels = m_hotReloader->GetWatchedModels();
    Logger::GetInstance().Info("Optimizing " + std::to_string(watchedModels.size()) + " watched models");

    for (const auto& watchedModel : watchedModels) {
        if (!watchedModel.modelRef.expired()) {
            auto model = watchedModel.modelRef.lock();
            if (model) {
                OptimizeModel(model);
            }
        }
    }

    Logger::GetInstance().Info("Optimization complete for all watched models");
}

void ModelDevelopmentTools::ReloadAllWatchedModels() {
    if (!m_hotReloader) {
        Logger::GetInstance().Warning("Hot-reloader not available");
        return;
    }

    m_hotReloader->ReloadAll();
}

ModelDevelopmentTools::PerformanceMetrics ModelDevelopmentTools::GetPerformanceMetrics() const {
    std::lock_guard<std::mutex> lock(m_metricsMutex);
    return m_metrics;
}

void ModelDevelopmentTools::ResetPerformanceMetrics() {
    std::lock_guard<std::mutex> lock(m_metricsMutex);
    m_metrics = PerformanceMetrics{};
    m_metrics.lastUpdate = std::chrono::system_clock::now();
    
    Logger::GetInstance().Info("Performance metrics reset");
}

void ModelDevelopmentTools::LogPerformanceReport() const {
    auto metrics = GetPerformanceMetrics();
    
    Logger::GetInstance().Info("=== Model Development Performance Report ===");
    Logger::GetInstance().Info("Total models loaded: " + std::to_string(metrics.totalModelsLoaded));
    Logger::GetInstance().Info("Total reloads: " + std::to_string(metrics.totalReloads));
    Logger::GetInstance().Info("Validation failures: " + std::to_string(metrics.validationFailures));
    Logger::GetInstance().Info("Average load time: " + FormatDuration(metrics.averageLoadTimeMs));
    Logger::GetInstance().Info("Average reload time: " + FormatDuration(metrics.averageReloadTimeMs));
    Logger::GetInstance().Info("Total memory usage: " + FormatFileSize(metrics.totalMemoryUsage));
    
    if (m_hotReloader) {
        auto hotReloadStats = m_hotReloader->GetStats();
        Logger::GetInstance().Info("Hot-reload stats:");
        Logger::GetInstance().Info("  Watched files: " + std::to_string(hotReloadStats.totalWatchedFiles));
        Logger::GetInstance().Info("  Successful reloads: " + std::to_string(hotReloadStats.successfulReloads));
        Logger::GetInstance().Info("  Failed reloads: " + std::to_string(hotReloadStats.failedReloads));
        Logger::GetInstance().Info("  Average reload time: " + FormatDuration(hotReloadStats.averageReloadTimeMs));
    }
}

void ModelDevelopmentTools::PrintModelInfo(std::shared_ptr<Model> model) const {
    if (!model) {
        Logger::GetInstance().Info("Model is null");
        return;
    }

    auto stats = model->GetStats();
    
    Logger::GetInstance().Info("=== Model Information ===");
    Logger::GetInstance().Info("Name: " + model->GetName());
    Logger::GetInstance().Info("Path: " + model->GetPath());
    Logger::GetInstance().Info("Meshes: " + std::to_string(stats.meshCount));
    Logger::GetInstance().Info("Materials: " + std::to_string(stats.materialCount));
    Logger::GetInstance().Info("Animations: " + std::to_string(stats.animationCount));
    Logger::GetInstance().Info("Vertices: " + std::to_string(stats.totalVertices));
    Logger::GetInstance().Info("Triangles: " + std::to_string(stats.totalTriangles));
    Logger::GetInstance().Info("Memory usage: " + FormatFileSize(model->GetMemoryUsage()));
    
    auto bounds = model->GetBoundingBox();
    Logger::GetInstance().Info("Bounding box: (" + 
                              std::to_string(bounds.min.x) + ", " + std::to_string(bounds.min.y) + ", " + std::to_string(bounds.min.z) + ") to (" +
                              std::to_string(bounds.max.x) + ", " + std::to_string(bounds.max.y) + ", " + std::to_string(bounds.max.z) + ")");
}

void ModelDevelopmentTools::PrintModelFileInfo(const std::string& modelPath) const {
    if (!m_modelLoader) {
        Logger::GetInstance().Error("ModelLoader not available");
        return;
    }

    try {
        auto model = m_modelLoader->LoadModelAsResource(modelPath);
        if (model) {
            PrintModelInfo(model);
        } else {
            Logger::GetInstance().Error("Failed to load model: " + modelPath);
        }
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("Exception loading model info: " + std::string(e.what()));
    }
}

void ModelDevelopmentTools::PrintWatchedModelsStatus() const {
    if (!m_hotReloader) {
        Logger::GetInstance().Warning("Hot-reloader not available");
        return;
    }

    m_hotReloader->PrintWatchedModels();
}

void ModelDevelopmentTools::PrintAssetDirectoryStatus() const {
    Logger::GetInstance().Info("=== Asset Directory Status ===");
    
    for (const auto& directory : m_config.assetDirectories) {
        if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
            auto modelFiles = FindModelFiles(directory, true);
            Logger::GetInstance().Info(directory + ": " + std::to_string(modelFiles.size()) + " model files");
        } else {
            Logger::GetInstance().Info(directory + ": Directory not found");
        }
    }
}

bool ModelDevelopmentTools::ProcessAssetDirectory(const std::string& directoryPath, bool recursive) {
    if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
        Logger::GetInstance().Error("Asset directory does not exist: " + directoryPath);
        return false;
    }

    auto modelFiles = FindModelFiles(directoryPath, recursive);
    Logger::GetInstance().Info("Processing " + std::to_string(modelFiles.size()) + 
                              " model files in: " + directoryPath);

    uint32_t processedFiles = 0;
    uint32_t validFiles = 0;
    uint32_t invalidFiles = 0;

    for (const auto& filePath : modelFiles) {
        try {
            auto result = ValidateModelFile(filePath);
            processedFiles++;
            
            if (result.isValid) {
                validFiles++;
            } else {
                invalidFiles++;
                Logger::GetInstance().Warning("Issues found in: " + filePath);
                for (const auto& error : result.errors) {
                    Logger::GetInstance().Warning("  Error: " + error);
                }
            }
            
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("Exception processing " + filePath + ": " + e.what());
            invalidFiles++;
        }
    }

    Logger::GetInstance().Info("Asset directory processing complete:");
    Logger::GetInstance().Info("  Processed: " + std::to_string(processedFiles));
    Logger::GetInstance().Info("  Valid: " + std::to_string(validFiles));
    Logger::GetInstance().Info("  Invalid: " + std::to_string(invalidFiles));

    return invalidFiles == 0;
}

std::vector<std::string> ModelDevelopmentTools::FindProblematicModels(const std::string& directoryPath) {
    std::vector<std::string> problematicModels;
    
    if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
        return problematicModels;
    }

    auto modelFiles = FindModelFiles(directoryPath, true);
    
    for (const auto& filePath : modelFiles) {
        try {
            auto result = ValidateModelFile(filePath);
            if (!result.isValid) {
                problematicModels.push_back(filePath);
            }
        } catch (const std::exception&) {
            problematicModels.push_back(filePath);
        }
    }

    return problematicModels;
}

void ModelDevelopmentTools::GenerateAssetReport(const std::string& outputPath) {
    std::ofstream report(outputPath);
    if (!report.is_open()) {
        Logger::GetInstance().Error("Failed to create asset report: " + outputPath);
        return;
    }

    report << "# Model Asset Report\n\n";
    report << "Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n\n";

    // Performance metrics
    auto metrics = GetPerformanceMetrics();
    report << "## Performance Metrics\n\n";
    report << "- Total models loaded: " << metrics.totalModelsLoaded << "\n";
    report << "- Total reloads: " << metrics.totalReloads << "\n";
    report << "- Validation failures: " << metrics.validationFailures << "\n";
    report << "- Average load time: " << std::fixed << std::setprecision(2) << metrics.averageLoadTimeMs << "ms\n";
    report << "- Average reload time: " << std::fixed << std::setprecision(2) << metrics.averageReloadTimeMs << "ms\n";
    report << "- Total memory usage: " << FormatFileSize(metrics.totalMemoryUsage) << "\n\n";

    // Asset directories
    report << "## Asset Directories\n\n";
    for (const auto& directory : m_config.assetDirectories) {
        if (std::filesystem::exists(directory)) {
            auto modelFiles = FindModelFiles(directory, true);
            report << "### " << directory << "\n";
            report << "- Model files: " << modelFiles.size() << "\n";
            
            // Find problematic models
            auto problematic = FindProblematicModels(directory);
            if (!problematic.empty()) {
                report << "- Problematic models: " << problematic.size() << "\n";
                for (const auto& model : problematic) {
                    report << "  - " << model << "\n";
                }
            }
            report << "\n";
        }
    }

    // Hot-reload statistics
    if (m_hotReloader) {
        auto hotReloadStats = m_hotReloader->GetStats();
        report << "## Hot-Reload Statistics\n\n";
        report << "- Watched files: " << hotReloadStats.totalWatchedFiles << "\n";
        report << "- Successful reloads: " << hotReloadStats.successfulReloads << "\n";
        report << "- Failed reloads: " << hotReloadStats.failedReloads << "\n";
        report << "- Total reload attempts: " << hotReloadStats.totalReloadAttempts << "\n";
        report << "- Average reload time: " << std::fixed << std::setprecision(2) << hotReloadStats.averageReloadTimeMs << "ms\n\n";
    }

    report.close();
    Logger::GetInstance().Info("Asset report generated: " + outputPath);
}

// Private methods

void ModelDevelopmentTools::OnModelReloaded(const std::string& modelPath, std::shared_ptr<Model> newModel, bool success) {
    if (m_config.enablePerformanceMonitoring) {
        std::lock_guard<std::mutex> lock(m_metricsMutex);
        m_metrics.totalReloads++;
    }

    if (success && newModel && m_config.enableValidation) {
        // Validate the reloaded model
        auto result = ValidateModel(newModel);
        if (!result.isValid) {
            Logger::GetInstance().Warning("Reloaded model failed validation: " + modelPath);
            for (const auto& error : result.errors) {
                Logger::GetInstance().Warning("  Error: " + error);
            }
        }
    }
}

void ModelDevelopmentTools::UpdatePerformanceMetrics(const std::string& operation, float timeMs, bool success) {
    std::lock_guard<std::mutex> lock(m_metricsMutex);
    
    if (operation == "load") {
        m_metrics.totalModelsLoaded++;
        // Update average load time
        float totalTime = m_metrics.averageLoadTimeMs * (m_metrics.totalModelsLoaded - 1);
        m_metrics.averageLoadTimeMs = (totalTime + timeMs) / m_metrics.totalModelsLoaded;
    } else if (operation == "reload") {
        // Average reload time is handled by the hot-reloader
    }
    
    m_metrics.lastUpdate = std::chrono::system_clock::now();
}

// Validation helpers

void ModelDevelopmentTools::ValidateMeshes(std::shared_ptr<Model> model, ValidationResult& result) const {
    auto meshes = model->GetMeshes();
    
    if (meshes.empty()) {
        result.errors.push_back("Model has no meshes");
        return;
    }

    for (size_t i = 0; i < meshes.size(); ++i) {
        const auto& mesh = meshes[i];
        if (!mesh) {
            result.errors.push_back("Mesh " + std::to_string(i) + " is null");
            continue;
        }

        uint32_t vertexCount = mesh->GetVertexCount();
        uint32_t triangleCount = mesh->GetTriangleCount();

        result.vertexCount += vertexCount;
        result.triangleCount += triangleCount;

        if (vertexCount == 0) {
            result.errors.push_back("Mesh " + std::to_string(i) + " has no vertices");
        }

        if (triangleCount == 0) {
            result.warnings.push_back("Mesh " + std::to_string(i) + " has no triangles");
        }

        // Check for reasonable vertex/triangle counts
        if (vertexCount > 1000000) {
            result.warnings.push_back("Mesh " + std::to_string(i) + " has very high vertex count: " + std::to_string(vertexCount));
        }

        if (triangleCount > 500000) {
            result.warnings.push_back("Mesh " + std::to_string(i) + " has very high triangle count: " + std::to_string(triangleCount));
        }
    }

    result.memoryUsage += model->GetMemoryUsage();
}

void ModelDevelopmentTools::ValidateMaterials(std::shared_ptr<Model> model, ValidationResult& result) const {
    auto materials = model->GetMaterials();
    result.materialCount = static_cast<uint32_t>(materials.size());

    if (materials.empty()) {
        result.warnings.push_back("Model has no materials");
    }

    // Additional material validation could be added here
}

void ModelDevelopmentTools::ValidateBounds(std::shared_ptr<Model> model, ValidationResult& result) const {
    try {
        auto bounds = model->GetBoundingBox();
        
        // Check if bounds are valid (not infinite or NaN)
        bool validBounds = true;
        if (std::isinf(bounds.min.x) || std::isinf(bounds.min.y) || std::isinf(bounds.min.z) ||
            std::isinf(bounds.max.x) || std::isinf(bounds.max.y) || std::isinf(bounds.max.z) ||
            std::isnan(bounds.min.x) || std::isnan(bounds.min.y) || std::isnan(bounds.min.z) ||
            std::isnan(bounds.max.x) || std::isnan(bounds.max.y) || std::isnan(bounds.max.z)) {
            validBounds = false;
        }

        result.hasValidBounds = validBounds;
        
        if (!validBounds) {
            result.errors.push_back("Model has invalid bounding box");
        }

    } catch (const std::exception& e) {
        result.errors.push_back("Exception validating bounds: " + std::string(e.what()));
        result.hasValidBounds = false;
    }
}

void ModelDevelopmentTools::ValidatePerformance(std::shared_ptr<Model> model, ValidationResult& result) const {
    // Calculate mesh complexity score
    if (result.triangleCount > 0) {
        result.meshComplexity = static_cast<float>(result.triangleCount) / 1000.0f; // Normalized to thousands
    }

    // Estimate texture memory usage (simplified)
    result.textureMemoryUsage = static_cast<float>(result.materialCount) * 2.0f; // Rough estimate in MB

    // Check if meshes appear to be optimized (simplified check)
    result.hasOptimizedMeshes = result.triangleCount > 0 && result.vertexCount > 0;
}

// Optimization helpers

void ModelDevelopmentTools::OptimizeMeshes(std::shared_ptr<Model> model) const {
    // Update bounding volumes
    model->UpdateBounds();
    
    // Additional mesh optimization could be added here
    // For example: vertex cache optimization, mesh simplification, etc.
}

void ModelDevelopmentTools::OptimizeMaterials(std::shared_ptr<Model> model) const {
    // Material optimization could be added here
    // For example: texture compression, material consolidation, etc.
}

void ModelDevelopmentTools::OptimizeBounds(std::shared_ptr<Model> model) const {
    // Ensure bounds are up to date
    model->UpdateBounds();
}

// Utility methods

std::vector<std::string> ModelDevelopmentTools::FindModelFiles(const std::string& directoryPath, bool recursive) const {
    std::vector<std::string> modelFiles;
    
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    std::string filePath = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    
                    // Remove leading dot and convert to lowercase
                    if (!extension.empty() && extension[0] == '.') {
                        extension = extension.substr(1);
                    }
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    
                    if (std::find(m_config.watchedExtensions.begin(), m_config.watchedExtensions.end(), extension) 
                        != m_config.watchedExtensions.end()) {
                        modelFiles.push_back(filePath);
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    std::string filePath = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    
                    // Remove leading dot and convert to lowercase
                    if (!extension.empty() && extension[0] == '.') {
                        extension = extension.substr(1);
                    }
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    
                    if (std::find(m_config.watchedExtensions.begin(), m_config.watchedExtensions.end(), extension) 
                        != m_config.watchedExtensions.end()) {
                        modelFiles.push_back(filePath);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::GetInstance().Warning("Exception finding model files in " + directoryPath + ": " + e.what());
    }
    
    return modelFiles;
}

bool ModelDevelopmentTools::IsAssetDirectory(const std::string& path) const {
    return std::find(m_config.assetDirectories.begin(), m_config.assetDirectories.end(), path) 
           != m_config.assetDirectories.end();
}

std::string ModelDevelopmentTools::FormatFileSize(size_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}

std::string ModelDevelopmentTools::FormatDuration(float milliseconds) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << milliseconds << "ms";
    return oss.str();
}

} // namespace GameEngine