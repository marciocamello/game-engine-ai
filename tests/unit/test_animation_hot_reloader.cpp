#include "TestUtils.h"
#include "Animation/AnimationHotReloader.h"
#include "Animation/SkeletalAnimation.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test animation hot reloader initialization
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAnimationHotReloaderInitialization() {
    TestOutput::PrintTestStart("animation hot reloader initialization");

    AnimationHotReloader reloader;
    
    // Test initialization
    EXPECT_TRUE(reloader.Initialize());
    EXPECT_FALSE(reloader.IsEnabled()); // Should be disabled by default
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), static_cast<size_t>(0));
    
    // Test shutdown
    reloader.Shutdown();

    TestOutput::PrintTestPass("animation hot reloader initialization");
    return true;
}

/**
 * Test animation hot reloader configuration
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAnimationHotReloaderConfiguration() {
    TestOutput::PrintTestStart("animation hot reloader configuration");

    AnimationHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Test enable/disable
    reloader.SetEnabled(true);
    EXPECT_TRUE(reloader.IsEnabled());
    
    reloader.SetEnabled(false);
    EXPECT_FALSE(reloader.IsEnabled());

    // Test check interval
    reloader.SetCheckInterval(2.0f);
    EXPECT_NEARLY_EQUAL(reloader.GetCheckInterval(), 2.0f);

    // Test auto validation
    reloader.SetAutoValidation(false);
    EXPECT_FALSE(reloader.IsAutoValidationEnabled());
    
    reloader.SetAutoValidation(true);
    EXPECT_TRUE(reloader.IsAutoValidationEnabled());

    // Test optimization
    reloader.SetOptimizationEnabled(true);
    EXPECT_TRUE(reloader.IsOptimizationEnabled());

    reloader.Shutdown();

    TestOutput::PrintTestPass("animation hot reloader configuration");
    return true;
}

/**
 * Test animation file watching
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAnimationFileWatching() {
    TestOutput::PrintTestStart("animation file watching");

    AnimationHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create a test animation file
    std::string testFilePath = "test_animation_watch.json";
    std::ofstream testFile(testFilePath);
    testFile << R"({
        "type": "skeletal_animation",
        "version": "1.0.0",
        "name": "TestAnimation",
        "duration": 1.0,
        "frameRate": 30.0,
        "loopMode": 0,
        "boneAnimations": [],
        "events": []
    })";
    testFile.close();

    // Test file watching
    reloader.WatchAnimationFile(testFilePath);
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), static_cast<size_t>(1));
    EXPECT_TRUE(reloader.IsFileWatched(testFilePath));
    EXPECT_EQUAL(reloader.GetAssetType(testFilePath), "skeletal_animation");

    // Test file unwatching
    reloader.UnwatchAnimationFile(testFilePath);
    EXPECT_EQUAL(reloader.GetWatchedFileCount(), static_cast<size_t>(0));
    EXPECT_FALSE(reloader.IsFileWatched(testFilePath));

    // Clean up
    std::remove(testFilePath.c_str());
    reloader.Shutdown();

    TestOutput::PrintTestPass("animation file watching");
    return true;
}

/**
 * Test animation validation
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAnimationValidation() {
    TestOutput::PrintTestStart("animation validation");

    AnimationHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create a valid animation file
    std::string validFilePath = "test_animation_valid.json";
    std::ofstream validFile(validFilePath);
    validFile << R"({
        "type": "skeletal_animation",
        "version": "1.0.0",
        "name": "ValidAnimation",
        "duration": 2.0,
        "frameRate": 30.0,
        "loopMode": 0,
        "boneAnimations": [
            {
                "boneName": "root",
                "positionKeyframes": [
                    {"time": 0.0, "value": [0.0, 0.0, 0.0], "interpolation": 0},
                    {"time": 1.0, "value": [1.0, 0.0, 0.0], "interpolation": 0}
                ]
            }
        ],
        "events": []
    })";
    validFile.close();

    // Create an invalid animation file
    std::string invalidFilePath = "test_animation_invalid.json";
    std::ofstream invalidFile(invalidFilePath);
    invalidFile << R"({
        "type": "skeletal_animation",
        "name": "InvalidAnimation"
    })";
    invalidFile.close();

    // Watch files (auto-validation is enabled by default)
    reloader.WatchAnimationFile(validFilePath);
    reloader.WatchAnimationFile(invalidFilePath);

    // Check validation results
    auto validResult = reloader.GetValidationResult(validFilePath);
    EXPECT_TRUE(validResult.isValid);
    EXPECT_EQUAL(validResult.assetType, "skeletal_animation");

    auto invalidResult = reloader.GetValidationResult(invalidFilePath);
    EXPECT_FALSE(invalidResult.isValid);
    EXPECT_FALSE(invalidResult.errors.empty());

    // Test file validity
    EXPECT_TRUE(reloader.IsFileValid(validFilePath));
    EXPECT_FALSE(reloader.IsFileValid(invalidFilePath));

    // Clean up
    std::remove(validFilePath.c_str());
    std::remove(invalidFilePath.c_str());
    reloader.Shutdown();

    TestOutput::PrintTestPass("animation validation");
    return true;
}

/**
 * Test animation hot reloader callbacks
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAnimationHotReloaderCallbacks() {
    TestOutput::PrintTestStart("animation hot reloader callbacks");

    AnimationHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Set up callback tracking
    bool reloadCallbackCalled = false;
    bool errorCallbackCalled = false;
    bool validationCallbackCalled = false;
    std::string callbackFilePath;
    std::string callbackAssetType;

    reloader.SetReloadCallback([&](const std::string& filepath, const std::string& assetType) {
        reloadCallbackCalled = true;
        callbackFilePath = filepath;
        callbackAssetType = assetType;
    });

    reloader.SetErrorCallback([&](const std::string& filepath, const std::string& error) {
        errorCallbackCalled = true;
    });

    reloader.SetValidationCallback([&](const std::string& filepath, const AnimationValidationResult& result) {
        validationCallbackCalled = true;
    });

    // Create test file
    std::string testFilePath = "test_animation_callback.json";
    std::ofstream testFile(testFilePath);
    testFile << R"({
        "type": "skeletal_animation",
        "version": "1.0.0",
        "name": "CallbackTestAnimation",
        "duration": 1.0,
        "frameRate": 30.0,
        "loopMode": 0,
        "boneAnimations": [],
        "events": []
    })";
    testFile.close();

    // Watch file and trigger reload
    reloader.WatchAnimationFile(testFilePath);
    reloader.ReloadAnimation(testFilePath);

    // Check callbacks
    EXPECT_TRUE(reloadCallbackCalled);
    EXPECT_TRUE(validationCallbackCalled);
    EXPECT_EQUAL(callbackAssetType, "skeletal_animation");

    // Clean up
    std::remove(testFilePath.c_str());
    reloader.Shutdown();

    TestOutput::PrintTestPass("animation hot reloader callbacks");
    return true;
}

/**
 * Test animation development workflow
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAnimationDevelopmentWorkflow() {
    TestOutput::PrintTestStart("animation development workflow");

    AnimationDevelopmentWorkflow workflow;
    EXPECT_TRUE(workflow.Initialize());

    // Test configuration
    workflow.SetProjectDirectory("test_project");
    workflow.SetOutputDirectory("test_output");
    workflow.SetSourceDirectory("test_source");

    // Test live preview
    EXPECT_FALSE(workflow.IsLivePreviewActive());
    workflow.StartLivePreview();
    EXPECT_TRUE(workflow.IsLivePreviewActive());
    workflow.StopLivePreview();
    EXPECT_FALSE(workflow.IsLivePreviewActive());

    // Test asset watching
    EXPECT_TRUE(workflow.IsAssetWatchingEnabled());
    workflow.EnableAssetWatching(false);
    EXPECT_FALSE(workflow.IsAssetWatchingEnabled());
    workflow.EnableAssetWatching(true);
    EXPECT_TRUE(workflow.IsAssetWatchingEnabled());

    // Test statistics
    auto stats = workflow.GetStatistics();
    EXPECT_EQUAL(stats.totalAssets, static_cast<size_t>(0));
    EXPECT_EQUAL(stats.validAssets, static_cast<size_t>(0));
    EXPECT_EQUAL(stats.invalidAssets, static_cast<size_t>(0));

    workflow.Shutdown();

    TestOutput::PrintTestPass("animation development workflow");
    return true;
}

/**
 * Test asset type detection
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestAssetTypeDetection() {
    TestOutput::PrintTestStart("asset type detection");

    AnimationHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create different asset types
    std::string animFilePath = "test_skeletal_anim.json";
    std::ofstream animFile(animFilePath);
    animFile << R"({"type": "skeletal_animation", "version": "1.0.0", "name": "Test"})";
    animFile.close();

    std::string stateMachineFilePath = "test_state_machine.json";
    std::ofstream stateMachineFile(stateMachineFilePath);
    stateMachineFile << R"({"type": "state_machine", "version": "1.0.0", "states": []})";
    stateMachineFile.close();

    std::string blendTreeFilePath = "test_blend_tree.json";
    std::ofstream blendTreeFile(blendTreeFilePath);
    blendTreeFile << R"({"type": "blend_tree", "version": "1.0.0", "blendType": 0})";
    blendTreeFile.close();

    // Watch files and check asset types
    reloader.WatchAnimationFile(animFilePath);
    reloader.WatchAnimationFile(stateMachineFilePath);
    reloader.WatchAnimationFile(blendTreeFilePath);

    EXPECT_EQUAL(reloader.GetAssetType(animFilePath), "skeletal_animation");
    EXPECT_EQUAL(reloader.GetAssetType(stateMachineFilePath), "state_machine");
    EXPECT_EQUAL(reloader.GetAssetType(blendTreeFilePath), "blend_tree");

    // Clean up
    std::remove(animFilePath.c_str());
    std::remove(stateMachineFilePath.c_str());
    std::remove(blendTreeFilePath.c_str());
    reloader.Shutdown();

    TestOutput::PrintTestPass("asset type detection");
    return true;
}

/**
 * Test report generation
 * Requirements: 10.6, 7.7, 10.4
 */
bool TestReportGeneration() {
    TestOutput::PrintTestStart("report generation");

    AnimationHotReloader reloader;
    EXPECT_TRUE(reloader.Initialize());

    // Create test files
    std::string testFile1 = "test_report_1.json";
    std::ofstream file1(testFile1);
    file1 << R"({"type": "skeletal_animation", "version": "1.0.0", "name": "Test1"})";
    file1.close();

    std::string testFile2 = "test_report_2.json";
    std::ofstream file2(testFile2);
    file2 << R"({"invalid": "data"})";
    file2.close();

    // Watch files
    reloader.WatchAnimationFile(testFile1);
    reloader.WatchAnimationFile(testFile2);
    reloader.ValidateAllAnimations();

    // Generate reports
    std::string reportPath = "test_asset_report.txt";
    std::string statsPath = "test_asset_stats.json";
    
    reloader.GenerateAssetReport(reportPath);
    reloader.ExportAssetStatistics(statsPath);

    // Check if reports were created
    EXPECT_TRUE(std::filesystem::exists(reportPath));
    EXPECT_TRUE(std::filesystem::exists(statsPath));

    // Clean up
    std::remove(testFile1.c_str());
    std::remove(testFile2.c_str());
    std::remove(reportPath.c_str());
    std::remove(statsPath.c_str());
    reloader.Shutdown();

    TestOutput::PrintTestPass("report generation");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationHotReloader");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationHotReloader Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Hot Reloader Initialization", TestAnimationHotReloaderInitialization);
        allPassed &= suite.RunTest("Animation Hot Reloader Configuration", TestAnimationHotReloaderConfiguration);
        allPassed &= suite.RunTest("Animation File Watching", TestAnimationFileWatching);
        allPassed &= suite.RunTest("Animation Validation", TestAnimationValidation);
        allPassed &= suite.RunTest("Animation Hot Reloader Callbacks", TestAnimationHotReloaderCallbacks);
        allPassed &= suite.RunTest("Animation Development Workflow", TestAnimationDevelopmentWorkflow);
        allPassed &= suite.RunTest("Asset Type Detection", TestAssetTypeDetection);
        allPassed &= suite.RunTest("Report Generation", TestReportGeneration);

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