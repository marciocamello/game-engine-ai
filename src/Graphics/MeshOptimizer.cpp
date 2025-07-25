#include "Graphics/MeshOptimizer.h"
#include "Core/Logger.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <cmath>
#include <cfloat>

namespace GameEngine {
    
    // Static member initialization
    uint32_t MeshOptimizer::s_cacheSize = 32;  // Typical GPU vertex cache size
    bool MeshOptimizer::s_verboseLogging = false;
    
    // MeshOptimizationStats implementation
    void MeshOptimizationStats::CalculateImprovements() {
        if (originalVertexCount > 0) {
            vertexReduction = ((float)(originalVertexCount - optimizedVertexCount) / originalVertexCount) * 100.0f;
        }
        if (originalTriangleCount > 0) {
            triangleReduction = ((float)(originalTriangleCount - optimizedTriangleCount) / originalTriangleCount) * 100.0f;
        }
        if (originalACMR > 0) {
            cacheImprovement = ((originalACMR - optimizedACMR) / originalACMR) * 100.0f;
        }
        if (originalMemoryUsage > 0) {
            memoryReduction = ((float)(originalMemoryUsage - optimizedMemoryUsage) / originalMemoryUsage) * 100.0f;
        }
    }
    
    std::string MeshOptimizationStats::GetSummary() const {
        std::string summary = "Mesh Optimization Results:\n";
        summary += "  Vertices: " + std::to_string(originalVertexCount) + " -> " + std::to_string(optimizedVertexCount);
        summary += " (" + std::to_string(vertexReduction) + "% reduction)\n";
        summary += "  Triangles: " + std::to_string(originalTriangleCount) + " -> " + std::to_string(optimizedTriangleCount);
        summary += " (" + std::to_string(triangleReduction) + "% reduction)\n";
        summary += "  ACMR: " + std::to_string(originalACMR) + " -> " + std::to_string(optimizedACMR);
        summary += " (" + std::to_string(cacheImprovement) + "% improvement)\n";
        summary += "  Memory: " + std::to_string(originalMemoryUsage) + " -> " + std::to_string(optimizedMemoryUsage);
        summary += " (" + std::to_string(memoryReduction) + "% reduction)\n";
        summary += "  Optimization time: " + std::to_string(optimizationTimeMs) + "ms";
        return summary;
    }
    
    // Vertex cache optimization using Tom Forsyth's algorithm
    void MeshOptimizer::OptimizeVertexCache(Mesh& mesh) {
        auto indices = mesh.GetIndices();
        if (indices.empty() || indices.size() < 3) {
            if (s_verboseLogging) {
                LOG_WARNING("Cannot optimize vertex cache: insufficient indices");
            }
            return;
        }
        
        if (s_verboseLogging) {
            LOG_INFO("Optimizing vertex cache using Tom Forsyth's algorithm...");
        }
        
        auto optimizedIndices = OptimizeIndices(indices, mesh.GetVertexCount());
        
        // Create a new mesh with optimized indices
        auto vertices = mesh.GetVertices();
        mesh.SetIndices(optimizedIndices);
        
        if (s_verboseLogging) {
            float originalACMR = CalculateACMR(indices, s_cacheSize);
            float optimizedACMR = CalculateACMR(optimizedIndices, s_cacheSize);
            LOG_INFO("Vertex cache optimization completed. ACMR: " + std::to_string(originalACMR) + 
                    " -> " + std::to_string(optimizedACMR));
        }
    }
    
    void MeshOptimizer::OptimizeVertexFetch(Mesh& mesh) {
        auto vertices = mesh.GetVertices();
        auto indices = mesh.GetIndices();
        
        if (vertices.empty() || indices.empty()) {
            if (s_verboseLogging) {
                LOG_WARNING("Cannot optimize vertex fetch: insufficient data");
            }
            return;
        }
        
        if (s_verboseLogging) {
            LOG_INFO("Optimizing vertex fetch order...");
        }
        
        ReorderVerticesForFetch(vertices, indices);
        
        mesh.SetVertices(vertices);
        mesh.SetIndices(indices);
        
        if (s_verboseLogging) {
            LOG_INFO("Vertex fetch optimization completed");
        }
    }
    
    std::vector<uint32_t> MeshOptimizer::OptimizeIndices(const std::vector<uint32_t>& indices, size_t vertexCount) {
        if (indices.empty() || indices.size() < 3) {
            return indices;
        }
        
        // Tom Forsyth's vertex cache optimization algorithm
        const float CACHE_DECAY_POWER = 1.5f;
        const float LAST_TRI_SCORE = 0.75f;
        const float VALENCE_BOOST_SCALE = 2.0f;
        const float VALENCE_BOOST_POWER = 0.5f;
        
        uint32_t numTriangles = static_cast<uint32_t>(indices.size() / 3);
        
        // Build adjacency information
        std::vector<std::vector<uint32_t>> vertexTriangles(vertexCount);
        std::vector<bool> triangleAdded(numTriangles, false);
        
        for (uint32_t i = 0; i < numTriangles; ++i) {
            uint32_t i0 = indices[i * 3];
            uint32_t i1 = indices[i * 3 + 1];
            uint32_t i2 = indices[i * 3 + 2];
            
            if (i0 < vertexCount) vertexTriangles[i0].push_back(i);
            if (i1 < vertexCount) vertexTriangles[i1].push_back(i);
            if (i2 < vertexCount) vertexTriangles[i2].push_back(i);
        }
        
        // Vertex cache simulation
        std::vector<uint32_t> cache;
        std::vector<uint32_t> vertexValence(vertexCount);
        
        // Initialize vertex valence
        for (size_t i = 0; i < vertexCount; ++i) {
            vertexValence[i] = static_cast<uint32_t>(vertexTriangles[i].size());
        }
        
        // Calculate vertex scores
        auto calculateVertexScore = [&](uint32_t vertex) -> float {
            if (vertexValence[vertex] == 0) return -1.0f;
            
            float score = 0.0f;
            
            // Cache position score
            auto cacheIt = std::find(cache.begin(), cache.end(), vertex);
            if (cacheIt != cache.end()) {
                uint32_t cachePos = static_cast<uint32_t>(std::distance(cache.begin(), cacheIt));
                if (cachePos < 3) {
                    score = LAST_TRI_SCORE;
                } else {
                    const float scaler = 1.0f / (s_cacheSize - 3);
                    score = 1.0f - (cachePos - 3) * scaler;
                    score = std::pow(score, CACHE_DECAY_POWER);
                }
            }
            
            // Valence score
            float valenceScore = std::pow(static_cast<float>(vertexValence[vertex]), -VALENCE_BOOST_POWER);
            score += VALENCE_BOOST_SCALE * valenceScore;
            
            return score;
        };
        
        // Optimize triangle order
        std::vector<uint32_t> newIndices;
        newIndices.reserve(indices.size());
        
        for (uint32_t addedTriangles = 0; addedTriangles < numTriangles; ++addedTriangles) {
            // Find best triangle to add next
            uint32_t bestTriangle = 0;
            float bestScore = -1.0f;
            
            for (uint32_t i = 0; i < numTriangles; ++i) {
                if (triangleAdded[i]) continue;
                
                uint32_t i0 = indices[i * 3];
                uint32_t i1 = indices[i * 3 + 1];
                uint32_t i2 = indices[i * 3 + 2];
                
                float score = 0.0f;
                if (i0 < vertexCount) score += calculateVertexScore(i0);
                if (i1 < vertexCount) score += calculateVertexScore(i1);
                if (i2 < vertexCount) score += calculateVertexScore(i2);
                
                if (score > bestScore) {
                    bestScore = score;
                    bestTriangle = i;
                }
            }
            
            // Add best triangle
            triangleAdded[bestTriangle] = true;
            uint32_t i0 = indices[bestTriangle * 3];
            uint32_t i1 = indices[bestTriangle * 3 + 1];
            uint32_t i2 = indices[bestTriangle * 3 + 2];
            
            newIndices.push_back(i0);
            newIndices.push_back(i1);
            newIndices.push_back(i2);
            
            // Update cache
            auto updateCache = [&](uint32_t vertex) {
                if (vertex >= vertexCount) return;
                
                // Remove vertex from cache if present
                cache.erase(std::remove(cache.begin(), cache.end(), vertex), cache.end());
                // Add to front
                cache.insert(cache.begin(), vertex);
                // Limit cache size
                if (cache.size() > s_cacheSize) {
                    cache.resize(s_cacheSize);
                }
            };
            
            updateCache(i0);
            updateCache(i1);
            updateCache(i2);
            
            // Update vertex valences
            auto updateVertex = [&](uint32_t vertex) {
                if (vertex >= vertexCount) return;
                
                // Remove this triangle from vertex's triangle list
                auto& triangles = vertexTriangles[vertex];
                triangles.erase(std::remove(triangles.begin(), triangles.end(), bestTriangle), triangles.end());
                vertexValence[vertex] = static_cast<uint32_t>(triangles.size());
            };
            
            updateVertex(i0);
            updateVertex(i1);
            updateVertex(i2);
        }
        
        return newIndices;
    }
    
    // Overdraw optimization
    void MeshOptimizer::OptimizeOverdraw(Mesh& mesh, float threshold) {
        auto indices = mesh.GetIndices();
        auto vertices = mesh.GetVertices();
        
        if (indices.empty() || vertices.empty() || indices.size() < 3) {
            if (s_verboseLogging) {
                LOG_WARNING("Cannot optimize overdraw: insufficient data");
            }
            return;
        }
        
        if (s_verboseLogging) {
            LOG_INFO("Optimizing overdraw with threshold: " + std::to_string(threshold));
        }
        
        auto optimizedIndices = OptimizeOverdrawIndices(indices, vertices, threshold);
        mesh.SetIndices(optimizedIndices);
        
        if (s_verboseLogging) {
            LOG_INFO("Overdraw optimization completed");
        }
    }
    
    std::vector<uint32_t> MeshOptimizer::OptimizeOverdrawIndices(const std::vector<uint32_t>& indices,
                                                                 const std::vector<Vertex>& vertices,
                                                                 float threshold) {
        struct Triangle {
            uint32_t indices[3];
            float depth;
            Math::Vec3 normal;
        };
        
        std::vector<Triangle> triangles;
        
        // Calculate triangle depths and normals
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 < indices.size()) {
                Triangle tri;
                tri.indices[0] = indices[i];
                tri.indices[1] = indices[i + 1];
                tri.indices[2] = indices[i + 2];
                
                if (tri.indices[0] < vertices.size() && 
                    tri.indices[1] < vertices.size() && 
                    tri.indices[2] < vertices.size()) {
                    
                    const Math::Vec3& v0 = vertices[tri.indices[0]].position;
                    const Math::Vec3& v1 = vertices[tri.indices[1]].position;
                    const Math::Vec3& v2 = vertices[tri.indices[2]].position;
                    
                    // Calculate centroid depth
                    tri.depth = (v0.z + v1.z + v2.z) / 3.0f;
                    
                    // Calculate normal
                    Math::Vec3 edge1 = v1 - v0;
                    Math::Vec3 edge2 = v2 - v0;
                    tri.normal = glm::normalize(glm::cross(edge1, edge2));
                    
                    triangles.push_back(tri);
                }
            }
        }
        
        // Sort triangles by depth (back-to-front for better overdraw)
        std::sort(triangles.begin(), triangles.end(),
            [](const Triangle& a, const Triangle& b) {
                return a.depth > b.depth; // Back-to-front
            });
        
        // Rebuild indices
        std::vector<uint32_t> newIndices;
        newIndices.reserve(triangles.size() * 3);
        
        for (const auto& tri : triangles) {
            newIndices.push_back(tri.indices[0]);
            newIndices.push_back(tri.indices[1]);
            newIndices.push_back(tri.indices[2]);
        }
        
        return newIndices;
    }
    
    // Mesh simplification
    std::shared_ptr<Mesh> MeshOptimizer::Simplify(const Mesh& mesh, float ratio) {
        if (ratio >= 1.0f) {
            // No simplification needed, return copy
            auto newMesh = std::make_shared<Mesh>();
            newMesh->SetVertices(mesh.GetVertices());
            newMesh->SetIndices(mesh.GetIndices());
            return newMesh;
        }
        
        if (s_verboseLogging) {
            LOG_INFO("Simplifying mesh with ratio: " + std::to_string(ratio));
        }
        
        return SimplifyQuadricErrorMetrics(mesh, ratio, 0.01f);
    }
    
    std::shared_ptr<Mesh> MeshOptimizer::SimplifyToTargetError(const Mesh& mesh, float maxError) {
        if (s_verboseLogging) {
            LOG_INFO("Simplifying mesh to target error: " + std::to_string(maxError));
        }
        
        return SimplifyQuadricErrorMetrics(mesh, 0.1f, maxError);
    }
    
    std::shared_ptr<Mesh> MeshOptimizer::SimplifyToTriangleCount(const Mesh& mesh, uint32_t targetTriangles) {
        uint32_t currentTriangles = mesh.GetTriangleCount();
        if (targetTriangles >= currentTriangles) {
            // No simplification needed
            auto newMesh = std::make_shared<Mesh>();
            newMesh->SetVertices(mesh.GetVertices());
            newMesh->SetIndices(mesh.GetIndices());
            return newMesh;
        }
        
        float ratio = static_cast<float>(targetTriangles) / currentTriangles;
        
        if (s_verboseLogging) {
            LOG_INFO("Simplifying mesh to " + std::to_string(targetTriangles) + " triangles (ratio: " + std::to_string(ratio) + ")");
        }
        
        return SimplifyEdgeCollapse(mesh, targetTriangles);
    }
    
    // LOD generation
    std::vector<std::shared_ptr<Mesh>> MeshOptimizer::GenerateLODChain(const Mesh& mesh, const std::vector<float>& ratios) {
        std::vector<std::shared_ptr<Mesh>> lodChain;
        
        // Add original mesh as LOD 0
        auto originalMesh = std::make_shared<Mesh>();
        originalMesh->SetVertices(mesh.GetVertices());
        originalMesh->SetIndices(mesh.GetIndices());
        lodChain.push_back(originalMesh);
        
        // Generate simplified LODs
        for (float ratio : ratios) {
            if (ratio > 0.0f && ratio < 1.0f) {
                auto lodMesh = Simplify(mesh, ratio);
                if (lodMesh && lodMesh->GetTriangleCount() > 0) {
                    lodChain.push_back(lodMesh);
                }
            }
        }
        
        if (s_verboseLogging) {
            LOG_INFO("Generated LOD chain with " + std::to_string(lodChain.size()) + " levels");
        }
        
        return lodChain;
    }
    
    std::vector<std::shared_ptr<Mesh>> MeshOptimizer::GenerateAutomaticLODs(const Mesh& mesh, uint32_t lodCount) {
        if (lodCount == 0) return {};
        
        std::vector<float> ratios;
        for (uint32_t i = 1; i < lodCount; ++i) {
            float ratio = 1.0f - (static_cast<float>(i) / lodCount);
            ratios.push_back(ratio);
        }
        
        return GenerateLODChain(mesh, ratios);
    }
    
    std::shared_ptr<Mesh> MeshOptimizer::SelectLOD(const std::vector<std::shared_ptr<Mesh>>& lodChain, 
                                                   float distance, const LODGenerationConfig& config) {
        if (lodChain.empty()) return nullptr;
        if (!config.enableDistanceBasedSelection) return lodChain[0];
        
        // Find appropriate LOD based on distance
        for (size_t i = 0; i < config.lodDistances.size() && i < lodChain.size(); ++i) {
            if (distance <= config.lodDistances[i]) {
                return lodChain[i];
            }
        }
        
        // Return highest LOD (most simplified) if distance is very far
        return lodChain.back();
    }
    
    // Vertex processing
    void MeshOptimizer::RemoveDuplicateVertices(Mesh& mesh, float epsilon) {
        auto vertices = mesh.GetVertices();
        auto indices = mesh.GetIndices();
        
        if (vertices.empty()) return;
        
        std::vector<Vertex> uniqueVertices;
        std::vector<uint32_t> vertexRemap(vertices.size());
        
        for (size_t i = 0; i < vertices.size(); ++i) {
            bool found = false;
            for (size_t j = 0; j < uniqueVertices.size(); ++j) {
                if (vertices[i].IsNearlyEqual(uniqueVertices[j], epsilon)) {
                    vertexRemap[i] = static_cast<uint32_t>(j);
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                vertexRemap[i] = static_cast<uint32_t>(uniqueVertices.size());
                uniqueVertices.push_back(vertices[i]);
            }
        }
        
        // Update indices
        for (auto& index : indices) {
            if (index < vertexRemap.size()) {
                index = vertexRemap[index];
            }
        }
        
        mesh.SetVertices(uniqueVertices);
        mesh.SetIndices(indices);
        
        if (s_verboseLogging) {
            LOG_INFO("Removed duplicate vertices: " + std::to_string(vertices.size()) + 
                    " -> " + std::to_string(uniqueVertices.size()));
        }
    }
    
    void MeshOptimizer::GenerateNormals(Mesh& mesh, bool smooth) {
        mesh.GenerateNormals(smooth);
    }
    
    void MeshOptimizer::GenerateTangents(Mesh& mesh) {
        mesh.GenerateTangents();
    }
    
    void MeshOptimizer::FlipNormals(Mesh& mesh) {
        auto vertices = mesh.GetVertices();
        for (auto& vertex : vertices) {
            vertex.normal = -vertex.normal;
        }
        mesh.SetVertices(vertices);
        
        if (s_verboseLogging) {
            LOG_INFO("Flipped normals for mesh");
        }
    }
    
    // Mesh analysis
    MeshAnalysis MeshOptimizer::AnalyzeMesh(const Mesh& mesh) {
        MeshAnalysis analysis;
        
        const auto& vertices = mesh.GetVertices();
        const auto& indices = mesh.GetIndices();
        
        analysis.vertexCount = static_cast<uint32_t>(vertices.size());
        analysis.triangleCount = static_cast<uint32_t>(indices.size() / 3);
        analysis.memoryUsage = mesh.GetMemoryUsage();
        
        if (vertices.empty()) return analysis;
        
        // Calculate bounding box
        analysis.bounds.min = vertices[0].position;
        analysis.bounds.max = vertices[0].position;
        for (const auto& vertex : vertices) {
            analysis.bounds.Expand(vertex.position);
        }
        
        // Analyze vertex attributes
        analysis.hasNormals = std::any_of(vertices.begin(), vertices.end(),
            [](const Vertex& v) { return glm::length(v.normal) > 0.001f; });
        analysis.hasTangents = std::any_of(vertices.begin(), vertices.end(),
            [](const Vertex& v) { return glm::length(v.tangent) > 0.001f; });
        analysis.hasTextureCoords = std::any_of(vertices.begin(), vertices.end(),
            [](const Vertex& v) { return glm::length(v.texCoords) > 0.001f; });
        analysis.hasColors = std::any_of(vertices.begin(), vertices.end(),
            [](const Vertex& v) { return v.color != Math::Vec4(1.0f); });
        analysis.hasBoneWeights = std::any_of(vertices.begin(), vertices.end(),
            [](const Vertex& v) { return glm::length(v.boneWeights) > 0.001f; });
        
        // Analyze triangles
        if (!indices.empty() && indices.size() >= 3) {
            float totalArea = 0.0f;
            float totalQuality = 0.0f;
            analysis.minTriangleArea = FLT_MAX;
            analysis.maxTriangleArea = 0.0f;
            analysis.minTriangleQuality = FLT_MAX;
            uint32_t validTriangles = 0;
            
            for (size_t i = 0; i < indices.size(); i += 3) {
                if (i + 2 < indices.size() && 
                    indices[i] < vertices.size() && 
                    indices[i + 1] < vertices.size() && 
                    indices[i + 2] < vertices.size()) {
                    
                    const Math::Vec3& v0 = vertices[indices[i]].position;
                    const Math::Vec3& v1 = vertices[indices[i + 1]].position;
                    const Math::Vec3& v2 = vertices[indices[i + 2]].position;
                    
                    // Calculate area
                    Math::Vec3 edge1 = v1 - v0;
                    Math::Vec3 edge2 = v2 - v0;
                    float area = 0.5f * glm::length(glm::cross(edge1, edge2));
                    
                    if (area > 0.0001f) { // Valid triangle
                        totalArea += area;
                        analysis.minTriangleArea = std::min(analysis.minTriangleArea, area);
                        analysis.maxTriangleArea = std::max(analysis.maxTriangleArea, area);
                        
                        // Calculate quality
                        float quality = CalculateTriangleQuality(v0, v1, v2);
                        totalQuality += quality;
                        analysis.minTriangleQuality = std::min(analysis.minTriangleQuality, quality);
                        
                        // Check for thin triangles
                        if (IsTriangleThin(v0, v1, v2)) {
                            analysis.thinTriangles++;
                        }
                        
                        // Check for small triangles
                        if (IsTriangleSmall(v0, v1, v2)) {
                            analysis.smallTriangles++;
                        }
                        
                        validTriangles++;
                    } else {
                        analysis.degenerateTriangles++;
                    }
                }
            }
            
            if (validTriangles > 0) {
                analysis.averageTriangleArea = totalArea / validTriangles;
                analysis.averageTriangleQuality = totalQuality / validTriangles;
            }
        }
        
        // Calculate cache efficiency
        analysis.cacheEfficiency = CalculateACMR(indices, s_cacheSize);
        
        // Calculate overdraw ratio
        analysis.overdrawRatio = CalculateOverdrawRatio(indices, vertices);
        
        // Count duplicate vertices
        std::unordered_set<size_t> uniqueVertices;
        for (const auto& vertex : vertices) {
            size_t hash = std::hash<float>{}(vertex.position.x) ^
                         (std::hash<float>{}(vertex.position.y) << 1) ^
                         (std::hash<float>{}(vertex.position.z) << 2);
            if (uniqueVertices.find(hash) != uniqueVertices.end()) {
                analysis.duplicateVertices++;
            } else {
                uniqueVertices.insert(hash);
            }
        }
        
        return analysis;
    }
    
    bool MeshOptimizer::ValidateMesh(const Mesh& mesh) {
        return GetMeshIssues(mesh).empty();
    }
    
    std::vector<std::string> MeshOptimizer::GetMeshIssues(const Mesh& mesh) {
        std::vector<std::string> issues;
        
        const auto& vertices = mesh.GetVertices();
        const auto& indices = mesh.GetIndices();
        
        if (vertices.empty()) {
            issues.push_back("Mesh has no vertices");
        }
        
        if (indices.empty() && vertices.size() % 3 != 0) {
            issues.push_back("Mesh has no indices and vertex count is not divisible by 3");
        }
        
        // Check for out-of-bounds indices
        for (size_t i = 0; i < indices.size(); ++i) {
            if (indices[i] >= vertices.size()) {
                issues.push_back("Index " + std::to_string(i) + " is out of bounds");
                break; // Don't spam with too many errors
            }
        }
        
        // Check for degenerate triangles
        uint32_t degenerateCount = 0;
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 < indices.size() && 
                indices[i] < vertices.size() && 
                indices[i + 1] < vertices.size() && 
                indices[i + 2] < vertices.size()) {
                
                const Math::Vec3& v0 = vertices[indices[i]].position;
                const Math::Vec3& v1 = vertices[indices[i + 1]].position;
                const Math::Vec3& v2 = vertices[indices[i + 2]].position;
                
                if (IsTriangleSmall(v0, v1, v2, 0.0001f)) {
                    degenerateCount++;
                }
            }
        }
        
        if (degenerateCount > 0) {
            issues.push_back("Mesh has " + std::to_string(degenerateCount) + " degenerate triangles");
        }
        
        return issues;
    }
    
    // Optimization statistics
    MeshOptimizationStats MeshOptimizer::GetOptimizationStats(const Mesh& originalMesh, const Mesh& optimizedMesh) {
        MeshOptimizationStats stats;
        
        // Original mesh stats
        stats.originalVertexCount = originalMesh.GetVertexCount();
        stats.originalTriangleCount = originalMesh.GetTriangleCount();
        stats.originalACMR = CalculateACMR(originalMesh.GetIndices(), s_cacheSize);
        stats.originalATVR = CalculateATVR(originalMesh.GetIndices(), originalMesh.GetVertexCount());
        stats.originalMemoryUsage = originalMesh.GetMemoryUsage();
        
        // Optimized mesh stats
        stats.optimizedVertexCount = optimizedMesh.GetVertexCount();
        stats.optimizedTriangleCount = optimizedMesh.GetTriangleCount();
        stats.optimizedACMR = CalculateACMR(optimizedMesh.GetIndices(), s_cacheSize);
        stats.optimizedATVR = CalculateATVR(optimizedMesh.GetIndices(), optimizedMesh.GetVertexCount());
        stats.optimizedMemoryUsage = optimizedMesh.GetMemoryUsage();
        
        stats.CalculateImprovements();
        
        return stats;
    }
    
    MeshOptimizationStats MeshOptimizer::OptimizeWithStats(Mesh& mesh, bool optimizeCache, 
                                                           bool optimizeFetch, bool optimizeOverdraw) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Create copy for comparison
        Mesh originalMesh;
        originalMesh.SetVertices(mesh.GetVertices());
        originalMesh.SetIndices(mesh.GetIndices());
        
        // Perform optimizations
        if (optimizeCache) {
            OptimizeVertexCache(mesh);
        }
        if (optimizeFetch) {
            OptimizeVertexFetch(mesh);
        }
        if (optimizeOverdraw) {
            OptimizeOverdraw(mesh);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        MeshOptimizationStats stats = GetOptimizationStats(originalMesh, mesh);
        stats.optimizationTimeMs = duration.count() / 1000.0f;
        
        return stats;
    }
    
    // Advanced optimization pipeline
    void MeshOptimizer::OptimizeForRendering(Mesh& mesh, const LODGenerationConfig& config) {
        if (s_verboseLogging) {
            LOG_INFO("Starting comprehensive mesh optimization for rendering...");
        }
        
        // Step 1: Remove duplicate vertices
        RemoveDuplicateVertices(mesh);
        
        // Step 2: Generate missing attributes
        auto analysis = AnalyzeMesh(mesh);
        if (!analysis.hasNormals) {
            GenerateNormals(mesh, true);
        }
        if (!analysis.hasTangents && analysis.hasTextureCoords) {
            GenerateTangents(mesh);
        }
        
        // Step 3: Optimize for GPU rendering
        OptimizeVertexCache(mesh);
        OptimizeVertexFetch(mesh);
        OptimizeOverdraw(mesh);
        
        if (s_verboseLogging) {
            LOG_INFO("Comprehensive mesh optimization completed");
        }
    }
    
    std::vector<std::shared_ptr<Mesh>> MeshOptimizer::CreateOptimizedLODChain(const Mesh& mesh, 
                                                                              const LODGenerationConfig& config) {
        if (s_verboseLogging) {
            LOG_INFO("Creating optimized LOD chain...");
        }
        
        // Generate LOD chain
        auto lodChain = GenerateLODChain(mesh, config.simplificationRatios);
        
        // Optimize each LOD level
        for (auto& lodMesh : lodChain) {
            if (lodMesh) {
                OptimizeForRendering(*lodMesh, config);
            }
        }
        
        if (s_verboseLogging) {
            LOG_INFO("Created optimized LOD chain with " + std::to_string(lodChain.size()) + " levels");
        }
        
        return lodChain;
    }
    
    // Helper methods implementation
    void MeshOptimizer::ReorderIndicesForCache(std::vector<uint32_t>& indices, size_t vertexCount) {
        // This is already implemented in OptimizeIndices
        indices = MeshOptimizer::OptimizeIndices(indices, vertexCount);
    }
    
    void MeshOptimizer::ReorderVerticesForFetch(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) return;
        
        // Create vertex usage order based on index order
        std::vector<uint32_t> vertexOrder;
        std::vector<bool> vertexUsed(vertices.size(), false);
        
        for (uint32_t index : indices) {
            if (index < vertices.size() && !vertexUsed[index]) {
                vertexOrder.push_back(index);
                vertexUsed[index] = true;
            }
        }
        
        // Add any unused vertices at the end
        for (uint32_t i = 0; i < vertices.size(); ++i) {
            if (!vertexUsed[i]) {
                vertexOrder.push_back(i);
            }
        }
        
        // Create remapping table
        std::vector<uint32_t> vertexRemap(vertices.size());
        for (uint32_t i = 0; i < vertexOrder.size(); ++i) {
            vertexRemap[vertexOrder[i]] = i;
        }
        
        // Reorder vertices
        std::vector<Vertex> newVertices;
        newVertices.reserve(vertices.size());
        for (uint32_t oldIndex : vertexOrder) {
            newVertices.push_back(vertices[oldIndex]);
        }
        
        // Update indices
        for (uint32_t& index : indices) {
            if (index < vertexRemap.size()) {
                index = vertexRemap[index];
            }
        }
        
        vertices = std::move(newVertices);
    }
    
    float MeshOptimizer::CalculateACMR(const std::vector<uint32_t>& indices, size_t cacheSize) {
        if (indices.empty()) return 0.0f;
        
        VertexCacheSimulator cache(static_cast<uint32_t>(cacheSize));
        
        for (uint32_t index : indices) {
            cache.AccessVertex(index);
        }
        
        return cache.GetCacheMissRatio();
    }
    
    float MeshOptimizer::CalculateATVR(const std::vector<uint32_t>& indices, size_t vertexCount) {
        if (indices.empty() || vertexCount == 0) return 0.0f;
        
        return static_cast<float>(indices.size()) / static_cast<float>(vertexCount);
    }
    
    float MeshOptimizer::CalculateOverdrawRatio(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices) {
        // Simple overdraw estimation based on triangle facing
        if (indices.size() < 3 || vertices.empty()) return 0.0f;
        
        uint32_t frontFacingTriangles = 0;
        uint32_t totalTriangles = 0;
        
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 < indices.size() && 
                indices[i] < vertices.size() && 
                indices[i + 1] < vertices.size() && 
                indices[i + 2] < vertices.size()) {
                
                const Math::Vec3& v0 = vertices[indices[i]].position;
                const Math::Vec3& v1 = vertices[indices[i + 1]].position;
                const Math::Vec3& v2 = vertices[indices[i + 2]].position;
                
                // Calculate normal
                Math::Vec3 edge1 = v1 - v0;
                Math::Vec3 edge2 = v2 - v0;
                Math::Vec3 normal = glm::cross(edge1, edge2);
                
                // Simple front-facing test (assuming camera at origin looking down -Z)
                if (normal.z > 0.0f) {
                    frontFacingTriangles++;
                }
                totalTriangles++;
            }
        }
        
        if (totalTriangles == 0) return 0.0f;
        
        // Overdraw ratio: higher values mean more overdraw
        return 1.0f - (static_cast<float>(frontFacingTriangles) / totalTriangles);
    }
    
    // Triangle quality analysis
    float MeshOptimizer::CalculateTriangleQuality(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2) {
        // Calculate triangle quality based on aspect ratio and area
        Math::Vec3 edge1 = v1 - v0;
        Math::Vec3 edge2 = v2 - v0;
        Math::Vec3 edge3 = v2 - v1;
        
        float len1 = glm::length(edge1);
        float len2 = glm::length(edge2);
        float len3 = glm::length(edge3);
        
        if (len1 < 0.0001f || len2 < 0.0001f || len3 < 0.0001f) {
            return 0.0f; // Degenerate triangle
        }
        
        // Calculate area
        float area = 0.5f * glm::length(glm::cross(edge1, edge2));
        if (area < 0.0001f) {
            return 0.0f; // Degenerate triangle
        }
        
        // Calculate perimeter
        float perimeter = len1 + len2 + len3;
        
        // Quality metric: 4 * sqrt(3) * area / perimeter^2
        // This gives 1.0 for equilateral triangles and approaches 0 for degenerate triangles
        float quality = (4.0f * std::sqrt(3.0f) * area) / (perimeter * perimeter);
        
        return std::max(0.0f, std::min(1.0f, quality));
    }
    
    float MeshOptimizer::CalculateTriangleAspectRatio(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2) {
        Math::Vec3 edge1 = v1 - v0;
        Math::Vec3 edge2 = v2 - v0;
        Math::Vec3 edge3 = v2 - v1;
        
        float len1 = glm::length(edge1);
        float len2 = glm::length(edge2);
        float len3 = glm::length(edge3);
        
        float maxLen = std::max({len1, len2, len3});
        float minLen = std::min({len1, len2, len3});
        
        if (minLen < 0.0001f) {
            return FLT_MAX; // Degenerate triangle
        }
        
        return maxLen / minLen;
    }
    
    bool MeshOptimizer::IsTriangleThin(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2, float threshold) {
        return CalculateTriangleAspectRatio(v0, v1, v2) > threshold;
    }
    
    bool MeshOptimizer::IsTriangleSmall(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2, float threshold) {
        Math::Vec3 edge1 = v1 - v0;
        Math::Vec3 edge2 = v2 - v0;
        float area = 0.5f * glm::length(glm::cross(edge1, edge2));
        return area < threshold;
    }
    
    // Simplification algorithms
    std::shared_ptr<Mesh> MeshOptimizer::SimplifyQuadricErrorMetrics(const Mesh& mesh, float ratio, float maxError) {
        // Simplified implementation of quadric error metrics
        // This is a basic version - a full implementation would be much more complex
        
        auto vertices = mesh.GetVertices();
        auto indices = mesh.GetIndices();
        
        if (vertices.empty() || indices.empty() || indices.size() < 3) {
            return nullptr;
        }
        
        uint32_t targetTriangles = static_cast<uint32_t>(indices.size() / 3 * ratio);
        if (targetTriangles == 0) targetTriangles = 1;
        
        // For now, use simple edge collapse
        return SimplifyEdgeCollapse(mesh, targetTriangles);
    }
    
    std::shared_ptr<Mesh> MeshOptimizer::SimplifyEdgeCollapse(const Mesh& mesh, uint32_t targetTriangles) {
        // Simplified edge collapse implementation
        // This is a basic version for demonstration purposes
        
        auto vertices = mesh.GetVertices();
        auto indices = mesh.GetIndices();
        
        if (vertices.empty() || indices.empty() || indices.size() < 3) {
            return nullptr;
        }
        
        uint32_t currentTriangles = static_cast<uint32_t>(indices.size() / 3);
        if (targetTriangles >= currentTriangles) {
            // No simplification needed
            auto newMesh = std::make_shared<Mesh>();
            newMesh->SetVertices(vertices);
            newMesh->SetIndices(indices);
            return newMesh;
        }
        
        // Simple decimation: remove every nth triangle
        uint32_t removeCount = currentTriangles - targetTriangles;
        uint32_t step = currentTriangles / removeCount;
        if (step == 0) step = 1;
        
        std::vector<uint32_t> newIndices;
        for (uint32_t i = 0; i < currentTriangles; ++i) {
            if (i % step != 0 || newIndices.size() / 3 >= targetTriangles) {
                // Keep this triangle
                if ((i * 3 + 2) < indices.size()) {
                    newIndices.push_back(indices[i * 3]);
                    newIndices.push_back(indices[i * 3 + 1]);
                    newIndices.push_back(indices[i * 3 + 2]);
                }
            }
        }
        
        auto newMesh = std::make_shared<Mesh>();
        newMesh->SetVertices(vertices);
        newIndices.resize((targetTriangles * 3));
        newMesh->SetIndices(newIndices);
        
        return newMesh;
    }
    
    // Vertex cache simulator implementation
    bool MeshOptimizer::VertexCacheSimulator::AccessVertex(uint32_t vertex) {
        totalAccesses++;
        
        // Check if vertex is in cache
        auto it = std::find(cache.begin(), cache.end(), vertex);
        if (it != cache.end()) {
            // Cache hit - move to front
            cache.erase(it);
            cache.insert(cache.begin(), vertex);
            return true;
        } else {
            // Cache miss
            cacheMisses++;
            cache.insert(cache.begin(), vertex);
            
            // Limit cache size
            if (cache.size() > cacheSize) {
                cache.resize(cacheSize);
            }
            return false;
        }
    }
    
    float MeshOptimizer::VertexCacheSimulator::GetCacheMissRatio() const {
        if (totalAccesses == 0) return 0.0f;
        return static_cast<float>(cacheMisses) / totalAccesses;
    }
    
    void MeshOptimizer::VertexCacheSimulator::Reset() {
        cache.clear();
        cacheMisses = 0;
        totalAccesses = 0;
    }
    
    // Vertex scoring for Tom Forsyth's algorithm
    float MeshOptimizer::CalculateVertexScore(uint32_t vertex, const std::vector<uint32_t>& cache, 
                                             const std::vector<uint32_t>& valence, uint32_t cacheSize) {
        const float CACHE_DECAY_POWER = 1.5f;
        const float LAST_TRI_SCORE = 0.75f;
        const float VALENCE_BOOST_SCALE = 2.0f;
        const float VALENCE_BOOST_POWER = 0.5f;
        
        if (vertex >= valence.size() || valence[vertex] == 0) {
            return -1.0f;
        }
        
        float score = 0.0f;
        
        // Cache position score
        auto cacheIt = std::find(cache.begin(), cache.end(), vertex);
        if (cacheIt != cache.end()) {
            uint32_t cachePos = static_cast<uint32_t>(std::distance(cache.begin(), cacheIt));
            if (cachePos < 3) {
                score = LAST_TRI_SCORE;
            } else {
                const float scaler = 1.0f / (cacheSize - 3);
                score = 1.0f - (cachePos - 3) * scaler;
                score = std::pow(score, CACHE_DECAY_POWER);
            }
        }
        
        // Valence score
        float valenceScore = std::pow(static_cast<float>(valence[vertex]), -VALENCE_BOOST_POWER);
        score += VALENCE_BOOST_SCALE * valenceScore;
        
        return score;
    }
    
    // Utility functions for quadric error metrics (simplified)
    Math::Mat4 MeshOptimizer::CalculateQuadricMatrix(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Vec3& v2) {
        // Calculate plane equation for triangle
        Math::Vec3 edge1 = v1 - v0;
        Math::Vec3 edge2 = v2 - v0;
        Math::Vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        float d = -glm::dot(normal, v0);
        
        // Create quadric matrix from plane equation ax + by + cz + d = 0
        Math::Mat4 quadric(0.0f);
        quadric[0][0] = normal.x * normal.x;
        quadric[0][1] = normal.x * normal.y;
        quadric[0][2] = normal.x * normal.z;
        quadric[0][3] = normal.x * d;
        
        quadric[1][0] = normal.y * normal.x;
        quadric[1][1] = normal.y * normal.y;
        quadric[1][2] = normal.y * normal.z;
        quadric[1][3] = normal.y * d;
        
        quadric[2][0] = normal.z * normal.x;
        quadric[2][1] = normal.z * normal.y;
        quadric[2][2] = normal.z * normal.z;
        quadric[2][3] = normal.z * d;
        
        quadric[3][0] = d * normal.x;
        quadric[3][1] = d * normal.y;
        quadric[3][2] = d * normal.z;
        quadric[3][3] = d * d;
        
        return quadric;
    }
    
    float MeshOptimizer::CalculateEdgeCollapseError(const Math::Vec3& v0, const Math::Vec3& v1, const Math::Mat4& quadric) {
        // Calculate error for collapsing edge to midpoint
        Math::Vec3 midpoint = (v0 + v1) * 0.5f;
        Math::Vec4 point(midpoint, 1.0f);
        
        // Error = point^T * quadric * point
        Math::Vec4 temp = quadric * point;
        return glm::dot(point, temp);
    }
    
    bool MeshOptimizer::CanCollapseEdge(uint32_t v0, uint32_t v1, const std::vector<Vertex>& vertices, 
                                       const std::vector<uint32_t>& indices) {
        // Simple check - in a full implementation, this would check for topology preservation
        return v0 < vertices.size() && v1 < vertices.size() && v0 != v1;
    }
    
} // namespace GameEngine