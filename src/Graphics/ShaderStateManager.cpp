#include "Graphics/ShaderStateManager.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <algorithm>

namespace GameEngine {
    ShaderStateManager& ShaderStateManager::GetInstance() {
        static ShaderStateManager instance;
        return instance;
    }

    bool ShaderStateManager::Initialize() {
        if (m_initialized) {
            return true;
        }

        // Query maximum texture units from OpenGL
        int maxTextureUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        m_maxTextureSlots = static_cast<uint32_t>(maxTextureUnits);
        
        // Initialize texture slot usage tracking
        m_textureSlotUsage.resize(m_maxTextureSlots, false);
        
        LOG_INFO("ShaderStateManager initialized with " + std::to_string(m_maxTextureSlots) + " texture slots");
        
        m_initialized = true;
        return true;
    }

    void ShaderStateManager::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Clear all state
        m_activeShader.reset();
        m_pendingUniforms.clear();
        m_pendingTextures.clear();
        m_pendingUniformBuffers.clear();
        m_pendingStorageBuffers.clear();
        m_textureSlots.clear();
        m_uniformLocationCache.clear();
        
        std::fill(m_textureSlotUsage.begin(), m_textureSlotUsage.end(), false);
        m_nextAvailableSlot = 0;
        
        m_initialized = false;
        LOG_INFO("ShaderStateManager shutdown");
    }

    void ShaderStateManager::BeginFrame() {
        // Reset per-frame statistics
        m_stateChangeCount = 0;
        m_performanceStats.batchedUniforms = 0;
        m_performanceStats.textureSlotOptimizations = 0;
    }

    void ShaderStateManager::EndFrame() {
        // Flush any remaining updates
        FlushPendingUpdates();
        
        // Update performance statistics
        m_performanceStats.totalStateChanges += m_stateChangeCount;
        
        if (!m_pendingUniforms.empty()) {
            m_performanceStats.averageBatchSize = 
                (m_performanceStats.averageBatchSize + static_cast<float>(m_pendingUniforms.size())) / 2.0f;
        }
    }

    void ShaderStateManager::FlushPendingUpdates() {
        if (m_activeProgramId == 0) {
            return;
        }

        // Apply all pending updates
        ApplyUniformUpdates();
        ApplyTextureBindings();
        ApplyBufferBindings();
        
        // Clear pending updates
        m_pendingUniforms.clear();
        m_pendingTextures.clear();
        m_pendingUniformBuffers.clear();
        m_pendingStorageBuffers.clear();
    }

    void ShaderStateManager::SetActiveShader(std::shared_ptr<Shader> shader) {
        if (shader == m_activeShader) {
            return; // No change needed
        }

        // Flush pending updates for previous shader
        if (m_activeShader) {
            FlushPendingUpdates();
        }

        m_activeShader = shader;
        
        if (shader && shader->IsValid()) {
            // Check if we need to actually change the OpenGL state
            uint32_t newProgramId = shader->GetProgramID();
            if (m_currentState.programId != newProgramId) {
                glUseProgram(newProgramId);
                m_currentState.programId = newProgramId;
                m_stateChangeCount++;
                
                if (m_logStateChanges) {
                    LOG_INFO("Shader state change: Program " + std::to_string(newProgramId));
                }
            } else {
                m_performanceStats.avoidedStateChanges++;
            }
            
            // Clear uniform location cache for new shader
            m_uniformLocationCache.clear();
        }
    }

    bool ShaderStateManager::IsShaderActive(uint32_t programId) const {
        return m_currentState.programId == programId;
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, const UniformValue& value) {
        // Check if this uniform value has actually changed
        auto it = m_currentState.uniforms.find(name);
        if (it != m_currentState.uniforms.end() && IsUniformValueEqual(it->second, value)) {
            m_performanceStats.avoidedStateChanges++;
            return; // No change needed
        }

        // Queue the update
        UniformUpdate update;
        update.name = name;
        update.value = value;
        update.location = GetUniformLocation(name);
        
        m_pendingUniforms.push_back(update);
        m_currentState.uniforms[name] = value;
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, bool value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, int value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, float value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, const Math::Vec2& value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, const Math::Vec3& value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, const Math::Vec4& value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, const Math::Mat3& value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformUpdate(const std::string& name, const Math::Mat4& value) {
        QueueUniformUpdate(name, UniformValue(value));
    }

    void ShaderStateManager::QueueUniformArrayUpdate(const std::string& name, const std::vector<Math::Mat4>& values) {
        QueueUniformUpdate(name, UniformValue(values));
    }

    void ShaderStateManager::QueueUniformArrayUpdate(const std::string& name, const std::vector<Math::Vec3>& values) {
        QueueUniformUpdate(name, UniformValue(values));
    }

    void ShaderStateManager::QueueUniformArrayUpdate(const std::string& name, const std::vector<Math::Vec4>& values) {
        QueueUniformUpdate(name, UniformValue(values));
    }

    void ShaderStateManager::QueueUniformArrayUpdate(const std::string& name, const std::vector<float>& values) {
        QueueUniformUpdate(name, UniformValue(values));
    }

    void ShaderStateManager::QueueUniformArrayUpdate(const std::string& name, const std::vector<int>& values) {
        QueueUniformUpdate(name, UniformValue(values));
    }

    void ShaderStateManager::QueueTextureBinding(const std::string& name, uint32_t textureId, uint32_t target) {
        // Check if we already have this texture bound to the same slot
        auto it = m_currentState.textures.find(name);
        if (it != m_currentState.textures.end() && 
            it->second.textureId == textureId && 
            it->second.target == target) {
            m_performanceStats.avoidedStateChanges++;
            return; // No change needed
        }

        // Get or allocate texture slot
        uint32_t slot = GetTextureSlot(name);
        if (slot == UINT32_MAX) {
            slot = AllocateTextureSlot(name);
        }

        TextureBinding binding;
        binding.name = name;
        binding.textureId = textureId;
        binding.slot = slot;
        binding.target = target;
        binding.isDirty = true;

        m_pendingTextures.push_back(binding);
        m_currentState.textures[name] = binding;
    }

    void ShaderStateManager::QueueTextureBinding(const std::string& name, const Texture& texture) {
        QueueTextureBinding(name, texture.GetID(), GL_TEXTURE_2D);
    }

    uint32_t ShaderStateManager::AllocateTextureSlot(const std::string& name) {
        uint32_t slot = FindAvailableTextureSlot();
        if (slot < m_maxTextureSlots) {
            m_textureSlots[name] = slot;
            m_textureSlotUsage[slot] = true;
            m_performanceStats.textureSlotOptimizations++;
            
            if (m_logStateChanges) {
                LOG_INFO("Allocated texture slot " + std::to_string(slot) + " for " + name);
            }
        }
        return slot;
    }

    void ShaderStateManager::ReleaseTextureSlot(const std::string& name) {
        auto it = m_textureSlots.find(name);
        if (it != m_textureSlots.end()) {
            uint32_t slot = it->second;
            if (slot < m_maxTextureSlots) {
                m_textureSlotUsage[slot] = false;
            }
            m_textureSlots.erase(it);
            
            if (m_logStateChanges) {
                LOG_INFO("Released texture slot " + std::to_string(slot) + " for " + name);
            }
        }
    }

    void ShaderStateManager::ResetTextureSlots() {
        m_textureSlots.clear();
        std::fill(m_textureSlotUsage.begin(), m_textureSlotUsage.end(), false);
        m_nextAvailableSlot = 0;
        
        if (m_logStateChanges) {
            LOG_INFO("Reset all texture slots");
        }
    }

    uint32_t ShaderStateManager::GetTextureSlot(const std::string& name) const {
        auto it = m_textureSlots.find(name);
        return (it != m_textureSlots.end()) ? it->second : UINT32_MAX;
    }

    void ShaderStateManager::QueueUniformBufferBinding(const std::string& name, uint32_t bufferId, uint32_t binding) {
        // Check if this buffer is already bound
        auto it = m_currentState.uniformBuffers.find(name);
        if (it != m_currentState.uniformBuffers.end() && it->second == bufferId) {
            m_performanceStats.avoidedStateChanges++;
            return; // No change needed
        }

        m_pendingUniformBuffers[name] = bufferId;
        m_currentState.uniformBuffers[name] = bufferId;
    }

    void ShaderStateManager::QueueStorageBufferBinding(const std::string& name, uint32_t bufferId, uint32_t binding) {
        // Check if this buffer is already bound
        auto it = m_currentState.storageBuffers.find(name);
        if (it != m_currentState.storageBuffers.end() && it->second == bufferId) {
            m_performanceStats.avoidedStateChanges++;
            return; // No change needed
        }

        m_pendingStorageBuffers[name] = bufferId;
        m_currentState.storageBuffers[name] = bufferId;
    }

    void ShaderStateManager::ValidateState() const {
        if (!m_activeShader || !m_activeShader->IsValid()) {
            LOG_WARNING("ShaderStateManager: No active shader or shader is invalid");
            return;
        }

        // Validate that current OpenGL state matches our cached state
        int currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        
        if (static_cast<uint32_t>(currentProgram) != m_currentState.programId) {
            LOG_WARNING("ShaderStateManager: OpenGL program state mismatch. Expected: " + 
                       std::to_string(m_currentState.programId) + ", Actual: " + std::to_string(currentProgram));
        }
    }

    void ShaderStateManager::ResetPerformanceStats() {
        m_performanceStats = PerformanceStats{};
    }

    void ShaderStateManager::RegisterShader(uint32_t programId, const std::string& name) {
        if (programId != 0) {
            m_registeredShaders[programId] = name;
            
            if (m_logStateChanges) {
                LOG_INFO("Registered shader '" + name + "' with program ID " + std::to_string(programId));
            }
        }
    }

    void ShaderStateManager::UnregisterShader(uint32_t programId) {
        auto it = m_registeredShaders.find(programId);
        if (it != m_registeredShaders.end()) {
            if (m_logStateChanges) {
                LOG_INFO("Unregistered shader '" + it->second + "' with program ID " + std::to_string(programId));
            }
            m_registeredShaders.erase(it);
        }
        
        // If this was the active shader, clear it
        if (m_activeProgramId == programId) {
            m_activeProgramId = 0;
            m_currentState.programId = 0;
        }
    }

    void ShaderStateManager::SetActiveShaderByProgramId(uint32_t programId) {
        if (programId == m_activeProgramId) {
            return; // No change needed
        }

        // Flush pending updates for previous shader
        if (m_activeProgramId != 0) {
            FlushPendingUpdates();
        }

        m_activeProgramId = programId;
        
        if (programId != 0) {
            // Check if we need to actually change the OpenGL state
            if (m_currentState.programId != programId) {
                glUseProgram(programId);
                m_currentState.programId = programId;
                m_stateChangeCount++;
                
                if (m_logStateChanges) {
                    auto it = m_registeredShaders.find(programId);
                    std::string shaderName = (it != m_registeredShaders.end()) ? it->second : "Unknown";
                    LOG_INFO("Shader state change: Program " + std::to_string(programId) + " (" + shaderName + ")");
                }
            } else {
                m_performanceStats.avoidedStateChanges++;
            }
            
            // Clear uniform location cache for new shader
            m_uniformLocationCache.clear();
        }
    }

    // Private methods
    void ShaderStateManager::ApplyUniformUpdates() {
        if (m_pendingUniforms.empty()) {
            return;
        }

        for (const auto& update : m_pendingUniforms) {
            if (update.location != -1) {
                ApplyUniformValue(update.name, update.value, update.location);
            }
        }

        m_performanceStats.batchedUniforms += m_pendingUniforms.size();
        
        if (m_logStateChanges && !m_pendingUniforms.empty()) {
            LOG_INFO("Applied " + std::to_string(m_pendingUniforms.size()) + " batched uniform updates");
        }
    }

    void ShaderStateManager::ApplyTextureBindings() {
        if (m_pendingTextures.empty()) {
            return;
        }

        for (const auto& binding : m_pendingTextures) {
            if (binding.slot < m_maxTextureSlots) {
                glActiveTexture(GL_TEXTURE0 + binding.slot);
                glBindTexture(binding.target, binding.textureId);
                
                // Set the uniform sampler to the correct slot
                int location = GetUniformLocation(binding.name);
                if (location != -1) {
                    glUniform1i(location, static_cast<int>(binding.slot));
                }
                
                m_stateChangeCount++;
            }
        }

        if (m_logStateChanges && !m_pendingTextures.empty()) {
            LOG_INFO("Applied " + std::to_string(m_pendingTextures.size()) + " texture bindings");
        }
    }

    void ShaderStateManager::ApplyBufferBindings() {
        // Apply uniform buffer bindings
        for (const auto& pair : m_pendingUniformBuffers) {
            // Find binding point for this uniform block
            uint32_t blockIndex = glGetUniformBlockIndex(m_currentState.programId, pair.first.c_str());
            if (blockIndex != GL_INVALID_INDEX) {
                glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, pair.second);
                glUniformBlockBinding(m_currentState.programId, blockIndex, blockIndex);
                m_stateChangeCount++;
            }
        }

        // Apply storage buffer bindings
        for (const auto& pair : m_pendingStorageBuffers) {
            // Find binding point for this storage block
            uint32_t blockIndex = glGetProgramResourceIndex(m_currentState.programId, GL_SHADER_STORAGE_BLOCK, pair.first.c_str());
            if (blockIndex != GL_INVALID_INDEX) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, blockIndex, pair.second);
                glShaderStorageBlockBinding(m_currentState.programId, blockIndex, blockIndex);
                m_stateChangeCount++;
            }
        }

        if (m_logStateChanges && (!m_pendingUniformBuffers.empty() || !m_pendingStorageBuffers.empty())) {
            LOG_INFO("Applied " + std::to_string(m_pendingUniformBuffers.size() + m_pendingStorageBuffers.size()) + " buffer bindings");
        }
    }

    bool ShaderStateManager::IsUniformValueEqual(const UniformValue& a, const UniformValue& b) const {
        if (a.index() != b.index()) {
            return false; // Different types
        }

        // Compare based on the actual type
        return std::visit([&b](const auto& aVal) -> bool {
            using T = std::decay_t<decltype(aVal)>;
            if (const T* bVal = std::get_if<T>(&b)) {
                if constexpr (std::is_same_v<T, Math::Vec2> || std::is_same_v<T, Math::Vec3> || std::is_same_v<T, Math::Vec4>) {
                    // Use epsilon comparison for floating point vectors
                    constexpr float epsilon = 1e-6f;
                    for (int i = 0; i < aVal.length(); ++i) {
                        if (std::abs(aVal[i] - (*bVal)[i]) > epsilon) {
                            return false;
                        }
                    }
                    return true;
                } else if constexpr (std::is_same_v<T, Math::Mat3> || std::is_same_v<T, Math::Mat4>) {
                    // Use epsilon comparison for matrices
                    constexpr float epsilon = 1e-6f;
                    for (int i = 0; i < aVal.length(); ++i) {
                        for (int j = 0; j < aVal[i].length(); ++j) {
                            if (std::abs(aVal[i][j] - (*bVal)[i][j]) > epsilon) {
                                return false;
                            }
                        }
                    }
                    return true;
                } else if constexpr (std::is_same_v<T, float>) {
                    // Use epsilon comparison for floats
                    constexpr float epsilon = 1e-6f;
                    return std::abs(aVal - *bVal) <= epsilon;
                } else {
                    // Direct comparison for other types
                    return aVal == *bVal;
                }
            }
            return false;
        }, a);
    }

    void ShaderStateManager::ApplyUniformValue(const std::string& name, const UniformValue& value, int location) {
        std::visit([location](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, bool>) {
                glUniform1i(location, static_cast<int>(val));
            } else if constexpr (std::is_same_v<T, int>) {
                glUniform1i(location, val);
            } else if constexpr (std::is_same_v<T, float>) {
                glUniform1f(location, val);
            } else if constexpr (std::is_same_v<T, Math::Vec2>) {
                glUniform2fv(location, 1, &val[0]);
            } else if constexpr (std::is_same_v<T, Math::Vec3>) {
                glUniform3fv(location, 1, &val[0]);
            } else if constexpr (std::is_same_v<T, Math::Vec4>) {
                glUniform4fv(location, 1, &val[0]);
            } else if constexpr (std::is_same_v<T, Math::Mat3>) {
                glUniformMatrix3fv(location, 1, GL_FALSE, &val[0][0]);
            } else if constexpr (std::is_same_v<T, Math::Mat4>) {
                glUniformMatrix4fv(location, 1, GL_FALSE, &val[0][0]);
            } else if constexpr (std::is_same_v<T, std::vector<float>>) {
                if (!val.empty()) {
                    glUniform1fv(location, static_cast<GLsizei>(val.size()), val.data());
                }
            } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                if (!val.empty()) {
                    glUniform1iv(location, static_cast<GLsizei>(val.size()), val.data());
                }
            } else if constexpr (std::is_same_v<T, std::vector<Math::Vec3>>) {
                if (!val.empty()) {
                    glUniform3fv(location, static_cast<GLsizei>(val.size()), &val[0][0]);
                }
            } else if constexpr (std::is_same_v<T, std::vector<Math::Vec4>>) {
                if (!val.empty()) {
                    glUniform4fv(location, static_cast<GLsizei>(val.size()), &val[0][0]);
                }
            } else if constexpr (std::is_same_v<T, std::vector<Math::Mat4>>) {
                if (!val.empty()) {
                    glUniformMatrix4fv(location, static_cast<GLsizei>(val.size()), GL_FALSE, &val[0][0][0]);
                }
            }
        }, value);
    }

    int ShaderStateManager::GetUniformLocation(const std::string& name) {
        // Check cache first
        auto it = m_uniformLocationCache.find(name);
        if (it != m_uniformLocationCache.end()) {
            return it->second;
        }

        // Query OpenGL and cache the result
        int location = -1;
        if (m_activeProgramId != 0) {
            location = glGetUniformLocation(m_activeProgramId, name.c_str());
            m_uniformLocationCache[name] = location;
        }

        return location;
    }

    uint32_t ShaderStateManager::FindAvailableTextureSlot() const {
        for (uint32_t i = 0; i < m_maxTextureSlots; ++i) {
            if (!m_textureSlotUsage[i]) {
                return i;
            }
        }
        return m_maxTextureSlots; // No available slots
    }

    void ShaderStateManager::UpdateTextureSlotUsage() {
        // Reset usage tracking
        std::fill(m_textureSlotUsage.begin(), m_textureSlotUsage.end(), false);
        
        // Mark slots as used based on current texture bindings
        for (const auto& pair : m_textureSlots) {
            uint32_t slot = pair.second;
            if (slot < m_maxTextureSlots) {
                m_textureSlotUsage[slot] = true;
            }
        }
    }
}