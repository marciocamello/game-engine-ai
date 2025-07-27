# Model Hot-Reloading System

The Model Hot-Reloading system provides seamless development workflow support for 3D models, allowing real-time updates when model files are modified during development.

## Overview

The hot-reloading system consists of three main components:

1. **ModelHotReloader** - Core file watching and reloading functionality
2. **ModelDevelopmentTools** - High-level development workflow integration
3. **Development Examples** - Sample code showing how to integrate hot-reloading

## Quick Start

### Basic Setup

```cpp
#include "Resource/ModelDevelopmentTools.h"
#include "Resource/ModelLoader.h"

// Initialize model loader
auto modelLoader = std::make_shared<ModelLoader>();
modelLoader->Initialize();

// Initialize development tools
auto devTools = std::make_shared<ModelDevelopmentTools>();
devTools->Initialize(modelLoader);

// Enable hot-reloading
devTools->EnableHotReloading();
```

### Watching Individual Models

```cpp
// Load a model
auto model = modelLoader->LoadModelAsResource("assets/meshes/character.fbx");

// Start watching it for changes
devTools->WatchModel("assets/meshes/character.fbx", model);

// Set up reload callback (optional)
devTools->SetReloadCallback([](const std::string& path, std::shared_ptr<Model> newModel, bool success) {
    if (success) {
        Logger::GetInstance().Info("Model reloaded: " + path);
        // Update your scene with the new model
    } else {
        Logger::GetInstance().Error("Failed to reload: " + path);
    }
});
```

### Auto-Watching Asset Directories

```cpp
// Configure to automatically watch asset directories
ModelDevelopmentTools::DevelopmentConfig config;
config.enableHotReloading = true;
config.autoWatchAssetDirectories = true;
config.assetDirectories = {"assets/meshes", "assets/models", "assets/GLTF"};
config.hotReloadInterval = std::chrono::milliseconds(500);

devTools->SetConfig(config);
```

## Configuration Options

### Hot-Reload Configuration

```cpp
ModelDevelopmentTools::DevelopmentConfig config;

// Enable/disable features
config.enableHotReloading = true;           // Enable hot-reloading
config.enableValidation = true;             // Validate models after reload
config.enableOptimization = true;           // Optimize models after reload
config.enablePerformanceMonitoring = true;  // Track performance metrics

// File watching settings
config.hotReloadInterval = std::chrono::milliseconds(500);  // Poll interval
config.autoWatchAssetDirectories = true;    // Auto-watch configured directories

// Asset directories to watch
config.assetDirectories = {
    "assets/meshes",
    "assets/models",
    "assets/GLTF"
};

// File extensions to watch
config.watchedExtensions = {
    "obj", "fbx", "gltf", "glb", "dae", "3ds", "blend"
};

devTools->SetConfig(config);
```

### Advanced Hot-Reloader Configuration

```cpp
ModelHotReloader::HotReloadConfig hotConfig;
hotConfig.enabled = true;
hotConfig.pollInterval = std::chrono::milliseconds(250);
hotConfig.validateOnReload = true;
hotConfig.optimizeOnReload = true;
hotConfig.clearCacheOnReload = true;
hotConfig.logReloadEvents = true;

// Directories to ignore during recursive watching
hotConfig.ignoredDirectories = {"cache", "temp", ".git", ".kiro", "build"};

// Apply configuration
hotReloader->SetConfig(hotConfig);
```

## Development Workflow

### Typical Development Session

1. **Initialize the system** at application startup
2. **Load your models** using the ModelLoader
3. **Watch the models** you're actively working on
4. **Enable hot-reloading** to start file monitoring
5. **Modify model files** in your 3D modeling software
6. **Save the files** - hot-reloading will automatically detect changes
7. **See updates** in real-time in your application

### Manual Operations

```cpp
// Manually reload all watched models
devTools->ReloadAllWatchedModels();

// Validate all watched models
devTools->ValidateAllWatchedModels();

// Optimize all watched models
devTools->OptimizeAllWatchedModels();

// Get performance metrics
auto metrics = devTools->GetPerformanceMetrics();
Logger::GetInstance().Info("Total reloads: " + std::to_string(metrics.totalReloads));

// Generate development report
devTools->GenerateAssetReport("development_report.md");
```

## Model Validation

The system can automatically validate models after reloading:

```cpp
// Validate a specific model
auto validation = devTools->ValidateModel(model);

if (validation.isValid) {
    Logger::GetInstance().Info("Model is valid");
} else {
    Logger::GetInstance().Warning("Model has issues:");
    for (const auto& error : validation.errors) {
        Logger::GetInstance().Warning("  Error: " + error);
    }
    for (const auto& warning : validation.warnings) {
        Logger::GetInstance().Warning("  Warning: " + warning);
    }
    for (const auto& suggestion : validation.suggestions) {
        Logger::GetInstance().Info("  Suggestion: " + suggestion);
    }
}

// Validation provides detailed metrics
Logger::GetInstance().Info("Vertices: " + std::to_string(validation.vertexCount));
Logger::GetInstance().Info("Triangles: " + std::to_string(validation.triangleCount));
Logger::GetInstance().Info("Materials: " + std::to_string(validation.materialCount));
Logger::GetInstance().Info("Memory usage: " + std::to_string(validation.memoryUsage / 1024) + " KB");
```

## Performance Monitoring

Track development workflow performance:

```cpp
// Get current metrics
auto metrics = devTools->GetPerformanceMetrics();

std::cout << "=== Performance Metrics ===" << std::endl;
std::cout << "Total models loaded: " << metrics.totalModelsLoaded << std::endl;
std::cout << "Total reloads: " << metrics.totalReloads << std::endl;
std::cout << "Validation failures: " << metrics.validationFailures << std::endl;
std::cout << "Average load time: " << metrics.averageLoadTimeMs << "ms" << std::endl;
std::cout << "Average reload time: " << metrics.averageReloadTimeMs << "ms" << std::endl;
std::cout << "Total memory usage: " << metrics.totalMemoryUsage / (1024*1024) << " MB" << std::endl;

// Reset metrics
devTools->ResetPerformanceMetrics();

// Log detailed performance report
devTools->LogPerformanceReport();
```

## Asset Pipeline Integration

### Batch Processing

```cpp
// Process an entire asset directory
bool success = devTools->ProcessAssetDirectory("assets/meshes", true); // recursive

// Find problematic models
auto problematicModels = devTools->FindProblematicModels("assets/meshes");
for (const auto& modelPath : problematicModels) {
    Logger::GetInstance().Warning("Issues found in: " + modelPath);

    // Get detailed validation results
    auto validation = devTools->ValidateModelFile(modelPath);
    for (const auto& error : validation.errors) {
        Logger::GetInstance().Warning("  " + error);
    }
}
```

### Asset Reports

```cpp
// Generate comprehensive asset report
devTools->GenerateAssetReport("asset_report.md");
```

The generated report includes:

- Performance metrics
- Asset directory status
- Hot-reload statistics
- Problematic models list

## Example Integration

See `examples/model_hot_reload_example.cpp` for a complete working example that demonstrates:

- Setting up the hot-reloading system
- Loading and watching multiple models
- Handling reload callbacks
- Interactive development commands
- Performance monitoring

## Best Practices

### Performance

- Use reasonable poll intervals (250-500ms) to balance responsiveness and CPU usage
- Enable validation and optimization only during active development
- Clear caches when reloading to ensure fresh data
- Monitor memory usage with large models

### Workflow

- Watch only the models you're actively working on
- Use asset directories for broader monitoring
- Set up reload callbacks to update your scene automatically
- Generate regular asset reports to track project health

### File Organization

- Keep model files in organized directory structures
- Use consistent naming conventions
- Avoid deeply nested directory structures for better performance
- Exclude build/cache directories from watching

## Troubleshooting

### Common Issues

**Hot-reloading not detecting changes:**

- Check file permissions
- Verify the file path is correct
- Ensure the file extension is in the watched extensions list
- Check if the file is in an ignored directory

**Slow reload performance:**

- Reduce poll interval if too frequent
- Disable validation/optimization for faster reloads
- Check for large model files that may take time to process

**Memory usage growing:**

- Enable cache clearing on reload
- Monitor for memory leaks in model loading
- Use performance metrics to track memory usage

### Debug Information

```cpp
// Print detailed status information
devTools->PrintWatchedModelsStatus();
devTools->PrintAssetDirectoryStatus();

// Print model information
devTools->PrintModelInfo(model);
devTools->PrintModelFileInfo("assets/meshes/character.fbx");

// Enable verbose logging
Logger::GetInstance().SetLogLevel(LogLevel::Debug);
```

## Integration with Build Systems

The hot-reloading system works well with:

- **Visual Studio** - Modify models in external tools, save, see updates immediately
- **Blender** - Export models and see changes in real-time
- **Maya/3ds Max** - Export workflow with automatic reload
- **Asset pipelines** - Integrate with automated asset processing

## Thread Safety

The hot-reloading system is designed to be thread-safe:

- File watching runs on a separate thread
- Model loading is synchronized
- Callbacks are executed on the main thread
- Statistics are protected with mutexes

## Platform Support

Currently supported on:

- **Windows** - Full support with filesystem monitoring
- **Linux** - Basic support (future enhancement planned)
- **macOS** - Basic support (future enhancement planned)

## Future Enhancements

Planned improvements:

- Native filesystem event monitoring (instead of polling)
- Network-based model streaming
- Collaborative development features
- Advanced asset pipeline integration
- Real-time model editing support
