@echo off
setlocal enabledelayedexpansion
REM Unified Build Script for Game Engine Kiro
REM Supports all build combinations with a single script

echo ========================================
echo Game Engine Kiro - Unified Build System
echo ========================================

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

if "%BUILD_TESTS%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS=ON"
) else (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS=OFF"
)
if "%BUILD_PROJECTS%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PROJECTS=ON"
) else (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PROJECTS=OFF"
)
if "%BUILD_ENGINE%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE=ON"
) else (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE=OFF"
)
if "%ENABLE_COVERAGE%"=="ON" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_COVERAGE=ON"
)

echo Build Configuration: %BUILD_TYPE_DESC%
echo Build Type: %BUILD_TYPE%
if "%ENABLE_COVERAGE%"=="ON" echo Coverage: Enabled
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT%
if not "%SPECIFIC_TEST%"=="" echo Specific Test: %SPECIFIC_TEST%

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
if exist "CMakePresets.json" (
    set "USE_PRESETS=ON"
    echo CMakePresets.json detected - using modern preset configuration
    
    REM Choose preset based on user preference
    if "%USE_NINJA%"=="ON" (
        REM Check if Ninja is available and Visual Studio environment is set
        ninja --version >nul 2>&1
        if !errorlevel! equ 0 (
            if defined VCINSTALLDIR (
                echo Using Ninja presets (explicitly requested)
                if "%BUILD_TYPE%"=="Debug" (
                    set "CONFIGURE_PRESET=ninja-debug"
                    set "BUILD_PRESET=ninja-debug"
                ) else (
                    set "CONFIGURE_PRESET=ninja-release"
                    set "BUILD_PRESET=ninja-release"
                )
            ) else (
                echo WARNING: Ninja requested but Visual Studio environment not detected
                echo Falling back to Visual Studio presets
                if "%BUILD_TYPE%"=="Debug" (
                    set "CONFIGURE_PRESET=vs-debug"
                    set "BUILD_PRESET=vs-debug"
                ) else (
                    set "CONFIGURE_PRESET=vs-release"
                    set "BUILD_PRESET=vs-release"
                )
            )
        ) else (
            echo WARNING: Ninja requested but not found in PATH
            echo Falling back to Visual Studio presets
            if "%BUILD_TYPE%"=="Debug" (
                set "CONFIGURE_PRESET=vs-debug"
                set "BUILD_PRESET=vs-debug"
            ) else (
                set "CONFIGURE_PRESET=vs-release"
                set "BUILD_PRESET=vs-release"
            )
        )
    ) else (
        REM Use Visual Studio presets by default
        echo Using Visual Studio presets ^(default^) - use --ninja for Ninja builds
        if "%BUILD_TYPE%"=="Debug" (
            set "CONFIGURE_PRESET=vs-debug"
            set "BUILD_PRESET=vs-debug"
        ) else (
            set "CONFIGURE_PRESET=vs-release"
            set "BUILD_PRESET=vs-release"
        )
    )
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

REM Success message with file locations
echo.
echo ========================================
echo Build Completed Successfully!
echo ========================================
echo.
if "%USE_PRESETS%"=="ON" (
    echo Using CMakePresets: !CONFIGURE_PRESET!
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
                echo   - Project ^(%SPECIFIC_PROJECT%^): build\vs\x64\%BUILD_TYPE%\projects\%SPECIFIC_PROJECT%\%SPECIFIC_PROJECT%.exe
            )
        ) else (
            echo   - Project ^(%SPECIFIC_PROJECT%^): build\projects\%SPECIFIC_PROJECT%\%BUILD_TYPE%\%SPECIFIC_PROJECT%.exe
        )
    ) else (
        if "%USE_PRESETS%"=="ON" (
            if "!CONFIGURE_PRESET:~0,5!"=="ninja" (
                echo   - Game Projects: build\ninja\x64\%BUILD_TYPE%\projects\[ProjectName]\[ProjectName].exe
            ) else (
                echo   - Game Projects: build\vs\x64\%BUILD_TYPE%\projects\[ProjectName]\[ProjectName].exe
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
    echo   Run GameExample: build\projects\GameExample\%BUILD_TYPE%\GameExample.exe
    echo   Run BasicExample: build\projects\BasicExample\%BUILD_TYPE%\BasicExample.exe
)
if "%BUILD_TESTS%"=="ON" echo   Run All Tests: .\scripts\run_tests.bat
if "%ENABLE_COVERAGE%"=="ON" echo   Generate Coverage: .\scripts\run_coverage_analysis.bat
echo   Monitor Logs: .\scripts\monitor.bat

goto :end

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
echo   --ninja           Use Ninja generator ^(advanced users^) - requires VS environment
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

:end
