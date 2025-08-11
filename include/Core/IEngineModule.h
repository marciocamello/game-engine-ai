#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace GameEngine {
    
    enum class ModuleType {
        Core,
        Graphics,
        Physics,
        Audio,
        Input,
        Scripting,
        Resource,
        Network
    };

    struct ModuleConfig {
        std::string name;
        std::string version;
        bool enabled = true;
        std::unordered_map<std::string, std::string> parameters;
    };

    class IEngineModule {
    public:
        virtual ~IEngineModule() = default;

        // Module lifecycle
        virtual bool Initialize(const ModuleConfig& config) = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Shutdown() = 0;

        // Module information
        virtual const char* GetName() const = 0;
        virtual const char* GetVersion() const = 0;
        virtual ModuleType GetType() const = 0;
        virtual std::vector<std::string> GetDependencies() const = 0;

        // Module state
        virtual bool IsInitialized() const = 0;
        virtual bool IsEnabled() const = 0;
        virtual void SetEnabled(bool enabled) = 0;
    };

    struct EngineConfig {
        std::vector<ModuleConfig> modules;
        std::string configVersion;
        std::string engineVersion;
    };
}