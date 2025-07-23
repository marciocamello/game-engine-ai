# OpenGL Context Limitations in Testing - Game Engine Kiro

## Table of Contents

1. [Overview](#overview)
2. [The Problem](#the-problem)
3. [Context Detection](#context-detection)
4. [Testing Strategies](#testing-strategies)
5. [Implementation Patterns](#implementation-patterns)
6. [CI/CD Integration](#cicd-integration)
7. [Best Practices](#best-practices)
8. [Common Pitfalls](#common-pitfalls)
9. [Troubleshooting](#troubleshooting)

## Overview

Game Engine Kiro's testing system must handle the reality that many tests run in environments without an active OpenGL context. This document provides comprehensive guidance on understanding, detecting, and working around OpenGL context limitations in testing environments.

### Why This Matters

- **Headless Testing**: CI/CD systems and automated testing often run without display servers
- **Unit Testing**: Pure logic tests shouldn't require graphics initialization
- **Development Workflow**: Developers need fast, reliable tests that don't depend on graphics hardware
- **Cross-Platform Compatibility**: Different platforms have varying OpenGL context requirements

## The Problem

### What Happens Without OpenGL Context

When OpenGL functions are called without an active context, several issues occur:

```cpp
// ❌ These calls will CRASH without OpenGL context:
glGenTextures(1, &textureID);           // Segmentation fault
glGenVertexArrays(1, &vaoID);           // Access violation
glCreateShader(GL_VERTEX_SHADER);       // Undefined behavior
glGetString(GL_VERSION);                // May return null or crash
```

### Common Crash Scenarios

1. **Texture Creation**: Any `glGen*` or `glCreate*` calls
2. **Buffer Operations**: Vertex arrays, uniform buffers, framebuffers
3. **Shader Compilation**: Creating and compiling shaders
4. **State Queries**: Getting OpenGL version, extensions, or limits
5. **Resource Cleanup**: Deleting OpenGL objects

### Testing Environment Challenges

```
Headless CI/CD Environment:
┌─────────────────────────────┐
│ No Display Server          │
│ No Window Manager          │
│ No Graphics Hardware       │
│ No OpenGL Context          │
└─────────────────────────────┘
                │
                ▼
        Tests Must Still Pass!
```

## Context Detection

### Using OpenGLContext Utility

Game Engine Kiro provides the `OpenGLContext` utility class for safe context detection:

```cpp
#include "Graphics/OpenGLContext.h"

bool TestGraphicsFeature() {
    TestOutput::PrintTestStart("graphics feature");

    // Always check context before OpenGL operations
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL-dependent test (no context)");
        TestOutput::PrintTestPass("graphics feature");
        return true; // Pass the test gracefully
    }

    // Safe to use OpenGL functions here
    GLuint textureID;
    glGenTextures(1, &textureID);

    TestOutput::PrintTestPass("graphics feature");
    return true;
}
```

### Context Detection Methods

#### Method 1: HasActiveContext()

```cpp
// Recommended approach - uses engine utility
if (OpenGLContext::HasActiveContext()) {
    // OpenGL operations are safe
    performOpenGLOperations();
} else {
    // Use fallback or skip test
    TestOutput::PrintInfo("No OpenGL context - using fallback");
}
```

#### Method 2: IsReady()

```cpp
// More comprehensive check - includes GLAD initialization
if (OpenGLContext::IsReady()) {
    // OpenGL is fully initialized and ready
    performComplexOpenGLOperations();
} else {
    // OpenGL not ready or context missing
    useAlternativeApproach();
}
```

#### Method 3: Version String Check

```cpp
// Get context information if available
const char* version = OpenGLContext::GetVersionString();
if (strlen(version) > 0) {
    TestOutput::PrintInfo("OpenGL Version: " + std::string(version));
    // Context is available
} else {
    TestOutput::PrintInfo("No OpenGL context available");
    // No context
}
```

## Testing Strategies

### Strategy 1: Mock Resources

Use mock implementations that don't require OpenGL context:

```cpp
// Mock resource for testing resource management logic
class MockTexture : public Resource {
public:
    MockTexture(const std::string& path)
        : Resource(path), m_width(256), m_height(256), m_format(GL_RGBA) {}

    // Simulate texture properties without OpenGL
    size_t GetMemoryUsage() const override {
        return Resource::GetMemoryUsage() + (m_width * m_height * 4);
    }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    GLenum GetFormat() const { return m_format; }

    // Simulate loading without OpenGL calls
    bool Load() override {
        if (GetPath().empty()) return false;
        m_isLoaded = true;
        return true;
    }

private:
    int m_width, m_height;
    GLenum m_format;
};

bool TestTextureManagement() {
    TestOutput::PrintTestStart("texture management with mocks");

    ResourceManager manager;

    // Use mock texture - no OpenGL context required
    auto texture = manager.Load<MockTexture>("test.png");
    EXPECT_NOT_NULL(texture);
    EXPECT_TRUE(texture->GetWidth() > 0);
    EXPECT_TRUE(texture->GetMemoryUsage() > 0);

    TestOutput::PrintTestPass("texture management with mocks");
    return true;
}
```

### Strategy 2: Context-Aware Resource Classes

Design resource classes to handle missing context gracefully:

```cpp
class Texture : public Resource {
public:
    bool Load() override {
        // Load image data first (CPU operation)
        if (!LoadImageData()) {
            return false;
        }

        // Only create OpenGL texture if context is available
        if (OpenGLContext::HasActiveContext()) {
            return CreateOpenGLTexture();
        } else {
            // Mark as loaded but without GPU resource
            m_isLoaded = true;
            m_hasGPUResource = false;
            LOG_INFO("Texture loaded without OpenGL context: " + GetPath());
            return true;
        }
    }

    GLuint GetTextureID() const {
        if (!m_hasGPUResource) {
            LOG_WARNING("Texture has no GPU resource (no OpenGL context)");
            return 0;
        }
        return m_textureID;
    }

private:
    bool LoadImageData() {
        // CPU-only image loading (STB, etc.)
        // This works without OpenGL context
        return true;
    }

    bool CreateOpenGLTexture() {
        // OpenGL texture creation
        glGenTextures(1, &m_textureID);
        // ... texture setup ...
        m_hasGPUResource = true;
        return true;
    }

    GLuint m_textureID = 0;
    bool m_hasGPUResource = false;
};
```

### Strategy 3: Separate Data and GPU Tests

Split tests into data processing and GPU resource creation:

```cpp
// Test 1: Data loading (no OpenGL required)
bool TestMeshDataLoading() {
    TestOutput::PrintTestStart("mesh data loading");

    // Test CPU-side mesh data processing
    MeshLoader::MeshData data = MeshLoader::LoadOBJ("cube.obj");

    EXPECT_TRUE(data.isValid);
    EXPECT_TRUE(data.vertices.size() > 0);
    EXPECT_TRUE(data.indices.size() > 0);

    // Validate mesh data structure
    for (const auto& vertex : data.vertices) {
        EXPECT_TRUE(vertex.position.x >= -10.0f && vertex.position.x <= 10.0f);
    }

    TestOutput::PrintTestPass("mesh data loading");
    return true;
}

// Test 2: GPU resource creation (OpenGL required)
bool TestMeshGPUCreation() {
    TestOutput::PrintTestStart("mesh GPU creation");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping GPU test (no OpenGL context)");
        TestOutput::PrintTestPass("mesh GPU creation");
        return true;
    }

    // Test OpenGL mesh creation
    Mesh mesh;
    bool loaded = mesh.LoadFromFile("cube.obj");

    EXPECT_TRUE(loaded);
    EXPECT_TRUE(mesh.GetVAO() != 0);
    EXPECT_TRUE(mesh.GetVertexCount() > 0);

    TestOutput::PrintTestPass("mesh GPU creation");
    return true;
}
```

### Strategy 4: Conditional Test Execution

Use preprocessor directives and runtime checks for conditional testing:

```cpp
bool TestGraphicsIntegration() {
    TestOutput::PrintTestStart("graphics integration");

#ifdef GAMEENGINE_HEADLESS_TESTING
    TestOutput::PrintInfo("Headless testing mode - skipping graphics tests");
    TestOutput::PrintTestPass("graphics integration");
    return true;
#endif

    // Runtime context check
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("No OpenGL context - testing fallback behavior");

        // Test that systems handle missing context gracefully
        GraphicsRenderer renderer;
        bool initialized = renderer.Initialize();

        // Should fail gracefully, not crash
        EXPECT_FALSE(initialized);

        TestOutput::PrintTestPass("graphics integration");
        return true;
    }

    // Full graphics testing with context
    return TestFullGraphicsIntegration();
}
```

## Implementation Patterns

### Pattern 1: Safe Resource Initialization

```cpp
class SafeTexture {
public:
    SafeTexture() : m_textureID(0), m_isValid(false) {}

    bool Initialize(const std::string& path) {
        // Always load image data first
        if (!LoadImageData(path)) {
            return false;
        }

        // Create GPU resource only if context is available
        if (OpenGLContext::HasActiveContext()) {
            return CreateGPUResource();
        } else {
            // Valid texture without GPU resource
            m_isValid = true;
            return true;
        }
    }

    bool IsValid() const { return m_isValid; }
    bool HasGPUResource() const { return m_textureID != 0; }

    // Safe getter that doesn't crash
    GLuint GetTextureID() const {
        if (!HasGPUResource()) {
            LOG_WARNING("Texture has no GPU resource");
            return 0;
        }
        return m_textureID;
    }

private:
    bool LoadImageData(const std::string& path) {
        // CPU-only image loading
        m_imageData = LoadImageFromFile(path);
        return !m_imageData.empty();
    }

    bool CreateGPUResource() {
        glGenTextures(1, &m_textureID);
        // ... OpenGL texture setup ...
        return m_textureID != 0;
    }

    GLuint m_textureID;
    bool m_isValid;
    std::vector<uint8_t> m_imageData;
};
```

### Pattern 2: Context-Aware Test Fixtures

```cpp
class GraphicsTestFixture {
public:
    bool SetUp() {
        m_hasContext = OpenGLContext::HasActiveContext();

        if (m_hasContext) {
            TestOutput::PrintInfo("OpenGL context available - full testing enabled");
            return SetUpWithContext();
        } else {
            TestOutput::PrintInfo("No OpenGL context - mock testing enabled");
            return SetUpWithoutContext();
        }
    }

    void TearDown() {
        if (m_hasContext) {
            TearDownWithContext();
        } else {
            TearDownWithoutContext();
        }
    }

    bool HasContext() const { return m_hasContext; }

private:
    bool SetUpWithContext() {
        // Initialize real graphics resources
        return true;
    }

    bool SetUpWithoutContext() {
        // Initialize mock resources
        return true;
    }

    void TearDownWithContext() {
        // Clean up OpenGL resources
    }

    void TearDownWithoutContext() {
        // Clean up mock resources
    }

    bool m_hasContext;
};
```

### Pattern 3: Fallback Resource Creation

```cpp
template<typename ResourceType>
std::shared_ptr<ResourceType> CreateTestResource(const std::string& path) {
    if (OpenGLContext::HasActiveContext()) {
        // Create real resource
        auto resource = std::make_shared<ResourceType>();
        if (resource->LoadFromFile(path)) {
            return resource;
        }
    }

    // Fallback to mock resource
    auto mockResource = std::make_shared<MockResource>(path);
    mockResource->Load();
    return std::static_pointer_cast<ResourceType>(mockResource);
}

bool TestResourceLoading() {
    TestOutput::PrintTestStart("resource loading with fallback");

    auto texture = CreateTestResource<Texture>("test.png");
    EXPECT_NOT_NULL(texture);
    EXPECT_TRUE(texture->IsLoaded());

    TestOutput::PrintTestPass("resource loading with fallback");
    return true;
}
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Game Engine Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v3

      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake build-essential
          # Note: No OpenGL/graphics dependencies for headless testing

      - name: Build Engine
        run: |
          mkdir build && cd build
          cmake .. -DGAMEENGINE_HEADLESS_TESTING=ON
          make -j$(nproc)

      - name: Run Unit Tests
        run: |
          cd build
          # Run tests that don't require OpenGL context
          ./MathTest
          ./LoggerTest
          ./ResourceManagerTest

      - name: Run Integration Tests
        run: |
          cd build
          # Integration tests with context detection
          ./PhysicsIntegrationTest
          ./AudioIntegrationTest
```

### Jenkins Pipeline Example

```groovy
pipeline {
    agent any

    environment {
        GAMEENGINE_HEADLESS_TESTING = 'true'
        DISPLAY = ':99' // Virtual display for X11 if needed
    }

    stages {
        stage('Build') {
            steps {
                sh '''
                    mkdir -p build
                    cd build
                    cmake .. -DCMAKE_BUILD_TYPE=Release
                    make -j$(nproc)
                '''
            }
        }

        stage('Test') {
            parallel {
                stage('Unit Tests') {
                    steps {
                        sh '''
                            cd build
                            for test in *Test; do
                                echo "Running $test..."
                                ./$test || exit 1
                            done
                        '''
                    }
                }

                stage('Integration Tests') {
                    steps {
                        sh '''
                            cd build
                            # Tests handle missing OpenGL context gracefully
                            ./BulletIntegrationTest
                            ./ResourceStatisticsTest
                        '''
                    }
                }
            }
        }
    }

    post {
        always {
            // Collect test results
            archiveArtifacts artifacts: 'build/*.log', allowEmptyArchive: true
        }
    }
}
```

### Docker Testing Environment

```dockerfile
# Dockerfile for headless testing
FROM ubuntu:22.04

# Install build dependencies (no graphics)
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    libglfw3-dev \
    libopenal-dev \
    && rm -rf /var/lib/apt/lists/*

# Note: No OpenGL/X11 packages needed for headless testing

WORKDIR /app
COPY . .

# Build with headless testing enabled
RUN mkdir build && cd build && \
    cmake .. -DGAMEENGINE_HEADLESS_TESTING=ON && \
    make -j$(nproc)

# Run tests
CMD ["sh", "-c", "cd build && ./run_all_tests.sh"]
```

### CMake Configuration for Headless Testing

```cmake
# CMakeLists.txt additions for headless testing
option(GAMEENGINE_HEADLESS_TESTING "Enable headless testing mode" OFF)

if(GAMEENGINE_HEADLESS_TESTING)
    add_definitions(-DGAMEENGINE_HEADLESS_TESTING)
    message(STATUS "Headless testing mode enabled")
endif()

# Test configuration
if(GAMEENGINE_HEADLESS_TESTING)
    # Skip tests that require OpenGL context
    set(SKIP_GRAPHICS_TESTS ON)
else()
    set(SKIP_GRAPHICS_TESTS OFF)
endif()
```

## Best Practices

### 1. Always Check Context First

```cpp
// ✅ GOOD - Always check context before OpenGL calls
bool TestGraphicsFeature() {
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Skipping OpenGL test (no context)");
        return true;
    }

    // OpenGL operations here
    return true;
}

// ❌ BAD - Assumes OpenGL context exists
bool TestGraphicsFeature() {
    GLuint texture;
    glGenTextures(1, &texture); // May crash!
    return true;
}
```

### 2. Design Context-Agnostic APIs

```cpp
// ✅ GOOD - API works with or without context
class Texture {
public:
    bool LoadFromFile(const std::string& path) {
        // Always load image data
        if (!LoadImageData(path)) return false;

        // Create GPU resource if possible
        if (OpenGLContext::HasActiveContext()) {
            CreateGPUTexture();
        }

        return true;
    }

    bool IsLoaded() const { return m_imageData.size() > 0; }
    bool HasGPUResource() const { return m_textureID != 0; }
};

// ❌ BAD - API requires OpenGL context
class Texture {
public:
    bool LoadFromFile(const std::string& path) {
        // Crashes without context
        glGenTextures(1, &m_textureID);
        return LoadImageAndUpload(path);
    }
};
```

### 3. Use Meaningful Test Names

```cpp
// ✅ GOOD - Clear about context requirements
bool TestTextureLoadingWithoutContext() { /* ... */ }
bool TestTextureGPUCreationWithContext() { /* ... */ }
bool TestTextureFallbackBehavior() { /* ... */ }

// ❌ BAD - Unclear about context requirements
bool TestTextureLoading() { /* ... */ }
bool TestTexture() { /* ... */ }
```

### 4. Provide Informative Messages

```cpp
// ✅ GOOD - Informative test output
if (!OpenGLContext::HasActiveContext()) {
    TestOutput::PrintInfo("No OpenGL context - testing CPU-only functionality");
    TestOutput::PrintInfo("GPU-specific features will be skipped");
    // Test CPU functionality
    TestOutput::PrintTestPass("texture loading (CPU only)");
    return true;
}

// ❌ BAD - Confusing test output
if (!OpenGLContext::HasActiveContext()) {
    return true; // Silent skip - unclear why test passed
}
```

### 5. Test Both Scenarios

```cpp
// Test suite that covers both scenarios
int main() {
    TestOutput::PrintHeader("Texture");

    TestSuite suite("Texture Tests");
    bool allPassed = true;

    // Tests that work without context
    allPassed &= suite.RunTest("Data Loading", TestTextureDataLoading);
    allPassed &= suite.RunTest("Without Context", TestTextureWithoutContext);

    // Tests that require context (with graceful fallback)
    allPassed &= suite.RunTest("GPU Creation", TestTextureGPUCreation);
    allPassed &= suite.RunTest("OpenGL Integration", TestTextureOpenGLIntegration);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    return allPassed ? 0 : 1;
}
```

## Common Pitfalls

### Pitfall 1: Assuming Context Exists

```cpp
// ❌ WRONG - Will crash in headless environments
bool TestTexture() {
    Texture texture;
    texture.LoadFromFile("test.png"); // Crashes if LoadFromFile calls OpenGL
    return texture.IsValid();
}

// ✅ CORRECT - Check context or use safe APIs
bool TestTexture() {
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("Testing without OpenGL context");
        // Test CPU-only functionality
        return TestTextureCPUFunctionality();
    }

    // Full OpenGL testing
    return TestTextureWithOpenGL();
}
```

### Pitfall 2: Silent Failures

```cpp
// ❌ WRONG - Fails silently, hard to debug
bool TestGraphics() {
    try {
        GLuint texture;
        glGenTextures(1, &texture);
        return texture != 0;
    } catch (...) {
        return false; // What went wrong?
    }
}

// ✅ CORRECT - Clear error reporting
bool TestGraphics() {
    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("No OpenGL context available");
        TestOutput::PrintTestPass("graphics (context check)");
        return true;
    }

    GLuint texture;
    glGenTextures(1, &texture);

    if (texture == 0) {
        TestOutput::PrintError("Failed to create OpenGL texture");
        return false;
    }

    return true;
}
```

### Pitfall 3: Inconsistent Mock Behavior

```cpp
// ❌ WRONG - Mock doesn't match real behavior
class MockTexture {
public:
    bool LoadFromFile(const std::string& path) {
        return true; // Always succeeds - unrealistic
    }

    int GetWidth() const { return 256; } // Fixed size - unrealistic
};

// ✅ CORRECT - Realistic mock behavior
class MockTexture {
public:
    bool LoadFromFile(const std::string& path) {
        if (path.empty() || path.find(".invalid") != std::string::npos) {
            return false; // Realistic failure cases
        }

        // Simulate different image sizes based on filename
        if (path.find("small") != std::string::npos) {
            m_width = 64; m_height = 64;
        } else if (path.find("large") != std::string::npos) {
            m_width = 1024; m_height = 1024;
        } else {
            m_width = 256; m_height = 256;
        }

        return true;
    }

private:
    int m_width = 0, m_height = 0;
};
```

### Pitfall 4: Platform-Specific Assumptions

```cpp
// ❌ WRONG - Assumes Windows behavior
bool TestOpenGLContext() {
    // This might work on Windows but fail on Linux headless
    return OpenGLContext::HasActiveContext();
}

// ✅ CORRECT - Platform-agnostic testing
bool TestOpenGLContext() {
    bool hasContext = OpenGLContext::HasActiveContext();

    TestOutput::PrintInfo("OpenGL context available: " +
                         std::string(hasContext ? "yes" : "no"));

    // Test should pass regardless of context availability
    // The important thing is that the check doesn't crash
    TestOutput::PrintTestPass("OpenGL context check");
    return true;
}
```

## Troubleshooting

### Issue 1: Tests Crash in CI/CD

**Symptoms:**

- Tests pass locally but crash in CI/CD
- Segmentation faults or access violations
- OpenGL-related error messages

**Solution:**

```cpp
// Add context checks to all graphics tests
bool TestGraphicsFeature() {
    TestOutput::PrintTestStart("graphics feature");

    if (!OpenGLContext::HasActiveContext()) {
        TestOutput::PrintInfo("CI/CD environment detected - no OpenGL context");
        TestOutput::PrintInfo("Testing fallback behavior");

        // Test that the system handles missing context gracefully
        GraphicsSystem graphics;
        bool initialized = graphics.Initialize();

        // Should fail gracefully in headless environment
        EXPECT_FALSE(initialized);

        TestOutput::PrintTestPass("graphics feature (headless)");
        return true;
    }

    // Normal testing with OpenGL context
    return TestWithOpenGL();
}
```

### Issue 2: Inconsistent Test Results

**Symptoms:**

- Tests sometimes pass, sometimes fail
- Different behavior on different machines
- Timing-related issues

**Solution:**

```cpp
// Make tests deterministic regardless of context
bool TestResourceLoading() {
    TestOutput::PrintTestStart("resource loading");

    ResourceManager manager;

    // Use consistent mock resources for testing
    auto resource = std::make_shared<MockResource>("test.dat");
    manager.RegisterResource("test.dat", resource);

    auto loaded = manager.GetResource("test.dat");
    EXPECT_NOT_NULL(loaded);
    EXPECT_TRUE(loaded->IsLoaded());

    TestOutput::PrintTestPass("resource loading");
    return true;
}
```

### Issue 3: Mock Resources Don't Match Real Behavior

**Symptoms:**

- Tests pass with mocks but fail with real resources
- Integration issues not caught by unit tests
- Behavioral differences between mock and real implementations

**Solution:**

```cpp
// Create comprehensive mock that matches real behavior
class RealisticMockTexture : public Texture {
public:
    bool LoadFromFile(const std::string& path) override {
        // Simulate real loading behavior
        if (!FileExists(path)) {
            return false;
        }

        // Simulate format validation
        if (!IsSupportedFormat(path)) {
            LOG_ERROR("Unsupported texture format: " + path);
            return false;
        }

        // Simulate memory allocation
        SimulateImageData(path);

        m_isLoaded = true;
        return true;
    }

private:
    void SimulateImageData(const std::string& path) {
        // Create realistic image data based on filename
        // This helps catch memory-related issues
        size_t size = EstimateImageSize(path);
        m_imageData.resize(size);
        std::fill(m_imageData.begin(), m_imageData.end(), 0xFF);
    }
};
```

### Issue 4: Performance Issues in Headless Mode

**Symptoms:**

- Tests run slower without OpenGL context
- Memory usage higher than expected
- CPU usage spikes during testing

**Solution:**

```cpp
// Optimize mock resources for testing
class OptimizedMockTexture : public Texture {
public:
    bool LoadFromFile(const std::string& path) override {
        // Don't actually load image data in tests
        // Just validate the path and set properties
        if (path.empty()) return false;

        // Set realistic properties without loading data
        m_width = 256;
        m_height = 256;
        m_format = GL_RGBA;
        m_isLoaded = true;

        return true;
    }

    size_t GetMemoryUsage() const override {
        // Return realistic memory usage without allocating
        return m_width * m_height * 4;
    }
};
```

### Debugging Context Issues

```cpp
// Comprehensive context debugging
void DebugOpenGLContext() {
    TestOutput::PrintInfo("=== OpenGL Context Debug Info ===");

    // Check basic context availability
    bool hasContext = OpenGLContext::HasActiveContext();
    TestOutput::PrintInfo("HasActiveContext(): " + std::string(hasContext ? "true" : "false"));

    // Check if OpenGL is ready
    bool isReady = OpenGLContext::IsReady();
    TestOutput::PrintInfo("IsReady(): " + std::string(isReady ? "true" : "false"));

    // Get version information
    const char* version = OpenGLContext::GetVersionString();
    TestOutput::PrintInfo("Version: " + std::string(version ? version : "N/A"));

    // Platform-specific information
#ifdef _WIN32
    TestOutput::PrintInfo("Platform: Windows");
#elif defined(__linux__)
    TestOutput::PrintInfo("Platform: Linux");
    TestOutput::PrintInfo("DISPLAY: " + std::string(getenv("DISPLAY") ? getenv("DISPLAY") : "not set"));
#elif defined(__APPLE__)
    TestOutput::PrintInfo("Platform: macOS");
#endif

    TestOutput::PrintInfo("=== End Debug Info ===");
}

// Use in tests when debugging context issues
bool TestWithDebugInfo() {
    DebugOpenGLContext();

    // Continue with actual test...
    return true;
}
```

This comprehensive guide provides everything needed to handle OpenGL context limitations in Game Engine Kiro's testing system. By following these patterns and practices, tests will be robust, reliable, and work consistently across all environments.
