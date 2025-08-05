#include "../TestUtils.h"
#include "Graphics/ShaderVariantManager.h"
#include "Graphics/ShaderVariant.h"
#include "Graphics/ShaderManager.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestShaderVariantManagerInitialization() {
    TestOutput::PrintTestStart("shader variant manager initialization");

    auto& variantManager = ShaderVariantManager::GetInstance();
    
    // Initialize should succeed
    EXPECT_TRUE(variantManager.Initialize());
    
    // Double initialization should not fail
    EXPECT_TRUE(variantManager.Initialize());
    
    // Basic state checks
    EXPECT_EQUAL(variantManager.GetVariantCount(), 0);
    EXPECT_FALSE(variantManager.IsDebugMode());
    
    variantManager.Shutdown();

    TestOutput::PrintTestPass("shader variant manager initialization");
    return true;
}

bool TestShaderVariantManagerBasicOperations() {
    TestOutput::PrintTestStart("shader variant manager basic operations");

    auto& variantManager = ShaderVariantManager::GetInstance();
    EXPECT_TRUE(variantManager.Initialize());
    
    // Test variant count operations
    EXPECT_EQUAL(variantManager.GetVariantCount(), 0);
    EXPECT_EQUAL(variantManager.GetVariantCount("test_shader"), 0);
    
    // Test base shader names (should be empty initially)
    auto baseNames = variantManager.GetBaseShaderNames();
    EXPECT_EQUAL(baseNames.size(), 0);
    
    // Test cache operations
    variantManager.SetMaxCacheSize(500);
    variantManager.ClearVariantCache();
    EXPECT_EQUAL(variantManager.GetVariantCount(), 0);
    
    variantManager.Shutdown();

    TestOutput::PrintTestPass("shader variant manager basic operations");
    return true;
}

bool TestRenderContextVariantGeneration() {
    TestOutput::PrintTestStart("render context variant generation");

    auto& variantManager = ShaderVariantManager::GetInstance();
    EXPECT_TRUE(variantManager.Initialize());
    
    // Create test render context
    RenderContext context;
    context.hasDirectionalLight = true;
    context.pointLightCount = 4;
    context.hasAlbedoMap = true;
    context.hasNormalMap = true;
    context.hasSkinning = true;
    context.maxBones = 32;
    context.useDebugMode = false;
    context.useOptimizedPath = true;
    
    // Generate variant from context
    ShaderVariant variant = variantManager.GenerateVariantFromContext(context);
    
    // Check that variant has expected defines and features
    EXPECT_TRUE(variant.HasDefine("HAS_DIRECTIONAL_LIGHT"));
    EXPECT_TRUE(variant.HasDefine("HAS_POINT_LIGHTS"));
    EXPECT_EQUAL(variant.GetDefineValue("MAX_POINT_LIGHTS"), "4");
    EXPECT_TRUE(variant.HasDefine("HAS_ALBEDO_MAP"));
    EXPECT_TRUE(variant.HasDefine("HAS_NORMAL_MAP"));
    EXPECT_TRUE(variant.HasDefine("HAS_SKINNING"));
    EXPECT_EQUAL(variant.GetDefineValue("MAX_BONES"), "32");
    EXPECT_TRUE(variant.HasDefine("OPTIMIZED"));
    
    EXPECT_TRUE(variant.HasFeature("DIRECTIONAL_LIGHTING"));
    EXPECT_TRUE(variant.HasFeature("POINT_LIGHTING"));
    EXPECT_TRUE(variant.HasFeature("ALBEDO_TEXTURE"));
    EXPECT_TRUE(variant.HasFeature("NORMAL_MAPPING"));
    EXPECT_TRUE(variant.HasFeature("VERTEX_SKINNING"));
    EXPECT_TRUE(variant.HasFeature("PERFORMANCE_MODE"));
    
    // Should not have debug features
    EXPECT_FALSE(variant.HasDefine("DEBUG"));
    EXPECT_FALSE(variant.HasFeature("DEBUG_OUTPUT"));
    
    variantManager.Shutdown();

    TestOutput::PrintTestPass("render context variant generation");
    return true;
}

bool TestVariantManagerStats() {
    TestOutput::PrintTestStart("variant manager stats");

    auto& variantManager = ShaderVariantManager::GetInstance();
    EXPECT_TRUE(variantManager.Initialize());
    
    // Get initial stats
    VariantStats stats = variantManager.GetVariantStats();
    EXPECT_EQUAL(stats.totalVariants, 0);
    EXPECT_EQUAL(stats.activeVariants, 0);
    EXPECT_EQUAL(stats.cacheHits, 0);
    EXPECT_EQUAL(stats.cacheMisses, 0);
    
    variantManager.Shutdown();

    TestOutput::PrintTestPass("variant manager stats");
    return true;
}

bool TestVariantManagerConfiguration() {
    TestOutput::PrintTestStart("variant manager configuration");

    auto& variantManager = ShaderVariantManager::GetInstance();
    EXPECT_TRUE(variantManager.Initialize());
    
    // Test debug mode
    EXPECT_FALSE(variantManager.IsDebugMode());
    variantManager.SetDebugMode(true);
    EXPECT_TRUE(variantManager.IsDebugMode());
    variantManager.SetDebugMode(false);
    EXPECT_FALSE(variantManager.IsDebugMode());
    
    // Test cache size
    variantManager.SetMaxCacheSize(100);
    // No direct way to verify, but should not crash
    
    // Test variant lifetime
    variantManager.SetVariantLifetime(60.0f);
    // No direct way to verify, but should not crash
    
    // Test async compilation
    variantManager.SetAsyncCompilation(true);
    variantManager.SetAsyncCompilation(false);
    
    variantManager.Shutdown();

    TestOutput::PrintTestPass("variant manager configuration");
    return true;
}

bool TestVariantKeyOperations() {
    TestOutput::PrintTestStart("variant key operations");

    // Test VariantKey equality
    VariantKey key1{"shader1", "hash123"};
    VariantKey key2{"shader1", "hash123"};
    VariantKey key3{"shader2", "hash123"};
    VariantKey key4{"shader1", "hash456"};
    
    EXPECT_TRUE(key1 == key2);
    EXPECT_FALSE(key1 == key3);
    EXPECT_FALSE(key1 == key4);
    
    // Test VariantKeyHash
    VariantKeyHash hasher;
    size_t hash1 = hasher(key1);
    size_t hash2 = hasher(key2);
    size_t hash3 = hasher(key3);
    
    EXPECT_EQUAL(hash1, hash2); // Same keys should have same hash
    EXPECT_NOT_EQUAL(hash1, hash3); // Different keys should have different hash

    TestOutput::PrintTestPass("variant key operations");
    return true;
}

bool TestRenderContextDefaults() {
    TestOutput::PrintTestStart("render context defaults");

    RenderContext context; // Default constructor
    
    // Check default values
    EXPECT_FALSE(context.hasDirectionalLight);
    EXPECT_EQUAL(context.pointLightCount, 0);
    EXPECT_EQUAL(context.spotLightCount, 0);
    EXPECT_FALSE(context.hasShadows);
    EXPECT_FALSE(context.hasAlbedoMap);
    EXPECT_FALSE(context.hasNormalMap);
    EXPECT_FALSE(context.hasMetallicRoughnessMap);
    EXPECT_FALSE(context.hasEmissionMap);
    EXPECT_FALSE(context.hasAOMap);
    EXPECT_FALSE(context.hasSkinning);
    EXPECT_FALSE(context.hasInstancing);
    EXPECT_FALSE(context.useDebugMode);
    EXPECT_TRUE(context.useOptimizedPath);
    EXPECT_TRUE(context.supportsGeometryShaders);
    EXPECT_TRUE(context.supportsTessellation);
    EXPECT_TRUE(context.supportsComputeShaders);
    EXPECT_EQUAL(context.maxBones, 64);
    EXPECT_EQUAL(context.maxPointLights, 8);
    EXPECT_EQUAL(context.maxSpotLights, 4);

    TestOutput::PrintTestPass("render context defaults");
    return true;
}

bool TestVariantManagerUpdate() {
    TestOutput::PrintTestStart("variant manager update");

    auto& variantManager = ShaderVariantManager::GetInstance();
    EXPECT_TRUE(variantManager.Initialize());
    
    // Update should not crash with no variants
    variantManager.Update(0.016f); // 16ms frame time
    variantManager.Update(1.0f);   // 1 second
    
    // Multiple updates should work
    for (int i = 0; i < 10; ++i) {
        variantManager.Update(0.1f);
    }
    
    variantManager.Shutdown();

    TestOutput::PrintTestPass("variant manager update");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderVariantManager");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderVariantManager Tests");

        // Run all tests
        allPassed &= suite.RunTest("Initialization", TestShaderVariantManagerInitialization);
        allPassed &= suite.RunTest("Basic Operations", TestShaderVariantManagerBasicOperations);
        allPassed &= suite.RunTest("Render Context Variant Generation", TestRenderContextVariantGeneration);
        allPassed &= suite.RunTest("Stats", TestVariantManagerStats);
        allPassed &= suite.RunTest("Configuration", TestVariantManagerConfiguration);
        allPassed &= suite.RunTest("Variant Key Operations", TestVariantKeyOperations);
        allPassed &= suite.RunTest("Render Context Defaults", TestRenderContextDefaults);
        allPassed &= suite.RunTest("Update", TestVariantManagerUpdate);

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