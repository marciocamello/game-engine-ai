#include "Scripting/ScriptingEngine.h"
#include "../../engine/core/Logger.h"

namespace GameEngine {
    ScriptingEngine::ScriptingEngine() {
    }

    ScriptingEngine::~ScriptingEngine() {
        Shutdown();
    }

    bool ScriptingEngine::Initialize() {
        ExposeEngineAPI();
        LOG_INFO("Scripting Engine initialized");
        return true;
    }

    void ScriptingEngine::Shutdown() {
        m_scripts.clear();
        m_registeredFunctions.clear();
        LOG_INFO("Scripting Engine shutdown");
    }

    void ScriptingEngine::Update(float deltaTime) {
        for (auto& [name, script] : m_scripts) {
            if (script) {
                script->Update(deltaTime);
            }
        }
    }

    std::shared_ptr<Script> ScriptingEngine::LoadScript(const std::string& name, const std::string& filepath) {
        auto script = std::make_shared<LuaScript>(name);
        if (script->Load(filepath)) {
            m_scripts[name] = script;
            LOG_INFO("Script loaded: " + name);
            return script;
        }
        LOG_ERROR("Failed to load script: " + name);
        return nullptr;
    }

    void ScriptingEngine::UnloadScript(const std::string& name) {
        m_scripts.erase(name);
        LOG_INFO("Script unloaded: " + name);
    }

    std::shared_ptr<Script> ScriptingEngine::GetScript(const std::string& name) {
        auto it = m_scripts.find(name);
        return it != m_scripts.end() ? it->second : nullptr;
    }

    bool ScriptingEngine::ExecuteScript(const std::string& name) {
        auto script = GetScript(name);
        if (script) {
            return script->Execute();
        }
        return false;
    }

    void ScriptingEngine::RegisterFunction(const std::string& name, std::function<void()> func) {
        m_registeredFunctions[name] = func;
    }

    void ScriptingEngine::ExposeEngineAPI() {
        // Register common engine functions that scripts can call
        RegisterFunction("log_info", []() {
            LOG_INFO("Called from script");
        });
    }

    // LuaScript implementation (basic stub - would need Lua library integration)
    LuaScript::LuaScript(const std::string& name) : Script(name) {
        // Initialize Lua state (would require lua library)
        // m_luaState = luaL_newstate();
    }

    LuaScript::~LuaScript() {
        // Clean up Lua state
        // if (m_luaState) lua_close(static_cast<lua_State*>(m_luaState));
    }

    bool LuaScript::Load(const std::string& filepath) {
        // Load and compile Lua script
        // Implementation would use luaL_loadfile and lua_pcall
        LOG_INFO("Loading Lua script: " + filepath);
        return true; // Placeholder
    }

    bool LuaScript::Execute() {
        // Execute the loaded script
        // Implementation would use lua_pcall
        return true; // Placeholder
    }

    void LuaScript::Update(float deltaTime) {
        // Call script's update function if it exists
        // CallFunction("update");
    }

    bool LuaScript::CallFunction(const std::string& functionName) {
        // Call a specific Lua function
        return true; // Placeholder
    }

    void LuaScript::SetGlobal(const std::string& name, float value) {
        // Set a global variable in Lua
    }

    void LuaScript::SetGlobal(const std::string& name, const std::string& value) {
        // Set a global string variable in Lua
    }

    float LuaScript::GetGlobalFloat(const std::string& name) {
        // Get a global float from Lua
        return 0.0f;
    }

    std::string LuaScript::GetGlobalString(const std::string& name) {
        // Get a global string from Lua
        return "";
    }
}