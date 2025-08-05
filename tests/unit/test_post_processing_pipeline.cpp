#include "TestUtils.h"
#include "Graphics/PostProcessingPipeline.h"
#include "Graphics/PostProcessEffects.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test PostProcessingPipeline basic functionality without OpenGL
 * Requirements: 5.1 (post-processing pipeline configuration)
 */
bool TestPostProcessingPipelineBasics() {
    TestOutput::PrintTestStart("post processing pipeline basics");

    PostProcessingPipeline pipeline;
    
    // Test initial state
    EXPECT_FALSE(pipeline.IsInitialized());
    EXPECT_EQUAL(pipeline.GetWidth(), 0);
    EXPECT_EQUAL(pipeline.GetHeight(), 0);

    // Test global settings
    pipeline.SetGlobalExposure(1.5f);
    pipeline.SetGlobalGamma(2.4f);
    EXPECT_NEARLY_EQUAL(pipeline.GetGlobalExposure(), 1.5f);
    EXPECT_NEARLY_EQUAL(pipeline.GetGlobalGamma(), 2.4f);

    // Test quality level
    pipeline.SetQualityLevel(QualityLevel::Ultra);
    EXPECT_EQUAL(static_cast<int>(pipeline.GetQualityLevel()), static_cast<int>(QualityLevel::Ultra));

    TestOutput::PrintTestPass("post processing pipeline basics");
    return true;
}

/**
 * Test tone mapping effect creation
 * Requirements: 5.2 (tone mapping effect)
 */
bool TestToneMappingEffect() {
    TestOutput::PrintTestStart("tone mapping effect");

    ToneMappingEffect effect;
    
    // Test initial state
    EXPECT_EQUAL(effect.GetName(), "ToneMapping");
    EXPECT_TRUE(effect.IsEnabled());

    // Test parameters
    effect.SetExposure(1.5f);
    effect.SetGamma(2.4f);
    effect.SetToneMappingType(ToneMappingType::Reinhard);

    EXPECT_NEARLY_EQUAL(effect.GetExposure(), 1.5f);
    EXPECT_NEARLY_EQUAL(effect.GetGamma(), 2.4f);
    EXPECT_EQUAL(static_cast<int>(effect.GetToneMappingType()), static_cast<int>(ToneMappingType::Reinhard));

    // Test parameter setting via string interface
    effect.SetParameter("exposure", 2.0f);
    effect.SetParameter("gamma", 2.2f);
    EXPECT_NEARLY_EQUAL(effect.GetExposure(), 2.0f);
    EXPECT_NEARLY_EQUAL(effect.GetGamma(), 2.2f);

    TestOutput::PrintTestPass("tone mapping effect");
    return true;
}

/**
 * Test FXAA effect creation
 * Requirements: 5.3 (FXAA anti-aliasing effect)
 */
bool TestFXAAEffect() {
    TestOutput::PrintTestStart("FXAA effect");

    FXAAEffect effect;
    
    // Test initial state
    EXPECT_EQUAL(effect.GetName(), "FXAA");
    EXPECT_TRUE(effect.IsEnabled());

    // Test parameters
    effect.SetQuality(0.8f);
    effect.SetSubPixelShift(0.3f);
    effect.SetEdgeThreshold(0.2f);
    effect.SetEdgeThresholdMin(0.1f);

    EXPECT_NEARLY_EQUAL(effect.GetQuality(), 0.8f);
    EXPECT_NEARLY_EQUAL(effect.GetSubPixelShift(), 0.3f);
    EXPECT_NEARLY_EQUAL(effect.GetEdgeThreshold(), 0.2f);
    EXPECT_NEARLY_EQUAL(effect.GetEdgeThresholdMin(), 0.1f);

    // Test parameter setting via string interface
    effect.SetParameter("quality", 0.9f);
    EXPECT_NEARLY_EQUAL(effect.GetQuality(), 0.9f);

    TestOutput::PrintTestPass("FXAA effect");
    return true;
}

/**
 * Test bloom effect creation
 * Requirements: 5.4 (bloom effect)
 */
bool TestBloomEffect() {
    TestOutput::PrintTestStart("bloom effect");

    BloomEffect effect;
    
    // Test initial state
    EXPECT_EQUAL(effect.GetName(), "Bloom");
    EXPECT_TRUE(effect.IsEnabled());

    // Test parameters
    effect.SetThreshold(1.2f);
    effect.SetIntensity(0.7f);
    effect.SetRadius(2.0f);
    effect.SetBlurPasses(8);

    EXPECT_NEARLY_EQUAL(effect.GetThreshold(), 1.2f);
    EXPECT_NEARLY_EQUAL(effect.GetIntensity(), 0.7f);
    EXPECT_NEARLY_EQUAL(effect.GetRadius(), 2.0f);
    EXPECT_EQUAL(effect.GetBlurPasses(), 8);

    // Test parameter setting via string interface
    effect.SetParameter("threshold", 1.5f);
    effect.SetParameter("intensity", 0.8f);
    EXPECT_NEARLY_EQUAL(effect.GetThreshold(), 1.5f);
    EXPECT_NEARLY_EQUAL(effect.GetIntensity(), 0.8f);

    TestOutput::PrintTestPass("bloom effect");
    return true;
}

/**
 * Test tone mapping types enumeration
 * Requirements: 5.2 (tone mapping operators)
 */
bool TestToneMappingTypes() {
    TestOutput::PrintTestStart("tone mapping types");

    // Test that all tone mapping types are available
    ToneMappingType types[] = {
        ToneMappingType::None,
        ToneMappingType::Reinhard,
        ToneMappingType::ACES,
        ToneMappingType::Filmic
    };

    // Test that we can create effects with different types
    for (auto type : types) {
        ToneMappingEffect effect;
        effect.SetToneMappingType(type);
        EXPECT_EQUAL(static_cast<int>(effect.GetToneMappingType()), static_cast<int>(type));
    }

    TestOutput::PrintTestPass("tone mapping types");
    return true;
}

int main() {
    TestOutput::PrintHeader("PostProcessingPipeline");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("PostProcessingPipeline Tests");

        // Run all tests
        allPassed &= suite.RunTest("PostProcessingPipeline Basics", TestPostProcessingPipelineBasics);
        allPassed &= suite.RunTest("ToneMapping Effect", TestToneMappingEffect);
        allPassed &= suite.RunTest("FXAA Effect", TestFXAAEffect);
        allPassed &= suite.RunTest("Bloom Effect", TestBloomEffect);
        allPassed &= suite.RunTest("ToneMapping Types", TestToneMappingTypes);

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