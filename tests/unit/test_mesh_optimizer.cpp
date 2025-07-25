#include "Graphics/MeshOptimizer.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include "TestUtils.h"
#include <iostream>
#include <vector>
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Test mesh creation helper
std::shared_ptr<Mesh> CreateTestMesh() {
    auto mesh = std::make_shared<Mesh>("test_mesh");
    
    // Create a simple triangle for testing
    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}},
        
        // Duplicate vertex for testing
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
    };
    
    std::vector<uint32_t> indices = {0, 1, 2, 0, 3, 2}; // Two triangles, one using duplicate
    
    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    
    return mesh;
}

/**
 * Test mesh analysis functionality
 * Requirements: Mesh optimization and analysis
 */
bool TestMeshAnalysis() {
    TestOutput::PrintTestStart("mesh analysis");
    
    auto mesh = CreateTestMesh();
    EXPECT_NOT_NULL(mesh);
    
    // Analyze the mesh
    MeshAnalysis analysis = MeshOptimizer::AnalyzeMesh(*mesh);
    
    // Verify analysis results
    EXPECT_EQUAL(analysis.vertexCount, static_cast<uint32_t>(4));
    EXPECT_EQUAL(analysis.triangleCount, static_cast<uint32_t>(2));
    EXPECT_TRUE(analysis.hasNormals);
    EXPECT_TRUE(analysis.hasTextureCoords);
    EXPECT_FALSE(analysis.hasColors);
    EXPECT_FALSE(analysis.hasBoneWeights);
    
    // Check bounding box
    EXPECT_TRUE(analysis.bounds.IsValid());
    
    TestOutput::PrintTestPass("mesh analysis");
    return true;
}

/**
 * Test mesh validation functionality
 * Requirements: Mesh validation and issue detection
 */
bool TestMeshValidation() {
    TestOutput::PrintTestStart("mesh validation");
    
    auto mesh = CreateTestMesh();
    EXPECT_NOT_NULL(mesh);
    
    // Test validation
    bool isValid = MeshOptimizer::ValidateMesh(*mesh);
    EXPECT_TRUE(isValid);
    
    auto issues = MeshOptimizer::GetMeshIssues(*mesh);
    EXPECT_TRUE(issues.empty() || issues.size() <= 1);
    
    TestOutput::PrintTestPass("mesh validation");
    return true;
}

/**
 * Test vertex cache optimization
 * Requirements: Mesh optimization for GPU performance
 */
bool TestVertexCacheOptimization() {
    TestOutput::PrintTestStart("vertex cache optimization");
    
    auto mesh = CreateTestMesh();
    EXPECT_NOT_NULL(mesh);
    
    // Get original indices
    auto originalIndices = mesh->GetIndices();
    
    // Calculate original ACMR
    float originalACMR = MeshOptimizer::CalculateACMR(originalIndices, 32);
    
    // Optimize vertex cache
    MeshOptimizer::OptimizeVertexCache(*mesh);
    
    // Get optimized indices
    auto optimizedIndices = mesh->GetIndices();
    
    // Calculate optimized ACMR
    float optimizedACMR = MeshOptimizer::CalculateACMR(optimizedIndices, 32);
    
    // Verify optimization maintained or improved performance
    EXPECT_TRUE(optimizedACMR <= originalACMR + 0.1f);
    
    // Verify mesh integrity
    EXPECT_EQUAL(originalIndices.size(), optimizedIndices.size());
    EXPECT_TRUE(MeshOptimizer::ValidateMesh(*mesh));
    
    TestOutput::PrintTestPass("vertex cache optimization");
    return true;
}

/**
 * Test mesh simplification functionality
 * Requirements: Mesh LOD generation and simplification
 */
bool TestMeshSimplification() {
    TestOutput::PrintTestStart("mesh simplification");
    
    auto mesh = CreateTestMesh();
    EXPECT_NOT_NULL(mesh);
    
    uint32_t originalTriangleCount = mesh->GetTriangleCount();
    
    // Test simplification with 50% ratio
    auto simplifiedMesh = MeshOptimizer::Simplify(*mesh, 0.5f);
    EXPECT_NOT_NULL(simplifiedMesh);
    
    if (simplifiedMesh) {
        uint32_t simplifiedTriangleCount = simplifiedMesh->GetTriangleCount();
        EXPECT_TRUE(simplifiedTriangleCount <= originalTriangleCount);
        
        // Verify simplified mesh is valid
        EXPECT_TRUE(MeshOptimizer::ValidateMesh(*simplifiedMesh));
    }
    
    TestOutput::PrintTestPass("mesh simplification");
    return true;
}

/**
 * Test duplicate vertex removal
 * Requirements: Mesh optimization and vertex deduplication
 */
bool TestDuplicateVertexRemoval() {
    TestOutput::PrintTestStart("duplicate vertex removal");
    
    auto mesh = CreateTestMesh();
    EXPECT_NOT_NULL(mesh);
    
    uint32_t originalVertexCount = mesh->GetVertexCount();
    
    // Remove duplicate vertices
    MeshOptimizer::RemoveDuplicateVertices(*mesh);
    
    uint32_t optimizedVertexCount = mesh->GetVertexCount();
    
    // Should have fewer vertices after removing duplicates
    EXPECT_TRUE(optimizedVertexCount <= originalVertexCount);
    
    // Mesh should still be valid
    EXPECT_TRUE(MeshOptimizer::ValidateMesh(*mesh));
    
    TestOutput::PrintTestPass("duplicate vertex removal");
    return true;
}

int main() {
    TestOutput::PrintHeader("Mesh Optimizer");

    bool allPassed = true;

    try {
        // Initialize logger for testing
        Logger::GetInstance().SetLogLevel(LogLevel::Info);
        MeshOptimizer::SetVerboseLogging(false); // Keep it quiet for tests
        
        // Create test suite for result tracking
        TestSuite suite("Mesh Optimizer Tests");

        // Run all tests
        allPassed &= suite.RunTest("Mesh Analysis", TestMeshAnalysis);
        allPassed &= suite.RunTest("Mesh Validation", TestMeshValidation);
        allPassed &= suite.RunTest("Vertex Cache Optimization", TestVertexCacheOptimization);
        allPassed &= suite.RunTest("Mesh Simplification", TestMeshSimplification);
        allPassed &= suite.RunTest("Duplicate Vertex Removal", TestDuplicateVertexRemoval);

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