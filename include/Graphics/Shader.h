#pragma once

#include "Core/Math.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

namespace GameEngine {
    class Texture;
    class ShaderCompilationError;
    class ShaderStateManager;

    class Shader : public std::enable_shared_from_this<Shader> {
    public:
        enum class Type { Vertex, Fragment, Geometry, Compute, TessControl, TessEvaluation };
        enum class State { Uncompiled, Compiling, Compiled, Linked, Error };

        Shader();
        ~Shader();

        // Traditional shader loading (vertex + fragment)
        bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
        bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
        
        // Compute shader loading
        bool CompileFromSource(const std::string& source, Type type);
        bool CompileFromFile(const std::string& filepath, Type type);
        bool LinkProgram();
        
        void Use() const;
        void Unuse() const;
        
        // Enhanced uniform setters (with state management optimization)
        void SetUniform(const std::string& name, bool value);
        void SetUniform(const std::string& name, int value);
        void SetUniform(const std::string& name, float value);
        void SetUniform(const std::string& name, const Math::Vec2& value);
        void SetUniform(const std::string& name, const Math::Vec3& value);
        void SetUniform(const std::string& name, const Math::Vec4& value);
        void SetUniform(const std::string& name, const Math::Mat3& value);
        void SetUniform(const std::string& name, const Math::Mat4& value);
        void SetUniformArray(const std::string& name, const std::vector<Math::Mat4>& values);
        void SetUniformArray(const std::string& name, const std::vector<Math::Vec3>& values);
        void SetUniformArray(const std::string& name, const std::vector<Math::Vec4>& values);
        void SetUniformArray(const std::string& name, const std::vector<float>& values);
        void SetUniformArray(const std::string& name, const std::vector<int>& values);
        
        // Direct uniform setters (bypass state management for immediate updates)
        void SetUniformDirect(const std::string& name, bool value);
        void SetUniformDirect(const std::string& name, int value);
        void SetUniformDirect(const std::string& name, float value);
        void SetUniformDirect(const std::string& name, const Math::Vec2& value);
        void SetUniformDirect(const std::string& name, const Math::Vec3& value);
        void SetUniformDirect(const std::string& name, const Math::Vec4& value);
        void SetUniformDirect(const std::string& name, const Math::Mat3& value);
        void SetUniformDirect(const std::string& name, const Math::Mat4& value);
        
        // Legacy uniform setters (for backward compatibility)
        void SetBool(const std::string& name, bool value) { SetUniform(name, value); }
        void SetInt(const std::string& name, int value) { SetUniform(name, value); }
        void SetFloat(const std::string& name, float value) { SetUniform(name, value); }
        void SetVec2(const std::string& name, const Math::Vec2& value) { SetUniform(name, value); }
        void SetVec3(const std::string& name, const Math::Vec3& value) { SetUniform(name, value); }
        void SetVec4(const std::string& name, const Math::Vec4& value) { SetUniform(name, value); }
        void SetMat3(const std::string& name, const Math::Mat3& value) { SetUniform(name, value); }
        void SetMat4(const std::string& name, const Math::Mat4& value) { SetUniform(name, value); }
        
        // Texture binding with automatic slot management (optimized)
        void BindTexture(const std::string& name, uint32_t textureId, uint32_t slot = 0);
        void BindTexture(const std::string& name, const Texture& texture, uint32_t slot = 0);
        void BindTextureAuto(const std::string& name, uint32_t textureId); // Automatic slot assignment
        void BindTextureAuto(const std::string& name, const Texture& texture); // Automatic slot assignment
        void BindImageTexture(const std::string& name, uint32_t textureId, uint32_t slot, uint32_t access);
        
        // Direct texture binding (bypass state management)
        void BindTextureDirect(const std::string& name, uint32_t textureId, uint32_t slot = 0);
        void BindTextureDirect(const std::string& name, const Texture& texture, uint32_t slot = 0);
        
        // Storage buffer and uniform buffer binding
        void BindStorageBuffer(const std::string& name, uint32_t bufferId, uint32_t binding);
        void BindUniformBuffer(const std::string& name, uint32_t bufferId, uint32_t binding);
        
        // Compute shader dispatch
        void Dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1);
        void DispatchIndirect(uint32_t indirectBuffer);
        
        // Synchronization
        void MemoryBarrier(uint32_t barriers);
        void WaitForCompletion();
        
        // Resource management utilities
        void ResetTextureSlots();
        uint32_t GetTextureSlot(const std::string& name) const;
        bool HasUniform(const std::string& name) const;
        
        // State management utilities
        void FlushPendingUpdates(); // Force immediate application of queued updates
        void EnableStateOptimization(bool enable) { m_useStateOptimization = enable; }
        bool IsStateOptimizationEnabled() const { return m_useStateOptimization; }
        void RegisterWithStateManager(); // Register this shader with the state manager
        
        uint32_t GetProgramID() const { return m_programID; }
        bool IsValid() const { return m_programID != 0 && m_state == State::Linked; }
        State GetState() const { return m_state; }
        std::string GetCompileLog() const { return m_compileLog; }
        std::string GetLinkLog() const { return m_linkLog; }
        
        // Error handling and debugging
        void SetErrorCallback(std::function<void(const ShaderCompilationError&)> callback);
        void SetWarningCallback(std::function<void(const std::string&, const std::string&)> callback);
        bool ValidateShader() const;
        std::vector<std::string> GetValidationWarnings() const;
        std::vector<std::string> GetPerformanceWarnings() const;

    private:
        uint32_t CompileShader(const std::string& source, uint32_t type);
        bool LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
        bool LinkComputeProgram(uint32_t computeShader);
        int GetUniformLocation(const std::string& name);
        uint32_t GetGLShaderType(Type type);
        uint32_t GetNextTextureSlot();
        
        uint32_t m_programID = 0;
        State m_state = State::Uncompiled;
        std::unordered_map<std::string, int> m_uniformCache;
        std::unordered_map<Type, uint32_t> m_shaders;
        std::string m_compileLog;
        std::string m_linkLog;
        uint32_t m_nextTextureSlot = 0;
        std::unordered_map<std::string, uint32_t> m_textureSlots; // Track texture slot assignments
        
        // Error handling callbacks
        std::function<void(const ShaderCompilationError&)> m_errorCallback;
        std::function<void(const std::string&, const std::string&)> m_warningCallback;
        
        // Cached shader sources for validation
        std::unordered_map<Type, std::string> m_shaderSources;
        
        // State management optimization
        bool m_useStateOptimization = false; // Disabled by default for safety
        bool m_registeredWithStateManager = false;
    };
}