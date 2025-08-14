# Design Document

## Overview

The modern build system for Game Engine Kiro will implement industry-standard build optimizations while maintaining full backward compatibility with existing workflows. The design focuses on reliability improvements to fix current `build_unified.bat` issues and performance optimizations through modern CMake practices.

## Architecture

### Build System Components

```
Modern Build System
├── CMakePresets.json          # Standardized configurations
├── Enhanced build_unified.bat # Improved reliability and preset support
├── vcpkg Binary Cache         # Dependency optimization
├── Ninja Generator            # Fast incremental builds
└── Build Diagnostics         # Problem detection and reporting
```

### Integration Strategy

The system will use a layered approach:

1. **Compatibility Layer**: Existing scripts continue working unchanged
2. **Optimization Layer**: CMakePresets + Ninja
3. **Cache Layer**: vcpkg binary cache for dependencies
4. **Diagnostic Layer**: Build problem detection and reporting

## Components and Interfaces

### 1. CMakePresets.json Configuration

**Purpose**: Provide standardized, reproducible build configurations

**Structure**:

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "dev",
      "generator": "Ninja",
      "binaryDir": "build/dev",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo",
        "CMAKE_CXX_STANDARD": "20",
        "CMAKE_TOOLCHAIN_FILE": "${sourceDir}/vcpkg/scripts/buildsystems/vcpkg.cmake"
      }
    },
    {
      "name": "dev-debug",
      "inherits": "dev",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "dev-release",
      "inherits": "dev",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "dev-build",
      "configurePreset": "dev"
    },
    {
      "name": "dev-debug-build",
      "configurePreset": "dev-debug"
    },
    {
      "name": "dev-release-build",
      "configurePreset": "dev-release"
    }
  ]
}
```

**Fallback Strategy**: If CMakePresets is not supported, system falls back to manual configuration with same settings.

### 2. Enhanced build_unified.bat

**Current Issues to Fix**:

- Inconsistent CMake argument handling
- Cache invalidation problems
- Flag combination conflicts
- State management between builds

**Design Improvements**:

```batch
# Improved argument parsing with proper quoting
# Better state management between consecutive builds
# Automatic preset detection and usage
# Enhanced error reporting and recovery
# Consistent behavior regardless of build history
```

**Key Changes**:

1. **Argument Sanitization**: Proper quoting and escaping of all CMake arguments
2. **State Isolation**: Each build type uses separate build directories when needed
3. **Preset Integration**: Automatic detection and use of CMakePresets when available
4. **Error Recovery**: Better handling of failed builds and state cleanup
5. **Consistency Checks**: Validation of build state before and after operations

### 3. Ninja Generator Integration

**Detection Logic**:

```cmake
# CMakeLists.txt enhancement
if(CMAKE_GENERATOR STREQUAL "Ninja" OR CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    set(USING_NINJA TRUE)
    message(STATUS "Using Ninja generator for fast incremental builds")
else()
    set(USING_NINJA FALSE)
    message(STATUS "Using ${CMAKE_GENERATOR} generator")
endif()
```

**Automatic Selection**:

- CMakePresets will prefer Ninja when available
- build_unified.bat will detect Ninja availability
- Fallback to Visual Studio generator if Ninja not found

### 4. vcpkg Binary Cache

**Configuration**:

```batch
# Environment variables for binary caching
set VCPKG_FEATURE_FLAGS=manifests,binarycaching,versions
set VCPKG_BINARY_SOURCES=files,%USERPROFILE%\.vcpkg-cache,readwrite
```

**Cache Strategy**:

- Local cache in user profile for individual developers
- Shared cache location for team environments
- Automatic fallback to normal compilation if cache unavailable

### 5. Build Diagnostics System

**Components**:

1. **Build Time Reporting**: Track and report compilation times
2. **Incremental Build Analysis**: Show what was recompiled and why
3. **Cache State Verification**: Commands to check cache health
4. **Problem Detection**: Identify common build issues automatically

**Implementation**:

```batch
# Enhanced build_unified.bat will include:
# - Build time measurement
# - File count reporting (compiled vs cached)
# - Cache hit/miss statistics
# - Problem detection and suggestions
```

## Data Models

### Build Configuration Model

```cpp
struct BuildConfiguration {
    std::string generator;           // "Ninja" or "Visual Studio 16 2019"
    std::string buildType;          // "Debug", "Release", "RelWithDebInfo"

    bool binaryCache;               // Enable vcpkg binary cache
    std::string cacheLocation;      // Binary cache path
    std::vector<std::string> targets; // Specific targets to build
};
```

### Build Metrics Model

```cpp
struct BuildMetrics {
    std::chrono::duration<double> totalTime;
    int filesCompiled;
    int filesFromCache;
    int targetsBuilt;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};
```

## Error Handling

### Build Failure Recovery

1. **State Preservation**: Failed builds don't corrupt future build attempts
2. **Granular Cleanup**: Option to clean specific components without full rebuild
3. **Error Classification**: Distinguish between configuration, compilation, and linking errors
4. **Recovery Suggestions**: Automatic suggestions for common build problems

### Cache Corruption Handling

1. **Cache Validation**: Verify cache integrity before use
2. **Automatic Recovery**: Rebuild from source if cache is corrupted
3. **Cache Repair**: Tools to fix common cache issues
4. **Fallback Strategy**: Always able to build without cache

### Preset Compatibility

1. **Version Detection**: Check CMake version for preset support
2. **Graceful Degradation**: Fall back to manual configuration if presets unavailable
3. **Migration Path**: Easy upgrade path when CMake is updated

## Testing Strategy

### Compatibility Testing

1. **Regression Tests**: Ensure all existing commands work identically
2. **Performance Benchmarks**: Measure build time improvements
3. **Cross-Platform Testing**: Verify behavior on different Windows configurations
4. **Stress Testing**: Test with various project sizes and configurations

### Build System Testing

1. **Clean Build Tests**: Verify clean builds work correctly
2. **Incremental Build Tests**: Test incremental build accuracy
3. **Cache Tests**: Verify binary cache functionality
4. **Error Recovery Tests**: Test recovery from various failure scenarios

### Integration Testing

1. **CI/CD Integration**: Test in automated build environments
2. **Multi-Developer Testing**: Verify shared cache functionality
3. **Tool Integration**: Test with various IDEs and editors
4. **Dependency Testing**: Verify vcpkg integration remains stable

## Implementation Phases

### Phase 1: Foundation (High Impact, Low Risk)

1. **CMakePresets.json**: Create standardized configurations
2. **build_unified.bat fixes**: Resolve current reliability issues
3. **Basic Ninja integration**: Automatic detection and usage
4. **Build time reporting**: Add performance metrics

**Success Criteria**:

- All existing commands work reliably
- Build times reduced by 30-50%
- No regressions in functionality

### Phase 2: Optimization (Medium Impact, Medium Risk)

1. **vcpkg Binary Cache**: Configure and enable
2. **Advanced diagnostics**: Build problem detection
3. **Performance monitoring**: Detailed metrics and reporting

**Success Criteria**:

- Build times reduced by 50-70% total
- Cache hit rates above 80% for dependencies
- Automatic problem detection working

### Phase 3: Advanced Features (Low Impact, Low Risk)

1. **Precompiled Headers**: For most-used headers
2. **Advanced caching**: More sophisticated cache strategies
3. **Build optimization**: Fine-tuning for specific scenarios
4. **Documentation and training**: Complete user guides

**Success Criteria**:

- Maximum performance achieved
- All features documented
- Team trained on new capabilities

## Backward Compatibility Guarantees

1. **Command Compatibility**: All existing `build_unified.bat` commands work unchanged
2. **Output Compatibility**: Build artifacts in same locations
3. **Script Compatibility**: All other scripts (`run_tests.bat`, etc.) work unchanged
4. **IDE Compatibility**: Visual Studio and VS Code integration maintained
5. **Workflow Compatibility**: No changes required to developer workflows

## Performance Targets

1. **Clean Build**: 30-50% reduction in time (Ninja)
2. **Incremental Build**: 70-90% reduction in time (Ninja + better dependency tracking)
3. **Dependency Build**: 90%+ reduction in time (Binary Cache)
4. **Test Build**: 80%+ reduction in time (Individual test compilation)

## Migration Strategy

1. **Gradual Rollout**: Features enabled incrementally
2. **Opt-in Advanced Features**: New features available but not required
3. **Monitoring**: Performance tracking during migration
4. **Rollback Plan**: Ability to disable features if issues arise
5. **Documentation**: Clear guides for using new features
