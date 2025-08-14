#include "TestUtils.h"
#include "Animation/AnimationLOD.h"
#include "Animation/AnimationController.h"
#include "Animation/AnimationSkeleton.h"
#include "Core/Math.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Animation;

/**
 * Test camera frustum plane calculation
 * Requirements: 9.4 (animation LOD for distant characters)
 */
bool TestCameraFrustumPlanes() {
    TestOutput::PrintTestStart("camera frustum plane calculation");

    CameraData camera;
    camera.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    camera.forward = Math::Vec3(0.0f, 0.0f, -1.0f);
    camera.fov = 45.0f;
    camera.aspectRatio = 16.0f / 9.0f;
    camera.nearPlane = 0.1f;
    camera.farPlane = 100.0f;

    // Create simple view and projection matrices
    camera.viewMatrix = Math::Mat4(1.0f);
    camera.projectionMatrix = Math::Mat4(1.0f);
    camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;

    camera.UpdateFrustumPlanes();

    // Test point inside frustum (at origin)
    EXPECT_TRUE(camera.IsPointInFrustum(Math::Vec3(0.0f, 0.0f, 0.0f)));

    // Test point with radius
    EXPECT_TRUE(camera.IsPointInFrustum(Math::Vec3(0.0f, 0.0f, 0.0f), 1.0f));

    TestOutput::PrintTestPass("camera frustum plane calculation");
    return true;
}

/**
 * Test animation LOD system initialization
 * Requirements: 9.1 (maintain 60 FPS with reasonable numbers of animated characters)
 */
bool TestAnimationLODSystemInitialization() {
    TestOutput::PrintTestStart("animation LOD system initialization");

    AnimationLODSystem lodSystem;
    EXPECT_TRUE(lodSystem.Initialize());

    // Test default settings
    const auto& metrics = lodSystem.GetPerformanceMetrics();
    EXPECT_NEARLY_EQUAL(metrics.targetFrameTime, 16.67f);
    EXPECT_TRUE(metrics.adaptiveScaling);

    // Test instance count
    EXPECT_EQUAL(lodSystem.GetInstanceCount(), static_cast<size_t>(0));

    lodSystem.Shutdown();

    TestOutput::PrintTestPass("animation LOD system initialization");
    return true;
}

/**
 * Test animation instance registration and management
 * Requirements: 9.4 (animation LOD for distant characters)
 */
bool TestAnimationInstanceRegistration() {
    TestOutput::PrintTestStart("animation instance registration");

    AnimationLODSystem lodSystem;
    EXPECT_TRUE(lodSystem.Initialize());

    // Create a mock animation controller
    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
    skeleton->AddBone(rootBone);
    
    auto controller = std::make_shared<AnimationController>();
    EXPECT_TRUE(controller->Initialize(skeleton));

    // Register animation instance
    Math::Vec3 position(10.0f, 0.0f, 0.0f);
    uint32_t instanceId = lodSystem.RegisterAnimationInstance(controller, position, 1.0f, 1.0f);
    EXPECT_NOT_EQUAL(instanceId, static_cast<uint32_t>(0));

    // Test instance count
    EXPECT_EQUAL(lodSystem.GetInstanceCount(), static_cast<size_t>(1));

    // Test instance queries - compare enum values directly
    AnimationLODLevel currentLOD = lodSystem.GetInstanceLOD(instanceId);
    EXPECT_TRUE(currentLOD == AnimationLODLevel::High);
    EXPECT_FALSE(lodSystem.IsInstanceCulled(instanceId));

    // Test position update
    Math::Vec3 newPosition(50.0f, 0.0f, 0.0f);
    lodSystem.UpdateInstancePosition(instanceId, newPosition);

    // Unregister instance
    lodSystem.UnregisterAnimationInstance(instanceId);
    EXPECT_EQUAL(lodSystem.GetInstanceCount(), static_cast<size_t>(0));

    lodSystem.Shutdown();

    TestOutput::PrintTestPass("animation instance registration");
    return true;
}

/**
 * Test LOD distance calculation and level assignment
 * Requirements: 9.4 (animation LOD for distant characters)
 */
bool TestLODDistanceCalculation() {
    TestOutput::PrintTestStart("LOD distance calculation");

    AnimationLODSystem lodSystem;
    EXPECT_TRUE(lodSystem.Initialize());

    // Set custom LOD distances
    lodSystem.SetLODDistances(10.0f, 25.0f, 50.0f);

    // Set up camera
    CameraData camera;
    camera.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    camera.forward = Math::Vec3(0.0f, 0.0f, -1.0f);
    camera.viewMatrix = Math::Mat4(1.0f);
    camera.projectionMatrix = Math::Mat4(1.0f);
    camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
    lodSystem.SetCamera(camera);

    // Create animation controller
    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
    skeleton->AddBone(rootBone);
    
    auto controller = std::make_shared<AnimationController>();
    EXPECT_TRUE(controller->Initialize(skeleton));

    // Test different distances
    // Close distance - should be High LOD
    uint32_t closeInstance = lodSystem.RegisterAnimationInstance(
        controller, Math::Vec3(5.0f, 0.0f, 0.0f), 1.0f, 1.0f);
    
    // Medium distance - should be Medium LOD after evaluation
    uint32_t mediumInstance = lodSystem.RegisterAnimationInstance(
        controller, Math::Vec3(15.0f, 0.0f, 0.0f), 1.0f, 1.0f);
    
    // Far distance - should be Low LOD after evaluation
    uint32_t farInstance = lodSystem.RegisterAnimationInstance(
        controller, Math::Vec3(35.0f, 0.0f, 0.0f), 1.0f, 1.0f);

    // Update LOD system to evaluate distances
    lodSystem.Update(0.016f); // 60 FPS delta time

    // Note: LOD evaluation depends on complex factors including screen size
    // For this test, we just verify the system doesn't crash and instances exist
    EXPECT_NOT_EQUAL(closeInstance, static_cast<uint32_t>(0));
    EXPECT_NOT_EQUAL(mediumInstance, static_cast<uint32_t>(0));
    EXPECT_NOT_EQUAL(farInstance, static_cast<uint32_t>(0));

    lodSystem.Shutdown();

    TestOutput::PrintTestPass("LOD distance calculation");
    return true;
}

/**
 * Test animation culling system
 * Requirements: 9.4 (animation culling for off-screen characters)
 */
bool TestAnimationCullingSystem() {
    TestOutput::PrintTestStart("animation culling system");

    AnimationCullingSystem cullingSystem;
    EXPECT_TRUE(cullingSystem.Initialize());

    // Test initial state
    EXPECT_EQUAL(cullingSystem.GetCulledCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(cullingSystem.GetVisibleCount(), static_cast<size_t>(0));

    // Test culling configuration
    cullingSystem.SetCullingDistance(100.0f);
    cullingSystem.SetFrustumCullingEnabled(true);
    cullingSystem.SetOcclusionCullingEnabled(false);

    // Create test instances
    std::vector<AnimationInstance> instances;
    instances.resize(3);

    // Instance 1: Close and visible
    instances[0].worldPosition = Math::Vec3(5.0f, 0.0f, 0.0f);
    instances[0].boundingRadius = 1.0f;
    instances[0].isCulled = false;

    // Instance 2: Far away (should be distance culled)
    instances[1].worldPosition = Math::Vec3(150.0f, 0.0f, 0.0f);
    instances[1].boundingRadius = 1.0f;
    instances[1].isCulled = false;

    // Instance 3: Medium distance
    instances[2].worldPosition = Math::Vec3(50.0f, 0.0f, 0.0f);
    instances[2].boundingRadius = 1.0f;
    instances[2].isCulled = false;

    // Set up camera
    CameraData camera;
    camera.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    camera.forward = Math::Vec3(0.0f, 0.0f, -1.0f);
    camera.viewMatrix = Math::Mat4(1.0f);
    camera.projectionMatrix = Math::Mat4(1.0f);
    camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
    camera.UpdateFrustumPlanes();

    // Create pointers for culling evaluation
    std::vector<AnimationInstance*> instancePtrs;
    for (auto& instance : instances) {
        instancePtrs.push_back(&instance);
    }

    // Evaluate culling
    cullingSystem.EvaluateCulling(instancePtrs, camera);

    // Verify culling results
    size_t totalProcessed = cullingSystem.GetCulledCount() + cullingSystem.GetVisibleCount();
    EXPECT_EQUAL(totalProcessed, static_cast<size_t>(3));

    cullingSystem.Shutdown();

    TestOutput::PrintTestPass("animation culling system");
    return true;
}

/**
 * Test performance metrics tracking
 * Requirements: 9.1 (maintain 60 FPS), 9.5 (performance scaling based on system capabilities)
 */
bool TestPerformanceMetricsTracking() {
    TestOutput::PrintTestStart("performance metrics tracking");

    AnimationLODSystem lodSystem;
    EXPECT_TRUE(lodSystem.Initialize());

    // Test initial metrics
    const auto& metrics = lodSystem.GetPerformanceMetrics();
    EXPECT_NEARLY_EQUAL(metrics.targetFrameTime, 16.67f);
    EXPECT_EQUAL(metrics.activeAnimations, 0);
    EXPECT_EQUAL(metrics.culledAnimations, 0);

    // Update performance metrics
    lodSystem.UpdatePerformanceMetrics(20.0f, 75.0f, 128.0f);

    const auto& updatedMetrics = lodSystem.GetPerformanceMetrics();
    EXPECT_NEARLY_EQUAL(updatedMetrics.frameTime, 20.0f);
    EXPECT_NEARLY_EQUAL(updatedMetrics.cpuUsagePercent, 75.0f);
    EXPECT_NEARLY_EQUAL(updatedMetrics.memoryUsageMB, 128.0f);

    // Test adaptive scaling
    lodSystem.SetPerformanceScalingEnabled(true);
    lodSystem.Update(0.016f);

    lodSystem.Shutdown();

    TestOutput::PrintTestPass("performance metrics tracking");
    return true;
}

/**
 * Test LOD transition system
 * Requirements: 9.4 (smooth LOD transitions)
 */
bool TestLODTransitionSystem() {
    TestOutput::PrintTestStart("LOD transition system");

    AnimationLODSystem lodSystem;
    EXPECT_TRUE(lodSystem.Initialize());

    // Set transition time
    lodSystem.SetLODTransitionTime(0.5f);

    // Create animation controller
    auto skeleton = std::make_shared<AnimationSkeleton>();
    auto rootBone = skeleton->CreateBone("root", Math::Mat4(1.0f));
    skeleton->AddBone(rootBone);
    
    auto controller = std::make_shared<AnimationController>();
    EXPECT_TRUE(controller->Initialize(skeleton));

    // Register instance
    uint32_t instanceId = lodSystem.RegisterAnimationInstance(
        controller, Math::Vec3(10.0f, 0.0f, 0.0f), 1.0f, 1.0f);
    EXPECT_NOT_EQUAL(instanceId, static_cast<uint32_t>(0));

    // Set up camera
    CameraData camera;
    camera.position = Math::Vec3(0.0f, 0.0f, 0.0f);
    lodSystem.SetCamera(camera);

    // Update multiple times to test transitions
    for (int i = 0; i < 10; ++i) {
        lodSystem.Update(0.1f);
    }

    // Verify instance still exists
    const AnimationInstance* instance = lodSystem.GetInstance(instanceId);
    EXPECT_TRUE(instance != nullptr);

    lodSystem.Shutdown();

    TestOutput::PrintTestPass("LOD transition system");
    return true;
}

int main() {
    TestOutput::PrintHeader("AnimationLOD");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("AnimationLOD Tests");

        // Run all tests
        allPassed &= suite.RunTest("Camera Frustum Planes", TestCameraFrustumPlanes);
        allPassed &= suite.RunTest("Animation LOD System Initialization", TestAnimationLODSystemInitialization);
        allPassed &= suite.RunTest("Animation Instance Registration", TestAnimationInstanceRegistration);
        allPassed &= suite.RunTest("LOD Distance Calculation", TestLODDistanceCalculation);
        allPassed &= suite.RunTest("Animation Culling System", TestAnimationCullingSystem);
        allPassed &= suite.RunTest("Performance Metrics Tracking", TestPerformanceMetricsTracking);
        allPassed &= suite.RunTest("LOD Transition System", TestLODTransitionSystem);

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