#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include <fstream>
#include <algorithm>
#include <cstring>

namespace GameEngine {

    std::string AudioLoader::s_lastError;

    AudioLoader::AudioLoader() {
    }

    AudioLoader::~AudioLoader() {
    }

    AudioData AudioLoader::LoadWAV(const std::string& filepath) {
        s_lastError.clear();
        
        if (!IsWAVFile(filepath)) {
            s_lastError = "File is not a WAV file: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return AudioData{};
        }

        return LoadWAVImpl(filepath);
    }

    AudioData AudioLoader::LoadWAVImpl(const std::string& filepath) {
        AudioData audioData;
        
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            s_lastError = "Failed to open file: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return audioData;
        }

        // Read WAV header
        WAVHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
        
        if (file.gcount() != sizeof(WAVHeader)) {
            s_lastError = "Failed to read WAV header from: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return audioData;
        }

        // Validate header
        if (!ValidateWAVHeader(header)) {
            s_lastError = "Invalid WAV header in: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return audioData;
        }

        // Find data chunk
        WAVDataChunk dataChunk;
        if (!FindDataChunk(file, dataChunk)) {
            s_lastError = "Failed to find data chunk in: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return audioData;
        }

        // Read audio data
        audioData.data.resize(dataChunk.dataSize);
        file.read(audioData.data.data(), dataChunk.dataSize);
        
        if (file.gcount() != static_cast<std::streamsize>(dataChunk.dataSize)) {
            s_lastError = "Failed to read audio data from: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return audioData;
        }

        // Fill audio data structure
        audioData.sampleRate = header.sampleRate;
        audioData.channels = header.numChannels;
        audioData.bitsPerSample = header.bitsPerSample;
        audioData.duration = static_cast<float>(dataChunk.dataSize) / header.byteRate;
        audioData.isValid = true;

#ifdef GAMEENGINE_HAS_OPENAL
        audioData.format = GetOpenALFormat(header.numChannels, header.bitsPerSample);
        if (audioData.format == AL_NONE) {
            s_lastError = "Unsupported audio format in: " + filepath;
            LOG_ERROR("AudioLoader: {}", s_lastError);
            audioData.isValid = false;
            return audioData;
        }
#endif

        LOG_INFO("AudioLoader: Successfully loaded WAV file: {}", filepath);
        LOG_INFO("  Sample Rate: {} Hz", audioData.sampleRate);
        LOG_INFO("  Channels: {}", audioData.channels);
        LOG_INFO("  Bits per Sample: {}", audioData.bitsPerSample);
        LOG_INFO("  Duration: {:.2f} seconds", audioData.duration);

        return audioData;
    }

    bool AudioLoader::ValidateWAVHeader(const WAVHeader& header) {
        // Check RIFF signature
        if (std::strncmp(header.riff, "RIFF", 4) != 0) {
            LOG_ERROR("AudioLoader: Invalid RIFF signature");
            return false;
        }

        // Check WAVE signature
        if (std::strncmp(header.wave, "WAVE", 4) != 0) {
            LOG_ERROR("AudioLoader: Invalid WAVE signature");
            return false;
        }

        // Check fmt signature
        if (std::strncmp(header.fmt, "fmt ", 4) != 0) {
            LOG_ERROR("AudioLoader: Invalid fmt signature");
            return false;
        }

        // Check audio format (only PCM supported)
        if (header.audioFormat != 1) {
            LOG_ERROR("AudioLoader: Unsupported audio format: {} (only PCM supported)", header.audioFormat);
            return false;
        }

        // Validate channels
        if (header.numChannels == 0 || header.numChannels > 8) {
            LOG_ERROR("AudioLoader: Invalid number of channels: {}", header.numChannels);
            return false;
        }

        // Validate sample rate
        if (header.sampleRate == 0 || header.sampleRate > 192000) {
            LOG_ERROR("AudioLoader: Invalid sample rate: {}", header.sampleRate);
            return false;
        }

        // Validate bits per sample
        if (header.bitsPerSample != 8 && header.bitsPerSample != 16 && header.bitsPerSample != 24 && header.bitsPerSample != 32) {
            LOG_ERROR("AudioLoader: Unsupported bits per sample: {}", header.bitsPerSample);
            return false;
        }

        // Validate calculated values
        uint32_t expectedByteRate = header.sampleRate * header.numChannels * (header.bitsPerSample / 8);
        if (header.byteRate != expectedByteRate) {
            LOG_WARNING("AudioLoader: Byte rate mismatch. Expected: {}, Got: {}", expectedByteRate, header.byteRate);
        }

        uint16_t expectedBlockAlign = header.numChannels * (header.bitsPerSample / 8);
        if (header.blockAlign != expectedBlockAlign) {
            LOG_WARNING("AudioLoader: Block align mismatch. Expected: {}, Got: {}", expectedBlockAlign, header.blockAlign);
        }

        return true;
    }

    bool AudioLoader::FindDataChunk(std::ifstream& file, WAVDataChunk& dataChunk) {
        // Skip any additional chunks after fmt to find data chunk
        while (file.good()) {
            char chunkId[4];
            uint32_t chunkSize;
            
            file.read(chunkId, 4);
            if (file.gcount() != 4) {
                break;
            }
            
            file.read(reinterpret_cast<char*>(&chunkSize), sizeof(uint32_t));
            if (file.gcount() != sizeof(uint32_t)) {
                break;
            }
            
            if (std::strncmp(chunkId, "data", 4) == 0) {
                // Found data chunk
                std::memcpy(dataChunk.data, chunkId, 4);
                dataChunk.dataSize = chunkSize;
                return true;
            } else {
                // Skip this chunk
                file.seekg(chunkSize, std::ios::cur);
                if (file.fail()) {
                    LOG_ERROR("AudioLoader: Failed to skip chunk");
                    break;
                }
            }
        }
        
        return false;
    }

#ifdef GAMEENGINE_HAS_OPENAL
    ALenum AudioLoader::GetOpenALFormat(int channels, int bitsPerSample) {
        if (channels == 1) {
            switch (bitsPerSample) {
                case 8:  return AL_FORMAT_MONO8;
                case 16: return AL_FORMAT_MONO16;
                default: return AL_NONE;
            }
        } else if (channels == 2) {
            switch (bitsPerSample) {
                case 8:  return AL_FORMAT_STEREO8;
                case 16: return AL_FORMAT_STEREO16;
                default: return AL_NONE;
            }
        }
        
        // Unsupported format
        return AL_NONE;
    }

    ALuint AudioLoader::CreateOpenALBuffer(const AudioData& audioData) {
        if (!audioData.isValid) {
            s_lastError = "Invalid audio data provided";
            LOG_ERROR("AudioLoader: {}", s_lastError);
            return 0;
        }

        ALuint buffer = 0;
        alGenBuffers(1, &buffer);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            s_lastError = "Failed to generate OpenAL buffer";
            LOG_ERROR("AudioLoader: {} (OpenAL error: {})", s_lastError, error);
            return 0;
        }

        // Upload audio data to buffer
        alBufferData(buffer, audioData.format, audioData.data.data(), 
                    static_cast<ALsizei>(audioData.data.size()), audioData.sampleRate);
        
        error = alGetError();
        if (error != AL_NO_ERROR) {
            s_lastError = "Failed to upload audio data to OpenAL buffer";
            LOG_ERROR("AudioLoader: {} (OpenAL error: {})", s_lastError, error);
            alDeleteBuffers(1, &buffer);
            return 0;
        }

        LOG_INFO("AudioLoader: Successfully created OpenAL buffer (ID: {})", buffer);
        return buffer;
    }
#endif

    bool AudioLoader::IsWAVFile(const std::string& filepath) {
        // Check file extension
        size_t dotPos = filepath.find_last_of('.');
        if (dotPos == std::string::npos) {
            return false;
        }
        
        std::string extension = filepath.substr(dotPos + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        return extension == "wav";
    }

} // namespace GameEngine