# Game Engine Kiro - Technical Stack

## Build System

- **CMake 3.16+**: Primary build system with modern CMake practices
- **vcpkg**: Dependency management with manifest mode (vcpkg.json)
- **C++20 Standard**: Required with MSVC 2019+

## Core Dependencies

- **GLFW3**: Window management and input handling
- **GLM**: OpenGL mathematics library with experimental features enabled
- **GLAD**: OpenGL function loader
- **OpenGL 4.6+**: Graphics API
- **Bullet3**: Primary physics engine (with PhysX planned)
- **OpenAL**: 3D spatial audio system
- **nlohmann/json**: JSON parsing and configuration
- **fmt**: String formatting library

## Optional Dependencies

- **Assimp**: 3D model loading (FBX, GLTF, OBJ support) - ACTIVE
- **Lua**: Scripting engine integration
- **STB**: Image loading utilities

## Development Tools

- **clangd**: Language server with compile_commands.json generation
- **Visual Studio**: Primary IDE on Windows
- **VS Code**: Alternative development environment

## Primary Build Commands

```powershell
# Windows - ONLY permitted build command (NEVER use cmake directly)
.\scripts\build.bat

# Clean build - ONLY permitted cleanup command
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
.\scripts\build.bat

# Development console with multiple options
.\scripts\dev.bat

# Run GameExample
build\projects\GameExample\Release\GameExample.exe

# Run BasicExample
build\projects\BasicExample\Release\BasicExample.exe

# Monitor logs in real-time
.\scripts\monitor.bat

# Debug session
.\scripts\debug.bat
```

### Test Execution

```powershell
# Run all tests after build
.\scripts\run_tests.bat

# Individual test execution
.\build\Release\[TestName].exe

# Tests are automatically discovered by CMake from:
# - tests/unit/test_*.cpp -> Unit tests (13 tests)
# - tests/integration/test_*.cpp -> Integration tests (18 tests)
```

## Current Test Suite Status

### Unit Tests (13 total - 100% passing)

- MathTest, MatrixTest, QuaternionTest
- AssertionmacrosTest, Audio3dpositioningTest, AudioengineTest
- AudioloaderTest, MeshloaderTest, MeshoptimizerTest
- ModelnodeTest, ResourcefallbacksTest, ResourcemanagerTest
- TextureloaderTest

### Integration Tests (18 total - 100% passing)

- Physics: BulletUtilsSimpleTest, BulletIntegrationTest, BulletConversionTest
- Physics: CollisionShapeFactorySimpleTest, PhysicsQueriesTest, PhysicsConfigurationTest
- Movement: MovementComponentComparisonTest, PhysicsPerformanceSimpleTest
- Memory: MemoryUsageSimpleTest, CharacterBehaviorSimpleTest
- Audio: OpenALIntegrationTest, AudioCameraIntegrationTest
- Resources: ResourceStatisticsTest, ErrorHandlingTest, FinalV1ValidationTest
- 3D Models: ModelLoaderAssimpTest, MaterialImporterTest, GLTFLoaderTest, FBXLoaderTest

## 3D Model Loading System Status

### Implemented Components

- **ModelLoader**: Assimp-based loader with format detection
- **MeshOptimizer**: Vertex cache optimization, mesh analysis
- **MaterialImporter**: PBR material conversion, texture search/fallback
- **GLTF Support**: Scene parsing, animation, skinning
- **FBX Support**: Scene import, animation, rigging
- **OBJ Enhancement**: MTL materials, optimization, validation

### Current Capabilities

- Multi-format support (GLTF, FBX, OBJ via Assimp)
- Mesh optimization algorithms (Tom Forsyth's algorithm)
- Material import with PBR conversion
- Texture loading with fallback system
- Scene graph hierarchy support
- Animation and skinning support
- Comprehensive mesh analysis and validation

## Compiler Configuration

- **MSVC**: `/W4` warning level, `_CRT_SECURE_NO_WARNINGS` defined
- **Windows**: `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for clangd support

## Windows Libraries

- **Windows**: `winmm` for multimedia

## CMake Features

- **Automatic vcpkg detection**: Uses vcpkg toolchain when available
- **Optional dependency handling**: Graceful fallback when dependencies missing
- **Asset copying**: Automatic asset deployment to build directory
- **Multiple executables**: Engine library + example game + tests
- **Automatic test discovery**: CMake discovers tests from tests/ directory
