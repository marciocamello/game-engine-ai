#pragma once

#include "IEngineModule.h"
#include "DynamicModuleLoader.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace GameEngine {

    enum class ModuleEvent {
        Loaded,
        Unloaded,
        Enabled,
        Disabled,
        Reloaded,
        Error
    };

    struct ModuleEventData {
        std::string moduleName;
        ModuleEvent event;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
    };

    using ModuleEventCallback = std::function<void(const ModuleEventData&)>;

    class RuntimeModuleManager {
    public:
        static RuntimeModuleManager& GetInstance();

        // Initialization and shutdown
        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // Module discovery and management
        bool RefreshModuleList();
        std::vector<ModuleLoadInfo> GetAvailableModules() const;
        std::vector<ModuleLoadInfo> GetLoadedModules() const;
        std::vector<ModuleLoadInfo> GetEnabledModules() const;

        // Runtime module operations
        bool LoadModule(const std::string& name, const ModuleConfig& config = {});
        bool UnloadModule(const std::string& name);
        bool ReloadModule(const std::string& name);
        bool EnableModule(const std::string& name);
        bool DisableModule(const std::string& name);

        // Batch operations
        bool LoadModules(const std::vector<std::string>& moduleNames);
        bool UnloadModules(const std::vector<std::string>& moduleNames);
        bool EnableModules(const std::vector<std::string>& moduleNames);
        bool DisableModules(const std::vector<std::string>& moduleNames);

        // Hot-swap functionality
        bool EnableHotSwap(bool enabled);
        bool IsHotSwapEnabled() const;
        bool HotSwapModule(const std::string& name, const std::string& newPath = "");

        // Module state queries
        bool IsModuleAvailable(const std::string& name) const;
        bool IsModuleLoaded(const std::string& name) const;
        bool IsModuleEnabled(const std::string& name) const;
        ModuleLoadInfo GetModuleInfo(const std::string& name) const;

        // Configuration management
        bool SaveModuleConfiguration(const std::string& filePath = "") const;
        bool LoadModuleConfiguration(const std::string& filePath = "");
        EngineConfig GetCurrentConfiguration() const;
        bool ApplyConfiguration(const EngineConfig& config);

        // Event system
        void RegisterEventCallback(ModuleEventCallback callback);
        void UnregisterEventCallback(ModuleEventCallback callback);
        std::vector<ModuleEventData> GetRecentEvents(size_t maxEvents = 100) const;

        // Dependency management
        std::vector<std::string> GetModuleDependencies(const std::string& name) const;
        std::vector<std::string> GetDependentModules(const std::string& name) const;
        bool CanUnloadModule(const std::string& name) const;
        std::vector<std::string> GetLoadOrder(const std::vector<std::string>& moduleNames) const;

        // Error handling
        std::string GetLastError() const;
        bool HasErrors() const;
        void ClearErrors();

        // Statistics and monitoring
        struct ModuleStats {
            size_t totalModules = 0;
            size_t loadedModules = 0;
            size_t enabledModules = 0;
            size_t failedModules = 0;
            std::chrono::system_clock::time_point lastRefresh;
        };
        ModuleStats GetStatistics() const;

    private:
        RuntimeModuleManager() = default;
        ~RuntimeModuleManager() = default;
        RuntimeModuleManager(const RuntimeModuleManager&) = delete;
        RuntimeModuleManager& operator=(const RuntimeModuleManager&) = delete;

        // Internal event handling
        void FireEvent(const std::string& moduleName, ModuleEvent event, const std::string& message = "");
        void AddEventToHistory(const ModuleEventData& eventData);

        // Internal validation
        bool ValidateModuleOperation(const std::string& name, const std::string& operation) const;
        bool CheckDependencies(const std::string& name, bool loading) const;

        // Configuration helpers
        std::string GetDefaultConfigPath() const;
        bool CreateDefaultConfiguration() const;

        // Data members
        DynamicModuleLoader* m_loader = nullptr;
        bool m_initialized = false;
        mutable std::string m_lastError;
        
        // Event system
        std::vector<ModuleEventCallback> m_eventCallbacks;
        std::vector<ModuleEventData> m_eventHistory;
        static constexpr size_t MAX_EVENT_HISTORY = 1000;

        // Statistics
        mutable ModuleStats m_stats;
        mutable std::chrono::system_clock::time_point m_lastStatsUpdate;
    };
}