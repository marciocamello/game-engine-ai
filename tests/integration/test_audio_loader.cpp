#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include <iostream>
#include <fstream>

using namespace GameEngine;

// Create a simple test WAV file
bool CreateTestWAVFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // WAV header for 1 second of 44.1kHz 16-bit mono sine wave
    struct WAVHeader {
        char riff[4] = {'R', 'I', 'F', 'F'};
        uint32_t fileSize = 44036; // 44 bytes header + 88200 bytes data - 8
        char wave[4] = {'W', 'A', 'V', 'E'};
        char fmt[4] = {'f', 'm', 't', ' '};
        uint32_t fmtSize = 16;
        uint16_t audioFormat = 1; // PCM
        uint16_t numChannels = 1; // Mono
        uint32_t sampleRate = 44100;
        uint32_t byteRate = 88200; // sampleRate * numChannels * bitsPerSample/8
        uint16_t blockAlign = 2; // numChannels * bitsPerSample/8
        uint16_t bitsPerSample = 16;
    } header;

    struct DataChunk {
        char data[4] = {'d', 'a', 't', 'a'};
        uint32_t dataSize = 88200; // 1 second of 44.1kHz 16-bit mono
    } dataChunk;

    // Write header
    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(&dataChunk), sizeof(dataChunk));

    // Write simple sine wave data (440Hz tone)
    const int sampleCount = 44100;
    const double frequency = 440.0; // A4 note
    const double amplitude = 16000.0; // Safe amplitude for 16-bit
    
    for (int i = 0; i < sampleCount; ++i) {
        double time = static_cast<double>(i) / 44100.0;
        double sample = amplitude * std::sin(2.0 * 3.14159265359 * frequency * time);
        int16_t sampleValue = static_cast<int16_t>(sample);
        file.write(reinterpret_cast<const char*>(&sampleValue), sizeof(sampleValue));
    }

    return file.good();
}

int main() {
    std::cout << "AudioLoader Integration Test\n";
    std::cout << "============================\n\n";

    // Test 1: Create test WAV file
    std::cout << "Test 1: Creating test WAV file...\n";
    const std::string testFile = "test_audio.wav";
    
    if (!CreateTestWAVFile(testFile)) {
        std::cout << "FAILED: Could not create test WAV file\n";
        return 1;
    }
    std::cout << "PASSED: Test WAV file created\n\n";

    // Test 2: Check file type detection
    std::cout << "Test 2: File type detection...\n";
    if (!AudioLoader::IsWAVFile(testFile)) {
        std::cout << "FAILED: WAV file not detected correctly\n";
        return 1;
    }
    
    if (AudioLoader::IsWAVFile("test.mp3")) {
        std::cout << "FAILED: Non-WAV file incorrectly detected as WAV\n";
        return 1;
    }
    std::cout << "PASSED: File type detection working\n\n";

    // Test 3: Load WAV file
    std::cout << "Test 3: Loading WAV file...\n";
    AudioLoader loader;
    AudioData audioData = loader.LoadWAV(testFile);
    
    if (!audioData.isValid) {
        std::cout << "FAILED: Could not load WAV file\n";
        std::cout << "Error: " << AudioLoader::GetLastError() << "\n";
        return 1;
    }
    
    // Verify audio data properties
    if (audioData.sampleRate != 44100) {
        std::cout << "FAILED: Incorrect sample rate. Expected: 44100, Got: " << audioData.sampleRate << "\n";
        return 1;
    }
    
    if (audioData.channels != 1) {
        std::cout << "FAILED: Incorrect channel count. Expected: 1, Got: " << audioData.channels << "\n";
        return 1;
    }
    
    if (audioData.bitsPerSample != 16) {
        std::cout << "FAILED: Incorrect bits per sample. Expected: 16, Got: " << audioData.bitsPerSample << "\n";
        return 1;
    }
    
    if (audioData.data.size() != 88200) {
        std::cout << "FAILED: Incorrect data size. Expected: 88200, Got: " << audioData.data.size() << "\n";
        return 1;
    }
    
    std::cout << "PASSED: WAV file loaded successfully\n";
    std::cout << "  Sample Rate: " << audioData.sampleRate << " Hz\n";
    std::cout << "  Channels: " << audioData.channels << "\n";
    std::cout << "  Bits per Sample: " << audioData.bitsPerSample << "\n";
    std::cout << "  Duration: " << audioData.duration << " seconds\n";
    std::cout << "  Data Size: " << audioData.data.size() << " bytes\n\n";

#ifdef GAMEENGINE_HAS_OPENAL
    // Test 4: OpenAL buffer creation (if OpenAL is available)
    std::cout << "Test 4: OpenAL buffer creation...\n";
    ALuint buffer = loader.CreateOpenALBuffer(audioData);
    
    if (buffer == 0) {
        std::cout << "FAILED: Could not create OpenAL buffer\n";
        std::cout << "Error: " << AudioLoader::GetLastError() << "\n";
        return 1;
    }
    
    std::cout << "PASSED: OpenAL buffer created (ID: " << buffer << ")\n\n";
    
    // Clean up OpenAL buffer
    alDeleteBuffers(1, &buffer);
#else
    std::cout << "Test 4: SKIPPED - OpenAL not available\n\n";
#endif

    // Test 5: Error handling
    std::cout << "Test 5: Error handling...\n";
    AudioData invalidData = loader.LoadWAV("nonexistent_file.wav");
    
    if (invalidData.isValid) {
        std::cout << "FAILED: Should have failed to load nonexistent file\n";
        return 1;
    }
    
    std::string error = AudioLoader::GetLastError();
    if (error.empty()) {
        std::cout << "FAILED: Error message should not be empty\n";
        return 1;
    }
    
    std::cout << "PASSED: Error handling working correctly\n";
    std::cout << "Error message: " << error << "\n\n";

    // Clean up test file
    std::remove(testFile.c_str());

    std::cout << "All tests passed!\n";
    std::cout << "AudioLoader is working correctly.\n";
    
    return 0;
}