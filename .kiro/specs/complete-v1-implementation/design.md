# Design Document - Complete v1.0 Implementation

## Overview

This design document outlines the implementation of the missing core systems needed to complete Game Engine Kiro v1.0: a fully functional OpenAL-based audio system and a comprehensive resource management system with real file loading capabilities.

## Architecture

### System Integration Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        Game Layer                           │
├─────────────────────────────────────────────────────────────┤
│  Character  │  Camera   │  Game Logic  │  Scene Management  │
├─────────────────────────────────────────────────────────────┤
│                      Engine Core                            │
├─────────────────────────────────────────────────────────────┤
│ Graphics │ Physics │ [Audio] │ Input │ [Resource] │ Script  │
├─────────────────────────────────────────────────────────────┤
│                    Foundation Layer                         │
├─────────────────────────────────────────────────────────────┤
│   Math   │  Logger  │ Memory │ Time  │  Events │   Utils    │
├─────────────────────────────────────────────────────────────┤
│                   Platform Layer                            │
└─────────────────────────────────────────────────────────────┘
│  OpenGL  │  GLFW   │[OpenAL]│ Bullet │  vcpkg  │    OS      │
└─────────────────────────────────────────────────────────────┘
```

_[Audio] and [Resource] systems will be completed in this implementation_

## Components and Interfaces

### 1. OpenAL Audio System

#### Core Components

**AudioEngine (Enhanced)**

```cpp
class AudioEngine {
private:
    // OpenAL Context Management
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;

    // Resource Management
    std::unordered_map<std::string, ALuint> m_audioBuffers;
    std::unordered_map<uint32_t, ALuint> m_audioSources;

    // Audio Loading
    std::unique_ptr<AudioLoader> m_audioLoader;

public:
    bool InitializeOpenAL();
    void ShutdownOpenAL();
    ALuint LoadAudioBuffer(const std::string& filepath);
    void UpdateListener(const Math::Vec3& position, const Math::Vec3& forward, const Math::Vec3& up);
};
```

**AudioLoader (New Component)**

```cpp
class AudioLoader {
public:
    struct AudioData {
        std::vector<char> data;
        ALenum format;
        ALsizei frequency;
        bool isValid = false;
    };

    AudioData LoadWAV(const std::string& filepath);
    AudioData LoadOGG(const std::string& filepath);

private:
    AudioData LoadWAVImpl(const std::string& filepath);
    AudioData LoadOGGImpl(const std::string& filepath);
};
```

**AudioSource (Enhanced)**

```cpp
class AudioSource {
private:
    ALuint m_sourceId = 0;
    uint32_t m_engineId;

public:
    void PlayBuffer(ALuint buffer);
    void SetPosition3D(const Math::Vec3& position);
    void SetVelocity3D(const Math::Vec3& velocity);
    void SetGain(float gain);
    void SetPitch(float pitch);
    void SetLooping(bool loop);

    bool IsPlaying() const;
    ALenum GetState() const;
};
```

#### OpenAL Integration Strategy

1. **Initialization Sequence**

   - Enumerate available audio devices
   - Create OpenAL device and context
   - Set up default listener properties
   - Initialize audio loader subsystem

2. **Audio Loading Pipeline**

   - File format detection (WAV/OGG)
   - Audio decoding using appropriate decoder
   - OpenAL buffer creation and data upload
   - Buffer caching and management

3. **3D Audio Processing**
   - Listener position/orientation updates
   - Source position/velocity updates
   - Distance attenuation calculations
   - Doppler effect processing

### 2. Resource Management System

#### Core Components

**ResourceManager (Enhanced)**

```cpp
template<typename T>
class ResourceManager {
private:
    std::unordered_map<std::string, std::weak_ptr<T>> m_resources;
    std::unique_ptr<ResourceLoader<T>> m_loader;

public:
    std::shared_ptr<T> Load(const std::string& filepath);
    void Unload(const std::string& filepath);
    void UnloadUnused();

    size_t GetMemoryUsage() const;
    size_t GetResourceCount() const;
};
```

**ResourceLoader (New Template System)**

```cpp
template<typename T>
class ResourceLoader {
public:
    virtual std::shared_ptr<T> LoadFromFile(const std::string& filepath) = 0;
    virtual std::shared_ptr<T> CreateDefault() = 0;
};

// Specialized loaders
class TextureLoader : public ResourceLoader<Texture> { ... };
class MeshLoader : public ResourceLoader<Mesh> { ... };
class AudioClipLoader : public ResourceLoader<AudioClip> { ... };
```

**Resource Types (Enhanced)**

```cpp
class Texture : public Resource {
private:
    GLuint m_textureId = 0;
    int m_width, m_height, m_channels;

public:
    bool LoadFromFile(const std::string& filepath);
    void CreateDefault(); // Pink/magenta texture
    GLuint GetTextureId() const { return m_textureId; }
};

class Mesh : public Resource {
private:
    GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
    size_t m_vertexCount = 0, m_indexCount = 0;

public:
    bool LoadFromFile(const std::string& filepath);
    void CreateDefault(); // Default cube
    void Render() const;
};

class AudioClip : public Resource {
private:
    ALuint m_bufferId = 0;
    float m_duration = 0.0f;

public:
    bool LoadFromFile(const std::string& filepath);
    ALuint GetBufferId() const { return m_bufferId; }
};
```

#### File Loading Strategy

1. **Image Loading (STB Integration)**

   - Support for PNG, JPG, TGA formats
   - Automatic format detection
   - Proper color channel handling
   - GPU texture upload optimization

2. **Audio Loading (Custom + Libraries)**

   - WAV loading using custom parser
   - OGG loading using stb_vorbis
   - Format validation and error handling
   - Memory-efficient streaming for large files

3. **Mesh Loading (Basic OBJ)**
   - Simple OBJ file parser
   - Vertex/normal/texture coordinate extraction
   - Index buffer generation
   - OpenGL buffer creation

## Data Models

### Audio System Data Flow

```
Audio File → AudioLoader → OpenAL Buffer → AudioSource → OpenAL Source → Hardware
     ↓              ↓            ↓             ↓             ↓
   Format      Decode/Parse   GPU Upload   3D Properties  Audio Output
  Detection    Audio Data     to OpenAL    Position/Vol   to Speakers
```

### Resource Loading Data Flow

```
File Request → ResourceManager → ResourceLoader → Resource Object → GPU/Memory
      ↓              ↓               ↓               ↓              ↓
   Cache Check   Load/Create     File Parsing   Object Creation  Resource Ready
   Hit/Miss      Decision        & Validation    & GPU Upload     for Use
```

### Memory Management Strategy

```cpp
// Weak reference caching prevents memory leaks
std::unordered_map<std::string, std::weak_ptr<Resource>> m_cache;

// Automatic cleanup when no strong references exist
void ResourceManager::CleanupUnused() {
    for (auto it = m_cache.begin(); it != m_cache.end();) {
        if (it->second.expired()) {
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }
}
```

## Error Handling

### Audio System Error Handling

1. **OpenAL Initialization Failures**

   - Device enumeration fallbacks
   - Context creation error recovery
   - Graceful degradation to no-audio mode

2. **Audio Loading Failures**

   - File format validation
   - Corrupted file detection
   - Default silence buffer fallback

3. **Runtime Audio Errors**
   - Source exhaustion handling
   - Buffer underrun recovery
   - Device disconnection handling

### Resource System Error Handling

1. **File Loading Failures**

   - File not found handling
   - Permission error recovery
   - Corrupted file detection

2. **Memory Allocation Failures**

   - GPU memory exhaustion handling
   - System memory pressure response
   - Resource priority management

3. **Format Support Errors**
   - Unsupported format detection
   - Format conversion fallbacks
   - Default resource provision

## Testing Strategy

### Test Implementation Standards

All tests MUST follow the established Game Engine Kiro testing patterns:

1. **Test Structure Requirements**

   - Use `TestUtils.h` for standardized test utilities
   - Include `Core/Logger.h` and initialize logger in main()
   - Use `TestOutput` class for consistent formatting
   - Use `TestSuite` class for result tracking
   - Follow the established assertion macros (EXPECT_TRUE, EXPECT_EQUAL, etc.)

2. **Test Output Format**

   - Use `TestOutput::PrintHeader()` for test suite headers
   - Use `TestOutput::PrintTestStart()` for individual test starts
   - Use `TestOutput::PrintTestPass()` and `TestOutput::PrintTestFail()` for results
   - Use `TestOutput::PrintFooter()` for final results

3. **Build and Execution**
   - Build scripts are located in `/scripts/` directory
   - Manual build: `cmake --build build --config Release --target <TestName>`
   - All tests should be terminable and not require user interaction

### Unit Testing

1. **Audio System Tests**

   - OpenAL initialization/shutdown using TestSuite pattern
   - Audio file loading (WAV/OGG) with proper error handling tests
   - 3D positioning accuracy using EXPECT_NEAR_VEC3 macros
   - Memory leak detection using MemoryTest utilities

2. **Resource System Tests**
   - File loading for each format with TestOutput formatting
   - Cache hit/miss behavior validation
   - Memory usage tracking with detailed logging
   - Error condition handling with proper test assertions

### Integration Testing

1. **Engine Integration**

   - Audio system with character movement using established test patterns
   - Resource loading with graphics rendering
   - Performance under load using PerformanceTest utilities
   - Memory usage over time with TestTimer measurements

2. **Cross-System Testing**
   - Audio resources through resource manager
   - Texture loading with OpenGL renderer
   - Mesh loading with primitive renderer

### Performance Testing

1. **Audio Performance**

   - Multiple simultaneous sources using PerformanceTest::ValidatePerformance
   - 3D audio calculation overhead measurements
   - Memory usage with large audio files

2. **Resource Performance**
   - Large texture loading times with TestTimer
   - Cache efficiency measurements
   - Memory fragmentation analysis

### Test Example Template

```cpp
#include "Audio/AudioEngine.h"
#include "Core/Logger.h"
#include "../TestUtils.h"

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestAudioLoaderWAV() {
    TestOutput::PrintTestStart("WAV file loading");

    AudioLoader loader;
    AudioData data = loader.LoadWAV("test.wav");

    EXPECT_TRUE(data.isValid);
    EXPECT_EQUAL(data.sampleRate, 44100);
    EXPECT_EQUAL(data.channels, 2);

    TestOutput::PrintTestPass("WAV file loading");
    return true;
}

int main() {
    TestOutput::PrintHeader("Audio Loader");
    Logger::GetInstance().Initialize();

    TestSuite suite("Audio Loader Tests");
    bool allPassed = suite.RunTest("WAV Loading", TestAudioLoaderWAV);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    return allPassed ? 0 : 1;
}
```

## Implementation Phases

### Phase 1: OpenAL Audio Foundation

- OpenAL context initialization
- Basic WAV file loading
- Simple audio source playback
- 3D positioning basics

### Phase 2: Audio System Completion

- OGG file support
- Advanced 3D audio features
- Error handling and robustness
- Integration with existing engine

### Phase 3: Resource System Foundation

- Enhanced resource manager architecture
- Texture loading with STB
- Basic mesh loading (OBJ)
- Memory management improvements

### Phase 4: Resource System Completion

- Audio clip integration
- Advanced caching strategies
- Performance optimizations
- Comprehensive error handling

### Phase 5: Integration and Polish

- Full engine integration
- Performance testing and optimization
- Documentation updates
- Example usage implementation

## Dependencies

### New Dependencies

- **OpenAL**: 3D audio library
- **STB**: Image loading (stb_image)
- **STB**: Audio loading (stb_vorbis)

### Existing Dependencies (Enhanced Usage)

- **OpenGL**: Texture and mesh GPU resources
- **GLM**: Math operations for 3D audio
- **vcpkg**: Dependency management

## Performance Considerations

### Audio System

- Buffer pooling for frequently used sounds
- Streaming for large audio files
- Efficient 3D audio calculations
- Hardware acceleration utilization

### Resource System

- Asynchronous loading capabilities
- Memory-mapped file access
- GPU upload optimization
- Cache-friendly data structures

## Future Extensibility

### Audio System Extensions

- Audio streaming for music
- Audio effects and filters
- Multi-channel audio support
- Audio compression support

### Resource System Extensions

- Asynchronous loading
- Asset pipeline integration
- Hot-reloading capabilities
- Advanced caching strategies

This design provides a solid foundation for completing Game Engine Kiro v1.0 with fully functional audio and resource management systems that integrate seamlessly with the existing engine architecture.
