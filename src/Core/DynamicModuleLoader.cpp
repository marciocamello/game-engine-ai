#include "Core/DynamicModuleLoader.h"
#include "Core/Logger.h"
#include "Core/ModuleRegistry.h"
#include "../../engine/modules/OpenGLGraphicsModule.h"
#include "../../engine/modules/BulletPhysicsModule.h"
#include "../../engine/modules/audio-openal/OpenALAudioModule.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace GameEngine {

    DynamicModuleLoader& DynamicModuleLoader::GetInstance() {
        static DynamicModuleLoader instance;
        return instance;
    }

    std::vector<ModuleLoadInfo> DynamicModuleLoader::DiscoverModules(const std::string& searchPath) {
        std::vector<ModuleLoadInfo> discoveredModules;
        
        // Get search paths
        std::vector<std::string> searchPaths;
        if (!searchPath.empty()) {
            searchPaths.push_back(searchPath);
        } else {
            searchPaths = GetDefaultSearchPaths();
        }

        // Register built-in modules if not already registered
        if (m_moduleFactories.empty()) {
            RegisterModuleFactory("OpenGLGraphics", []() {
                return std::make_unique<Graphics::OpenGLGraphicsModule>();
            });
            RegisterModuleFactory("BulletPhysics", []() {
                return std::make_unique<Physics::BulletPhysicsModule>();
            });
            RegisterModuleFactory("OpenALAudio", []() {
                return std::make_unique<Audio::OpenALAudioModule>();
            });
        }

        // Add built-in modules to discovery
        for (const auto& factory : m_moduleFactories) {
            ModuleLoadInfo info;
            info.name = factory.first;
            info.path = "built-in";
            info.version = "1.0.0";
            
            // Create temporary instance to get module info
            auto tempModule = factory.second();
            if (tempModule) {
                info.type = tempModule->GetType();
                info.dependencies = tempModule->GetDependencies();
            }
            
            info.isLoaded = IsModuleLoaded(factory.first);
            info.isEnabled = IsModuleEnabled(factory.first);
            
            discoveredModules.push_back(info);
            m_availableModules[info.name] = info;
        }

        // Search for external modules in file system
        for (const std::string& path : searchPaths) {
            if (!std::filesystem::exists(path)) {
                continue;
            }

            std::vector<std::string> moduleFiles = FindModuleFiles(path);
            for (const std::string& filePath : moduleFiles) {
                ModuleLoadInfo info = ParseModuleInfo(filePath);
                if (!info.name.empty()) {
                    // Check if module is already discovered (built-in takes precedence)
                    if (m_availableModules.find(info.name) == m_availableModules.end()) {
                        info.isLoaded = IsModuleLoaded(info.name);
                        info.isEnabled = IsModuleEnabled(info.name);
                        discoveredModules.push_back(info);
                        m_availableModules[info.name] = info;
                    }
                }
            }
        }

        LOG_INFO("Discovered " + std::to_string(discoveredModules.size()) + " modules");
        return discoveredModules;
    }

    std::vector<ModuleLoadInfo> DynamicModuleLoader::GetAvailableModules() const {
        std::vector<ModuleLoadInfo> modules;
        for (const auto& pair : m_availableModules) {
            modules.push_back(pair.second);
        }
        return modules;
    }

    std::vector<ModuleLoadInfo> DynamicModuleLoader::GetLoadedModules() const {
        std::vector<ModuleLoadInfo> modules;
        for (const auto& pair : m_loadedModules) {
            auto it = m_availableModules.find(pair.first);
            if (it != m_availableModules.end()) {
                ModuleLoadInfo info = it->second;
                info.isLoaded = true;
                info.isEnabled = pair.second->IsEnabled();
                modules.push_back(info);
            }
        }
        return modules;
    }

    ModuleLoadResult DynamicModuleLoader::LoadModule(const std::string& name, const ModuleConfig& config) {
        ClearLastError();

        // Check if module is already loaded
        if (IsModuleLoaded(name)) {
            m_lastError = "Module '" + name + "' is already loaded";
            return ModuleLoadResult::AlreadyLoaded;
        }

        // Check if module is available
        auto it = m_availableModules.find(name);
        if (it == m_availableModules.end()) {
            // Try to discover modules first
            DiscoverModules();
            it = m_availableModules.find(name);
            if (it == m_availableModules.end()) {
                m_lastError = "Module '" + name + "' not found";
                return ModuleLoadResult::FileNotFound;
            }
        }

        try {
            // Create module instance
            std::unique_ptr<IEngineModule> module = CreateModuleInstance(name);
            if (!module) {
                m_lastError = "Failed to create instance of module '" + name + "'";
                return ModuleLoadResult::InvalidModule;
            }

            // Validate module
            if (!ValidateModule(module.get(), name)) {
                m_lastError = "Module validation failed for '" + name + "'";
                return ModuleLoadResult::InvalidModule;
            }

            // Check dependencies
            std::vector<std::string> dependencies = module->GetDependencies();
            for (const std::string& dependency : dependencies) {
                if (!IsModuleLoaded(dependency)) {
                    m_lastError = "Missing dependency '" + dependency + "' for module '" + name + "'";
                    return ModuleLoadResult::DependencyMissing;
                }
            }

            // Register with module registry
            ModuleRegistry& registry = ModuleRegistry::GetInstance();
            
            // Store the module before registering (in case registration fails)
            IEngineModule* modulePtr = module.get();
            m_loadedModules[name] = std::move(module);
            
            // Register with the registry
            registry.RegisterModule(std::unique_ptr<IEngineModule>(modulePtr));

            // Initialize if config is provided or use default
            ModuleConfig moduleConfig = config;
            if (moduleConfig.name.empty()) {
                moduleConfig.name = name;
                moduleConfig.version = modulePtr->GetVersion();
                moduleConfig.enabled = true;
            }

            if (!modulePtr->Initialize(moduleConfig)) {
                m_lastError = "Failed to initialize module '" + name + "'";
                // Clean up
                registry.UnregisterModule(name);
                m_loadedModules.erase(name);
                return ModuleLoadResult::InitializationFailed;
            }

            // Update module info
            m_availableModules[name].isLoaded = true;
            m_availableModules[name].isEnabled = modulePtr->IsEnabled();

            LOG_INFO("Successfully loaded module: " + name);
            return ModuleLoadResult::Success;
        }
        catch (const std::exception& e) {
            m_lastError = "Exception while loading module '" + name + "': " + e.what();
            return ModuleLoadResult::UnknownError;
        }
    }

    ModuleLoadResult DynamicModuleLoader::UnloadModule(const std::string& name) {
        ClearLastError();

        auto it = m_loadedModules.find(name);
        if (it == m_loadedModules.end()) {
            m_lastError = "Module '" + name + "' is not loaded";
            return ModuleLoadResult::FileNotFound;
        }

        try {
            // Shutdown the module
            if (it->second->IsInitialized()) {
                it->second->Shutdown();
            }

            // Unregister from module registry
            ModuleRegistry& registry = ModuleRegistry::GetInstance();
            registry.UnregisterModule(name);

            // Remove from loaded modules
            m_loadedModules.erase(it);

            // Update module info
            auto availableIt = m_availableModules.find(name);
            if (availableIt != m_availableModules.end()) {
                availableIt->second.isLoaded = false;
                availableIt->second.isEnabled = false;
            }

            LOG_INFO("Successfully unloaded module: " + name);
            return ModuleLoadResult::Success;
        }
        catch (const std::exception& e) {
            m_lastError = "Exception while unloading module '" + name + "': " + e.what();
            return ModuleLoadResult::UnknownError;
        }
    }

    ModuleLoadResult DynamicModuleLoader::ReloadModule(const std::string& name, const ModuleConfig& config) {
        ClearLastError();

        // Store the current enabled state
        bool wasEnabled = IsModuleEnabled(name);
        
        // Unload the module
        ModuleLoadResult unloadResult = UnloadModule(name);
        if (unloadResult != ModuleLoadResult::Success && unloadResult != ModuleLoadResult::FileNotFound) {
            return unloadResult;
        }

        // Load the module again
        ModuleLoadResult loadResult = LoadModule(name, config);
        if (loadResult != ModuleLoadResult::Success) {
            return loadResult;
        }

        // Restore enabled state
        if (!wasEnabled) {
            DisableModule(name);
        }

        LOG_INFO("Successfully reloaded module: " + name);
        return ModuleLoadResult::Success;
    }

    bool DynamicModuleLoader::EnableModule(const std::string& name) {
        auto it = m_loadedModules.find(name);
        if (it == m_loadedModules.end()) {
            m_lastError = "Module '" + name + "' is not loaded";
            return false;
        }

        it->second->SetEnabled(true);
        
        // Update module info
        auto availableIt = m_availableModules.find(name);
        if (availableIt != m_availableModules.end()) {
            availableIt->second.isEnabled = true;
        }

        LOG_INFO("Enabled module: " + name);
        return true;
    }

    bool DynamicModuleLoader::DisableModule(const std::string& name) {
        auto it = m_loadedModules.find(name);
        if (it == m_loadedModules.end()) {
            m_lastError = "Module '" + name + "' is not loaded";
            return false;
        }

        it->second->SetEnabled(false);
        
        // Update module info
        auto availableIt = m_availableModules.find(name);
        if (availableIt != m_availableModules.end()) {
            availableIt->second.isEnabled = false;
        }

        LOG_INFO("Disabled module: " + name);
        return true;
    }

    bool DynamicModuleLoader::IsModuleLoaded(const std::string& name) const {
        return m_loadedModules.find(name) != m_loadedModules.end();
    }

    bool DynamicModuleLoader::IsModuleEnabled(const std::string& name) const {
        auto it = m_loadedModules.find(name);
        if (it != m_loadedModules.end()) {
            return it->second->IsEnabled();
        }
        return false;
    }

    bool DynamicModuleLoader::SupportsHotSwap(const std::string& name) const {
        // For now, all modules support hot-swap except core modules
        auto it = m_availableModules.find(name);
        if (it != m_availableModules.end()) {
            return it->second.type != ModuleType::Core;
        }
        return false;
    }

    ModuleLoadResult DynamicModuleLoader::HotSwapModule(const std::string& name, const std::string& newPath) {
        if (!SupportsHotSwap(name)) {
            m_lastError = "Module '" + name + "' does not support hot-swapping";
            return ModuleLoadResult::InvalidModule;
        }

        // Store current configuration
        ModuleConfig currentConfig;
        auto it = m_loadedModules.find(name);
        if (it != m_loadedModules.end()) {
            currentConfig.name = name;
            currentConfig.version = it->second->GetVersion();
            currentConfig.enabled = it->second->IsEnabled();
        }

        // Update module path
        auto availableIt = m_availableModules.find(name);
        if (availableIt != m_availableModules.end()) {
            availableIt->second.path = newPath;
        }

        // Reload the module
        return ReloadModule(name, currentConfig);
    }

    void DynamicModuleLoader::EnableHotSwapWatching(bool enabled) {
        if (enabled && !m_fileWatcherActive) {
            StartFileWatcher();
        } else if (!enabled && m_fileWatcherActive) {
            StopFileWatcher();
        }
        m_hotSwapEnabled = enabled;
    }

    bool DynamicModuleLoader::IsHotSwapWatchingEnabled() const {
        return m_hotSwapEnabled;
    }

    ModuleLoadInfo DynamicModuleLoader::GetModuleInfo(const std::string& name) const {
        auto it = m_availableModules.find(name);
        if (it != m_availableModules.end()) {
            ModuleLoadInfo info = it->second;
            info.isLoaded = IsModuleLoaded(name);
            info.isEnabled = IsModuleEnabled(name);
            return info;
        }
        return ModuleLoadInfo{};
    }

    std::string DynamicModuleLoader::GetModuleLoadResultString(ModuleLoadResult result) const {
        switch (result) {
            case ModuleLoadResult::Success:
                return "Success";
            case ModuleLoadResult::FileNotFound:
                return "File not found";
            case ModuleLoadResult::InvalidModule:
                return "Invalid module";
            case ModuleLoadResult::AlreadyLoaded:
                return "Already loaded";
            case ModuleLoadResult::DependencyMissing:
                return "Dependency missing";
            case ModuleLoadResult::InitializationFailed:
                return "Initialization failed";
            case ModuleLoadResult::UnknownError:
                return "Unknown error";
            default:
                return "Unknown result";
        }
    }

    std::string DynamicModuleLoader::GetLastError() const {
        return m_lastError;
    }

    void DynamicModuleLoader::ClearLastError() {
        m_lastError.clear();
    }

    void DynamicModuleLoader::RegisterModuleFactory(const std::string& name, ModuleFactory factory) {
        m_moduleFactories[name] = factory;
        LOG_INFO("Registered module factory: " + name);
    }

    void DynamicModuleLoader::UnregisterModuleFactory(const std::string& name) {
        m_moduleFactories.erase(name);
        LOG_INFO("Unregistered module factory: " + name);
    }

    std::unique_ptr<IEngineModule> DynamicModuleLoader::CreateModuleInstance(const std::string& name) {
        // Try built-in factory first
        auto factoryIt = m_moduleFactories.find(name);
        if (factoryIt != m_moduleFactories.end()) {
            return factoryIt->second();
        }

        // For external modules, we would load from DLL/SO files here
        // This is a placeholder for future dynamic library loading
        LOG_WARNING("External module loading not yet implemented: " + name);
        return nullptr;
    }

    bool DynamicModuleLoader::ValidateModule(IEngineModule* module, const std::string& expectedName) {
        if (!module) {
            return false;
        }

        // Check that module name matches expected name
        if (std::string(module->GetName()) != expectedName) {
            LOG_ERROR("Module name mismatch: expected '" + expectedName + "', got '" + module->GetName() + "'");
            return false;
        }

        // Check that module has valid version
        std::string version = module->GetVersion();
        if (version.empty()) {
            LOG_ERROR("Module '" + expectedName + "' has empty version");
            return false;
        }

        return true;
    }

    std::vector<std::string> DynamicModuleLoader::GetDefaultSearchPaths() const {
        std::vector<std::string> paths;
        
        // Add standard module search paths
        paths.push_back("engine/modules");
        paths.push_back("modules");
        paths.push_back("plugins");
        
        // Add current directory
        paths.push_back(".");
        
        return paths;
    }

    bool DynamicModuleLoader::FileExists(const std::string& path) const {
        return std::filesystem::exists(path);
    }

    std::vector<std::string> DynamicModuleLoader::FindModuleFiles(const std::string& searchPath) const {
        std::vector<std::string> moduleFiles;
        
        try {
            if (!std::filesystem::exists(searchPath)) {
                return moduleFiles;
            }

            // Look for module descriptor files (*.module.json)
            for (const auto& entry : std::filesystem::recursive_directory_iterator(searchPath)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.ends_with(".module.json")) {
                        moduleFiles.push_back(entry.path().string());
                    }
                }
            }
        }
        catch (const std::exception& e) {
            LOG_WARNING("Error searching for modules in '" + searchPath + "': " + e.what());
        }
        
        return moduleFiles;
    }

    ModuleLoadInfo DynamicModuleLoader::ParseModuleInfo(const std::string& filePath) const {
        ModuleLoadInfo info;
        
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return info;
            }

            // For now, return empty info since we don't have external modules yet
            // In the future, this would parse JSON module descriptor files
            LOG_INFO("Found module descriptor: " + filePath + " (parsing not yet implemented)");
        }
        catch (const std::exception& e) {
            LOG_WARNING("Error parsing module info from '" + filePath + "': " + e.what());
        }
        
        return info;
    }

    void DynamicModuleLoader::StartFileWatcher() {
        // File watching implementation would go here
        // For now, just mark as active
        m_fileWatcherActive = true;
        LOG_INFO("File watcher started for hot-swap support");
    }

    void DynamicModuleLoader::StopFileWatcher() {
        // File watching cleanup would go here
        m_fileWatcherActive = false;
        LOG_INFO("File watcher stopped");
    }

    void DynamicModuleLoader::OnFileChanged(const std::string& filePath) {
        // Handle file change events for hot-swapping
        LOG_INFO("File changed: " + filePath + " (hot-swap handling not yet implemented)");
    }
}