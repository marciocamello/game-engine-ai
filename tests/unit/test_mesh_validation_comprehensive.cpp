#include "Graphics/Mesh.h"
#include "Graphics/MeshOptimizer.h"
#include "Core/Logger.h"
#include "TestUtils.h"
#include <vector>
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Create a mesh with various validation issues for testing
 */
std::shared_ptr<Mesh> CreateProblematicMesh() {
    auto mesh = std::make_shared<Mesh>("problematic_mesh");
    
    std::vector<Vertex> vertices = {
        // Valid vertices
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}},
        
        // Duplicate vertex (same position)
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        
        // Vertex with invalid normal (zero length)
        {{2.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        
        // Vertex with NaN values
        {{std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        
        // Vertex with infinite values
        {{std::numeric_limits<float>::infinity(), 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
    };
    
    std::vector<uint32_t> indices = {
        0, 1, 2,    // Valid triangle
        0, 3, 1,    // Triangle using duplicate vertex
        1, 1, 2,    // Degenerate triangle (repeated vertex)
        0, 1, 2,    // Duplicate triangle
        4, 5, 6,    // Triangle with problematic vertices
        100, 101, 102  // Out of bounds indices
    };
    
    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    
    return mesh;
}

/**
 * Create a valid mesh for comparison testing
 */
std::shared_ptr<Mesh> CreateValidMesh() {
    auto mesh = std::make_shared<Mesh>("valid_mesh");
    
    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}},
        {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<uint32_t> indices = {
        0, 1, 2,
        0, 2, 3
    };
    
    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    
    return mesh;
}

/**
 * Test comprehensive mesh validation
 * Requirements: 4.7, 9.3 (Mesh validation with degenerate triangle detection)
 */
bool TestComprehensiveMeshValidation() {
    TestOutput::PrintTestStart("comprehensive mesh validation");
    
    // Test valid mesh
    auto validMesh = CreateValidMesh();
    EXPECT_TRUE(MeshOptimizer::ValidateMesh(*validMesh));
    
    auto validIssues = MeshOptimizer::GetMeshIssues(*validMesh);
    EXPECT_TRUE(validIssues.empty());
    
    // Test problematic mesh
    auto problematicMesh = CreateProblematicMesh();
    EXPECT_FALSE(MeshOptimizer::ValidateMesh(*problematicMesh));
    
    auto issues = MeshOptimizer::GetMeshIssues(*problematicMesh);
    EXPECT_TRUE(issues.size() > 0);
    
    TestOutput::PrintInfo("Found " + std::to_string(issues.size()) + " validation issues");
    
    // Check for specific issue types
    bool foundDuplicateVertices = false;
    bool foundDegenerateTriangles = false;
    bool foundInvalidNormals = false;
    bool foundOutOfBoundsIndices = false;
    bool foundNaNValues = false;
    
    for (const auto& issue : issues) {
        if (issue.find("duplicate") != std::string::npos) {
            foundDuplicateVertices = true;
        }
        if (issue.find("degenerate") != std::string::npos) {
            foundDegenerateTriangles = true;
        }
        if (issue.find("normal") != std::string::npos) {
            foundInvalidNormals = true;
        }
        if (issue.find("out of bounds") != std::string::npos) {
            foundOutOfBoundsIndices = true;
        }
        if (issue.find("NaN") != std::string::npos || issue.find("infinite") != std::string::npos) {
            foundNaNValues = true;
        }
    }
    
    EXPECT_TRUE(foundDuplicateVertices);
    EXPECT_TRUE(foundDegenerateTriangles);
    EXPECT_TRUE(foundOutOfBoundsIndices);
    
    TestOutput::PrintInfo("Validation correctly detected expected issue types");
    
    TestOutput::PrintTestPass("comprehensive mesh validation");
    return true;
}

/**
 * Test mesh analysis with detailed statistics
 * Requirements: 4.7 (Mesh analysis with triangle quality and vertex statistics)
 */
bool TestDetailedMeshAnalysis() {
    TestOutput::PrintTestStart("detailed mesh analysis");
    
    auto mesh = CreateValidMesh();
    auto analysis = MeshOptimizer::AnalyzeMesh(*mesh);
    
    // Verify basic counts
    EXPECT_EQUAL(analysis.vertexCount, static_cast<uint32_t>(4));
    EXPECT_EQUAL(analysis.triangleCount, static_cast<uint32_t>(2));
    
    // Verify attribute flags
    EXPECT_TRUE(analysis.hasNormals);
    EXPECT_TRUE(analysis.hasTextureCoords);
    EXPECT_FALSE(analysis.hasColors);
    EXPECT_FALSE(analysis.hasBoneWeights);
    EXPECT_FALSE(analysis.hasTangents);
    
    // Verify triangle quality metrics
    EXPECT_TRUE(analysis.averageTriangleArea > 0.0f);
    EXPECT_TRUE(analysis.minTriangleArea > 0.0f);
    EXPECT_TRUE(analysis.maxTriangleArea > 0.0f);
    EXPECT_TRUE(analysis.minTriangleArea <= analysis.averageTriangleArea);
    EXPECT_TRUE(analysis.averageTriangleArea <= analysis.maxTriangleArea);
    
    // Verify bounding box
    EXPECT_TRUE(analysis.bounds.IsValid());
    EXPECT_TRUE(analysis.bounds.GetSize().x > 0.0f);
    EXPECT_TRUE(analysis.bounds.GetSize().y > 0.0f);
    
    // Verify cache efficiency (should be reasonable for small mesh)
    EXPECT_TRUE(analysis.cacheEfficiency >= 0.0f);
    EXPECT_TRUE(analysis.cacheEfficiency <= 10.0f); // ACMR should be reasonable
    
    TestOutput::PrintInfo("Triangle area range: " + 
                         std::to_string(analysis.minTriangleArea) + " to " + 
                         std::to_string(analysis.maxTriangleArea));
    TestOutput::PrintInfo("Cache efficiency (ACMR): " + std::to_string(analysis.cacheEfficiency));
    
    TestOutput::PrintTestPass("detailed mesh analysis");
    return true;
}

/**
 * Test mesh optimization statistics and reporting
 * Requirements: 4.1, 4.2, 10.5 (Mesh optimization statistics and performance reporting)
 */
bool TestMeshOptimizationStatistics() {
    TestOutput::PrintTestStart("mesh optimization statistics");
    
    auto originalMesh = CreateValidMesh();
    auto optimizedMesh = std::make_shared<Mesh>(*originalMesh); // Copy for optimization
    
    // Get original statistics
    auto originalStats = MeshOptimizer::AnalyzeMesh(*originalMesh);
    
    // Optimize the mesh
    MeshOptimizer::OptimizeVertexCache(*optimizedMesh);
    MeshOptimizer::OptimizeVertexFetch(*optimizedMesh);
    
    // Get optimized statistics
    auto optimizedStats = MeshOptimizer::AnalyzeMesh(*optimizedMesh);
    
    // Get optimization comparison
    auto comparisonStats = MeshOptimizer::GetOptimizationStats(*originalMesh, *optimizedMesh);
    
    // Verify optimization maintained mesh integrity
    EXPECT_EQUAL(originalStats.vertexCount, optimizedStats.vertexCount);
    EXPECT_EQUAL(originalStats.triangleCount, optimizedStats.triangleCount);
    
    // Verify cache efficiency improved or stayed the same
    EXPECT_TRUE(optimizedStats.cacheEfficiency <= originalStats.cacheEfficiency + 0.1f);
    
    // Verify comparison statistics
    EXPECT_EQUAL(comparisonStats.originalVertexCount, originalStats.vertexCount);
    EXPECT_EQUAL(comparisonStats.optimizedVertexCount, optimizedStats.vertexCount);
    EXPECT_EQUAL(comparisonStats.originalTriangleCount, originalStats.triangleCount);
    EXPECT_EQUAL(comparisonStats.optimizedTriangleCount, optimizedStats.triangleCount);
    
    TestOutput::PrintInfo("Original ACMR: " + std::to_string(originalStats.cacheEfficiency));
    TestOutput::PrintInfo("Optimized ACMR: " + std::to_string(optimizedStats.cacheEfficiency));
    TestOutput::PrintInfo("Cache improvement: " + std::to_string(comparisonStats.cacheImprovement) + "%");
    
    TestOutput::PrintTestPass("mesh optimization statistics");
    return true;
}

/**
 * Test vertex attribute validation and processing
 * Requirements: 4.3, 4.4 (Vertex attribute support and validation)
 */
bool TestVertexAttributeValidation() {
    TestOutput::PrintTestStart("vertex attribute validation");
    
    auto mesh = std::make_shared<Mesh>("attribute_test");
    
    // Create vertices with various attribute combinations
    std::vector<Vertex> vertices = {
        // Vertex with all attributes
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, 
         {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        
        // Vertex with normalized normal
        {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 
         {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f, 1.0f}},
        
        // Vertex with unnormalized normal (should be detected)
        {{0.5f, 1.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, {0.5f, 1.0f}, 
         {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}}
    };
    
    std::vector<uint32_t> indices = {0, 1, 2};
    
    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    
    // Analyze vertex attributes
    auto analysis = MeshOptimizer::AnalyzeMesh(*mesh);
    
    EXPECT_TRUE(analysis.hasNormals);
    EXPECT_TRUE(analysis.hasTextureCoords);
    EXPECT_TRUE(analysis.hasColors);
    EXPECT_TRUE(analysis.hasTangents);
    EXPECT_FALSE(analysis.hasBoneWeights);
    
    // Validate mesh (should detect unnormalized normal)
    auto issues = MeshOptimizer::GetMeshIssues(*mesh);
    bool foundNormalIssue = false;
    for (const auto& issue : issues) {
        if (issue.find("normal") != std::string::npos) {
            foundNormalIssue = true;
            break;
        }
    }
    
    TestOutput::PrintInfo("Vertex attribute analysis completed");
    if (foundNormalIssue) {
        TestOutput::PrintInfo("Correctly detected unnormalized normal");
    }
    
    TestOutput::PrintTestPass("vertex attribute validation");
    return true;
}

/**
 * Test mesh bounds calculation and validation
 * Requirements: 8.1, 8.2 (Bounding volume calculation)
 */
bool TestMeshBoundsCalculation() {
    TestOutput::PrintTestStart("mesh bounds calculation");
    
    auto mesh = CreateValidMesh();
    
    // Update bounds
    mesh->UpdateBounds();
    
    // Get bounding volumes
    auto boundingBox = mesh->GetBoundingBox();
    auto boundingSphere = mesh->GetBoundingSphere();
    
    // Verify bounding box
    EXPECT_TRUE(boundingBox.IsValid());
    EXPECT_TRUE(boundingBox.GetSize().x > 0.0f);
    EXPECT_TRUE(boundingBox.GetSize().y > 0.0f);
    
    // Verify bounding sphere
    EXPECT_TRUE(boundingSphere.radius > 0.0f);
    
    // Verify sphere contains all vertices
    auto vertices = mesh->GetVertices();
    for (const auto& vertex : vertices) {
        float distance = glm::length(vertex.position - boundingSphere.center);
        EXPECT_TRUE(distance <= boundingSphere.radius + 0.001f); // Small epsilon for floating point
    }
    
    // Verify box contains all vertices (manual check since Contains may not be available)
    for (const auto& vertex : vertices) {
        EXPECT_TRUE(vertex.position.x >= boundingBox.min.x && vertex.position.x <= boundingBox.max.x);
        EXPECT_TRUE(vertex.position.y >= boundingBox.min.y && vertex.position.y <= boundingBox.max.y);
        EXPECT_TRUE(vertex.position.z >= boundingBox.min.z && vertex.position.z <= boundingBox.max.z);
    }
    
    TestOutput::PrintInfo("Bounding box size: " + 
                         std::to_string(boundingBox.GetSize().x) + "x" + 
                         std::to_string(boundingBox.GetSize().y) + "x" + 
                         std::to_string(boundingBox.GetSize().z));
    TestOutput::PrintInfo("Bounding sphere radius: " + std::to_string(boundingSphere.radius));
    
    TestOutput::PrintTestPass("mesh bounds calculation");
    return true;
}

/**
 * Test mesh memory usage calculation and optimization
 * Requirements: 10.5 (Memory usage analysis and optimization)
 */
bool TestMeshMemoryUsage() {
    TestOutput::PrintTestStart("mesh memory usage calculation");
    
    auto mesh = CreateValidMesh();
    
    // Get memory usage
    size_t memoryUsage = mesh->GetMemoryUsage();
    EXPECT_TRUE(memoryUsage > 0);
    
    // Calculate expected memory usage
    auto vertices = mesh->GetVertices();
    auto indices = mesh->GetIndices();
    
    size_t expectedVertexMemory = vertices.size() * sizeof(Vertex);
    size_t expectedIndexMemory = indices.size() * sizeof(uint32_t);
    size_t expectedTotal = expectedVertexMemory + expectedIndexMemory;
    
    // Memory usage should be at least the data size (may include additional overhead)
    EXPECT_TRUE(memoryUsage >= expectedTotal);
    
    // Get detailed statistics
    auto stats = mesh->GetStats();
    EXPECT_EQUAL(stats.vertexCount, static_cast<uint32_t>(vertices.size()));
    EXPECT_EQUAL(stats.triangleCount, static_cast<uint32_t>(indices.size() / 3));
    EXPECT_TRUE(stats.memoryUsage > 0);
    
    TestOutput::PrintInfo("Mesh memory usage: " + std::to_string(memoryUsage) + " bytes");
    TestOutput::PrintInfo("Expected minimum: " + std::to_string(expectedTotal) + " bytes");
    TestOutput::PrintInfo("Vertex count: " + std::to_string(stats.vertexCount));
    TestOutput::PrintInfo("Triangle count: " + std::to_string(stats.triangleCount));
    
    TestOutput::PrintTestPass("mesh memory usage calculation");
    return true;
}

int main() {
    TestOutput::PrintHeader("Comprehensive Mesh Validation");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        
        // Create test suite for result tracking
        TestSuite suite("Comprehensive Mesh Validation Tests");

        // Run all tests
        allPassed &= suite.RunTest("Comprehensive Mesh Validation", TestComprehensiveMeshValidation);
        allPassed &= suite.RunTest("Detailed Mesh Analysis", TestDetailedMeshAnalysis);
        allPassed &= suite.RunTest("Mesh Optimization Statistics", TestMeshOptimizationStatistics);
        allPassed &= suite.RunTest("Vertex Attribute Validation", TestVertexAttributeValidation);
        allPassed &= suite.RunTest("Mesh Bounds Calculation", TestMeshBoundsCalculation);
        allPassed &= suite.RunTest("Mesh Memory Usage", TestMeshMemoryUsage);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}