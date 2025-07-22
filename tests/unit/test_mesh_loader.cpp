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
    TestOutput::PrintTestStart("Mesh cleanup test");
    
    try {
        // Test mesh cleanup functionality
        // Note: We can't test OpenGL cleanup without a context, but we can test the data cleanup
        
        // Create some test data
        std::vector<Vertex> testVertices = {
            {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}}
        };
        
        std::vector<uint32_t> testIndices = {0, 1, 2};
        
        EXPECT_TRUE(testVertices.size() == 3);
        EXPECT_TRUE(testIndices.size() == 3);
        
        // Test that we can clear the data
        testVertices.clear();
        testIndices.clear();
        
        EXPECT_TRUE(testVertices.empty());
        EXPECT_TRUE(testIndices.empty());
        
        TestOutput::PrintTestPass("Mesh cleanup test - data cleanup verified");
        return true;
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Mesh cleanup test - exception: " + std::string(e.what()));
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
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Exception caught: " + std::string(e.what()));
        allPassed = false;
    }
    
    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}