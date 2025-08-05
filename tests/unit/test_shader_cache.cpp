#include "TestUtils.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderVariant.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test ShaderCache initialization and basic functionality
 * Requirements: 4.4, 9.2, 9.5 (shader caching system with variant support)
 */
bool TestShaderCacheInitialization() {
    TestOutput::PrintTestStart("shader cache initialization");

    ShaderCache cache;
    ShaderCacheConfig config;
    config.maxEntries = 100;
    config.maxMemoryUsage = 1024 * 1024; // 1MB
    config.enablePersistentCache = false; // Disable for testing
    
    EXPECT_TRUE(cache.Initialize(config));
    
    // Test configuration
    ShaderCacheConfig retrievedConfig = cache.GetConfig();
    EXPECT_EQUAL(retrievedConfig.maxEntries, 100);
    EXPECT_EQUAL(retrievedConfig.maxMemoryUsage, 1024 * 1024);
    EXPECT_FALSE(retrievedConfig.enablePersistentCache);
    
    // Test initial stats
    ShaderCacheStats stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 0);
    EXPECT_EQUAL(stats.hitCount, 0);
    EXPECT_EQUAL(stats.missCount, 0);
    
    cache.Shutdown();

    TestOutput::PrintTestPass("shader cache initialization");
    return true;
}

/**
 * Test basic cache operations (store and retrieve)
 * Requirements: 4.4, 9.2 (cache invalidation and cleanup mechanisms)
 */
bool TestBasicCacheOperations() {
    TestOutput::PrintTestStart("basic cache operations");

    ShaderCache cache;
    ShaderCacheConfig config;
    config.enablePersistentCache = false;
    EXPECT_TRUE(cache.Initialize(config));
    
    // Create a mock shader (we can't create a real one without OpenGL context)
    auto mockShader = std::make_shared<Shader>();
    
    // Test storing shader
    cache.StoreShader("test_shader", mockShader, "hash123", false);
    
    // Test cache stats after storing
    ShaderCacheStats stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 1);
    EXPECT_EQUAL(stats.temporaryEntries, 1);
    EXPECT_EQUAL(stats.persistentEntries, 0);
    
    // Test retrieving shader
    EXPECT_TRUE(cache.HasShader("test_shader", "hash123"));
    auto retrievedShader = cache.GetShader("test_shader", "hash123");
    EXPECT_NOT_NULL(retrievedShader);
    EXPECT_EQUAL(retrievedShader, mockShader);
    
    // Test cache hit statistics
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.hitCount, 1);
    EXPECT_EQUAL(stats.missCount, 0);
    
    // Test cache miss
    auto missingShader = cache.GetShader("nonexistent_shader", "hash456");
    EXPECT_NULL(missingShader);
    
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.hitCount, 1);
    EXPECT_EQUAL(stats.missCount, 1);
    
    cache.Shutdown();

    TestOutput::PrintTestPass("basic cache operations");
    return true;
}

/**
 * Test shader variant caching
 * Requirements: 4.4 (shader caching system with variant support)
 */
bool TestShaderVariantCaching() {
    TestOutput::PrintTestStart("shader variant caching");

    ShaderCache cache;
    ShaderCacheConfig config;
    config.enablePersistentCache = false;
    config.enableVariantCaching = true;
    EXPECT_TRUE(cache.Initialize(config));
    
    // Create a mock shader and variant
    auto mockShader = std::make_shared<Shader>();
    ShaderVariant variant;
    variant.AddDefine("USE_LIGHTING", "1");
    variant.AddFeature("PBR");
    
    // Test storing shader variant
    cache.StoreShaderVariant("base_shader", variant, mockShader, "hash123", false);
    
    // Test cache stats
    ShaderCacheStats stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 1);
    
    // Test retrieving shader variant
    EXPECT_TRUE(cache.HasShaderVariant("base_shader", variant, "hash123"));
    auto retrievedShader = cache.GetShaderVariant("base_shader", variant, "hash123");
    EXPECT_NOT_NULL(retrievedShader);
    EXPECT_EQUAL(retrievedShader, mockShader);
    
    // Test different variant should miss
    ShaderVariant differentVariant;
    differentVariant.AddDefine("USE_SHADOWS", "1");
    auto missingVariant = cache.GetShaderVariant("base_shader", differentVariant, "hash123");
    EXPECT_NULL(missingVariant);
    
    cache.Shutdown();

    TestOutput::PrintTestPass("shader variant caching");
    return true;
}

/**
 * Test cache eviction policies
 * Requirements: 9.2 (cache invalidation and cleanup mechanisms)
 */
bool TestCacheEviction() {
    TestOutput::PrintTestStart("cache eviction");

    ShaderCache cache;
    ShaderCacheConfig config;
    config.maxEntries = 2; // Small limit to test eviction
    config.enablePersistentCache = false;
    config.evictionPolicy = CacheEvictionPolicy::LRU;
    EXPECT_TRUE(cache.Initialize(config));
    
    // Create mock shaders
    auto shader1 = std::make_shared<Shader>();
    auto shader2 = std::make_shared<Shader>();
    auto shader3 = std::make_shared<Shader>();
    
    // Store shaders up to limit
    cache.StoreShader("shader1", shader1, "hash1", false);
    cache.StoreShader("shader2", shader2, "hash2", false);
    
    ShaderCacheStats stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 2);
    
    // Access shader1 to make it more recently used
    cache.GetShader("shader1", "hash1");
    
    // Store third shader, should evict shader2 (least recently used)
    cache.StoreShader("shader3", shader3, "hash3", false);
    
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 2); // Should still be at limit
    
    // shader1 and shader3 should exist, shader2 should be evicted
    EXPECT_TRUE(cache.HasShader("shader1", "hash1"));
    EXPECT_TRUE(cache.HasShader("shader3", "hash3"));
    EXPECT_FALSE(cache.HasShader("shader2", "hash2"));
    
    cache.Shutdown();

    TestOutput::PrintTestPass("cache eviction");
    return true;
}

/**
 * Test cache clearing and removal operations
 * Requirements: 9.2 (cache invalidation and cleanup mechanisms)
 */
bool TestCacheClearing() {
    TestOutput::PrintTestStart("cache clearing");

    ShaderCache cache;
    ShaderCacheConfig config;
    config.enablePersistentCache = false;
    EXPECT_TRUE(cache.Initialize(config));
    
    // Store some shaders
    auto shader1 = std::make_shared<Shader>();
    auto shader2 = std::make_shared<Shader>();
    
    cache.StoreShader("shader1", shader1, "hash1", false); // temporary
    cache.StoreShader("shader2", shader2, "hash2", true);  // persistent
    
    ShaderCacheStats stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 2);
    EXPECT_EQUAL(stats.temporaryEntries, 1);
    EXPECT_EQUAL(stats.persistentEntries, 1);
    
    // Clear temporary entries only
    cache.ClearTemporaryEntries();
    
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 1);
    EXPECT_EQUAL(stats.temporaryEntries, 0);
    EXPECT_EQUAL(stats.persistentEntries, 1);
    
    // shader1 should be gone, shader2 should remain
    EXPECT_FALSE(cache.HasShader("shader1", "hash1"));
    EXPECT_TRUE(cache.HasShader("shader2", "hash2"));
    
    // Clear all
    cache.ClearCache();
    
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 0);
    EXPECT_EQUAL(stats.temporaryEntries, 0);
    EXPECT_EQUAL(stats.persistentEntries, 0);
    
    cache.Shutdown();

    TestOutput::PrintTestPass("cache clearing");
    return true;
}

/**
 * Test cache statistics and monitoring
 * Requirements: 9.5 (precompilation system for faster startup)
 */
bool TestCacheStatistics() {
    TestOutput::PrintTestStart("cache statistics");

    ShaderCache cache;
    ShaderCacheConfig config;
    config.enablePersistentCache = false;
    config.enableStatistics = true;
    EXPECT_TRUE(cache.Initialize(config));
    
    // Initial stats should be zero
    ShaderCacheStats stats = cache.GetStats();
    EXPECT_EQUAL(stats.totalEntries, 0);
    EXPECT_EQUAL(stats.hitCount, 0);
    EXPECT_EQUAL(stats.missCount, 0);
    EXPECT_NEARLY_EQUAL(stats.hitRatio, 0.0f);
    
    // Store and access shaders to generate statistics
    auto shader = std::make_shared<Shader>();
    cache.StoreShader("test_shader", shader, "hash", false);
    
    // Generate hits and misses
    cache.GetShader("test_shader", "hash");     // hit
    cache.GetShader("test_shader", "hash");     // hit
    cache.GetShader("missing_shader", "hash");  // miss
    
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.hitCount, 2);
    EXPECT_EQUAL(stats.missCount, 1);
    EXPECT_NEARLY_EQUAL(stats.hitRatio, 2.0f / 3.0f);
    
    // Test stats reset
    cache.ResetStats();
    stats = cache.GetStats();
    EXPECT_EQUAL(stats.hitCount, 0);
    EXPECT_EQUAL(stats.missCount, 0);
    EXPECT_NEARLY_EQUAL(stats.hitRatio, 0.0f);
    
    cache.Shutdown();

    TestOutput::PrintTestPass("cache statistics");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderCache");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderCache Tests");

        // Run all tests
        allPassed &= suite.RunTest("Shader Cache Initialization", TestShaderCacheInitialization);
        allPassed &= suite.RunTest("Basic Cache Operations", TestBasicCacheOperations);
        allPassed &= suite.RunTest("Shader Variant Caching", TestShaderVariantCaching);
        allPassed &= suite.RunTest("Cache Eviction", TestCacheEviction);
        allPassed &= suite.RunTest("Cache Clearing", TestCacheClearing);
        allPassed &= suite.RunTest("Cache Statistics", TestCacheStatistics);

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