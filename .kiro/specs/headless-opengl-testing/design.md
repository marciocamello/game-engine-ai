# Design Document

## Overview

The headless OpenGL testing system provides automated graphics testing capabilities for Game Engine Kiro on Windows through two complementary approaches: offscreen rendering contexts and configurable GPU usage flags. The design integrates seamlessly with the existing test infrastructure while supporting multiple testing scenarios from unit tests to full integration testing.

## Architecture

### Core Components

```
HeadlessTestingSystem
├── OffscreenContextManager     # Manages EGL/WGL offscreen contexts
├── TestConfigurationManager    # Handles useGPU and test mode flags
├── GraphicsTestFramework       # Base classes for graphics tests
└── MockGraphicsRenderer        # CPU-only renderer for unit tests
```

### Integration Points

- **Graphics System**: Extends existing GraphicsRenderer interface
- **Test Infrastructure**: Integrates with current integration test patterns
- **Build System**: CMake detection and vcpkg dependency management
- **Resource Management**: Works with existing ResourceManager for asset loading

## Components and Interfaces

### OffscreenContextManager

```cpp
namespace GameEngine::Testing {
    class OffscreenContextManager {
    public:
        struct ContextConfig {
            int width = 1024;
            int height = 768;
            int colorBits = 32;
            int depthBits = 24;
            int stencilBits = 8;
            bool enableMSAA = false;
        };

        bool Initialize(const ContextConfig& config);
        bool MakeCurrent();
        void SwapBuffers();
        void Cleanup();

        bool IsContextValid() const;
        std::string GetLastError() const;

    private:
        // Platform-specific implementation
        #ifdef _WIN32
            void* m_eglDisplay = nullptr;
            void* m_eglContext = nullptr;
            void* m_eglSurface = nullptr;
        #endif
    };
}
```

### TestConfigurationManager

```cpp
namespace GameEngine::Testing {
    struct TestConfig {
        bool useGPU = true;
        bool generateBuffers = true;
        bool loadTextures = true;
        bool enableValidation = true;

        enum class TestMode {
            GPU_FULL,      // Full GPU testing with offscreen context
            CPU_ONLY,      // No GPU operations, data validation only
            HYBRID         // Data loading + GPU validation
        };
        TestMode mode = TestMode::GPU_FULL;
    };

    class TestConfigurationManager {
    public:
        static TestConfig& GetConfig();
        static void SetTestMode(TestConfig::TestMode mode);
        static bool ShouldUseGPU();
        static bool ShouldGenerateBuffers();
    };
}
```

### GraphicsTestFramework

```cpp
namespace GameEngine::Testing {
    class GraphicsTestBase {
    public:
        virtual void SetUp();
        virtual void TearDown();

    protected:
        std::unique_ptr<OffscreenContextManager> m_contextManager;
        std::unique_ptr<GraphicsRenderer> m_renderer;
        TestConfig m_config;

        void InitializeOffscreenContext();
        void ValidateOpenGLState();
    };

    // Specialized test classes
    class MeshLoadingTest : public GraphicsTestBase { /* ... */ };
    class ShaderCompilationTest : public GraphicsTestBase { /* ... */ };
    class TextureLoadingTest : public GraphicsTestBase { /* ... */ };
}
```

### MockGraphicsRenderer

```cpp
namespace GameEngine::Testing {
    class MockGraphicsRenderer : public GraphicsRenderer {
    public:
        // Override all GraphicsRenderer methods
        bool Initialize() override { return true; }
        void Render() override { /* No-op or validation */ }

        // Mock-specific methods
        void SetValidationMode(bool enabled);
        std::vector<std::string> GetValidationErrors() const;

    private:
        bool m_validationEnabled = true;
        std::vector<std::string> m_validationErrors;
    };
}
```

## Data Models

### Context Configuration

```cpp
struct OffscreenContextConfig {
    // Display settings
    int width = 1024;
    int height = 768;

    // OpenGL settings
    int majorVersion = 4;
    int minorVersion = 6;
    bool coreProfile = true;

    // Buffer settings
    int colorBits = 32;
    int depthBits = 24;
    int stencilBits = 8;
    int samples = 0;  // MSAA samples

    // Platform-specific
    bool useANGLE = true;  // Prefer ANGLE EGL on Windows
    bool fallbackToWGL = true;
};
```

### Test Result Data

```cpp
struct GraphicsTestResult {
    bool success = false;
    std::string testName;
    std::chrono::milliseconds executionTime;

    // GPU-specific results
    bool contextCreated = false;
    std::string openglVersion;
    std::string renderer;

    // Validation results
    std::vector<std::string> warnings;
    std::vector<std::string> errors;

    // Performance metrics
    size_t memoryUsed = 0;
    size_t buffersCreated = 0;
    size_t texturesLoaded = 0;
};
```

## Error Handling

### Context Creation Failures

1. **EGL Initialization Failure**: Fallback to WGL_ARB_pbuffer if available
2. **No GPU Available**: Automatic switch to CPU-only mode with warnings
3. **Driver Issues**: Detailed error reporting with system information
4. **Memory Allocation**: Graceful cleanup and resource deallocation

### Test Execution Errors

1. **OpenGL Errors**: Capture and report GL error states
2. **Resource Loading**: Handle missing assets gracefully
3. **Validation Failures**: Detailed reporting without stopping test suite
4. **Timeout Handling**: Prevent hanging tests in CI environments

## Testing Strategy

### Unit Tests

- **OffscreenContextManager**: Context creation, cleanup, error handling
- **TestConfigurationManager**: Configuration parsing and validation
- **MockGraphicsRenderer**: Validation logic and error reporting

### Integration Tests

Following existing patterns in `tests/integration/`:

```cpp
// tests/integration/test_headless_rendering.cpp
class HeadlessRenderingTest : public GraphicsTestBase {
    void TestMeshRendering();
    void TestShaderCompilation();
    void TestTextureLoading();
};

// tests/integration/test_cpu_only_loading.cpp
class CPUOnlyLoadingTest : public GraphicsTestBase {
    void TestFBXLoading();
    void TestMaterialParsing();
    void TestDataValidation();
};
```

### Build Integration

CMakeLists.txt additions:

```cmake
# Headless testing dependencies
find_package(EGL QUIET)
if(EGL_FOUND)
    set(HEADLESS_TESTING_ENABLED ON)
    target_link_libraries(GameEngineKiro PRIVATE EGL::EGL)
else()
    message(WARNING "EGL not found, headless testing will use fallback mode")
    set(HEADLESS_TESTING_ENABLED OFF)
endif()

# Test executables
if(HEADLESS_TESTING_ENABLED)
    add_executable(HeadlessRenderingTest tests/integration/test_headless_rendering.cpp)
    add_executable(CPUOnlyLoadingTest tests/integration/test_cpu_only_loading.cpp)

    target_link_libraries(HeadlessRenderingTest PRIVATE GameEngineKiro)
    target_link_libraries(CPUOnlyLoadingTest PRIVATE GameEngineKiro)
endif()
```

### vcpkg Integration

vcpkg.json additions:

```json
{
  "dependencies": [
    {
      "name": "angle",
      "platform": "windows",
      "features": ["egl"]
    }
  ]
}
```

## Platform-Specific Implementation

### Windows with ANGLE EGL

- Primary implementation using Google ANGLE
- EGL 1.4+ with OpenGL ES 3.0+ support
- Automatic fallback to native OpenGL if needed

### Windows with WGL_ARB_pbuffer (Fallback)

- Native Windows implementation for maximum compatibility
- Direct WGL context creation for pbuffer surfaces
- Legacy support for older systems

### Build Script Integration

The `/scripts/build_unified.bat --tests` will automatically:

1. Detect available headless rendering libraries
2. Configure CMake with appropriate flags
3. Build test executables in correct directories
4. Provide clear feedback on available test modes

This design ensures seamless integration with existing Game Engine Kiro architecture while providing robust headless testing capabilities for both development and CI/CD environments.
