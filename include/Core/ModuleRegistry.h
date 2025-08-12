#pragma once

#include "IEngineModule.h"
#include "ModuleError.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

namespace GameEngine {

    struct ModuleInitializationResult {
        bool success = false;
        ModuleErrorCollector errors;
        std::vector<std::string> initializedModules;
        std::vector<std::string> skippedModules;
        std::vector<std::string> fallbackModules;
        
        bool HasCriticalErrors() const { return errors.HasCriticalErrors(); }
        std::string GetSummary() const;
    };

    using ModuleFallbackProvider = std::function<std::unique_ptr<IEngineModule>(const std::string&, ModuleType)>;

    class ModuleRegistry {
    public:
        static ModuleRegistry& GetInstance();

        // Module registration
        bool RegisterModule(std::unique_ptr<IEngineModule> module, ModuleErrorCollector* errorCollector = nullptr);
        bool UnregisterModule(const std::string& name, ModuleErrorCollector* errorCollector = nullptr);

        // Module access
        IEngineModule* GetModule(const std::string& name);
        std::vector<IEngineModule*> GetModulesByType(ModuleType type);
        std::vector<IEngineModule*> GetAllModules();

        // Module lifecycle management with enhanced error handling
        ModuleInitializationResult InitializeModules(const EngineConfig& config);
        void UpdateModules(float deltaTime);
        void ShutdownModules();

        // Dependency resolution with error reporting
        std::vector<IEngineModule*> ResolveDependencies(ModuleErrorCollector* errorCollector = nullptr);
        bool ValidateDependencies(ModuleErrorCollector* errorCollector = nullptr);

        // Configuration validation
        ConfigurationValidator::ValidationContext ValidateConfiguration(const EngineConfig& config);

        // Fallback mechanism
        void SetFallbackProvider(ModuleFallbackProvider provider) { m_fallbackProvider = provider; }
        bool EnableGracefulFallbacks(bool enable) { 
            bool old = m_gracefulFallbacks; 
            m_gracefulFallbacks = enable; 
            return old; 
        }

        // Module state queries
        bool IsModuleRegistered(const std::string& name) const;
        size_t GetModuleCount() const;
        std::vector<std::string> GetModuleNames() const;
        std::vector<std::string> GetMissingDependencies() const;

        // Error recovery
        bool AttemptModuleRecovery(const std::string& moduleName, ModuleErrorCollector* errorCollector = nullptr);
        void ClearErrorState();

    private:
        ModuleRegistry() = default;
        ~ModuleRegistry() = default;
        ModuleRegistry(const ModuleRegistry&) = delete;
        ModuleRegistry& operator=(const ModuleRegistry&) = delete;

        // Internal dependency resolution helpers
        bool HasCircularDependency(const std::string& moduleName, 
                                   std::vector<std::string>& visitedModules,
                                   ModuleErrorCollector* errorCollector = nullptr) const;
        std::vector<IEngineModule*> TopologicalSort(ModuleErrorCollector* errorCollector = nullptr);

        // Fallback and recovery mechanisms
        std::unique_ptr<IEngineModule> CreateFallbackModule(const std::string& moduleName, ModuleType type);
        bool TryLoadAlternativeModule(const std::string& originalName, ModuleType type, 
                                     ModuleErrorCollector* errorCollector = nullptr);

        // Configuration helpers
        ModuleConfig GetDefaultModuleConfig(const std::string& moduleName) const;
        bool ValidateModuleCompatibility(IEngineModule* module, const ModuleConfig& config,
                                        ModuleErrorCollector* errorCollector = nullptr);

        std::unordered_map<std::string, std::unique_ptr<IEngineModule>> m_modules;
        std::vector<IEngineModule*> m_initializationOrder;
        bool m_dependenciesResolved = false;
        bool m_gracefulFallbacks = true;
        ModuleFallbackProvider m_fallbackProvider;
        
        // Error tracking
        mutable ModuleErrorCollector m_lastErrors;
    };
}