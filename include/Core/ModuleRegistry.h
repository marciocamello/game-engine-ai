#pragma once

#include "IEngineModule.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace GameEngine {

    class ModuleRegistry {
    public:
        static ModuleRegistry& GetInstance();

        // Module registration
        void RegisterModule(std::unique_ptr<IEngineModule> module);
        void UnregisterModule(const std::string& name);

        // Module access
        IEngineModule* GetModule(const std::string& name);
        std::vector<IEngineModule*> GetModulesByType(ModuleType type);
        std::vector<IEngineModule*> GetAllModules();

        // Module lifecycle management
        bool InitializeModules(const EngineConfig& config);
        void UpdateModules(float deltaTime);
        void ShutdownModules();

        // Dependency resolution
        std::vector<IEngineModule*> ResolveDependencies();
        bool ValidateDependencies();

        // Module state queries
        bool IsModuleRegistered(const std::string& name) const;
        size_t GetModuleCount() const;
        std::vector<std::string> GetModuleNames() const;

    private:
        ModuleRegistry() = default;
        ~ModuleRegistry() = default;
        ModuleRegistry(const ModuleRegistry&) = delete;
        ModuleRegistry& operator=(const ModuleRegistry&) = delete;

        // Internal dependency resolution helpers
        bool HasCircularDependency(const std::string& moduleName, 
                                   std::vector<std::string>& visitedModules) const;
        std::vector<IEngineModule*> TopologicalSort();

        std::unordered_map<std::string, std::unique_ptr<IEngineModule>> m_modules;
        std::vector<IEngineModule*> m_initializationOrder;
        bool m_dependenciesResolved = false;
    };
}