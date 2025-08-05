#include "Graphics/ShaderManager.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderHotReloader.h"
#include "Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace GameEngine {
    ShaderManager& ShaderManager::GetInstance() {
        static ShaderManager instance;
        return instance;
    }

    bool ShaderManager::Initialize() {
        if (m_initialized) {
            LOG_WARNING("ShaderManager already initialized");
            return true;
        }

        LOG_INFO("Initializing ShaderManager");
        
        m_shaders.clear();
        m_shaderDescs.clear();
        m_fileToShaderMap.clear();
        
        m_stats = ShaderStats{};
        m_hotReloadEnabled = false;
        m_debugMode = false;
        
        // Initialize hot reloader
        m_hotReloader = std::make_unique<ShaderHotReloader>();
        if (!m_hotReloader->Initialize()) {
            LOG_ERROR("Failed to initialize ShaderHotReloader");
            return false;
        }
        
        // Set up hot reload callbacks
        m_hotReloader->SetReloadCallback([this](const std::string& filepath) {
            OnShaderFileChanged(filepath);
        });
        
        m_hotReloader->SetErrorCallback([this](const std::string& filepath, const std::string& error) {
            OnShaderFileError(filepath, error);
        });
        
        m_initialized = true;
        LOG_INFO("ShaderManager initialized successfully");
        return true;
    }

    void ShaderManager::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down ShaderManager");
        
        UnloadAllShaders();
        m_shaderDescs.clear();
        m_fileToShaderMap.clear();
        m_hotReloadCallback = nullptr;
        m_hotReloadErrorCallback = nullptr;
        
        // Shutdown hot reloader
        if (m_hotReloader) {
            m_hotReloader->Shutdown();
            m_hotReloader.reset();
        }
        
        m_initialized = false;
        LOG_INFO("ShaderManager shutdown complete");
    }

    void ShaderManager::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        // Update hot reloader
        if (m_hotReloader && m_hotReloadEnabled) {
            m_hotReloader->Update();
        }
    }

    std::shared_ptr<Shader> ShaderManager::LoadShader(const std::string& name, const ShaderDesc& desc) {
        if (!m_initialized) {
            LOG_ERROR("ShaderManager not initialized");
            return nullptr;
        }

        if (!ValidateShaderDesc(desc)) {
            LOG_ERROR("Invalid shader description for: " + name);
            return nullptr;
        }

        // Check if shader already exists
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            if (m_debugMode) {
                LOG_INFO("Shader already loaded: " + name);
            }
            return it->second;
        }

        // Create shader from description
        auto shader = CreateShaderFromDesc(desc);
        if (!shader) {
            LOG_ERROR("Failed to create shader: " + name);
            m_stats.compilationErrors++;
            return nullptr;
        }

        // Register shader
        m_shaders[name] = shader;
        m_shaderDescs[name] = desc;

        // Register shader files for hot reload
        if (desc.enableHotReload && m_hotReloader) {
            RegisterShaderFiles(name, desc);
        }

        UpdateShaderStats();
        
        if (m_debugMode) {
            LOG_INFO("Shader loaded successfully: " + name);
        }
        
        return shader;
    }

    std::shared_ptr<Shader> ShaderManager::LoadShaderFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        ShaderDesc desc;
        desc.name = name;
        desc.vertexPath = vertexPath;
        desc.fragmentPath = fragmentPath;
        desc.enableHotReload = true;
        desc.enableOptimization = true;
        
        return LoadShader(name, desc);
    }

    std::shared_ptr<Shader> ShaderManager::LoadShaderFromSource(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) {
        if (!m_initialized) {
            LOG_ERROR("ShaderManager not initialized");
            return nullptr;
        }

        // Check if shader already exists
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            if (m_debugMode) {
                LOG_INFO("Shader already loaded: " + name);
            }
            return it->second;
        }

        // Create shader from source
        auto shader = std::make_shared<Shader>();
        if (!shader->LoadFromSource(vertexSource, fragmentSource)) {
            LOG_ERROR("Failed to compile shader from source: " + name);
            m_stats.compilationErrors++;
            return nullptr;
        }

        // Register shader
        m_shaders[name] = shader;
        
        // Create minimal desc for source-based shaders
        ShaderDesc desc;
        desc.name = name;
        desc.enableHotReload = false; // Source-based shaders don't support hot reload
        desc.enableOptimization = true;
        m_shaderDescs[name] = desc;

        UpdateShaderStats();
        
        if (m_debugMode) {
            LOG_INFO("Shader loaded from source: " + name);
        }
        
        return shader;
    }

    std::shared_ptr<Shader> ShaderManager::GetShader(const std::string& name) {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            return it->second;
        }
        
        if (m_debugMode) {
            LOG_WARNING("Shader not found: " + name);
        }
        return nullptr;
    }

    void ShaderManager::UnloadShader(const std::string& name) {
        auto it = m_shaders.find(name);
        if (it != m_shaders.end()) {
            if (m_debugMode) {
                LOG_INFO("Unloading shader: " + name);
            }
            
            // Unregister shader files from hot reload
            UnregisterShaderFiles(name);
            
            m_shaders.erase(it);
            m_shaderDescs.erase(name);
            
            UpdateShaderStats();
        }
    }

    void ShaderManager::UnloadAllShaders() {
        if (m_debugMode) {
            LOG_INFO("Unloading all shaders (" + std::to_string(m_shaders.size()) + " shaders)");
        }
        
        // Unregister all shader files from hot reload
        for (const auto& pair : m_shaderDescs) {
            UnregisterShaderFiles(pair.first);
        }
        
        m_shaders.clear();
        m_shaderDescs.clear();
        m_fileToShaderMap.clear();
        UpdateShaderStats();
    }

    bool ShaderManager::RegisterShader(const std::string& name, std::shared_ptr<Shader> shader) {
        if (!shader) {
            LOG_ERROR("Cannot register null shader: " + name);
            return false;
        }

        if (HasShader(name)) {
            LOG_WARNING("Shader already registered, replacing: " + name);
        }

        m_shaders[name] = shader;
        
        // Create minimal desc for registered shaders
        ShaderDesc desc;
        desc.name = name;
        desc.enableHotReload = false;
        desc.enableOptimization = false;
        m_shaderDescs[name] = desc;
        
        UpdateShaderStats();
        
        if (m_debugMode) {
            LOG_INFO("Shader registered: " + name);
        }
        
        return true;
    }

    bool ShaderManager::HasShader(const std::string& name) const {
        return m_shaders.find(name) != m_shaders.end();
    }

    std::vector<std::string> ShaderManager::GetShaderNames() const {
        std::vector<std::string> names;
        names.reserve(m_shaders.size());
        
        for (const auto& pair : m_shaders) {
            names.push_back(pair.first);
        }
        
        return names;
    }

    void ShaderManager::EnableHotReload(bool enable) {
        m_hotReloadEnabled = enable;
        
        if (m_hotReloader) {
            m_hotReloader->SetEnabled(enable);
        }
        
        if (m_debugMode) {
            LOG_INFO("Hot reload " + std::string(enable ? "enabled" : "disabled"));
        }
    }

    void ShaderManager::SetHotReloadCallback(std::function<void(const std::string&)> callback) {
        m_hotReloadCallback = callback;
    }

    void ShaderManager::SetHotReloadErrorCallback(std::function<void(const std::string&, const std::string&)> callback) {
        m_hotReloadErrorCallback = callback;
    }

    void ShaderManager::SetHotReloadCheckInterval(float intervalSeconds) {
        if (m_hotReloader) {
            m_hotReloader->SetCheckInterval(intervalSeconds);
        }
    }

    void ShaderManager::CheckForShaderChanges() {
        // This method is now handled by the ShaderHotReloader
        // The hot reloader will call OnShaderFileChanged when files change
        if (m_debugMode) {
            LOG_INFO("Manual check for shader changes requested");
        }
    }

    void ShaderManager::ReloadShader(const std::string& name) {
        auto descIt = m_shaderDescs.find(name);
        if (descIt == m_shaderDescs.end()) {
            LOG_WARNING("Cannot reload shader, description not found: " + name);
            return;
        }

        const auto& desc = descIt->second;
        
        if (m_debugMode) {
            LOG_INFO("Reloading shader: " + name);
        }

        // Create new shader with graceful fallback
        auto newShader = CreateShaderFromDesc(desc);
        if (newShader) {
            // Replace old shader
            m_shaders[name] = newShader;
            
            // Notify callback
            if (m_hotReloadCallback) {
                m_hotReloadCallback(name);
            }
            
            if (m_debugMode) {
                LOG_INFO("Shader reloaded successfully: " + name);
            }
        } else {
            LOG_ERROR("Failed to reload shader: " + name + " - keeping previous version");
            m_stats.compilationErrors++;
            
            // Graceful fallback: keep the old shader and notify error callback
            if (m_hotReloadErrorCallback) {
                m_hotReloadErrorCallback(name, "Shader compilation failed, keeping previous version");
            }
        }
    }

    void ShaderManager::ReloadAllShaders() {
        if (m_debugMode) {
            LOG_INFO("Reloading all shaders");
        }

        std::vector<std::string> shaderNames = GetShaderNames();
        for (const std::string& name : shaderNames) {
            ReloadShader(name);
        }
    }

    ShaderStats ShaderManager::GetShaderStats() const {
        return m_stats;
    }

    void ShaderManager::PrecompileShaders() {
        if (m_debugMode) {
            LOG_INFO("Precompiling shaders (placeholder implementation)");
        }
        // TODO: Implement shader precompilation in future tasks
    }

    void ShaderManager::ClearShaderCache() {
        if (m_debugMode) {
            LOG_INFO("Clearing shader cache (placeholder implementation)");
        }
        // TODO: Implement shader cache clearing in future tasks
    }

    std::shared_ptr<Shader> ShaderManager::CreateShaderFromDesc(const ShaderDesc& desc) {
        auto shader = std::make_shared<Shader>();

        // For now, only support vertex + fragment shaders (basic implementation)
        if (!desc.vertexPath.empty() && !desc.fragmentPath.empty()) {
            if (!shader->LoadFromFiles(desc.vertexPath, desc.fragmentPath)) {
                return nullptr;
            }
        } else {
            LOG_ERROR("Shader description must include vertex and fragment paths: " + desc.name);
            return nullptr;
        }

        return shader;
    }

    bool ShaderManager::ValidateShaderDesc(const ShaderDesc& desc) {
        if (desc.name.empty()) {
            LOG_ERROR("Shader description must have a name");
            return false;
        }

        // Must have at least vertex and fragment shaders
        if (desc.vertexPath.empty() || desc.fragmentPath.empty()) {
            LOG_ERROR("Shader description must include vertex and fragment paths: " + desc.name);
            return false;
        }

        // Check if files exist
        if (!std::filesystem::exists(desc.vertexPath)) {
            LOG_ERROR("Vertex shader file not found: " + desc.vertexPath);
            return false;
        }

        if (!std::filesystem::exists(desc.fragmentPath)) {
            LOG_ERROR("Fragment shader file not found: " + desc.fragmentPath);
            return false;
        }

        return true;
    }

    void ShaderManager::UpdateShaderStats() {
        m_stats.totalShaders = m_shaders.size();
        m_stats.loadedShaders = 0;
        m_stats.memoryUsage = 0;

        for (const auto& pair : m_shaders) {
            if (pair.second && pair.second->IsValid()) {
                m_stats.loadedShaders++;
                // Rough estimate of memory usage per shader
                m_stats.memoryUsage += 1024; // 1KB per shader (placeholder)
            }
        }
    }

    void ShaderManager::ReloadShadersFromFiles(const std::vector<std::string>& filepaths) {
        if (m_debugMode) {
            LOG_INFO("Batch reloading shaders from " + std::to_string(filepaths.size()) + " files");
        }

        std::vector<std::string> shadersToReload;
        
        // Find all shaders that use these files
        for (const std::string& filepath : filepaths) {
            auto shaderNames = GetShadersUsingFile(filepath);
            for (const std::string& shaderName : shaderNames) {
                // Avoid duplicates
                if (std::find(shadersToReload.begin(), shadersToReload.end(), shaderName) == shadersToReload.end()) {
                    shadersToReload.push_back(shaderName);
                }
            }
        }

        // Reload all affected shaders
        for (const std::string& shaderName : shadersToReload) {
            ReloadShader(shaderName);
        }

        if (m_debugMode) {
            LOG_INFO("Batch reload completed for " + std::to_string(shadersToReload.size()) + " shaders");
        }
    }

    void ShaderManager::WatchShaderDirectory(const std::string& directory) {
        if (!m_hotReloader) {
            LOG_ERROR("Hot reloader not initialized");
            return;
        }

        m_hotReloader->WatchShaderDirectory(directory);
        
        if (m_debugMode) {
            LOG_INFO("Watching shader directory: " + directory);
        }
    }

    void ShaderManager::WatchShaderFile(const std::string& filepath) {
        if (!m_hotReloader) {
            LOG_ERROR("Hot reloader not initialized");
            return;
        }

        m_hotReloader->WatchShaderFile(filepath);
        
        if (m_debugMode) {
            LOG_INFO("Watching shader file: " + filepath);
        }
    }

    void ShaderManager::UnwatchShaderFile(const std::string& filepath) {
        if (!m_hotReloader) {
            LOG_ERROR("Hot reloader not initialized");
            return;
        }

        m_hotReloader->UnwatchShaderFile(filepath);
        
        if (m_debugMode) {
            LOG_INFO("Stopped watching shader file: " + filepath);
        }
    }

    void ShaderManager::OnShaderFileChanged(const std::string& filepath) {
        if (m_debugMode) {
            LOG_INFO("Shader file changed: " + filepath);
        }

        // Find all shaders that use this file and reload them
        auto shaderNames = GetShadersUsingFile(filepath);
        
        if (shaderNames.empty()) {
            if (m_debugMode) {
                LOG_WARNING("No shaders found using file: " + filepath);
            }
            return;
        }

        // Batch reload all affected shaders
        for (const std::string& shaderName : shaderNames) {
            ReloadShader(shaderName);
        }
    }

    void ShaderManager::OnShaderFileError(const std::string& filepath, const std::string& error) {
        LOG_ERROR("Shader file error for " + filepath + ": " + error);
        
        if (m_hotReloadErrorCallback) {
            m_hotReloadErrorCallback(filepath, error);
        }
    }

    void ShaderManager::RegisterShaderFiles(const std::string& shaderName, const ShaderDesc& desc) {
        // Register all shader files with the hot reloader and map them to the shader name
        if (!desc.vertexPath.empty()) {
            std::string normalizedPath = std::filesystem::absolute(desc.vertexPath).string();
            m_fileToShaderMap[normalizedPath] = shaderName;
            if (m_hotReloader) {
                m_hotReloader->WatchShaderFile(normalizedPath);
            }
        }
        
        if (!desc.fragmentPath.empty()) {
            std::string normalizedPath = std::filesystem::absolute(desc.fragmentPath).string();
            m_fileToShaderMap[normalizedPath] = shaderName;
            if (m_hotReloader) {
                m_hotReloader->WatchShaderFile(normalizedPath);
            }
        }
        
        if (!desc.geometryPath.empty()) {
            std::string normalizedPath = std::filesystem::absolute(desc.geometryPath).string();
            m_fileToShaderMap[normalizedPath] = shaderName;
            if (m_hotReloader) {
                m_hotReloader->WatchShaderFile(normalizedPath);
            }
        }
        
        if (!desc.computePath.empty()) {
            std::string normalizedPath = std::filesystem::absolute(desc.computePath).string();
            m_fileToShaderMap[normalizedPath] = shaderName;
            if (m_hotReloader) {
                m_hotReloader->WatchShaderFile(normalizedPath);
            }
        }
        
        if (!desc.tessControlPath.empty()) {
            std::string normalizedPath = std::filesystem::absolute(desc.tessControlPath).string();
            m_fileToShaderMap[normalizedPath] = shaderName;
            if (m_hotReloader) {
                m_hotReloader->WatchShaderFile(normalizedPath);
            }
        }
        
        if (!desc.tessEvaluationPath.empty()) {
            std::string normalizedPath = std::filesystem::absolute(desc.tessEvaluationPath).string();
            m_fileToShaderMap[normalizedPath] = shaderName;
            if (m_hotReloader) {
                m_hotReloader->WatchShaderFile(normalizedPath);
            }
        }
    }

    void ShaderManager::UnregisterShaderFiles(const std::string& shaderName) {
        // Find and remove all file mappings for this shader
        std::vector<std::string> filesToRemove;
        
        for (const auto& pair : m_fileToShaderMap) {
            if (pair.second == shaderName) {
                filesToRemove.push_back(pair.first);
            }
        }
        
        for (const std::string& filepath : filesToRemove) {
            m_fileToShaderMap.erase(filepath);
            if (m_hotReloader) {
                m_hotReloader->UnwatchShaderFile(filepath);
            }
        }
    }

    std::vector<std::string> ShaderManager::GetShadersUsingFile(const std::string& filepath) const {
        std::vector<std::string> shaderNames;
        
        std::string normalizedPath = std::filesystem::absolute(filepath).string();
        
        // Check direct mapping
        auto it = m_fileToShaderMap.find(normalizedPath);
        if (it != m_fileToShaderMap.end()) {
            shaderNames.push_back(it->second);
        }
        
        // Also check all shader descriptions in case of path variations
        for (const auto& pair : m_shaderDescs) {
            const std::string& shaderName = pair.first;
            const ShaderDesc& desc = pair.second;
            
            // Check if any of the shader's files match the given filepath
            if ((!desc.vertexPath.empty() && std::filesystem::equivalent(desc.vertexPath, filepath)) ||
                (!desc.fragmentPath.empty() && std::filesystem::equivalent(desc.fragmentPath, filepath)) ||
                (!desc.geometryPath.empty() && std::filesystem::equivalent(desc.geometryPath, filepath)) ||
                (!desc.computePath.empty() && std::filesystem::equivalent(desc.computePath, filepath)) ||
                (!desc.tessControlPath.empty() && std::filesystem::equivalent(desc.tessControlPath, filepath)) ||
                (!desc.tessEvaluationPath.empty() && std::filesystem::equivalent(desc.tessEvaluationPath, filepath))) {
                
                // Avoid duplicates
                if (std::find(shaderNames.begin(), shaderNames.end(), shaderName) == shaderNames.end()) {
                    shaderNames.push_back(shaderName);
                }
            }
        }
        
        return shaderNames;
    }
}