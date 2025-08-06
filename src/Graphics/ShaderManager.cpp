#include "Graphics/ShaderManager.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderHotReloader.h"
#include "Graphics/ShaderVariantManager.h"
#include "Graphics/ShaderBackgroundCompiler.h"
#include "Graphics/ShaderFallbackManager.h"
#include "Graphics/HardwareCapabilities.h"
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
        
        // Get reference to variant manager
        m_variantManager = &ShaderVariantManager::GetInstance();
        
        // Set up hot reload callbacks
        m_hotReloader->SetReloadCallback([this](const std::string& filepath) {
            OnShaderFileChanged(filepath);
        });
        
        m_hotReloader->SetErrorCallback([this](const std::string& filepath, const std::string& error) {
            OnShaderFileError(filepath, error);
        });
        
        // Initialize background compiler if enabled
        if (m_backgroundCompilationEnabled) {
            if (!ShaderBackgroundCompiler::GetInstance().Initialize()) {
                LOG_WARNING("Failed to initialize ShaderBackgroundCompiler, disabling background compilation");
                m_backgroundCompilationEnabled = false;
            }
        }
        
        // Initialize hardware capabilities and fallback system if enabled
        if (m_fallbackEnabled) {
            InitializeHardwareCapabilities();
        }
        
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
        
        // Shutdown background compiler
        if (m_backgroundCompilationEnabled) {
            ShaderBackgroundCompiler::GetInstance().Shutdown();
        }
        
        m_variantManager = nullptr;
        
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

    std::shared_ptr<Shader> ShaderManager::GetShaderWithFallback(const std::string& name) {
        // First try to get the original shader
        auto originalShader = GetShader(name);
        
        // If fallback system is disabled, return original shader (or nullptr)
        if (!m_fallbackEnabled) {
            return originalShader;
        }
        
        // Check if a fallback is needed for this shader
        auto& fallbackManager = ShaderFallbackManager::GetInstance();
        if (!fallbackManager.IsInitialized()) {
            if (m_debugMode) {
                LOG_WARNING("Fallback system not initialized, returning original shader");
            }
            return originalShader;
        }
        
        // Try to get fallback shader
        auto fallbackShader = fallbackManager.GetFallbackShader(name);
        if (fallbackShader) {
            // Mark this shader as using fallback
            m_shaderFallbackStatus[name] = true;
            
            if (m_debugMode) {
                LOG_INFO("Using fallback shader for: " + name);
            }
            
            return fallbackShader;
        }
        
        // No fallback needed or available, return original shader
        m_shaderFallbackStatus[name] = false;
        return originalShader;
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

    // Shader variant support methods
    std::shared_ptr<Shader> ShaderManager::CreateShaderVariant(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_variantManager) {
            LOG_ERROR("ShaderVariantManager not available");
            return nullptr;
        }
        
        return m_variantManager->CreateVariant(baseName, variant);
    }

    std::shared_ptr<Shader> ShaderManager::GetShaderVariant(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_variantManager) {
            LOG_ERROR("ShaderVariantManager not available");
            return nullptr;
        }
        
        return m_variantManager->GetVariant(baseName, variant);
    }

    void ShaderManager::RemoveShaderVariant(const std::string& baseName, const ShaderVariant& variant) {
        if (!m_variantManager) {
            LOG_ERROR("ShaderVariantManager not available");
            return;
        }
        
        m_variantManager->RemoveVariant(baseName, variant);
    }

    std::vector<ShaderVariant> ShaderManager::GetShaderVariants(const std::string& baseName) const {
        if (!m_variantManager) {
            LOG_ERROR("ShaderVariantManager not available");
            return {};
        }
        
        return m_variantManager->GetVariants(baseName);
    }

    // Background compilation methods
    std::future<std::shared_ptr<Shader>> ShaderManager::LoadShaderAsync(const std::string& name, const ShaderDesc& desc) {
        if (!m_backgroundCompilationEnabled) {
            // Fallback to synchronous loading
            auto promise = std::promise<std::shared_ptr<Shader>>();
            auto future = promise.get_future();
            promise.set_value(LoadShader(name, desc));
            return future;
        }

        // Load shader sources
        std::string vertexSource, fragmentSource, geometrySource, computeSource;
        
        if (!desc.vertexPath.empty()) {
            std::ifstream file(desc.vertexPath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                vertexSource = buffer.str();
            }
        }
        
        if (!desc.fragmentPath.empty()) {
            std::ifstream file(desc.fragmentPath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                fragmentSource = buffer.str();
            }
        }
        
        if (!desc.geometryPath.empty()) {
            std::ifstream file(desc.geometryPath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                geometrySource = buffer.str();
            }
        }
        
        if (!desc.computePath.empty()) {
            std::ifstream file(desc.computePath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                computeSource = buffer.str();
            }
        }

        // Submit to background compiler
        return ShaderBackgroundCompiler::GetInstance().SubmitCompilationJob(
            name, vertexSource, fragmentSource, geometrySource, computeSource, desc.variant, 0,
            [this, name, desc](std::shared_ptr<Shader> shader) {
                if (shader) {
                    // Register the compiled shader
                    RegisterShader(name, shader);
                    m_shaderDescs[name] = desc;
                    
                    // Set up hot reload if enabled
                    if (desc.enableHotReload && m_hotReloader) {
                        RegisterShaderFiles(name, desc);
                    }
                }
            }
        );
    }

    std::future<std::shared_ptr<Shader>> ShaderManager::LoadShaderFromFilesAsync(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        ShaderDesc desc;
        desc.name = name;
        desc.vertexPath = vertexPath;
        desc.fragmentPath = fragmentPath;
        desc.enableHotReload = true;
        
        return LoadShaderAsync(name, desc);
    }

    void ShaderManager::StartProgressiveShaderLoading(const std::vector<std::string>& shaderPaths) {
        if (!m_backgroundCompilationEnabled) {
            LOG_WARNING("Background compilation disabled, cannot start progressive loading");
            return;
        }

        ShaderBackgroundCompiler::GetInstance().StartProgressiveLoading(shaderPaths);
        LOG_INFO("Started progressive loading of " + std::to_string(shaderPaths.size()) + " shaders");
    }

    void ShaderManager::StopProgressiveShaderLoading() {
        if (m_backgroundCompilationEnabled) {
            ShaderBackgroundCompiler::GetInstance().StopProgressiveLoading();
        }
    }

    void ShaderManager::PrecompileCommonVariants() {
        if (!m_backgroundCompilationEnabled) {
            LOG_WARNING("Background compilation disabled, cannot precompile variants");
            return;
        }

        ShaderBackgroundCompiler::GetInstance().PrecompileCommonVariants();
        LOG_INFO("Started precompilation of common shader variants");
    }

    void ShaderManager::EnableBackgroundCompilation(bool enable) {
        if (enable == m_backgroundCompilationEnabled) {
            return;
        }

        m_backgroundCompilationEnabled = enable;
        
        if (enable) {
            if (!ShaderBackgroundCompiler::GetInstance().Initialize()) {
                LOG_ERROR("Failed to initialize ShaderBackgroundCompiler");
                m_backgroundCompilationEnabled = false;
            } else {
                LOG_INFO("Background compilation enabled");
            }
        } else {
            ShaderBackgroundCompiler::GetInstance().Shutdown();
            LOG_INFO("Background compilation disabled");
        }
    }

    void ShaderManager::SetMaxBackgroundThreads(size_t count) {
        if (m_backgroundCompilationEnabled) {
            ShaderBackgroundCompiler::GetInstance().SetMaxWorkerThreads(count);
        }
    }

    void ShaderManager::PauseBackgroundCompilation() {
        if (m_backgroundCompilationEnabled) {
            ShaderBackgroundCompiler::GetInstance().PauseCompilation();
        }
    }

    void ShaderManager::ResumeBackgroundCompilation() {
        if (m_backgroundCompilationEnabled) {
            ShaderBackgroundCompiler::GetInstance().ResumeCompilation();
        }
    }

    bool ShaderManager::IsShaderUsingFallback(const std::string& name) const {
        auto it = m_shaderFallbackStatus.find(name);
        return (it != m_shaderFallbackStatus.end()) ? it->second : false;
    }

    std::vector<std::string> ShaderManager::GetShadersUsingFallbacks() const {
        std::vector<std::string> fallbackShaders;
        
        for (const auto& pair : m_shaderFallbackStatus) {
            if (pair.second) { // If using fallback
                fallbackShaders.push_back(pair.first);
            }
        }
        
        return fallbackShaders;
    }

    void ShaderManager::InitializeHardwareCapabilities() {
        if (!HardwareCapabilities::Initialize()) {
            LOG_ERROR("Failed to initialize hardware capabilities");
            return;
        }
        
        if (!ShaderFallbackManager::GetInstance().Initialize()) {
            LOG_ERROR("Failed to initialize shader fallback manager");
            return;
        }
        
        LOG_INFO("Hardware capabilities and fallback system initialized");
        
        // Log any shaders that will need fallbacks
        auto fallbackShaders = GetShadersUsingFallbacks();
        if (!fallbackShaders.empty()) {
            LOG_INFO("Shaders using fallbacks:");
            for (const auto& shaderName : fallbackShaders) {
                LOG_INFO("  - " + shaderName);
            }
        }
    }
}