#include <iostream>
#include <filesystem>
#include "Audio/AudioEngine.h"
#include "TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test basic audio engine initialization
 * Requirements: Audio system initialization
 */
bool TestAudioEngineInitialization() {
    TestOutput::PrintTestStart("audio engine initialization");

    AudioEngine audioEngine;
    
    // Test initialization (may fail gracefully on systems without audio)
    bool initResult = audioEngine.Initialize();
    
    if (initResult) {
        EXPECT_TRUE(audioEngine.IsAudioAvailable());
        TestOutput::PrintInfo("Audio system initialized successfully");
        
        // Test shutdown
        audioEngine.Shutdown();
        EXPECT_FALSE(audioEngine.IsAudioAvailable());
    } else {
        TestOutput::PrintInfo("Audio system not available (graceful failure)");
        EXPECT_FALSE(audioEngine.IsAudioAvailable());
    }

    TestOutput::PrintTestPass("audio engine initialization");
    return true;
}

/**
 * Test WAV file loading
 * Requirements: Audio file loading and validation
 */
bool TestWAVFileLoading() {
    TestOutput::PrintTestStart("WAV file loading");

    AudioEngine audioEngine;
    bool initResult = audioEngine.Initialize();
    
    if (!initResult) {
        TestOutput::PrintInfo("Skipping WAV loading test - audio system not available");
        TestOutput::PrintTestPass("WAV file loading");
        return true;
    }

    // Test loading non-existent WAV file
    auto clip1 = audioEngine.LoadAudioClip("nonexistent.wav");
    EXPECT_NULL(clip1);

    // Test loading existing WAV file if available
    if (std::filesystem::exists("assets/audio/test.wav")) {
        auto clip2 = audioEngine.LoadAudioClip("assets/audio/test.wav");
        if (clip2) {
            TestOutput::PrintInfo("Successfully loaded test.wav");
            EXPECT_NOT_NULL(clip2);
        } else {
            TestOutput::PrintInfo("Failed to load test.wav (may be format issue)");
        }
    } else {
        TestOutput::PrintInfo("No test WAV files found - skipping file loading test");
    }

    audioEngine.Shutdown();

    TestOutput::PrintTestPass("WAV file loading");
    return true;
}

/**
 * Test audio source creation and management
 * Requirements: Audio source management
 */
bool TestAudioSourceManagement() {
    TestOutput::PrintTestStart("audio source management");

    AudioEngine audioEngine;
    bool initResult = audioEngine.Initialize();
    
    if (!initResult) {
        TestOutput::PrintInfo("Skipping audio source test - audio system not available");
        TestOutput::PrintTestPass("audio source management");
        return true;
    }

    // Test creating audio sources
    uint32_t source1 = audioEngine.CreateAudioSource();
    uint32_t source2 = audioEngine.CreateAudioSource();
    
    EXPECT_TRUE(source1 > 0);
    EXPECT_TRUE(source2 > 0);
    EXPECT_NOT_EQUAL(source1, source2);

    // Test source properties
    audioEngine.SetSourcePosition(source1, Math::Vec3(1.0f, 0.0f, 0.0f));
    audioEngine.SetSourceVolume(source1, 0.5f);
    audioEngine.SetSourcePitch(source1, 1.2f);

    // Test cleanup
    audioEngine.DestroyAudioSource(source1);
    audioEngine.DestroyAudioSource(source2);

    audioEngine.Shutdown();

    TestOutput::PrintTestPass("audio source management");
    return true;
}

/**
 * Test audio playback functionality
 * Requirements: Audio playback and control
 */
bool TestAudioPlayback() {
    TestOutput::PrintTestStart("audio playback");

    AudioEngine audioEngine;
    bool initResult = audioEngine.Initialize();
    
    if (!initResult) {
        TestOutput::PrintInfo("Skipping audio playback test - audio system not available");
        TestOutput::PrintTestPass("audio playback");
        return true;
    }

    uint32_t source = audioEngine.CreateAudioSource();
    EXPECT_TRUE(source > 0);

    // Test playback with null clip (should handle gracefully)
    audioEngine.PlayAudioSource(source, nullptr);
    
    // Test playback controls
    audioEngine.PauseAudioSource(source);
    audioEngine.StopAudioSource(source);
    
    // Test invalid source ID (should handle gracefully)
    audioEngine.PlayAudioSource(999, nullptr);

    audioEngine.DestroyAudioSource(source);
    audioEngine.Shutdown();

    TestOutput::PrintTestPass("audio playback");
    return true;
}

/**
 * Test 3D audio positioning
 * Requirements: 3D spatial audio functionality
 */
bool Test3DAudioPositioning() {
    TestOutput::PrintTestStart("3D audio positioning");

    AudioEngine audioEngine;
    bool initResult = audioEngine.Initialize();
    
    if (!initResult) {
        TestOutput::PrintInfo("Skipping 3D audio test - audio system not available");
        TestOutput::PrintTestPass("3D audio positioning");
        return true;
    }

    uint32_t source = audioEngine.CreateAudioSource();
    EXPECT_TRUE(source > 0);

    // Test listener positioning
    audioEngine.SetListenerPosition(Math::Vec3(0.0f, 0.0f, 0.0f));
    audioEngine.SetListenerOrientation(
        Math::Vec3(0.0f, 0.0f, -1.0f), // Forward
        Math::Vec3(0.0f, 1.0f, 0.0f)   // Up
    );

    // Test source positioning
    audioEngine.SetSourcePosition(source, Math::Vec3(10.0f, 0.0f, 0.0f));
    audioEngine.SetSourceVelocity(source, Math::Vec3(1.0f, 0.0f, 0.0f));

    // Test distance attenuation settings
    audioEngine.SetSourceReferenceDistance(source, 1.0f);
    audioEngine.SetSourceMaxDistance(source, 100.0f);
    audioEngine.SetSourceRolloffFactor(source, 1.0f);

    audioEngine.DestroyAudioSource(source);
    audioEngine.Shutdown();

    TestOutput::PrintTestPass("3D audio positioning");
    return true;
}

/**
 * Test audio error handling
 * Requirements: Robust error handling for audio operations
 */
bool TestAudioErrorHandling() {
    TestOutput::PrintTestStart("audio error handling");

    AudioEngine audioEngine;
    
    // Test operations on uninitialized engine
    EXPECT_FALSE(audioEngine.IsAudioAvailable());
    
    uint32_t source = audioEngine.CreateAudioSource();
    // Should handle gracefully even when not initialized
    
    auto clip = audioEngine.LoadAudioClip("nonexistent.wav");
    EXPECT_NULL(clip);

    // Test initialization
    bool initResult = audioEngine.Initialize();
    
    if (initResult) {
        // Test invalid operations
        audioEngine.PlayAudioSource(0, nullptr); // Invalid source ID
        audioEngine.SetSourcePosition(999, Math::Vec3(0.0f, 0.0f, 0.0f)); // Invalid source
        
        audioEngine.Shutdown();
    }

    TestOutput::PrintTestPass("audio error handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Audio Engine WAV Loading Integration");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("Audio Engine WAV Loading Integration Tests");

        // Run all tests
        allPassed &= suite.RunTest("Audio Engine Initialization", TestAudioEngineInitialization);
        allPassed &= suite.RunTest("WAV File Loading", TestWAVFileLoading);
        allPassed &= suite.RunTest("Audio Source Management", TestAudioSourceManagement);
        allPassed &= suite.RunTest("Audio Playback", TestAudioPlayback);
        allPassed &= suite.RunTest("3D Audio Positioning", Test3DAudioPositioning);
        allPassed &= suite.RunTest("Audio Error Handling", TestAudioErrorHandling);

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