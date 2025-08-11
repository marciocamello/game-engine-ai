#include "Core/ModuleRegistry.h"
#include "Core/Logger.h"
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <sstream>

namespace GameEngine {

    ModuleRegistry& ModuleRegistry::GetInstance() {
        static ModuleRegistry instance;
        return instance;
    }

    void ModuleRegistry::RegisterModule(std::unique_ptr<IEngineModule> module) {
        if (!module) {
            LOG_ERROR("Attempted to register null module");
            return;
        }

        std::string moduleName = module->GetName();
        if (IsModuleRegistered(moduleName)) {
            LOG_WARNING("Module '" + moduleName + "' is already registered, replacing existing module");
        }

        LOG_INFO("Registering module: " + moduleName);
        m_modules[moduleName] = std::move(module);
        m_dependenciesResolved = false; // Need to re-resolve dependencies
    }

    void ModuleRegistry::UnregisterModule(const std::string& name) {
        auto it = m_modules.find(name);
        if (it != m_modules.end()) {
            LOG_INFO("Unregistering module: " + name);
            
            // Shutdown the module if it's initialized
            if (it->second->IsInitialized()) {
                it->second->Shutdown();
            }
            
            m_modules.erase(it);
            m_dependenciesResolved = false; // Need to re-resolve dependencies
            
            // Remove from initialization order
            m_initializationOrder.erase(
                std::remove_if(m_initializationOrder.begin(), m_initializationOrder.end(),
                    [&name](const IEngineModule* module) {
                        return module->GetName() == name;
                    }),
                m_initializationOrder.end()
            );
        } else {
            LOG_WARNING("Attempted to unregister non-existent module: " + name);
        }
    }

    IEngineModule* ModuleRegistry::GetModule(const std::string& name) {
        auto it = m_modules.find(name);
        return (it != m_modules.end()) ? it->second.get() : nullptr;
    }

    std::vector<IEngineModule*> ModuleRegistry::GetModulesByType(ModuleType type) {
        std::vector<IEngineModule*> result;
        for (const auto& pair : m_modules) {
            if (pair.second->GetType() == type) {
                result.push_back(pair.second.get());
            }
        }
        return result;
    }

    std::vector<IEngineModule*> ModuleRegistry::GetAllModules() {
        std::vector<IEngineModule*> result;
        for (const auto& pair : m_modules) {
            result.push_back(pair.second.get());
        }
        return result;
    }

    bool ModuleRegistry::InitializeModules(const EngineConfig& config) {
        LOG_INFO("Initializing modules...");

        // First, validate dependencies
        if (!ValidateDependencies()) {
            LOG_ERROR("Module dependency validation failed");
            return false;
        }

        // Resolve initialization order
        m_initializationOrder = ResolveDependencies();
        if (m_initializationOrder.empty() && !m_modules.empty()) {
            LOG_ERROR("Failed to resolve module dependencies");
            return false;
        }

        // Create a map of module configurations for quick lookup
        std::unordered_map<std::string, ModuleConfig> configMap;
        for (const auto& moduleConfig : config.modules) {
            configMap[moduleConfig.name] = moduleConfig;
        }

        // Initialize modules in dependency order
        for (IEngineModule* module : m_initializationOrder) {
            // Find configuration for this module
            ModuleConfig moduleConfig;
            auto configIt = configMap.find(module->GetName());
            if (configIt != configMap.end()) {
                moduleConfig = configIt->second;
            } else {
                // Use default configuration
                moduleConfig.name = module->GetName();
                moduleConfig.version = module->GetVersion();
                moduleConfig.enabled = true;
            }

            // Set module enabled state based on configuration
            module->SetEnabled(moduleConfig.enabled);

            if (!module->IsEnabled()) {
                LOG_INFO("Skipping disabled module: " + std::string(module->GetName()));
                continue;
            }

            LOG_INFO("Initializing module: " + std::string(module->GetName()));
            if (!module->Initialize(moduleConfig)) {
                LOG_ERROR("Failed to initialize module: " + std::string(module->GetName()));
                return false;
            }
        }

        LOG_INFO("All modules initialized successfully");
        return true;
    }

    void ModuleRegistry::UpdateModules(float deltaTime) {
        for (IEngineModule* module : m_initializationOrder) {
            if (module->IsInitialized() && module->IsEnabled()) {
                module->Update(deltaTime);
            }
        }
    }

    void ModuleRegistry::ShutdownModules() {
        LOG_INFO("Shutting down modules...");

        // Shutdown in reverse order of initialization
        for (auto it = m_initializationOrder.rbegin(); it != m_initializationOrder.rend(); ++it) {
            IEngineModule* module = *it;
            if (module->IsInitialized()) {
                LOG_INFO("Shutting down module: " + std::string(module->GetName()));
                module->Shutdown();
            }
        }

        m_initializationOrder.clear();
        LOG_INFO("All modules shut down");
    }

    std::vector<IEngineModule*> ModuleRegistry::ResolveDependencies() {
        if (m_dependenciesResolved && !m_initializationOrder.empty()) {
            return m_initializationOrder;
        }

        std::vector<IEngineModule*> result = TopologicalSort();
        if (!result.empty()) {
            m_dependenciesResolved = true;
        }

        return result;
    }

    bool ModuleRegistry::ValidateDependencies() {
        // Check for circular dependencies
        for (const auto& pair : m_modules) {
            std::vector<std::string> visitedModules;
            if (HasCircularDependency(pair.first, visitedModules)) {
                LOG_ERROR("Circular dependency detected involving module: " + pair.first);
                return false;
            }
        }

        // Check that all dependencies exist
        for (const auto& pair : m_modules) {
            const std::vector<std::string>& dependencies = pair.second->GetDependencies();
            for (const std::string& dependency : dependencies) {
                if (!IsModuleRegistered(dependency)) {
                    LOG_ERROR("Module '" + pair.first + "' depends on non-existent module '" + dependency + "'");
                    return false;
                }
            }
        }

        return true;
    }

    bool ModuleRegistry::IsModuleRegistered(const std::string& name) const {
        return m_modules.find(name) != m_modules.end();
    }

    size_t ModuleRegistry::GetModuleCount() const {
        return m_modules.size();
    }

    std::vector<std::string> ModuleRegistry::GetModuleNames() const {
        std::vector<std::string> names;
        for (const auto& pair : m_modules) {
            names.push_back(pair.first);
        }
        return names;
    }

    bool ModuleRegistry::HasCircularDependency(const std::string& moduleName, 
                                               std::vector<std::string>& visitedModules) const {
        // Check if we've already visited this module in the current path
        if (std::find(visitedModules.begin(), visitedModules.end(), moduleName) != visitedModules.end()) {
            return true; // Circular dependency found
        }

        auto it = m_modules.find(moduleName);
        if (it == m_modules.end()) {
            return false; // Module doesn't exist, not our concern here
        }

        visitedModules.push_back(moduleName);

        // Check all dependencies of this module
        const std::vector<std::string>& dependencies = it->second->GetDependencies();
        for (const std::string& dependency : dependencies) {
            if (HasCircularDependency(dependency, visitedModules)) {
                return true;
            }
        }

        visitedModules.pop_back();
        return false;
    }

    std::vector<IEngineModule*> ModuleRegistry::TopologicalSort() {
        std::vector<IEngineModule*> result;
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> visiting;

        std::function<bool(const std::string&)> visit = [&](const std::string& moduleName) -> bool {
            if (visiting.find(moduleName) != visiting.end()) {
                // Circular dependency detected
                return false;
            }
            if (visited.find(moduleName) != visited.end()) {
                // Already processed
                return true;
            }

            auto it = m_modules.find(moduleName);
            if (it == m_modules.end()) {
                // Module doesn't exist
                return false;
            }

            visiting.insert(moduleName);

            // Visit all dependencies first
            const std::vector<std::string>& dependencies = it->second->GetDependencies();
            for (const std::string& dependency : dependencies) {
                if (!visit(dependency)) {
                    return false;
                }
            }

            visiting.erase(moduleName);
            visited.insert(moduleName);
            result.push_back(it->second.get());

            return true;
        };

        // Visit all modules
        for (const auto& pair : m_modules) {
            if (!visit(pair.first)) {
                LOG_ERROR("Failed to resolve dependencies for module: " + pair.first);
                return {}; // Return empty vector on failure
            }
        }

        return result;
    }
}