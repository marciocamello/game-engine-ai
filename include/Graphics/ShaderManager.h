#pragma once

#include "Core/Math.h"
#include "Graphics/ShaderVariant.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <filesystem>
#include <vector>

namespace GameEngine {
    class Shader;
    class ShaderHotReloader;
    class ShaderVariantManager;

    struct ShaderDesc {
        std::string name;
        std::string vertexPath;
        std::string fragmentPath;
        std::string geometryPath;
        std::string computePath;
        std::string tessControlPath;
        std::string tessEvaluationPath;
        
        ShaderVariant variant; // Shader variant configuration
        bool enableHotReload = true;
        bool enableOptimization = true;
    };

    struct ShaderStats {
        size_t totalShaders = 0;
        size_t loadedShaders = 0;
        size_t compilationErrors = 0;
        size_t memoryUsage = 0;
        float averageCompileTime = 0.0f;
    };

    class ShaderManager {
    public:
        // Singleton access
        static ShaderManager& GetInstance();
        
        // Lifecycle
        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // Shader loading and management
        std::shared_ptr<Shader> LoadShader(const std::string& name, const ShaderDesc& desc);
        std::shared_ptr<Shader> LoadShaderFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
        std::shared_ptr<Shader> LoadShaderFromSource(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
        std::shared_ptr<Shader> GetShader(const std::string& name);
        void UnloadShader(const std::string& name);
        void UnloadAllShaders();

        // Shader registration and lookup
        bool RegisterShader(const std::string& name, std::shared_ptr<Shader> shader);
        bool HasShader(const std::string& name) const;
        std::vector<std::string> GetShaderNames() const;

        // Hot-reloading system
        void EnableHotReload(bool enable);
        bool IsHotReloadEnabled() const { return m_hotReloadEnabled; }
        void SetHotReloadCallback(std::function<void(const std::string&)> callback);
        void SetHotReloadErrorCallback(std::function<void(const std::string&, const std::string&)> callback);
        void CheckForShaderChanges();
        void ReloadShader(const std::string& name);
        void ReloadAllShaders();
        void SetHotReloadCheckInterval(float intervalSeconds);
        
        // Batch recompilation support
        void ReloadShadersFromFiles(const std::vector<std::string>& filepaths);
        void WatchShaderDirectory(const std::string& directory);
        void WatchShaderFile(const std::string& filepath);
        void UnwatchShaderFile(const std::string& filepath);

        // Performance and debugging
        ShaderStats GetShaderStats() const;
        void SetDebugMode(bool enabled) { m_debugMode = enabled; }
        bool IsDebugMode() const { return m_debugMode; }

        // Shader compilation and caching
        void PrecompileShaders();
        void ClearShaderCache();

        // Shader variant support
        std::shared_ptr<Shader> CreateShaderVariant(const std::string& baseName, const ShaderVariant& variant);
        std::shared_ptr<Shader> GetShaderVariant(const std::string& baseName, const ShaderVariant& variant);
        void RemoveShaderVariant(const std::string& baseName, const ShaderVariant& variant);
        std::vector<ShaderVariant> GetShaderVariants(const std::string& baseName) const;

    private:
        ShaderManager() = default;
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        // Internal shader management
        std::shared_ptr<Shader> CreateShaderFromDesc(const ShaderDesc& desc);
        bool ValidateShaderDesc(const ShaderDesc& desc);
        void UpdateShaderStats();
        
        // Hot-reload internal methods
        void OnShaderFileChanged(const std::string& filepath);
        void OnShaderFileError(const std::string& filepath, const std::string& error);
        void RegisterShaderFiles(const std::string& shaderName, const ShaderDesc& desc);
        void UnregisterShaderFiles(const std::string& shaderName);
        std::vector<std::string> GetShadersUsingFile(const std::string& filepath) const;

        // Member variables
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
        std::unordered_map<std::string, ShaderDesc> m_shaderDescs;
        std::unordered_map<std::string, std::string> m_fileToShaderMap; // Maps file paths to shader names

        bool m_initialized = false;
        bool m_hotReloadEnabled = false;
        bool m_debugMode = false;

        std::unique_ptr<ShaderHotReloader> m_hotReloader;
        std::function<void(const std::string&)> m_hotReloadCallback;
        std::function<void(const std::string&, const std::string&)> m_hotReloadErrorCallback;
        ShaderStats m_stats;
        
        ShaderVariantManager* m_variantManager = nullptr; // Reference to variant manager
    };
}