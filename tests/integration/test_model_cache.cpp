#include "../TestUtils.h"
#include "Resource/ModelCache.h"
#include "Resource/ModelLoader.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <filesystem>
#include <chrono>
#include <thread>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestModelCacheInitialization() {
    TestOutput::PrintTestStart("model cache initialization");

    ModelCache cache;
    
    // Test initialization
    EXPECT_TRUE(cache.Initialize("test_cache"));
    EXPECT_TRUE(cache.IsInitialized());
    
    // Test directory creation
    EXPECT_TRUE(std::filesystem::exists("test_cache"));
    
    // Test shutdown
    cache.Shutdown();
    EXPECT_FALSE(cache.IsInitialized());
    
    // Cleanup
    std::filesystem::remove_all("test_cache");

    TestOutput::PrintTestPass("model cache initialization");
    return true;
}

bool TestModelCacheBasicOperations() {
    TestOutput::PrintTestStart("model cache basic operations");

    ModelCache cache;
    EXPECT_TRUE(cache.Initialize("test_cache"));

    // Create a test model
    auto model = std::make_shared<Model>("test_model.obj");
    model->SetName("TestModel");
    
    // Create a simple mesh for the model
    auto mesh = std::make_shared<Mesh>("test_mesh");
    mesh->CreateDefault(); // This should create a default cube
    model->AddMesh(mesh);

    std::string testPath = "test_model.obj";
    
    // Test that model is not cached initially
    EXPECT_FALSE(cache.IsCached(testPath));
    EXPECT_FALSE(cache.IsValidCache(testPath));
    
    // Create a dummy file for the test
    std::ofstream dummyFile(testPath);
    dummyFile << "# Test OBJ file\n";
    dummyFile.close();
    
    // Save model to cache
    EXPECT_TRUE(cache.SaveToCache(testPath, model));
    
    // Test that model is now cached
    EXPECT_TRUE(cache.IsCached(testPath));
    EXPECT_TRUE(cache.IsValidCache(testPath));
    
    // Load model from cache
    auto cachedModel = cache.LoadFromCache(testPath);
    EXPECT_NOT_NULL(cachedModel);
    
    if (cachedModel) {
        EXPECT_EQUAL(cachedModel->GetName(), "TestModel");
        EXPECT_EQUAL(cachedModel->GetMeshCount(), static_cast<size_t>(1));
    }
    
    // Test cache invalidation
    cache.InvalidateCache(testPath);
    EXPECT_FALSE(cache.IsCached(testPath));
    
    // Cleanup
    std::filesystem::remove(testPath);
    cache.Shutdown();
    std::filesystem::remove_all("test_cache");

    TestOutput::PrintTestPass("model cache basic operations");
    return true;
}

bool TestModelCacheVersionCompatibility() {
    TestOutput::PrintTestStart("model cache version compatibility");

    ModelCache cache;
    EXPECT_TRUE(cache.Initialize("test_cache"));

    // Test cache key generation
    std::string path1 = "model1.obj";
    std::string path2 = "model2.obj";
    
    std::string key1 = ModelCache::GenerateCacheKey(path1);
    std::string key2 = ModelCache::GenerateCacheKey(path2);
    
    EXPECT_NOT_EQUAL(key1, key2);
    
    // Test same path generates same key
    std::string key1_again = ModelCache::GenerateCacheKey(path1);
    EXPECT_EQUAL(key1, key1_again);

    cache.Shutdown();
    std::filesystem::remove_all("test_cache");

    TestOutput::PrintTestPass("model cache version compatibility");
    return true;
}

bool TestModelCacheStatistics() {
    TestOutput::PrintTestStart("model cache statistics");

    ModelCache cache;
    EXPECT_TRUE(cache.Initialize("test_cache"));

    // Get initial stats
    auto initialStats = cache.GetStats();
    EXPECT_EQUAL(initialStats.totalEntries, static_cast<uint32_t>(0));
    EXPECT_EQUAL(initialStats.cacheHits, static_cast<uint32_t>(0));
    EXPECT_EQUAL(initialStats.cacheMisses, static_cast<uint32_t>(0));

    // Create test model and file
    auto model = std::make_shared<Model>("test_stats.obj");
    auto mesh = std::make_shared<Mesh>("test_mesh");
    mesh->CreateDefault();
    model->AddMesh(mesh);

    std::string testPath = "test_stats.obj";
    std::ofstream dummyFile(testPath);
    dummyFile << "# Test OBJ file\n";
    dummyFile.close();

    // Save to cache
    EXPECT_TRUE(cache.SaveToCache(testPath, model));

    // Check stats after save
    auto afterSaveStats = cache.GetStats();
    EXPECT_EQUAL(afterSaveStats.totalEntries, static_cast<uint32_t>(1));
    EXPECT_EQUAL(afterSaveStats.validEntries, static_cast<uint32_t>(1));

    // Load from cache (should increment cache hits)
    auto cachedModel = cache.LoadFromCache(testPath);
    EXPECT_NOT_NULL(cachedModel);

    auto afterLoadStats = cache.GetStats();
    EXPECT_EQUAL(afterLoadStats.cacheHits, static_cast<uint32_t>(1));

    // Try to load non-existent model (should increment cache misses)
    auto nonExistentModel = cache.LoadFromCache("non_existent.obj");
    EXPECT_NULL(nonExistentModel);

    auto finalStats = cache.GetStats();
    EXPECT_EQUAL(finalStats.cacheMisses, static_cast<uint32_t>(1));

    // Cleanup
    std::filesystem::remove(testPath);
    cache.Shutdown();
    std::filesystem::remove_all("test_cache");

    TestOutput::PrintTestPass("model cache statistics");
    return true;
}

bool TestModelLoaderCacheIntegration() {
    TestOutput::PrintTestStart("model loader cache integration");

    // Create a simple OBJ file for testing
    std::string testObjPath = "test_integration.obj";
    std::ofstream objFile(testObjPath);
    objFile << "# Test OBJ file\n";
    objFile << "v 0.0 0.0 0.0\n";
    objFile << "v 1.0 0.0 0.0\n";
    objFile << "v 0.0 1.0 0.0\n";
    objFile << "f 1 2 3\n";
    objFile.close();

    ModelLoader loader;
    EXPECT_TRUE(loader.Initialize());
    
    // Ensure caching is enabled
    loader.SetCacheEnabled(true);
    EXPECT_TRUE(loader.IsCacheEnabled());

    // Load model first time (should cache it)
    auto model1 = loader.LoadModelAsResource(testObjPath);
    EXPECT_NOT_NULL(model1);

    // Check if model is cached
    auto& cache = GlobalModelCache::GetInstance();
    EXPECT_TRUE(cache.IsCached(testObjPath));

    // Load model second time (should load from cache)
    auto model2 = loader.LoadModelAsResource(testObjPath);
    EXPECT_NOT_NULL(model2);

    // Test cache invalidation
    loader.InvalidateCache(testObjPath);
    EXPECT_FALSE(cache.IsCached(testObjPath));

    // Test cache clearing
    auto model3 = loader.LoadModelAsResource(testObjPath);
    EXPECT_NOT_NULL(model3);
    EXPECT_TRUE(cache.IsCached(testObjPath));

    loader.ClearAllCache();
    EXPECT_FALSE(cache.IsCached(testObjPath));

    // Cleanup
    std::filesystem::remove(testObjPath);
    loader.Shutdown();

    TestOutput::PrintTestPass("model loader cache integration");
    return true;
}

bool TestGlobalModelCache() {
    TestOutput::PrintTestStart("global model cache");

    // Test singleton behavior
    auto& cache1 = GlobalModelCache::GetInstance();
    auto& cache2 = GlobalModelCache::GetInstance();
    
    // Should be the same instance
    EXPECT_EQUAL(&cache1, &cache2);

    // Test initialization
    EXPECT_TRUE(cache1.Initialize("test_global_cache"));
    EXPECT_TRUE(cache1.IsInitialized());

    // Test that second instance is also initialized
    EXPECT_TRUE(cache2.IsInitialized());

    cache1.Shutdown();
    std::filesystem::remove_all("test_global_cache");

    TestOutput::PrintTestPass("global model cache");
    return true;
}

int main() {
    TestOutput::PrintHeader("Model Cache Integration Tests");

    TestSuite suite("Model Cache Integration Tests");

    try {
        suite.RunTest("Model Cache Initialization", TestModelCacheInitialization);
        suite.RunTest("Model Cache Basic Operations", TestModelCacheBasicOperations);
        suite.RunTest("Model Cache Version Compatibility", TestModelCacheVersionCompatibility);
        suite.RunTest("Model Cache Statistics", TestModelCacheStatistics);
        suite.RunTest("Model Loader Cache Integration", TestModelLoaderCacheIntegration);
        suite.RunTest("Global Model Cache", TestGlobalModelCache);

        suite.PrintSummary();
        return suite.AllTestsPassed() ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception: " + std::string(e.what()));
        return 1;
    }
}