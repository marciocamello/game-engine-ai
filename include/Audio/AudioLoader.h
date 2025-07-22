#pragma once

#include <string>
#include <vector>
#include <memory>

#ifdef GAMEENGINE_HAS_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace GameEngine {

    struct AudioData {
        std::vector<char> data;
        int sampleRate = 0;
        int channels = 0;
        int bitsPerSample = 0;
        float duration = 0.0f;
        bool isValid = false;

#ifdef GAMEENGINE_HAS_OPENAL
        ALenum format = AL_NONE;
#endif
    };

    class AudioLoader {
    public:
        AudioLoader();
        ~AudioLoader();

        // Load audio data from file
        AudioData LoadWAV(const std::string& filepath);
        AudioData LoadOGG(const std::string& filepath);

        // Create OpenAL buffer from audio data
#ifdef GAMEENGINE_HAS_OPENAL
        ALuint CreateOpenALBuffer(const AudioData& audioData);
        static ALenum GetOpenALFormat(int channels, int bitsPerSample);
#endif

        // Utility functions
        static bool IsWAVFile(const std::string& filepath);
        static bool IsOGGFile(const std::string& filepath);

    private:
        // WAV file parsing
        struct WAVHeader {
            char riff[4];           // "RIFF"
            uint32_t fileSize;      // File size - 8
            char wave[4];           // "WAVE"
            char fmt[4];            // "fmt "
            uint32_t fmtSize;       // Format chunk size
            uint16_t audioFormat;   // Audio format (1 = PCM)
            uint16_t channels;      // Number of channels
            uint32_t sampleRate;    // Sample rate
            uint32_t byteRate;      // Byte rate
            uint16_t blockAlign;    // Block align
            uint16_t bitsPerSample; // Bits per sample
            char data[4];           // "data"
            uint32_t dataSize;      // Data size
        };

        AudioData LoadWAVImpl(const std::string& filepath);
        AudioData LoadOGGImpl(const std::string& filepath);
        
        bool ValidateWAVHeader(const WAVHeader& header);
        AudioData ParseWAVData(const std::vector<char>& fileData);
    };

} // namespace GameEngine