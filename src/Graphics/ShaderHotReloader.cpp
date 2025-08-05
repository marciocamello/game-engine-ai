#include "Graphics/ShaderHotReloader.h"
#include "Core/Logger.h"
#include <filesystem>
#include <algorithm>

namespace GameEngine {

    bool ShaderHotReloader::Initialize() {
        if (m_initialized) {
            LOG_WARNING("ShaderHotReloader already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderHotReloader");
        
        m_watchedFiles.clear();
        m_enabled = false;
        m_timeSinceLastCheck = 0.0f;
        
        m_initialized = true;
        LOG_INFO("ShaderHotReloader initialized successfully");
        return true;
    }

    void ShaderHotReloader::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down ShaderHotReloader");
        
        m_watchedFiles.clear();
        m_reloadCallback = nullptr;
        m_errorCallback = nullptr;
        m_enabled = false;
        
        m_initialized = false;
        LOG_INFO("ShaderHotReloader shutdown complete");
    }

    void ShaderHotReloader::Update() {
        if (!m_initialized || !m_enabled) {
            return;
        }

        m_timeSinceLastCheck += 0.016f; // Assume ~60 FPS for now, will be replaced with actual deltaTime
        
        if (m_timeSinceLastCheck >= m_checkInterval) {
            CheckFileChanges();
            m_timeSinceLastCheck = 0.0f;
        }
    }

    void ShaderHotReloader::WatchShaderDirectory(const std::string& directory) {
        if (!m_initialized) {
            LOG_ERROR("ShaderHotReloader not initialized");
            return;
        }

        if (!std::filesystem::exists(directory)) {
            std::string error = "Directory does not exist: " + directory;
            LOG_ERROR(error);
            if (m_errorCallback) {
                m_errorCallback(directory, error);
            }
            return;
        }

        if (!std::filesystem::is_directory(directory)) {
            std::string error = "Path is not a directory: " + directory;
            LOG_ERROR(error);
            if (m_errorCallback) {
                m_errorCallback(directory, error);
            }
            return;
        }

        LOG_INFO("Watching shader directory: " + directory);
        ProcessDirectoryRecursively(directory);
    }

    void ShaderHotReloader::WatchShaderFile(const std::string& filepath) {
        if (!m_initialized) {
            LOG_ERROR("ShaderHotReloader not initialized");
            return;
        }

        if (!std::filesystem::exists(filepath)) {
            std::string error = "Shader file does not exist: " + filepath;
            LOG_ERROR(error);
            if (m_errorCallback) {
                m_errorCallback(filepath, error);
            }
            return;
        }

        if (!IsShaderFile(filepath)) {
            std::string error = "File is not a recognized shader file: " + filepath;
            LOG_WARNING(error);
            if (m_errorCallback) {
                m_errorCallback(filepath, error);
            }
            return;
        }

        // Normalize path
        std::string normalizedPath = std::filesystem::absolute(filepath).string();
        
        // Check if already watching
        if (m_watchedFiles.find(normalizedPath) != m_watchedFiles.end()) {
            LOG_INFO("File already being watched: " + normalizedPath);
            return;
        }

        // Add to watched files
        WatchedFile watchedFile;
        watchedFile.filepath = normalizedPath;
        watchedFile.lastWriteTime = std::filesystem::last_write_time(filepath);
        watchedFile.needsReload = false;

        m_watchedFiles[normalizedPath] = watchedFile;
        
        LOG_INFO("Now watching shader file: " + normalizedPath);
    }

    void ShaderHotReloader::UnwatchShaderFile(const std::string& filepath) {
        if (!m_initialized) {
            LOG_ERROR("ShaderHotReloader not initialized");
            return;
        }

        std::string normalizedPath = std::filesystem::absolute(filepath).string();
        
        auto it = m_watchedFiles.find(normalizedPath);
        if (it != m_watchedFiles.end()) {
            m_watchedFiles.erase(it);
            LOG_INFO("Stopped watching shader file: " + normalizedPath);
        } else {
            LOG_WARNING("File was not being watched: " + normalizedPath);
        }
    }

    void ShaderHotReloader::SetReloadCallback(std::function<void(const std::string&)> callback) {
        m_reloadCallback = callback;
    }

    void ShaderHotReloader::SetErrorCallback(std::function<void(const std::string&, const std::string&)> callback) {
        m_errorCallback = callback;
    }

    void ShaderHotReloader::ReloadShader(const std::string& filepath) {
        if (!m_initialized) {
            LOG_ERROR("ShaderHotReloader not initialized");
            return;
        }

        std::string normalizedPath = std::filesystem::absolute(filepath).string();
        
        LOG_INFO("Manual reload requested for: " + normalizedPath);
        
        if (m_reloadCallback) {
            m_reloadCallback(normalizedPath);
        }
        
        // Update timestamp to prevent immediate re-reload
        UpdateFileTimestamp(normalizedPath);
    }

    void ShaderHotReloader::ReloadAllShaders() {
        if (!m_initialized) {
            LOG_ERROR("ShaderHotReloader not initialized");
            return;
        }

        LOG_INFO("Manual reload requested for all watched shaders (" + 
                std::to_string(m_watchedFiles.size()) + " files)");
        
        for (const auto& pair : m_watchedFiles) {
            const std::string& filepath = pair.first;
            
            if (m_reloadCallback) {
                m_reloadCallback(filepath);
            }
            
            // Update timestamp to prevent immediate re-reload
            UpdateFileTimestamp(filepath);
        }
    }

    void ShaderHotReloader::SetEnabled(bool enabled) {
        m_enabled = enabled;
        
        if (m_enabled) {
            LOG_INFO("ShaderHotReloader enabled");
        } else {
            LOG_INFO("ShaderHotReloader disabled");
        }
    }

    void ShaderHotReloader::SetCheckInterval(float intervalSeconds) {
        if (intervalSeconds <= 0.0f) {
            LOG_WARNING("Invalid check interval, using default: 0.5s");
            m_checkInterval = 0.5f;
        } else {
            m_checkInterval = intervalSeconds;
            LOG_INFO("Check interval set to: " + std::to_string(intervalSeconds) + "s");
        }
    }

    std::vector<std::string> ShaderHotReloader::GetWatchedFiles() const {
        std::vector<std::string> files;
        files.reserve(m_watchedFiles.size());
        
        for (const auto& pair : m_watchedFiles) {
            files.push_back(pair.first);
        }
        
        return files;
    }

    bool ShaderHotReloader::IsFileWatched(const std::string& filepath) const {
        std::string normalizedPath = std::filesystem::absolute(filepath).string();
        return m_watchedFiles.find(normalizedPath) != m_watchedFiles.end();
    }

    void ShaderHotReloader::CheckFileChanges() {
        std::vector<std::string> filesToReload;

        for (auto& pair : m_watchedFiles) {
            const std::string& filepath = pair.first;
            WatchedFile& watchedFile = pair.second;

            if (HasFileChanged(watchedFile)) {
                filesToReload.push_back(filepath);
                watchedFile.needsReload = true;
            }
        }

        // Process reloads
        for (const std::string& filepath : filesToReload) {
            LOG_INFO("Detected change in shader file: " + filepath);
            
            if (m_reloadCallback) {
                try {
                    m_reloadCallback(filepath);
                    
                    // Update timestamp on successful reload
                    UpdateFileTimestamp(filepath);
                    
                    auto it = m_watchedFiles.find(filepath);
                    if (it != m_watchedFiles.end()) {
                        it->second.needsReload = false;
                    }
                    
                } catch (const std::exception& e) {
                    std::string error = "Exception during shader reload: " + std::string(e.what());
                    LOG_ERROR(error);
                    if (m_errorCallback) {
                        m_errorCallback(filepath, error);
                    }
                }
            }
        }
    }

    bool ShaderHotReloader::HasFileChanged(const WatchedFile& file) {
        if (!std::filesystem::exists(file.filepath)) {
            // File was deleted
            std::string error = "Watched shader file was deleted: " + file.filepath;
            LOG_WARNING(error);
            if (m_errorCallback) {
                m_errorCallback(file.filepath, error);
            }
            return false;
        }

        try {
            auto currentTime = std::filesystem::last_write_time(file.filepath);
            return currentTime > file.lastWriteTime;
        } catch (const std::filesystem::filesystem_error& e) {
            std::string error = "Failed to check file timestamp: " + std::string(e.what());
            LOG_ERROR(error);
            if (m_errorCallback) {
                m_errorCallback(file.filepath, error);
            }
            return false;
        }
    }

    void ShaderHotReloader::UpdateFileTimestamp(const std::string& filepath) {
        auto it = m_watchedFiles.find(filepath);
        if (it != m_watchedFiles.end()) {
            try {
                if (std::filesystem::exists(filepath)) {
                    it->second.lastWriteTime = std::filesystem::last_write_time(filepath);
                }
            } catch (const std::filesystem::filesystem_error& e) {
                LOG_ERROR("Failed to update file timestamp: " + std::string(e.what()));
            }
        }
    }

    void ShaderHotReloader::ProcessDirectoryRecursively(const std::string& directory) {
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file() && IsShaderFile(entry.path().string())) {
                    WatchShaderFile(entry.path().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::string error = "Failed to process directory: " + std::string(e.what());
            LOG_ERROR(error);
            if (m_errorCallback) {
                m_errorCallback(directory, error);
            }
        }
    }

    bool ShaderHotReloader::IsShaderFile(const std::string& filepath) const {
        std::string extension = std::filesystem::path(filepath).extension().string();
        
        // Convert to lowercase for case-insensitive comparison
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        // Common shader file extensions
        return extension == ".glsl" || 
               extension == ".vert" || 
               extension == ".frag" || 
               extension == ".geom" || 
               extension == ".comp" || 
               extension == ".tesc" || 
               extension == ".tese" ||
               extension == ".vs" ||
               extension == ".fs" ||
               extension == ".gs" ||
               extension == ".cs";
    }
}