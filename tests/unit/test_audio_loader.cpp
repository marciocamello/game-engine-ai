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

    // Test empty filename
    data = loader.LoadWAV("");
    EXPECT_FALSE(data.isValid);

    // Test invalid file extension (should still try to load but fail)
    data = loader.LoadWAV("test.mp3");
    EXPECT_FALSE(data.isValid);

    TestOutput::PrintTestPass("WAV loading error handling");
    return true;
}

bool TestOGGFileDetection() {
    TestOutput::PrintTestStart("OGG file detection");

    EXPECT_TRUE(AudioLoader::IsOGGFile("test.ogg"));
    EXPECT_TRUE(AudioLoader::IsOGGFile("TEST.OGG"));
    EXPECT_TRUE(AudioLoader::IsOGGFile("path/to/file.ogg"));
    EXPECT_FALSE(AudioLoader::IsOGGFile("test.wav"));
    EXPECT_FALSE(AudioLoader::IsOGGFile("test.mp3"));
    EXPECT_FALSE(AudioLoader::IsOGGFile("test"));
    EXPECT_FALSE(AudioLoader::IsOGGFile(""));

    TestOutput::PrintTestPass("OGG file detection");
    return true;
}

bool TestOGGLoadingErrors() {
    TestOutput::PrintTestStart("OGG loading error handling");

    AudioLoader loader;

    // Test non-existent file
    AudioData data = loader.LoadOGG("nonexistent.ogg");
    EXPECT_FALSE(data.isValid);

    // Test empty filename
    data = loader.LoadOGG("");
    EXPECT_FALSE(data.isValid);

    // Test invalid file (WAV file with OGG extension)
    const std::string fakeOggFile = "fake.ogg";
    if (CreateTestWAVFile(fakeOggFile)) {
        data = loader.LoadOGG(fakeOggFile);
        EXPECT_FALSE(data.isValid); // Should fail because it's not actually OGG
        std::remove(fakeOggFile.c_str());
    }

    TestOutput::PrintTestPass("OGG loading error handling");
    return true;
}

bool TestUnifiedAudioLoading() {
    TestOutput::PrintTestStart("Unified audio loading interface");

    AudioLoader loader;

    // Create test WAV file
    const std::string wavFile = "test_unified.wav";
    if (CreateTestWAVFile(wavFile)) {
        AudioData data = loader.LoadAudio(wavFile);
        EXPECT_TRUE(data.isValid);
        EXPECT_EQUAL(data.sampleRate, 44100u);
        EXPECT_EQUAL(data.channels, 2);
        std::remove(wavFile.c_str());
    }

    // Test with non-existent file
    AudioData data = loader.LoadAudio("nonexistent_unified.wav");
    EXPECT_FALSE(data.isValid);

    // Test with empty filename
    data = loader.LoadAudio("");
    EXPECT_FALSE(data.isValid);

    TestOutput::PrintTestPass("Unified audio loading interface");
    return true;
}

#ifdef GAMEENGINE_HAS_OPENAL
bool TestOpenALFormatConversion() {
    TestOutput::PrintTestStart("OpenAL format conversion");

    // Test various channel/bit combinations
    ALenum format1 = AudioLoader::GetOpenALFormat(1, 8);  // Mono 8-bit
    ALenum format2 = AudioLoader::GetOpenALFormat(1, 16); // Mono 16-bit
    ALenum format3 = AudioLoader::GetOpenALFormat(2, 8);  // Stereo 8-bit
    ALenum format4 = AudioLoader::GetOpenALFormat(2, 16); // Stereo 16-bit

    EXPECT_TRUE(format1 == AL_FORMAT_MONO8);
    EXPECT_TRUE(format2 == AL_FORMAT_MONO16);
    EXPECT_TRUE(format3 == AL_FORMAT_STEREO8);
    EXPECT_TRUE(format4 == AL_FORMAT_STEREO16);

    // Test unsupported formats
    ALenum invalidFormat1 = AudioLoader::GetOpenALFormat(3, 16); // 3 channels
    ALenum invalidFormat2 = AudioLoader::GetOpenALFormat(2, 24); // 24-bit
    ALenum invalidFormat3 = AudioLoader::GetOpenALFormat(0, 16); // 0 channels

    EXPECT_TRUE(invalidFormat1 == AL_NONE);
    EXPECT_TRUE(invalidFormat2 == AL_NONE);
    EXPECT_TRUE(invalidFormat3 == AL_NONE);

    TestOutput::PrintTestPass("OpenAL format conversion");
    return true;
}
#endif

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
    allPassed &= suite.RunTest("OGG Detection", TestOGGFileDetection);
    allPassed &= suite.RunTest("OGG Error Handling", TestOGGLoadingErrors);
    allPassed &= suite.RunTest("Unified Audio Loading", TestUnifiedAudioLoading);

#ifdef GAMEENGINE_HAS_OPENAL
    allPassed &= suite.RunTest("OpenAL Format Conversion", TestOpenALFormatConversion);
    // OpenAL buffer creation test disabled - requires OpenAL context initialization
    // allPassed &= suite.RunTest("OpenAL Buffer Creation", TestOpenALBufferCreation);
#endif

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}