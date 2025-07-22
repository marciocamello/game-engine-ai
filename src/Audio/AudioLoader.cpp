#include "Audio/AudioLoader.h"
#include "Core/Logger.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace GameEngine {

    AudioLoader::AudioLoader() {
    }

    AudioLoader::~AudioLoader() {
    }

    AudioData AudioLoader::LoadWAV(const std::string& filepath) {
        return LoadWAVImpl(filepath);
    }

    AudioData AudioLoader::LoadOGG(const std::string& filepath) {
        // OGG loading will be implemented in a later task
        LOG_WARNING("OGG loading not yet implemented: " + filepath);
        AudioData data;
        data.isValid = false;
        return data;
    }

    AudioData AudioLoader::LoadWAVImpl(const std::string& filepath) {
        AudioData audioData;
        
        // Open file
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open WAV file: " + filepath);
            return audioData;
        }

        // Get file size
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (fileSize < sizeof(WAVHeader)) {
            LOG_ERROR("WAV file too small: " + filepath);
            return audioData;
        }

        // Read entire file into memory
        std::vector<char> fileData(fileSize);
        if (!file.read(fileData.data(), fileSize)) {
            LOG_ERROR("Failed to read WAV file: " + filepath);
            return audioData;
        }

        file.close();

        // Parse WAV data
        audioData = ParseWAVData(fileData);
        if (!audioData.isValid) {
            LOG_ERROR("Failed to parse WAV file: " + filepath);
        } else {
            std::ostringstream oss;
            oss << "Successfully loaded WAV file: " << filepath << " (" 
                << audioData.sampleRate << "Hz, " << audioData.channels 
                << " channels, " << audioData.bitsPerSample << " bits, " 
                << std::fixed << std::setprecision(2) << audioData.duration << "s)";
            LOG_INFO(oss.str());
        }

        return audioData;
    }

    AudioData AudioLoader::ParseWAVData(const std::vector<char>& fileData) {
        AudioData audioData;

        if (fileData.size() < sizeof(WAVHeader)) {
            LOG_ERROR("WAV data too small for header");
            return audioData;
        }

        // Read WAV header
        WAVHeader header;
        std::memcpy(&header, fileData.data(), sizeof(WAVHeader));

        // Validate header
        if (!ValidateWAVHeader(header)) {
            LOG_ERROR("Invalid WAV header");
            return audioData;
        }

        // Find data chunk (it might not be immediately after the format chunk)
        size_t dataOffset = 0;
        size_t dataSize = 0;
        bool foundDataChunk = false;

        // Start searching after the basic header
        size_t offset = 12; // Skip RIFF header
        
        while (offset + 8 < fileData.size()) {
            char chunkId[5] = {0};
            std::memcpy(chunkId, fileData.data() + offset, 4);
            
            uint32_t chunkSize;
            std::memcpy(&chunkSize, fileData.data() + offset + 4, 4);
            
            if (std::strncmp(chunkId, "fmt ", 4) == 0) {
                // Format chunk - extract audio format info
                if (offset + 8 + chunkSize > fileData.size()) {
                    LOG_ERROR("Format chunk extends beyond file");
                    return audioData;
                }
                
                // Read format data
                uint16_t audioFormat, channels, bitsPerSample;
                uint32_t sampleRate;
                
                std::memcpy(&audioFormat, fileData.data() + offset + 8, 2);
                std::memcpy(&channels, fileData.data() + offset + 10, 2);
                std::memcpy(&sampleRate, fileData.data() + offset + 12, 4);
                std::memcpy(&bitsPerSample, fileData.data() + offset + 22, 2);
                
                if (audioFormat != 1) { // Only support PCM
                    std::ostringstream oss;
                    oss << "Unsupported WAV format: " << audioFormat << " (only PCM supported)";
                    LOG_ERROR(oss.str());
                    return audioData;
                }
                
                audioData.channels = channels;
                audioData.sampleRate = sampleRate;
                audioData.bitsPerSample = bitsPerSample;
                
            } else if (std::strncmp(chunkId, "data", 4) == 0) {
                // Data chunk
                dataOffset = offset + 8;
                dataSize = chunkSize;
                foundDataChunk = true;
                break;
            }
            
            // Move to next chunk
            offset += 8 + chunkSize;
            // Align to even byte boundary
            if (chunkSize % 2 == 1) {
                offset++;
            }
        }

        if (!foundDataChunk) {
            LOG_ERROR("No data chunk found in WAV file");
            return audioData;
        }

        if (dataOffset + dataSize > fileData.size()) {
            LOG_ERROR("Data chunk extends beyond file");
            return audioData;
        }

        // Extract audio data
        audioData.data.resize(dataSize);
        std::memcpy(audioData.data.data(), fileData.data() + dataOffset, dataSize);

        // Calculate duration
        int bytesPerSample = (audioData.bitsPerSample / 8) * audioData.channels;
        if (bytesPerSample > 0) {
            audioData.duration = static_cast<float>(dataSize) / (audioData.sampleRate * bytesPerSample);
        }

#ifdef GAMEENGINE_HAS_OPENAL
        // Set OpenAL format
        audioData.format = GetOpenALFormat(audioData.channels, audioData.bitsPerSample);
        if (audioData.format == AL_NONE) {
            std::ostringstream oss;
            oss << "Unsupported audio format: " << audioData.channels 
                << " channels, " << audioData.bitsPerSample << " bits";
            LOG_ERROR(oss.str());
            return audioData;
        }
#endif

        audioData.isValid = true;
        return audioData;
    }

    bool AudioLoader::ValidateWAVHeader(const WAVHeader& header) {
        // Check RIFF signature
        if (std::strncmp(header.riff, "RIFF", 4) != 0) {
            LOG_ERROR("Invalid RIFF signature");
            return false;
        }

        // Check WAVE signature
        if (std::strncmp(header.wave, "WAVE", 4) != 0) {
            LOG_ERROR("Invalid WAVE signature");
            return false;
        }

        return true;
    }

#ifdef GAMEENGINE_HAS_OPENAL
    ALuint AudioLoader::CreateOpenALBuffer(const AudioData& audioData) {
        if (!audioData.isValid || audioData.data.empty()) {
            LOG_ERROR("Cannot create OpenAL buffer from invalid audio data");
            return 0;
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);
        
        ALenum error = alGetError();
        if (error != AL_NO_ERROR) {
            std::ostringstream oss;
            oss << "Failed to generate OpenAL buffer: " << error;
            LOG_ERROR(oss.str());
            return 0;
        }

        // Upload audio data to buffer
        alBufferData(buffer, audioData.format, audioData.data.data(), 
                     static_cast<ALsizei>(audioData.data.size()), audioData.sampleRate);
        
        error = alGetError();
        if (error != AL_NO_ERROR) {
            std::ostringstream oss;
            oss << "Failed to upload audio data to OpenAL buffer: " << error;
            LOG_ERROR(oss.str());
            alDeleteBuffers(1, &buffer);
            return 0;
        }

        std::ostringstream oss;
        oss << "Created OpenAL buffer " << buffer << " for audio data (" 
            << audioData.sampleRate << "Hz, " << audioData.channels 
            << " channels, " << audioData.bitsPerSample << " bits)";
        LOG_DEBUG(oss.str());

        return buffer;
    }

    ALenum AudioLoader::GetOpenALFormat(int channels, int bitsPerSample) {
        if (channels == 1) {
            if (bitsPerSample == 8) {
                return AL_FORMAT_MONO8;
            } else if (bitsPerSample == 16) {
                return AL_FORMAT_MONO16;
            }
        } else if (channels == 2) {
            if (bitsPerSample == 8) {
                return AL_FORMAT_STEREO8;
            } else if (bitsPerSample == 16) {
                return AL_FORMAT_STEREO16;
            }
        }

        return AL_NONE;
    }
#endif

    bool AudioLoader::IsWAVFile(const std::string& filepath) {
        // Simple extension check
        if (filepath.length() < 4) {
            return false;
        }
        
        std::string extension = filepath.substr(filepath.length() - 4);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension == ".wav";
    }

    bool AudioLoader::IsOGGFile(const std::string& filepath) {
        // Simple extension check
        if (filepath.length() < 4) {
            return false;
        }
        
        std::string extension = filepath.substr(filepath.length() - 4);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension == ".ogg";
    }

} // namespace GameEngine