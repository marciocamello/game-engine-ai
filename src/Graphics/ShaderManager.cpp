#include "Graphics/ShaderManager.h"
#include "Graphics/Shader.h"
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
        m_fileTimestamps.clear();
        
        m_stats = ShaderStats{};
        m_hotReloadEnabled = false;
        m_debugMode = false;
        m_timeSinceLastCheck = 0.0f;
        
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
        m_fileTimestamps.clear();
        m_hotReloadCallback = nullptr;
        
        m_initialized = false;
        LOG_INFO("ShaderManager shutdown complete");
    }

    void ShaderManager::Update(float deltaTime) {
        if (!m_initialized || !m_hotReloadEnabled) {
            return;
        }

        m_timeSinceLastCheck += deltaTime;
        if (m_timeSinceLastCheck >= m_hotReloadCheckInterval) {
            CheckForShaderChanges();
            m_timeSinceLastCheck = 0.0f;
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

        // Track file timestamps for hot reload
        if (desc.enableHotReload) {
            if (!desc.vertexPath.empty() && std::filesystem::exists(desc.vertexPath)) {
                m_fileTimestamps[desc.vertexPath] = std::filesystem::last_write_time(desc.vertexPath);
            }
            if (!desc.fragmentPath.empty() && std::filesystem::exists(desc.fragmentPath)) {
                m_fileTimestamps[desc.fragmentPath] = std::filesystem::last_write_time(desc.fragmentPath);
            }
            if (!desc.geometryPath.empty() && std::filesystem::exists(desc.geometryPath)) {
                m_fileTimestamps[desc.geometryPath] = std::filesystem::last_write_time(desc.geometryPath);
            }
            if (!desc.computePath.empty() && std::filesystem::exists(desc.computePath)) {
                m_fileTimestamps[desc.computePath] = std::filesystem::last_write_time(desc.computePath);
            }
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
            
            m_shaders.erase(it);
            m_shaderDescs.erase(name);
            
            // Remove file timestamps
            auto descIt = m_shaderDescs.find(name);
            if (descIt != m_shaderDescs.end()) {
                const auto& desc = descIt->second;
                m_fileTimestamps.erase(desc.vertexPath);
                m_fileTimestamps.erase(desc.fragmentPath);
                m_fileTimestamps.erase(desc.geometryPath);
                m_fileTimestamps.erase(desc.computePath);
            }
            
            UpdateShaderStats();
        }
    }

    void ShaderManager::UnloadAllShaders() {
        if (m_debugMode) {
            LOG_INFO("Unloading all shaders (" + std::to_string(m_shaders.size()) + " shaders)");
        }
        
        m_shaders.clear();
        m_shaderDescs.clear();
        m_fileTimestamps.clear();
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
        
        if (m_debugMode) {
            LOG_INFO("Hot reload " + std::string(enable ? "enabled" : "disabled"));
        }
    }

    void ShaderManager::SetHotReloadCallback(std::function<void(const std::string&)> callback) {
        m_hotReloadCallback = callback;
    }

    void ShaderManager::CheckForShaderChanges() {
        if (!m_hotReloadEnabled) {
            return;
        }

        std::vector<std::string> shadersToReload;

        // Check file timestamps
        for (const auto& pair : m_fileTimestamps) {
            const std::string& filepath = pair.first;
            const auto& lastTime = pair.second;

            if (std::filesystem::exists(filepath)) {
                auto currentTime = std::filesystem::last_write_time(filepath);
                if (currentTime > lastTime) {
                    // Find which shader uses this file
                    for (const auto& shaderPair : m_shaderDescs) {
                        const auto& desc = shaderPair.second;
                        if (desc.vertexPath == filepath || desc.fragmentPath == filepath ||
                            desc.geometryPath == filepath || desc.computePath == filepath) {
                            shadersToReload.push_back(shaderPair.first);
                            break;
                        }
                    }
                }
            }
        }

        // Reload changed shaders
        for (const std::string& shaderName : shadersToReload) {
            ReloadShader(shaderName);
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

        // Create new shader
        auto newShader = CreateShaderFromDesc(desc);
        if (newShader) {
            // Replace old shader
            m_shaders[name] = newShader;
            
            // Update file timestamps
            if (!desc.vertexPath.empty() && std::filesystem::exists(desc.vertexPath)) {
                m_fileTimestamps[desc.vertexPath] = std::filesystem::last_write_time(desc.vertexPath);
            }
            if (!desc.fragmentPath.empty() && std::filesystem::exists(desc.fragmentPath)) {
                m_fileTimestamps[desc.fragmentPath] = std::filesystem::last_write_time(desc.fragmentPath);
            }
            if (!desc.geometryPath.empty() && std::filesystem::exists(desc.geometryPath)) {
                m_fileTimestamps[desc.geometryPath] = std::filesystem::last_write_time(desc.geometryPath);
            }
            if (!desc.computePath.empty() && std::filesystem::exists(desc.computePath)) {
                m_fileTimestamps[desc.computePath] = std::filesystem::last_write_time(desc.computePath);
            }
            
            // Notify callback
            if (m_hotReloadCallback) {
                m_hotReloadCallback(name);
            }
            
            if (m_debugMode) {
                LOG_INFO("Shader reloaded successfully: " + name);
            }
        } else {
            LOG_ERROR("Failed to reload shader: " + name);
            m_stats.compilationErrors++;
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
}