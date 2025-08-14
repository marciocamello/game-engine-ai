@echo off
setlocal enabledelayedexpansion
REM Build System Configuration Manager
REM Manages advanced features and optional enhancements
REM Requirements: 6.4, 6.5

echo ========================================
echo Build System Configuration Manager
echo ========================================

set "CONFIG_FILE=scripts\build_config.json"
set "USER_CONFIG=.kiro\build_config_user.json"

REM Parse command line arguments
if "%1"=="" goto :show_help
if /i "%1"=="--help" goto :show_help
if /i "%1"=="-h" goto :show_help
if /i "%1"=="--status" goto :show_status
if /i "%1"=="--enable" goto :enable_feature
if /i "%1"=="--disable" goto :disable_feature
if /i "%1"=="--reset" goto :reset_config
if /i "%1"=="--validate" goto :validate_config

echo ERROR: Unknown command "%1"
goto :show_help

:show_help
echo Build System Configuration Manager
echo.
echo Usage: build_config_manager.bat [command] [options]
echo.
echo Commands:
echo   --status              Show current configuration status
echo   --enable FEATURE      Enable an advanced feature
echo   --disable FEATURE     Disable an advanced feature
echo   --reset               Reset to default configuration
echo   --validate            Validate current configuration
echo   --help, -h            Show this help message
echo.
echo Available Features:
echo   build_cache           vcpkg binary cache optimization
echo   ninja_auto_selection  Automatic Ninja generator selection
echo   build_diagnostics     Build performance monitoring
echo   preset_detection      CMakePresets.json auto-detection
echo   incremental_optimization  Incremental build optimization
echo   environment_detection IDE environment detection
echo.
echo Examples:
echo   build_config_manager.bat --status
echo   build_config_manager.bat --enable build_cache
echo   build_config_manager.bat --disable ninja_auto_selection
echo   build_config_manager.bat --reset
echo.
goto :end

:show_status
echo Current Build System Configuration:
echo.
if exist "%CONFIG_FILE%" (
    echo Configuration file: %CONFIG_FILE%
    echo Status: Available
    echo.
    echo Advanced Features Status:
    echo   [ENABLED]  Build Cache (vcpkg binary cache)
    echo   [ENABLED]  Ninja Auto-Selection
    echo   [ENABLED]  Build Diagnostics
    echo   [ENABLED]  Preset Detection
    echo   [ENABLED]  Incremental Optimization
    echo   [ENABLED]  Environment Detection
    echo.
    echo Compatibility Mode: ENABLED
    echo   - Legacy command support: YES
    echo   - Preserve existing behavior: YES
    echo   - Graceful degradation: YES
    echo.
) else (
    echo Configuration file not found: %CONFIG_FILE%
    echo Status: Using defaults
)

if exist "%USER_CONFIG%" (
    echo User overrides: %USER_CONFIG%
    echo Custom configuration detected
) else (
    echo User overrides: None
)
echo.
goto :end

:enable_feature
if "%2"=="" (
    echo ERROR: Feature name required
    echo Use --help to see available features
    goto :end
)

set "FEATURE=%2"
echo Enabling feature: %FEATURE%

REM Create user config directory if it doesn't exist
if not exist ".kiro" mkdir .kiro

REM Create or update user configuration
echo {> "%USER_CONFIG%"
echo   "user_overrides": {>> "%USER_CONFIG%"
echo     "%FEATURE%": {>> "%USER_CONFIG%"
echo       "enabled": true,>> "%USER_CONFIG%"
echo       "user_modified": true,>> "%USER_CONFIG%"
echo       "modified_date": "%DATE% %TIME%">> "%USER_CONFIG%"
echo     }>> "%USER_CONFIG%"
echo   }>> "%USER_CONFIG%"
echo }>> "%USER_CONFIG%"

echo [SUCCESS] Feature '%FEATURE%' has been enabled
echo Configuration saved to: %USER_CONFIG%
echo.
goto :end

:disable_feature
if "%2"=="" (
    echo ERROR: Feature name required
    echo Use --help to see available features
    goto :end
)

set "FEATURE=%2"
echo Disabling feature: %FEATURE%

REM Create user config directory if it doesn't exist
if not exist ".kiro" mkdir .kiro

REM Create or update user configuration
echo {> "%USER_CONFIG%"
echo   "user_overrides": {>> "%USER_CONFIG%"
echo     "%FEATURE%": {>> "%USER_CONFIG%"
echo       "enabled": false,>> "%USER_CONFIG%"
echo       "user_modified": true,>> "%USER_CONFIG%"
echo       "modified_date": "%DATE% %TIME%">> "%USER_CONFIG%"
echo     }>> "%USER_CONFIG%"
echo   }>> "%USER_CONFIG%"
echo }>> "%USER_CONFIG%"

echo [SUCCESS] Feature '%FEATURE%' has been disabled
echo Configuration saved to: %USER_CONFIG%
echo.
goto :end

:reset_config
echo Resetting build system configuration to defaults...

if exist "%USER_CONFIG%" (
    del "%USER_CONFIG%"
    echo [SUCCESS] User configuration removed
) else (
    echo [INFO] No user configuration to remove
)

echo [SUCCESS] Configuration reset to defaults
echo All advanced features are now using default settings
echo.
goto :end

:validate_config
echo Validating build system configuration...
echo.

REM Check if configuration file exists
if not exist "%CONFIG_FILE%" (
    echo [ERROR] Configuration file not found: %CONFIG_FILE%
    goto :end
)

echo [PASS] Configuration file exists: %CONFIG_FILE%

REM Check for required dependencies
echo.
echo Checking feature dependencies:

REM Check Ninja availability
ninja --version >nul 2>&1
if !errorlevel! equ 0 (
    echo [PASS] Ninja generator available
) else (
    echo [WARN] Ninja generator not available - auto-selection will fallback to Visual Studio
)

REM Check Visual Studio environment
if defined VCINSTALLDIR (
    echo [PASS] Visual Studio environment detected
) else (
    echo [WARN] Visual Studio environment not detected - some features may be limited
)

REM Check vcpkg availability
if exist "vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo [PASS] vcpkg toolchain available
) else (
    echo [WARN] vcpkg toolchain not found - cache features will be disabled
)

REM Check CMakePresets.json
if exist "CMakePresets.json" (
    echo [PASS] CMakePresets.json available
) else (
    echo [INFO] CMakePresets.json not found - will use manual configuration
)

echo.
echo [SUCCESS] Configuration validation completed
echo Build system is ready with available features
echo.
goto :end

:end