# Advanced Build System Features

This document describes the optional advanced features available in the Game Engine Kiro build system.

## Overview

The build system includes several advanced features that can be enabled or disabled based on your development needs. All features are designed to be:

- **Optional**: Can be enabled/disabled without breaking core functionality
- **Auto-detecting**: Automatically detect availability and fallback gracefully
- **Backward compatible**: Preserve existing command behavior

## Available Features

### 1. Build Cache (vcpkg Binary Cache)

**Status**: Enabled by default  
**Purpose**: Accelerate builds by caching compiled dependencies

**Benefits**:

- Significantly faster dependency compilation
- Shared cache across multiple projects
- Automatic cache management

**Requirements**:

- Writable cache directory (`%USERPROFILE%\.vcpkg-cache`)
- Sufficient disk space (recommended: 1GB+)

**Configuration**:

```bash
# Enable build cache
.\scripts\build_config_manager.bat --enable build_cache

# Disable build cache
.\scripts\build_config_manager.bat --disable build_cache

# Build without cache (one-time)
.\scripts\build_unified.bat --no-cache --tests
```

**Fallback**: Automatically falls back to normal compilation if cache is unavailable

### 2. Ninja Auto-Selection

**Status**: Enabled by default  
**Purpose**: Automatically use Ninja generator for faster builds when available

**Benefits**:

- Faster parallel builds
- Better incremental build performance
- Automatic fallback to Visual Studio generator

**Requirements**:

- Ninja installed and in PATH
- Visual Studio environment (Developer Command Prompt)

**Configuration**:

```bash
# Force Ninja usage
.\scripts\build_unified.bat --ninja

# Prefer Visual Studio generator
set GAMEENGINE_PREFER_VS=1

# Check Ninja availability
ninja --version
```

**Installation**:

```bash
# Install Ninja via winget
winget install Ninja-build.Ninja

# Or download from: https://github.com/ninja-build/ninja/releases
```

### 3. Build Diagnostics

**Status**: Enabled by default  
**Purpose**: Monitor build performance and generate diagnostic reports

**Features**:

- Build time measurement
- Performance comparison with previous builds
- Compilation statistics
- Cache usage reporting

**Output Files**:

- `logs/build_diagnostics.log` - Detailed diagnostic log
- `logs/build_performance_data.json` - Performance metrics
- `build_state.tmp` - Build state tracking (temporary)

**Configuration**:

```bash
# View current diagnostics
.\scripts\monitor.bat

# Check build performance
type logs\build_diagnostics.log
```

### 4. Preset Detection

**Status**: Enabled by default  
**Purpose**: Automatically detect and use CMakePresets.json configuration

**Benefits**:

- Modern CMake workflow
- Consistent build configurations
- IDE integration support

**Behavior**:

- Automatically uses presets when `CMakePresets.json` exists
- Falls back to manual configuration if presets unavailable
- Supports both Ninja and Visual Studio presets

**Preset Types**:

- `ninja-debug` / `ninja-release` - Ninja generator presets
- `vs-debug` / `vs-release` - Visual Studio generator presets

### 5. Incremental Optimization

**Status**: Enabled by default  
**Purpose**: Optimize incremental builds through intelligent state tracking

**Features**:

- Build signature tracking
- Identical build detection
- Cache conflict resolution
- Build failure recovery

**Files**:

- `build/.last_build_signature` - Build signature cache
- `build/.build_failed` - Build failure marker

**Benefits**:

- Faster consecutive identical builds
- Automatic cache cleanup after failures
- Intelligent rebuild decisions

### 6. Environment Detection

**Status**: Enabled by default  
**Purpose**: Detect IDE environments and adjust build behavior accordingly

**Supported IDEs**:

- **Kiro IDE**: Detected via `KIRO_IDE_SESSION` environment variable
- **VS Code**: Detected via `VSCODE_PID` environment variable
- **Visual Studio**: Detected via `VCINSTALLDIR` environment variable

**Automatic Adjustments**:

- Disable cache in IDE environments for compatibility
- Adjust output verbosity
- Optimize for IDE integration

## Configuration Management

### Using the Configuration Manager

```bash
# Show current status
.\scripts\build_config_manager.bat --status

# Enable a feature
.\scripts\build_config_manager.bat --enable build_cache

# Disable a feature
.\scripts\build_config_manager.bat --disable ninja_auto_selection

# Reset to defaults
.\scripts\build_config_manager.bat --reset

# Validate configuration
.\scripts\build_config_manager.bat --validate
```

### Configuration Files

1. **System Configuration**: `scripts/build_config.json`

   - Default feature settings
   - System-wide configuration
   - Should not be modified by users

2. **User Configuration**: `.kiro/build_config_user.json`
   - User-specific overrides
   - Created automatically when features are enabled/disabled
   - Takes precedence over system configuration

### Environment Variables

- `GAMEENGINE_PREFER_VS=1` - Force Visual Studio generator over Ninja
- `KIRO_IDE_SESSION=1` - Indicate Kiro IDE environment
- `VSCODE_PID` - Automatically set by VS Code

## Compatibility Mode

All advanced features operate in compatibility mode by default:

- **Legacy Command Support**: All existing commands continue to work unchanged
- **Preserve Existing Behavior**: Core functionality remains identical
- **Graceful Degradation**: Features automatically disable if requirements not met

## Troubleshooting

### Common Issues

1. **Cache Not Working**

   ```bash
   # Check cache health
   .\scripts\build_config_manager.bat --validate

   # Force disable cache
   .\scripts\build_unified.bat --no-cache
   ```

2. **Ninja Not Found**

   ```bash
   # Install Ninja
   winget install Ninja-build.Ninja

   # Verify installation
   ninja --version
   ```

3. **Preset Errors**

   ```bash
   # Validate CMakePresets.json
   cmake --list-presets

   # Force manual configuration
   del CMakePresets.json
   ```

### Diagnostic Commands

```bash
# Full system validation
.\scripts\build_config_manager.bat --validate

# Check build environment
.\scripts\build_unified.bat --help

# Monitor build performance
.\scripts\monitor.bat

# Run comprehensive tests
.\tests\build_system\run_all_build_tests.bat
```

## Performance Impact

| Feature                  | Build Time Impact | Disk Usage | Memory Usage |
| ------------------------ | ----------------- | ---------- | ------------ |
| Build Cache              | -50% to -80%      | +500MB-1GB | Minimal      |
| Ninja Auto-Selection     | -20% to -40%      | Minimal    | Minimal      |
| Build Diagnostics        | +1% to +2%        | +10MB      | Minimal      |
| Preset Detection         | Minimal           | Minimal    | Minimal      |
| Incremental Optimization | -10% to -30%      | +1MB       | Minimal      |
| Environment Detection    | Minimal           | Minimal    | Minimal      |

## Best Practices

1. **Enable All Features**: Default configuration is optimized for most users
2. **Monitor Disk Usage**: Clean cache periodically if disk space is limited
3. **Use Individual Test Builds**: For spec development, use specific test compilation
4. **Regular Validation**: Run configuration validation after system changes
5. **IDE Integration**: Let environment detection handle IDE-specific optimizations

## Future Enhancements

Planned advanced features for future releases:

- **Distributed Build Cache**: Share cache across team members
- **Build Analytics**: Detailed build performance analytics
- **Custom Preset Templates**: User-defined preset configurations
- **Parallel Test Execution**: Run tests in parallel for faster validation
- **Smart Dependency Detection**: Intelligent dependency change detection

## Support

For issues with advanced features:

1. Run diagnostic validation: `.\scripts\build_config_manager.bat --validate`
2. Check logs: `logs/build_diagnostics.log`
3. Reset to defaults: `.\scripts\build_config_manager.bat --reset`
4. Run test suite: `.\tests\build_system\run_all_build_tests.bat`
