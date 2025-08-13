#include "TestUtils.h"
#include "Animation/Animation.h"
#include "Animation/AnimationCompression.h"
#include "Animation/AnimationStreaming.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test animation keyframe optimization
 * Requirements: 7.1, 7.2 (keyframe reduction and compression algorithms)
 */
bool TestAnimationKeyframeOptimization() {
    TestOutput::PrintTestStart("animation keyframe optimization");

    // Create test animation with redundant keyframes
    Animation::Animation animation("test_animation");
    animation.SetDuration(3.0f);
    animation.SetFrameRate(30.0f);

    // Add position keyframes with some redundant ones
    animation.AddPositionKeyframe("root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("root", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("root", 1.5f, Math::Vec3(1.5f, 0.0f, 0.0f)); // Should be redundant
    animation.AddPositionKeyframe("root", 2.0f, Math::Vec3(2.0f, 0.0f, 0.0f));
    animation.AddPositionKeyframe("root", 3.0f, Math::Vec3(3.0f, 0.0f, 0.0f));

    size_t originalKeyframes = animation.GetKeyframeCount();
    EXPECT_TRUE(originalKeyframes > 0);

    // Optimize keyframes
    animation.OptimizeKeyframes(0.01f);

    size_t optimizedKeyframes = animation.GetKeyframeCount();
    EXPECT_TRUE(optimizedKeyframes <= originalKeyframes);

    // Verify animation still works
    EXPECT_TRUE(animation.ValidateAnimation());

    TestOutput::PrintTestPass("animation keyframe optimization");
    return true;
}

/**
 * Test animation compression
 * Requirements: 7.1, 7.4 (compression algorithms and memory efficiency)
 */
bool TestAnimationCompression() {
    TestOutput::PrintTestStart("animation compression");

    // Create test animation
    Animation::Animation original("original_animation");
    original.SetDuration(2.0f);
    original.SetFrameRate(30.0f);

    // Add keyframes for multiple bones
    original.AddPositionKeyframe("root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    original.AddPositionKeyframe("root", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    original.AddPositionKeyframe("root", 2.0f, Math::Vec3(2.0f, 0.0f, 0.0f));

    original.AddRotationKeyframe("root", 0.0f, Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));
    original.AddRotationKeyframe("root", 1.0f, Math::Quat(0.707f, 0.0f, 0.707f, 0.0f));
    original.AddRotationKeyframe("root", 2.0f, Math::Quat(0.0f, 0.0f, 1.0f, 0.0f));

    size_t originalMemory = original.GetMemoryUsage();
    size_t originalKeyframes = original.GetKeyframeCount();

    // Create compressed copy
    auto compressed = original.CreateCompressedCopy(0.01f);
    EXPECT_TRUE(compressed != nullptr);

    size_t compressedMemory = compressed->GetMemoryUsage();
    size_t compressedKeyframes = compressed->GetKeyframeCount();

    // Verify compression worked
    EXPECT_TRUE(compressedKeyframes <= originalKeyframes);
    EXPECT_TRUE(compressed->ValidateAnimation());

    // Test sampling still works
    auto originalPose = original.SampleBone("root", 1.0f);
    auto compressedPose = compressed->SampleBone("root", 1.0f);

    EXPECT_TRUE(originalPose.hasPosition);
    EXPECT_TRUE(compressedPose.hasPosition);

    TestOutput::PrintTestPass("animation compression");
    return true;
}

/**
 * Test animation compressor class
 * Requirements: 7.1, 7.2 (compression algorithms and redundant keyframe removal)
 */
bool TestAnimationCompressor() {
    TestOutput::PrintTestStart("animation compressor");

    // Create test animation
    Animation::Animation original("test_animation");
    original.SetDuration(2.0f);
    original.AddPositionKeyframe("bone1", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    original.AddPositionKeyframe("bone1", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    original.AddPositionKeyframe("bone1", 2.0f, Math::Vec3(2.0f, 0.0f, 0.0f));

    // Create compressor
    Animation::AnimationCompressor compressor;
    Animation::CompressionSettings settings;
    settings.positionTolerance = 0.01f;
    settings.enableKeyframeReduction = true;

    // Compress animation
    auto compressed = compressor.CompressAnimation(original, settings);
    EXPECT_TRUE(compressed != nullptr);

    // Check compression stats
    const auto& stats = compressor.GetLastCompressionStats();
    EXPECT_TRUE(stats.originalKeyframes > 0);
    EXPECT_TRUE(stats.compressedKeyframes <= stats.originalKeyframes);

    TestOutput::PrintTestPass("animation compressor");
    return true;
}

/**
 * Test animation streaming manager
 * Requirements: 7.5, 7.6 (streaming and memory management)
 */
bool TestAnimationStreamingManager() {
    TestOutput::PrintTestStart("animation streaming manager");

    // Create streaming manager
    Animation::AnimationStreamingManager streamingManager;
    Animation::StreamingConfig config;
    config.memoryLimitBytes = 10 * 1024 * 1024; // 10MB
    config.maxConcurrentLoads = 2;

    EXPECT_TRUE(streamingManager.Initialize(config));

    // Register test animations
    streamingManager.RegisterAnimation("anim1", "test_anim1.dat");
    streamingManager.RegisterAnimation("anim2", "test_anim2.dat");

    EXPECT_TRUE(streamingManager.IsAnimationRegistered("anim1"));
    EXPECT_TRUE(streamingManager.IsAnimationRegistered("anim2"));
    EXPECT_FALSE(streamingManager.IsAnimationRegistered("nonexistent"));

    // Update to initialize memory stats
    streamingManager.Update(0.016f);

    // Test memory stats
    auto stats = streamingManager.GetMemoryStats();
    EXPECT_EQUAL(stats.memoryLimit, config.memoryLimitBytes);

    // Test configuration
    const auto& retrievedConfig = streamingManager.GetConfig();
    EXPECT_EQUAL(retrievedConfig.memoryLimitBytes, config.memoryLimitBytes);
    EXPECT_EQUAL(retrievedConfig.maxConcurrentLoads, config.maxConcurrentLoads);

    streamingManager.Shutdown();

    TestOutput::PrintTestPass("animation streaming manager");
    return true;
}

/**
 * Test animation data cache
 * Requirements: 7.3, 7.6 (data sharing and memory management)
 */
bool TestAnimationDataCache() {
    TestOutput::PrintTestStart("animation data cache");

    Animation::AnimationDataCache cache;

    // Create test animation
    auto animation = std::make_shared<Animation::Animation>("cached_animation");
    animation->SetDuration(1.0f);

    // Cache animation
    cache.CacheAnimation("test_anim", animation);
    EXPECT_EQUAL(cache.GetCachedAnimationCount(), static_cast<size_t>(1));

    // Retrieve cached animation
    auto retrieved = cache.GetCachedAnimation("test_anim");
    EXPECT_TRUE(retrieved != nullptr);
    EXPECT_EQUAL(retrieved->GetName(), "cached_animation");

    // Test cache miss
    auto missing = cache.GetCachedAnimation("nonexistent");
    EXPECT_TRUE(missing == nullptr);

    // Test cache stats
    auto stats = cache.GetCacheStats();
    EXPECT_TRUE(stats.hits > 0);
    EXPECT_TRUE(stats.misses > 0);

    // Clear cache
    cache.ClearCache();
    EXPECT_EQUAL(cache.GetCachedAnimationCount(), static_cast<size_t>(0));

    TestOutput::PrintTestPass("animation data cache");
    return true;
}

/**
 * Test animation preloader
 * Requirements: 7.5 (streaming and predictive loading)
 */
bool TestAnimationPreloader() {
    TestOutput::PrintTestStart("animation preloader");

    // Create streaming manager
    Animation::AnimationStreamingManager streamingManager;
    Animation::StreamingConfig config;
    EXPECT_TRUE(streamingManager.Initialize(config));

    // Create preloader
    Animation::AnimationPreloader preloader(&streamingManager);

    // Record some transitions
    preloader.RecordAnimationTransition("idle", "walk");
    preloader.RecordAnimationTransition("idle", "run");
    preloader.RecordAnimationTransition("walk", "run");
    preloader.RecordAnimationTransition("idle", "walk"); // Record again

    // Get predictions
    auto predictions = preloader.GetPredictedAnimations("idle");
    EXPECT_TRUE(predictions.size() > 0);

    // Test configuration
    preloader.SetPredictionThreshold(0.5f);
    preloader.SetMaxPredictions(3);

    streamingManager.Shutdown();

    TestOutput::PrintTestPass("animation preloader");
    return true;
}

int main() {
    TestOutput::PrintHeader("Animation Compression and Streaming");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Animation Compression and Streaming Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Keyframe Optimization", TestAnimationKeyframeOptimization);
        allPassed &= suite.RunTest("Animation Compression", TestAnimationCompression);
        allPassed &= suite.RunTest("Animation Compressor", TestAnimationCompressor);
        allPassed &= suite.RunTest("Animation Streaming Manager", TestAnimationStreamingManager);
        allPassed &= suite.RunTest("Animation Data Cache", TestAnimationDataCache);
        allPassed &= suite.RunTest("Animation Preloader", TestAnimationPreloader);

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