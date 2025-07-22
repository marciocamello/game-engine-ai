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
audioEngine.Initialize();

// Load audio clips
auto clip = audioEngine.LoadAudioClip("assets/audio/sound.wav");
auto music = audioEngine.LoadAudioClip("assets/audio/music.ogg");

// Create and control audio sources
uint32_t sourceId = audioEngine.CreateAudioSource();
audioEngine.PlayAudioSource(sourceId, clip);
audioEngine.SetAudioSourcePosition(sourceId, Math::Vec3(10.0f, 0.0f, 5.0f));
audioEngine.SetAudioSourceVolume(sourceId, 0.8f);
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

## Future Enhancements

### Planned Features

- [ ] MP3 support (if patent-free solution available)
- [ ] Audio streaming for very large files
- [ ] Advanced 3D audio effects (reverb, occlusion)
- [ ] Audio compression options
- [ ] Real-time audio processing

### Integration Opportunities

- Physics-based audio occlusion
- Dynamic range compression
- Environmental audio effects
- Procedural audio generation

## Dependencies

- **OpenAL Soft**: 3D audio processing and hardware acceleration
- **STB Vorbis**: OGG Vorbis decoding (header-only library)
- **Native WAV Parser**: Custom implementation for WAV support

## Compatibility

- **Windows**: Full support with OpenAL Soft
- **Linux**: Full support (planned)
- **macOS**: Full support (planned)
- **Audio Hardware**: Any OpenAL-compatible audio device
