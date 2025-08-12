#include "Core/RuntimeModuleManager.h"
#include "Core/Logger.h"
#include "Core/ModuleConfigLoader.h"
#include "Core/ModuleRegistry.h"
#include <algorithm>
#include <filesystem>
#include <chrono>

namespace GameEngine {

    RuntimeModuleManager& RuntimeModuleManager::GetInstance() {
        static RuntimeModuleManager instance;
        return instance;
    }

    bool RuntimeModuleManager::Initialize() {
        if (m_initialized) {
            LOG_WARNING("RuntimeModuleManager already initialized");
            return true;
        }

        try {
            m_loader = &DynamicModuleLoader::GetInstance();
            
            // Discover available modules
            if (!RefreshModuleList()) {
                LOG_ERROR("Failed to refresh module list during initialization");
                return false;
            }

            m_initialized = true;
            LOG_INFO("RuntimeModuleManager initialized successfully");
            
            FireEvent("RuntimeModuleManager", ModuleEvent::Loaded, "Runtime module manager initialized");
            return true;
        }
        catch (const std::exception& e) {
            m_lastError = "Exception during initialization: " + std::string(e.what());
            LOG_ERROR(m_lastError);
            return false;
        }
    }

    void RuntimeModuleManager::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down RuntimeModuleManager...");

        // Disable hot-swap
        EnableHotSwap(false);

        // Clear event callbacks
        m_eventCallbacks.clear();
        m_eventHistory.clear();

        m_initialized = false;
        m_loader = nullptr;

        LOG_INFO("RuntimeModuleManager shutdown complete");
    }

    bool RuntimeModuleManager::RefreshModuleList() {
        if (!m_loader) {
            m_lastError = "Module loader not initialized";
            return false;
        }

        try {
            m_loader->DiscoverModules();
            
            // Update statistics
            m_stats.lastRefresh = std::chrono::system_clock::now();
            
            LOG_INFO("Module list refreshed successfully");
            return true;
        }
        catch (const std::exception& e) {
            m_lastError = "Exception during module refresh: " + std::string(e.what());
            LOG_ERROR(m_lastError);
            return false;
        }
    }

    std::vector<ModuleLoadInfo> RuntimeModuleManager::GetAvailableModules() const {
        if (!m_loader) {
            return {};
        }
        return m_loader->GetAvailableModules();
    }

    std::vector<ModuleLoadInfo> RuntimeModuleManager::GetLoadedModules() const {
        if (!m_loader) {
            return {};
        }
        return m_loader->GetLoadedModules();
    }

    std::vector<ModuleLoadInfo> RuntimeModuleManager::GetEnabledModules() const {
        std::vector<ModuleLoadInfo> enabledModules;
        auto loadedModules = GetLoadedModules();
        
        for (const auto& module : loadedModules) {
            if (module.isEnabled) {
                enabledModules.push_back(module);
            }
        }
        
        return enabledModules;
    }

    bool RuntimeModuleManager::LoadModule(const std::string& name, const ModuleConfig& config) {
        if (!ValidateModuleOperation(name, "load")) {
            return false;
        }

        if (!CheckDependencies(name, true)) {
            return false;
        }

        ModuleLoadResult result = m_loader->LoadModule(name, config);
        if (result == ModuleLoadResult::Success) {
            FireEvent(name, ModuleEvent::Loaded, "Module loaded successfully");
            LOG_INFO("Module loaded: " + name);
            return true;
        } else {
            std::string error = "Failed to load module '" + name + "': " + 
                              m_loader->GetModuleLoadResultString(result);
            if (!m_loader->GetLastError().empty()) {
                error += " (" + m_loader->GetLastError() + ")";
            }
            m_lastError = error;
            FireEvent(name, ModuleEvent::Error, error);
            LOG_ERROR(error);
            return false;
        }
    }

    bool RuntimeModuleManager::UnloadModule(const std::string& name) {
        if (!ValidateModuleOperation(name, "unload")) {
            return false;
        }

        if (!CanUnloadModule(name)) {
            m_lastError = "Cannot unload module '" + name + "' - other modules depend on it";
            FireEvent(name, ModuleEvent::Error, m_lastError);
            return false;
        }

        ModuleLoadResult result = m_loader->UnloadModule(name);
        if (result == ModuleLoadResult::Success) {
            FireEvent(name, ModuleEvent::Unloaded, "Module unloaded successfully");
            LOG_INFO("Module unloaded: " + name);
            return true;
        } else {
            std::string error = "Failed to unload module '" + name + "': " + 
                              m_loader->GetModuleLoadResultString(result);
            if (!m_loader->GetLastError().empty()) {
                error += " (" + m_loader->GetLastError() + ")";
            }
            m_lastError = error;
            FireEvent(name, ModuleEvent::Error, error);
            LOG_ERROR(error);
            return false;
        }
    }

    bool RuntimeModuleManager::ReloadModule(const std::string& name) {
        if (!ValidateModuleOperation(name, "reload")) {
            return false;
        }

        ModuleLoadResult result = m_loader->ReloadModule(name);
        if (result == ModuleLoadResult::Success) {
            FireEvent(name, ModuleEvent::Reloaded, "Module reloaded successfully");
            LOG_INFO("Module reloaded: " + name);
            return true;
        } else {
            std::string error = "Failed to reload module '" + name + "': " + 
                              m_loader->GetModuleLoadResultString(result);
            if (!m_loader->GetLastError().empty()) {
                error += " (" + m_loader->GetLastError() + ")";
            }
            m_lastError = error;
            FireEvent(name, ModuleEvent::Error, error);
            LOG_ERROR(error);
            return false;
        }
    }

    bool RuntimeModuleManager::EnableModule(const std::string& name) {
        if (!ValidateModuleOperation(name, "enable")) {
            return false;
        }

        if (!IsModuleLoaded(name)) {
            m_lastError = "Cannot enable module '" + name + "' - module is not loaded";
            return false;
        }

        if (m_loader->EnableModule(name)) {
            FireEvent(name, ModuleEvent::Enabled, "Module enabled successfully");
            LOG_INFO("Module enabled: " + name);
            return true;
        } else {
            m_lastError = "Failed to enable module '" + name + "'";
            if (!m_loader->GetLastError().empty()) {
                m_lastError += ": " + m_loader->GetLastError();
            }
            FireEvent(name, ModuleEvent::Error, m_lastError);
            LOG_ERROR(m_lastError);
            return false;
        }
    }

    bool RuntimeModuleManager::DisableModule(const std::string& name) {
        if (!ValidateModuleOperation(name, "disable")) {
            return false;
        }

        if (!IsModuleLoaded(name)) {
            m_lastError = "Cannot disable module '" + name + "' - module is not loaded";
            return false;
        }

        if (m_loader->DisableModule(name)) {
            FireEvent(name, ModuleEvent::Disabled, "Module disabled successfully");
            LOG_INFO("Module disabled: " + name);
            return true;
        } else {
            m_lastError = "Failed to disable module '" + name + "'";
            if (!m_loader->GetLastError().empty()) {
                m_lastError += ": " + m_loader->GetLastError();
            }
            FireEvent(name, ModuleEvent::Error, m_lastError);
            LOG_ERROR(m_lastError);
            return false;
        }
    }

    bool RuntimeModuleManager::LoadModules(const std::vector<std::string>& moduleNames) {
        // Get proper load order
        std::vector<std::string> orderedNames = GetLoadOrder(moduleNames);
        
        bool allSucceeded = true;
        for (const std::string& name : orderedNames) {
            if (!LoadModule(name)) {
                allSucceeded = false;
                LOG_ERROR("Failed to load module in batch operation: " + name);
                // Continue loading other modules
            }
        }
        
        return allSucceeded;
    }

    bool RuntimeModuleManager::UnloadModules(const std::vector<std::string>& moduleNames) {
        // Unload in reverse dependency order
        std::vector<std::string> orderedNames = GetLoadOrder(moduleNames);
        std::reverse(orderedNames.begin(), orderedNames.end());
        
        bool allSucceeded = true;
        for (const std::string& name : orderedNames) {
            if (IsModuleLoaded(name) && !UnloadModule(name)) {
                allSucceeded = false;
                LOG_ERROR("Failed to unload module in batch operation: " + name);
                // Continue unloading other modules
            }
        }
        
        return allSucceeded;
    }

    bool RuntimeModuleManager::EnableModules(const std::vector<std::string>& moduleNames) {
        bool allSucceeded = true;
        for (const std::string& name : moduleNames) {
            if (!EnableModule(name)) {
                allSucceeded = false;
                LOG_ERROR("Failed to enable module in batch operation: " + name);
            }
        }
        return allSucceeded;
    }

    bool RuntimeModuleManager::DisableModules(const std::vector<std::string>& moduleNames) {
        bool allSucceeded = true;
        for (const std::string& name : moduleNames) {
            if (!DisableModule(name)) {
                allSucceeded = false;
                LOG_ERROR("Failed to disable module in batch operation: " + name);
            }
        }
        return allSucceeded;
    }

    bool RuntimeModuleManager::EnableHotSwap(bool enabled) {
        if (!m_loader) {
            m_lastError = "Module loader not initialized";
            return false;
        }

        m_loader->EnableHotSwapWatching(enabled);
        LOG_INFO(enabled ? "Hot-swap enabled" : "Hot-swap disabled");
        return true;
    }

    bool RuntimeModuleManager::IsHotSwapEnabled() const {
        return m_loader ? m_loader->IsHotSwapWatchingEnabled() : false;
    }

    bool RuntimeModuleManager::HotSwapModule(const std::string& name, const std::string& newPath) {
        if (!ValidateModuleOperation(name, "hot-swap")) {
            return false;
        }

        if (!m_loader->SupportsHotSwap(name)) {
            m_lastError = "Module '" + name + "' does not support hot-swapping";
            return false;
        }

        ModuleLoadResult result = m_loader->HotSwapModule(name, newPath);
        if (result == ModuleLoadResult::Success) {
            FireEvent(name, ModuleEvent::Reloaded, "Module hot-swapped successfully");
            LOG_INFO("Module hot-swapped: " + name);
            return true;
        } else {
            std::string error = "Failed to hot-swap module '" + name + "': " + 
                              m_loader->GetModuleLoadResultString(result);
            if (!m_loader->GetLastError().empty()) {
                error += " (" + m_loader->GetLastError() + ")";
            }
            m_lastError = error;
            FireEvent(name, ModuleEvent::Error, error);
            LOG_ERROR(error);
            return false;
        }
    }

    bool RuntimeModuleManager::IsModuleAvailable(const std::string& name) const {
        auto modules = GetAvailableModules();
        return std::any_of(modules.begin(), modules.end(),
            [&name](const ModuleLoadInfo& info) { return info.name == name; });
    }

    bool RuntimeModuleManager::IsModuleLoaded(const std::string& name) const {
        return m_loader ? m_loader->IsModuleLoaded(name) : false;
    }

    bool RuntimeModuleManager::IsModuleEnabled(const std::string& name) const {
        return m_loader ? m_loader->IsModuleEnabled(name) : false;
    }

    ModuleLoadInfo RuntimeModuleManager::GetModuleInfo(const std::string& name) const {
        return m_loader ? m_loader->GetModuleInfo(name) : ModuleLoadInfo{};
    }

    bool RuntimeModuleManager::SaveModuleConfiguration(const std::string& filePath) const {
        std::string configPath = filePath.empty() ? GetDefaultConfigPath() : filePath;
        
        try {
            EngineConfig config = GetCurrentConfiguration();
            return ModuleConfigLoader::SaveToFile(config, configPath);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception while saving module configuration: " + std::string(e.what()));
            return false;
        }
    }

    bool RuntimeModuleManager::LoadModuleConfiguration(const std::string& filePath) {
        std::string configPath = filePath.empty() ? GetDefaultConfigPath() : filePath;
        
        try {
            auto configOpt = ModuleConfigLoader::LoadFromFile(configPath);
            if (configOpt) {
                return ApplyConfiguration(*configOpt);
            } else {
                m_lastError = "Failed to load configuration from: " + configPath;
                return false;
            }
        }
        catch (const std::exception& e) {
            m_lastError = "Exception while loading module configuration: " + std::string(e.what());
            LOG_ERROR(m_lastError);
            return false;
        }
    }

    EngineConfig RuntimeModuleManager::GetCurrentConfiguration() const {
        EngineConfig config;
        config.engineVersion = "1.0.0";
        config.configVersion = "1.0.0";
        
        auto loadedModules = GetLoadedModules();
        for (const auto& moduleInfo : loadedModules) {
            ModuleConfig moduleConfig;
            moduleConfig.name = moduleInfo.name;
            moduleConfig.version = moduleInfo.version;
            moduleConfig.enabled = moduleInfo.isEnabled;
            config.modules.push_back(moduleConfig);
        }
        
        return config;
    }

    bool RuntimeModuleManager::ApplyConfiguration(const EngineConfig& config) {
        LOG_INFO("Applying module configuration...");
        
        bool success = true;
        
        // Load and configure modules
        for (const auto& moduleConfig : config.modules) {
            if (!IsModuleLoaded(moduleConfig.name)) {
                if (!LoadModule(moduleConfig.name, moduleConfig)) {
                    success = false;
                    continue;
                }
            }
            
            // Set enabled state
            if (moduleConfig.enabled && !IsModuleEnabled(moduleConfig.name)) {
                if (!EnableModule(moduleConfig.name)) {
                    success = false;
                }
            } else if (!moduleConfig.enabled && IsModuleEnabled(moduleConfig.name)) {
                if (!DisableModule(moduleConfig.name)) {
                    success = false;
                }
            }
        }
        
        LOG_INFO(std::string("Module configuration applied ") + (success ? "successfully" : "with errors"));
        return success;
    }

    void RuntimeModuleManager::RegisterEventCallback(ModuleEventCallback callback) {
        m_eventCallbacks.push_back(callback);
    }

    void RuntimeModuleManager::UnregisterEventCallback(ModuleEventCallback callback) {
        // Note: This is a simplified implementation. In practice, you'd need a way to identify callbacks
        // For now, we'll clear all callbacks if the provided callback matches any
        m_eventCallbacks.clear();
    }

    std::vector<ModuleEventData> RuntimeModuleManager::GetRecentEvents(size_t maxEvents) const {
        size_t startIndex = 0;
        if (m_eventHistory.size() > maxEvents) {
            startIndex = m_eventHistory.size() - maxEvents;
        }
        
        return std::vector<ModuleEventData>(
            m_eventHistory.begin() + static_cast<std::vector<ModuleEventData>::difference_type>(startIndex),
            m_eventHistory.end()
        );
    }

    std::vector<std::string> RuntimeModuleManager::GetModuleDependencies(const std::string& name) const {
        ModuleLoadInfo info = GetModuleInfo(name);
        return info.dependencies;
    }

    std::vector<std::string> RuntimeModuleManager::GetDependentModules(const std::string& name) const {
        std::vector<std::string> dependents;
        auto loadedModules = GetLoadedModules();
        
        for (const auto& module : loadedModules) {
            auto dependencies = module.dependencies;
            if (std::find(dependencies.begin(), dependencies.end(), name) != dependencies.end()) {
                dependents.push_back(module.name);
            }
        }
        
        return dependents;
    }

    bool RuntimeModuleManager::CanUnloadModule(const std::string& name) const {
        // Check if any loaded modules depend on this module
        std::vector<std::string> dependents = GetDependentModules(name);
        return dependents.empty();
    }

    std::vector<std::string> RuntimeModuleManager::GetLoadOrder(const std::vector<std::string>& moduleNames) const {
        // Simple topological sort based on dependencies
        std::vector<std::string> result;
        std::vector<std::string> remaining = moduleNames;
        
        while (!remaining.empty()) {
            bool progress = false;
            
            for (auto it = remaining.begin(); it != remaining.end(); ) {
                const std::string& moduleName = *it;
                ModuleLoadInfo info = GetModuleInfo(moduleName);
                
                // Check if all dependencies are already in result or not in the list
                bool canLoad = true;
                for (const std::string& dep : info.dependencies) {
                    if (std::find(moduleNames.begin(), moduleNames.end(), dep) != moduleNames.end() &&
                        std::find(result.begin(), result.end(), dep) == result.end()) {
                        canLoad = false;
                        break;
                    }
                }
                
                if (canLoad) {
                    result.push_back(moduleName);
                    it = remaining.erase(it);
                    progress = true;
                } else {
                    ++it;
                }
            }
            
            if (!progress) {
                // Circular dependency or missing dependency
                LOG_WARNING("Circular dependency detected or missing dependencies in module load order");
                // Add remaining modules anyway
                result.insert(result.end(), remaining.begin(), remaining.end());
                break;
            }
        }
        
        return result;
    }

    std::string RuntimeModuleManager::GetLastError() const {
        return m_lastError;
    }

    bool RuntimeModuleManager::HasErrors() const {
        return !m_lastError.empty();
    }

    void RuntimeModuleManager::ClearErrors() {
        m_lastError.clear();
    }

    RuntimeModuleManager::ModuleStats RuntimeModuleManager::GetStatistics() const {
        auto now = std::chrono::system_clock::now();
        
        // Update stats if they're stale (older than 1 second)
        if (now - m_lastStatsUpdate > std::chrono::seconds(1)) {
            auto availableModules = GetAvailableModules();
            auto loadedModules = GetLoadedModules();
            auto enabledModules = GetEnabledModules();
            
            m_stats.totalModules = availableModules.size();
            m_stats.loadedModules = loadedModules.size();
            m_stats.enabledModules = enabledModules.size();
            m_stats.failedModules = 0; // Would need to track failed modules
            
            m_lastStatsUpdate = now;
        }
        
        return m_stats;
    }

    void RuntimeModuleManager::FireEvent(const std::string& moduleName, ModuleEvent event, const std::string& message) {
        ModuleEventData eventData;
        eventData.moduleName = moduleName;
        eventData.event = event;
        eventData.message = message;
        eventData.timestamp = std::chrono::system_clock::now();
        
        // Add to history
        AddEventToHistory(eventData);
        
        // Notify callbacks
        for (const auto& callback : m_eventCallbacks) {
            try {
                callback(eventData);
            }
            catch (const std::exception& e) {
                LOG_ERROR("Exception in module event callback: " + std::string(e.what()));
            }
        }
    }

    void RuntimeModuleManager::AddEventToHistory(const ModuleEventData& eventData) {
        m_eventHistory.push_back(eventData);
        
        // Limit history size
        if (m_eventHistory.size() > MAX_EVENT_HISTORY) {
            m_eventHistory.erase(m_eventHistory.begin());
        }
    }

    bool RuntimeModuleManager::ValidateModuleOperation(const std::string& name, const std::string& operation) const {
        if (!m_initialized) {
            m_lastError = "RuntimeModuleManager not initialized";
            return false;
        }

        if (!m_loader) {
            m_lastError = "Module loader not available";
            return false;
        }

        if (name.empty()) {
            m_lastError = "Module name cannot be empty";
            return false;
        }

        return true;
    }

    bool RuntimeModuleManager::CheckDependencies(const std::string& name, bool loading) const {
        ModuleLoadInfo info = GetModuleInfo(name);
        
        if (loading) {
            // Check that all dependencies are loaded
            for (const std::string& dep : info.dependencies) {
                if (!IsModuleLoaded(dep)) {
                    m_lastError = "Missing dependency '" + dep + "' for module '" + name + "'";
                    return false;
                }
            }
        }
        
        return true;
    }

    std::string RuntimeModuleManager::GetDefaultConfigPath() const {
        return "runtime_modules.json";
    }

    bool RuntimeModuleManager::CreateDefaultConfiguration() const {
        EngineConfig defaultConfig = ModuleConfigLoader::CreateDefaultConfig();
        return ModuleConfigLoader::SaveToFile(defaultConfig, GetDefaultConfigPath());
    }
}