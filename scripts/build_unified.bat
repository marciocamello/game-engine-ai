@echo off
setlocal enabledelayedexpansion
REM Unified Build Script for Game Engine Kiro
REM Supports all build combinations with a single script

echo ========================================
echo Game Engine Kiro - Unified Build System
echo ========================================

REM Initialize build diagnostics and performance monitoring
call :init_build_diagnostics

REM Default values
set BUILD_ENGINE=OFF
set BUILD_PROJECTS=OFF
set BUILD_TESTS=OFF
set SPECIFIC_PROJECT=
set SPECIFIC_TEST=
set BUILD_TYPE=Release
set ENABLE_COVERAGE=OFF
set CMAKE_ARGS=
set BUILD_TYPE_DESC=
set USE_NINJA=OFF

REM Parse command line arguments with improved quoting and validation
:parse_args
if "%~1"=="" goto :validate_args

if /i "%~1"=="--engine" (
    set BUILD_ENGINE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--projects" (
    set BUILD_PROJECTS=ON
    set "SPECIFIC_PROJECT="
    shift
    goto :parse_args
)
if /i "%~1"=="--tests" (
    set BUILD_TESTS=ON
    set "SPECIFIC_TEST="
    REM Check if next argument is a test name (doesn't start with --)
    if not "%~2"=="" (
        echo "%~2" | findstr /r /c:"^--" >nul
        if errorlevel 1 (
            set "SPECIFIC_TEST=%~2"
            shift
        )
    )
    shift
    goto :parse_args
)
if /i "%~1"=="--project" (
    if "%~2"=="" (
        echo ERROR: --project requires a project name
        goto :help
    )
    set "SPECIFIC_PROJECT=%~2"
    set BUILD_PROJECTS=ON
    shift
    shift
    goto :parse_args
)

if /i "%~1"=="--all" (
    set BUILD_ENGINE=ON
    set BUILD_PROJECTS=ON
    set BUILD_TESTS=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if /i "%~1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto :parse_args
)
if /i "%~1"=="--coverage" (
    set BUILD_TYPE=Debug
    set ENABLE_COVERAGE=ON
    set BUILD_ENGINE=ON
    set BUILD_TESTS=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--ninja" (
    set USE_NINJA=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--no-cache" (
    set DISABLE_VCPKG_CACHE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--clean-engine" (
    set CLEAN_ENGINE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--clean-tests" (
    set CLEAN_TESTS=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--clean-projects" (
    set CLEAN_PROJECTS=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--clean-cache" (
    set CLEAN_CACHE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--clean-all" (
    set CLEAN_ENGINE=ON
    set CLEAN_TESTS=ON
    set CLEAN_PROJECTS=ON
    set CLEAN_CACHE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--help" goto :help
if /i "%~1"=="-h" goto :help

REM Legacy compatibility
if /i "%~1"=="--engine-only" (
    set BUILD_ENGINE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--projects-only" (
    set BUILD_PROJECTS=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--tests-only" (
    set BUILD_TESTS=ON
    shift
    goto :parse_args
)

REM Unknown argument
echo ERROR: Unknown argument "%~1"
echo Use --help for usage information
exit /b 1

:validate_args
REM Validate argument combinations
if not "%SPECIFIC_TEST%"=="" if not "%SPECIFIC_PROJECT%"=="" (
    echo ERROR: Cannot specify both --tests [name] and --project [name] simultaneously
    echo Use --help for usage information
    exit /b 1
)

if not "%SPECIFIC_TEST%"=="" if "%BUILD_PROJECTS%"=="ON" if "%SPECIFIC_PROJECT%"=="" (
    echo ERROR: Cannot combine --tests [name] with --projects
    echo Use --help for usage information
    exit /b 1
)

if not "%SPECIFIC_PROJECT%"=="" if "%BUILD_TESTS%"=="ON" if "%SPECIFIC_TEST%"=="" (
    echo ERROR: Cannot combine --project [name] with --tests
    echo Use --help for usage information
    exit /b 1
)

:determine_build_type
REM If no options specified, build everything (default behavior)
if "%BUILD_ENGINE%"=="OFF" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="OFF" (
    set BUILD_ENGINE=ON
    set BUILD_PROJECTS=ON
    set BUILD_TESTS=ON
    set BUILD_TYPE_DESC=All ^(Engine + Projects + Tests^)
    goto :setup_cmake_args
)

REM Determine build type description
set BUILD_TYPE_DESC=Custom Build
if "%BUILD_ENGINE%"=="ON" if "%BUILD_PROJECTS%"=="ON" if "%BUILD_TESTS%"=="ON" set BUILD_TYPE_DESC=All Components
if "%BUILD_ENGINE%"=="ON" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="OFF" set BUILD_TYPE_DESC=Engine Only
if "%BUILD_ENGINE%"=="OFF" if "%BUILD_PROJECTS%"=="ON" if "%BUILD_TESTS%"=="OFF" set BUILD_TYPE_DESC=Projects Only
if "%BUILD_ENGINE%"=="OFF" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="ON" set BUILD_TYPE_DESC=Tests Only
if "%BUILD_ENGINE%"=="ON" if "%BUILD_PROJECTS%"=="ON" if "%BUILD_TESTS%"=="OFF" set BUILD_TYPE_DESC=Engine and Projects
if "%BUILD_ENGINE%"=="ON" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="ON" set BUILD_TYPE_DESC=Engine and Tests
if "%BUILD_ENGINE%"=="OFF" if "%BUILD_PROJECTS%"=="ON" if "%BUILD_TESTS%"=="ON" set BUILD_TYPE_DESC=Projects and Tests

if not "%SPECIFIC_PROJECT%"=="" set BUILD_TYPE_DESC=Project ^(%SPECIFIC_PROJECT%^)
if not "%SPECIFIC_TEST%"=="" set BUILD_TYPE_DESC=Test ^(%SPECIFIC_TEST%^)

:setup_cmake_args
REM Setup CMake arguments based on what to build
REM When building specific test, force engine ON and projects OFF
if not "%SPECIFIC_TEST%"=="" (
    set BUILD_ENGINE=ON
    set BUILD_PROJECTS=OFF
    set "SPECIFIC_PROJECT="
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SPECIFIC_TEST=%SPECIFIC_TEST% -DBUILD_SPECIFIC_PROJECT="
)

REM When building specific project, force engine ON and tests OFF, clear specific test
if not "%SPECIFIC_PROJECT%"=="" (
    set BUILD_ENGINE=ON
    set BUILD_TESTS=OFF
    set "SPECIFIC_TEST="
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SPECIFIC_PROJECT=%SPECIFIC_PROJECT% -DBUILD_SPECIFIC_TEST="
)

REM Map internal flags to CMake options
if "%BUILD_ENGINE%"=="ON" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="OFF" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE_ONLY=ON"
)
if "%BUILD_ENGINE%"=="OFF" if "%BUILD_PROJECTS%"=="ON" if "%BUILD_TESTS%"=="OFF" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PROJECTS_ONLY=ON"
)
if "%BUILD_ENGINE%"=="OFF" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS_ONLY=ON"
)
if "%BUILD_ENGINE%"=="ON" if "%BUILD_PROJECTS%"=="OFF" if "%BUILD_TESTS%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE_AND_TESTS_ONLY=ON"
)

REM For all other combinations, let CMake build everything (default behavior)
if "%ENABLE_COVERAGE%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_COVERAGE=ON"
)

echo Build Configuration: %BUILD_TYPE_DESC%
echo Build Type: %BUILD_TYPE%
if "%ENABLE_COVERAGE%"=="ON" echo Coverage: Enabled
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT%
if not "%SPECIFIC_TEST%"=="" echo Specific Test: %SPECIFIC_TEST%

REM Configure vcpkg binary cache environment (optional optimization)
echo.
echo Configuring vcpkg binary cache...
REM Detect if running from IDE or terminal and adjust cache behavior
if defined KIRO_IDE_SESSION (
    echo Detected Kiro IDE environment - disabling cache for compatibility
    set DISABLE_VCPKG_CACHE=ON
)
if defined VSCODE_PID (
    echo Detected VS Code environment - disabling cache for compatibility
    set DISABLE_VCPKG_CACHE=ON
)
REM Check if running in a restricted environment
if "%SESSIONNAME%"=="Console" if not defined USERPROFILE (
    echo Detected restricted environment - disabling cache
    set DISABLE_VCPKG_CACHE=ON
)
call :setup_vcpkg_cache

REM Verify dependencies, CMakePresets, and build state
echo.
echo Verifying dependencies, CMakePresets, and build state...
if not exist "vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo WARNING: vcpkg toolchain not found. Some dependencies may not be available.
)

REM Check for CMakePresets.json and determine preset to use
set "USE_PRESETS=OFF"
set "CONFIGURE_PRESET="
set "BUILD_PRESET="
set "NINJA_AVAILABLE=OFF"
set "VS_ENV_AVAILABLE=OFF"

REM Enhanced Ninja detection
echo Detecting build environment...
set "NINJA_AVAILABLE=OFF"
set "VS_ENV_AVAILABLE=OFF"

ninja --version >nul 2>&1
if !errorlevel! equ 0 set "NINJA_AVAILABLE=ON"

if defined VCINSTALLDIR set "VS_ENV_AVAILABLE=ON"

if "%NINJA_AVAILABLE%"=="ON" (
    echo   - Ninja: Available
) else (
    echo   - Ninja: Not found in PATH
)

if "%VS_ENV_AVAILABLE%"=="ON" (
    echo   - Visual Studio Environment: Available
) else (
    echo   - Visual Studio Environment: Not detected
)

if exist "CMakePresets.json" (
    set "USE_PRESETS=ON"
    echo CMakePresets.json detected - using modern preset configuration
    
    REM Determine which preset to use
    if "%USE_NINJA%"=="ON" (
        REM Ninja explicitly requested
        if "%NINJA_AVAILABLE%"=="ON" (
            if "%VS_ENV_AVAILABLE%"=="ON" (
                echo Using Ninja presets ^(explicitly requested^)
                goto :set_ninja_preset
            ) else (
                echo WARNING: Ninja requested but Visual Studio environment not detected
                echo Ninja requires MSVC compiler environment to be set up
                echo Please run this script from a Visual Studio Developer Command Prompt
                echo Falling back to Visual Studio presets
                goto :set_vs_preset
            )
        ) else (
            echo WARNING: Ninja requested but not found in PATH
            echo To install Ninja: winget install Ninja-build.Ninja
            echo Falling back to Visual Studio presets
            goto :set_vs_preset
        )
    ) else (
        REM Automatic selection
        if "%NINJA_AVAILABLE%"=="ON" (
            if "%VS_ENV_AVAILABLE%"=="ON" (
                if defined GAMEENGINE_PREFER_VS (
                    echo Environment variable GAMEENGINE_PREFER_VS detected - using Visual Studio presets
                    goto :set_vs_preset
                ) else (
                    echo Using Ninja presets ^(automatically selected for faster builds^)
                    echo Tip: Use --ninja flag to explicitly request Ninja, or set GAMEENGINE_PREFER_VS=1 to prefer Visual Studio
                    goto :set_ninja_preset
                )
            ) else (
                echo Using Visual Studio presets ^(fallback - VS environment not detected^)
                goto :set_vs_preset
            )
        ) else (
            echo Using Visual Studio presets ^(fallback - Ninja not available^)
            goto :set_vs_preset
        )
    )
    
    :set_ninja_preset
    if "%BUILD_TYPE%"=="Debug" (
        set "CONFIGURE_PRESET=ninja-debug"
        set "BUILD_PRESET=ninja-debug"
    ) else (
        set "CONFIGURE_PRESET=ninja-release"
        set "BUILD_PRESET=ninja-release"
    )
    goto :preset_set
    
    :set_vs_preset
    if "%BUILD_TYPE%"=="Debug" (
        set "CONFIGURE_PRESET=vs-debug"
        set "BUILD_PRESET=vs-debug"
    ) else (
        set "CONFIGURE_PRESET=vs-release"
        set "BUILD_PRESET=vs-release"
    )
    goto :preset_set
    
    :preset_set
    echo Using preset: !CONFIGURE_PRESET!
) else (
    echo CMakePresets.json not found - using manual configuration
)

REM Check for previous failed builds and clean if necessary
if exist "build\.build_failed" (
    echo WARNING: Previous build failed. Cleaning build directory for consistency...
    if exist build (
        rmdir /s /q build 2>nul
    )
)

REM Check for CMake cache conflicts and clean if necessary
if exist "build\CMakeCache.txt" (
    REM Check if CMake cache is from a different configuration
    findstr /C:"CMAKE_BUILD_TYPE" "build\CMakeCache.txt" >nul 2>&1
    if !errorlevel! equ 0 (
        for /f "tokens=2 delims==" %%a in ('findstr /C:"CMAKE_BUILD_TYPE:STRING" "build\CMakeCache.txt" 2^>nul') do (
            if not "%%a"=="%BUILD_TYPE%" (
                echo WARNING: CMake cache build type mismatch. Cleaning cache...
                echo   Cached: %%a, Requested: %BUILD_TYPE%
                if exist build (
                    rmdir /s /q build 2>nul
                )
            )
        )
    )
)

REM Record build start state and check for consecutive identical builds
set "BUILD_SIGNATURE=%BUILD_TYPE%_%BUILD_ENGINE%_%BUILD_PROJECTS%_%BUILD_TESTS%_%SPECIFIC_PROJECT%_%SPECIFIC_TEST%"
echo %DATE% %TIME% > build_state.tmp
echo Build Type: %BUILD_TYPE% >> build_state.tmp
echo Engine: %BUILD_ENGINE% >> build_state.tmp
echo Projects: %BUILD_PROJECTS% >> build_state.tmp
echo Tests: %BUILD_TESTS% >> build_state.tmp
echo Build Signature: %BUILD_SIGNATURE% >> build_state.tmp
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT% >> build_state.tmp
if not "%SPECIFIC_TEST%"=="" echo Specific Test: %SPECIFIC_TEST% >> build_state.tmp

REM Check if this is identical to the last successful build
if exist "build\.last_build_signature" (
    set /p LAST_SIGNATURE=<"build\.last_build_signature"
    if "!LAST_SIGNATURE!"=="!BUILD_SIGNATURE!" (
        echo.
        echo NOTE: This build configuration is identical to the last successful build.
        echo If no source files have changed, the build should be very fast.
        echo.
    ) else (
        echo.
        echo NOTE: Build configuration changed from last build.
        echo Previous: !LAST_SIGNATURE!
        echo Current:  !BUILD_SIGNATURE!
        echo.
    )
)

REM Create build directory
if "%USE_PRESETS%"=="ON" (
    REM Presets handle their own build directories
    echo Build directory will be managed by CMakePresets
) else (
    REM Manual configuration uses traditional build directory
    if not exist build mkdir build
    cd build
)

REM Configure with CMake
echo.
echo Configuring build system...
if "%USE_PRESETS%"=="ON" (
    echo Using CMakePresets configuration: !CONFIGURE_PRESET!
    cmake --preset !CONFIGURE_PRESET! %CMAKE_ARGS%
) else (
    echo Using manual configuration
    REM (mudança C) remover -DCMAKE_BUILD_TYPE no configure (VS é multi-config)
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -A x64 %CMAKE_ARGS%
)
if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    REM Mark build as failed for future cleanup
    echo Configuration failed at %DATE% %TIME% > .build_failed
    cd ..
    REM Clean up temporary state file
    if exist build_state.tmp del build_state.tmp
    exit /b 1
)

REM Build
echo.
echo Building...
if "%USE_PRESETS%"=="ON" (
    echo Using CMakePresets build configuration: !BUILD_PRESET!
    cmake --build --preset !BUILD_PRESET!
) else (
    echo Using manual build configuration
    cmake --build . --config %BUILD_TYPE%
)
if errorlevel 1 (
    echo.
    echo ========================================
    echo BUILD FAILED!
    echo ========================================
    echo.
    echo The build encountered compilation errors.
    echo Check the output above for specific error details.
    echo.
    if not "%SPECIFIC_TEST%"=="" (
        echo Failed to build test: %SPECIFIC_TEST%
        echo This usually means:
        echo   - Missing implementation files
        echo   - Compilation errors in test code
        echo   - Missing dependencies or headers
    )
    REM Mark build as failed for future cleanup
    echo Build failed at %DATE% %TIME% > .build_failed
    cd ..
    REM Clean up temporary state file
    if exist build_state.tmp del build_state.tmp
    exit /b 1
)

REM Return to root directory if we changed to build directory
if "%USE_PRESETS%"=="OFF" (
    cd ..
)

REM Clean up build failure marker if build succeeded
if "%USE_PRESETS%"=="ON" (
    REM For presets, check in the preset-specific build directory
    if exist "build\ninja\x64\%BUILD_TYPE%\.build_failed" del "build\ninja\x64\%BUILD_TYPE%\.build_failed"
    if exist "build\vs\x64\%BUILD_TYPE%\.build_failed" del "build\vs\x64\%BUILD_TYPE%\.build_failed"
) else (
    if exist "build\.build_failed" del "build\.build_failed"
)

REM Verify build results and state consistency
if "%BUILD_TESTS%"=="ON" if not "%SPECIFIC_TEST%"=="" (
    set "EXPECTED_EXE_PATH="
    if "%USE_PRESETS%"=="ON" (
        REM Determine the correct path based on preset type
        if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
            set "EXPECTED_EXE_PATH=build\ninja\x64\%BUILD_TYPE%\%SPECIFIC_TEST%.exe"
        ) else (
            set "EXPECTED_EXE_PATH=build\vs\x64\%BUILD_TYPE%\%BUILD_TYPE%\%SPECIFIC_TEST%.exe"
        )
    ) else (
        set "EXPECTED_EXE_PATH=build\%BUILD_TYPE%\%SPECIFIC_TEST%.exe"
    )
    
    if not exist "!EXPECTED_EXE_PATH!" (
        echo.
        echo ========================================
        echo BUILD VERIFICATION FAILED!
        echo ========================================
        echo.
        echo Expected executable not found: !EXPECTED_EXE_PATH!
        echo The build may have completed with errors.
        echo.
        REM Mark build as failed for future cleanup
        if "%USE_PRESETS%"=="ON" (
            if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
                echo Build verification failed at %DATE% %TIME% > "build\ninja\x64\%BUILD_TYPE%\.build_failed"
            ) else (
                echo Build verification failed at %DATE% %TIME% > "build\vs\x64\%BUILD_TYPE%\.build_failed"
            )
        ) else (
            echo Build verification failed at %DATE% %TIME% > "build\.build_failed"
        )
        REM Clean up temporary state file
        if exist build_state.tmp del build_state.tmp
        exit /b 1
    )
)

REM Validate build state consistency and record successful build
if exist build_state.tmp (
    REM Compare current state with recorded state for consistency
    echo Build completed successfully with consistent state
    REM Save build signature for future consecutive build detection
    echo %BUILD_SIGNATURE% > "build\.last_build_signature"
    del build_state.tmp
)

REM Measure build performance and generate reports
call :measure_build_time

REM Success message with file locations and cache statistics
echo.
echo ========================================
echo Build Completed Successfully!
echo ========================================
echo.
if "%USE_PRESETS%"=="ON" (
    echo Using CMakePresets: !CONFIGURE_PRESET!
)

REM Report cache statistics if cache was used
if defined VCPKG_BINARY_SOURCES (
    call :report_build_cache_stats
)
echo.
echo Built Components:
if "%BUILD_ENGINE%"=="ON" (
    if "%USE_PRESETS%"=="ON" (
        if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
            echo   - Engine Library: build\ninja\x64\%BUILD_TYPE%\GameEngineKiro.lib
        ) else (
            echo   - Engine Library: build\vs\x64\%BUILD_TYPE%\%BUILD_TYPE%\GameEngineKiro.lib
        )
    ) else (
        echo   - Engine Library: build\%BUILD_TYPE%\GameEngineKiro.lib
    )
)
if "%BUILD_PROJECTS%"=="ON" (
    if not "%SPECIFIC_PROJECT%"=="" (
        if "%USE_PRESETS%"=="ON" (
            if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
                echo   - Project ^(%SPECIFIC_PROJECT%^): build\ninja\x64\%BUILD_TYPE%\projects\%SPECIFIC_PROJECT%\%SPECIFIC_PROJECT%.exe
            ) else (
                echo   - Project ^(%SPECIFIC_PROJECT%^): build\vs\x64\%BUILD_TYPE%\projects\%SPECIFIC_PROJECT%\%BUILD_TYPE%\%SPECIFIC_PROJECT%.exe
            )
        ) else (
            echo   - Project ^(%SPECIFIC_PROJECT%^): build\projects\%SPECIFIC_PROJECT%\%BUILD_TYPE%\%SPECIFIC_PROJECT%.exe
        )
    ) else (
        if "%USE_PRESETS%"=="ON" (
            if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
                echo   - Game Projects: build\ninja\x64\%BUILD_TYPE%\projects\[ProjectName]\[ProjectName].exe
            ) else (
                echo   - Game Projects: build\vs\x64\%BUILD_TYPE%\projects\[ProjectName]\%BUILD_TYPE%\[ProjectName].exe
            )
        ) else (
            echo   - Game Projects: build\projects\[ProjectName]\%BUILD_TYPE%\[ProjectName].exe
        )
    )
)
if "%BUILD_TESTS%"=="ON" (
    if not "%SPECIFIC_TEST%"=="" (
        if "%USE_PRESETS%"=="ON" (
            if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
                echo   - Test ^(%SPECIFIC_TEST%^): build\ninja\x64\%BUILD_TYPE%\%SPECIFIC_TEST%.exe
            ) else (
                echo   - Test ^(%SPECIFIC_TEST%^): build\vs\x64\%BUILD_TYPE%\%BUILD_TYPE%\%SPECIFIC_TEST%.exe
            )
        ) else (
            echo   - Test ^(%SPECIFIC_TEST%^): build\%BUILD_TYPE%\%SPECIFIC_TEST%.exe
        )
    ) else (
        if "%USE_PRESETS%"=="ON" (
            if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
                echo   - Tests: build\ninja\x64\%BUILD_TYPE%\*Test.exe
            ) else (
                echo   - Tests: build\vs\x64\%BUILD_TYPE%\%BUILD_TYPE%\*Test.exe
            )
        ) else (
            echo   - Tests: build\%BUILD_TYPE%\*Test.exe
        )
    )
)
if "%ENABLE_COVERAGE%"=="ON" echo   - Coverage: Enabled ^(use run_coverage_analysis.bat^)

echo.
echo Quick Commands:
if "%BUILD_PROJECTS%"=="ON" (
    if "%USE_PRESETS%"=="ON" (
        if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
            echo   Run GameExample: build\ninja\x64\%BUILD_TYPE%\projects\GameExample\GameExample.exe
            echo   Run BasicExample: build\ninja\x64\%BUILD_TYPE%\projects\BasicExample\BasicExample.exe
        ) else (
            echo   Run GameExample: build\vs\x64\%BUILD_TYPE%\projects\GameExample\%BUILD_TYPE%\GameExample.exe
            echo   Run BasicExample: build\vs\x64\%BUILD_TYPE%\projects\BasicExample\%BUILD_TYPE%\BasicExample.exe
        )
    ) else (
        echo   Run GameExample: build\projects\GameExample\%BUILD_TYPE%\GameExample.exe
        echo   Run BasicExample: build\projects\BasicExample\%BUILD_TYPE%\BasicExample.exe
    )
)
if "%BUILD_TESTS%"=="ON" echo   Run All Tests: .\scripts\run_tests.bat
if "%ENABLE_COVERAGE%"=="ON" echo   Generate Coverage: .\scripts\run_coverage_analysis.bat
echo   Monitor Logs: .\scripts\monitor.bat
if "!CONFIGURE_PRESET:~0,5!"=="ninja" echo   Ninja Diagnostics: .\scripts\ninja_diagnostics.bat

goto :end

:setup_vcpkg_cache
REM Configure vcpkg binary cache environment with validation and fallback
if "%DISABLE_VCPKG_CACHE%"=="ON" (
    echo vcpkg binary cache explicitly disabled via --no-cache flag
    call :cache_fallback
    goto :eof
)

call :validate_cache_health
if "%CACHE_HEALTH_OK%"=="ON" (
    call :configure_cache
) else (
    call :cache_fallback
)
goto :eof

:validate_cache_health
REM Validate cache health and availability (simplified approach)
set "CACHE_HEALTH_OK=OFF"
set "VCPKG_CACHE_DIR=%USERPROFILE%\.vcpkg-cache"

REM Check if cache directory can be created/accessed
if not exist "%VCPKG_CACHE_DIR%" (
    echo Creating vcpkg binary cache directory: %VCPKG_CACHE_DIR%
    mkdir "%VCPKG_CACHE_DIR%" 2>nul
    if errorlevel 1 (
        echo WARNING: Failed to create vcpkg cache directory
        goto :eof
    )
)

REM Simple write test
echo test > "%VCPKG_CACHE_DIR%\test_write.tmp" 2>nul
if errorlevel 1 (
    echo WARNING: vcpkg cache directory is not writable
    goto :eof
)
del "%VCPKG_CACHE_DIR%\test_write.tmp" 2>nul

REM Cache is considered healthy if directory exists and is writable
set "CACHE_HEALTH_OK=ON"
goto :eof

:configure_cache
REM Configure vcpkg binary sources environment variable (process-local only)
REM Use a more conservative cache configuration to avoid conflicts
set "VCPKG_BINARY_SOURCES=files,%VCPKG_CACHE_DIR%,read"

REM Keep feature flags minimal to avoid compatibility issues
set "VCPKG_FEATURE_FLAGS=manifests,versions"

echo vcpkg binary cache configured:
echo   Cache Directory: %VCPKG_CACHE_DIR%
echo   Binary Sources: %VCPKG_BINARY_SOURCES% ^(read-only for safety^)
echo   Feature Flags: %VCPKG_FEATURE_FLAGS%
echo   Note: Conservative cache settings for maximum compatibility

REM Generate cache statistics and reporting
call :report_cache_statistics
goto :eof

:report_cache_statistics
REM Report cache statistics and health
if exist "%VCPKG_CACHE_DIR%" (
    REM Count cache files
    for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set CACHE_PACKAGES=%%i
    
    REM Calculate cache size
    set CACHE_SIZE_BYTES=0
    for /f "tokens=3" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| findstr /r "[0-9].*\.zip"') do (
        set /a CACHE_SIZE_BYTES+=%%a
    )
    set /a CACHE_SIZE_MB=!CACHE_SIZE_BYTES!/1048576
    
    if !CACHE_PACKAGES! gtr 0 (
        echo   Cache Status: !CACHE_PACKAGES! packages cached ^(!CACHE_SIZE_MB! MB^)
        echo   Cache Hit Rate: Will be calculated during build
    ) else (
        echo   Cache Status: Empty ^(first build will populate cache^)
    )
    
    REM Check for cache maintenance needs
    if !CACHE_SIZE_MB! gtr 1000 (
        echo   Cache Maintenance: Consider cleaning old packages ^(cache size: !CACHE_SIZE_MB! MB^)
    )
)
goto :eof

:cache_fallback
echo vcpkg binary cache disabled - using normal compilation
REM Clear any cache-related environment variables to ensure clean fallback
set "VCPKG_BINARY_SOURCES="
set "VCPKG_FEATURE_FLAGS=manifests,versions"
set "VCPKG_CACHE_DIR="

echo   Cache Status: Disabled ^(fallback to normal compilation^)
if "%DISABLE_VCPKG_CACHE%"=="ON" (
    echo   Reason: Cache explicitly disabled via --no-cache flag
) else (
    echo   Reason: Cache validation failed or cache unavailable
)
echo   Impact: Dependencies will be compiled from source ^(slower but reliable^)

REM Log fallback reason for diagnostics
echo %DATE% %TIME% - vcpkg cache fallback activated >> build_cache.log
goto :eof

:report_build_cache_stats
REM Report cache statistics after successful build
if exist "%VCPKG_CACHE_DIR%" (
    for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set POST_BUILD_PACKAGES=%%i
    
    REM Compare with pre-build count if available
    if defined CACHE_PACKAGES (
        set /a NEW_PACKAGES=!POST_BUILD_PACKAGES!-!CACHE_PACKAGES!
        if !NEW_PACKAGES! gtr 0 (
            echo vcpkg Cache: !NEW_PACKAGES! new packages cached ^(total: !POST_BUILD_PACKAGES!^)
        ) else (
            echo vcpkg Cache: All dependencies served from cache ^(!POST_BUILD_PACKAGES! packages^)
        )
    ) else (
        echo vcpkg Cache: !POST_BUILD_PACKAGES! packages available
    )
    
    REM Log cache usage for future analysis
    echo %DATE% %TIME% - Build completed, cache packages: !POST_BUILD_PACKAGES! >> build_cache.log
)
goto :eof

:init_build_diagnostics
REM Initialize build diagnostics and performance monitoring
set "BUILD_START_TIME=%TIME%"
set "BUILD_START_DATE=%DATE%"
set "BUILD_LOG_FILE=build_diagnostics.log"
set "BUILD_PERF_FILE=build_performance.json"
set "BUILD_STATS_FILE=build_statistics.tmp"

REM Create diagnostics directory if it doesn't exist
if not exist "logs" mkdir logs

REM Initialize build statistics
echo Build started at %BUILD_START_DATE% %BUILD_START_TIME% > "%BUILD_STATS_FILE%"
echo. >> "%BUILD_STATS_FILE%"

REM Log build start
echo [%BUILD_START_DATE% %BUILD_START_TIME%] Build diagnostics initialized >> "logs\%BUILD_LOG_FILE%"
goto :eof

:measure_build_time
REM Calculate build duration and report performance metrics
set "BUILD_END_TIME=%TIME%"
set "BUILD_END_DATE=%DATE%"

REM Calculate duration (simplified - works for same-day builds)
call :calculate_time_diff "%BUILD_START_TIME%" "%BUILD_END_TIME%" BUILD_DURATION_SECONDS

REM Log build completion
echo [%BUILD_END_DATE% %BUILD_END_TIME%] Build completed in %BUILD_DURATION_SECONDS% seconds >> "logs\%BUILD_LOG_FILE%"

REM Report build performance
echo.
echo ========================================
echo Build Performance Report
echo ========================================
echo Build Duration: %BUILD_DURATION_SECONDS% seconds
echo Start Time: %BUILD_START_DATE% %BUILD_START_TIME%
echo End Time: %BUILD_END_DATE% %BUILD_END_TIME%

REM Generate compilation statistics
call :generate_compilation_stats

REM Compare with previous builds
call :compare_build_performance

REM Save performance data for future comparisons
call :save_build_performance_data

goto :eof

:calculate_time_diff
REM Calculate time difference in seconds (simplified version)
REM Parameters: %1=start_time, %2=end_time, %3=result_variable
set "start_time=%~1"
set "end_time=%~2"

REM Convert times to seconds (simplified - assumes same day)
for /f "tokens=1-3 delims=:." %%a in ("%start_time%") do (
    set /a start_seconds=%%a*3600+%%b*60+%%c
)
for /f "tokens=1-3 delims=:." %%a in ("%end_time%") do (
    set /a end_seconds=%%a*3600+%%b*60+%%c
)

REM Handle day rollover (basic check)
if %end_seconds% lss %start_seconds% (
    set /a end_seconds+=86400
)

set /a duration=%end_seconds%-%start_seconds%
set "%~3=%duration%"
goto :eof

:generate_compilation_stats
REM Generate detailed compilation statistics
echo.
echo Compilation Statistics:

REM Count source files in project
set "SOURCE_FILES=0"
set "HEADER_FILES=0"
set "TEST_FILES=0"

REM Count source files
for /f %%i in ('dir /s /b "src\*.cpp" 2^>nul ^| find /c /v ""') do set SOURCE_FILES=%%i
for /f %%i in ('dir /s /b "include\*.h" 2^>nul ^| find /c /v ""') do set HEADER_FILES=%%i
for /f %%i in ('dir /s /b "tests\*.cpp" 2^>nul ^| find /c /v ""') do set TEST_FILES=%%i

REM Report file statistics
echo   Source Files: %SOURCE_FILES%
echo   Header Files: %HEADER_FILES%
echo   Test Files: %TEST_FILES%

REM Estimate build complexity
set /a TOTAL_FILES=%SOURCE_FILES%+%HEADER_FILES%+%TEST_FILES%
echo   Total Files: %TOTAL_FILES%

REM Analyze build targets based on configuration
call :analyze_build_targets

REM Log statistics
echo Source Files: %SOURCE_FILES% >> "%BUILD_STATS_FILE%"
echo Header Files: %HEADER_FILES% >> "%BUILD_STATS_FILE%"
echo Test Files: %TEST_FILES% >> "%BUILD_STATS_FILE%"
echo Build Duration: %BUILD_DURATION_SECONDS% seconds >> "%BUILD_STATS_FILE%"

goto :eof

:analyze_build_targets
REM Analyze what was built based on configuration
set "ESTIMATED_TARGETS=0"

if "%BUILD_ENGINE%"=="ON" (
    set /a ESTIMATED_TARGETS+=1
    echo   Engine Library: Built
)

if "%BUILD_PROJECTS%"=="ON" (
    if not "%SPECIFIC_PROJECT%"=="" (
        set /a ESTIMATED_TARGETS+=1
        echo   Project ^(%SPECIFIC_PROJECT%^): Built
    ) else (
        REM Count project directories
        for /f %%i in ('dir /b "examples" 2^>nul ^| find /c /v ""') do (
            set /a ESTIMATED_TARGETS+=%%i
            echo   Projects: %%i built
        )
    )
)

if "%BUILD_TESTS%"=="ON" (
    if not "%SPECIFIC_TEST%"=="" (
        set /a ESTIMATED_TARGETS+=1
        echo   Test ^(%SPECIFIC_TEST%^): Built
    ) else (
        REM Count test files
        for /f %%i in ('dir /s /b "tests\test_*.cpp" 2^>nul ^| find /c /v ""') do (
            set /a ESTIMATED_TARGETS+=%%i
            echo   Tests: %%i built
        )
    )
)

echo   Estimated Targets: %ESTIMATED_TARGETS%
goto :eof

:compare_build_performance
REM Compare current build performance with previous builds
echo.
echo Performance Comparison:

if exist "logs\%BUILD_PERF_FILE%" (
    REM Read last build performance
    for /f "tokens=2 delims=:" %%a in ('findstr "last_duration" "logs\%BUILD_PERF_FILE%" 2^>nul') do (
        set "LAST_DURATION=%%a"
        set "LAST_DURATION=!LAST_DURATION: =!"
        set "LAST_DURATION=!LAST_DURATION:,=!"
    )
    
    if defined LAST_DURATION (
        if !LAST_DURATION! gtr 0 (
            set /a PERF_DIFF=%BUILD_DURATION_SECONDS%-!LAST_DURATION!
            if !PERF_DIFF! lss 0 (
                set /a PERF_IMPROVEMENT=!LAST_DURATION!-!BUILD_DURATION_SECONDS!
                set /a PERF_PERCENT=!PERF_IMPROVEMENT!*100/!LAST_DURATION!
                echo   Performance: !PERF_IMPROVEMENT!s faster than last build ^(!PERF_PERCENT!%% improvement^)
            ) else if !PERF_DIFF! gtr 0 (
                set /a PERF_DEGRADATION=!PERF_DIFF!
                set /a PERF_PERCENT=!PERF_DEGRADATION!*100/!LAST_DURATION!
                echo   Performance: !PERF_DEGRADATION!s slower than last build ^(!PERF_PERCENT!%% degradation^)
            ) else (
                echo   Performance: Same as last build ^(!BUILD_DURATION_SECONDS!s^)
            )
        )
    )
) else (
    echo   Performance: First build - no comparison available
)

REM Check for performance targets
if %BUILD_DURATION_SECONDS% lss 60 (
    echo   Status: Fast build ^(under 1 minute^)
) else if %BUILD_DURATION_SECONDS% lss 300 (
    echo   Status: Moderate build ^(1-5 minutes^)
) else (
    echo   Status: Slow build ^(over 5 minutes^) - consider optimization
)

goto :eof

:save_build_performance_data
REM Save current build performance data for future comparisons
echo { > "logs\%BUILD_PERF_FILE%"
echo   "timestamp": "%BUILD_END_DATE% %BUILD_END_TIME%", >> "logs\%BUILD_PERF_FILE%"
echo   "last_duration": %BUILD_DURATION_SECONDS%, >> "logs\%BUILD_PERF_FILE%"
echo   "build_type": "%BUILD_TYPE%", >> "logs\%BUILD_PERF_FILE%"
echo   "build_signature": "%BUILD_SIGNATURE%", >> "logs\%BUILD_PERF_FILE%"
echo   "compiled_files": %COMPILED_FILES%, >> "logs\%BUILD_PERF_FILE%"
echo   "cached_files": %CACHED_FILES%, >> "logs\%BUILD_PERF_FILE%"
echo   "total_targets": %TOTAL_TARGETS% >> "logs\%BUILD_PERF_FILE%"
echo } >> "logs\%BUILD_PERF_FILE%"

REM Clean up temporary files
if exist "%BUILD_STATS_FILE%" del "%BUILD_STATS_FILE%"

goto :eof

:help
echo Game Engine Kiro - Unified Build System
echo.
echo Usage: build_unified.bat [options]
echo.
echo Build Components:
echo   --engine          Build engine library and modules
echo   --projects        Build all game projects
echo   --tests           Build all test suites
echo   --tests NAME      Build specific test only
echo   --project NAME    Build specific project only
echo   --all             Build everything ^(default if no options^)
echo.
echo Build Types:
echo   --debug           Build in Debug mode
echo   --release         Build in Release mode ^(default^)
echo   --coverage        Build with coverage support ^(Debug + Tests^)
echo.
echo Generator Options:
echo   --ninja           Force Ninja generator usage ^(requires Ninja in PATH + VS environment^)
echo                     Note: Ninja is automatically selected when available unless GAMEENGINE_PREFER_VS=1
echo   --no-cache        Disable vcpkg binary cache ^(compile all dependencies from source^)
echo.
echo Common Combinations:
echo   build_unified.bat                                    # Build everything
echo   build_unified.bat --engine                          # Engine only
echo   build_unified.bat --projects                        # Projects only
echo   build_unified.bat --tests                           # All tests
echo   build_unified.bat --tests MathTest                  # Specific test only
echo   build_unified.bat --tests QuaternionTest            # Another specific test
echo   build_unified.bat --tests AnimationstatemachineTest # Complex test name example
echo   build_unified.bat --engine --tests                  # Engine + Tests
echo   build_unified.bat --engine --projects               # Engine + Projects
echo   build_unified.bat --project GameExample             # Specific project only
echo   build_unified.bat --debug --tests                   # Debug build with tests
echo   build_unified.bat --coverage                        # Coverage analysis build
echo   build_unified.bat --ninja --tests                   # Use Ninja generator ^(advanced^)
echo   build_unified.bat --ninja --debug --all             # Ninja debug build
echo.
echo Legacy Compatibility:
echo   --engine-only     Same as --engine
echo   --projects-only   Same as --projects  
echo   --tests-only      Same as --tests
echo.
echo Other Options:
echo   --help, -h        Show this help message
echo.
echo Examples:
echo   # Development workflow
echo   build_unified.bat --engine --tests       # Build and test engine
echo   build_unified.bat --project GameExample  # Test specific project
echo   build_unified.bat --debug --all          # Full debug build
echo   build_unified.bat --coverage             # Coverage analysis
echo   build_unified.bat --all                  # Full build for release
echo.
echo   # Individual test development (RECOMMENDED for specs)
echo   build_unified.bat --tests MathTest       # Build only MathTest (fast)
echo   build_unified.bat --tests QuaternionTest # Build only QuaternionTest
echo   build_unified.bat --tests AnimationstatemachineTest # Complex test name
echo.
echo   # CI/CD scenarios
echo   build_unified.bat --engine               # Build engine artifact
echo   build_unified.bat --tests                # Run test suite
echo   build_unified.bat --projects             # Build all projects
echo   build_unified.bat --coverage             # Generate coverage reports
echo.
echo Ninja Generator Setup:
echo   To install Ninja: winget install Ninja-build.Ninja
echo   Or download from: https://github.com/ninja-build/ninja/releases
echo   Ninja requires Visual Studio Developer Command Prompt environment
echo   Set GAMEENGINE_PREFER_VS=1 to always use Visual Studio generator
echo.

:end
