#include "Audio/AudioEngine.h"
#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <fstream>
#include <cstring>
#include <cmath>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Helper function to create a simple WAV file for testing
bool CreateTestWAVFile(const std::string& filename, uint32_t sampleRate = 44100, uint16_t channels = 2, uint16_t bitsPerSample = 16, float durationSeconds = 0.1f) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    uint32_t samplesPerChannel = static_cast<uint32_t>(sampleRate * durationSeconds);
    uint32_t dataSize = samplesPerChannel * channels * (bitsPerSample / 8);

    // WAV Header
    file.write("RIFF", 4);
    uint32_t fileSize = 36 + dataSize;
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("WAVE", 4);

    // Format chunk
    file.write("fmt ", 4);
    uint32_t fmtChunkSize = 16;
    file.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
    uint16_t audioFormat = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&channels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    uint16_t blockAlign = channels * bitsPerSample / 8;
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // Data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Write simple sine wave data (440Hz tone)
    for (uint32_t i = 0; i < samplesPerChannel; ++i) {
        for (uint16_t ch = 0; ch < channels; ++ch) {
            if (bitsPerSample == 16) {
                int16_t sample = static_cast<int16_t>(16383.0 * sin(2.0 * 3.14159 * 440.0 * i / sampleRate));
                file.write(reinterpret_cast<const char*>(&sample), 2);
            } else if (bitsPerSample == 8) {
                uint8_t sample = static_cast<uint8_t>(127 + 63 * sin(2.0 * 3.14159 * 440.0 * i / sampleRate));
                file.write(reinterpret_cast<const char*>(&sample), 1);
            }
        }
    }

    file.close();
    return true;
}

bool TestAudioEngineConstruction() {
    TestOutput::PrintTestStart("AudioEngine construction");

    AudioEngine engine;
    
    // Engine should be constructed successfully
    EXPECT_FALSE(engine.IsAudioAvailable()); // Not initialized yet
    EXPECT_FALSE(engine.IsOpenALInitialized()); // Not initialized yet

    TestOutput::PrintTestPass("AudioEngine construction");
    return true;
}

bool TestAudioEngineInitialization() {
    TestOutput::PrintTestStart("AudioEngine initialization and cleanup");

    AudioEngine engine;
    
    // Test initialization
    bool initResult = engine.Initialize();
    EXPECT_TRUE(initResult); // Should always return true (graceful fallback)
    
    // Check if OpenAL is available (may fail on systems without audio)
    if (engine.IsOpenALInitialized()) {
        TestOutput::PrintInfo("OpenAL initialized successfully");
        EXPECT_TRUE(engine.IsAudioAvailable());
    } else {
        TestOutput::PrintInfo("OpenAL not available, running in silent mode");
        // Engine should still work in silent mode
        EXPECT_FALSE(engine.IsAudioAvailable());
    }
    
    // Test shutdown (should not crash)
    engine.Shutdown();
    EXPECT_FALSE(engine.IsAudioAvailable());
    EXPECT_FALSE(engine.IsOpenALInitialized());

    TestOutput::PrintTestPass("AudioEngine initialization and cleanup");
    return true;
}

bool TestAudioClipLoading() {
    TestOutput::PrintTestStart("Audio clip loading");

    AudioEngine engine;
    engine.Initialize();

    // Create test WAV file
    const std::string testFile = "test_audio_clip.wav";
    if (!CreateTestWAVFile(testFile)) {
        TestOutput::PrintTestFail("Audio clip loading - Failed to create test file");
        return false;
    }

    // Test loading valid audio clip
    auto clip = engine.LoadAudioClip(testFile);
    if (engine.IsAudioAvailable()) {
        EXPECT_NOT_NULL(clip);
        if (clip) {
            EXPECT_STRING_EQUAL(clip->path, testFile);
            EXPECT_EQUAL(clip->sampleRate, 44100);
            EXPECT_EQUAL(clip->channels, 2);
            EXPECT_TRUE(clip->duration > 0.05f && clip->duration < 0.15f); // ~0.1 seconds
        }
    } else {
        // In silent mode, loading may still work but won't have OpenAL buffer
        TestOutput::PrintInfo("Testing in silent mode - clip loading behavior may vary");
    }

    // Test loading non-existent file
    auto nullClip = engine.LoadAudioClip("nonexistent.wav");
    EXPECT_NULL(nullClip);

    // Clean up
    std::remove(testFile.c_str());
    engine.Shutdown();

    TestOutput::PrintTestPass("Audio clip loading");
    return true;
}

bool TestAudioSourceManagement() {
    TestOutput::PrintTestStart("Audio source management");

    AudioEngine engine;
    engine.Initialize();

    // Test creating audio sources
    uint32_t source1 = engine.CreateAudioSource();
    uint32_t source2 = engine.CreateAudioSource();
    
    EXPECT_TRUE(source1 != 0);
    EXPECT_TRUE(source2 != 0);
    EXPECT_TRUE(source1 != source2);

    // Test destroying audio sources (should not crash)
    engine.DestroyAudioSource(source1);
    engine.DestroyAudioSource(source2);
    
    // Test destroying non-existent source (should not crash)
    engine.DestroyAudioSource(999999);

    engine.Shutdown();

    TestOutput::PrintTestPass("Audio source management");
    return true;
}

bool TestAudioSourcePlayback() {
    TestOutput::PrintTestStart("Audio source playback");

    AudioEngine engine;
    engine.Initialize();

    // Create test audio file
    const std::string testFile = "test_playback.wav";
    if (!CreateTestWAVFile(testFile)) {
        TestOutput::PrintTestFail("Audio source playback - Failed to create test file");
        return false;
    }

    auto clip = engine.LoadAudioClip(testFile);
    uint32_t sourceId = engine.CreateAudioSource();
    
    EXPECT_TRUE(sourceId != 0);

    // Test playback operations (should not crash regardless of audio availability)
    engine.PlayAudioSource(sourceId, clip);
    engine.PauseAudioSource(sourceId);
    engine.StopAudioSource(sourceId);

    // Test with null clip (should not crash)
    engine.PlayAudioSource(sourceId, nullptr);

    // Test with invalid source ID (should not crash)
    engine.PlayAudioSource(999999, clip);

    // Clean up
    engine.DestroyAudioSource(sourceId);
    std::remove(testFile.c_str());
    engine.Shutdown();

    TestOutput::PrintTestPass("Audio source playback");
    return true;
}

bool TestAudioSource3DPositioning() {
    TestOutput::PrintTestStart("Audio source 3D positioning");

    AudioEngine engine;
    engine.Initialize();

    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Test setting 3D position (should not crash)
    Math::Vec3 position1(1.0f, 2.0f, 3.0f);
    Math::Vec3 position2(-5.0f, 0.0f, 10.0f);
    
    engine.SetAudioSourcePosition(sourceId, position1);
    engine.SetAudioSourcePosition(sourceId, position2);

    // Test with invalid source ID (should not crash)
    engine.SetAudioSourcePosition(999999, position1);

    // Test setting volume, pitch, and looping
    engine.SetAudioSourceVolume(sourceId, 0.5f);
    engine.SetAudioSourceVolume(sourceId, 0.0f);
    engine.SetAudioSourceVolume(sourceId, 1.0f);
    
    engine.SetAudioSourcePitch(sourceId, 0.5f);
    engine.SetAudioSourcePitch(sourceId, 2.0f);
    engine.SetAudioSourcePitch(sourceId, 1.0f);
    
    engine.SetAudioSourceLooping(sourceId, true);
    engine.SetAudioSourceLooping(sourceId, false);

    // Clean up
    engine.DestroyAudioSource(sourceId);
    engine.Shutdown();

    TestOutput::PrintTestPass("Audio source 3D positioning");
    return true;
}

bool TestAudioListenerManagement() {
    TestOutput::PrintTestStart("Audio listener management");

    AudioEngine engine;
    engine.Initialize();

    // Test setting listener properties (should not crash)
    Math::Vec3 position(0.0f, 1.0f, 0.0f);
    Math::Vec3 forward(0.0f, 0.0f, -1.0f);
    Math::Vec3 up(0.0f, 1.0f, 0.0f);
    Math::Vec3 velocity(1.0f, 0.0f, 0.0f);

    engine.SetListenerPosition(position);
    engine.SetListenerOrientation(forward, up);
    engine.SetListenerVelocity(velocity);

    // Test with different values
    engine.SetListenerPosition(Math::Vec3(10.0f, -5.0f, 20.0f));
    engine.SetListenerOrientation(Math::Vec3(1.0f, 0.0f, 0.0f), Math::Vec3(0.0f, 1.0f, 0.0f));
    engine.SetListenerVelocity(Math::Vec3(0.0f, 0.0f, 0.0f));

    engine.Shutdown();

    TestOutput::PrintTestPass("Audio listener management");
    return true;
}

bool TestAudioVolumeControls() {
    TestOutput::PrintTestStart("Audio volume controls");

    AudioEngine engine;
    engine.Initialize();

    // Test setting master volume
    engine.SetMasterVolume(0.5f);
    engine.SetMasterVolume(0.0f);
    engine.SetMasterVolume(1.0f);
    engine.SetMasterVolume(2.0f); // Should be clamped to 1.0f
    engine.SetMasterVolume(-0.5f); // Should be clamped to 0.0f

    // Test setting music volume
    engine.SetMusicVolume(0.7f);
    engine.SetMusicVolume(0.0f);
    engine.SetMusicVolume(1.0f);

    // Test setting SFX volume
    engine.SetSFXVolume(0.8f);
    engine.SetSFXVolume(0.0f);
    engine.SetSFXVolume(1.0f);

    engine.Shutdown();

    TestOutput::PrintTestPass("Audio volume controls");
    return true;
}

bool TestAudioEngineUpdate() {
    TestOutput::PrintTestStart("Audio engine update");

    AudioEngine engine;
    engine.Initialize();

    // Test update with various delta times (should not crash)
    engine.Update(0.016f); // ~60 FPS
    engine.Update(0.033f); // ~30 FPS
    engine.Update(0.0f);   // Zero delta
    engine.Update(1.0f);   // Large delta

    engine.Shutdown();

    TestOutput::PrintTestPass("Audio engine update");
    return true;
}

bool TestAudioErrorRecovery() {
    TestOutput::PrintTestStart("Audio error recovery");

    AudioEngine engine;
    engine.Initialize();

    // Test recovery attempt (should not crash)
    bool recoveryResult = engine.AttemptAudioRecovery();
    
    // Recovery behavior depends on initial state
    if (engine.IsOpenALInitialized()) {
        TestOutput::PrintInfo("Audio recovery tested with OpenAL available");
    } else {
        TestOutput::PrintInfo("Audio recovery tested in silent mode");
    }

    // Test device disconnection handling (should not crash)
    engine.HandleDeviceDisconnection();

    engine.Shutdown();

    TestOutput::PrintTestPass("Audio error recovery");
    return true;
}

#ifdef GAMEENGINE_HAS_OPENAL
bool TestOpenALErrorChecking() {
    TestOutput::PrintTestStart("OpenAL error checking");

    // Test error string conversion
    std::string noError = AudioEngine::GetOpenALErrorString(AL_NO_ERROR);
    EXPECT_STRING_EQUAL(noError, "No error");

    std::string invalidName = AudioEngine::GetOpenALErrorString(AL_INVALID_NAME);
    EXPECT_STRING_EQUAL(invalidName, "Invalid name parameter");

    std::string invalidEnum = AudioEngine::GetOpenALErrorString(AL_INVALID_ENUM);
    EXPECT_STRING_EQUAL(invalidEnum, "Invalid enum parameter");

    std::string invalidValue = AudioEngine::GetOpenALErrorString(AL_INVALID_VALUE);
    EXPECT_STRING_EQUAL(invalidValue, "Invalid value parameter");

    std::string invalidOp = AudioEngine::GetOpenALErrorString(AL_INVALID_OPERATION);
    EXPECT_STRING_EQUAL(invalidOp, "Invalid operation");

    std::string outOfMemory = AudioEngine::GetOpenALErrorString(AL_OUT_OF_MEMORY);
    EXPECT_STRING_EQUAL(outOfMemory, "Out of memory");

    // Test unknown error
    std::string unknownError = AudioEngine::GetOpenALErrorString(0x9999);
    EXPECT_TRUE(unknownError.find("Unknown OpenAL error") != std::string::npos);

    TestOutput::PrintTestPass("OpenAL error checking");
    return true;
}
#endif

bool TestAudioPerformanceOptimizations() {
    TestOutput::PrintTestStart("Audio performance optimizations");

    AudioEngine engine;
    engine.Initialize();

    // Test enabling/disabling performance features (should not crash)
    engine.EnableBufferPooling(true);
    engine.EnableBufferPooling(false);
    engine.EnableBufferPooling(true);

    engine.EnableSourcePooling(true);
    engine.EnableSourcePooling(false);
    engine.EnableSourcePooling(true);

    engine.EnableOptimized3DAudio(true);
    engine.EnableOptimized3DAudio(false);
    engine.EnableOptimized3DAudio(true);

    // Test setting pool sizes (should not crash)
    engine.SetBufferPoolSize(10);
    engine.SetBufferPoolSize(100);
    engine.SetSourcePoolSize(5, 20);
    engine.SetSourcePoolSize(1, 50);

    // Test hot audio marking (should not crash)
    engine.MarkAudioAsHot("test.wav");
    engine.UnmarkAudioAsHot("test.wav");

    // Test performance statistics (should not crash)
    float hitRatio = engine.GetBufferPoolHitRatio();
    float utilization = engine.GetSourcePoolUtilization();
    size_t memoryUsage = engine.GetBufferPoolMemoryUsage();
    int calculations = engine.GetAudio3DCalculationsPerSecond();

    // Values should be reasonable (not negative, not extremely large)
    EXPECT_TRUE(hitRatio >= 0.0f && hitRatio <= 1.0f);
    EXPECT_TRUE(utilization >= 0.0f && utilization <= 1.0f);
    EXPECT_TRUE(memoryUsage < 1000000000); // Less than 1GB
    EXPECT_TRUE(calculations >= 0 && calculations < 1000000); // Reasonable range

    engine.Shutdown();

    TestOutput::PrintTestPass("Audio performance optimizations");
    return true;
}

int main() {
    TestOutput::PrintHeader("Audio Engine Unit Tests");
    Logger::GetInstance().Initialize();

    TestSuite suite("Audio Engine Unit Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("Construction", TestAudioEngineConstruction);
    allPassed &= suite.RunTest("Initialization", TestAudioEngineInitialization);
    allPassed &= suite.RunTest("Audio Clip Loading", TestAudioClipLoading);
    allPassed &= suite.RunTest("Audio Source Management", TestAudioSourceManagement);
    allPassed &= suite.RunTest("Audio Source Playback", TestAudioSourcePlayback);
    allPassed &= suite.RunTest("3D Positioning", TestAudioSource3DPositioning);
    allPassed &= suite.RunTest("Listener Management", TestAudioListenerManagement);
    allPassed &= suite.RunTest("Volume Controls", TestAudioVolumeControls);
    allPassed &= suite.RunTest("Engine Update", TestAudioEngineUpdate);
    allPassed &= suite.RunTest("Error Recovery", TestAudioErrorRecovery);
    allPassed &= suite.RunTest("Performance Optimizations", TestAudioPerformanceOptimizations);

#ifdef GAMEENGINE_HAS_OPENAL
    allPassed &= suite.RunTest("OpenAL Error Checking", TestOpenALErrorChecking);
#endif

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}