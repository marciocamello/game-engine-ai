#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>

namespace GameEngine {

    /**
     * Shader performance statistics
     * Requirements: 6.2, 6.3, 6.5
     */
    struct ShaderPerformanceStats {
        // Compilation metrics
        double compilationTimeMs = 0.0;
        double linkingTimeMs = 0.0;
        size_t compiledShaderCount = 0;
        
        // Runtime metrics
        double averageFrameTimeMs = 0.0;
        double maxFrameTimeMs = 0.0;
        double minFrameTimeMs = 999999.0;
        uint64_t totalDrawCalls = 0;
        uint64_t totalFrames = 0;
        
        // Memory usage
        size_t gpuMemoryUsageBytes = 0;
        size_t uniformBufferSize = 0;
        size_t textureMemoryUsage = 0;
        
        // Shader complexity metrics
        int uniformCount = 0;
        int attributeCount = 0;
        int textureUnitCount = 0;
        int instructionCount = 0; // Estimated
        
        // Error statistics
        int compilationErrors = 0;
        int linkingErrors = 0;
        int runtimeErrors = 0;
        
        void Reset() {
            compilationTimeMs = 0.0;
            linkingTimeMs = 0.0;
            compiledShaderCount = 0;
            averageFrameTimeMs = 0.0;
            maxFrameTimeMs = 0.0;
            minFrameTimeMs = 999999.0;
            totalDrawCalls = 0;
            totalFrames = 0;
            gpuMemoryUsageBytes = 0;
            uniformBufferSize = 0;
            textureMemoryUsage = 0;
            uniformCount = 0;
            attributeCount = 0;
            textureUnitCount = 0;
            instructionCount = 0;
            compilationErrors = 0;
            linkingErrors = 0;
            runtimeErrors = 0;
        }
    };

    /**
     * Shader resource usage information
     */
    struct ShaderResourceInfo {
        std::string name;
        uint32_t location;
        uint32_t type;
        int size;
        bool isActive;
        
        ShaderResourceInfo() = default;
        ShaderResourceInfo(const std::string& n, uint32_t loc, uint32_t t, int s, bool active)
            : name(n), location(loc), type(t), size(s), isActive(active) {}
    };

    /**
     * Comprehensive shader profiling and debugging system
     * Requirements: 6.2, 6.3, 6.5
     */
    class ShaderProfiler {
    public:
        static ShaderProfiler& GetInstance();
        
        // Profiling control
        void StartProfiling();
        void StopProfiling();
        void ResetStats();
        bool IsProfilingEnabled() const { return m_profilingEnabled; }
        
        // Shader registration and tracking
        void RegisterShader(const std::string& name, uint32_t programId);
        void UnregisterShader(const std::string& name);
        
        // Performance measurement
        void BeginShaderTiming(const std::string& shaderName);
        void EndShaderTiming(const std::string& shaderName);
        void RecordCompilationTime(const std::string& shaderName, double timeMs);
        void RecordLinkingTime(const std::string& shaderName, double timeMs);
        void RecordDrawCall(const std::string& shaderName);
        void RecordFrameTime(const std::string& shaderName, double frameTimeMs);
        
        // Memory tracking
        void UpdateGPUMemoryUsage(const std::string& shaderName, size_t bytes);
        void UpdateUniformBufferSize(const std::string& shaderName, size_t bytes);
        void UpdateTextureMemoryUsage(const std::string& shaderName, size_t bytes);
        
        // Resource introspection
        std::vector<ShaderResourceInfo> GetShaderUniforms(uint32_t programId) const;
        std::vector<ShaderResourceInfo> GetShaderAttributes(uint32_t programId) const;
        std::vector<ShaderResourceInfo> GetShaderStorageBuffers(uint32_t programId) const;
        int EstimateInstructionCount(uint32_t programId) const;
        
        // Statistics retrieval
        ShaderPerformanceStats GetShaderStats(const std::string& shaderName) const;
        ShaderPerformanceStats GetGlobalStats() const;
        std::vector<std::string> GetProfiledShaders() const;
        
        // Performance analysis
        std::vector<std::string> GetPerformanceBottlenecks() const;
        std::vector<std::string> GetMemoryHogs() const;
        std::vector<std::string> GetOptimizationSuggestions(const std::string& shaderName) const;
        
        // Debug information
        std::string GeneratePerformanceReport() const;
        std::string GenerateMemoryReport() const;
        void DumpShaderInfo(const std::string& shaderName) const;
        
        // GPU memory tracking utilities
        size_t GetTotalGPUMemoryUsage() const;
        size_t GetShaderMemoryUsage(const std::string& shaderName) const;
        float GetMemoryUsagePercentage() const; // Requires GPU memory query support
        
        // Performance thresholds
        void SetPerformanceThresholds(double maxFrameTimeMs, size_t maxMemoryMB);
        bool IsShaderPerformanceAcceptable(const std::string& shaderName) const;
        
    private:
        ShaderProfiler() = default;
        ~ShaderProfiler() = default;
        ShaderProfiler(const ShaderProfiler&) = delete;
        ShaderProfiler& operator=(const ShaderProfiler&) = delete;
        
        bool m_profilingEnabled = false;
        std::unordered_map<std::string, ShaderPerformanceStats> m_shaderStats;
        std::unordered_map<std::string, uint32_t> m_shaderPrograms;
        std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_timingStart;
        
        // Performance thresholds
        double m_maxFrameTimeMs = 16.67; // 60 FPS
        size_t m_maxMemoryMB = 512;
        
        // Helper methods
        void UpdateShaderComplexity(const std::string& shaderName, uint32_t programId);
        std::string FormatMemorySize(size_t bytes) const;
        std::string FormatTime(double timeMs) const;
        double CalculateAverageFrameTime(const ShaderPerformanceStats& stats) const;
    };

    /**
     * GPU memory tracker for shader resources
     * Requirements: 6.2, 6.3, 6.5
     */
    class GPUMemoryTracker {
    public:
        static GPUMemoryTracker& GetInstance();
        
        // Memory tracking
        void TrackShaderMemory(const std::string& shaderName, size_t bytes);
        void TrackTextureMemory(uint32_t textureId, size_t bytes);
        void TrackBufferMemory(uint32_t bufferId, size_t bytes);
        void ReleaseShaderMemory(const std::string& shaderName);
        void ReleaseTextureMemory(uint32_t textureId);
        void ReleaseBufferMemory(uint32_t bufferId);
        
        // Memory queries
        size_t GetTotalMemoryUsage() const;
        size_t GetShaderMemoryUsage() const;
        size_t GetTextureMemoryUsage() const;
        size_t GetBufferMemoryUsage() const;
        
        // Memory analysis
        std::vector<std::pair<std::string, size_t>> GetTopMemoryConsumers() const;
        bool IsMemoryUsageHigh() const;
        std::vector<std::string> GetMemoryOptimizationSuggestions() const;
        
        // Memory reporting
        std::string GenerateMemoryReport() const;
        void DumpMemoryUsage() const;
        
        // Memory limits and warnings
        void SetMemoryLimit(size_t limitBytes);
        bool IsMemoryLimitExceeded() const;
        float GetMemoryUsagePercentage() const;
        
    private:
        GPUMemoryTracker() = default;
        ~GPUMemoryTracker() = default;
        GPUMemoryTracker(const GPUMemoryTracker&) = delete;
        GPUMemoryTracker& operator=(const GPUMemoryTracker&) = delete;
        
        std::unordered_map<std::string, size_t> m_shaderMemory;
        std::unordered_map<uint32_t, size_t> m_textureMemory;
        std::unordered_map<uint32_t, size_t> m_bufferMemory;
        
        size_t m_memoryLimit = 1024 * 1024 * 1024; // 1GB default limit
        
        std::string FormatMemorySize(size_t bytes) const;
    };

    /**
     * Shader validation and analysis tools
     * Requirements: 6.2, 6.3, 6.5
     */
    class ShaderAnalyzer {
    public:
        struct AnalysisResult {
            // Performance metrics
            int estimatedInstructions = 0;
            int textureReads = 0;
            int branchingComplexity = 0;
            int loopComplexity = 0;
            
            // Resource usage
            int uniformsUsed = 0;
            int attributesUsed = 0;
            int varyingsUsed = 0;
            
            // Optimization opportunities
            std::vector<std::string> optimizationSuggestions;
            std::vector<std::string> performanceWarnings;
            std::vector<std::string> compatibilityIssues;
            
            // Quality score (0-100)
            int qualityScore = 100;
        };
        
        // Static analysis methods
        static AnalysisResult AnalyzeShaderSource(const std::string& source, const std::string& shaderType);
        static AnalysisResult AnalyzeCompiledShader(uint32_t programId);
        
        // Performance analysis
        static int EstimateInstructionCount(const std::string& source);
        static int CountTextureReads(const std::string& source);
        static int AnalyzeBranchingComplexity(const std::string& source);
        static int AnalyzeLoopComplexity(const std::string& source);
        
        // Resource analysis
        static int CountUniforms(const std::string& source);
        static int CountAttributes(const std::string& source);
        static int CountVaryings(const std::string& source);
        
        // Optimization suggestions
        static std::vector<std::string> GenerateOptimizationSuggestions(const AnalysisResult& analysis);
        static std::vector<std::string> DetectPerformanceIssues(const std::string& source);
        static std::vector<std::string> DetectCompatibilityIssues(const std::string& source);
        
        // Quality scoring
        static int CalculateQualityScore(const AnalysisResult& analysis);
        
    private:
        // Analysis helper methods
        static bool ContainsExpensiveOperations(const std::string& source);
        static bool HasRedundantCalculations(const std::string& source);
        static bool UsesEfficientDataTypes(const std::string& source);
    };

}