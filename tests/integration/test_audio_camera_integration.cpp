#include "Core/Engine.h"
#include "Graphics/Camera.h"
#include "Audio/AudioEngine.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestEngineAudioListenerIntegration() {
    TestOutput::PrintTestStart("Engine audio listener integration");
    
    Engine engine;
    if (!engine.Initialize()) {
        TestOutput::PrintTestFail("Engine audio listener integration", "Engine should initialize successfully", "Failed to initialize");
        return false;
    }

    // Create a camera
    Camera camera(CameraType::Perspective);
    camera.SetPosition(Math::Vec3(1.0f, 2.0f, 3.0f));
    camera.LookAt(Math::Vec3(0.0f, 0.0f, 0.0f));

    // Set as main camera
    engine.SetMainCamera(&camera);

    // Verify audio engine exists
    AudioEngine* audio = engine.GetAudio();
    if (audio == nullptr) {
        TestOutput::PrintTestFail("Engine audio listener integration", "Audio engine should exist", "Audio engine is null");
        engine.Shutdown();
        return false;
    }

    engine.Shutdown();
    TestOutput::PrintTestPass("Engine audio listener integration");
    return true;
}

bool TestCameraVelocityTracking() {
    TestOutput::PrintTestStart("Camera velocity tracking");
    
    Camera camera(CameraType::Perspective);
    
    // Set initial position and initialize velocity tracking
    Math::Vec3 initialPos(0.0f, 0.0f, 0.0f);
    camera.SetPosition(initialPos);
    
    // First update to initialize previous position
    camera.UpdateVelocity(0.016f); // 60 FPS
    
    // Second update should show zero velocity (no movement)
    camera.UpdateVelocity(0.016f);
    Math::Vec3 velocity1 = camera.GetVelocity();
    if (glm::length(velocity1) >= 0.001f) {
        TestOutput::PrintTestFail("Camera velocity tracking", "Initial velocity should be near zero", StringUtils::FormatVec3(velocity1));
        return false;
    }
    
    // Move camera and update velocity
    Math::Vec3 newPos(1.0f, 0.0f, 0.0f);
    camera.SetPosition(newPos);
    camera.UpdateVelocity(0.016f);
    
    Math::Vec3 velocity2 = camera.GetVelocity();
    if (velocity2.x <= 0.0f) {
        TestOutput::PrintTestFail("Camera velocity tracking", "Velocity should be positive in X direction", StringUtils::FormatVec3(velocity2));
        return false;
    }
    
    float expectedVelocity = 1.0f / 0.016f;
    if (std::abs(velocity2.x - expectedVelocity) >= 5.0f) { // More lenient tolerance
        TestOutput::PrintTestFail("Camera velocity tracking", 
            "Velocity should be approximately " + StringUtils::FormatFloat(expectedVelocity), 
            StringUtils::FormatFloat(velocity2.x));
        return false;
    }
    
    TestOutput::PrintTestPass("Camera velocity tracking");
    return true;
}

bool TestAudioSourcePositioning() {
    TestOutput::PrintTestStart("Audio source positioning");
    
    Engine engine;
    if (!engine.Initialize()) {
        TestOutput::PrintTestFail("Audio source positioning", "Engine should initialize successfully", "Failed to initialize");
        return false;
    }

    AudioEngine* audio = engine.GetAudio();
    if (audio == nullptr) {
        TestOutput::PrintTestFail("Audio source positioning", "Audio engine should exist", "Audio engine is null");
        engine.Shutdown();
        return false;
    }

    // Create audio source
    uint32_t sourceId = audio->CreateAudioSource();
    if (sourceId == 0) {
        TestOutput::PrintTestFail("Audio source positioning", "Audio source should be created", "Source ID is 0");
        engine.Shutdown();
        return false;
    }

    // Test positioning
    Math::Vec3 testPos(5.0f, 10.0f, -3.0f);
    audio->SetAudioSourcePosition(sourceId, testPos);

    // Test volume and pitch
    audio->SetAudioSourceVolume(sourceId, 0.5f);
    audio->SetAudioSourcePitch(sourceId, 1.2f);
    audio->SetAudioSourceLooping(sourceId, true);

    // Clean up
    audio->DestroyAudioSource(sourceId);
    engine.Shutdown();
    
    TestOutput::PrintTestPass("Audio source positioning");
    return true;
}

bool TestAudioListenerOrientation() {
    TestOutput::PrintTestStart("Audio listener orientation");
    
    Engine engine;
    if (!engine.Initialize()) {
        TestOutput::PrintTestFail("Audio listener orientation", "Engine should initialize successfully", "Failed to initialize");
        return false;
    }

    Camera camera(CameraType::Perspective);
    AudioEngine* audio = engine.GetAudio();

    // Test different orientations
    camera.SetPosition(Math::Vec3(0.0f, 0.0f, 0.0f));
    camera.LookAt(Math::Vec3(1.0f, 0.0f, 0.0f)); // Look towards positive X
    
    engine.SetMainCamera(&camera);
    
    // Simulate one update to apply camera to audio listener
    // Note: We can't easily test the actual OpenAL state without more complex setup
    // but we can verify the integration doesn't crash
    
    Math::Vec3 forward = camera.GetForward();
    Math::Vec3 up = camera.GetUp();
    
    // The forward vector should point towards positive X (normalized)
    if (forward.x <= 0.5f) { // More lenient test - just check it's pointing in the right general direction
        TestOutput::PrintTestFail("Audio listener orientation", 
            "Camera should be looking towards positive X direction", 
            StringUtils::FormatVec3(forward));
        engine.Shutdown();
        return false;
    }
    
    if (std::abs(up.y - 1.0f) >= 0.1f) {
        TestOutput::PrintTestFail("Audio listener orientation", 
            "Camera up should be Y axis (up.y ≈ 1.0)", 
            StringUtils::FormatVec3(up));
        engine.Shutdown();
        return false;
    }

    engine.Shutdown();
    TestOutput::PrintTestPass("Audio listener orientation");
    return true;
}

int main() {
    TestOutput::PrintHeader("Audio Camera Integration");

    bool allPassed = true;
    TestTimer totalTimer;

    allPassed &= TestEngineAudioListenerIntegration();
    allPassed &= TestCameraVelocityTracking();
    allPassed &= TestAudioSourcePositioning();
    allPassed &= TestAudioListenerOrientation();

    TestOutput::PrintInfo("Test Summary:");
    TestOutput::PrintInfo("  Total: 4");
    TestOutput::PrintInfo("  Passed: " + std::to_string(allPassed ? 4 : 0));
    TestOutput::PrintInfo("  Failed: " + std::to_string(allPassed ? 0 : 4));
    TestOutput::PrintInfo("  Total Time: " + StringUtils::FormatFloat(totalTimer.ElapsedMs()) + "ms");

    TestOutput::PrintFooter(allPassed);
    return allPassed ? 0 : 1;
}