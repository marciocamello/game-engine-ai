#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace GameEngine {
    class Script {
    public:
        Script(const std::string& name) : m_name(name) {}
        virtual ~Script() = default;

        virtual bool Load(const std::string& filepath) = 0;
        virtual bool Execute() = 0;
        virtual void Update(float deltaTime) {}

        const std::string& GetName() const { return m_name; }

    protected:
        std::string m_name;
    };

    class LuaScript : public Script {
    public:
        LuaScript(const std::string& name);
        ~LuaScript() override;

        bool Load(const std::string& filepath) override;
        bool Execute() override;
        void Update(float deltaTime) override;

        // Lua-specific functions
        bool CallFunction(const std::string& functionName);
        void SetGlobal(const std::string& name, float value);
        void SetGlobal(const std::string& name, const std::string& value);
        float GetGlobalFloat(const std::string& name);
        std::string GetGlobalString(const std::string& name);

    private:
        void* m_luaState = nullptr; // lua_State*
    };

    class ScriptingEngine {
    public:
        ScriptingEngine();
        ~ScriptingEngine();

        bool Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // Script management
        std::shared_ptr<Script> LoadScript(const std::string& name, const std::string& filepath);
        void UnloadScript(const std::string& name);
        std::shared_ptr<Script> GetScript(const std::string& name);
        bool ExecuteScript(const std::string& name);

        // Global functions registration
        void RegisterFunction(const std::string& name, std::function<void()> func);
        void RegisterFunction(const std::string& name, std::function<float(float)> func);
        void RegisterFunction(const std::string& name, std::function<void(const std::string&)> func);

        // Engine API exposure
        void ExposeEngineAPI();

    private:
        std::unordered_map<std::string, std::shared_ptr<Script>> m_scripts;
        std::unordered_map<std::string, std::function<void()>> m_registeredFunctions;
    };
}