#include "Audio/AudioEngine.h"
#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <fstream>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Helper function to create a simple WAV file for testing
bool CreateTestWAVFile(const std::string& filename, float durationSeconds = 0.1f) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    uint32_t sampleRate = 44100;
    uint16_t channels = 2;
    uint16_t bitsPerSample = 16;
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

    // Write simple sine wave data
    for (uint32_t i = 0; i < samplesPerChannel; ++i) {
        for (uint16_t ch = 0; ch < channels; ++ch) {
            int16_t sample = static_cast<int16_t>(16383.0 * sin(2.0 * 3.14159 * 440.0 * i / sampleRate));
            file.write(reinterpret_cast<const char*>(&sample), 2);
        }
    }

    file.close();
    return true;
}

bool TestBasic3DPositioning() {
    TestOutput::PrintTestStart("Basic 3D positioning");

    AudioEngine engine;
    engine.Initialize();

    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Test setting various 3D positions
    Math::Vec3 positions[] = {
        Math::Vec3(0.0f, 0.0f, 0.0f),    // Origin
        Math::Vec3(1.0f, 0.0f, 0.0f),    // Right
        Math::Vec3(-1.0f, 0.0f, 0.0f),   // Left
        Math::Vec3(0.0f, 1.0f, 0.0f),    // Up
        Math::Vec3(0.0f, -1.0f, 0.0f),   // Down
        Math::Vec3(0.0f, 0.0f, 1.0f),    // Forward
        Math::Vec3(0.0f, 0.0f, -1.0f),   // Backward
        Math::Vec3(10.0f, 5.0f, -3.0f),  // Arbitrary position
        Math::Vec3(-100.0f, 50.0f, 200.0f) // Far position
    };

    for (const auto& pos : positions) {
        engine.SetAudioSourcePosition(sourceId, pos);
        // Position setting should not crash and should be accepted
        // We can't easily verify the actual OpenAL state without OpenAL context access
    }

    engine.DestroyAudioSource(sourceId);
    engine.Shutdown();

    TestOutput::PrintTestPass("Basic 3D positioning");
    return true;
}

bool TestListenerPositioning() {
    TestOutput::PrintTestStart("Listener positioning");

    AudioEngine engine;
    engine.Initialize();

    // Test various listener positions
    Math::Vec3 positions[] = {
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(5.0f, 2.0f, -1.0f),
        Math::Vec3(-10.0f, 0.0f, 5.0f)
    };

    for (const auto& pos : positions) {
        engine.SetListenerPosition(pos);
    }

    // Test various listener orientations
    struct OrientationTest {
        Math::Vec3 forward;
        Math::Vec3 up;
    };

    OrientationTest orientations[] = {
        { Math::Vec3(0.0f, 0.0f, -1.0f), Math::Vec3(0.0f, 1.0f, 0.0f) }, // Default
        { Math::Vec3(1.0f, 0.0f, 0.0f), Math::Vec3(0.0f, 1.0f, 0.0f) },  // Looking right
        { Math::Vec3(-1.0f, 0.0f, 0.0f), Math::Vec3(0.0f, 1.0f, 0.0f) }, // Looking left
        { Math::Vec3(0.0f, 1.0f, 0.0f), Math::Vec3(0.0f, 0.0f, 1.0f) },  // Looking up
        { Math::Vec3(0.0f, -1.0f, 0.0f), Math::Vec3(0.0f, 0.0f, -1.0f) } // Looking down
    };

    for (const auto& orient : orientations) {
        engine.SetListenerOrientation(orient.forward, orient.up);
    }

    // Test listener velocity
    Math::Vec3 velocities[] = {
        Math::Vec3(0.0f, 0.0f, 0.0f),   // Stationary
        Math::Vec3(1.0f, 0.0f, 0.0f),   // Moving right
        Math::Vec3(0.0f, 0.0f, -5.0f),  // Moving forward
        Math::Vec3(-2.0f, 1.0f, 3.0f)   // Complex movement
    };

    for (const auto& vel : velocities) {
        engine.SetListenerVelocity(vel);
    }

    engine.Shutdown();

    TestOutput::PrintTestPass("Listener positioning");
    return true;
}

bool TestAudioSourceProperties() {
    TestOutput::PrintTestStart("Audio source properties");

    AudioEngine engine;
    engine.Initialize();

    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Test volume settings
    float volumes[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
    for (float volume : volumes) {
        engine.SetAudioSourceVolume(sourceId, volume);
    }

    // Test pitch settings
    float pitches[] = { 0.1f, 0.5f, 1.0f, 1.5f, 2.0f, 4.0f };
    for (float pitch : pitches) {
        engine.SetAudioSourcePitch(sourceId, pitch);
    }

    // Test looping settings
    engine.SetAudioSourceLooping(sourceId, false);
    engine.SetAudioSourceLooping(sourceId, true);
    engine.SetAudioSourceLooping(sourceId, false);

    engine.DestroyAudioSource(sourceId);
    engine.Shutdown();

    TestOutput::PrintTestPass("Audio source properties");
    return true;
}

bool TestMultipleSourcePositioning() {
    TestOutput::PrintTestStart("Multiple source positioning");

    AudioEngine engine;
    engine.Initialize();

    // Create multiple audio sources
    const int numSources = 5;
    uint32_t sourceIds[numSources];
    
    for (int i = 0; i < numSources; ++i) {
        sourceIds[i] = engine.CreateAudioSource();
        EXPECT_TRUE(sourceIds[i] != 0);
    }

    // Position sources in a circle around the listener
    const float radius = 5.0f;
    for (int i = 0; i < numSources; ++i) {
        float angle = (2.0f * 3.14159f * i) / numSources;
        Math::Vec3 position(
            radius * cos(angle),
            0.0f,
            radius * sin(angle)
        );
        engine.SetAudioSourcePosition(sourceIds[i], position);
        
        // Set different properties for each source
        engine.SetAudioSourceVolume(sourceIds[i], 0.2f * (i + 1));
        engine.SetAudioSourcePitch(sourceIds[i], 0.8f + 0.1f * i);
    }

    // Move listener around
    Math::Vec3 listenerPositions[] = {
        Math::Vec3(0.0f, 0.0f, 0.0f),
        Math::Vec3(1.0f, 0.0f, 0.0f),
        Math::Vec3(0.0f, 1.0f, 0.0f),
        Math::Vec3(2.0f, 2.0f, 2.0f)
    };

    for (const auto& pos : listenerPositions) {
        engine.SetListenerPosition(pos);
        
        // Update all source positions relative to new listener position
        for (int i = 0; i < numSources; ++i) {
            float angle = (2.0f * 3.14159f * i) / numSources;
            Math::Vec3 sourcePos = pos + Math::Vec3(
                radius * cos(angle),
                0.0f,
                radius * sin(angle)
            );
            engine.SetAudioSourcePosition(sourceIds[i], sourcePos);
        }
    }

    // Clean up
    for (int i = 0; i < numSources; ++i) {
        engine.DestroyAudioSource(sourceIds[i]);
    }
    engine.Shutdown();

    TestOutput::PrintTestPass("Multiple source positioning");
    return true;
}

bool TestDistanceAttenuation() {
    TestOutput::PrintTestStart("Distance attenuation simulation");

    AudioEngine engine;
    engine.Initialize();

    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Test sources at various distances from listener
    float distances[] = { 0.1f, 1.0f, 5.0f, 10.0f, 50.0f, 100.0f, 1000.0f };
    
    for (float distance : distances) {
        Math::Vec3 position(distance, 0.0f, 0.0f);
        engine.SetAudioSourcePosition(sourceId, position);
        
        // In a real implementation, we might expect volume to decrease with distance
        // For now, we just verify the position can be set without crashing
    }

    engine.DestroyAudioSource(sourceId);
    engine.Shutdown();

    TestOutput::PrintTestPass("Distance attenuation simulation");
    return true;
}

bool TestDopplerEffect() {
    TestOutput::PrintTestStart("Doppler effect simulation");

    AudioEngine engine;
    engine.Initialize();

    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Simulate moving source (Doppler effect)
    const int numSteps = 10;
    const float totalTime = 2.0f; // 2 seconds
    const float deltaTime = totalTime / numSteps;
    
    // Source moving from left to right past stationary listener
    for (int i = 0; i < numSteps; ++i) {
        float t = (float)i / (numSteps - 1);
        float x = -10.0f + 20.0f * t; // Move from x=-10 to x=+10
        Math::Vec3 position(x, 0.0f, 0.0f);
        
        // Calculate velocity for Doppler effect
        Math::Vec3 velocity(20.0f / totalTime, 0.0f, 0.0f); // Constant velocity
        
        engine.SetAudioSourcePosition(sourceId, position);
        // Note: AudioEngine doesn't currently expose SetVelocity, but position changes
        // could be used to calculate velocity internally
        
        // Simulate time passing
        engine.Update(deltaTime);
    }

    // Test listener movement (also affects Doppler)
    Math::Vec3 listenerVelocity(5.0f, 0.0f, 0.0f);
    engine.SetListenerVelocity(listenerVelocity);
    
    for (int i = 0; i < numSteps; ++i) {
        float t = (float)i / (numSteps - 1);
        Math::Vec3 listenerPos(5.0f * t, 0.0f, 0.0f);
        engine.SetListenerPosition(listenerPos);
        engine.Update(deltaTime);
    }

    engine.DestroyAudioSource(sourceId);
    engine.Shutdown();

    TestOutput::PrintTestPass("Doppler effect simulation");
    return true;
}

bool Test3DAudioWithPlayback() {
    TestOutput::PrintTestStart("3D audio with playback");

    AudioEngine engine;
    engine.Initialize();

    // Create test audio file
    const std::string testFile = "test_3d_audio.wav";
    if (!CreateTestWAVFile(testFile)) {
        TestOutput::PrintTestFail("3D audio with playback - Failed to create test file");
        return false;
    }

    auto clip = engine.LoadAudioClip(testFile);
    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Test 3D positioning with actual audio playback
    Math::Vec3 testPositions[] = {
        Math::Vec3(0.0f, 0.0f, 0.0f),   // At listener
        Math::Vec3(2.0f, 0.0f, 0.0f),   // To the right
        Math::Vec3(-2.0f, 0.0f, 0.0f),  // To the left
        Math::Vec3(0.0f, 0.0f, -2.0f),  // Behind listener
        Math::Vec3(0.0f, 2.0f, 0.0f)    // Above listener
    };

    for (const auto& pos : testPositions) {
        engine.SetAudioSourcePosition(sourceId, pos);
        
        if (clip) {
            engine.PlayAudioSource(sourceId, clip);
            
            // Let it play briefly (simulated)
            engine.Update(0.05f); // 50ms
            
            engine.StopAudioSource(sourceId);
        }
    }

    // Test with different listener orientations
    Math::Vec3 forward(1.0f, 0.0f, 0.0f);
    Math::Vec3 up(0.0f, 1.0f, 0.0f);
    engine.SetListenerOrientation(forward, up);
    
    engine.SetAudioSourcePosition(sourceId, Math::Vec3(0.0f, 0.0f, -1.0f));
    if (clip) {
        engine.PlayAudioSource(sourceId, clip);
        engine.Update(0.05f);
        engine.StopAudioSource(sourceId);
    }

    // Clean up
    engine.DestroyAudioSource(sourceId);
    std::remove(testFile.c_str());
    engine.Shutdown();

    TestOutput::PrintTestPass("3D audio with playback");
    return true;
}

bool TestExtremePositions() {
    TestOutput::PrintTestStart("Extreme position handling");

    AudioEngine engine;
    engine.Initialize();

    uint32_t sourceId = engine.CreateAudioSource();
    EXPECT_TRUE(sourceId != 0);

    // Test extreme positions (should not crash)
    Math::Vec3 extremePositions[] = {
        Math::Vec3(1e6f, 0.0f, 0.0f),      // Very far
        Math::Vec3(-1e6f, 0.0f, 0.0f),     // Very far negative
        Math::Vec3(0.0f, 1e6f, 0.0f),      // Very high
        Math::Vec3(0.0f, -1e6f, 0.0f),     // Very low
        Math::Vec3(1e-6f, 1e-6f, 1e-6f),   // Very close to origin
        Math::Vec3(std::numeric_limits<float>::max(), 0.0f, 0.0f), // Maximum float
        Math::Vec3(std::numeric_limits<float>::lowest(), 0.0f, 0.0f) // Minimum float
    };

    for (const auto& pos : extremePositions) {
        engine.SetAudioSourcePosition(sourceId, pos);
        // Should not crash or cause undefined behavior
    }

    // Test extreme listener positions
    for (const auto& pos : extremePositions) {
        engine.SetListenerPosition(pos);
    }

    // Test extreme velocities
    Math::Vec3 extremeVelocities[] = {
        Math::Vec3(1000.0f, 0.0f, 0.0f),   // Very fast
        Math::Vec3(-1000.0f, 0.0f, 0.0f),  // Very fast negative
        Math::Vec3(0.0f, 0.0f, 1e6f),      // Extremely fast
        Math::Vec3(1e-6f, 1e-6f, 1e-6f)    // Very slow
    };

    for (const auto& vel : extremeVelocities) {
        engine.SetListenerVelocity(vel);
    }

    engine.DestroyAudioSource(sourceId);
    engine.Shutdown();

    TestOutput::PrintTestPass("Extreme position handling");
    return true;
}

int main() {
    TestOutput::PrintHeader("Audio 3D Positioning Tests");
    Logger::GetInstance().Initialize();

    TestSuite suite("Audio 3D Positioning Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("Basic 3D Positioning", TestBasic3DPositioning);
    allPassed &= suite.RunTest("Listener Positioning", TestListenerPositioning);
    allPassed &= suite.RunTest("Audio Source Properties", TestAudioSourceProperties);
    allPassed &= suite.RunTest("Multiple Source Positioning", TestMultipleSourcePositioning);
    allPassed &= suite.RunTest("Distance Attenuation", TestDistanceAttenuation);
    allPassed &= suite.RunTest("Doppler Effect", TestDopplerEffect);
    allPassed &= suite.RunTest("3D Audio with Playback", Test3DAudioWithPlayback);
    allPassed &= suite.RunTest("Extreme Position Handling", TestExtremePositions);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}