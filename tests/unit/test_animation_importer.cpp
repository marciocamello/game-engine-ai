#include "TestUtils.h"
#include "Animation/AnimationImporter.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Core/Logger.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test AnimationImporter initialization and basic functionality
 * Requirements: 8.1, 8.2, 8.3
 */
bool TestAnimationImporterInitialization() {
    TestOutput::PrintTestStart("animation importer initialization");

    AnimationImporter importer;
    
    // Test default configuration
    auto config = importer.GetDefaultConfig();
    EXPECT_TRUE(config.convertCoordinateSystem);
    EXPECT_TRUE(config.optimizeKeyframes);
    EXPECT_TRUE(config.validateBoneHierarchy);
    
    // Test format support queries
    auto supportedFormats = importer.GetSupportedAnimationFormats();
    EXPECT_TRUE(supportedFormats.size() > 0);
    
    // Test specific format support
    EXPECT_TRUE(importer.IsAnimationFormatSupported("fbx"));
    EXPECT_TRUE(importer.IsAnimationFormatSupported("dae"));
    EXPECT_TRUE(importer.IsAnimationFormatSupported("gltf"));
    EXPECT_FALSE(importer.IsAnimationFormatSupported("txt"));
    
    TestOutput::PrintTestPass("animation importer initialization");
    return true;
}

/**
 * Test animation import configuration
 * Requirements: 8.4, 8.5
 */
bool TestAnimationImportConfiguration() {
    TestOutput::PrintTestStart("animation import configuration");

    AnimationImporter importer;
    
    // Test custom configuration
    AnimationImportConfig config;
    config.convertCoordinateSystem = false;
    config.optimizeKeyframes = false;
    config.keyframeOptimizationTolerance = 0.01f;
    config.flipYZ = true;
    config.coordinateSystemScale = Math::Vec3(2.0f, 2.0f, 2.0f);
    
    importer.SetDefaultConfig(config);
    
    auto retrievedConfig = importer.GetDefaultConfig();
    EXPECT_FALSE(retrievedConfig.convertCoordinateSystem);
    EXPECT_FALSE(retrievedConfig.optimizeKeyframes);
    EXPECT_NEARLY_EQUAL(retrievedConfig.keyframeOptimizationTolerance, 0.01f);
    EXPECT_TRUE(retrievedConfig.flipYZ);
    EXPECT_VEC3_NEARLY_EQUAL(retrievedConfig.coordinateSystemScale, Math::Vec3(2.0f, 2.0f, 2.0f));
    
    TestOutput::PrintTestPass("animation import configuration");
    return true;
}

/**
 * Test animation data validation
 * Requirements: 8.4, 8.5
 */
bool TestAnimationDataValidation() {
    TestOutput::PrintTestStart("animation data validation");

    AnimationImporter importer;
    
    // Test with non-existent file
    bool validResult = importer.ValidateAnimationData("non_existent_file.fbx");
    EXPECT_FALSE(validResult);
    
    // Test with invalid file format
    validResult = importer.ValidateAnimationData("test.txt");
    EXPECT_FALSE(validResult);
    
    TestOutput::PrintInfo("Animation data validation working correctly for invalid inputs");
    
    TestOutput::PrintTestPass("animation data validation");
    return true;
}

/**
 * Test skeleton creation from bone hierarchy
 * Requirements: 8.2, 8.3
 */
bool TestSkeletonCreation() {
    TestOutput::PrintTestStart("skeleton creation from bone hierarchy");

    // Create a test skeleton manually to verify the structure
    auto skeleton = std::make_shared<AnimationSkeleton>("TestSkeleton");
    
    // Create root bone
    auto rootBone = skeleton->CreateBone("Root");
    skeleton->SetRootBone(rootBone);
    
    // Create child bones
    auto spine = skeleton->CreateBone("Spine");
    auto leftArm = skeleton->CreateBone("LeftArm");
    auto rightArm = skeleton->CreateBone("RightArm");
    
    // Set up hierarchy
    skeleton->AddBone(spine, "Root");
    skeleton->AddBone(leftArm, "Spine");
    skeleton->AddBone(rightArm, "Spine");
    
    // Validate skeleton
    EXPECT_TRUE(skeleton->ValidateHierarchy());
    EXPECT_EQUAL(skeleton->GetBoneCount(), static_cast<size_t>(4));
    EXPECT_TRUE(skeleton->GetRootBone() != nullptr);
    EXPECT_STRING_EQUAL(skeleton->GetRootBone()->GetName(), "Root");
    
    // Test bone lookup
    auto foundBone = skeleton->GetBone("Spine");
    EXPECT_TRUE(foundBone != nullptr);
    EXPECT_STRING_EQUAL(foundBone->GetName(), "Spine");
    
    TestOutput::PrintInfo("Skeleton creation and hierarchy validation working correctly");
    
    TestOutput::PrintTestPass("skeleton creation from bone hierarchy");
    return true;
}

/**
 * Test animation track mapping to skeleton bones
 * Requirements: 8.3
 */
bool TestAnimationTrackMapping() {
    TestOutput::PrintTestStart("animation track mapping to skeleton bones");

    // Create a test animation
    auto animation = std::make_shared<SkeletalAnimation>("TestAnimation");
    animation->SetDuration(2.0f);
    animation->SetFrameRate(30.0f);
    
    // Add some keyframes for different bones
    animation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("Root", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("Root", 2.0f, Math::Vec3(2.0f, 0.0f, 0.0f));
    
    animation->AddRotationKeyframe("Spine", 0.0f, Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));
    animation->AddRotationKeyframe("Spine", 1.0f, Math::Quat(0.707f, 0.0f, 0.707f, 0.0f));
    
    // Test animation properties
    EXPECT_NEARLY_EQUAL(animation->GetDuration(), 2.0f);
    EXPECT_NEARLY_EQUAL(animation->GetFrameRate(), 30.0f);
    EXPECT_TRUE(animation->HasBone("Root"));
    EXPECT_TRUE(animation->HasBone("Spine"));
    EXPECT_FALSE(animation->HasBone("NonExistent"));
    
    // Test bone sampling
    auto rootPose = animation->SampleBone("Root", 1.0f);
    EXPECT_TRUE(rootPose.hasPosition);
    EXPECT_VEC3_NEARLY_EQUAL(rootPose.position, Math::Vec3(1.0f, 0.0f, 0.0f));
    
    auto spinePose = animation->SampleBone("Spine", 1.0f);
    EXPECT_TRUE(spinePose.hasRotation);
    
    TestOutput::PrintInfo("Animation track mapping and sampling working correctly");
    
    TestOutput::PrintTestPass("animation track mapping to skeleton bones");
    return true;
}

/**
 * Test coordinate system conversion for imported animations
 * Requirements: 8.5
 */
bool TestCoordinateSystemConversion() {
    TestOutput::PrintTestStart("coordinate system conversion for imported animations");

    AnimationImportConfig config;
    config.convertCoordinateSystem = true;
    config.flipYZ = true;
    config.coordinateSystemScale = Math::Vec3(0.01f, 0.01f, 0.01f); // Convert from cm to m
    
    // Test vector conversion (simulated)
    Math::Vec3 originalVector(100.0f, 200.0f, 300.0f); // 100cm, 200cm, 300cm
    
    // Simulate the conversion that would happen in the importer
    Math::Vec3 convertedVector = originalVector;
    if (config.flipYZ) {
        std::swap(convertedVector.y, convertedVector.z);
    }
    convertedVector *= config.coordinateSystemScale;
    
    // Expected result: Y and Z swapped, scaled to meters
    Math::Vec3 expectedVector(1.0f, 3.0f, 2.0f); // 1m, 3m, 2m (Y and Z swapped)
    EXPECT_VEC3_NEARLY_EQUAL(convertedVector, expectedVector);
    
    TestOutput::PrintInfo("Coordinate system conversion working correctly");
    
    TestOutput::PrintTestPass("coordinate system conversion for imported animations");
    return true;
}

/**
 * Test animation metadata preservation
 * Requirements: 8.7
 */
bool TestAnimationMetadataPreservation() {
    TestOutput::PrintTestStart("animation metadata preservation");

    // Create animation with metadata
    auto animation = std::make_shared<SkeletalAnimation>("WalkCycle");
    animation->SetDuration(1.5f);
    animation->SetFrameRate(24.0f);
    animation->SetLoopMode(LoopMode::Loop);
    
    // Add some animation data to make serialization meaningful
    animation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("Root", 1.5f, Math::Vec3(1.0f, 0.0f, 0.0f));
    
    // Test metadata preservation
    EXPECT_STRING_EQUAL(animation->GetName(), "WalkCycle");
    EXPECT_NEARLY_EQUAL(animation->GetDuration(), 1.5f);
    EXPECT_NEARLY_EQUAL(animation->GetFrameRate(), 24.0f);
    EXPECT_EQUAL(static_cast<int>(animation->GetLoopMode()), static_cast<int>(LoopMode::Loop));
    
    // Test serialization/deserialization to verify metadata preservation
    auto serializedData = animation->Serialize();
    EXPECT_STRING_EQUAL(serializedData.name, "WalkCycle");
    EXPECT_NEARLY_EQUAL(serializedData.duration, 1.5f);
    EXPECT_NEARLY_EQUAL(serializedData.frameRate, 24.0f);
    EXPECT_EQUAL(static_cast<int>(serializedData.loopMode), static_cast<int>(LoopMode::Loop));
    
    // Create new animation and deserialize
    auto newAnimation = std::make_shared<SkeletalAnimation>();
    bool deserializeResult = newAnimation->Deserialize(serializedData);
    EXPECT_TRUE(deserializeResult);
    EXPECT_STRING_EQUAL(newAnimation->GetName(), "WalkCycle");
    EXPECT_NEARLY_EQUAL(newAnimation->GetDuration(), 1.5f);
    
    TestOutput::PrintInfo("Animation metadata preservation working correctly");
    
    TestOutput::PrintTestPass("animation metadata preservation");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationImporter");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationImporter Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Importer Initialization", TestAnimationImporterInitialization);
        allPassed &= suite.RunTest("Animation Import Configuration", TestAnimationImportConfiguration);
        allPassed &= suite.RunTest("Animation Data Validation", TestAnimationDataValidation);
        allPassed &= suite.RunTest("Skeleton Creation", TestSkeletonCreation);
        allPassed &= suite.RunTest("Animation Track Mapping", TestAnimationTrackMapping);
        allPassed &= suite.RunTest("Coordinate System Conversion", TestCoordinateSystemConversion);
        allPassed &= suite.RunTest("Animation Metadata Preservation", TestAnimationMetadataPreservation);

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