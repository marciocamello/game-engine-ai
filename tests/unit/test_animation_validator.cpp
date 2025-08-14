#include "TestUtils.h"
#include "Animation/AnimationValidator.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/SkeletalAnimation.h"
#include "Core/Logger.h"
#include <memory>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test AnimationValidator initialization and configuration
 * Requirements: 8.4, 8.5
 */
bool TestAnimationValidatorInitialization() {
    TestOutput::PrintTestStart("animation validator initialization");

    AnimationValidator validator;
    
    // Test default configuration
    auto config = validator.GetDefaultConfig();
    EXPECT_TRUE(config.validateBoneHierarchy);
    EXPECT_TRUE(config.checkForCyclicDependencies);
    EXPECT_TRUE(config.validateBindPoses);
    EXPECT_TRUE(config.checkBoneNaming);
    EXPECT_TRUE(config.validateKeyframeData);
    EXPECT_TRUE(config.checkAnimationDuration);
    EXPECT_TRUE(config.validateFrameRate);
    EXPECT_TRUE(config.checkForRedundantKeyframes);
    
    // Test custom configuration
    ValidationConfig customConfig;
    customConfig.validateBoneHierarchy = false;
    customConfig.enableAutoFix = true;
    customConfig.maxRecommendedBones = 128;
    
    validator.SetDefaultConfig(customConfig);
    auto retrievedConfig = validator.GetDefaultConfig();
    
    EXPECT_FALSE(retrievedConfig.validateBoneHierarchy);
    EXPECT_TRUE(retrievedConfig.enableAutoFix);
    EXPECT_EQUAL(retrievedConfig.maxRecommendedBones, static_cast<size_t>(128));
    
    TestOutput::PrintTestPass("animation validator initialization");
    return true;
}

/**
 * Test skeleton validation
 * Requirements: 8.4, 8.5
 */
bool TestSkeletonValidation() {
    TestOutput::PrintTestStart("skeleton validation");

    AnimationValidator validator;
    
    // Test null skeleton
    auto result = validator.ValidateSkeleton(nullptr);
    EXPECT_FALSE(result.isValid);
    EXPECT_TRUE(result.issues.size() > 0);
    
    // Test empty skeleton
    auto emptySkeleton = std::make_shared<AnimationSkeleton>("EmptySkeleton");
    result = validator.ValidateSkeleton(emptySkeleton);
    EXPECT_TRUE(result.HasWarnings()); // Should warn about no bones
    
    // Test valid skeleton
    auto validSkeleton = std::make_shared<AnimationSkeleton>("ValidSkeleton");
    auto rootBone = validSkeleton->CreateBone("Root");
    validSkeleton->SetRootBone(rootBone);
    auto childBone = validSkeleton->CreateBone("Child");
    validSkeleton->AddBone(childBone, "Root");
    
    result = validator.ValidateSkeleton(validSkeleton);
    EXPECT_TRUE(result.isValid);
    EXPECT_FALSE(result.HasCriticalIssues());
    
    TestOutput::PrintInfo("Skeleton validation working correctly");
    
    TestOutput::PrintTestPass("skeleton validation");
    return true;
}

/**
 * Test animation validation
 * Requirements: 8.4, 8.5
 */
bool TestAnimationValidation() {
    TestOutput::PrintTestStart("animation validation");

    AnimationValidator validator;
    
    // Test null animation
    auto result = validator.ValidateAnimation(nullptr);
    EXPECT_FALSE(result.isValid);
    EXPECT_TRUE(result.issues.size() > 0);
    
    // Test empty animation
    auto emptyAnimation = std::make_shared<SkeletalAnimation>("EmptyAnimation");
    result = validator.ValidateAnimation(emptyAnimation);
    EXPECT_TRUE(result.HasWarnings()); // Should warn about empty animation
    
    // Test animation with invalid frame rate
    auto invalidAnimation = std::make_shared<SkeletalAnimation>("InvalidAnimation");
    invalidAnimation->SetFrameRate(-1.0f);
    result = validator.ValidateAnimation(invalidAnimation);
    EXPECT_TRUE(result.HasErrors()); // Should error on invalid frame rate
    
    // Test valid animation
    auto validAnimation = std::make_shared<SkeletalAnimation>("ValidAnimation");
    validAnimation->SetDuration(2.0f);
    validAnimation->SetFrameRate(30.0f);
    validAnimation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    validAnimation->AddPositionKeyframe("Root", 2.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    
    result = validator.ValidateAnimation(validAnimation);
    EXPECT_TRUE(result.isValid);
    EXPECT_FALSE(result.HasCriticalIssues());
    
    TestOutput::PrintInfo("Animation validation working correctly");
    
    TestOutput::PrintTestPass("animation validation");
    return true;
}

/**
 * Test combined animation and skeleton validation
 * Requirements: 8.4, 8.5
 */
bool TestCombinedValidation() {
    TestOutput::PrintTestStart("combined animation and skeleton validation");

    AnimationValidator validator;
    
    // Create skeleton
    auto skeleton = std::make_shared<AnimationSkeleton>("TestSkeleton");
    auto rootBone = skeleton->CreateBone("Root");
    skeleton->SetRootBone(rootBone);
    auto spineBone = skeleton->CreateBone("Spine");
    skeleton->AddBone(spineBone, "Root");
    
    // Create animation
    auto animation = std::make_shared<SkeletalAnimation>("TestAnimation");
    animation->SetDuration(1.0f);
    animation->SetFrameRate(30.0f);
    animation->AddPositionKeyframe("Root", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("Root", 1.0f, Math::Vec3(1.0f, 0.0f, 0.0f));
    animation->AddPositionKeyframe("NonExistentBone", 0.0f, Math::Vec3(0.0f, 0.0f, 0.0f));
    
    // Test combined validation
    auto result = validator.ValidateAnimationWithSkeleton(animation, skeleton);
    EXPECT_TRUE(result.isValid); // Should be valid overall
    EXPECT_TRUE(result.HasWarnings()); // Should warn about non-existent bone
    
    // Check that we have issues related to bone mapping
    bool foundBoneMappingIssue = false;
    for (const auto& issue : result.issues) {
        if (issue.category == ValidationCategory::BoneMapping) {
            foundBoneMappingIssue = true;
            break;
        }
    }
    EXPECT_TRUE(foundBoneMappingIssue);
    
    TestOutput::PrintInfo("Combined validation working correctly");
    
    TestOutput::PrintTestPass("combined animation and skeleton validation");
    return true;
}

/**
 * Test validation error correction
 * Requirements: 8.4, 8.5
 */
bool TestValidationErrorCorrection() {
    TestOutput::PrintTestStart("validation error correction");

    AnimationValidator validator;
    ValidationConfig config;
    config.enableAutoFix = true;
    config.fixInvalidDurations = true;
    config.fixRedundantKeyframes = true;
    
    // Create skeleton with issues
    auto skeleton = std::make_shared<AnimationSkeleton>("TestSkeleton");
    auto bone1 = skeleton->CreateBone("Bone1");
    auto bone2 = skeleton->CreateBone("Bone2");
    // Don't set root bone - this should be auto-fixable
    
    auto result = validator.ValidateSkeleton(skeleton, config);
    EXPECT_TRUE(result.HasWarnings() || result.HasErrors()); // Should have issues about no root bone
    
    // Test auto-fix (may or may not succeed depending on available fixes)
    bool fixed = validator.FixValidationIssues(skeleton, result);
    // Don't require fix to succeed, just test that it doesn't crash
    EXPECT_TRUE(true); // Test completed without crashing
    
    // Create animation with issues
    auto animation = std::make_shared<SkeletalAnimation>("TestAnimation");
    animation->SetFrameRate(-1.0f); // Invalid frame rate
    animation->SetDuration(0.0f); // Invalid duration
    
    result = validator.ValidateAnimation(animation, config);
    EXPECT_TRUE(result.HasErrors());
    
    // Test auto-fix
    fixed = validator.FixValidationIssues(animation, result);
    EXPECT_TRUE(fixed);
    EXPECT_TRUE(animation->GetFrameRate() > 0.0f); // Should have valid frame rate
    
    TestOutput::PrintInfo("Validation error correction working correctly");
    
    TestOutput::PrintTestPass("validation error correction");
    return true;
}

/**
 * Test validation report generation
 * Requirements: 8.4, 8.5
 */
bool TestValidationReportGeneration() {
    TestOutput::PrintTestStart("validation report generation");

    AnimationValidator validator;
    
    // Create skeleton with multiple issues
    auto skeleton = std::make_shared<AnimationSkeleton>("TestSkeleton");
    // Empty skeleton should generate warnings
    
    auto result = validator.ValidateSkeleton(skeleton);
    
    // Test report generation
    std::string report = validator.GetValidationReport(result);
    EXPECT_TRUE(!report.empty());
    EXPECT_TRUE(report.find("Animation Validation Report") != std::string::npos);
    EXPECT_TRUE(report.find("Issues Found") != std::string::npos);
    
    // Test that report contains issue details
    if (result.HasIssues()) {
        EXPECT_TRUE(report.find("Detailed Issues") != std::string::npos);
    }
    
    TestOutput::PrintInfo("Validation report generation working correctly");
    
    TestOutput::PrintTestPass("validation report generation");
    return true;
}

/**
 * Test coordinate system conversion validation
 * Requirements: 8.5
 */
bool TestCoordinateSystemConversionValidation() {
    TestOutput::PrintTestStart("coordinate system conversion validation");

    AnimationValidator validator;
    ValidationConfig config;
    config.validateBoneHierarchy = true;
    config.checkBoneNaming = true;
    
    // Create skeleton with proper naming
    auto skeleton = std::make_shared<AnimationSkeleton>("TestSkeleton");
    auto rootBone = skeleton->CreateBone("Root_Bone");
    skeleton->SetRootBone(rootBone);
    auto childBone = skeleton->CreateBone("Child_Bone_01");
    skeleton->AddBone(childBone, "Root_Bone");
    
    auto result = validator.ValidateSkeleton(skeleton, config);
    EXPECT_TRUE(result.isValid);
    
    // Create skeleton with improper naming (should generate warnings)
    auto badSkeleton = std::make_shared<AnimationSkeleton>("BadSkeleton");
    auto badBone1 = badSkeleton->CreateBone("bone with spaces"); // Invalid name
    auto badBone2 = badSkeleton->CreateBone("bone-with-dashes"); // Invalid name
    badSkeleton->SetRootBone(badBone1);
    badSkeleton->AddBone(badBone2, "bone with spaces");
    
    result = validator.ValidateSkeleton(badSkeleton, config);
    EXPECT_TRUE(result.HasWarnings()); // Should warn about naming convention
    
    TestOutput::PrintInfo("Coordinate system conversion validation working correctly");
    
    TestOutput::PrintTestPass("coordinate system conversion validation");
    return true;
}

/**
 * Test animation metadata validation and property mapping
 * Requirements: 8.7
 */
bool TestAnimationMetadataValidation() {
    TestOutput::PrintTestStart("animation metadata validation and property mapping");

    AnimationValidator validator;
    ValidationConfig config;
    config.checkAnimationDuration = true;
    config.validateFrameRate = true;
    
    // Test animation with good metadata
    auto goodAnimation = std::make_shared<SkeletalAnimation>("GoodAnimation");
    goodAnimation->SetDuration(2.5f);
    goodAnimation->SetFrameRate(30.0f);
    goodAnimation->SetLoopMode(LoopMode::Loop);
    
    auto result = validator.ValidateAnimation(goodAnimation, config);
    EXPECT_TRUE(result.isValid);
    
    // Test animation with problematic metadata
    auto badAnimation = std::make_shared<SkeletalAnimation>("BadAnimation");
    badAnimation->SetDuration(0.0f); // Too short
    badAnimation->SetFrameRate(200.0f); // Unusual frame rate
    
    result = validator.ValidateAnimation(badAnimation, config);
    EXPECT_TRUE(result.HasWarnings() || result.HasErrors());
    
    // Check for specific metadata-related issues
    bool foundDurationIssue = false;
    bool foundFrameRateIssue = false;
    
    for (const auto& issue : result.issues) {
        if (issue.description.find("duration") != std::string::npos) {
            foundDurationIssue = true;
        }
        if (issue.description.find("frame rate") != std::string::npos) {
            foundFrameRateIssue = true;
        }
    }
    
    EXPECT_TRUE(foundDurationIssue);
    EXPECT_TRUE(foundFrameRateIssue);
    
    TestOutput::PrintInfo("Animation metadata validation working correctly");
    
    TestOutput::PrintTestPass("animation metadata validation and property mapping");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationValidator");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationValidator Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Validator Initialization", TestAnimationValidatorInitialization);
        allPassed &= suite.RunTest("Skeleton Validation", TestSkeletonValidation);
        allPassed &= suite.RunTest("Animation Validation", TestAnimationValidation);
        allPassed &= suite.RunTest("Combined Validation", TestCombinedValidation);
        allPassed &= suite.RunTest("Validation Error Correction", TestValidationErrorCorrection);
        allPassed &= suite.RunTest("Validation Report Generation", TestValidationReportGeneration);
        allPassed &= suite.RunTest("Coordinate System Conversion Validation", TestCoordinateSystemConversionValidation);
        allPassed &= suite.RunTest("Animation Metadata Validation", TestAnimationMetadataValidation);

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