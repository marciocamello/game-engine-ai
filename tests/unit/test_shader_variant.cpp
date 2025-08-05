#include "../TestUtils.h"
#include "Graphics/ShaderVariant.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestShaderVariantBasicOperations() {
    TestOutput::PrintTestStart("shader variant basic operations");

    ShaderVariant variant("test_variant");
    
    // Test adding defines
    variant.AddDefine("TEST_DEFINE", "1");
    variant.AddDefine("MAX_LIGHTS", "8");
    
    EXPECT_TRUE(variant.HasDefine("TEST_DEFINE"));
    EXPECT_TRUE(variant.HasDefine("MAX_LIGHTS"));
    EXPECT_FALSE(variant.HasDefine("NONEXISTENT"));
    
    EXPECT_EQUAL(variant.GetDefineValue("TEST_DEFINE"), "1");
    EXPECT_EQUAL(variant.GetDefineValue("MAX_LIGHTS"), "8");
    EXPECT_EQUAL(variant.GetDefineValue("NONEXISTENT"), "");
    
    // Test adding features
    variant.AddFeature("LIGHTING");
    variant.AddFeature("SHADOWS");
    
    EXPECT_TRUE(variant.HasFeature("LIGHTING"));
    EXPECT_TRUE(variant.HasFeature("SHADOWS"));
    EXPECT_FALSE(variant.HasFeature("NONEXISTENT"));
    
    // Test counts
    EXPECT_EQUAL(variant.GetDefineCount(), 2);
    EXPECT_EQUAL(variant.GetFeatureCount(), 2);
    EXPECT_FALSE(variant.IsEmpty());

    TestOutput::PrintTestPass("shader variant basic operations");
    return true;
}

bool TestShaderVariantHashGeneration() {
    TestOutput::PrintTestStart("shader variant hash generation");

    ShaderVariant variant1("test");
    variant1.AddDefine("TEST", "1");
    variant1.AddFeature("FEATURE");
    
    ShaderVariant variant2("test");
    variant2.AddDefine("TEST", "1");
    variant2.AddFeature("FEATURE");
    
    ShaderVariant variant3("test");
    variant3.AddDefine("TEST", "2");
    variant3.AddFeature("FEATURE");
    
    // Same variants should have same hash
    std::string hash1 = variant1.GenerateHash();
    std::string hash2 = variant2.GenerateHash();
    std::string hash3 = variant3.GenerateHash();
    
    EXPECT_EQUAL(hash1, hash2);
    EXPECT_NOT_EQUAL(hash1, hash3);
    
    // Hash should be non-empty
    EXPECT_FALSE(hash1.empty());

    TestOutput::PrintTestPass("shader variant hash generation");
    return true;
}

bool TestShaderVariantCompatibility() {
    TestOutput::PrintTestStart("shader variant compatibility");

    ShaderVariant variant1("test1");
    variant1.AddDefine("LIGHTS", "4");
    variant1.AddFeature("LIGHTING");
    
    ShaderVariant variant2("test2");
    variant2.AddDefine("SHADOWS", "1");
    variant2.AddFeature("SHADOW_MAPPING");
    
    ShaderVariant variant3("test3");
    variant3.AddDefine("LIGHTS", "8"); // Conflicting value
    variant3.AddFeature("LIGHTING");
    
    // Compatible variants (no conflicting defines)
    EXPECT_TRUE(variant1.IsCompatibleWith(variant2));
    EXPECT_TRUE(variant2.IsCompatibleWith(variant1));
    
    // Incompatible variants (conflicting define values)
    EXPECT_FALSE(variant1.IsCompatibleWith(variant3));
    EXPECT_FALSE(variant3.IsCompatibleWith(variant1));

    TestOutput::PrintTestPass("shader variant compatibility");
    return true;
}

bool TestShaderVariantPreprocessorString() {
    TestOutput::PrintTestStart("shader variant preprocessor string");

    ShaderVariant variant("test");
    variant.AddDefine("MAX_LIGHTS", "8");
    variant.AddDefine("ENABLE_SHADOWS", "1");
    variant.AddFeature("LIGHTING");
    variant.AddFeature("PBR");
    
    std::string preprocessor = variant.GeneratePreprocessorString();
    
    // Check that defines are present
    EXPECT_TRUE(preprocessor.find("#define MAX_LIGHTS 8") != std::string::npos);
    EXPECT_TRUE(preprocessor.find("#define ENABLE_SHADOWS") != std::string::npos);
    EXPECT_TRUE(preprocessor.find("#define LIGHTING") != std::string::npos);
    EXPECT_TRUE(preprocessor.find("#define PBR") != std::string::npos);

    TestOutput::PrintTestPass("shader variant preprocessor string");
    return true;
}

bool TestShaderVariantComparison() {
    TestOutput::PrintTestStart("shader variant comparison");

    ShaderVariant variant1("test");
    variant1.AddDefine("TEST", "1");
    variant1.AddFeature("FEATURE");
    
    ShaderVariant variant2("test");
    variant2.AddDefine("TEST", "1");
    variant2.AddFeature("FEATURE");
    
    ShaderVariant variant3("different");
    variant3.AddDefine("TEST", "1");
    variant3.AddFeature("FEATURE");
    
    // Same variants should be equal
    EXPECT_TRUE(variant1 == variant2);
    EXPECT_FALSE(variant1 != variant2);
    
    // Different variants should not be equal
    EXPECT_FALSE(variant1 == variant3);
    EXPECT_TRUE(variant1 != variant3);

    TestOutput::PrintTestPass("shader variant comparison");
    return true;
}

bool TestShaderVariantRemoval() {
    TestOutput::PrintTestStart("shader variant removal");

    ShaderVariant variant("test");
    variant.AddDefine("TEST1", "1");
    variant.AddDefine("TEST2", "2");
    variant.AddFeature("FEATURE1");
    variant.AddFeature("FEATURE2");
    
    EXPECT_EQUAL(variant.GetDefineCount(), 2);
    EXPECT_EQUAL(variant.GetFeatureCount(), 2);
    
    // Remove define
    variant.RemoveDefine("TEST1");
    EXPECT_FALSE(variant.HasDefine("TEST1"));
    EXPECT_TRUE(variant.HasDefine("TEST2"));
    EXPECT_EQUAL(variant.GetDefineCount(), 1);
    
    // Remove feature
    variant.RemoveFeature("FEATURE1");
    EXPECT_FALSE(variant.HasFeature("FEATURE1"));
    EXPECT_TRUE(variant.HasFeature("FEATURE2"));
    EXPECT_EQUAL(variant.GetFeatureCount(), 1);
    
    // Clear all
    variant.Clear();
    EXPECT_TRUE(variant.IsEmpty());
    EXPECT_EQUAL(variant.GetDefineCount(), 0);
    EXPECT_EQUAL(variant.GetFeatureCount(), 0);

    TestOutput::PrintTestPass("shader variant removal");
    return true;
}

bool TestPredefinedShaderVariants() {
    TestOutput::PrintTestStart("predefined shader variants");

    // Test default variant
    auto defaultVariant = ShaderVariants::CreateDefault();
    EXPECT_EQUAL(defaultVariant.name, "default");
    EXPECT_TRUE(defaultVariant.IsEmpty());
    
    // Test debug variant
    auto debugVariant = ShaderVariants::CreateDebug();
    EXPECT_EQUAL(debugVariant.name, "debug");
    EXPECT_TRUE(debugVariant.HasDefine("DEBUG"));
    EXPECT_TRUE(debugVariant.HasFeature("DEBUG_OUTPUT"));
    
    // Test optimized variant
    auto optimizedVariant = ShaderVariants::CreateOptimized();
    EXPECT_EQUAL(optimizedVariant.name, "optimized");
    EXPECT_TRUE(optimizedVariant.HasDefine("OPTIMIZED"));
    EXPECT_TRUE(optimizedVariant.HasFeature("PERFORMANCE_MODE"));
    
    // Test lighting variants
    auto directionalLight = ShaderVariants::CreateWithDirectionalLight();
    EXPECT_TRUE(directionalLight.HasDefine("HAS_DIRECTIONAL_LIGHT"));
    EXPECT_TRUE(directionalLight.HasFeature("DIRECTIONAL_LIGHTING"));
    
    auto pointLights = ShaderVariants::CreateWithPointLights(4);
    EXPECT_TRUE(pointLights.HasDefine("HAS_POINT_LIGHTS"));
    EXPECT_EQUAL(pointLights.GetDefineValue("MAX_POINT_LIGHTS"), "4");
    EXPECT_TRUE(pointLights.HasFeature("POINT_LIGHTING"));
    
    // Test material variants
    auto albedoMap = ShaderVariants::CreateWithAlbedoMap();
    EXPECT_TRUE(albedoMap.HasDefine("HAS_ALBEDO_MAP"));
    EXPECT_TRUE(albedoMap.HasFeature("ALBEDO_TEXTURE"));
    
    auto normalMap = ShaderVariants::CreateWithNormalMap();
    EXPECT_TRUE(normalMap.HasDefine("HAS_NORMAL_MAP"));
    EXPECT_TRUE(normalMap.HasFeature("NORMAL_MAPPING"));

    TestOutput::PrintTestPass("predefined shader variants");
    return true;
}

bool TestShaderVariantHash() {
    TestOutput::PrintTestStart("shader variant hash function");

    ShaderVariant variant1("test");
    variant1.AddDefine("TEST", "1");
    variant1.AddFeature("FEATURE");
    
    ShaderVariant variant2("test");
    variant2.AddDefine("TEST", "1");
    variant2.AddFeature("FEATURE");
    
    ShaderVariantHash hasher;
    size_t hash1 = hasher(variant1);
    size_t hash2 = hasher(variant2);
    
    // Same variants should have same hash
    EXPECT_EQUAL(hash1, hash2);
    
    // Hash should be non-zero for non-empty variants
    EXPECT_NOT_EQUAL(hash1, 0);

    TestOutput::PrintTestPass("shader variant hash function");
    return true;
}

int main() {
    TestOutput::PrintHeader("ShaderVariant");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("ShaderVariant Tests");

        // Run all tests
        allPassed &= suite.RunTest("Basic Operations", TestShaderVariantBasicOperations);
        allPassed &= suite.RunTest("Hash Generation", TestShaderVariantHashGeneration);
        allPassed &= suite.RunTest("Compatibility", TestShaderVariantCompatibility);
        allPassed &= suite.RunTest("Preprocessor String", TestShaderVariantPreprocessorString);
        allPassed &= suite.RunTest("Comparison", TestShaderVariantComparison);
        allPassed &= suite.RunTest("Removal", TestShaderVariantRemoval);
        allPassed &= suite.RunTest("Predefined Variants", TestPredefinedShaderVariants);
        allPassed &= suite.RunTest("Hash Function", TestShaderVariantHash);

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