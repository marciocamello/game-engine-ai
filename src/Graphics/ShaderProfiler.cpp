#include "Graphics/ShaderProfiler.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>

namespace GameEngine {

    // ShaderProfiler implementation
    ShaderProfiler& ShaderProfiler::GetInstance() {
        static ShaderProfiler instance;
        return instance;
    }

    void ShaderProfiler::StartProfiling() {
        m_profilingEnabled = true;
        LOG_INFO("Shader profiling started");
    }

    void ShaderProfiler::StopProfiling() {
        m_profilingEnabled = false;
        LOG_INFO("Shader profiling stopped");
    }

    void ShaderProfiler::ResetStats() {
        for (auto& pair : m_shaderStats) {
            pair.second.Reset();
        }
        LOG_INFO("Shader profiling statistics reset");
    }

    void ShaderProfiler::RegisterShader(const std::string& name, uint32_t programId) {
        if (!m_profilingEnabled) return;
        
        m_shaderPrograms[name] = programId;
        if (m_shaderStats.find(name) == m_shaderStats.end()) {
            m_shaderStats[name] = ShaderPerformanceStats();
        }
        
        // Update shader complexity information
        UpdateShaderComplexity(name, programId);
        
        LOG_INFO("Registered shader for profiling: " + name);
    }

    void ShaderProfiler::UnregisterShader(const std::string& name) {
        m_shaderPrograms.erase(name);
        m_shaderStats.erase(name);
        m_timingStart.erase(name);
        LOG_INFO("Unregistered shader from profiling: " + name);
    }

    void ShaderProfiler::BeginShaderTiming(const std::string& shaderName) {
        if (!m_profilingEnabled) return;
        
        m_timingStart[shaderName] = std::chrono::high_resolution_clock::now();
    }

    void ShaderProfiler::EndShaderTiming(const std::string& shaderName) {
        if (!m_profilingEnabled) return;
        
        auto it = m_timingStart.find(shaderName);
        if (it != m_timingStart.end()) {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second);
            double frameTimeMs = duration.count() / 1000.0;
            
            RecordFrameTime(shaderName, frameTimeMs);
            m_timingStart.erase(it);
        }
    }

    void ShaderProfiler::RecordCompilationTime(const std::string& shaderName, double timeMs) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.compilationTimeMs += timeMs;
        stats.compiledShaderCount++;
    }

    void ShaderProfiler::RecordLinkingTime(const std::string& shaderName, double timeMs) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.linkingTimeMs += timeMs;
    }

    void ShaderProfiler::RecordDrawCall(const std::string& shaderName) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.totalDrawCalls++;
    }

    void ShaderProfiler::RecordFrameTime(const std::string& shaderName, double frameTimeMs) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.totalFrames++;
        
        // Update frame time statistics
        stats.maxFrameTimeMs = std::max(stats.maxFrameTimeMs, frameTimeMs);
        stats.minFrameTimeMs = std::min(stats.minFrameTimeMs, frameTimeMs);
        
        // Calculate running average
        double totalTime = stats.averageFrameTimeMs * (stats.totalFrames - 1) + frameTimeMs;
        stats.averageFrameTimeMs = totalTime / stats.totalFrames;
    }

    void ShaderProfiler::UpdateGPUMemoryUsage(const std::string& shaderName, size_t bytes) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.gpuMemoryUsageBytes = bytes;
    }

    void ShaderProfiler::UpdateUniformBufferSize(const std::string& shaderName, size_t bytes) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.uniformBufferSize = bytes;
    }

    void ShaderProfiler::UpdateTextureMemoryUsage(const std::string& shaderName, size_t bytes) {
        if (!m_profilingEnabled) return;
        
        auto& stats = m_shaderStats[shaderName];
        stats.textureMemoryUsage = bytes;
    }

    std::vector<ShaderResourceInfo> ShaderProfiler::GetShaderUniforms(uint32_t programId) {
        std::vector<ShaderResourceInfo> uniforms;
        
        if (programId == 0) return uniforms;
        
        int uniformCount;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
        
        for (int i = 0; i < uniformCount; ++i) {
            char name[256];
            int size;
            uint32_t type;
            glGetActiveUniform(programId, i, sizeof(name), nullptr, &size, &type, name);
            
            int location = glGetUniformLocation(programId, name);
            uniforms.emplace_back(name, location, type, size, location != -1);
        }
        
        return uniforms;
    }

    std::vector<ShaderResourceInfo> ShaderProfiler::GetShaderAttributes(uint32_t programId) {
        std::vector<ShaderResourceInfo> attributes;
        
        if (programId == 0) return attributes;
        
        int attributeCount;
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        
        for (int i = 0; i < attributeCount; ++i) {
            char name[256];
            int size;
            uint32_t type;
            glGetActiveAttrib(programId, i, sizeof(name), nullptr, &size, &type, name);
            
            int location = glGetAttribLocation(programId, name);
            attributes.emplace_back(name, location, type, size, location != -1);
        }
        
        return attributes;
    }

    std::vector<ShaderResourceInfo> ShaderProfiler::GetShaderStorageBuffers(uint32_t programId) {
        std::vector<ShaderResourceInfo> storageBuffers;
        
        if (programId == 0) return storageBuffers;
        
        // Check if the OpenGL version supports shader storage buffers
        int majorVersion, minorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
        
        if (majorVersion < 4 || (majorVersion == 4 && minorVersion < 3)) {
            return storageBuffers; // SSBO requires OpenGL 4.3+
        }
        
        int storageBlockCount;
        glGetProgramInterfaceiv(programId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storageBlockCount);
        
        for (int i = 0; i < storageBlockCount; ++i) {
            char name[256];
            glGetProgramResourceName(programId, GL_SHADER_STORAGE_BLOCK, i, sizeof(name), nullptr, name);
            
            uint32_t index = glGetProgramResourceIndex(programId, GL_SHADER_STORAGE_BLOCK, name);
            storageBuffers.emplace_back(name, index, GL_SHADER_STORAGE_BLOCK, 1, index != GL_INVALID_INDEX);
        }
        
        return storageBuffers;
    }

    int ShaderProfiler::EstimateInstructionCount(uint32_t programId) {
        // This is a rough estimation based on available OpenGL queries
        // Real instruction counting would require vendor-specific extensions
        
        if (programId == 0) return 0;
        
        int uniformCount, attributeCount;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        
        // Rough estimation: more uniforms and attributes typically mean more instructions
        return (uniformCount * 2) + (attributeCount * 3) + 10; // Base instruction count
    }

    ShaderPerformanceStats ShaderProfiler::GetShaderStats(const std::string& shaderName) const {
        auto it = m_shaderStats.find(shaderName);
        if (it != m_shaderStats.end()) {
            return it->second;
        }
        return ShaderPerformanceStats();
    }

    ShaderPerformanceStats ShaderProfiler::GetGlobalStats() const {
        ShaderPerformanceStats globalStats;
        
        for (const auto& pair : m_shaderStats) {
            const auto& stats = pair.second;
            globalStats.compilationTimeMs += stats.compilationTimeMs;
            globalStats.linkingTimeMs += stats.linkingTimeMs;
            globalStats.compiledShaderCount += stats.compiledShaderCount;
            globalStats.totalDrawCalls += stats.totalDrawCalls;
            globalStats.totalFrames += stats.totalFrames;
            globalStats.gpuMemoryUsageBytes += stats.gpuMemoryUsageBytes;
            globalStats.uniformBufferSize += stats.uniformBufferSize;
            globalStats.textureMemoryUsage += stats.textureMemoryUsage;
            globalStats.uniformCount += stats.uniformCount;
            globalStats.attributeCount += stats.attributeCount;
            globalStats.textureUnitCount += stats.textureUnitCount;
            globalStats.instructionCount += stats.instructionCount;
            globalStats.compilationErrors += stats.compilationErrors;
            globalStats.linkingErrors += stats.linkingErrors;
            globalStats.runtimeErrors += stats.runtimeErrors;
            
            globalStats.maxFrameTimeMs = std::max(globalStats.maxFrameTimeMs, stats.maxFrameTimeMs);
            if (stats.minFrameTimeMs < 999999.0) {
                globalStats.minFrameTimeMs = std::min(globalStats.minFrameTimeMs, stats.minFrameTimeMs);
            }
        }
        
        // Calculate global average frame time
        if (globalStats.totalFrames > 0) {
            double totalFrameTime = 0.0;
            for (const auto& pair : m_shaderStats) {
                totalFrameTime += pair.second.averageFrameTimeMs * pair.second.totalFrames;
            }
            globalStats.averageFrameTimeMs = totalFrameTime / globalStats.totalFrames;
        }
        
        return globalStats;
    }

    std::vector<std::string> ShaderProfiler::GetProfiledShaders() const {
        std::vector<std::string> shaderNames;
        for (const auto& pair : m_shaderStats) {
            shaderNames.push_back(pair.first);
        }
        return shaderNames;
    }

    std::vector<std::string> ShaderProfiler::GetPerformanceBottlenecks() const {
        std::vector<std::string> bottlenecks;
        
        for (const auto& pair : m_shaderStats) {
            const auto& name = pair.first;
            const auto& stats = pair.second;
            
            if (stats.averageFrameTimeMs > m_maxFrameTimeMs) {
                bottlenecks.push_back("Shader '" + name + "' exceeds frame time threshold (" + 
                                    FormatTime(stats.averageFrameTimeMs) + " > " + 
                                    FormatTime(m_maxFrameTimeMs) + ")");
            }
            
            if (stats.compilationTimeMs > 100.0) { // 100ms compilation threshold
                bottlenecks.push_back("Shader '" + name + "' has slow compilation time (" + 
                                    FormatTime(stats.compilationTimeMs) + ")");
            }
            
            if (stats.instructionCount > 200) { // High instruction count threshold
                bottlenecks.push_back("Shader '" + name + "' has high instruction count (" + 
                                    std::to_string(stats.instructionCount) + ")");
            }
        }
        
        return bottlenecks;
    }

    std::vector<std::string> ShaderProfiler::GetMemoryHogs() const {
        std::vector<std::string> memoryHogs;
        
        // Sort shaders by memory usage
        std::vector<std::pair<std::string, size_t>> shaderMemory;
        for (const auto& pair : m_shaderStats) {
            size_t totalMemory = pair.second.gpuMemoryUsageBytes + 
                               pair.second.uniformBufferSize + 
                               pair.second.textureMemoryUsage;
            shaderMemory.emplace_back(pair.first, totalMemory);
        }
        
        std::sort(shaderMemory.begin(), shaderMemory.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Report top memory consumers
        size_t threshold = m_maxMemoryMB * 1024 * 1024 / 10; // 10% of max memory
        for (const auto& pair : shaderMemory) {
            if (pair.second > threshold) {
                memoryHogs.push_back("Shader '" + pair.first + "' uses " + 
                                   FormatMemorySize(pair.second) + " of GPU memory");
            }
        }
        
        return memoryHogs;
    }

    std::vector<std::string> ShaderProfiler::GetOptimizationSuggestions(const std::string& shaderName) const {
        std::vector<std::string> suggestions;
        
        auto it = m_shaderStats.find(shaderName);
        if (it == m_shaderStats.end()) {
            return suggestions;
        }
        
        const auto& stats = it->second;
        
        if (stats.averageFrameTimeMs > m_maxFrameTimeMs) {
            suggestions.push_back("Consider reducing shader complexity or optimizing expensive operations");
        }
        
        if (stats.uniformCount > 50) {
            suggestions.push_back("High uniform count - consider using uniform buffer objects");
        }
        
        if (stats.textureUnitCount > 8) {
            suggestions.push_back("High texture unit usage - consider texture atlasing");
        }
        
        if (stats.instructionCount > 200) {
            suggestions.push_back("High instruction count - review shader for optimization opportunities");
        }
        
        if (stats.compilationTimeMs > 100.0) {
            suggestions.push_back("Slow compilation - consider shader caching or precompilation");
        }
        
        return suggestions;
    }

    std::string ShaderProfiler::GeneratePerformanceReport() const {
        std::stringstream report;
        report << std::fixed << std::setprecision(2);
        
        report << "=== Shader Performance Report ===\n\n";
        
        auto globalStats = GetGlobalStats();
        report << "Global Statistics:\n";
        report << "  Total Shaders: " << m_shaderStats.size() << "\n";
        report << "  Total Compilation Time: " << FormatTime(globalStats.compilationTimeMs) << "\n";
        report << "  Total Linking Time: " << FormatTime(globalStats.linkingTimeMs) << "\n";
        report << "  Average Frame Time: " << FormatTime(globalStats.averageFrameTimeMs) << "\n";
        report << "  Max Frame Time: " << FormatTime(globalStats.maxFrameTimeMs) << "\n";
        report << "  Total Draw Calls: " << globalStats.totalDrawCalls << "\n";
        report << "  Total GPU Memory: " << FormatMemorySize(globalStats.gpuMemoryUsageBytes) << "\n\n";
        
        report << "Per-Shader Statistics:\n";
        for (const auto& pair : m_shaderStats) {
            const auto& name = pair.first;
            const auto& stats = pair.second;
            
            report << "  " << name << ":\n";
            report << "    Compilation Time: " << FormatTime(stats.compilationTimeMs) << "\n";
            report << "    Average Frame Time: " << FormatTime(stats.averageFrameTimeMs) << "\n";
            report << "    Draw Calls: " << stats.totalDrawCalls << "\n";
            report << "    GPU Memory: " << FormatMemorySize(stats.gpuMemoryUsageBytes) << "\n";
            report << "    Uniforms: " << stats.uniformCount << "\n";
            report << "    Instructions (est.): " << stats.instructionCount << "\n\n";
        }
        
        auto bottlenecks = GetPerformanceBottlenecks();
        if (!bottlenecks.empty()) {
            report << "Performance Issues:\n";
            for (const auto& bottleneck : bottlenecks) {
                report << "  - " << bottleneck << "\n";
            }
            report << "\n";
        }
        
        return report.str();
    }

    std::string ShaderProfiler::GenerateMemoryReport() const {
        std::stringstream report;
        
        report << "=== Shader Memory Report ===\n\n";
        
        size_t totalMemory = 0;
        for (const auto& pair : m_shaderStats) {
            totalMemory += pair.second.gpuMemoryUsageBytes + 
                          pair.second.uniformBufferSize + 
                          pair.second.textureMemoryUsage;
        }
        
        report << "Total Shader Memory Usage: " << FormatMemorySize(totalMemory) << "\n";
        report << "Memory Limit: " << FormatMemorySize(m_maxMemoryMB * 1024 * 1024) << "\n";
        report << "Usage Percentage: " << std::fixed << std::setprecision(1) 
               << (totalMemory * 100.0 / (m_maxMemoryMB * 1024 * 1024)) << "%\n\n";
        
        auto memoryHogs = GetMemoryHogs();
        if (!memoryHogs.empty()) {
            report << "High Memory Usage Shaders:\n";
            for (const auto& hog : memoryHogs) {
                report << "  - " << hog << "\n";
            }
        }
        
        return report.str();
    }

    void ShaderProfiler::DumpShaderInfo(const std::string& shaderName) const {
        auto it = m_shaderPrograms.find(shaderName);
        if (it == m_shaderPrograms.end()) {
            LOG_WARN("Shader not found for info dump: " + shaderName);
            return;
        }
        
        uint32_t programId = it->second;
        LOG_INFO("=== Shader Info: " + shaderName + " ===");
        
        auto uniforms = GetShaderUniforms(programId);
        LOG_INFO("Uniforms (" + std::to_string(uniforms.size()) + "):");
        for (const auto& uniform : uniforms) {
            LOG_INFO("  " + uniform.name + " (location: " + std::to_string(uniform.location) + 
                    ", type: " + std::to_string(uniform.type) + ")");
        }
        
        auto attributes = GetShaderAttributes(programId);
        LOG_INFO("Attributes (" + std::to_string(attributes.size()) + "):");
        for (const auto& attribute : attributes) {
            LOG_INFO("  " + attribute.name + " (location: " + std::to_string(attribute.location) + 
                    ", type: " + std::to_string(attribute.type) + ")");
        }
        
        auto stats = GetShaderStats(shaderName);
        LOG_INFO("Performance Stats:");
        LOG_INFO("  Average Frame Time: " + FormatTime(stats.averageFrameTimeMs));
        LOG_INFO("  Draw Calls: " + std::to_string(stats.totalDrawCalls));
        LOG_INFO("  GPU Memory: " + FormatMemorySize(stats.gpuMemoryUsageBytes));
    }

    size_t ShaderProfiler::GetTotalGPUMemoryUsage() const {
        size_t total = 0;
        for (const auto& pair : m_shaderStats) {
            total += pair.second.gpuMemoryUsageBytes + 
                    pair.second.uniformBufferSize + 
                    pair.second.textureMemoryUsage;
        }
        return total;
    }

    size_t ShaderProfiler::GetShaderMemoryUsage(const std::string& shaderName) const {
        auto it = m_shaderStats.find(shaderName);
        if (it != m_shaderStats.end()) {
            const auto& stats = it->second;
            return stats.gpuMemoryUsageBytes + stats.uniformBufferSize + stats.textureMemoryUsage;
        }
        return 0;
    }

    float ShaderProfiler::GetMemoryUsagePercentage() const {
        size_t totalUsage = GetTotalGPUMemoryUsage();
        size_t maxMemory = m_maxMemoryMB * 1024 * 1024;
        return (totalUsage * 100.0f) / maxMemory;
    }

    void ShaderProfiler::SetPerformanceThresholds(double maxFrameTimeMs, size_t maxMemoryMB) {
        m_maxFrameTimeMs = maxFrameTimeMs;
        m_maxMemoryMB = maxMemoryMB;
    }

    bool ShaderProfiler::IsShaderPerformanceAcceptable(const std::string& shaderName) const {
        auto it = m_shaderStats.find(shaderName);
        if (it == m_shaderStats.end()) {
            return true; // No data, assume acceptable
        }
        
        const auto& stats = it->second;
        return stats.averageFrameTimeMs <= m_maxFrameTimeMs &&
               GetShaderMemoryUsage(shaderName) <= (m_maxMemoryMB * 1024 * 1024);
    }

    void ShaderProfiler::UpdateShaderComplexity(const std::string& shaderName, uint32_t programId) {
        auto& stats = m_shaderStats[shaderName];
        
        auto uniforms = GetShaderUniforms(programId);
        auto attributes = GetShaderAttributes(programId);
        
        stats.uniformCount = static_cast<int>(uniforms.size());
        stats.attributeCount = static_cast<int>(attributes.size());
        stats.instructionCount = EstimateInstructionCount(programId);
        
        // Count active texture units
        stats.textureUnitCount = 0;
        for (const auto& uniform : uniforms) {
            if (uniform.type == GL_SAMPLER_2D || uniform.type == GL_SAMPLER_CUBE || 
                uniform.type == GL_SAMPLER_3D || uniform.type == GL_SAMPLER_2D_ARRAY) {
                stats.textureUnitCount++;
            }
        }
    }

    std::string ShaderProfiler::FormatMemorySize(size_t bytes) const {
        if (bytes < 1024) {
            return std::to_string(bytes) + " B";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024) + " KB";
        } else {
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        }
    }

    std::string ShaderProfiler::FormatTime(double timeMs) const {
        if (timeMs < 1.0) {
            return std::to_string(static_cast<int>(timeMs * 1000)) + " μs";
        } else {
            return std::to_string(timeMs) + " ms";
        }
    }

    double ShaderProfiler::CalculateAverageFrameTime(const ShaderPerformanceStats& stats) const {
        return stats.averageFrameTimeMs;
    }

} 
   // GPUMemoryTracker implementation
    GPUMemoryTracker& GPUMemoryTracker::GetInstance() {
        static GPUMemoryTracker instance;
        return instance;
    }

    void GPUMemoryTracker::TrackShaderMemory(const std::string& shaderName, size_t bytes) {
        m_shaderMemory[shaderName] = bytes;
    }

    void GPUMemoryTracker::TrackTextureMemory(uint32_t textureId, size_t bytes) {
        m_textureMemory[textureId] = bytes;
    }

    void GPUMemoryTracker::TrackBufferMemory(uint32_t bufferId, size_t bytes) {
        m_bufferMemory[bufferId] = bytes;
    }

    void GPUMemoryTracker::ReleaseShaderMemory(const std::string& shaderName) {
        m_shaderMemory.erase(shaderName);
    }

    void GPUMemoryTracker::ReleaseTextureMemory(uint32_t textureId) {
        m_textureMemory.erase(textureId);
    }

    void GPUMemoryTracker::ReleaseBufferMemory(uint32_t bufferId) {
        m_bufferMemory.erase(bufferId);
    }

    size_t GPUMemoryTracker::GetTotalMemoryUsage() const {
        return GetShaderMemoryUsage() + GetTextureMemoryUsage() + GetBufferMemoryUsage();
    }

    size_t GPUMemoryTracker::GetShaderMemoryUsage() const {
        size_t total = 0;
        for (const auto& pair : m_shaderMemory) {
            total += pair.second;
        }
        return total;
    }

    size_t GPUMemoryTracker::GetTextureMemoryUsage() const {
        size_t total = 0;
        for (const auto& pair : m_textureMemory) {
            total += pair.second;
        }
        return total;
    }

    size_t GPUMemoryTracker::GetBufferMemoryUsage() const {
        size_t total = 0;
        for (const auto& pair : m_bufferMemory) {
            total += pair.second;
        }
        return total;
    }

    std::vector<std::pair<std::string, size_t>> GPUMemoryTracker::GetTopMemoryConsumers() const {
        std::vector<std::pair<std::string, size_t>> consumers;
        
        // Add shader memory usage
        for (const auto& pair : m_shaderMemory) {
            consumers.emplace_back("Shader: " + pair.first, pair.second);
        }
        
        // Add texture memory usage
        for (const auto& pair : m_textureMemory) {
            consumers.emplace_back("Texture: " + std::to_string(pair.first), pair.second);
        }
        
        // Add buffer memory usage
        for (const auto& pair : m_bufferMemory) {
            consumers.emplace_back("Buffer: " + std::to_string(pair.first), pair.second);
        }
        
        // Sort by memory usage (descending)
        std::sort(consumers.begin(), consumers.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        return consumers;
    }

    bool GPUMemoryTracker::IsMemoryUsageHigh() const {
        return GetMemoryUsagePercentage() > 80.0f; // 80% threshold
    }

    std::vector<std::string> GPUMemoryTracker::GetMemoryOptimizationSuggestions() const {
        std::vector<std::string> suggestions;
        
        float usage = GetMemoryUsagePercentage();
        if (usage > 90.0f) {
            suggestions.push_back("Critical: Memory usage above 90% - immediate optimization required");
        } else if (usage > 80.0f) {
            suggestions.push_back("Warning: Memory usage above 80% - consider optimization");
        }
        
        size_t textureMemory = GetTextureMemoryUsage();
        size_t totalMemory = GetTotalMemoryUsage();
        
        if (textureMemory > totalMemory * 0.7) {
            suggestions.push_back("Textures consume >70% of memory - consider compression or atlasing");
        }
        
        size_t bufferMemory = GetBufferMemoryUsage();
        if (bufferMemory > totalMemory * 0.3) {
            suggestions.push_back("Buffers consume >30% of memory - review buffer usage patterns");
        }
        
        auto topConsumers = GetTopMemoryConsumers();
        if (!topConsumers.empty() && topConsumers[0].second > totalMemory * 0.5) {
            suggestions.push_back("Single resource consumes >50% of memory: " + topConsumers[0].first);
        }
        
        return suggestions;
    }

    std::string GPUMemoryTracker::GenerateMemoryReport() const {
        std::stringstream report;
        
        report << "=== GPU Memory Usage Report ===\n\n";
        
        size_t totalMemory = GetTotalMemoryUsage();
        report << "Total Memory Usage: " << FormatMemorySize(totalMemory) << "\n";
        report << "Memory Limit: " << FormatMemorySize(m_memoryLimit) << "\n";
        report << "Usage Percentage: " << std::fixed << std::setprecision(1) 
               << GetMemoryUsagePercentage() << "%\n\n";
        
        report << "Memory Breakdown:\n";
        report << "  Shaders: " << FormatMemorySize(GetShaderMemoryUsage()) << "\n";
        report << "  Textures: " << FormatMemorySize(GetTextureMemoryUsage()) << "\n";
        report << "  Buffers: " << FormatMemorySize(GetBufferMemoryUsage()) << "\n\n";
        
        auto topConsumers = GetTopMemoryConsumers();
        if (!topConsumers.empty()) {
            report << "Top Memory Consumers:\n";
            for (size_t i = 0; i < std::min(size_t(10), topConsumers.size()); ++i) {
                report << "  " << (i + 1) << ". " << topConsumers[i].first 
                       << ": " << FormatMemorySize(topConsumers[i].second) << "\n";
            }
            report << "\n";
        }
        
        auto suggestions = GetMemoryOptimizationSuggestions();
        if (!suggestions.empty()) {
            report << "Optimization Suggestions:\n";
            for (const auto& suggestion : suggestions) {
                report << "  - " << suggestion << "\n";
            }
        }
        
        return report.str();
    }

    void GPUMemoryTracker::DumpMemoryUsage() const {
        LOG_INFO(GenerateMemoryReport());
    }

    void GPUMemoryTracker::SetMemoryLimit(size_t limitBytes) {
        m_memoryLimit = limitBytes;
    }

    bool GPUMemoryTracker::IsMemoryLimitExceeded() const {
        return GetTotalMemoryUsage() > m_memoryLimit;
    }

    float GPUMemoryTracker::GetMemoryUsagePercentage() const {
        return (GetTotalMemoryUsage() * 100.0f) / m_memoryLimit;
    }

    std::string GPUMemoryTracker::FormatMemorySize(size_t bytes) const {
        if (bytes < 1024) {
            return std::to_string(bytes) + " B";
        } else if (bytes < 1024 * 1024) {
            return std::to_string(bytes / 1024) + " KB";
        } else if (bytes < 1024 * 1024 * 1024) {
            return std::to_string(bytes / (1024 * 1024)) + " MB";
        } else {
            return std::to_string(bytes / (1024 * 1024 * 1024)) + " GB";
        }
    }

    // ShaderAnalyzer implementation
    ShaderAnalyzer::AnalysisResult ShaderAnalyzer::AnalyzeShaderSource(const std::string& source, const std::string& shaderType) {
        AnalysisResult result;
        
        // Performance analysis
        result.estimatedInstructions = EstimateInstructionCount(source);
        result.textureReads = CountTextureReads(source);
        result.branchingComplexity = AnalyzeBranchingComplexity(source);
        result.loopComplexity = AnalyzeLoopComplexity(source);
        
        // Resource analysis
        result.uniformsUsed = CountUniforms(source);
        result.attributesUsed = CountAttributes(source);
        result.varyingsUsed = CountVaryings(source);
        
        // Generate optimization suggestions
        result.optimizationSuggestions = GenerateOptimizationSuggestions(result);
        result.performanceWarnings = DetectPerformanceIssues(source);
        result.compatibilityIssues = DetectCompatibilityIssues(source);
        
        // Calculate quality score
        result.qualityScore = CalculateQualityScore(result);
        
        return result;
    }

    ShaderAnalyzer::AnalysisResult ShaderAnalyzer::AnalyzeCompiledShader(uint32_t programId) {
        AnalysisResult result;
        
        if (programId == 0) {
            return result;
        }
        
        // Get resource counts from compiled program
        int uniformCount, attributeCount;
        glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &uniformCount);
        glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &attributeCount);
        
        result.uniformsUsed = uniformCount;
        result.attributesUsed = attributeCount;
        result.estimatedInstructions = uniformCount * 2 + attributeCount * 3 + 10; // Rough estimate
        
        // Count texture samplers
        for (int i = 0; i < uniformCount; ++i) {
            char name[256];
            int size;
            uint32_t type;
            glGetActiveUniform(programId, i, sizeof(name), nullptr, &size, &type, name);
            
            if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE || 
                type == GL_SAMPLER_3D || type == GL_SAMPLER_2D_ARRAY) {
                result.textureReads++;
            }
        }
        
        result.qualityScore = CalculateQualityScore(result);
        
        return result;
    }

    int ShaderAnalyzer::EstimateInstructionCount(const std::string& source) {
        int instructions = 0;
        
        // Count basic operations
        instructions += std::count(source.begin(), source.end(), '+');
        instructions += std::count(source.begin(), source.end(), '-');
        instructions += std::count(source.begin(), source.end(), '*');
        instructions += std::count(source.begin(), source.end(), '/');
        instructions += std::count(source.begin(), source.end(), '=');
        
        // Count function calls (rough estimate)
        std::regex functionRegex(R"(\w+\s*\()");
        auto begin = std::sregex_iterator(source.begin(), source.end(), functionRegex);
        auto end = std::sregex_iterator();
        instructions += std::distance(begin, end);
        
        // Count control flow statements
        size_t pos = 0;
        while ((pos = source.find("if", pos)) != std::string::npos) {
            instructions += 2; // Branch instruction + condition
            pos += 2;
        }
        
        pos = 0;
        while ((pos = source.find("for", pos)) != std::string::npos) {
            instructions += 5; // Loop overhead
            pos += 3;
        }
        
        return std::max(instructions, 10); // Minimum instruction count
    }

    int ShaderAnalyzer::CountTextureReads(const std::string& source) {
        int textureReads = 0;
        
        // Count texture function calls
        std::vector<std::string> textureFunctions = {
            "texture", "texture2D", "textureCube", "texture3D", 
            "textureProj", "textureLod", "textureGrad"
        };
        
        for (const auto& func : textureFunctions) {
            size_t pos = 0;
            while ((pos = source.find(func, pos)) != std::string::npos) {
                textureReads++;
                pos += func.length();
            }
        }
        
        return textureReads;
    }

    int ShaderAnalyzer::AnalyzeBranchingComplexity(const std::string& source) {
        int complexity = 0;
        
        // Count branching statements
        size_t pos = 0;
        while ((pos = source.find("if", pos)) != std::string::npos) {
            complexity++;
            pos += 2;
        }
        
        pos = 0;
        while ((pos = source.find("switch", pos)) != std::string::npos) {
            complexity += 2; // Switch statements are more complex
            pos += 6;
        }
        
        // Count nested branching (rough estimate)
        int braceDepth = 0;
        int maxDepth = 0;
        for (char c : source) {
            if (c == '{') {
                braceDepth++;
                maxDepth = std::max(maxDepth, braceDepth);
            } else if (c == '}') {
                braceDepth--;
            }
        }
        
        complexity += maxDepth; // Add nesting complexity
        
        return complexity;
    }

    int ShaderAnalyzer::AnalyzeLoopComplexity(const std::string& source) {
        int complexity = 0;
        
        // Count loop statements
        size_t pos = 0;
        while ((pos = source.find("for", pos)) != std::string::npos) {
            complexity += 2;
            pos += 3;
        }
        
        pos = 0;
        while ((pos = source.find("while", pos)) != std::string::npos) {
            complexity += 2;
            pos += 5;
        }
        
        pos = 0;
        while ((pos = source.find("do", pos)) != std::string::npos) {
            complexity += 2;
            pos += 2;
        }
        
        return complexity;
    }

    int ShaderAnalyzer::CountUniforms(const std::string& source) {
        std::regex uniformRegex(R"(uniform\s+\w+\s+\w+)");
        auto begin = std::sregex_iterator(source.begin(), source.end(), uniformRegex);
        auto end = std::sregex_iterator();
        return std::distance(begin, end);
    }

    int ShaderAnalyzer::CountAttributes(const std::string& source) {
        std::regex attributeRegex(R"((attribute|in)\s+\w+\s+\w+)");
        auto begin = std::sregex_iterator(source.begin(), source.end(), attributeRegex);
        auto end = std::sregex_iterator();
        return std::distance(begin, end);
    }

    int ShaderAnalyzer::CountVaryings(const std::string& source) {
        std::regex varyingRegex(R"((varying|out)\s+\w+\s+\w+)");
        auto begin = std::sregex_iterator(source.begin(), source.end(), varyingRegex);
        auto end = std::sregex_iterator();
        return std::distance(begin, end);
    }

    std::vector<std::string> ShaderAnalyzer::GenerateOptimizationSuggestions(const AnalysisResult& analysis) {
        std::vector<std::string> suggestions;
        
        if (analysis.estimatedInstructions > 200) {
            suggestions.push_back("High instruction count - consider simplifying calculations");
        }
        
        if (analysis.textureReads > 8) {
            suggestions.push_back("Many texture reads - consider texture atlasing or reducing samples");
        }
        
        if (analysis.branchingComplexity > 10) {
            suggestions.push_back("High branching complexity - consider reducing conditional statements");
        }
        
        if (analysis.loopComplexity > 5) {
            suggestions.push_back("Complex loops detected - consider unrolling or optimization");
        }
        
        if (analysis.uniformsUsed > 50) {
            suggestions.push_back("Many uniforms - consider using uniform buffer objects");
        }
        
        return suggestions;
    }

    std::vector<std::string> ShaderAnalyzer::DetectPerformanceIssues(const std::string& source) {
        std::vector<std::string> issues;
        
        // Check for expensive operations
        if (ContainsExpensiveOperations(source)) {
            issues.push_back("Contains expensive operations (sqrt, pow, sin, cos)");
        }
        
        if (HasRedundantCalculations(source)) {
            issues.push_back("Potential redundant calculations detected");
        }
        
        if (!UsesEfficientDataTypes(source)) {
            issues.push_back("Consider using more efficient data types (mediump, lowp)");
        }
        
        // Check for texture sampling in loops
        if (source.find("for") != std::string::npos && source.find("texture") != std::string::npos) {
            issues.push_back("Texture sampling inside loops can impact performance");
        }
        
        return issues;
    }

    std::vector<std::string> ShaderAnalyzer::DetectCompatibilityIssues(const std::string& source) {
        std::vector<std::string> issues;
        
        // Check for version compatibility
        if (source.find("#version 460") != std::string::npos) {
            issues.push_back("Requires OpenGL 4.6 - may not be compatible with older hardware");
        }
        
        // Check for deprecated functions
        if (source.find("texture2D") != std::string::npos) {
            issues.push_back("Uses deprecated texture2D function");
        }
        
        if (source.find("varying") != std::string::npos) {
            issues.push_back("Uses deprecated 'varying' keyword");
        }
        
        // Check for extensions
        if (source.find("#extension") != std::string::npos) {
            issues.push_back("Uses OpenGL extensions - verify hardware support");
        }
        
        return issues;
    }

    int ShaderAnalyzer::CalculateQualityScore(const AnalysisResult& analysis) {
        int score = 100;
        
        // Deduct points for complexity
        score -= std::min(20, analysis.estimatedInstructions / 10);
        score -= std::min(15, analysis.textureReads * 2);
        score -= std::min(15, analysis.branchingComplexity);
        score -= std::min(10, analysis.loopComplexity);
        
        // Deduct points for issues
        score -= analysis.performanceWarnings.size() * 5;
        score -= analysis.compatibilityIssues.size() * 3;
        
        return std::max(0, score);
    }

    bool ShaderAnalyzer::ContainsExpensiveOperations(const std::string& source) {
        std::vector<std::string> expensiveOps = {"sqrt", "pow", "sin", "cos", "tan", "exp", "log"};
        
        for (const auto& op : expensiveOps) {
            if (source.find(op) != std::string::npos) {
                return true;
            }
        }
        
        return false;
    }

    bool ShaderAnalyzer::HasRedundantCalculations(const std::string& source) {
        // Simple check for repeated normalize calls
        size_t normalizeCount = 0;
        size_t pos = 0;
        while ((pos = source.find("normalize", pos)) != std::string::npos) {
            normalizeCount++;
            pos += 9;
        }
        
        return normalizeCount > 3;
    }

    bool ShaderAnalyzer::UsesEfficientDataTypes(const std::string& source) {
        // Check if precision qualifiers are used
        return source.find("mediump") != std::string::npos || 
               source.find("lowp") != std::string::npos ||
               source.find("highp") != std::string::npos;
    }

}