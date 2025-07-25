#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/BoundingVolumes.h"
#include "Core/Math.h"
#include <vector>
#include <memory>
#include <functional>

namespace GameEngine {
    
    // Mesh analysis data structure
    struct MeshAnalysis {
        uint32_t vertexCount = 0;
        uint32_t triangleCount = 0;
        uint32_t duplicateVertices = 0;
        uint32_t degenerateTriangles = 0;
        float averageTriangleArea = 0.0f;
        float minTriangleArea = 0.0f;
        float maxTriangleArea = 0.0f;
        BoundingBox bounds;
        bool hasNormals = false;
        bool hasTangents = false;
        bool hasTextureCoords = false;
        bool hasColors = false;
        bool hasBoneWeights = false;
        float cacheEfficiency = 0.0f;  // ACMR (Average Cache Miss Ratio) score
        float overdrawRatio = 0.0f;
        size_t memoryUsage = 0;
        
        // Quality metrics
        float minTriangleQuality = 0.0f;  // Worst triangle quality (0-1)
        float averageTriangleQuality = 0.0f;  // Average triangle quality
        uint32_t thinTriangles = 0;  // Triangles with aspect ratio > 10:1
        uint32_t smallTriangles = 0;  // Triangles smaller than threshold
    };
    
    // Optimization statistics for performance reporting
    struct MeshOptimizationStats {
        // Before optimization
        uint32_t originalVertexCount = 0;
        uint32_t originalTriangleCount = 0;
        float originalACMR = 0.0f;
        float originalATVR = 0.0f;  // Average Transform to Vertex Ratio
        size_t originalMemoryUsage = 0;
        
        // After optimization
        uint32_t optimizedVertexCount = 0;
        uint32_t optimizedTriangleCount = 0;
        float optimizedACMR = 0.0f;
        float optimizedATVR = 0.0f;
        size_t optimizedMemoryUsage = 0;
        
        // Performance improvements
        float vertexReduction = 0.0f;  // Percentage reduction
        float triangleReduction = 0.0f;
        float cacheImprovement = 0.0f;
        float memoryReduction = 0.0f;
        
        // Timing information
        float optimizationTimeMs = 0.0f;
        
        void CalculateImprovements();
        std::string GetSummary() const;
    };
    
    // LOD generation configuration
    struct LODGenerationConfig {
        std::vector<float> simplificationRatios = {0.75f, 0.5f, 0.25f, 0.1f};  // LOD levels
        float maxError = 0.01f;  // Maximum allowed geometric error
        bool preserveBoundaries = true;  // Preserve mesh boundaries
        bool preserveUVSeams = true;  // Preserve UV coordinate seams
        bool preserveNormalSeams = true;  // Preserve normal discontinuities
        float aggressiveness = 7.0f;  // Simplification aggressiveness (0-10)
        uint32_t maxIterations = 100;  // Maximum simplification iterations
        
        // Distance-based LOD selection
        std::vector<float> lodDistances = {50.0f, 100.0f, 200.0f, 500.0f};  // Switch distances
        bool enableDistanceBasedSelection = true;
    };
    
    /**
     * @brief Advanced mesh optimization and LOD generation system
     * 
     * Provides industry-standard mesh optimization algorithms including:
     * - Tom Forsyth's vertex cache optimization
     * - Vertex fetch optimization
     * - Overdraw reduction
     * - Mesh simplification with configurable quality levels
     * - Automatic LOD generation with distance-based selection
     * - Comprehensive mesh analysis and validation
     */
    class MeshOptimizer {
    public:
        // Vertex cache optimization using industry-standard algorithms
        static void OptimizeVertexCache(Mesh& mesh);
        static void OptimizeVertexFetch(Mesh& mesh);
        static std::vector<uint32_t> OptimizeIndices(const std::vector<uint32_t>& indices, size_t vertexCount);
        
        // Overdraw optimization
        static void OptimizeOverdraw(Mesh& mesh, float threshold = 1.05f);
        static std::vector<uint32_t> OptimizeOverdrawIndices(const std::vector<uint32_t>& indices,
                                                             const std::vector<Vertex>& vertices,
                                                             float threshold);
        
        // Mesh simplification with configurable quality levels
        static std::shared_ptr<Mesh> Simplify(const Mesh& mesh, float ratio);
        static std::shared_ptr<Mesh> SimplifyToTargetError(const Mesh& mesh, float maxError);
        static std::shared_ptr<Mesh> SimplifyToTriangleCount(const Mesh& mesh, uint32_t targetTriangles);
        
        // Automatic LOD generation with distance-based selection
        static std::vector<std::shared_ptr<Mesh>> GenerateLODChain(const Mesh& mesh, const std::vector<float>& ratios);
        static std::vector<std::shared_ptr<Mesh>> GenerateAutomaticLODs(const Mesh& mesh, uint32_t lodCount);
        static std::shared_ptr<Mesh> SelectLOD(const std::vector<std::shared_ptr<Mesh>>& lodChain, 
                                               float distance, const LODGenerationConfig& config);
        
        // Vertex processing
        static void RemoveDuplicateVertices(Mesh& mesh, float epsilon = 0.0001f);
        static void GenerateNormals(Mesh& mesh, bool smooth = true);
        static void GenerateTangents(Mesh& mesh);
        static void FlipNormals(Mesh& mesh);
        
        // Mesh analysis with triangle quality and vertex statistics
        static MeshAnalysis AnalyzeMesh(const Mesh& mesh);
        static bool ValidateMesh(const Mesh& mesh);
        static std::vector<std::string> GetMeshIssues(const Mesh& mesh);
        
        // Mesh optimization statistics and performance reporting
        static MeshOptimizationStats GetOptimizationStats(const Mesh& originalMesh, const Mesh& optimizedMesh);
        static MeshOptimizationStats OptimizeWithStats(Mesh& mesh, bool optimizeCache = true, 
                                                       bool optimizeFetch = true, bool optimizeOverdraw = true);
        
        // Advanced optimization pipeline
        static void OptimizeForRendering(Mesh& mesh, const LODGenerationConfig& config = LODGenerationConfig());
        static std::vector<std::shared_ptr<Mesh>> CreateOptimizedLODChain(const Mesh& mesh, 
                                                                          const LODGenerationConfig& config = LODGenerationConfig());
        
        // Configuration
        static void SetCacheSize(uint32_t cacheSize) { s_cacheSize = cacheSize; }
        static uint32_t GetCacheSize() { return s_cacheSize; }
        static void SetVerboseLogging(bool enabled) { s_verboseLogging = enabled; }
        
        // Public helper methods for testing and external use
        static float CalculateACMR(const std::vector<uint32_t>& indices, size_t cacheSize = 32);
        static float CalculateATVR(const std::vector<uint32_t>& indices, size_t vertexCount);
        static float CalculateOverdrawRatio(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices);
        
    private:
        // Configuration
        static uint32_t s_cacheSize;  // GPU vertex cache size (typically 32)
        static bool s_verboseLogging;
        
        // Helper methods for optimization algorithms
        static void ReorderIndicesForCache(std::vector<uint32_t>& indices, size_t vertexCount);
        static void ReorderVerticesForFetch(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
        
        // Triangle quality analysis
        static float CalculateTriangleQuality(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2);
        static float CalculateTriangleAspectRatio(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2);
        static bool IsTriangleThin(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2, float threshold = 10.0f);
        static bool IsTriangleSmall(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2, float threshold = 0.0001f);
        
        // Simplification algorithms
        static std::shared_ptr<Mesh> SimplifyQuadricErrorMetrics(const Mesh& mesh, float ratio, float maxError);
        static std::shared_ptr<Mesh> SimplifyEdgeCollapse(const Mesh& mesh, uint32_t targetTriangles);
        
        // Vertex cache simulation for Tom Forsyth's algorithm
        struct VertexCacheSimulator {
            std::vector<uint32_t> cache;
            uint32_t cacheSize;
            uint32_t cacheMisses;
            uint32_t totalAccesses;
            
            VertexCacheSimulator(uint32_t size) : cacheSize(size), cacheMisses(0), totalAccesses(0) {}
            
            bool AccessVertex(uint32_t vertex);
            float GetCacheMissRatio() const;
            void Reset();
        };
        
        // Vertex scoring for Tom Forsyth's algorithm
        static float CalculateVertexScore(uint32_t vertex, const std::vector<uint32_t>& cache, 
                                         const std::vector<uint32_t>& valence, uint32_t cacheSize);
        
        // Edge collapse data structures for simplification
        struct Edge {
            uint32_t v0, v1;
            float cost;
            Math::Mat4 quadric;
            
            bool operator<(const Edge& other) const { return cost < other.cost; }
        };
        
        struct VertexQuadric {
            Math::Mat4 quadric;
            std::vector<uint32_t> adjacentTriangles;
            std::vector<uint32_t> adjacentVertices;
        };
        
        // Utility functions
        static Math::Mat4 CalculateQuadricMatrix(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2);
        static float CalculateEdgeCollapseError(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Mat4& quadric);
        static bool CanCollapseEdge(uint32_t v0, uint32_t v1, const std::vector<Vertex>& vertices, 
                                   const std::vector<uint32_t>& indices);
    };
    
} // namespace GameEngine