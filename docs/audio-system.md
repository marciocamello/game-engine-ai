# Audio System Documentation

## Overview

Game Engine Kiro features a comprehensive 3D audio system built on OpenAL, supporting multiple audio formats and providing spatial audio capabilities for immersive gaming experiences.

## Supported Audio Formats

### WAV (Waveform Audio File Format)

- **Status**: ✅ Fully Supported
- **Implementation**: Native WAV parser with PCM support
- **Supported Configurations**:
  - Sample Rates: 8kHz to 192kHz
  - Channels: Mono (1) and Stereo (2)
  - Bit Depths: 8-bit and 16-bit PCM
- **Use Cases**: High-quality sound effects, music, voice

### OGG Vorbis

- **Status**: ✅ Fully Supported
- **Implementation**: STB Vorbis integration
- **Supported Configurations**:
  - Sample Rates: Variable (typically 44.1kHz)
  - Channels: Mono and Stereo
  - Bit Depth: 16-bit (decoded from compressed format)
- **Use Cases**: Compressed music, ambient sounds, longer audio clips

### MP3 (MPEG Audio Layer III)

- **Status**: ❌ Not Implemented
- **Reason**: Patent concerns and complexity
- **Alternative**: Use OGG Vorbis for compressed audio

## Architecture

### Core Components

```
AudioEngine
├── AudioLoader (Format Detection & Loading)
├── AudioSource (3D Positioned Audio)
├── AudioListener (Player/Camera Audio Position)
└── OpenAL Integration (Hardware Acceleration)
```

### Audio Loading Pipeline

1. **Format Detection**: Automatic detection based on file extension
2. **Data Loading**: Format-specific parsers (WAV native, OGG via STB)
3. **OpenAL Buffer Creation**: Hardware-optimized audio buffers
4. **Caching**: Automatic caching to prevent duplicate loading

## API Reference

### AudioEngine

```cpp
// Initialize audio system
AudioEngine audioEngine;
if (!audioEngine.Initialize()) {
    LOG_ERROR("Failed to initialize audio system");
    return false;
}

// Load audio clips (returns shared_ptr)
auto clip = audioEngine.LoadAudioClip("assets/audio/sound.wav");
auto music = audioEngine.LoadAudioClip("assets/audio/music.ogg");

// Create and control audio sources
uint32_t sourceId = audioEngine.CreateAudioSource();
audioEngine.PlayAudioSource(sourceId, clip);
audioEngine.SetAudioSourcePosition(sourceId, Math::Vec3(10.0f, 0.0f, 5.0f));
audioEngine.SetAudioSourceVolume(sourceId, 0.8f);
audioEngine.SetAudioSourceLooping(sourceId, true);

// Error handling and recovery
if (!audioEngine.IsAudioAvailable()) {
    LOG_WARNING("Audio system not available");
} else if (!audioEngine.IsOpenALInitialized()) {
    if (audioEngine.AttemptAudioRecovery()) {
        LOG_INFO("Audio system recovered");
    }
}

// Performance optimization
audioEngine.EnableBufferPooling(true);
audioEngine.EnableSourcePooling(true);
audioEngine.SetBufferPoolSize(64);
audioEngine.SetSourcePoolSize(8, 32);

// Performance monitoring
float bufferHitRatio = audioEngine.GetBufferPoolHitRatio();
float sourceUtilization = audioEngine.GetSourcePoolUtilization();
size_t bufferMemory = audioEngine.GetBufferPoolMemoryUsage();
```

### AudioLoader

```cpp
AudioLoader loader;

// Direct format loading
AudioData wavData = loader.LoadWAV("sound.wav");
AudioData oggData = loader.LoadOGG("music.ogg");

// Format detection
bool isWav = AudioLoader::IsWAVFile("sound.wav");
bool isOgg = AudioLoader::IsOGGFile("music.ogg");
```

## 3D Spatial Audio

### Listener Configuration

```cpp
// Set listener position (typically camera/player position)
audioEngine.SetListenerPosition(Math::Vec3(0.0f, 1.8f, 0.0f));
audioEngine.SetListenerOrientation(forward, up);
audioEngine.SetListenerVelocity(velocity); // For Doppler effect
```

### Source Positioning

```cpp
// Position audio sources in 3D space
audioEngine.SetAudioSourcePosition(sourceId, worldPosition);
audioEngine.SetAudioSourceVolume(sourceId, 1.0f);
audioEngine.SetAudioSourcePitch(sourceId, 1.0f);
audioEngine.SetAudioSourceLooping(sourceId, true);
```

## Asset Organization

### Recommended Directory Structure

```
assets/
└── audio/
    ├── music/          # Background music (OGG recommended)
    │   ├── menu.ogg
    │   └── gameplay.ogg
    ├── sfx/            # Sound effects (WAV recommended)
    │   ├── jump.wav
    │   ├── footstep.wav
    │   └── explosion.wav
    └── voice/          # Voice acting (OGG recommended)
        ├── dialogue1.ogg
        └── narration.ogg
```

### Format Recommendations

| Use Case         | Recommended Format | Reason                                 |
| ---------------- | ------------------ | -------------------------------------- |
| Short SFX (< 5s) | WAV                | Low latency, no decompression overhead |
| Music/Ambient    | OGG                | Good compression, quality balance      |
| Voice/Dialogue   | OGG                | Efficient for longer clips             |
| UI Sounds        | WAV                | Instant playback, small file sizes     |

## Performance Considerations

### Memory Usage

- **WAV Files**: Loaded entirely into memory (uncompressed)
- **OGG Files**: Decoded to PCM and cached (16-bit stereo)
- **Caching**: Automatic caching prevents duplicate loading

### CPU Usage

- **WAV**: Minimal CPU overhead (direct PCM data)
- **OGG**: One-time decompression cost during loading
- **3D Processing**: Hardware-accelerated via OpenAL

### Optimization Tips

1. Use WAV for frequently played short sounds
2. Use OGG for music and longer audio clips
3. Preload critical audio during level loading
4. Unload unused audio clips to free memory

## Testing

### Automated Tests

- Format detection validation
- Loading functionality verification
- Error handling for invalid files
- OpenAL integration testing

### Manual Testing

```cpp
// In-game audio testing (F3/F4/F5 keys)
F3 - Play WAV audio sample
F4 - Play OGG audio sample
F5 - Stop all audio playback
```

## Error Handling

### Common Issues

1. **File Not Found**: Check file path and asset copying
2. **Unsupported Format**: Verify file format (WAV/OGG only)
3. **Corrupted Files**: Validate audio file integrity
4. **OpenAL Errors**: Check audio device availability

### Debug Information

```cpp
// Enable detailed audio logging
LOG_INFO("Audio clip loaded: " + clip->path);
LOG_INFO("Duration: " + std::to_string(clip->duration) + "s");
LOG_INFO("Channels: " + std::to_string(clip->channels));
LOG_INFO("Sample Rate: " + std::to_string(clip->sampleRate) + "Hz");
```

## Complete Usage Examples

### Basic Audio Setup

```cpp
#include "Audio/AudioEngine.h"
#include "Core/Engine.h"

// Initialize audio system
auto* audioEngine = engine.GetAudio();
if (!audioEngine->Initialize()) {
    LOG_ERROR("Failed to initialize audio system");
    return false;
}

// Load various audio formats
auto jumpSound = audioEngine->LoadAudioClip("assets/audio/jump.wav");
auto backgroundMusic = audioEngine->LoadAudioClip("assets/audio/music.ogg");
auto footstepSound = audioEngine->LoadAudioClip("assets/audio/footstep.wav");

// Create audio sources
uint32_t jumpSourceId = audioEngine->CreateAudioSource();
uint32_t musicSourceId = audioEngine->CreateAudioSource();
uint32_t footstepSourceId = audioEngine->CreateAudioSource();
```

### 3D Spatial Audio Implementation

```cpp
// Application-level 3D audio handling (not in Character class)
void GameApplication::UpdateCharacterAudio(float deltaTime) {
    // Update listener position based on character position
    Math::Vec3 characterPos = m_character->GetPosition();
    Math::Vec3 cameraForward = m_camera->GetForward();
    Math::Vec3 cameraUp = m_camera->GetUp();

    m_audioEngine->SetListenerPosition(characterPos);
    m_audioEngine->SetListenerOrientation(cameraForward, cameraUp);

    // Position footstep sounds at character feet
    m_audioEngine->SetAudioSourcePosition(m_footstepSourceId,
        Math::Vec3(characterPos.x, characterPos.y - 0.9f, characterPos.z));

    // Play footsteps based on movement
    if (character->IsGrounded() && character->GetVelocity().length() > 0.1f) {
        static float footstepTimer = 0.0f;
        footstepTimer += deltaTime;

        if (footstepTimer >= 0.5f) { // Every 0.5 seconds
            audioEngine->PlayAudioSource(footstepSourceId, footstepSound);
            footstepTimer = 0.0f;
        }
    }
}

// Environmental 3D audio sources
void SetupEnvironmentalAudio() {
    // Create positioned audio sources around the world
    struct AudioSource3D {
        uint32_t sourceId;
        Math::Vec3 position;
        std::shared_ptr<AudioClip> clip;
        std::string description;
    };

    std::vector<AudioSource3D> environmentalSources = {
        {audioEngine->CreateAudioSource(), {10.0f, 1.0f, 0.0f}, windSound, "Wind source"},
        {audioEngine->CreateAudioSource(), {-5.0f, 2.0f, 8.0f}, waterSound, "Water source"},
        {audioEngine->CreateAudioSource(), {0.0f, 5.0f, -10.0f}, birdSound, "Bird source"}
    };

    for (auto& source : environmentalSources) {
        audioEngine->SetAudioSourcePosition(source.sourceId, source.position);
        audioEngine->SetAudioSourceLooping(source.sourceId, true);
        audioEngine->SetAudioSourceVolume(source.sourceId, 0.6f);
        audioEngine->PlayAudioSource(source.sourceId, source.clip);

        LOG_INFO("Created environmental audio: " + source.description);
    }
}
```

### Performance Optimization

```cpp
// Optimize audio system for better performance
void OptimizeAudioSystem() {
    // Enable performance features
    audioEngine->EnableBufferPooling(true);
    audioEngine->EnableSourcePooling(true);
    audioEngine->EnableOptimized3DAudio(true);

    // Configure pool sizes based on game requirements
    audioEngine->SetBufferPoolSize(64);        // 64 audio buffers
    audioEngine->SetSourcePoolSize(16, 64);    // 16-64 audio sources

    // Mark frequently used audio as "hot" for optimization
    audioEngine->MarkAudioAsHot("assets/audio/footstep.wav");
    audioEngine->MarkAudioAsHot("assets/audio/jump.wav");
    audioEngine->MarkAudioAsHot("assets/audio/ui_click.wav");

    // Set volume levels for different audio categories
    audioEngine->SetMasterVolume(1.0f);
    audioEngine->SetMusicVolume(0.7f);
    audioEngine->SetSFXVolume(0.8f);
}

// Monitor audio performance
void MonitorAudioPerformance() {
    float bufferHitRatio = audioEngine->GetBufferPoolHitRatio();
    float sourceUtilization = audioEngine->GetSourcePoolUtilization();
    size_t bufferMemory = audioEngine->GetBufferPoolMemoryUsage();
    int calculations = audioEngine->GetAudio3DCalculationsPerSecond();

    LOG_INFO("Audio Performance Metrics:");
    LOG_INFO("  Buffer pool hit ratio: " + std::to_string(bufferHitRatio * 100) + "%");
    LOG_INFO("  Source pool utilization: " + std::to_string(sourceUtilization * 100) + "%");
    LOG_INFO("  Buffer memory usage: " + std::to_string(bufferMemory / 1024) + " KB");
    LOG_INFO("  3D calculations per second: " + std::to_string(calculations));

    // Optimize if performance is poor
    if (bufferHitRatio < 0.8f) {
        audioEngine->SetBufferPoolSize(audioEngine->GetBufferPoolSize() * 1.5f);
        LOG_INFO("Increased buffer pool size due to low hit ratio");
    }
}
```

### Error Handling and Recovery

```cpp
// Comprehensive error handling
void HandleAudioErrors() {
    if (!audioEngine->IsAudioAvailable()) {
        LOG_WARNING("Audio system not available, running in silent mode");
        return;
    }

    if (!audioEngine->IsOpenALInitialized()) {
        LOG_WARNING("OpenAL not initialized, attempting recovery...");

        if (audioEngine->AttemptAudioRecovery()) {
            LOG_INFO("Audio system recovered successfully");
        } else {
            LOG_ERROR("Audio recovery failed, disabling audio");
            return;
        }
    }

    // Handle device disconnection
    audioEngine->HandleDeviceDisconnection();

    // Check for OpenAL errors
    if (!AudioEngine::CheckOpenALError("Audio operation")) {
        LOG_ERROR("OpenAL error detected: " +
                 AudioEngine::GetOpenALErrorString(alGetError()));
    }
}
```

### Integration with Game Systems

```cpp
// Game application handles character audio (not Character class)
class GameApplication {
private:
    std::unique_ptr<Character> m_character;
    std::unique_ptr<AudioEngine> m_audioEngine;
    uint32_t m_jumpAudioSource = 0;
    uint32_t m_footstepAudioSource = 0;
    std::shared_ptr<AudioClip> m_jumpSound;
    std::shared_ptr<AudioClip> m_footstepSound;

public:
    bool Initialize() {
        // Initialize character without audio dependencies
        m_character = std::make_unique<Character>();
        m_character->Initialize(m_engine.GetPhysics());

        // Initialize audio system separately
        if (m_audioEngine) {
            m_jumpAudioSource = m_audioEngine->CreateAudioSource();
            m_footstepAudioSource = m_audioEngine->CreateAudioSource();

            m_jumpSound = m_audioEngine->LoadAudioClip("assets/audio/jump.wav");
            m_footstepSound = m_audioEngine->LoadAudioClip("assets/audio/footstep.wav");
        }
        return true;
    }

    void Update(float deltaTime) {
        // Update character
        m_character->Update(deltaTime, input, camera);

        // Handle audio feedback based on character state
        if (m_audioEngine && m_character->IsJumping()) {
            // Play jump sound at character position
            m_audioEngine->SetAudioSourcePosition(m_jumpAudioSource, m_character->GetPosition());
            m_audioEngine->PlayAudioSource(m_jumpAudioSource, m_jumpSound);
        }

        // Update footstep audio based on movement
        if (m_audioEngine && m_character->IsGrounded() && m_character->GetVelocity().length() > 0.1f) {
            static float timer = 0.0f;
            timer += deltaTime;

            float interval = 0.6f / (m_character->GetVelocity().length() / m_character->GetMoveSpeed());
            if (timer >= interval) {
                m_audioEngine->SetAudioSourcePosition(m_footstepAudioSource, m_character->GetPosition());
                m_audioEngine->PlayAudioSource(m_footstepAudioSource, m_footstepSound);
                timer = 0.0f;
            }
        }
    }
};
```

## Future Enhancements

### Planned Features

- [ ] MP3 support (if patent-free solution available)
- [ ] Audio streaming for very large files
- [ ] Advanced 3D audio effects (reverb, occlusion)
- [ ] Audio compression options
- [ ] Real-time audio processing
- [ ] Dynamic audio mixing
- [ ] Audio scripting system

### Integration Opportunities

- Physics-based audio occlusion
- Dynamic range compression
- Environmental audio effects
- Procedural audio generation
- Audio-driven visual effects
- Spatial audio zones

## Dependencies

- **OpenAL Soft**: 3D audio processing and hardware acceleration
- **STB Vorbis**: OGG Vorbis decoding (header-only library)
- **Native WAV Parser**: Custom implementation for WAV support

## Compatibility

- **Windows**: Full support with OpenAL Soft
- **Linux**: Full support (planned)
- **macOS**: Full support (planned)
- **Audio Hardware**: Any OpenAL-compatible audio device
