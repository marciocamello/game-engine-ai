#pragma once

#include "Core/Math.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <filesystem>
#include <vector>

namespace GameEngine {
    class Shader;

    struct ShaderDesc {
        std::string name;
        std::string vertexPath;
        std::string fragmentPath;
        std::string geometryPath;
        std::string computePath;
        std::string tessControlPath;
        std::string tessEvaluationPath;
        
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
        void CheckForShaderChanges();
        void ReloadShader(const std::string& name);
        void ReloadAllShaders();

        // Performance and debugging
        ShaderStats GetShaderStats() const;
        void SetDebugMode(bool enabled) { m_debugMode = enabled; }
        bool IsDebugMode() const { return m_debugMode; }

        // Shader compilation and caching
        void PrecompileShaders();
        void ClearShaderCache();

    private:
        ShaderManager() = default;
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        // Internal shader management
        std::shared_ptr<Shader> CreateShaderFromDesc(const ShaderDesc& desc);
        bool ValidateShaderDesc(const ShaderDesc& desc);
        void UpdateShaderStats();

        // Member variables
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
        std::unordered_map<std::string, ShaderDesc> m_shaderDescs;
        std::unordered_map<std::string, std::filesystem::file_time_type> m_fileTimestamps;

        bool m_initialized = false;
        bool m_hotReloadEnabled = false;
        bool m_debugMode = false;
        float m_hotReloadCheckInterval = 0.5f;
        float m_timeSinceLastCheck = 0.0f;

        std::function<void(const std::string&)> m_hotReloadCallback;
        ShaderStats m_stats;
    };
}