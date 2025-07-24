#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <filesystem>
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestTextureFallbackCreation() {
    TestOutput::PrintTestStart("Texture fallback creation");

    Texture texture;
    
    // Test creating fallback texture
    texture.CreateDefault();
    
    // Fallback texture should be valid
    EXPECT_TRUE(texture.IsValid());
    EXPECT_TRUE(texture.GetWidth() > 0);
    EXPECT_TRUE(texture.GetHeight() > 0);
    EXPECT_TRUE(texture.GetChannels() > 0);
    
    // Should have reasonable dimensions (typically power of 2)
    int width = texture.GetWidth();
    int height = texture.GetHeight();
    EXPECT_TRUE(width >= 1 && width <= 1024);
    EXPECT_TRUE(height >= 1 && height <= 1024);
    
    // Should have 3 or 4 channels (RGB or RGBA)
    int channels = texture.GetChannels();
    EXPECT_TRUE(channels == 3 || channels == 4);

    TestOutput::PrintTestPass("Texture fallback creation - " + 
                             std::to_string(width) + "x" + std::to_string(height) + 
                             " with " + std::to_string(channels) + " channels");
    return true;
}

bool TestMeshFallbackCreation() {
    TestOutput::PrintTestStart("Mesh fallback creation");

    Mesh mesh;
    
    // Test creating fallback mesh
    mesh.CreateDefault();
    
    // Fallback mesh should have vertices and indices
    const auto& vertices = mesh.GetVertices();
    const auto& indices = mesh.GetIndices();
    
    EXPECT_TRUE(vertices.size() > 0);
    EXPECT_TRUE(indices.size() > 0);
    
    // Should be a reasonable cube mesh
    EXPECT_EQUAL(vertices.size(), 24); // 6 faces * 4 vertices
    EXPECT_EQUAL(indices.size(), 36);  // 6 faces * 2 triangles * 3 vertices
    
    // Verify vertex data is reasonable
    bool hasValidPositions = false;
    bool hasValidNormals = false;
    bool hasValidTexCoords = false;
    
    for (const auto& vertex : vertices) {
        // Check positions are within reasonable bounds
        if (std::abs(vertex.position.x) <= 2.0f && 
            std::abs(vertex.position.y) <= 2.0f && 
            std::abs(vertex.position.z) <= 2.0f) {
            hasValidPositions = true;
        }
        
        // Check normals are unit vectors (approximately)
        float normalLength = sqrt(vertex.normal.x * vertex.normal.x + 
                                vertex.normal.y * vertex.normal.y + 
                                vertex.normal.z * vertex.normal.z);
        if (normalLength > 0.9f && normalLength < 1.1f) {
            hasValidNormals = true;
        }
        
        // Check texture coordinates are in valid range
        if (vertex.texCoords.x >= 0.0f && vertex.texCoords.x <= 1.0f &&
            vertex.texCoords.y >= 0.0f && vertex.texCoords.y <= 1.0f) {
            hasValidTexCoords = true;
        }
    }
    
    EXPECT_TRUE(hasValidPositions);
    EXPECT_TRUE(hasValidNormals);
    EXPECT_TRUE(hasValidTexCoords);

    TestOutput::PrintTestPass("Mesh fallback creation - cube with " + 
                             std::to_string(vertices.size()) + " vertices");
    return true;
}

bool TestResourceManagerFallbackBehavior() {
    TestOutput::PrintTestStart("ResourceManager fallback behavior");

    ResourceManager manager;
    manager.Initialize();
    
    // Ensure fallback resources are enabled
    manager.SetFallbackResourcesEnabled(true);
    EXPECT_TRUE(manager.IsFallbackResourcesEnabled());
    
    // Try to load non-existent texture
    auto texture = manager.Load<Texture>("nonexistent_texture.png");
    
    // Should create fallback texture
    EXPECT_NOT_NULL(texture);
    if (texture) {
        EXPECT_TRUE(texture->IsValid());
        EXPECT_TRUE(texture->GetWidth() > 0);
        EXPECT_TRUE(texture->GetHeight() > 0);
    }
    
    // Try to load non-existent mesh
    auto mesh = manager.Load<Mesh>("nonexistent_mesh.obj");
    
    // Should create fallback mesh
    EXPECT_NOT_NULL(mesh);
    if (mesh) {
        EXPECT_TRUE(mesh->GetVertices().size() > 0);
        EXPECT_TRUE(mesh->GetIndices().size() > 0);
    }
    
    manager.Shutdown();

    TestOutput::PrintTestPass("ResourceManager fallback behavior");
    return true;
}

bool TestResourceManagerFallbackDisabled() {
    TestOutput::PrintTestStart("ResourceManager with fallback disabled");

    ResourceManager manager;
    manager.Initialize();
    
    // Disable fallback resources
    manager.SetFallbackResourcesEnabled(false);
    EXPECT_FALSE(manager.IsFallbackResourcesEnabled());
    
    // Try to load non-existent texture
    auto texture = manager.Load<Texture>("nonexistent_texture.png");
    
    // Should return null when fallback is disabled
    EXPECT_NULL(texture);
    
    // Try to load non-existent mesh
    auto mesh = manager.Load<Mesh>("nonexistent_mesh.obj");
    
    // Should return null when fallback is disabled
    EXPECT_NULL(mesh);
    
    manager.Shutdown();

    TestOutput::PrintTestPass("ResourceManager with fallback disabled");
    return true;
}

bool TestResourceLoadFailureHandling() {
    TestOutput::PrintTestStart("Resource load failure handling");

    ResourceManager manager;
    manager.Initialize();
    
    // Create a corrupted file
    std::filesystem::create_directories("assets");
    const std::string corruptedFile = "assets/corrupted.png";
    std::ofstream file(corruptedFile, std::ios::binary);
    file.write("CORRUPTED_DATA", 14);
    file.close();
    
    // Try to load corrupted file
    auto texture = manager.Load<Texture>("corrupted.png");
    
    if (manager.IsFallbackResourcesEnabled()) {
        // Should create fallback resource
        EXPECT_NOT_NULL(texture);
        if (texture) {
            EXPECT_TRUE(texture->IsValid());
        }
    } else {
        // Should return null
        EXPECT_NULL(texture);
    }
    
    // Test error handling method
    manager.HandleResourceLoadFailure("test.png", "Test error message");
    
    // Clean up
    std::filesystem::remove(corruptedFile);
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource load failure handling");
    return true;
}

bool TestResourceMemoryPressureHandling() {
    TestOutput::PrintTestStart("Resource memory pressure handling");

    ResourceManager manager;
    manager.Initialize();
    
    // Set a low memory pressure threshold
    manager.SetMemoryPressureThreshold(1024); // 1KB
    
    // Load several resources to trigger memory pressure
    std::vector<std::shared_ptr<Texture>> textures;
    for (int i = 0; i < 10; ++i) {
        auto texture = manager.Load<Texture>("test_texture_" + std::to_string(i) + ".png");
        if (texture) {
            textures.push_back(texture);
        }
    }
    
    // Test memory pressure handling
    manager.HandleMemoryPressure();
    manager.CheckMemoryPressure();
    
    // Test LRU cleanup
    manager.UnloadLeastRecentlyUsed(512); // Try to free 512 bytes
    
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource memory pressure handling");
    return true;
}

bool TestResourceErrorRecovery() {
    TestOutput::PrintTestStart("Resource error recovery");

    ResourceManager manager;
    manager.Initialize();
    
    // Test various error conditions
    
    // 1. Empty filename
    auto texture1 = manager.Load<Texture>("");
    EXPECT_NULL(texture1);
    
    // 2. Invalid characters in filename
    auto texture2 = manager.Load<Texture>("invalid\0filename.png");
    EXPECT_NULL(texture2);
    
    // 3. Very long filename
    std::string longFilename(1000, 'a');
    longFilename += ".png";
    auto texture3 = manager.Load<Texture>(longFilename);
    // Should handle gracefully (either load fallback or return null)
    
    // 4. Directory instead of file
    std::filesystem::create_directories("assets/test_directory");
    auto texture4 = manager.Load<Texture>("test_directory");
    // Should handle gracefully
    
    // Clean up
    std::filesystem::remove_all("assets/test_directory");
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource error recovery");
    return true;
}

bool TestResourceStatisticsWithFallbacks() {
    TestOutput::PrintTestStart("Resource statistics with fallbacks");

    ResourceManager manager;
    manager.Initialize();
    
    // Load some resources (will create fallbacks)
    auto texture1 = manager.Load<Texture>("fallback_test1.png");
    auto texture2 = manager.Load<Texture>("fallback_test2.png");
    auto mesh1 = manager.Load<Mesh>("fallback_test1.obj");
    
    // Get statistics
    ResourceStats stats = manager.GetResourceStats();
    size_t resourceCount = manager.GetResourceCount();
    size_t memoryUsage = manager.GetMemoryUsage();
    
    // Should have some resources loaded
    EXPECT_TRUE(resourceCount > 0);
    EXPECT_TRUE(memoryUsage > 0);
    EXPECT_TRUE(stats.totalResources > 0);
    EXPECT_TRUE(stats.totalMemoryUsage > 0);
    
    // Test logging (should not crash)
    manager.LogResourceUsage();
    manager.LogDetailedResourceInfo();
    
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource statistics with fallbacks");
    return true;
}

bool TestResourceFallbackMemoryUsage() {
    TestOutput::PrintTestStart("Resource fallback memory usage");

    // Test individual resource memory usage
    Texture texture;
    size_t initialTextureMemory = texture.GetMemoryUsage();
    
    texture.CreateDefault();
    size_t fallbackTextureMemory = texture.GetMemoryUsage();
    
    EXPECT_TRUE(fallbackTextureMemory > initialTextureMemory);
    
    Mesh mesh;
    size_t initialMeshMemory = mesh.GetMemoryUsage();
    
    mesh.CreateDefault();
    size_t fallbackMeshMemory = mesh.GetMemoryUsage();
    
    EXPECT_TRUE(fallbackMeshMemory > initialMeshMemory);
    
    // Memory usage should be reasonable (not extremely large)
    EXPECT_TRUE(fallbackTextureMemory < 10 * 1024 * 1024); // Less than 10MB
    EXPECT_TRUE(fallbackMeshMemory < 1024 * 1024);         // Less than 1MB

    TestOutput::PrintTestPass("Resource fallback memory usage - Texture: " + 
                             std::to_string(fallbackTextureMemory) + " bytes, Mesh: " + 
                             std::to_string(fallbackMeshMemory) + " bytes");
    return true;
}

bool TestResourceFallbackConsistency() {
    TestOutput::PrintTestStart("Resource fallback consistency");

    // Test that fallback resources are consistent across multiple creations
    Texture texture1, texture2;
    texture1.CreateDefault();
    texture2.CreateDefault();
    
    // Should have same dimensions
    EXPECT_EQUAL(texture1.GetWidth(), texture2.GetWidth());
    EXPECT_EQUAL(texture1.GetHeight(), texture2.GetHeight());
    EXPECT_EQUAL(texture1.GetChannels(), texture2.GetChannels());
    
    Mesh mesh1, mesh2;
    mesh1.CreateDefault();
    mesh2.CreateDefault();
    
    // Should have same vertex/index counts
    EXPECT_EQUAL(mesh1.GetVertices().size(), mesh2.GetVertices().size());
    EXPECT_EQUAL(mesh1.GetIndices().size(), mesh2.GetIndices().size());
    
    // Vertex data should be identical
    const auto& vertices1 = mesh1.GetVertices();
    const auto& vertices2 = mesh2.GetVertices();
    
    for (size_t i = 0; i < vertices1.size() && i < vertices2.size(); ++i) {
        EXPECT_NEAR_VEC3(vertices1[i].position, vertices2[i].position);
        EXPECT_NEAR_VEC3(vertices1[i].normal, vertices2[i].normal);
    }

    TestOutput::PrintTestPass("Resource fallback consistency");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Fallback Tests");
    Logger::GetInstance().Initialize();

    TestSuite suite("Resource Fallback Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("Texture Fallback Creation", TestTextureFallbackCreation);
    allPassed &= suite.RunTest("Mesh Fallback Creation", TestMeshFallbackCreation);
    allPassed &= suite.RunTest("ResourceManager Fallback Behavior", TestResourceManagerFallbackBehavior);
    allPassed &= suite.RunTest("ResourceManager Fallback Disabled", TestResourceManagerFallbackDisabled);
    allPassed &= suite.RunTest("Resource Load Failure Handling", TestResourceLoadFailureHandling);
    allPassed &= suite.RunTest("Resource Memory Pressure Handling", TestResourceMemoryPressureHandling);
    allPassed &= suite.RunTest("Resource Error Recovery", TestResourceErrorRecovery);
    allPassed &= suite.RunTest("Resource Statistics with Fallbacks", TestResourceStatisticsWithFallbacks);
    allPassed &= suite.RunTest("Resource Fallback Memory Usage", TestResourceFallbackMemoryUsage);
    allPassed &= suite.RunTest("Resource Fallback Consistency", TestResourceFallbackConsistency);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}