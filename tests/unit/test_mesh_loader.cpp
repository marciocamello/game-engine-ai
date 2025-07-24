#include "Resource/MeshLoader.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestMeshLoaderOBJ() {
    TestOutput::PrintTestStart("OBJ file loading");
    
    // Temporarily set log level to reduce noise during tests
    Logger::GetInstance().SetLogLevel(LogLevel::Error);
    
    // Test loading a simple OBJ file
    MeshLoader::MeshData data = MeshLoader::LoadOBJ("assets/meshes/cube.obj");
    
    // Reset log level
    Logger::GetInstance().SetLogLevel(LogLevel::Info);
    
    EXPECT_TRUE(data.isValid);
    EXPECT_TRUE(data.vertices.size() > 0);
    
    if (data.isValid) {
        TestOutput::PrintTestPass("OBJ file loading - cube.obj loaded with " + 
                                 std::to_string(data.vertices.size()) + " vertices");
        return true;
    } else {
        TestOutput::PrintTestFail("OBJ file loading - failed to load cube.obj: " + data.errorMessage);
        return false;
    }
}

bool TestMeshLoaderCreateDefault() {
    TestOutput::PrintTestStart("Default cube data creation");
    
    try {
        // Test the headless version that doesn't require OpenGL context
        MeshLoader::MeshData cubeData = MeshLoader::CreateDefaultCubeData();
        
        EXPECT_TRUE(cubeData.isValid);
        EXPECT_TRUE(cubeData.vertices.size() > 0);
        EXPECT_TRUE(cubeData.indices.size() > 0);
        EXPECT_EQUAL(cubeData.vertices.size(), 24); // Cube has 24 vertices (6 faces * 4 vertices)
        EXPECT_EQUAL(cubeData.indices.size(), 36);  // Cube has 36 indices (6 faces * 2 triangles * 3 vertices)
        EXPECT_TRUE(cubeData.errorMessage.empty());
        
        // Verify some vertex data
        EXPECT_TRUE(cubeData.vertices[0].position.x != 0.0f || 
                   cubeData.vertices[0].position.y != 0.0f || 
                   cubeData.vertices[0].position.z != 0.0f);
        
        TestOutput::PrintTestPass("Default cube data creation - " + 
                                 std::to_string(cubeData.vertices.size()) + " vertices, " +
                                 std::to_string(cubeData.indices.size()) + " indices");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Default cube data creation - exception: " + std::string(e.what()));
        return false;
    }
}

bool TestMeshLoadFromFile() {
    TestOutput::PrintTestStart("Mesh LoadFromFile method (data only)");
    
    // Test only the data loading, not the OpenGL mesh creation
    // since we don't have an OpenGL context in unit tests
    
    // Temporarily set log level to reduce noise during tests
    Logger::GetInstance().SetLogLevel(LogLevel::Error);
    
    MeshLoader::MeshData data = MeshLoader::LoadOBJ("assets/meshes/cube.obj");
    
    // Reset log level
    Logger::GetInstance().SetLogLevel(LogLevel::Info);
    
    EXPECT_TRUE(data.isValid);
    EXPECT_TRUE(data.vertices.size() > 0);
    EXPECT_TRUE(data.indices.size() > 0);
    
    TestOutput::PrintTestPass("Mesh LoadFromFile method (data only) - loaded " + 
                             std::to_string(data.vertices.size()) + " vertices");
    return true;
}

bool TestMeshLoadFromInvalidFile() {
    TestOutput::PrintTestStart("Invalid file handling");
    
    // Test only the data loading error handling
    // Temporarily set log level to reduce noise during tests
    Logger::GetInstance().SetLogLevel(LogLevel::Critical);
    
    MeshLoader::MeshData data = MeshLoader::LoadOBJ("nonexistent.obj");
    
    // Reset log level
    Logger::GetInstance().SetLogLevel(LogLevel::Info);
    
    // Should return invalid data
    EXPECT_FALSE(data.isValid);
    EXPECT_TRUE(!data.errorMessage.empty());
    
    TestOutput::PrintTestPass("Invalid file handling - properly detected invalid file");
    return true;
}

bool TestMeshCreateDefault() {
    TestOutput::PrintTestStart("Mesh default creation");
    
    try {
        Mesh mesh;
        
        // Test creating default mesh
        mesh.CreateDefault();
        
        // Check that default mesh has vertices and indices
        const auto& vertices = mesh.GetVertices();
        const auto& indices = mesh.GetIndices();
        
        EXPECT_TRUE(vertices.size() > 0);
        EXPECT_TRUE(indices.size() > 0);
        
        // Default cube should have 24 vertices and 36 indices
        EXPECT_EQUAL(vertices.size(), 24);
        EXPECT_EQUAL(indices.size(), 36);
        
        // Verify some vertex data is reasonable
        bool hasValidPositions = false;
        for (const auto& vertex : vertices) {
            if (vertex.position.x != 0.0f || vertex.position.y != 0.0f || vertex.position.z != 0.0f) {
                hasValidPositions = true;
                break;
            }
        }
        EXPECT_TRUE(hasValidPositions);
        
        TestOutput::PrintTestPass("Mesh default creation - created cube with " + 
                                 std::to_string(vertices.size()) + " vertices");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Mesh default creation - exception: " + std::string(e.what()));
        return false;
    }
}

bool TestMeshMemoryUsage() {
    TestOutput::PrintTestStart("Mesh memory usage");
    
    try {
        Mesh mesh;
        
        // Initial memory usage
        size_t initialMemory = mesh.GetMemoryUsage();
        EXPECT_TRUE(initialMemory >= sizeof(Mesh));
        
        // Create default mesh and check memory usage
        mesh.CreateDefault();
        size_t afterDefault = mesh.GetMemoryUsage();
        EXPECT_TRUE(afterDefault > initialMemory);
        
        // Memory should account for vertex and index data
        const auto& vertices = mesh.GetVertices();
        const auto& indices = mesh.GetIndices();
        size_t expectedMinimum = vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint32_t);
        EXPECT_TRUE(afterDefault >= expectedMinimum);
        
        TestOutput::PrintTestPass("Mesh memory usage - " + std::to_string(afterDefault) + " bytes");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Mesh memory usage - exception: " + std::string(e.what()));
        return false;
    }
}

bool TestMeshVertexData() {
    TestOutput::PrintTestStart("Mesh vertex data manipulation");
    
    try {
        Mesh mesh;
        
        // Create test vertex data
        std::vector<Vertex> testVertices = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
        };
        
        std::vector<uint32_t> testIndices = {0, 1, 2};
        
        // Set vertex data
        mesh.SetVertices(testVertices);
        mesh.SetIndices(testIndices);
        
        // Verify data was set correctly
        const auto& vertices = mesh.GetVertices();
        const auto& indices = mesh.GetIndices();
        
        EXPECT_EQUAL(vertices.size(), 3);
        EXPECT_EQUAL(indices.size(), 3);
        
        // Check specific vertex data
        EXPECT_NEARLY_EQUAL(vertices[0].position.x, -1.0f);
        EXPECT_NEARLY_EQUAL(vertices[0].position.y, -1.0f);
        EXPECT_NEARLY_EQUAL(vertices[1].position.x, 1.0f);
        EXPECT_NEARLY_EQUAL(vertices[2].position.y, 1.0f);
        
        // Check indices
        EXPECT_EQUAL(indices[0], 0u);
        EXPECT_EQUAL(indices[1], 1u);
        EXPECT_EQUAL(indices[2], 2u);
        
        TestOutput::PrintTestPass("Mesh vertex data manipulation");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Mesh vertex data manipulation - exception: " + std::string(e.what()));
        return false;
    }
}

bool TestMeshCleanup() {
    TestOutput::PrintTestStart("Mesh cleanup");
    
    try {
        Mesh mesh;
        mesh.CreateDefault();
        
        // Verify mesh has data
        EXPECT_TRUE(mesh.GetVertices().size() > 0);
        EXPECT_TRUE(mesh.GetIndices().size() > 0);
        
        // Test cleanup (should not crash)
        mesh.Cleanup();
        
        // Note: We can't verify OpenGL resource cleanup without context,
        // but the method should not crash
        
        TestOutput::PrintTestPass("Mesh cleanup");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Mesh cleanup - exception: " + std::string(e.what()));
        return false;
    }
}

bool TestMeshBindingOperations() {
    TestOutput::PrintTestStart("Mesh binding operations");
    
    try {
        Mesh mesh;
        mesh.CreateDefault();
        
        // Test binding operations (should not crash without OpenGL context)
        mesh.Bind();
        mesh.Unbind();
        
        // Test draw operation (should not crash without OpenGL context)
        mesh.Draw();
        
        TestOutput::PrintTestPass("Mesh binding operations");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Mesh binding operations - exception: " + std::string(e.what()));
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Mesh Loader Tests");
    Logger::GetInstance().Initialize();
    
    TestSuite suite("Mesh Loader Tests");
    
    bool allPassed = true;
    
    try {
        allPassed &= suite.RunTest("OBJ Loading", TestMeshLoaderOBJ);
        allPassed &= suite.RunTest("Default Cube Creation", TestMeshLoaderCreateDefault);
        allPassed &= suite.RunTest("Mesh LoadFromFile", TestMeshLoadFromFile);
        allPassed &= suite.RunTest("Mesh LoadFromFile Invalid", TestMeshLoadFromInvalidFile);
        allPassed &= suite.RunTest("Mesh CreateDefault", TestMeshCreateDefault);
        allPassed &= suite.RunTest("Mesh Memory Usage", TestMeshMemoryUsage);
        allPassed &= suite.RunTest("Mesh Vertex Data", TestMeshVertexData);
        allPassed &= suite.RunTest("Mesh Cleanup", TestMeshCleanup);
        allPassed &= suite.RunTest("Mesh Binding Operations", TestMeshBindingOperations);
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Exception caught: " + std::string(e.what()));
        allPassed = false;
    }
    
    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}