#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <fstream>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Create a simple test WAV file in memory
std::vector<char> CreateTestWAVFile() {
    std::vector<char> wavData;
    
    // WAV header for 1 second of 44100Hz, 16-bit, mono sine wave
    const char* riff = "RIFF";
    const char* wave = "WAVE";
    const char* fmt = "fmt ";
    const char* data = "data";
    
    uint32_t fileSize = 36 + 88200; // Header size + data size
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t channels = 1;
    uint32_t sampleRate = 44100;
    uint32_t byteRate = 88200; // sampleRate * channels * bitsPerSample/8
    uint16_t blockAlign = 2; // channels * bitsPerSample/8
    uint16_t bitsPerSample = 16;
    uint32_t dataSize = 88200; // 1 second of audio
    
    // Write header
    wavData.insert(wavData.end(), riff, riff + 4);
    wavData.insert(wavData.end(), (char*)&fileSize, (char*)&fileSize + 4);
    wavData.insert(wavData.end(), wave, wave + 4);
    wavData.insert(wavData.end(), fmt, fmt + 4);
    wavData.insert(wavData.end(), (char*)&fmtSize, (char*)&fmtSize + 4);
    wavData.insert(wavData.end(), (char*)&audioFormat, (char*)&audioFormat + 2);
    wavData.insert(wavData.end(), (char*)&channels, (char*)&channels + 2);
    wavData.insert(wavData.end(), (char*)&sampleRate, (char*)&sampleRate + 4);
    wavData.insert(wavData.end(), (char*)&byteRate, (char*)&byteRate + 4);
    wavData.insert(wavData.end(), (char*)&blockAlign, (char*)&blockAlign + 2);
    wavData.insert(wavData.end(), (char*)&bitsPerSample, (char*)&bitsPerSample + 2);
    wavData.insert(wavData.end(), data, data + 4);
    wavData.insert(wavData.end(), (char*)&dataSize, (char*)&dataSize + 4);
    
    // Generate simple sine wave data
    for (int i = 0; i < 44100; i++) {
        double t = (double)i / 44100.0;
        double frequency = 440.0; // A4 note
        int16_t sample = (int16_t)(sin(2.0 * 3.14159 * frequency * t) * 16000);
        wavData.insert(wavData.end(), (char*)&sample, (char*)&sample + 2);
    }
    
    return wavData;
}

bool TestAudioLoaderCreation() {
    TestOutput::PrintTestStart("AudioLoader creation");
    
    AudioLoader loader;
    
    TestOutput::PrintTestPass("AudioLoader creation");
    return true;
}

bool TestWAVFileDetection() {
    TestOutput::PrintTestStart("WAV file detection");
    
    EXPECT_TRUE(AudioLoader::IsWAVFile("test.wav"));
    EXPECT_TRUE(AudioLoader::IsWAVFile("audio/music.WAV"));
    EXPECT_FALSE(AudioLoader::IsWAVFile("test.ogg"));
    EXPECT_FALSE(AudioLoader::IsWAVFile("test.mp3"));
    EXPECT_FALSE(AudioLoader::IsWAVFile("test"));
    
    TestOutput::PrintTestPass("WAV file detection");
    return true;
}

bool TestOGGFileDetection() {
    TestOutput::PrintTestStart("OGG file detection");
    
    EXPECT_TRUE(AudioLoader::IsOGGFile("test.ogg"));
    EXPECT_TRUE(AudioLoader::IsOGGFile("audio/music.OGG"));
    EXPECT_FALSE(AudioLoader::IsOGGFile("test.wav"));
    EXPECT_FALSE(AudioLoader::IsOGGFile("test.mp3"));
    EXPECT_FALSE(AudioLoader::IsOGGFile("test"));
    
    TestOutput::PrintTestPass("OGG file detection");
    return true;
}

bool TestOGGLoadingFromRealFile() {
    TestOutput::PrintTestStart("OGG loading from real file");
    
    AudioLoader loader;
    
    // Test loading the real OGG file
    AudioData oggData = loader.LoadOGG("assets/audio/file_example_OOG_1MG.ogg");
    
    EXPECT_TRUE(oggData.isValid);
    EXPECT_EQUAL(oggData.sampleRate, 44100);
    EXPECT_EQUAL(oggData.channels, 2);
    EXPECT_EQUAL(oggData.bitsPerSample, 16);
    EXPECT_TRUE(oggData.duration > 70.0f && oggData.duration < 80.0f); // ~75 seconds
    EXPECT_TRUE(!oggData.data.empty());
    
    TestOutput::PrintTestPass("OGG loading from real file");
    return true;
}

bool TestUnifiedAudioLoading() {
    TestOutput::PrintTestStart("Unified audio loading interface");
    
    AudioLoader loader;
    
    // Test loading WAV through unified interface
    AudioData wavData = loader.LoadWAV("assets/audio/file_example_WAV_5MG.wav");
    EXPECT_TRUE(wavData.isValid);
    
    // Test loading OGG through unified interface  
    AudioData oggData = loader.LoadOGG("assets/audio/file_example_OOG_1MG.ogg");
    EXPECT_TRUE(oggData.isValid);
    
    // Compare properties
    EXPECT_EQUAL(wavData.sampleRate, oggData.sampleRate); // Both should be 44100Hz
    EXPECT_EQUAL(wavData.channels, oggData.channels); // Both should be stereo
    EXPECT_EQUAL(wavData.bitsPerSample, oggData.bitsPerSample); // Both should be 16-bit
    
    TestOutput::PrintTestPass("Unified audio loading interface");
    return true;
}

bool TestWAVLoadingFromFile() {
    TestOutput::PrintTestStart("WAV loading from file");
    
    // Create a test WAV file
    std::vector<char> testWavData = CreateTestWAVFile();
    
    // Write to temporary file
    std::string tempFile = "test_audio.wav";
    std::ofstream file(tempFile, std::ios::binary);
    if (!file.is_open()) {
        TestOutput::PrintTestFail("WAV loading from file");
        return false;
    }
    
    file.write(testWavData.data(), testWavData.size());
    file.close();
    
    // Test loading
    AudioLoader loader;
    AudioData audioData = loader.LoadWAV(tempFile);
    
    EXPECT_TRUE(audioData.isValid);
    EXPECT_EQUAL(audioData.sampleRate, 44100);
    EXPECT_EQUAL(audioData.channels, 1);
    EXPECT_EQUAL(audioData.bitsPerSample, 16);
    EXPECT_TRUE(audioData.duration > 0.9f && audioData.duration < 1.1f); // ~1 second
    EXPECT_TRUE(!audioData.data.empty());
    
    // Clean up
    std::remove(tempFile.c_str());
    
    TestOutput::PrintTestPass("WAV loading from file");
    return true;
}

bool TestInvalidWAVFile() {
    TestOutput::PrintTestStart("Invalid WAV file handling");
    
    AudioLoader loader;
    
    // Test non-existent file
    AudioData audioData = loader.LoadWAV("nonexistent.wav");
    EXPECT_FALSE(audioData.isValid);
    
    // Test invalid file (create a file with wrong header)
    std::string tempFile = "invalid_test.wav";
    std::ofstream file(tempFile, std::ios::binary);
    if (file.is_open()) {
        file.write("INVALID_HEADER", 14);
        file.close();
        
        audioData = loader.LoadWAV(tempFile);
        EXPECT_FALSE(audioData.isValid);
        
        std::remove(tempFile.c_str());
    }
    
    TestOutput::PrintTestPass("Invalid WAV file handling");
    return true;
}

#ifdef GAMEENGINE_HAS_OPENAL
bool TestOpenALBufferCreation() {
    TestOutput::PrintTestStart("OpenAL buffer creation");
    
    // Create test audio data
    AudioData audioData;
    audioData.sampleRate = 44100;
    audioData.channels = 1;
    audioData.bitsPerSample = 16;
    audioData.data.resize(1024); // Small amount of data
    audioData.format = AudioLoader::GetOpenALFormat(1, 16);
    audioData.isValid = true;
    
    AudioLoader loader;
    ALuint buffer = loader.CreateOpenALBuffer(audioData);
    
    EXPECT_TRUE(buffer != 0);
    
    // Clean up
    if (buffer != 0) {
        alDeleteBuffers(1, &buffer);
    }
    
    TestOutput::PrintTestPass("OpenAL buffer creation");
    return true;
}

bool TestOpenALFormatDetection() {
    TestOutput::PrintTestStart("OpenAL format detection");
    
    EXPECT_EQUAL(AudioLoader::GetOpenALFormat(1, 8), AL_FORMAT_MONO8);
    EXPECT_EQUAL(AudioLoader::GetOpenALFormat(1, 16), AL_FORMAT_MONO16);
    EXPECT_EQUAL(AudioLoader::GetOpenALFormat(2, 8), AL_FORMAT_STEREO8);
    EXPECT_EQUAL(AudioLoader::GetOpenALFormat(2, 16), AL_FORMAT_STEREO16);
    EXPECT_EQUAL(AudioLoader::GetOpenALFormat(3, 16), AL_NONE); // Unsupported
    EXPECT_EQUAL(AudioLoader::GetOpenALFormat(1, 24), AL_NONE); // Unsupported
    
    TestOutput::PrintTestPass("OpenAL format detection");
    return true;
}
#endif

int main() {
    TestOutput::PrintHeader("AudioLoader Tests");
    Logger::GetInstance().Initialize();
    
    TestSuite suite("AudioLoader Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("AudioLoader Creation", TestAudioLoaderCreation);
    allPassed &= suite.RunTest("WAV File Detection", TestWAVFileDetection);
    allPassed &= suite.RunTest("OGG File Detection", TestOGGFileDetection);
    allPassed &= suite.RunTest("WAV Loading from File", TestWAVLoadingFromFile);
    allPassed &= suite.RunTest("Invalid WAV File Handling", TestInvalidWAVFile);
    
#ifdef GAMEENGINE_HAS_OPENAL
    allPassed &= suite.RunTest("OpenAL Buffer Creation", TestOpenALBufferCreation);
    allPassed &= suite.RunTest("OpenAL Format Detection", TestOpenALFormatDetection);
#endif
    
    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}