#include "Core/ModuleRegistry.h"
#include "Core/Logger.h"
#include <algorithm>
#include <unordered_set>
#include <functional>
#include <sstream>
#include <map>
#include <set>

namespace GameEngine {

    ModuleRegistry& ModuleRegistry::GetInstance() {
        static ModuleRegistry instance;
        return instance;
    }

    std::string ModuleInitializationResult::GetSummary() const {
        std::ostringstream oss;
        oss << "Module Initialization Summary:\n";
        oss << "  - Success: " << (success ? "Yes" : "No") << "\n";
        oss << "  - Initialized: " << initializedModules.size() << " modules\n";
        oss << "  - Skipped: " << skippedModules.size() << " modules\n";
        oss << "  - Fallbacks: " << fallbackModules.size() << " modules\n";
        oss << "  - Errors: " << errors.GetErrorCount() << "\n";
        
        if (!initializedModules.empty()) {
            oss << "  - Initialized modules: ";
            for (size_t i = 0; i < initializedModules.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << initializedModules[i];
            }
            oss << "\n";
        }
        
        if (!skippedModules.empty()) {
            oss << "  - Skipped modules: ";
            for (size_t i = 0; i < skippedModules.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << skippedModules[i];
            }
            oss << "\n";
        }
        
        if (!fallbackModules.empty()) {
            oss << "  - Fallback modules: ";
            for (size_t i = 0; i < fallbackModules.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << fallbackModules[i];
            }
            oss << "\n";
        }
        
        if (errors.HasErrors()) {
            oss << "\n" << errors.GetSummary();
        }
        
        return oss.str();
    }

    bool ModuleRegistry::RegisterModule(std::unique_ptr<IEngineModule> module, ModuleErrorCollector* errorCollector) {
        if (!module) {
            ModuleError error(ModuleErrorType::LoadingFailed, "", 
                            "Attempted to register null module", 
                            "Ensure module is properly constructed before registration");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            LOG_ERROR(error.GetFormattedMessage());
            return false;
        }

        std::string moduleName = module->GetName();
        
        // Validate module name
        if (moduleName.empty()) {
            ModuleError error(ModuleErrorType::ValidationFailed, moduleName, 
                            "Module name cannot be empty", 
                            "Implement GetName() method to return a valid module name");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            LOG_ERROR(error.GetFormattedMessage());
            return false;
        }

        // Check for existing module
        if (IsModuleRegistered(moduleName)) {
            ModuleError error(ModuleErrorType::LoadingFailed, moduleName, 
                            "Module is already registered", 
                            "Unregister existing module first or use a different name");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            LOG_WARNING(error.GetFormattedMessage());
            // Continue with replacement for now, but log as warning
        }

        // Validate module dependencies
        auto dependencies = module->GetDependencies();
        auto depValidation = ConfigurationValidator::ValidateModuleDependencies(dependencies);
        if (!depValidation.IsValid()) {
            ModuleError error(ModuleErrorType::ValidationFailed, moduleName, 
                            "Module dependencies are invalid", 
                            depValidation.GetSummary());
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            LOG_ERROR(error.GetFormattedMessage());
            
            if (depValidation.hasCriticalErrors) {
                return false;
            }
        }

        LOG_INFO("Registering module: " + moduleName);
        m_modules[moduleName] = std::move(module);
        m_dependenciesResolved = false; // Need to re-resolve dependencies
        return true;
    }

    bool ModuleRegistry::UnregisterModule(const std::string& name, ModuleErrorCollector* errorCollector) {
        auto it = m_modules.find(name);
        if (it != m_modules.end()) {
            LOG_INFO("Unregistering module: " + name);
            
            // Check if other modules depend on this one
            std::vector<std::string> dependentModules;
            for (const auto& pair : m_modules) {
                if (pair.first != name) {
                    auto deps = pair.second->GetDependencies();
                    if (std::find(deps.begin(), deps.end(), name) != deps.end()) {
                        dependentModules.push_back(pair.first);
                    }
                }
            }
            
            if (!dependentModules.empty()) {
                ModuleError error(ModuleErrorType::DependencyMissing, name, 
                                "Cannot unregister module with active dependents", 
                                "Other modules depend on this module");
                error.affectedModules = dependentModules;
                
                if (errorCollector) {
                    errorCollector->AddError(error);
                } else {
                    m_lastErrors.AddError(error);
                }
                LOG_WARNING(error.GetFormattedMessage());
                
                if (!m_gracefulFallbacks) {
                    return false;
                }
            }
            
            // Shutdown the module if it's initialized
            if (it->second->IsInitialized()) {
                try {
                    it->second->Shutdown();
                } catch (const std::exception& e) {
                    ModuleError error(ModuleErrorType::RuntimeError, name, 
                                    "Exception during module shutdown", 
                                    std::string("Exception: ") + e.what());
                    if (errorCollector) {
                        errorCollector->AddError(error);
                    } else {
                        m_lastErrors.AddError(error);
                    }
                    LOG_ERROR(error.GetFormattedMessage());
                }
            }
            
            m_modules.erase(it);
            m_dependenciesResolved = false; // Need to re-resolve dependencies
            
            // Remove from initialization order
            m_initializationOrder.erase(
                std::remove_if(m_initializationOrder.begin(), m_initializationOrder.end(),
                    [&name](const IEngineModule* module) {
                        return module && module->GetName() == name;
                    }),
                m_initializationOrder.end()
            );
            
            return true;
        } else {
            ModuleError error(ModuleErrorType::ModuleNotFound, name, 
                            "Attempted to unregister non-existent module", 
                            "Check if module name is correct and module is registered");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            LOG_WARNING(error.GetFormattedMessage());
            return false;
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

    ModuleInitializationResult ModuleRegistry::InitializeModules(const EngineConfig& config) {
        LOG_INFO("Initializing modules...");
        
        ModuleInitializationResult result;
        
        // Validate configuration first
        auto configValidation = ValidateConfiguration(config);
        if (!configValidation.IsValid()) {
            result.errors.AddError(ModuleErrorType::ConfigurationInvalid, "", 
                                 "Engine configuration validation failed", 
                                 configValidation.GetSummary());
            
            if (configValidation.hasCriticalErrors) {
                LOG_ERROR("Critical configuration errors detected, aborting initialization");
                result.success = false;
                return result;
            }
        }

        // Validate dependencies
        if (!ValidateDependencies(&result.errors)) {
            LOG_ERROR("Module dependency validation failed");
            if (result.errors.HasCriticalErrors()) {
                result.success = false;
                return result;
            }
        }

        // Resolve initialization order
        m_initializationOrder = ResolveDependencies(&result.errors);
        if (m_initializationOrder.empty() && !m_modules.empty()) {
            result.errors.AddError(ModuleErrorType::DependencyMissing, "", 
                                 "Failed to resolve module dependencies", 
                                 "Check for circular dependencies or missing modules");
            result.success = false;
            return result;
        }

        // Create a map of module configurations for quick lookup
        std::unordered_map<std::string, ModuleConfig> configMap;
        for (const auto& moduleConfig : config.modules) {
            configMap[moduleConfig.name] = moduleConfig;
        }

        // Initialize modules in dependency order
        bool allSuccessful = true;
        for (IEngineModule* module : m_initializationOrder) {
            std::string moduleName = module->GetName();
            
            // Find configuration for this module
            ModuleConfig moduleConfig;
            auto configIt = configMap.find(moduleName);
            if (configIt != configMap.end()) {
                moduleConfig = configIt->second;
                
                // Validate module-specific configuration
                auto moduleValidation = ConfigurationValidator::ValidateModuleConfig(moduleConfig);
                if (!moduleValidation.IsValid()) {
                    result.errors.AddError(ModuleErrorType::ConfigurationInvalid, moduleName, 
                                         "Module configuration validation failed", 
                                         moduleValidation.GetSummary());
                    
                    if (moduleValidation.hasCriticalErrors) {
                        result.skippedModules.push_back(moduleName);
                        allSuccessful = false;
                        continue;
                    }
                }
            } else {
                // Use default configuration
                moduleConfig = GetDefaultModuleConfig(moduleName);
                LOG_INFO("Using default configuration for module: " + moduleName);
            }

            // Validate module compatibility
            if (!ValidateModuleCompatibility(module, moduleConfig, &result.errors)) {
                if (m_gracefulFallbacks) {
                    // Try to create a fallback module
                    auto fallback = CreateFallbackModule(moduleName, module->GetType());
                    if (fallback) {
                        LOG_WARNING("Using fallback for module: " + moduleName);
                        result.fallbackModules.push_back(moduleName);
                        // Note: In a real implementation, we'd replace the module here
                    } else {
                        result.skippedModules.push_back(moduleName);
                        allSuccessful = false;
                        continue;
                    }
                } else {
                    result.skippedModules.push_back(moduleName);
                    allSuccessful = false;
                    continue;
                }
            }

            // Set module enabled state based on configuration
            module->SetEnabled(moduleConfig.enabled);

            if (!module->IsEnabled()) {
                LOG_INFO("Skipping disabled module: " + moduleName);
                result.skippedModules.push_back(moduleName);
                continue;
            }

            LOG_INFO("Initializing module: " + moduleName);
            
            try {
                if (!module->Initialize(moduleConfig)) {
                    result.errors.AddError(ModuleErrorType::InitializationFailed, moduleName, 
                                         "Module initialization returned false", 
                                         "Check module-specific logs for details");
                    
                    if (m_gracefulFallbacks) {
                        auto fallback = CreateFallbackModule(moduleName, module->GetType());
                        if (fallback) {
                            result.fallbackModules.push_back(moduleName);
                            LOG_WARNING("Using fallback for failed module: " + moduleName);
                        } else {
                            result.skippedModules.push_back(moduleName);
                            allSuccessful = false;
                            continue;
                        }
                    } else {
                        result.skippedModules.push_back(moduleName);
                        allSuccessful = false;
                        continue;
                    }
                } else {
                    result.initializedModules.push_back(moduleName);
                }
            } catch (const std::exception& e) {
                result.errors.AddError(ModuleErrorType::InitializationFailed, moduleName, 
                                     "Exception during module initialization", 
                                     std::string("Exception: ") + e.what());
                
                if (m_gracefulFallbacks) {
                    auto fallback = CreateFallbackModule(moduleName, module->GetType());
                    if (fallback) {
                        result.fallbackModules.push_back(moduleName);
                        LOG_WARNING("Using fallback after exception in module: " + moduleName);
                    } else {
                        result.skippedModules.push_back(moduleName);
                        allSuccessful = false;
                    }
                } else {
                    result.skippedModules.push_back(moduleName);
                    allSuccessful = false;
                }
            }
        }

        result.success = allSuccessful || (m_gracefulFallbacks && !result.errors.HasCriticalErrors());
        
        if (result.success) {
            LOG_INFO("Module initialization completed successfully");
        } else {
            LOG_ERROR("Module initialization completed with errors");
        }
        
        if (result.errors.HasErrors()) {
            result.errors.LogAllErrors();
        }
        
        LOG_INFO(result.GetSummary());
        return result;
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

    std::vector<IEngineModule*> ModuleRegistry::ResolveDependencies(ModuleErrorCollector* errorCollector) {
        if (m_dependenciesResolved && !m_initializationOrder.empty()) {
            return m_initializationOrder;
        }

        std::vector<IEngineModule*> result = TopologicalSort(errorCollector);
        if (!result.empty()) {
            m_dependenciesResolved = true;
        }

        return result;
    }

    bool ModuleRegistry::ValidateDependencies(ModuleErrorCollector* errorCollector) {
        bool isValid = true;
        
        // Check for circular dependencies
        for (const auto& pair : m_modules) {
            std::vector<std::string> visitedModules;
            if (HasCircularDependency(pair.first, visitedModules, errorCollector)) {
                isValid = false;
            }
        }

        // Check that all dependencies exist
        for (const auto& pair : m_modules) {
            const std::vector<std::string>& dependencies = pair.second->GetDependencies();
            for (const std::string& dependency : dependencies) {
                if (!IsModuleRegistered(dependency)) {
                    ModuleError error(ModuleErrorType::DependencyMissing, pair.first, 
                                    "Depends on non-existent module: " + dependency, 
                                    "Register the required module or remove the dependency");
                    if (errorCollector) {
                        errorCollector->AddError(error);
                    } else {
                        m_lastErrors.AddError(error);
                    }
                    LOG_ERROR(error.GetFormattedMessage());
                    isValid = false;
                }
            }
        }

        return isValid;
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
                                               std::vector<std::string>& visitedModules,
                                               ModuleErrorCollector* errorCollector) const {
        // Check if we've already visited this module in the current path
        if (std::find(visitedModules.begin(), visitedModules.end(), moduleName) != visitedModules.end()) {
            // Build the circular dependency chain
            std::ostringstream chain;
            bool foundStart = false;
            for (const auto& module : visitedModules) {
                if (module == moduleName) {
                    foundStart = true;
                }
                if (foundStart) {
                    if (chain.tellp() > 0) chain << " -> ";
                    chain << module;
                }
            }
            chain << " -> " << moduleName;
            
            ModuleError error(ModuleErrorType::CircularDependency, moduleName, 
                            "Circular dependency detected", 
                            "Dependency chain: " + chain.str());
            error.affectedModules = visitedModules;
            
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            LOG_ERROR(error.GetFormattedMessage());
            return true;
        }

        auto it = m_modules.find(moduleName);
        if (it == m_modules.end()) {
            return false; // Module doesn't exist, not our concern here
        }

        visitedModules.push_back(moduleName);

        // Check all dependencies of this module
        const std::vector<std::string>& dependencies = it->second->GetDependencies();
        for (const std::string& dependency : dependencies) {
            if (HasCircularDependency(dependency, visitedModules, errorCollector)) {
                return true;
            }
        }

        visitedModules.pop_back();
        return false;
    }

    std::vector<IEngineModule*> ModuleRegistry::TopologicalSort(ModuleErrorCollector* errorCollector) {
        std::vector<IEngineModule*> result;
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> visiting;

        std::function<bool(const std::string&)> visit = [&](const std::string& moduleName) -> bool {
            if (visiting.find(moduleName) != visiting.end()) {
                // Circular dependency detected - this should have been caught earlier
                ModuleError error(ModuleErrorType::CircularDependency, moduleName, 
                                "Circular dependency detected during topological sort", 
                                "This should have been caught during validation");
                if (errorCollector) {
                    errorCollector->AddError(error);
                } else {
                    m_lastErrors.AddError(error);
                }
                return false;
            }
            if (visited.find(moduleName) != visited.end()) {
                // Already processed
                return true;
            }

            auto it = m_modules.find(moduleName);
            if (it == m_modules.end()) {
                // Module doesn't exist
                ModuleError error(ModuleErrorType::ModuleNotFound, moduleName, 
                                "Module not found during dependency resolution", 
                                "Ensure all required modules are registered");
                if (errorCollector) {
                    errorCollector->AddError(error);
                } else {
                    m_lastErrors.AddError(error);
                }
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
                ModuleError error(ModuleErrorType::DependencyMissing, pair.first, 
                                "Failed to resolve dependencies", 
                                "Check module dependencies and registration order");
                if (errorCollector) {
                    errorCollector->AddError(error);
                } else {
                    m_lastErrors.AddError(error);
                }
                LOG_ERROR(error.GetFormattedMessage());
                return {}; // Return empty vector on failure
            }
        }

        return result;
    }

    ConfigurationValidator::ValidationContext ModuleRegistry::ValidateConfiguration(const EngineConfig& config) {
        return ConfigurationValidator::ValidateEngineConfig(config);
    }

    std::vector<std::string> ModuleRegistry::GetMissingDependencies() const {
        std::vector<std::string> missing;
        std::set<std::string> missingSet;
        
        for (const auto& pair : m_modules) {
            const std::vector<std::string>& dependencies = pair.second->GetDependencies();
            for (const std::string& dependency : dependencies) {
                if (!IsModuleRegistered(dependency) && missingSet.find(dependency) == missingSet.end()) {
                    missing.push_back(dependency);
                    missingSet.insert(dependency);
                }
            }
        }
        
        return missing;
    }

    bool ModuleRegistry::AttemptModuleRecovery(const std::string& moduleName, ModuleErrorCollector* errorCollector) {
        auto module = GetModule(moduleName);
        if (!module) {
            ModuleError error(ModuleErrorType::ModuleNotFound, moduleName, 
                            "Cannot recover non-existent module", 
                            "Register the module first");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            return false;
        }
        
        try {
            // Try to shutdown and reinitialize
            if (module->IsInitialized()) {
                module->Shutdown();
            }
            
            // Use default configuration for recovery
            ModuleConfig defaultConfig = GetDefaultModuleConfig(moduleName);
            
            if (module->Initialize(defaultConfig)) {
                LOG_INFO("Successfully recovered module: " + moduleName);
                return true;
            } else {
                ModuleError error(ModuleErrorType::InitializationFailed, moduleName, 
                                "Module recovery failed during initialization", 
                                "Module may have persistent issues");
                if (errorCollector) {
                    errorCollector->AddError(error);
                } else {
                    m_lastErrors.AddError(error);
                }
                return false;
            }
        } catch (const std::exception& e) {
            ModuleError error(ModuleErrorType::RuntimeError, moduleName, 
                            "Exception during module recovery", 
                            std::string("Exception: ") + e.what());
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            return false;
        }
    }

    void ModuleRegistry::ClearErrorState() {
        m_lastErrors.Clear();
    }

    std::unique_ptr<IEngineModule> ModuleRegistry::CreateFallbackModule(const std::string& moduleName, ModuleType type) {
        if (m_fallbackProvider) {
            try {
                return m_fallbackProvider(moduleName, type);
            } catch (const std::exception& e) {
                LOG_ERROR("Exception in fallback provider for module '" + moduleName + "': " + e.what());
            }
        }
        return nullptr;
    }

    bool ModuleRegistry::TryLoadAlternativeModule(const std::string& originalName, ModuleType type, 
                                                 ModuleErrorCollector* errorCollector) {
        // This is a placeholder for alternative module loading logic
        // In a real implementation, this might try different module implementations
        // or load from different sources
        
        LOG_INFO("Attempting to load alternative for module: " + originalName);
        
        // For now, just try the fallback provider
        auto fallback = CreateFallbackModule(originalName, type);
        if (fallback) {
            // In a real implementation, we'd replace the failed module with the fallback
            LOG_INFO("Alternative module created for: " + originalName);
            return true;
        }
        
        ModuleError error(ModuleErrorType::LoadingFailed, originalName, 
                        "No alternative module implementation available", 
                        "Consider implementing a fallback provider");
        if (errorCollector) {
            errorCollector->AddError(error);
        } else {
            m_lastErrors.AddError(error);
        }
        
        return false;
    }

    ModuleConfig ModuleRegistry::GetDefaultModuleConfig(const std::string& moduleName) const {
        auto it = m_modules.find(moduleName);
        ModuleConfig config;
        config.name = moduleName;
        config.enabled = true;
        
        if (it != m_modules.end()) {
            config.version = it->second->GetVersion();
        } else {
            config.version = "1.0.0";
        }
        
        return config;
    }

    bool ModuleRegistry::ValidateModuleCompatibility(IEngineModule* module, const ModuleConfig& config,
                                                    ModuleErrorCollector* errorCollector) {
        if (!module) {
            ModuleError error(ModuleErrorType::ValidationFailed, config.name, 
                            "Cannot validate compatibility of null module", 
                            "Ensure module is properly constructed");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            return false;
        }
        
        // Check name consistency
        if (module->GetName() != config.name) {
            ModuleError error(ModuleErrorType::ValidationFailed, config.name, 
                            "Module name mismatch", 
                            "Module reports name '" + std::string(module->GetName()) + 
                            "' but config specifies '" + config.name + "'");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            return false;
        }
        
        // Check version compatibility (basic check)
        if (!config.version.empty() && config.version != module->GetVersion()) {
            ModuleError error(ModuleErrorType::VersionMismatch, config.name, 
                            "Version mismatch", 
                            "Module version '" + std::string(module->GetVersion()) + 
                            "' does not match config version '" + config.version + "'");
            if (errorCollector) {
                errorCollector->AddError(error);
            } else {
                m_lastErrors.AddError(error);
            }
            // This is not necessarily a critical error, so we continue
        }
        
        return true;
    }
}