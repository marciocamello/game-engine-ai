#include "TestUtils.h"
#include "Animation/AnimationDebugRenderer.h"
#include "Animation/AnimationProfiler.h"
#include "Animation/AnimationSkeleton.h"
#include "Animation/AnimationController.h"
#include "Physics/PhysicsDebugDrawer.h"
#include <memory>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test AnimationDebugRenderer initialization and basic functionality
 * Requirements: 10.1, 10.3, 10.6
 */
bool TestAnimationDebugRendererInitialization() {
    TestOutput::PrintTestStart("animation debug renderer initialization");

    // Create a simple debug drawer for testing
    auto debugDrawer = std::make_shared<Physics::SimplePhysicsDebugDrawer>();
    
    AnimationDebugRenderer debugRenderer;
    EXPECT_TRUE(debugRenderer.Initialize(debugDrawer));
    
    // Test debug mode settings
    debugRenderer.SetDebugMode(AnimationDebugMode::Skeleton);
    EXPECT_TRUE(debugRenderer.IsDebugModeEnabled(AnimationDebugMode::Skeleton));
    EXPECT_FALSE(debugRenderer.IsDebugModeEnabled(AnimationDebugMode::StateMachine));
    
    // Test enabling multiple modes
    debugRenderer.EnableDebugMode(AnimationDebugMode::IKChains, true);
    EXPECT_TRUE(debugRenderer.IsDebugModeEnabled(AnimationDebugMode::Skeleton));
    EXPECT_TRUE(debugRenderer.IsDebugModeEnabled(AnimationDebugMode::IKChains));
    
    // Test disabling modes
    debugRenderer.EnableDebugMode(AnimationDebugMode::Skeleton, false);
    EXPECT_FALSE(debugRenderer.IsDebugModeEnabled(AnimationDebugMode::Skeleton));
    EXPECT_TRUE(debugRenderer.IsDebugModeEnabled(AnimationDebugMode::IKChains));

    TestOutput::PrintTestPass("animation debug renderer initialization");
    return true;
}

/**
 * Test AnimationDebugRenderer skeleton visualization
 * Requirements: 10.1, 10.6
 */
bool TestAnimationDebugRendererSkeletonVisualization() {
    TestOutput::PrintTestStart("animation debug renderer skeleton visualization");

    auto debugDrawer = std::make_shared<Physics::SimplePhysicsDebugDrawer>();
    AnimationDebugRenderer debugRenderer;
    EXPECT_TRUE(debugRenderer.Initialize(debugDrawer));
    
    // Enable skeleton debug mode
    debugRenderer.SetDebugMode(AnimationDebugMode::Skeleton);
    
    // Test drawing individual components (skeleton drawing requires complex setup)
    Math::Vec3 startPos(0.0f, 0.0f, 0.0f);
    Math::Vec3 endPos(1.0f, 0.0f, 0.0f);
    debugRenderer.DrawBone(startPos, endPos, 0.05f, Math::Vec3(1.0f, 1.0f, 0.0f));
    
    Math::Vec3 jointPos(0.5f, 0.0f, 0.0f);
    debugRenderer.DrawJoint(jointPos, 0.1f, Math::Vec3(0.0f, 1.0f, 0.0f));
    
    // Test configuration
    debugRenderer.SetBoneThickness(0.1f);
    debugRenderer.SetJointRadius(0.2f);
    debugRenderer.SetSkeletonColor(Math::Vec3(1.0f, 0.0f, 0.0f));
    
    // Clear debug drawing
    debugRenderer.Clear();

    TestOutput::PrintTestPass("animation debug renderer skeleton visualization");
    return true;
}

/**
 * Test AnimationProfiler initialization and basic functionality
 * Requirements: 10.2, 10.5, 10.4
 */
bool TestAnimationProfilerInitialization() {
    TestOutput::PrintTestStart("animation profiler initialization");

    AnimationProfiler profiler;
    EXPECT_TRUE(profiler.Initialize());
    
    // Test profiling control
    EXPECT_FALSE(profiler.IsProfilingActive());
    
    profiler.StartProfiling();
    EXPECT_TRUE(profiler.IsProfilingActive());
    
    profiler.PauseProfiling();
    EXPECT_FALSE(profiler.IsProfilingActive());
    
    profiler.ResumeProfiling();
    EXPECT_TRUE(profiler.IsProfilingActive());
    
    profiler.StopProfiling();
    EXPECT_FALSE(profiler.IsProfilingActive());

    TestOutput::PrintTestPass("animation profiler initialization");
    return true;
}

/**
 * Test AnimationProfiler timing functionality
 * Requirements: 10.2, 10.5
 */
bool TestAnimationProfilerTiming() {
    TestOutput::PrintTestStart("animation profiler timing");

    AnimationProfiler profiler;
    EXPECT_TRUE(profiler.Initialize());
    profiler.StartProfiling();
    
    // Test frame timing
    profiler.BeginFrame();
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    profiler.EndFrame();
    
    // Test operation timing
    const std::string operationName = "TestOperation";
    profiler.BeginOperation(operationName);
    
    // Simulate operation work
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    
    profiler.EndOperation(operationName);
    
    // Get timing data
    auto timingData = profiler.GetOperationTiming(operationName);
    EXPECT_TRUE(timingData.sampleCount > 0);
    EXPECT_TRUE(timingData.lastTimeMs > 0.0);
    EXPECT_TRUE(timingData.averageTimeMs > 0.0);
    
    // Test performance stats
    auto perfStats = profiler.GetPerformanceStats();
    EXPECT_TRUE(perfStats.framesSinceLastReset > 0);

    TestOutput::PrintTestPass("animation profiler timing");
    return true;
}

/**
 * Test AnimationProfiler validation functionality
 * Requirements: 10.4, 10.5
 */
bool TestAnimationProfilerValidation() {
    TestOutput::PrintTestStart("animation profiler validation");

    AnimationProfiler profiler;
    EXPECT_TRUE(profiler.Initialize());
    
    // Test performance issue detection
    AnimationPerformanceStats perfStats;
    perfStats.frameTimeMs = 50.0; // Exceeds typical 16.67ms target
    perfStats.animatedCharacterCount = 100; // High character count
    
    auto perfIssues = profiler.DetectPerformanceIssues(perfStats);
    EXPECT_TRUE(perfIssues.size() > 0); // Should detect performance issues
    
    // Test memory issue detection
    AnimationMemoryStats memStats;
    memStats.totalMemory = 200 * 1024 * 1024; // 200MB - high usage
    
    auto memIssues = profiler.DetectMemoryIssues(memStats);
    EXPECT_TRUE(memIssues.size() > 0); // Should detect memory issues

    TestOutput::PrintTestPass("animation profiler validation");
    return true;
}

/**
 * Test AnimationProfiler report generation
 * Requirements: 10.2, 10.4, 10.5
 */
bool TestAnimationProfilerReports() {
    TestOutput::PrintTestStart("animation profiler reports");

    AnimationProfiler profiler;
    EXPECT_TRUE(profiler.Initialize());
    profiler.StartProfiling();
    
    // Generate some timing data
    profiler.BeginFrame();
    profiler.BeginOperation("TestOp");
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    profiler.EndOperation("TestOp");
    profiler.EndFrame();
    
    // Test report generation
    std::string perfReport = profiler.GeneratePerformanceReport();
    EXPECT_TRUE(!perfReport.empty());
    EXPECT_TRUE(perfReport.find("Performance Report") != std::string::npos);
    
    std::string memReport = profiler.GenerateMemoryReport();
    EXPECT_TRUE(!memReport.empty());
    EXPECT_TRUE(memReport.find("Memory Report") != std::string::npos);
    
    // Test validation report
    AnimationValidationReport validationReport;
    AnimationValidationIssue issue;
    issue.type = AnimationValidationIssueType::Warning;
    issue.category = "Test";
    issue.description = "Test issue";
    issue.suggestion = "Test suggestion";
    issue.severity = 0.5f;
    validationReport.issues.push_back(issue);
    validationReport.CalculateCounts();
    
    std::string validationReportStr = profiler.GenerateValidationReport(validationReport);
    EXPECT_TRUE(!validationReportStr.empty());
    EXPECT_TRUE(validationReportStr.find("Validation Report") != std::string::npos);

    TestOutput::PrintTestPass("animation profiler reports");
    return true;
}

/**
 * Test AnimationTimer functionality
 * Requirements: 10.2, 10.5
 */
bool TestAnimationTimer() {
    TestOutput::PrintTestStart("animation timer");

    AnimationTimer timer;
    EXPECT_FALSE(timer.IsRunning());
    
    timer.Start();
    EXPECT_TRUE(timer.IsRunning());
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    timer.Stop();
    EXPECT_FALSE(timer.IsRunning());
    
    double elapsedMs = timer.GetElapsedMs();
    EXPECT_TRUE(elapsedMs > 0.0);
    EXPECT_TRUE(elapsedMs >= 1.0); // Should be at least 1ms
    
    double elapsedMicros = timer.GetElapsedMicroseconds();
    EXPECT_TRUE(elapsedMicros > 0.0);
    EXPECT_TRUE(elapsedMicros >= 1000.0); // Should be at least 1000 microseconds

    TestOutput::PrintTestPass("animation timer");
    return true;
}

/**
 * Test AnimationTimingData functionality
 * Requirements: 10.2, 10.5
 */
bool TestAnimationTimingData() {
    TestOutput::PrintTestStart("animation timing data");

    AnimationTimingData timingData;
    timingData.operationName = "TestOperation";
    
    // Initially should be empty
    EXPECT_EQUAL(timingData.sampleCount, static_cast<uint32_t>(0));
    EXPECT_NEARLY_EQUAL(timingData.averageTimeMs, 0.0);
    
    // Add some samples
    timingData.AddSample(1.0);
    EXPECT_EQUAL(timingData.sampleCount, static_cast<uint32_t>(1));
    EXPECT_NEARLY_EQUAL(timingData.averageTimeMs, 1.0);
    EXPECT_NEARLY_EQUAL(timingData.minTimeMs, 1.0);
    EXPECT_NEARLY_EQUAL(timingData.maxTimeMs, 1.0);
    
    timingData.AddSample(3.0);
    EXPECT_EQUAL(timingData.sampleCount, static_cast<uint32_t>(2));
    EXPECT_NEARLY_EQUAL(timingData.averageTimeMs, 2.0);
    EXPECT_NEARLY_EQUAL(timingData.minTimeMs, 1.0);
    EXPECT_NEARLY_EQUAL(timingData.maxTimeMs, 3.0);
    
    timingData.AddSample(2.0);
    EXPECT_EQUAL(timingData.sampleCount, static_cast<uint32_t>(3));
    EXPECT_NEARLY_EQUAL(timingData.averageTimeMs, 2.0);
    EXPECT_NEARLY_EQUAL(timingData.minTimeMs, 1.0);
    EXPECT_NEARLY_EQUAL(timingData.maxTimeMs, 3.0);
    
    // Test reset
    timingData.Reset();
    EXPECT_EQUAL(timingData.sampleCount, static_cast<uint32_t>(0));
    EXPECT_NEARLY_EQUAL(timingData.averageTimeMs, 0.0);

    TestOutput::PrintTestPass("animation timing data");
    return true;
}

int main() {
    TestOutput::PrintHeader("Animation Debug Tools");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Animation Debug Tools Tests");

        // Run all tests
        allPassed &= suite.RunTest("Animation Debug Renderer Initialization", TestAnimationDebugRendererInitialization);
        allPassed &= suite.RunTest("Animation Debug Renderer Skeleton Visualization", TestAnimationDebugRendererSkeletonVisualization);
        allPassed &= suite.RunTest("Animation Profiler Initialization", TestAnimationProfilerInitialization);
        allPassed &= suite.RunTest("Animation Profiler Timing", TestAnimationProfilerTiming);
        allPassed &= suite.RunTest("Animation Profiler Validation", TestAnimationProfilerValidation);
        allPassed &= suite.RunTest("Animation Profiler Reports", TestAnimationProfilerReports);
        allPassed &= suite.RunTest("Animation Timer", TestAnimationTimer);
        allPassed &= suite.RunTest("Animation Timing Data", TestAnimationTimingData);

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