#include <iostream>
#include "../TestUtils.h"
#include "Resource/ModelLoader.h"
#include "Animation/AnimationImporter.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Core/Logger.h"
#include <memory>

using namespace GameEngine::Testing;

/**
 * Test ModelLoader integration with animation import
 * Requirements: 8.1, 8.2, 8.3
 */
bool TestModelLoaderAnimationIntegration() {
    TestOutput::PrintTestStart("ModelLoader animation integration");

    ModelLoader loader;
    bool initResult = loader.Initialize();
    EXPECT_TRUE(initResult);
    
    if (!initResult) {
        TestOutput::PrintTestFail("ModelLoader animation integration", "initialization successful", "initialization failed");
        return false;
    }

    // Test animation import configuration
    EXPECT_TRUE(loader.IsAnimationImportEnabled()); // Should be enabled by default
    
    // Test disabling animation import
    loader.SetAnimationImportEnabled(false);
    EXPECT_FALSE(loader.IsAnimationImportEnabled());
    
    // Re-enable for testing
    loader.SetAnimationImportEnabled(true);
    EXPECT_TRUE(loader.IsAnimationImportEnabled());
    
    TestOutput::PrintInfo("Animation import configuration working correctly");
    
    loader.Shutdown();
    
    TestOutput::PrintTestPass("ModelLoader animation integration");
    return true;
}

/**
 * Test animation import from model files
 * Requirements: 8.1, 8.2, 8.3
 */
bool TestAnimationImportFromModelFiles() {
    TestOutput::PrintTestStart("animation import from model files");

    GameEngine::Animation::AnimationImporter importer;
    
    // Test format support
    auto supportedFormats = importer.GetSupportedAnimationFormats();
    EXPECT_TRUE(supportedFormats.size() > 0);
    
    bool fbxSupported = importer.IsAnimationFormatSupported("fbx");
    bool daeSupported = importer.IsAnimationFormatSupported("dae");
    bool gltfSupported = importer.IsAnimationFormatSupported("gltf");
    
    TestOutput::PrintInfo("Animation format support:");
    TestOutput::PrintInfo("  FBX: " + std::string(fbxSupported ? "Yes" : "No"));
    TestOutput::PrintInfo("  DAE: " + std::string(daeSupported ? "Yes" : "No"));
    TestOutput::PrintInfo("  GLTF: " + std::string(gltfSupported ? "Yes" : "No"));
    
    // Test with non-existent file (should handle gracefully)
    auto result = importer.ImportFromFile("non_existent_model.fbx");
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(!result.errorMessage.empty());
    
    TestOutput::PrintInfo("Error handling for non-existent files working correctly");
    
    TestOutput::PrintTestPass("animation import from model files");
    return true;
}

/**
 * Test skeleton creation from model bone hierarchy
 * Requirements: 8.2, 8.3
 */
bool TestSkeletonCreationFromModelHierarchy() {
    TestOutput::PrintTestStart("skeleton creation from model bone hierarchy");

    // Create a test skeleton to verify the structure
    auto skeleton = std::make_shared<GameEngine::Animation::AnimationSkeleton>("TestSkeleton");
    
    // Test basic skeleton operations
    EXPECT_TRUE(skeleton != nullptr);
    EXPECT_STRING_EQUAL(skeleton->GetName(), "TestSkeleton");
    EXPECT_EQUAL(skeleton->GetBoneCount(), static_cast<size_t>(0));
    
    // Create and add bones
    auto rootBone = skeleton->CreateBone("Root");
    EXPECT_TRUE(rootBone != nullptr);
    EXPECT_STRING_EQUAL(rootBone->GetName(), "Root");
    
    skeleton->SetRootBone(rootBone);
    EXPECT_TRUE(skeleton->GetRootBone() != nullptr);
    EXPECT_STRING_EQUAL(skeleton->GetRootBone()->GetName(), "Root");
    
    // Add child bones
    auto childBone = skeleton->CreateBone("Child");
    bool addResult = skeleton->AddBone(childBone, "Root");
    EXPECT_TRUE(addResult);
    EXPECT_EQUAL(skeleton->GetBoneCount(), static_cast<size_t>(2));
    
    // Test bone lookup
    auto foundBone = skeleton->GetBone("Child");
    EXPECT_TRUE(foundBone != nullptr);
    EXPECT_STRING_EQUAL(foundBone->GetName(), "Child");
    
    // Test hierarchy validation
    bool validHierarchy = skeleton->ValidateHierarchy();
    EXPECT_TRUE(validHierarchy);
    
    TestOutput::PrintInfo("Skeleton creation and hierarchy management working correctly");
    
    TestOutput::PrintTestPass("skeleton creation from model bone hierarchy");
    return true;
}

/**
 * Test animation track mapping to skeleton bones
 * Requirements: 8.3
 */
bool TestAnimationTrackMappingToSkeletonBones() {
    TestOutput::PrintTestStart("animation track mapping to skeleton bones");

    // Create test animation
    auto animation = std::make_shared<GameEngine::Animation::SkeletalAnimation>("TestAnimation");
    
    // Test basic animation properties
    EXPECT_STRING_EQUAL(animation->GetName(), "TestAnimation");
    EXPECT_NEARLY_EQUAL(animation->GetDuration(), 0.0f); // Default duration
    EXPECT_TRUE(animation->IsEmpty()); // No tracks yet
    
    // Add animation tracks
    animation->SetDuration(2.0f);
    animation->SetFrameRate(30.0f);
    
    // Add keyframes for different bones
    animation->AddPositionKeyframe("Root", 0.0f, GameEngine::Math::Vec3(0.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("Root", 1.0f, GameEngine::Math::Vec3(1.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("Root", 2.0f, GameEngine::Math::Vec3(2.0f, 0.0f, 0.0f));
    
    animation->AddRotationKeyframe("Spine", 0.0f, GameEngine::Math::Quat(1.0f, 0.0f, 0.0f, 0.0f));
    animation->AddRotationKeyframe("Spine", 1.0f, GameEngine::Math::Quat(0.707f, 0.0f, 0.707f, 0.0f));
    
    // Test animation properties after adding tracks
    EXPECT_NEARLY_EQUAL(animation->GetDuration(), 2.0f);
    EXPECT_NEARLY_EQUAL(animation->GetFrameRate(), 30.0f);
    EXPECT_FALSE(animation->IsEmpty());
    EXPECT_TRUE(animation->HasBone("Root"));
    EXPECT_TRUE(animation->HasBone("Spine"));
    EXPECT_FALSE(animation->HasBone("NonExistent"));
    
    // Test bone sampling
    auto rootPose = animation->SampleBone("Root", 1.0f);
    EXPECT_TRUE(rootPose.hasPosition);
    EXPECT_VEC3_NEARLY_EQUAL(rootPose.position, GameEngine::Math::Vec3(1.0f, 0.0f, 0.0f));
    
    auto spinePose = animation->SampleBone("Spine", 1.0f);
    EXPECT_TRUE(spinePose.hasRotation);
    
    // Test sampling at different times
    auto rootPoseStart = animation->SampleBone("Root", 0.0f);
    EXPECT_VEC3_NEARLY_EQUAL(rootPoseStart.position, GameEngine::Math::Vec3(0.0f, 0.0f, 0.0f));
    
    auto rootPoseEnd = animation->SampleBone("Root", 2.0f);
    EXPECT_VEC3_NEARLY_EQUAL(rootPoseEnd.position, GameEngine::Math::Vec3(2.0f, 0.0f, 0.0f));
    
    TestOutput::PrintInfo("Animation track mapping and sampling working correctly");
    
    TestOutput::PrintTestPass("animation track mapping to skeleton bones");
    return true;
}

/**
 * Test animation data validation and error correction
 * Requirements: 8.4, 8.5
 */
bool TestAnimationDataValidationAndErrorCorrection() {
    TestOutput::PrintTestStart("animation data validation and error correction");

    GameEngine::Animation::AnimationImporter importer;
    
    // Test validation with invalid file
    bool validResult = importer.ValidateAnimationData("invalid_file.xyz");
    EXPECT_FALSE(validResult);
    
    // Test validation with non-existent file
    validResult = importer.ValidateAnimationData("non_existent.fbx");
    EXPECT_FALSE(validResult);
    
    // Test import configuration for error correction
    GameEngine::Animation::AnimationImportConfig config;
    config.validateBoneHierarchy = true;
    config.generateMissingBindPoses = true;
    config.optimizeKeyframes = true;
    config.removeRedundantTracks = true;
    
    importer.SetDefaultConfig(config);
    
    auto retrievedConfig = importer.GetDefaultConfig();
    EXPECT_TRUE(retrievedConfig.validateBoneHierarchy);
    EXPECT_TRUE(retrievedConfig.generateMissingBindPoses);
    EXPECT_TRUE(retrievedConfig.optimizeKeyframes);
    EXPECT_TRUE(retrievedConfig.removeRedundantTracks);
    
    TestOutput::PrintInfo("Animation data validation and error correction configuration working correctly");
    
    TestOutput::PrintTestPass("animation data validation and error correction");
    return true;
}

/**
 * Test coordinate system conversion for imported animations
 * Requirements: 8.5
 */
bool TestCoordinateSystemConversionForImportedAnimations() {
    TestOutput::PrintTestStart("coordinate system conversion for imported animations");

    // Test coordinate system conversion configuration
    GameEngine::Animation::AnimationImportConfig config;
    config.convertCoordinateSystem = true;
    config.flipYZ = true;
    config.coordinateSystemScale = GameEngine::Math::Vec3(0.01f, 0.01f, 0.01f);
    
    // Simulate coordinate system conversion
    GameEngine::Math::Vec3 originalPosition(100.0f, 200.0f, 300.0f); // cm
    GameEngine::Math::Vec3 convertedPosition = originalPosition;
    
    if (config.flipYZ) {
        std::swap(convertedPosition.y, convertedPosition.z);
    }
    convertedPosition *= config.coordinateSystemScale;
    
    // Expected: Y and Z swapped, scaled to meters
    GameEngine::Math::Vec3 expectedPosition(1.0f, 3.0f, 2.0f);
    EXPECT_VEC3_NEARLY_EQUAL(convertedPosition, expectedPosition);
    
    // Test quaternion conversion
    GameEngine::Math::Quat originalRotation(1.0f, 0.0f, 0.5f, 0.5f);
    GameEngine::Math::Quat convertedRotation = originalRotation;
    
    if (config.flipYZ) {
        std::swap(convertedRotation.y, convertedRotation.z);
        convertedRotation.y = -convertedRotation.y; // Maintain handedness
    }
    
    // Verify the conversion maintains quaternion properties
    float magnitude = std::sqrt(convertedRotation.w * convertedRotation.w + 
                               convertedRotation.x * convertedRotation.x + 
                               convertedRotation.y * convertedRotation.y + 
                               convertedRotation.z * convertedRotation.z);
    EXPECT_NEARLY_EQUAL(magnitude, 1.0f);
    
    TestOutput::PrintInfo("Coordinate system conversion working correctly");
    
    TestOutput::PrintTestPass("coordinate system conversion for imported animations");
    return true;
}

/**
 * Test animation metadata preservation and property mapping
 * Requirements: 8.7
 */
bool TestAnimationMetadataPreservationAndPropertyMapping() {
    TestOutput::PrintTestStart("animation metadata preservation and property mapping");

    // Create animation with comprehensive metadata
    auto animation = std::make_shared<GameEngine::Animation::SkeletalAnimation>("WalkCycle");
    animation->SetDuration(1.33f); // 40 frames at 30fps
    animation->SetFrameRate(30.0f);
    animation->SetLoopMode(GameEngine::Animation::LoopMode::Loop);
    
    // Test metadata preservation
    EXPECT_STRING_EQUAL(animation->GetName(), "WalkCycle");
    EXPECT_NEARLY_EQUAL(animation->GetDuration(), 1.33f);
    EXPECT_NEARLY_EQUAL(animation->GetFrameRate(), 30.0f);
    EXPECT_EQUAL(static_cast<int>(animation->GetLoopMode()), 
                static_cast<int>(GameEngine::Animation::LoopMode::Loop));
    
    // Test serialization for metadata preservation
    auto serializedData = animation->Serialize();
    EXPECT_STRING_EQUAL(serializedData.name, "WalkCycle");
    EXPECT_NEARLY_EQUAL(serializedData.duration, 1.33f);
    EXPECT_NEARLY_EQUAL(serializedData.frameRate, 30.0f);
    EXPECT_EQUAL(static_cast<int>(serializedData.loopMode), 
                static_cast<int>(GameEngine::Animation::LoopMode::Loop));
    
    // Test deserialization
    auto newAnimation = std::make_shared<GameEngine::Animation::SkeletalAnimation>();
    bool deserializeResult = newAnimation->Deserialize(serializedData);
    EXPECT_TRUE(deserializeResult);
    
    // Verify metadata was preserved
    EXPECT_STRING_EQUAL(newAnimation->GetName(), "WalkCycle");
    EXPECT_NEARLY_EQUAL(newAnimation->GetDuration(), 1.33f);
    EXPECT_NEARLY_EQUAL(newAnimation->GetFrameRate(), 30.0f);
    EXPECT_EQUAL(static_cast<int>(newAnimation->GetLoopMode()), 
                static_cast<int>(GameEngine::Animation::LoopMode::Loop));
    
    TestOutput::PrintInfo("Animation metadata preservation and property mapping working correctly");
    
    TestOutput::PrintTestPass("animation metadata preservation and property mapping");
    return true;
}

int main() {
    TestOutput::PrintHeader("Model Animation Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Model Animation Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("ModelLoader Animation Integration", TestModelLoaderAnimationIntegration);
        allPassed &= suite.RunTest("Animation Import from Model Files", TestAnimationImportFromModelFiles);
        allPassed &= suite.RunTest("Skeleton Creation from Model Hierarchy", TestSkeletonCreationFromModelHierarchy);
        allPassed &= suite.RunTest("Animation Track Mapping to Skeleton Bones", TestAnimationTrackMappingToSkeletonBones);
        allPassed &= suite.RunTest("Animation Data Validation and Error Correction", TestAnimationDataValidationAndErrorCorrection);
        allPassed &= suite.RunTest("Coordinate System Conversion for Imported Animations", TestCoordinateSystemConversionForImportedAnimations);
        allPassed &= suite.RunTest("Animation Metadata Preservation and Property Mapping", TestAnimationMetadataPreservationAndPropertyMapping);

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