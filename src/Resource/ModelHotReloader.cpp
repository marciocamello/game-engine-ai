#include "Resource/ModelHotReloader.h"
#include "Resource/ModelLoader.h"
#include "Graphics/Model.h"
#include "Core/Logger.h"
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <fstream>

namespace GameEngine {

// Global instance implementation
std::unique_ptr<ModelHotReloader> GlobalModelHotReloader::s_instance = nullptr;

ModelHotReloader& GlobalModelHotReloader::GetInstance() {
    if (!s_instance) {
        s_instance = std::make_unique<ModelHotReloader>();
    }
    return *s_instance;
}

ModelHotReloader::ModelHotReloader() {
    // Initialize default configuration
    m_config.enabled = true;
    m_config.pollInterval = std::chrono::milliseconds(500);
    m_config.validateOnReload = true;
    m_config.optimizeOnReload = true;
    m_config.clearCacheOnReload = true;
    m_config.logReloadEvents = true;
    m_config.watchedExtensions = {"obj", "fbx", "gltf", "glb", "dae", "3ds", "blend"};
    m_config.ignoredDirectories = {"cache", "temp", ".git", ".kiro", "build", "vcpkg"};
}

ModelHotReloader::~ModelHotReloader() {
    Shutdown();
}

bool ModelHotReloader::Initialize(std::shared_ptr<ModelLoader> modelLoader) {
    if (m_initialized) {
        Logger::GetInstance().Warning("ModelHotReloader already initialized");
        return true;
    }

    if (!modelLoader) {
        Logger::GetInstance().Error("ModelHotReloader::Initialize: ModelLoader is null");
        return false;
    }

    m_modelLoader = modelLoader;
    m_initialized = true;

    Logger::GetInstance().Info("ModelHotReloader initialized successfully");
    return true;
}

void ModelHotReloader::Shutdown() {
    if (!m_initialized) {
        return;
    }

    StopWatching();
    
    {
        std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
        m_watchedModels.clear();
        m_watchedDirectories.clear();
    }

    m_modelLoader.reset();
    m_initialized = false;

    Logger::GetInstance().Info("ModelHotReloader shutdown complete");
}

void ModelHotReloader::WatchModel(const std::string& modelPath, std::shared_ptr<Model> model) {
    if (!m_initialized) {
        Logger::GetInstance().Error("ModelHotReloader not initialized");
        return;
    }

    if (!std::filesystem::exists(modelPath)) {
        Logger::GetInstance().Warning("ModelHotReloader::WatchModel: File does not exist: " + modelPath);
        return;
    }

    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    WatchedModel watchedModel;
    watchedModel.filePath = std::filesystem::absolute(modelPath).string();
    watchedModel.modelRef = model;
    watchedModel.lastModified = GetFileModificationTime(watchedModel.filePath);
    watchedModel.lastChecked = std::chrono::system_clock::now();
    watchedModel.fileSize = GetFileSize(watchedModel.filePath);
    watchedModel.isValid = true;
    watchedModel.reloadCount = 0;

    m_watchedModels[watchedModel.filePath] = watchedModel;
    
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.totalWatchedFiles = static_cast<uint32_t>(m_watchedModels.size());
    }

    if (m_config.logReloadEvents) {
        Logger::GetInstance().Info("Now watching model: " + watchedModel.filePath);
    }
}

void ModelHotReloader::UnwatchModel(const std::string& modelPath) {
    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    std::string absolutePath = std::filesystem::absolute(modelPath).string();
    auto it = m_watchedModels.find(absolutePath);
    if (it != m_watchedModels.end()) {
        m_watchedModels.erase(it);
        
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalWatchedFiles = static_cast<uint32_t>(m_watchedModels.size());
        }

        if (m_config.logReloadEvents) {
            Logger::GetInstance().Info("Stopped watching model: " + absolutePath);
        }
    }
}

void ModelHotReloader::WatchDirectory(const std::string& directoryPath, bool recursive) {
    if (!m_initialized) {
        Logger::GetInstance().Error("ModelHotReloader not initialized");
        return;
    }

    if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
        Logger::GetInstance().Warning("ModelHotReloader::WatchDirectory: Directory does not exist: " + directoryPath);
        return;
    }

    std::string absolutePath = std::filesystem::absolute(directoryPath).string();
    
    {
        std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
        m_watchedDirectories.insert(absolutePath);
    }

    // Find all model files in the directory
    auto modelFiles = FindModelFilesInDirectory(absolutePath, recursive);
    
    Logger::GetInstance().Info("Found " + std::to_string(modelFiles.size()) + 
                              " model files in directory: " + absolutePath);

    // Note: We don't automatically watch these files since we don't have Model instances for them
    // This method is mainly for future directory monitoring functionality
}

void ModelHotReloader::UnwatchDirectory(const std::string& directoryPath) {
    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    std::string absolutePath = std::filesystem::absolute(directoryPath).string();
    m_watchedDirectories.erase(absolutePath);

    if (m_config.logReloadEvents) {
        Logger::GetInstance().Info("Stopped watching directory: " + absolutePath);
    }
}

void ModelHotReloader::StartWatching() {
    if (!m_initialized) {
        Logger::GetInstance().Error("ModelHotReloader not initialized");
        return;
    }

    if (m_isWatching) {
        Logger::GetInstance().Warning("ModelHotReloader already watching");
        return;
    }

    if (!m_config.enabled) {
        Logger::GetInstance().Info("ModelHotReloader disabled in configuration");
        return;
    }

    m_shouldStop = false;
    m_isWatching = true;
    
    m_watchThread = std::make_unique<std::thread>(&ModelHotReloader::WatchThreadFunction, this);

    Logger::GetInstance().Info("ModelHotReloader started watching (poll interval: " + 
                              std::to_string(m_config.pollInterval.count()) + "ms)");
}

void ModelHotReloader::StopWatching() {
    if (!m_isWatching) {
        return;
    }

    m_shouldStop = true;
    m_isWatching = false;

    if (m_watchThread && m_watchThread->joinable()) {
        m_watchThread->join();
        m_watchThread.reset();
    }

    Logger::GetInstance().Info("ModelHotReloader stopped watching");
}

void ModelHotReloader::TriggerReload(const std::string& modelPath) {
    if (!m_initialized) {
        Logger::GetInstance().Error("ModelHotReloader not initialized");
        return;
    }

    std::string absolutePath = std::filesystem::absolute(modelPath).string();
    ReloadModel(absolutePath);
}

void ModelHotReloader::ReloadAll() {
    if (!m_initialized) {
        Logger::GetInstance().Error("ModelHotReloader not initialized");
        return;
    }

    std::vector<std::string> modelsToReload;
    
    {
        std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
        modelsToReload.reserve(m_watchedModels.size());
        
        for (const auto& pair : m_watchedModels) {
            modelsToReload.push_back(pair.first);
        }
    }

    Logger::GetInstance().Info("Reloading all " + std::to_string(modelsToReload.size()) + " watched models");

    for (const auto& modelPath : modelsToReload) {
        ReloadModel(modelPath);
    }
}

void ModelHotReloader::SetConfig(const HotReloadConfig& config) {
    m_config = config;
    
    if (m_config.logReloadEvents) {
        Logger::GetInstance().Info("ModelHotReloader configuration updated");
    }
}

void ModelHotReloader::SetReloadCallback(ReloadCallback callback) {
    m_reloadCallback = callback;
}

ModelHotReloader::HotReloadStats ModelHotReloader::GetStats() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_stats;
}

std::vector<ModelHotReloader::WatchedModel> ModelHotReloader::GetWatchedModels() const {
    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    std::vector<WatchedModel> models;
    models.reserve(m_watchedModels.size());
    
    for (const auto& pair : m_watchedModels) {
        models.push_back(pair.second);
    }
    
    return models;
}

void ModelHotReloader::PrintWatchedModels() const {
    auto models = GetWatchedModels();
    
    Logger::GetInstance().Info("=== Watched Models (" + std::to_string(models.size()) + ") ===");
    
    for (const auto& model : models) {
        std::string status = model.isValid ? "Valid" : "Invalid";
        std::string hasModel = model.modelRef.expired() ? "No" : "Yes";
        
        Logger::GetInstance().Info("  " + model.filePath);
        Logger::GetInstance().Info("    Status: " + status + ", Model: " + hasModel + 
                                  ", Reloads: " + std::to_string(model.reloadCount));
    }
}

void ModelHotReloader::ValidateWatchedModels() {
    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    for (auto& pair : m_watchedModels) {
        auto& watchedModel = pair.second;
        
        // Check if file still exists
        if (!std::filesystem::exists(watchedModel.filePath)) {
            watchedModel.isValid = false;
            continue;
        }
        
        // Check if model reference is still valid
        if (watchedModel.modelRef.expired()) {
            watchedModel.isValid = false;
            continue;
        }
        
        // Validate the model itself
        auto model = watchedModel.modelRef.lock();
        if (model && !ValidateModel(model)) {
            watchedModel.isValid = false;
            continue;
        }
        
        watchedModel.isValid = true;
    }
    
    CleanupInvalidWatches();
}

void ModelHotReloader::OptimizeWatchedModels() {
    std::vector<std::shared_ptr<Model>> modelsToOptimize;
    
    {
        std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
        
        for (const auto& pair : m_watchedModels) {
            const auto& watchedModel = pair.second;
            if (watchedModel.isValid && !watchedModel.modelRef.expired()) {
                auto model = watchedModel.modelRef.lock();
                if (model) {
                    modelsToOptimize.push_back(model);
                }
            }
        }
    }
    
    Logger::GetInstance().Info("Optimizing " + std::to_string(modelsToOptimize.size()) + " watched models");
    
    for (auto& model : modelsToOptimize) {
        OptimizeModel(model);
    }
}

void ModelHotReloader::ClearCacheForWatchedModels() {
    if (!m_modelLoader) {
        return;
    }
    
    std::vector<std::string> pathsToClear;
    
    {
        std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
        pathsToClear.reserve(m_watchedModels.size());
        
        for (const auto& pair : m_watchedModels) {
            pathsToClear.push_back(pair.first);
        }
    }
    
    Logger::GetInstance().Info("Clearing cache for " + std::to_string(pathsToClear.size()) + " watched models");
    
    for (const auto& path : pathsToClear) {
        m_modelLoader->InvalidateCache(path);
    }
}

// Private methods

void ModelHotReloader::WatchThreadFunction() {
    Logger::GetInstance().Debug("ModelHotReloader watch thread started");
    
    while (!m_shouldStop) {
        try {
            CheckForChanges();
            std::this_thread::sleep_for(m_config.pollInterval);
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("ModelHotReloader watch thread exception: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Longer sleep on error
        }
    }
    
    Logger::GetInstance().Debug("ModelHotReloader watch thread stopped");
}

void ModelHotReloader::CheckForChanges() {
    std::vector<std::string> modelsToReload;
    
    {
        std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
        
        for (auto& pair : m_watchedModels) {
            auto& watchedModel = pair.second;
            
            // Skip invalid models
            if (!watchedModel.isValid) {
                continue;
            }
            
            // Check if model reference is still valid
            if (watchedModel.modelRef.expired()) {
                watchedModel.isValid = false;
                continue;
            }
            
            // Check if file has changed
            if (HasFileChanged(watchedModel)) {
                modelsToReload.push_back(watchedModel.filePath);
                
                // Update the watched model's metadata
                watchedModel.lastModified = GetFileModificationTime(watchedModel.filePath);
                watchedModel.fileSize = GetFileSize(watchedModel.filePath);
            }
            
            watchedModel.lastChecked = std::chrono::system_clock::now();
        }
    }
    
    // Reload changed models (outside of lock to avoid deadlock)
    for (const auto& modelPath : modelsToReload) {
        ReloadModel(modelPath);
    }
    
    // Cleanup invalid watches periodically
    static auto lastCleanup = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (now - lastCleanup > std::chrono::minutes(1)) {
        CleanupInvalidWatches();
        lastCleanup = now;
    }
}

bool ModelHotReloader::HasFileChanged(const WatchedModel& watchedModel) const {
    try {
        // Check if file still exists
        if (!std::filesystem::exists(watchedModel.filePath)) {
            return false; // File was deleted, don't trigger reload
        }
        
        // Check modification time
        auto currentModTime = GetFileModificationTime(watchedModel.filePath);
        if (currentModTime != watchedModel.lastModified) {
            return true;
        }
        
        // Check file size as additional verification
        size_t currentSize = GetFileSize(watchedModel.filePath);
        if (currentSize != watchedModel.fileSize) {
            return true;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Warning("ModelHotReloader::HasFileChanged exception for " + 
                                    watchedModel.filePath + ": " + e.what());
        return false;
    }
}

void ModelHotReloader::ReloadModel(const std::string& modelPath) {
    auto startTime = std::chrono::high_resolution_clock::now();
    bool success = false;
    std::shared_ptr<Model> newModel = nullptr;
    
    try {
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalReloadAttempts++;
        }
        
        if (m_config.logReloadEvents) {
            Logger::GetInstance().Info("Reloading model: " + modelPath);
        }
        
        // Clear cache if configured
        if (m_config.clearCacheOnReload && m_modelLoader) {
            m_modelLoader->InvalidateCache(modelPath);
        }
        
        // Load the new model
        if (m_modelLoader) {
            newModel = m_modelLoader->LoadModelAsResource(modelPath);
            success = (newModel != nullptr);
        }
        
        if (success) {
            // Validate if configured
            if (m_config.validateOnReload && !ValidateModel(newModel)) {
                Logger::GetInstance().Warning("Reloaded model failed validation: " + modelPath);
                success = false;
                newModel = nullptr;
            }
            
            // Optimize if configured
            if (success && m_config.optimizeOnReload) {
                OptimizeModel(newModel);
            }
        }
        
        // Update watched model metadata
        {
            std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
            auto it = m_watchedModels.find(modelPath);
            if (it != m_watchedModels.end()) {
                auto& watchedModel = it->second;
                watchedModel.reloadCount++;
                
                if (success && newModel) {
                    // Update the weak reference to point to the new model
                    watchedModel.modelRef = newModel;
                }
            }
        }
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Error("ModelHotReloader::ReloadModel exception for " + 
                                  modelPath + ": " + e.what());
        success = false;
        newModel = nullptr;
    }
    
    // Calculate reload time
    auto endTime = std::chrono::high_resolution_clock::now();
    float reloadTimeMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    
    // Update statistics
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        if (success) {
            m_stats.successfulReloads++;
        } else {
            m_stats.failedReloads++;
        }
        
        // Update average reload time
        float totalTime = m_stats.averageReloadTimeMs * (m_stats.successfulReloads + m_stats.failedReloads - 1);
        m_stats.averageReloadTimeMs = (totalTime + reloadTimeMs) / (m_stats.successfulReloads + m_stats.failedReloads);
        m_stats.lastReloadTime = std::chrono::system_clock::now();
    }
    
    // Log the reload event
    LogReloadEvent(modelPath, success, reloadTimeMs);
    
    // Call the reload callback if set
    if (m_reloadCallback) {
        try {
            m_reloadCallback(modelPath, newModel, success);
        } catch (const std::exception& e) {
            Logger::GetInstance().Error("ModelHotReloader reload callback exception: " + std::string(e.what()));
        }
    }
}

void ModelHotReloader::UpdateWatchedModel(const std::string& modelPath, 
                                         const std::filesystem::file_time_type& modTime, 
                                         size_t fileSize) {
    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    auto it = m_watchedModels.find(modelPath);
    if (it != m_watchedModels.end()) {
        auto& watchedModel = it->second;
        watchedModel.lastModified = std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(modTime.time_since_epoch()).count());
        watchedModel.fileSize = fileSize;
        watchedModel.lastChecked = std::chrono::system_clock::now();
    }
}

void ModelHotReloader::CleanupInvalidWatches() {
    std::lock_guard<std::mutex> lock(m_watchedModelsMutex);
    
    auto it = m_watchedModels.begin();
    while (it != m_watchedModels.end()) {
        const auto& watchedModel = it->second;
        
        // Remove if model reference expired or file no longer exists
        if (watchedModel.modelRef.expired() || !std::filesystem::exists(watchedModel.filePath)) {
            if (m_config.logReloadEvents) {
                Logger::GetInstance().Info("Removing invalid watch: " + watchedModel.filePath);
            }
            it = m_watchedModels.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update statistics
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.totalWatchedFiles = static_cast<uint32_t>(m_watchedModels.size());
    }
}

// File system utilities

std::chrono::system_clock::time_point ModelHotReloader::GetFileModificationTime(const std::string& path) const {
    try {
        auto ftime = std::filesystem::last_write_time(path);
        return std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count());
    } catch (const std::exception&) {
        return std::chrono::system_clock::now();
    }
}

size_t ModelHotReloader::GetFileSize(const std::string& path) const {
    try {
        return std::filesystem::file_size(path);
    } catch (const std::exception&) {
        return 0;
    }
}

bool ModelHotReloader::IsModelFile(const std::string& path) const {
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();
    
    // Remove leading dot and convert to lowercase
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return std::find(m_config.watchedExtensions.begin(), m_config.watchedExtensions.end(), extension) 
           != m_config.watchedExtensions.end();
}

bool ModelHotReloader::ShouldIgnoreDirectory(const std::string& path) const {
    std::filesystem::path dirPath(path);
    std::string dirName = dirPath.filename().string();
    
    return std::find(m_config.ignoredDirectories.begin(), m_config.ignoredDirectories.end(), dirName) 
           != m_config.ignoredDirectories.end();
}

std::vector<std::string> ModelHotReloader::FindModelFilesInDirectory(const std::string& directoryPath, bool recursive) const {
    std::vector<std::string> modelFiles;
    
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    std::string filePath = entry.path().string();
                    if (IsModelFile(filePath)) {
                        // Check if we should ignore the parent directory
                        bool shouldIgnore = false;
                        for (auto parent = entry.path().parent_path(); 
                             parent != directoryPath && !parent.empty(); 
                             parent = parent.parent_path()) {
                            if (ShouldIgnoreDirectory(parent.string())) {
                                shouldIgnore = true;
                                break;
                            }
                        }
                        
                        if (!shouldIgnore) {
                            modelFiles.push_back(filePath);
                        }
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_regular_file() && IsModelFile(entry.path().string())) {
                    modelFiles.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::GetInstance().Warning("ModelHotReloader::FindModelFilesInDirectory exception: " + std::string(e.what()));
    }
    
    return modelFiles;
}

// Validation and optimization

bool ModelHotReloader::ValidateModel(std::shared_ptr<Model> model) const {
    if (!model) {
        return false;
    }
    
    try {
        // Basic validation checks
        auto meshes = model->GetMeshes();
        if (meshes.empty()) {
            Logger::GetInstance().Warning("Model validation failed: No meshes");
            return false;
        }
        
        // Check each mesh
        for (const auto& mesh : meshes) {
            if (!mesh) {
                Logger::GetInstance().Warning("Model validation failed: Null mesh");
                return false;
            }
            
            if (mesh->GetVertexCount() == 0) {
                Logger::GetInstance().Warning("Model validation failed: Mesh has no vertices");
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Warning("Model validation exception: " + std::string(e.what()));
        return false;
    }
}

void ModelHotReloader::OptimizeModel(std::shared_ptr<Model> model) const {
    if (!model) {
        return;
    }
    
    try {
        // Update bounding volumes
        model->UpdateBounds();
        
        // Additional optimization could be added here
        // For example: mesh optimization, LOD generation, etc.
        
    } catch (const std::exception& e) {
        Logger::GetInstance().Warning("Model optimization exception: " + std::string(e.what()));
    }
}

void ModelHotReloader::LogReloadEvent(const std::string& modelPath, bool success, float reloadTimeMs) const {
    if (!m_config.logReloadEvents) {
        return;
    }
    
    std::string message = success ? "Successfully reloaded" : "Failed to reload";
    message += " model: " + modelPath + " (" + std::to_string(static_cast<int>(reloadTimeMs)) + "ms)";
    
    if (success) {
        Logger::GetInstance().Info(message);
    } else {
        Logger::GetInstance().Error(message);
    }
}

} // namespace GameEngine