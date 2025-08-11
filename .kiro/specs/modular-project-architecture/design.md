# Design Document

## Overview

This design establishes a modular, plugin-based architecture for Game Engine Kiro that separates engine modules, game projects, and tests into distinct, manageable components. The new architecture transforms the current monolithic structure into a flexible system that supports multiple game projects, swappable engine modules, and independent test execution.

The design introduces a hierarchical project structure with clear separation of concerns, standardized interfaces for engine modules, and a plugin system that allows runtime module loading and configuration.

## Architecture

### High-Level Structure

```
GameEngineKiro/
├── engine/                    # Core engine modules
│   ├── core/                 # Engine foundation (required)
│   ├── modules/              # Optional engine modules (plugins)
│   │   ├── graphics-opengl/  # OpenGL renderer module
│   │   ├── physics-bullet/   # Bullet Physics module
│   │   ├── physics-physx/    # PhysX module (future)
│   │   ├── audio-openal/     # OpenAL audio module
│   │   ├── scripting-lua/    # Lua scripting module
│   │   └── resource-assimp/  # Assimp model loading module
│   └── interfaces/           # Module interface definitions
├── projects/                 # Game projects and tests
│   ├── GameExample/          # Enhanced example game
│   ├── BasicExample/         # Simple example game
│   ├── Tests/               # All test suites
│   └── templates/           # Project templates
├── shared/                  # Shared resources and utilities
│   ├── assets/             # Common engine assets
│   ├── shaders/            # Shared shader library
│   └── configs/            # Default configurations
└── tools/                  # Development tools and scripts
```

### Module System Architecture

The engine uses a plugin-based module system with the following components:

1. **Module Interface**: Standardized interface that all modules must implement
2. **Module Registry**: Central registry for discovering and managing modules
3. **Module Loader**: Dynamic loading system for modules
4. **Dependency Manager**: Handles module dependencies and initialization order

### Project Architecture

Each game project follows a standardized structure:

```
projects/GameExample/
├── CMakeLists.txt           # Project-specific build configuration
├── src/                     # Game-specific source code
├── include/                 # Game-specific headers
├── assets/                  # Game-specific assets
├── config/                  # Game configuration files
└── README.md               # Project documentation
```

## Components and Interfaces

### Core Engine Module Interface

```cpp
namespace GameEngine {
    enum class ModuleType {
        Core,
        Graphics,
        Physics,
        Audio,
        Input,
        Scripting,
        Resource,
        Network
    };

    class IEngineModule {
    public:
        virtual ~IEngineModule() = default;

        // Module lifecycle
        virtual bool Initialize(const ModuleConfig& config) = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Shutdown() = 0;

        // Module information
        virtual const char* GetName() const = 0;
        virtual const char* GetVersion() const = 0;
        virtual ModuleType GetType() const = 0;
        virtual std::vector<std::string> GetDependencies() const = 0;

        // Module state
        virtual bool IsInitialized() const = 0;
        virtual bool IsEnabled() const = 0;
        virtual void SetEnabled(bool enabled) = 0;
    };
}
```

### Module Registry System

```cpp
namespace GameEngine {
    class ModuleRegistry {
    public:
        static ModuleRegistry& GetInstance();

        // Module registration
        void RegisterModule(std::unique_ptr<IEngineModule> module);
        void UnregisterModule(const std::string& name);

        // Module access
        IEngineModule* GetModule(const std::string& name);
        std::vector<IEngineModule*> GetModulesByType(ModuleType type);

        // Module lifecycle management
        bool InitializeModules(const EngineConfig& config);
        void UpdateModules(float deltaTime);
        void ShutdownModules();

        // Dependency resolution
        std::vector<IEngineModule*> ResolveDependencies();
        bool ValidateDependencies();
    };
}
```

### Graphics Module Interface

```cpp
namespace GameEngine::Graphics {
    class IGraphicsModule : public IEngineModule {
    public:
        virtual GraphicsRenderer* GetRenderer() = 0;
        virtual bool SupportsAPI(GraphicsAPI api) = 0;
        virtual void SetRenderSettings(const RenderSettings& settings) = 0;
    };
}
```

### Physics Module Interface

```cpp
namespace GameEngine::Physics {
    class IPhysicsModule : public IEngineModule {
    public:
        virtual PhysicsEngine* GetPhysicsEngine() = 0;
        virtual bool SupportsFeature(PhysicsFeature feature) = 0;
        virtual void SetPhysicsSettings(const PhysicsSettings& settings) = 0;
    };
}
```

### Project Template System

```cpp
namespace GameEngine::Tools {
    class ProjectTemplate {
    public:
        struct TemplateConfig {
            std::string projectName;
            std::string targetDirectory;
            std::vector<std::string> requiredModules;
            std::vector<std::string> optionalModules;
            bool includeExampleCode;
        };

        static bool CreateProject(const TemplateConfig& config);
        static std::vector<std::string> GetAvailableTemplates();
    };
}
```

## Data Models

### Module Configuration

```cpp
namespace GameEngine {
    struct ModuleConfig {
        std::string name;
        std::string version;
        bool enabled = true;
        std::unordered_map<std::string, std::string> parameters;
    };

    struct EngineConfig {
        std::vector<ModuleConfig> modules;
        std::string configVersion;
        std::string engineVersion;
    };
}
```

### Project Configuration

```cpp
namespace GameEngine {
    struct ProjectConfig {
        std::string projectName;
        std::string projectVersion;
        std::vector<std::string> requiredModules;
        std::vector<std::string> optionalModules;
        std::unordered_map<std::string, std::string> projectSettings;
        std::string assetPath;
        std::string configPath;
    };
}
```

### Build System Configuration

The new CMake structure uses a hierarchical approach:

#### Root CMakeLists.txt

- Orchestrates the entire build process
- Discovers and includes engine modules
- Discovers and includes game projects
- Manages global dependencies and settings

#### Engine Module CMakeLists.txt

- Each module has its own CMakeLists.txt
- Defines module dependencies and build requirements
- Supports conditional compilation based on available dependencies

#### Project CMakeLists.txt

- Each game project has its own CMakeLists.txt
- Declares required and optional engine modules
- Manages project-specific assets and configurations

## Error Handling

### Module Loading Errors

1. **Missing Dependencies**: Clear error messages indicating which dependencies are missing
2. **Version Conflicts**: Detection and reporting of incompatible module versions
3. **Initialization Failures**: Graceful fallback to alternative modules or safe defaults
4. **Runtime Errors**: Module isolation to prevent cascading failures

### Project Configuration Errors

1. **Invalid Module References**: Validation of module names and availability
2. **Asset Path Errors**: Verification of asset paths and fallback mechanisms
3. **Configuration Syntax Errors**: JSON/YAML validation with helpful error messages

### Build System Errors

1. **Missing CMakeLists.txt**: Template generation for missing build files
2. **Dependency Resolution**: Clear reporting of unresolved dependencies
3. **Compilation Errors**: Module-specific error isolation

## Testing Strategy

### Dual Test Architecture

The testing system maintains separation between engine tests and project tests:

#### Engine Tests (Current Structure)

```
tests/
├── unit/                    # Unit tests for engine components
│   ├── test_math.cpp       # Core engine tests
│   ├── test_audio_engine.cpp # Audio module tests
│   └── test_resource_manager.cpp # Resource tests
├── integration/            # Integration tests between engine modules
│   ├── test_bullet_integration.cpp # Physics integration
│   ├── test_openal_integration.cpp # Audio integration
│   └── test_model_loader_assimp.cpp # Resource integration
└── TestUtils.h             # Shared testing utilities
```

#### Project Tests (Future Structure)

```
projects/Tests/
├── CMakeLists.txt           # Project test suite build configuration
├── unit/                    # Unit tests for game projects
│   ├── GameExample/        # GameExample-specific tests
│   └── BasicExample/       # BasicExample-specific tests
├── integration/            # Integration tests for game projects
├── utilities/              # Project test utilities
└── config/                 # Project test configuration files
```

### Engine Testing Strategy

1. **Unit Tests**: Engine modules include comprehensive unit tests in `tests/unit/`
2. **Integration Tests**: Tests for module interactions in `tests/integration/`
3. **Mock Modules**: Lightweight mock implementations for testing
4. **Performance Tests**: Benchmarking and performance regression detection

### Project Testing Strategy (Future)

1. **Game Logic Tests**: Unit tests for game-specific components
2. **Gameplay Integration**: Tests for game mechanics and systems
3. **Project-Specific Mocks**: Mock implementations for game testing

### Test Execution

1. **Engine Tests**: Continue using existing `.\scripts\run_tests.bat` for engine testing
2. **Project Tests**: Independent test builds for game projects (when implemented)
3. **Automated Discovery**: CMake automatically discovers tests in both structures
4. **Continuous Integration**: Support for automated testing in CI/CD pipelines

### Test Configuration

```cpp
namespace GameEngine::Testing {
    struct TestConfig {
        std::vector<std::string> enabledModules;
        std::vector<std::string> testCategories;
        bool enablePerformanceTests = false;
        bool enableIntegrationTests = true;
        std::string outputFormat = "standard";
    };
}
```

## Implementation Phases

### Phase 1: Core Infrastructure

1. Create new directory structure
2. Implement module interface and registry system
3. Create basic module loader
4. Migrate core engine components

### Phase 2: Module Migration

1. Convert graphics system to module
2. Convert physics system to module
3. Convert audio system to module
4. Implement module dependency system

### Phase 3: Project System

1. Create project template system
2. Migrate existing examples to new structure
3. Implement project configuration system
4. Create project build system

### Phase 4: Test Reorganization

1. Move all tests to dedicated project
2. Implement modular test framework
3. Create test discovery and execution system
4. Add performance and integration test suites

### Phase 5: Advanced Features

1. Runtime module loading
2. Hot-swappable modules
3. Module marketplace/registry
4. Advanced dependency management
