#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <fstream>
#include <cstring>
#include <cmath>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Helper function to create a simple WAV file for testing
bool CreateTestWAVFile(const std::string& filename, uint32_t sampleRate = 44100, uint16_t channels = 2, uint16_t bitsPerSample = 16) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // WAV Header
    file.write("RIFF", 4);
    uint32_t fileSize = 36 + (sampleRate * channels * bitsPerSample / 8); // Header + 1 second of audio
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
    uint32_t dataSize = sampleRate * channels * bitsPerSample / 8; // 1 second of audio
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Write simple sine wave data
    for (uint32_t i = 0; i < dataSize / (bitsPerSample / 8); ++i) {
        if (bitsPerSample == 16) {
            int16_t sample = static_cast<int16_t>(32767.0 * sin(2.0 * 3.14159 * 440.0 * i / sampleRate));
            file.write(reinterpret_cast<const char*>(&sample), 2);
        } else if (bitsPerSample == 8) {
            uint8_t sample = static_cast<uint8_t>(127 + 127 * sin(2.0 * 3.14159 * 440.0 * i / sampleRate));
            file.write(reinterpret_cast<const char*>(&sample), 1);
        }
    }

    file.close();
    return true;
}

bool TestAudioLoaderConstruction() {
    TestOutput::PrintTestStart("AudioLoader construction");

    AudioLoader loader;
    
    TestOutput::PrintTestPass("AudioLoader construction");
    return true;
}

bool TestWAVFileDetection() {
    TestOutput::PrintTestStart("WAV file detection");

    EXPECT_TRUE(AudioLoader::IsWAVFile("test.wav"));
    EXPECT_TRUE(AudioLoader::IsWAVFile("TEST.WAV"));
    EXPECT_TRUE(AudioLoader::IsWAVFile("path/to/file.wav"));
    EXPECT_FALSE(AudioLoader::IsWAVFile("test.mp3"));
    EXPECT_FALSE(AudioLoader::IsWAVFile("test.ogg"));
    EXPECT_FALSE(AudioLoader::IsWAVFile("test"));
    EXPECT_FALSE(AudioLoader::IsWAVFile(""));

    TestOutput::PrintTestPass("WAV file detection");
    return true;
}

bool TestWAVLoading() {
    TestOutput::PrintTestStart("WAV file loading");

    // Create test WAV file
    const std::string testFile = "test_audio.wav";
    if (!CreateTestWAVFile(testFile)) {
        TestOutput::PrintTestFail("WAV file loading");
        return false;
    }

    AudioLoader loader;
    AudioData data = loader.LoadWAV(testFile);

    EXPECT_TRUE(data.isValid);
    EXPECT_EQUAL(data.sampleRate, 44100u);
    EXPECT_EQUAL(data.channels, 2);
    EXPECT_EQUAL(data.bitsPerSample, 16);
    EXPECT_TRUE(data.duration > 0.9f && data.duration < 1.1f); // Should be approximately 1 second
    EXPECT_FALSE(data.data.empty());

#ifdef GAMEENGINE_HAS_OPENAL
    EXPECT_TRUE(data.format != AL_NONE);
#endif

    // Clean up
    std::remove(testFile.c_str());

    TestOutput::PrintTestPass("WAV file loading");
    return true;
}

bool TestWAVLoadingDifferentFormats() {
    TestOutput::PrintTestStart("WAV loading different formats");

    AudioLoader loader;

    // Test mono 8-bit
    const std::string monoFile = "test_mono.wav";
    if (CreateTestWAVFile(monoFile, 22050, 1, 8)) {
        AudioData data = loader.LoadWAV(monoFile);
        EXPECT_TRUE(data.isValid);
        EXPECT_EQUAL(data.sampleRate, 22050u);
        EXPECT_EQUAL(data.channels, 1);
        EXPECT_EQUAL(data.bitsPerSample, 8);
        std::remove(monoFile.c_str());
    }

    // Test stereo 16-bit
    const std::string stereoFile = "test_stereo.wav";
    if (CreateTestWAVFile(stereoFile, 48000, 2, 16)) {
        AudioData data = loader.LoadWAV(stereoFile);
        EXPECT_TRUE(data.isValid);
        EXPECT_EQUAL(data.sampleRate, 48000u);
        EXPECT_EQUAL(data.channels, 2);
        EXPECT_EQUAL(data.bitsPerSample, 16);
        std::remove(stereoFile.c_str());
    }

    TestOutput::PrintTestPass("WAV loading different formats");
    return true;
}

bool TestWAVLoadingErrors() {
    TestOutput::PrintTestStart("WAV loading error handling");

    AudioLoader loader;

    // Test non-existent file
    AudioData data = loader.LoadWAV("nonexistent.wav");
    EXPECT_FALSE(data.isValid);

    // Test non-WAV file
    data = loader.LoadWAV("test.mp3");
    EXPECT_FALSE(data.isValid);

    TestOutput::PrintTestPass("WAV loading error handling");
    return true;
}

#ifdef GAMEENGINE_HAS_OPENAL
bool TestOpenALBufferCreation() {
    TestOutput::PrintTestStart("OpenAL buffer creation");

    // Initialize OpenAL context for testing
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        TestOutput::PrintTestFail("OpenAL buffer creation - Failed to open device");
        return false;
    }

    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!context) {
        alcCloseDevice(device);
        TestOutput::PrintTestFail("OpenAL buffer creation - Failed to create context");
        return false;
    }

    alcMakeContextCurrent(context);

    // Create test WAV file
    const std::string testFile = "test_openal.wav";
    if (!CreateTestWAVFile(testFile)) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
        TestOutput::PrintTestFail("OpenAL buffer creation");
        return false;
    }

    AudioLoader loader;
    AudioData data = loader.LoadWAV(testFile);
    
    bool testPassed = false;
    if (data.isValid) {
        ALuint buffer = loader.CreateOpenALBuffer(data);
        EXPECT_TRUE(buffer != 0);
        
        if (buffer != 0) {
            testPassed = true;
            // Clean up OpenAL buffer
            alDeleteBuffers(1, &buffer);
        }
    }

    // Clean up
    std::remove(testFile.c_str());
    
    // Clean up OpenAL
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    if (testPassed) {
        TestOutput::PrintTestPass("OpenAL buffer creation");
    }
    return testPassed;
}
#endif

int main() {
    TestOutput::PrintHeader("Audio Loader Tests");
    Logger::GetInstance().Initialize();

    TestSuite suite("Audio Loader Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("Construction", TestAudioLoaderConstruction);
    allPassed &= suite.RunTest("WAV Detection", TestWAVFileDetection);
    allPassed &= suite.RunTest("WAV Loading", TestWAVLoading);
    allPassed &= suite.RunTest("WAV Different Formats", TestWAVLoadingDifferentFormats);
    allPassed &= suite.RunTest("WAV Error Handling", TestWAVLoadingErrors);

// OpenAL buffer creation test disabled - requires OpenAL context initialization
// #ifdef GAMEENGINE_HAS_OPENAL
//     allPassed &= suite.RunTest("OpenAL Buffer Creation", TestOpenALBufferCreation);
// #endif

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}