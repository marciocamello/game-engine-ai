#pragma once

#include "IEngineModule.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace GameEngine {

    enum class ModuleLoadResult {
        Success,
        FileNotFound,
        InvalidModule,
        AlreadyLoaded,
        DependencyMissing,
        InitializationFailed,
        UnknownError
    };

    struct ModuleLoadInfo {
        std::string name;
        std::string path;
        std::string version;
        ModuleType type;
        std::vector<std::string> dependencies;
        bool isLoaded = false;
        bool isEnabled = false;
    };

    class DynamicModuleLoader {
    public:
        static DynamicModuleLoader& GetInstance();

        // Module discovery
        std::vector<ModuleLoadInfo> DiscoverModules(const std::string& searchPath = "");
        std::vector<ModuleLoadInfo> GetAvailableModules() const;
        std::vector<ModuleLoadInfo> GetLoadedModules() const;

        // Module loading/unloading
        ModuleLoadResult LoadModule(const std::string& name, const ModuleConfig& config = {});
        ModuleLoadResult UnloadModule(const std::string& name);
        ModuleLoadResult ReloadModule(const std::string& name, const ModuleConfig& config = {});

        // Runtime module management
        bool EnableModule(const std::string& name);
        bool DisableModule(const std::string& name);
        bool IsModuleLoaded(const std::string& name) const;
        bool IsModuleEnabled(const std::string& name) const;

        // Hot-swapping support
        bool SupportsHotSwap(const std::string& name) const;
        ModuleLoadResult HotSwapModule(const std::string& name, const std::string& newPath);
        void EnableHotSwapWatching(bool enabled);
        bool IsHotSwapWatchingEnabled() const;

        // Module information
        ModuleLoadInfo GetModuleInfo(const std::string& name) const;
        std::string GetModuleLoadResultString(ModuleLoadResult result) const;

        // Error handling
        std::string GetLastError() const;
        void ClearLastError();

        // Module factory registration (for built-in modules)
        using ModuleFactory = std::function<std::unique_ptr<IEngineModule>()>;
        void RegisterModuleFactory(const std::string& name, ModuleFactory factory);
        void UnregisterModuleFactory(const std::string& name);

    private:
        DynamicModuleLoader() = default;
        ~DynamicModuleLoader() = default;
        DynamicModuleLoader(const DynamicModuleLoader&) = delete;
        DynamicModuleLoader& operator=(const DynamicModuleLoader&) = delete;

        // Internal module management
        std::unique_ptr<IEngineModule> CreateModuleInstance(const std::string& name);
        bool ValidateModule(IEngineModule* module, const std::string& expectedName);
        std::vector<std::string> GetDefaultSearchPaths() const;
        
        // File system operations
        bool FileExists(const std::string& path) const;
        std::vector<std::string> FindModuleFiles(const std::string& searchPath) const;
        ModuleLoadInfo ParseModuleInfo(const std::string& filePath) const;

        // Hot-swap support
        void StartFileWatcher();
        void StopFileWatcher();
        void OnFileChanged(const std::string& filePath);

        // Data members
        std::unordered_map<std::string, ModuleLoadInfo> m_availableModules;
        std::unordered_map<std::string, std::unique_ptr<IEngineModule>> m_loadedModules;
        std::unordered_map<std::string, ModuleFactory> m_moduleFactories;
        
        std::string m_lastError;
        bool m_hotSwapEnabled = false;
        bool m_fileWatcherActive = false;
    };
}