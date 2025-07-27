#pragma once

#include "Resource/ModelValidator.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Core/Math.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <functional>

namespace GameEngine {

    /**
     * @brief Comprehensive model debugging and analysis system
     * 
     * Provides detailed statistics reporting, verbose logging, and analysis tools
     * for troubleshooting model loading and performance issues.
     */
    class ModelDebugger {
    public:
        /**
         * @brief Detailed model statistics with breakdowns
         */
        struct DetailedModelStats {
            // Basic statistics
            std::string filepath;
            std::string format;
            std::string name;
            float loadingTimeMs = 0.0f;
            
            // Hierarchy statistics
            uint32_t nodeCount = 0;
            uint32_t maxDepth = 0;
            uint32_t leafNodeCount = 0;
            uint32_t emptyNodeCount = 0;
            
            // Mesh statistics
            uint32_t meshCount = 0;
            uint32_t totalVertices = 0;
            uint32_t totalTriangles = 0;
            uint32_t minVerticesPerMesh = UINT32_MAX;
            uint32_t maxVerticesPerMesh = 0;
            uint32_t minTrianglesPerMesh = UINT32_MAX;
            uint32_t maxTrianglesPerMesh = 0;
            float avgVerticesPerMesh = 0.0f;
            float avgTrianglesPerMesh = 0.0f;
            
            // Material statistics
            uint32_t materialCount = 0;
            uint32_t textureCount = 0;
            uint32_t uniqueTextureCount = 0;
            uint32_t meshesWithoutMaterials = 0;
            
            // Animation statistics
            uint32_t animationCount = 0;
            uint32_t skeletonCount = 0;
            uint32_t skinCount = 0;
            uint32_t totalBones = 0;
            float totalAnimationDuration = 0.0f;
            
            // Memory usage breakdown
            size_t totalMemoryUsage = 0;
            size_t vertexDataMemory = 0;
            size_t indexDataMemory = 0;
            size_t textureMemory = 0;
            size_t animationMemory = 0;
            size_t nodeMemory = 0;
            
            // Geometry quality metrics
            float averageTriangleArea = 0.0f;
            float minTriangleArea = FLT_MAX;
            float maxTriangleArea = 0.0f;
            uint32_t degenerateTriangles = 0;
            uint32_t duplicateVertices = 0;
            float cacheEfficiency = 0.0f; // ACMR score
            
            // Bounding volume information
            Math::Vec3 boundingBoxMin;
            Math::Vec3 boundingBoxMax;
            Math::Vec3 boundingBoxSize;
            Math::Vec3 boundingSphereCenter;
            float boundingSphereRadius = 0.0f;
            
            // Performance indicators
            bool hasLODLevels = false;
            bool isOptimized = false;
            bool hasValidNormals = true;
            bool hasValidUVs = true;
            bool hasValidTangents = false;
            
            // Validation summary
            uint32_t validationIssues = 0;
            uint32_t criticalIssues = 0;
            uint32_t errorIssues = 0;
            uint32_t warningIssues = 0;
            uint32_t infoIssues = 0;
        };

        /**
         * @brief Mesh analysis breakdown
         */
        struct MeshAnalysis {
            std::string name;
            uint32_t vertexCount = 0;
            uint32_t triangleCount = 0;
            size_t memoryUsage = 0;
            
            // Vertex attributes
            bool hasPositions = false;
            bool hasNormals = false;
            bool hasTexCoords = false;
            bool hasTangents = false;
            bool hasColors = false;
            bool hasBoneWeights = false;
            
            // Quality metrics
            float averageTriangleArea = 0.0f;
            uint32_t degenerateTriangles = 0;
            uint32_t duplicateVertices = 0;
            float cacheEfficiency = 0.0f;
            
            // Bounding information
            Math::Vec3 boundingBoxMin;
            Math::Vec3 boundingBoxMax;
            Math::Vec3 boundingBoxSize;
            
            // Material association
            bool hasMaterial = false;
            std::string materialName;
            
            // Performance flags
            bool isOptimized = false;
            bool needsOptimization = false;
            
            // Issues found
            std::vector<std::string> issues;
            std::vector<std::string> suggestions;
        };

        /**
         * @brief Loading pipeline stage information
         */
        struct PipelineStage {
            std::string name;
            std::string description;
            std::chrono::high_resolution_clock::time_point startTime;
            std::chrono::high_resolution_clock::time_point endTime;
            float durationMs = 0.0f;
            bool success = false;
            std::string errorMessage;
            std::unordered_map<std::string, std::string> metadata;
        };

        /**
         * @brief Complete loading pipeline report
         */
        struct PipelineReport {
            std::string filepath;
            std::chrono::high_resolution_clock::time_point overallStartTime;
            std::chrono::high_resolution_clock::time_point overallEndTime;
            float totalDurationMs = 0.0f;
            bool overallSuccess = false;
            
            std::vector<PipelineStage> stages;
            std::unordered_map<std::string, std::string> globalMetadata;
            
            // Performance breakdown
            float fileIOTimeMs = 0.0f;
            float parsingTimeMs = 0.0f;
            float meshProcessingTimeMs = 0.0f;
            float materialProcessingTimeMs = 0.0f;
            float optimizationTimeMs = 0.0f;
            float validationTimeMs = 0.0f;
        };

    public:
        ModelDebugger();
        ~ModelDebugger();

        // Main analysis interface
        DetailedModelStats AnalyzeModel(std::shared_ptr<Model> model);
        DetailedModelStats AnalyzeModelFile(const std::string& filepath);
        std::vector<MeshAnalysis> AnalyzeMeshes(std::shared_ptr<Model> model);
        MeshAnalysis AnalyzeMesh(std::shared_ptr<Mesh> mesh, const std::string& name = "");

        // Statistics reporting
        std::string GenerateStatisticsReport(const DetailedModelStats& stats);
        std::string GenerateDetailedBreakdown(const DetailedModelStats& stats);
        std::string GenerateMeshAnalysisReport(const std::vector<MeshAnalysis>& analyses);
        std::string GeneratePerformanceReport(const DetailedModelStats& stats);
        std::string GenerateMemoryReport(const DetailedModelStats& stats);

        // Verbose logging control
        void EnableVerboseLogging(bool enabled = true);
        void SetLogLevel(ModelDiagnosticLogger::LogLevel level);
        void SetLogOutputFile(const std::string& filepath);
        
        // Pipeline monitoring
        void StartPipelineMonitoring(const std::string& filepath);
        void LogPipelineStage(const std::string& stageName, const std::string& description);
        void LogPipelineStageComplete(const std::string& stageName, bool success = true, const std::string& errorMessage = "");
        void LogPipelineMetadata(const std::string& key, const std::string& value);
        PipelineReport FinishPipelineMonitoring();
        std::string GeneratePipelineReport(const PipelineReport& report);

        // Issue detection and suggestions
        std::vector<std::string> DetectPerformanceIssues(const DetailedModelStats& stats);
        std::vector<std::string> DetectQualityIssues(const DetailedModelStats& stats);
        std::vector<std::string> GenerateOptimizationSuggestions(const DetailedModelStats& stats);
        std::vector<std::string> GenerateCompatibilitySuggestions(const DetailedModelStats& stats);

        // Comparison and benchmarking
        std::string CompareModels(const DetailedModelStats& stats1, const DetailedModelStats& stats2);
        std::string BenchmarkAgainstStandards(const DetailedModelStats& stats);

        // Performance profiling and optimization tools
        struct PerformanceProfile {
            std::string filepath;
            std::chrono::high_resolution_clock::time_point startTime;
            std::chrono::high_resolution_clock::time_point endTime;
            float totalLoadingTimeMs = 0.0f;
            float fileIOTimeMs = 0.0f;
            float parsingTimeMs = 0.0f;
            float meshProcessingTimeMs = 0.0f;
            float materialProcessingTimeMs = 0.0f;
            float optimizationTimeMs = 0.0f;
            float validationTimeMs = 0.0f;
            
            // Memory profiling
            size_t peakMemoryUsage = 0;
            size_t initialMemoryUsage = 0;
            size_t finalMemoryUsage = 0;
            size_t memoryLeakBytes = 0;
            
            // Performance metrics
            float verticesPerSecond = 0.0f;
            float trianglesPerSecond = 0.0f;
            float mbPerSecond = 0.0f;
            
            // Optimization suggestions
            std::vector<std::string> performanceIssues;
            std::vector<std::string> optimizationSuggestions;
            std::vector<std::string> memoryOptimizations;
        };

        struct LoadingBenchmark {
            std::string testName;
            std::vector<std::string> testFiles;
            std::vector<PerformanceProfile> profiles;
            float averageLoadingTime = 0.0f;
            float minLoadingTime = FLT_MAX;
            float maxLoadingTime = 0.0f;
            size_t totalVerticesProcessed = 0;
            size_t totalTrianglesProcessed = 0;
            size_t totalBytesProcessed = 0;
        };

        // Performance profiling methods
        void StartPerformanceProfiling(const std::string& filepath);
        void LogProfilingStage(const std::string& stageName, float durationMs);
        void LogMemoryUsage(const std::string& stage, size_t memoryBytes);
        PerformanceProfile FinishPerformanceProfiling();
        
        // Loading performance analysis
        PerformanceProfile ProfileModelLoading(const std::string& filepath);
        LoadingBenchmark BenchmarkModelLoading(const std::vector<std::string>& testFiles, const std::string& benchmarkName = "");
        std::string GeneratePerformanceReport(const PerformanceProfile& profile);
        std::string GenerateBenchmarkReport(const LoadingBenchmark& benchmark);
        
        // Memory analysis
        size_t GetCurrentMemoryUsage();
        void StartMemoryProfiling();
        void StopMemoryProfiling();
        std::vector<std::string> AnalyzeMemoryUsage(const DetailedModelStats& stats);
        std::vector<std::string> DetectMemoryLeaks(const PerformanceProfile& profile);
        
        // Optimization suggestions
        std::vector<std::string> GenerateLoadingOptimizations(const PerformanceProfile& profile);
        std::vector<std::string> GenerateMemoryOptimizations(const DetailedModelStats& stats);
        std::vector<std::string> GenerateCacheOptimizations(const DetailedModelStats& stats);
        
        // Export and reporting
        bool SaveAnalysisToFile(const DetailedModelStats& stats, const std::string& outputPath);
        bool SavePipelineReportToFile(const PipelineReport& report, const std::string& outputPath);
        bool SavePerformanceProfile(const PerformanceProfile& profile, const std::string& outputPath);
        bool SaveBenchmarkResults(const LoadingBenchmark& benchmark, const std::string& outputPath);
        bool ExportToJSON(const DetailedModelStats& stats, const std::string& outputPath);
        bool ExportToCSV(const std::vector<DetailedModelStats>& statsCollection, const std::string& outputPath);

        // Configuration
        void SetPerformanceThresholds(uint32_t maxVertices, uint32_t maxTriangles, float maxMemoryMB);
        void SetQualityThresholds(float minTriangleArea, float maxCacheThreshold);
        void EnableDetailedMeshAnalysis(bool enabled = true);
        void EnableMemoryProfiling(bool enabled = true);

    private:
        bool m_verboseLogging = false;
        bool m_detailedMeshAnalysis = true;
        
        // Performance thresholds
        uint32_t m_maxVertices = 100000;
        uint32_t m_maxTriangles = 200000;
        float m_maxMemoryMB = 100.0f;
        
        // Quality thresholds
        float m_minTriangleArea = 1e-6f;
        float m_maxCacheThreshold = 2.0f;
        
        // Pipeline monitoring state
        bool m_pipelineMonitoring = false;
        PipelineReport m_currentPipeline;
        std::unordered_map<std::string, PipelineStage> m_activeStages;
        
        // Performance profiling state
        bool m_performanceProfiling = false;
        PerformanceProfile m_currentProfile;
        std::unordered_map<std::string, float> m_stageTimings;
        
        // Memory profiling state
        bool m_memoryProfiling = false;
        size_t m_baselineMemory = 0;
        std::vector<std::pair<std::string, size_t>> m_memorySnapshots;
        
        // Validator instance
        std::unique_ptr<ModelValidator> m_validator;

        // Internal analysis methods
        void AnalyzeHierarchy(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzeMeshStatistics(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzeMaterialStatistics(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzeAnimationStatistics(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzeMemoryUsage(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzeGeometryQuality(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzeBoundingVolumes(std::shared_ptr<Model> model, DetailedModelStats& stats);
        void AnalyzePerformanceIndicators(std::shared_ptr<Model> model, DetailedModelStats& stats);
        
        // Mesh-specific analysis
        void AnalyzeMeshGeometry(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis);
        void AnalyzeMeshQuality(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis);
        void AnalyzeMeshPerformance(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis);
        void DetectMeshIssues(std::shared_ptr<Mesh> mesh, MeshAnalysis& analysis);
        
        // Utility methods
        void LogVerbose(const std::string& message, const std::string& component = "ModelDebugger");
        std::string FormatMemorySize(size_t bytes);
        std::string FormatDuration(float milliseconds);
        std::string FormatPercentage(float value);
        float CalculateTriangleArea(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3);
        bool IsTriangleDegenerate(const Math::Vec3& v1, const Math::Vec3& v2, const Math::Vec3& v3, float epsilon = 1e-6f);
        uint32_t CountDuplicateVertices(std::shared_ptr<Mesh> mesh, float epsilon = 1e-6f);
        float CalculateCacheEfficiency(std::shared_ptr<Mesh> mesh);
        uint32_t CalculateHierarchyDepth(std::shared_ptr<ModelNode> node, uint32_t currentDepth = 0);
        void CountNodeTypes(std::shared_ptr<ModelNode> node, uint32_t& leafNodes, uint32_t& emptyNodes);
    };

} // namespace GameEngine