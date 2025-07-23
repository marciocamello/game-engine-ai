# Mock Resource Implementation Guide

## Overview

This guide provides comprehensive patterns and examples for implementing mock resources in Game Engine Kiro's testing framework. Mock resources allow you to test resource-dependent code without requiring actual files, OpenGL context, or other external dependencies.

## Table of Contents

- [Base Mock Resource Pattern](#base-mock-resource-pattern)
- [Specialized Mock Resources](#specialized-mock-resources)
- [ResourceManager Integration](#resourcemanager-integration)
- [Behavior Simulation Techniques](#behavior-simulation-techniques)
- [Testing Patterns](#testing-patterns)
- [Best Practices](#best-practices)

## Related Documentation

**Essential Context:**

- **[Testing Guide](testing-guide.md)**: Comprehensive guide with mock resource usage examples
- **[OpenGL Context Limitations](testing-opengl-limitations.md)**: Why mock resources are essential for testing
- **[Resource Testing Patterns](testing-resource-patterns.md)**: How to use mocks in resource testing

**Implementation Standards:**

- **[Testing Guidelines](testing-guidelines.md)**: Guidelines for mock resource implementation
- **[Testing Standards](testing-standards.md)**: Coding standards for mock resource code
- **[Test Output Formatting](testing-output-formatting.md)**: Proper output formatting in mock tests

**API and Examples:**

- **[API Reference](api-reference.md)**: ResourceManager and Resource base class documentation
- **[Code Examples Validation](testing-code-examples-validation.md)**: Validating mock resource examples

**Advanced Topics:**

- **[Testing Strategy](testing-strategy.md)**: Strategic use of mock resources
- **[Testing Migration](testing-migration.md)**: Migrating to mock resource patterns

## Base Mock Resource Pattern

### MockResource Base Class

All mock resources should inherit from the base `Resource` class and follow this standardized pattern:

```cpp
#include "Resource/ResourceManager.h"
#include "tests/TestUtils.h"

namespace GameEngine {
namespace Testing {

class MockResource : public Resource {
public:
    MockResource(const std::string& path = "mock://resource")
        : Resource(path), m_simulateLoadFailure(false), m_customMemoryUsage(1024) {}

    virtual ~MockResource() = default;

    // Simulation controls
    void SimulateLoadFailure(bool shouldFail) { m_simulateLoadFailure = shouldFail; }
    void SetCustomMemoryUsage(size_t memoryUsage) { m_customMemoryUsage = memoryUsage; }
    void SetLoadDelay(std::chrono::milliseconds delay) { m_loadDelay = delay; }

    // Resource interface implementation
    size_t GetMemoryUsage() const override { return m_customMemoryUsage; }

    // Mock-specific functionality
    bool WasLoadCalled() const { return m_loadCalled; }
    int GetLoadCallCount() const { return m_loadCallCount; }
    void ResetCallTracking() { m_loadCalled = false; m_loadCallCount = 0; }

protected:
    // Simulation state
    bool m_simulateLoadFailure;
    size_t m_customMemoryUsage;
    std::chrono::milliseconds m_loadDelay{0};

    // Call tracking
    mutable bool m_loadCalled = false;
    mutable int m_loadCallCount = 0;

    // Helper method for simulating load behavior
    bool SimulateLoad() const {
        m_loadCalled = true;
        ++m_loadCallCount;

        // Simulate load delay if configured
        if (m_loadDelay.count() > 0) {
            std::this_thread::sleep_for(m_loadDelay);
        }

        return !m_simulateLoadFailure;
    }
};

} // namespace Testing
} // namespace GameEngine
```

### Key Features of Base Mock Pattern

1. **Failure Simulation**: Control whether operations succeed or fail
2. **Memory Usage Control**: Set custom memory usage for testing memory management
3. **Call Tracking**: Monitor how many times methods are called
4. **Timing Simulation**: Add artificial delays to test performance scenarios
5. **Consistent Interface**: Maintains compatibility with real Resource interface

## Specialized Mock Resources

### MockTexture

Mock implementation for graphics texture resources:

```cpp
#include "Graphics/Texture.h"
#include "Graphics/OpenGLContext.h"

namespace GameEngine {
namespace Testing {

class MockTexture : public Texture {
public:
    MockTexture(const std::string& path = "mock://texture.png")
        : Texture(path), m_mockWidth(256), m_mockHeight(256), m_mockChannels(4) {
        // Set default mock properties
        m_format = TextureFormat::RGBA;
    }

    // Override LoadFromFile to avoid actual file I/O
    bool LoadFromFile(const std::string& filepath) override {
        if (!SimulateLoad()) {
            return false;
        }

        // Simulate successful texture loading without OpenGL
        m_width = m_mockWidth;
        m_height = m_mockHeight;
        m_channels = m_mockChannels;
        m_filepath = filepath;

        TestOutput::PrintInfo("MockTexture: Simulated loading " + filepath +
                             " (" + std::to_string(m_width) + "x" + std::to_string(m_height) + ")");
        return true;
    }

    // Override CreateEmpty to avoid OpenGL calls
    bool CreateEmpty(int width, int height, TextureFormat format = TextureFormat::RGBA) override {
        if (!SimulateLoad()) {
            return false;
        }

        m_width = width;
        m_height = height;
        m_format = format;
        m_channels = GetChannelsFromFormat(format);

        TestOutput::PrintInfo("MockTexture: Created empty texture " +
                             std::to_string(width) + "x" + std::to_string(height));
        return true;
    }

    // Override OpenGL-dependent methods to work without context
    void Bind(uint32_t slot = 0) const override {
        if (!OpenGLContext::HasActiveContext()) {
            TestOutput::PrintInfo("MockTexture: Simulated bind to slot " + std::to_string(slot));
            m_lastBoundSlot = slot;
            return;
        }

        // Call parent implementation if OpenGL context is available
        Texture::Bind(slot);
    }

    void Unbind() const override {
        if (!OpenGLContext::HasActiveContext()) {
            TestOutput::PrintInfo("MockTexture: Simulated unbind");
            return;
        }

        Texture::Unbind();
    }

    // Mock-specific functionality
    void SetMockDimensions(int width, int height, int channels = 4) {
        m_mockWidth = width;
        m_mockHeight = height;
        m_mockChannels = channels;
    }

    uint32_t GetLastBoundSlot() const { return m_lastBoundSlot; }

    // Override memory usage calculation
    size_t GetMemoryUsage() const override {
        if (m_customMemoryUsage > 0) {
            return m_customMemoryUsage;
        }

        // Calculate realistic texture memory usage
        return m_width * m_height * m_channels * sizeof(unsigned char);
    }

private:
    int m_mockWidth, m_mockHeight, m_mockChannels;
    mutable uint32_t m_lastBoundSlot = 0;

    int GetChannelsFromFormat(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGB: return 3;
            case TextureFormat::RGBA: return 4;
            case TextureFormat::Depth: return 1;
            case TextureFormat::DepthStencil: return 2;
            default: return 4;
        }
    }
};

} // namespace Testing
} // namespace GameEngine
```

### MockMesh

Mock implementation for 3D mesh resources:

```cpp
#include "Graphics/Mesh.h"
#include "Graphics/OpenGLContext.h"

namespace GameEngine {
namespace Testing {

class MockMesh : public Mesh {
public:
    MockMesh(const std::string& path = "mock://mesh.obj") : Mesh(path) {}

    // Override LoadFromFile to avoid actual file I/O
    bool LoadFromFile(const std::string& filepath) override {
        if (!SimulateLoad()) {
            return false;
        }

        // Create mock mesh data
        CreateMockGeometry();

        TestOutput::PrintInfo("MockMesh: Simulated loading " + filepath +
                             " (" + std::to_string(m_vertices.size()) + " vertices, " +
                             std::to_string(m_indices.size()) + " indices)");
        return true;
    }

    // Override CreateDefault to work without OpenGL
    void CreateDefault() override {
        CreateMockCube();
        TestOutput::PrintInfo("MockMesh: Created default cube mesh");
    }

    // Override OpenGL-dependent methods
    void Bind() const override {
        if (!OpenGLContext::HasActiveContext()) {
            TestOutput::PrintInfo("MockMesh: Simulated bind (VAO would be " + std::to_string(m_VAO) + ")");
            m_bindCallCount++;
            return;
        }

        Mesh::Bind();
    }

    void Unbind() const override {
        if (!OpenGLContext::HasActiveContext()) {
            TestOutput::PrintInfo("MockMesh: Simulated unbind");
            return;
        }

        Mesh::Unbind();
    }

    void Draw() const override {
        if (!OpenGLContext::HasActiveContext()) {
            TestOutput::PrintInfo("MockMesh: Simulated draw (" + std::to_string(m_indices.size()) + " indices)");
            m_drawCallCount++;
            return;
        }

        Mesh::Draw();
    }

    // Mock-specific functionality
    void SetMockVertexCount(size_t count) {
        m_vertices.clear();
        m_vertices.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            Vertex vertex;
            vertex.position = Math::Vec3(static_cast<float>(i), 0.0f, 0.0f);
            vertex.normal = Math::Vec3(0.0f, 1.0f, 0.0f);
            vertex.texCoords = Math::Vec2(0.0f, 0.0f);
            m_vertices.push_back(vertex);
        }
    }

    void SetMockIndexCount(size_t count) {
        m_indices.clear();
        m_indices.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            m_indices.push_back(static_cast<uint32_t>(i % m_vertices.size()));
        }
    }

    int GetBindCallCount() const { return m_bindCallCount; }
    int GetDrawCallCount() const { return m_drawCallCount; }
    void ResetCallCounts() { m_bindCallCount = 0; m_drawCallCount = 0; }

    // Override memory usage calculation
    size_t GetMemoryUsage() const override {
        if (m_customMemoryUsage > 0) {
            return m_customMemoryUsage;
        }

        return (m_vertices.size() * sizeof(Vertex)) + (m_indices.size() * sizeof(uint32_t));
    }

private:
    mutable int m_bindCallCount = 0;
    mutable int m_drawCallCount = 0;

    void CreateMockGeometry() {
        // Create a simple triangle for testing
        m_vertices = {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
            {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {}, {}},
            {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}, {}, {}}
        };

        m_indices = {0, 1, 2};
    }

    void CreateMockCube() {
        // Create a simple cube for testing
        m_vertices = {
            // Front face
            {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
            {{ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {}, {}},
            {{ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {}, {}},
            {{-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {}, {}},
            // Add more vertices for complete cube...
        };

        m_indices = {
            0, 1, 2, 2, 3, 0,  // Front face
            // Add more indices for complete cube...
        };
    }
};

} // namespace Testing
} // namespace GameEngine
```

### MockAudioClip

Mock implementation for audio resources:

```cpp
#include "Audio/AudioEngine.h"

namespace GameEngine {
namespace Testing {

class MockAudioClip : public Resource {
public:
    MockAudioClip(const std::string& path = "mock://audio.wav")
        : Resource(path), m_mockDuration(5.0f), m_mockSampleRate(44100), m_mockChannels(2) {
        // Initialize mock audio properties
        m_format = AudioFormat::WAV;
    }

    // Simulate audio loading
    bool LoadFromFile(const std::string& filepath) {
        if (!SimulateLoad()) {
            return false;
        }

        // Determine format from file extension
        if (filepath.ends_with(".wav")) {
            m_format = AudioFormat::WAV;
        } else if (filepath.ends_with(".ogg")) {
            m_format = AudioFormat::OGG;
        } else if (filepath.ends_with(".mp3")) {
            m_format = AudioFormat::MP3;
        }

        TestOutput::PrintInfo("MockAudioClip: Simulated loading " + filepath +
                             " (duration: " + std::to_string(m_mockDuration) + "s, " +
                             std::to_string(m_mockSampleRate) + "Hz, " +
                             std::to_string(m_mockChannels) + " channels)");
        return true;
    }

    // Mock-specific functionality
    void SetMockProperties(float duration, int sampleRate, int channels, AudioFormat format) {
        m_mockDuration = duration;
        m_mockSampleRate = sampleRate;
        m_mockChannels = channels;
        m_format = format;
    }

    float GetDuration() const { return m_mockDuration; }
    int GetSampleRate() const { return m_mockSampleRate; }
    int GetChannels() const { return m_mockChannels; }
    AudioFormat GetFormat() const { return m_format; }
    bool Is3D() const { return m_is3D; }

    void SetIs3D(bool is3D) { m_is3D = is3D; }

    // Override memory usage calculation
    size_t GetMemoryUsage() const override {
        if (m_customMemoryUsage > 0) {
            return m_customMemoryUsage;
        }

        // Calculate realistic audio memory usage
        // Assume 16-bit samples (2 bytes per sample)
        size_t samplesPerSecond = m_mockSampleRate * m_mockChannels;
        size_t totalSamples = static_cast<size_t>(m_mockDuration * samplesPerSecond);
        return totalSamples * 2; // 2 bytes per 16-bit sample
    }

private:
    float m_mockDuration;
    int m_mockSampleRate;
    int m_mockChannels;
    AudioFormat m_format;
    bool m_is3D = true;
};

} // namespace Testing
} // namespace GameEngine
```

## ResourceManager Integration

### Mock Resource Factory

Create a factory system for generating mock resources:

```cpp
namespace GameEngine {
namespace Testing {

class MockResourceFactory {
public:
    template<typename T>
    static std::shared_ptr<T> CreateMockResource(const std::string& path = "") {
        static_assert(std::is_base_of_v<Resource, T>, "T must inherit from Resource");

        if constexpr (std::is_same_v<T, Texture>) {
            return std::make_shared<MockTexture>(path.empty() ? "mock://texture.png" : path);
        } else if constexpr (std::is_same_v<T, Mesh>) {
            return std::make_shared<MockMesh>(path.empty() ? "mock://mesh.obj" : path);
        } else if constexpr (std::is_same_v<T, MockAudioClip>) {
            return std::make_shared<MockAudioClip>(path.empty() ? "mock://audio.wav" : path);
        } else {
            return std::make_shared<MockResource>(path.empty() ? "mock://resource" : path);
        }
    }

    // Create mock resource with specific properties
    static std::shared_ptr<MockTexture> CreateMockTexture(int width, int height, TextureFormat format = TextureFormat::RGBA) {
        auto texture = std::make_shared<MockTexture>();
        texture->SetMockDimensions(width, height, texture->GetChannelsFromFormat(format));
        texture->CreateEmpty(width, height, format);
        return texture;
    }

    static std::shared_ptr<MockMesh> CreateMockMesh(size_t vertexCount, size_t indexCount) {
        auto mesh = std::make_shared<MockMesh>();
        mesh->SetMockVertexCount(vertexCount);
        mesh->SetMockIndexCount(indexCount);
        return mesh;
    }

    static std::shared_ptr<MockAudioClip> CreateMockAudioClip(float duration, int sampleRate = 44100, int channels = 2) {
        auto audioClip = std::make_shared<MockAudioClip>();
        audioClip->SetMockProperties(duration, sampleRate, channels, AudioFormat::WAV);
        return audioClip;
    }
};

} // namespace Testing
} // namespace GameEngine
```

### Testing ResourceManager with Mocks

Example of how to test ResourceManager functionality using mock resources:

```cpp
namespace GameEngine {
namespace Testing {

bool TestResourceManagerWithMocks() {
    TestOutput::PrintTestStart("ResourceManager with mock resources");

    ResourceManager resourceManager;
    EXPECT_TRUE(resourceManager.Initialize());

    // Test loading mock resources
    auto mockTexture = MockResourceFactory::CreateMockTexture(512, 512);
    auto mockMesh = MockResourceFactory::CreateMockMesh(100, 300);
    auto mockAudio = MockResourceFactory::CreateMockAudioClip(3.5f);

    // Simulate ResourceManager loading these resources
    // (In real implementation, you'd need to modify ResourceManager to accept mock resources)

    // Test memory usage calculation
    size_t expectedMemory = mockTexture->GetMemoryUsage() +
                           mockMesh->GetMemoryUsage() +
                           mockAudio->GetMemoryUsage();

    TestOutput::PrintInfo("Expected total memory usage: " + std::to_string(expectedMemory / 1024) + " KB");

    // Test resource statistics
    ResourceStats stats = resourceManager.GetResourceStats();
    TestOutput::PrintInfo("Resource count: " + std::to_string(stats.totalResources));
    TestOutput::PrintInfo("Memory usage: " + std::to_string(stats.totalMemoryUsage / 1024) + " KB");

    resourceManager.Shutdown();

    TestOutput::PrintTestPass("ResourceManager with mock resources");
    return true;
}

} // namespace Testing
} // namespace GameEngine
```

## Behavior Simulation Techniques

### Failure Simulation

Mock resources can simulate various failure scenarios:

```cpp
bool TestResourceLoadingFailures() {
    TestOutput::PrintTestStart("resource loading failure simulation");

    auto mockTexture = MockResourceFactory::CreateMockResource<MockTexture>();

    // Test normal loading
    EXPECT_TRUE(mockTexture->LoadFromFile("test.png"));
    EXPECT_TRUE(mockTexture->WasLoadCalled());

    // Test load failure simulation
    mockTexture->SimulateLoadFailure(true);
    EXPECT_FALSE(mockTexture->LoadFromFile("test.png"));

    // Reset and test again
    mockTexture->SimulateLoadFailure(false);
    mockTexture->ResetCallTracking();
    EXPECT_TRUE(mockTexture->LoadFromFile("test.png"));
    EXPECT_EQUAL(mockTexture->GetLoadCallCount(), 1);

    TestOutput::PrintTestPass("resource loading failure simulation");
    return true;
}
```

### Performance Simulation

Simulate loading delays and performance characteristics:

```cpp
bool TestResourceLoadingPerformance() {
    TestOutput::PrintTestStart("resource loading performance simulation");

    auto mockMesh = MockResourceFactory::CreateMockResource<MockMesh>();

    // Set artificial load delay
    mockMesh->SetLoadDelay(std::chrono::milliseconds(100));

    TestTimer timer;
    EXPECT_TRUE(mockMesh->LoadFromFile("test.obj"));
    double elapsedMs = timer.ElapsedMs();

    // Verify that the delay was applied
    EXPECT_IN_RANGE(elapsedMs, 95.0, 150.0); // Allow some tolerance

    TestOutput::PrintTiming("Mock mesh loading with 100ms delay", elapsedMs);

    TestOutput::PrintTestPass("resource loading performance simulation");
    return true;
}
```

### Memory Pressure Simulation

Test memory management under different conditions:

```cpp
bool TestMemoryPressureSimulation() {
    TestOutput::PrintTestStart("memory pressure simulation");

    ResourceManager resourceManager;
    resourceManager.Initialize();

    // Create resources with different memory footprints
    auto smallTexture = MockResourceFactory::CreateMockTexture(64, 64);
    auto largeTexture = MockResourceFactory::CreateMockTexture(2048, 2048);
    auto hugeMesh = MockResourceFactory::CreateMockMesh(100000, 300000);

    smallTexture->SetCustomMemoryUsage(64 * 64 * 4);      // 16 KB
    largeTexture->SetCustomMemoryUsage(2048 * 2048 * 4);  // 16 MB
    hugeMesh->SetCustomMemoryUsage(100 * 1024 * 1024);    // 100 MB

    // Test memory pressure threshold
    resourceManager.SetMemoryPressureThreshold(50 * 1024 * 1024); // 50 MB

    // Simulate loading resources and triggering memory pressure
    size_t totalMemory = smallTexture->GetMemoryUsage() +
                        largeTexture->GetMemoryUsage() +
                        hugeMesh->GetMemoryUsage();

    TestOutput::PrintInfo("Total simulated memory usage: " + std::to_string(totalMemory / 1024 / 1024) + " MB");

    resourceManager.Shutdown();

    TestOutput::PrintTestPass("memory pressure simulation");
    return true;
}
```

## Testing Patterns

### Context-Aware Testing

Test behavior with and without OpenGL context:

```cpp
bool TestContextAwareResourceBehavior() {
    TestOutput::PrintTestStart("context-aware resource behavior");

    auto mockTexture = MockResourceFactory::CreateMockTexture(256, 256);

    if (OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Testing with OpenGL context available");

        // Test with real OpenGL calls
        mockTexture->Bind(0);
        EXPECT_TRUE(mockTexture->IsValid());
        mockTexture->Unbind();

    } else {
        TestOutput::PrintInfo("Testing without OpenGL context (headless mode)");

        // Test mock behavior without OpenGL
        mockTexture->Bind(0);
        EXPECT_EQUAL(mockTexture->GetLastBoundSlot(), 0);
        mockTexture->Unbind();
    }

    TestOutput::PrintTestPass("context-aware resource behavior");
    return true;
}
```

### Integration Testing with Mocks

Test how mock resources integrate with other engine systems:

```cpp
bool TestMockResourceIntegration() {
    TestOutput::PrintTestStart("mock resource integration");

    // Create mock resources
    auto mockTexture = MockResourceFactory::CreateMockTexture(512, 512);
    auto mockMesh = MockResourceFactory::CreateMockMesh(1000, 3000);

    // Test integration with rendering system (without actual rendering)
    mockMesh->Bind();
    mockTexture->Bind(0);

    // Simulate rendering operations
    mockMesh->Draw();

    // Verify mock behavior
    EXPECT_EQUAL(mockMesh->GetBindCallCount(), 1);
    EXPECT_EQUAL(mockMesh->GetDrawCallCount(), 1);
    EXPECT_EQUAL(mockTexture->GetLastBoundSlot(), 0);

    mockTexture->Unbind();
    mockMesh->Unbind();

    TestOutput::PrintTestPass("mock resource integration");
    return true;
}
```

## Best Practices

### 1. Consistent Mock Behavior

- Always provide realistic default values
- Maintain the same interface as real resources
- Document any behavioral differences from real resources

### 2. Comprehensive Call Tracking

- Track all important method calls
- Provide reset functionality for call counters
- Include timing information when relevant

### 3. Failure Simulation

- Support multiple failure modes (load failure, memory allocation failure, etc.)
- Make failure simulation easily configurable
- Provide clear feedback about simulated failures

### 4. Memory Management

- Calculate realistic memory usage estimates
- Support custom memory usage for testing edge cases
- Integrate properly with ResourceManager memory tracking

### 5. Context Awareness

- Always check for OpenGL context availability
- Provide meaningful mock behavior when context is unavailable
- Fall back to real implementation when context is available

### 6. Performance Testing

- Support artificial delays for performance testing
- Provide timing information for operations
- Allow configuration of performance characteristics

### 7. Documentation and Examples

- Document all mock-specific functionality
- Provide usage examples for common scenarios
- Explain differences from real resource behavior

## Example Test Suite

Here's a complete example of how to structure a test suite using mock resources:

```cpp
#include "tests/TestUtils.h"
#include "testing-mock-resources.h" // Include mock resource definitions

namespace GameEngine {
namespace Testing {

bool RunMockResourceTests() {
    TestOutput::PrintHeader("Mock Resource Tests");

    TestSuite suite("Mock Resources");
    bool allPassed = true;

    // Basic mock functionality tests
    allPassed &= suite.RunTest("Basic Mock Resource", TestBasicMockResource);
    allPassed &= suite.RunTest("Mock Texture", TestMockTexture);
    allPassed &= suite.RunTest("Mock Mesh", TestMockMesh);
    allPassed &= suite.RunTest("Mock Audio Clip", TestMockAudioClip);

    // Integration tests
    allPassed &= suite.RunTest("ResourceManager Integration", TestResourceManagerWithMocks);
    allPassed &= suite.RunTest("Context-Aware Behavior", TestContextAwareResourceBehavior);
    allPassed &= suite.RunTest("Mock Resource Integration", TestMockResourceIntegration);

    // Simulation tests
    allPassed &= suite.RunTest("Failure Simulation", TestResourceLoadingFailures);
    allPassed &= suite.RunTest("Performance Simulation", TestResourceLoadingPerformance);
    allPassed &= suite.RunTest("Memory Pressure Simulation", TestMemoryPressureSimulation);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);

    return allPassed;
}

} // namespace Testing
} // namespace GameEngine

// Main function for running mock resource tests
int main() {
    return GameEngine::Testing::RunMockResourceTests() ? 0 : 1;
}
```

This comprehensive guide provides all the patterns and examples needed to implement effective mock resources for testing Game Engine Kiro's resource management system without requiring actual files, OpenGL context, or other external dependencies.
