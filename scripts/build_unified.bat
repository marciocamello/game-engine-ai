@echo off
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

REM Parse command line arguments
:parse_args
if "%1"=="" goto :determine_build_type

if "%1"=="--engine" (
    set BUILD_ENGINE=ON
    shift
    goto :parse_args
)
if "%1"=="--projects" (
    set BUILD_PROJECTS=ON
    shift
    goto :parse_args
)
if "%1"=="--tests" (
    set BUILD_TESTS=ON
    REM Check if next argument is a test name (doesn't start with --)
    if not "%2"=="" (
        echo %2 | findstr /r "^--" >nul
        if errorlevel 1 (
            set SPECIFIC_TEST=%2
            set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SPECIFIC_TEST=%2
            shift
        )
    )
    shift
    goto :parse_args
)
if "%1"=="--project" (
    set SPECIFIC_PROJECT=%2
    set BUILD_PROJECTS=ON
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SPECIFIC_PROJECT=%2
    shift
    shift
    goto :parse_args
)

if "%1"=="--all" (
    set BUILD_ENGINE=ON
    set BUILD_PROJECTS=ON
    set BUILD_TESTS=ON
    shift
    goto :parse_args
)
if "%1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if "%1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto :parse_args
)
if "%1"=="--coverage" (
    set BUILD_TYPE=Debug
    set ENABLE_COVERAGE=ON
    set BUILD_ENGINE=ON
    set BUILD_TESTS=ON
    shift
    goto :parse_args
)
if "%1"=="--help" goto :help
if "%1"=="-h" goto :help

REM Legacy compatibility
if "%1"=="--engine-only" (
    set BUILD_ENGINE=ON
    shift
    goto :parse_args
)
if "%1"=="--projects-only" (
    set BUILD_PROJECTS=ON
    shift
    goto :parse_args
)
if "%1"=="--tests-only" (
    set BUILD_TESTS=ON
    shift
    goto :parse_args
)

shift
goto :parse_args

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
set BUILD_TYPE_DESC=
if "%BUILD_ENGINE%"=="ON" set BUILD_TYPE_DESC=%BUILD_TYPE_DESC%Engine
if "%BUILD_PROJECTS%"=="ON" (
    if not "%BUILD_TYPE_DESC%"=="" set BUILD_TYPE_DESC=%BUILD_TYPE_DESC% + 
    if not "%SPECIFIC_PROJECT%"=="" (
        set BUILD_TYPE_DESC=%BUILD_TYPE_DESC%Project ^(%SPECIFIC_PROJECT%^)
    ) else (
        set BUILD_TYPE_DESC=%BUILD_TYPE_DESC%Projects
    )
)
if "%BUILD_TESTS%"=="ON" (
    if not "%BUILD_TYPE_DESC%"=="" set BUILD_TYPE_DESC=%BUILD_TYPE_DESC% + 
    if not "%SPECIFIC_TEST%"=="" (
        set BUILD_TYPE_DESC=%BUILD_TYPE_DESC%Test ^(%SPECIFIC_TEST%^)
    ) else (
        set BUILD_TYPE_DESC=%BUILD_TYPE_DESC%Tests
    )
)

:setup_cmake_args
REM Setup CMake arguments based on what to build
if "%BUILD_TESTS%"=="ON" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS=ON
) else (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS=OFF
)
if "%BUILD_PROJECTS%"=="ON" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PROJECTS=ON
) else (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PROJECTS=OFF
)
if "%BUILD_ENGINE%"=="ON" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE=ON
) else (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE=OFF
)
if "%ENABLE_COVERAGE%"=="ON" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_COVERAGE=ON
)
if not "%SPECIFIC_TEST%"=="" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SPECIFIC_TEST=%SPECIFIC_TEST%
)

echo Build Configuration: %BUILD_TYPE_DESC%
echo Build Type: %BUILD_TYPE%
if "%ENABLE_COVERAGE%"=="ON" echo Coverage: Enabled
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT%
if not "%SPECIFIC_TEST%"=="" echo Specific Test: %SPECIFIC_TEST%

REM Verify dependencies
echo.
echo Verifying dependencies...
if not exist "vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo WARNING: vcpkg toolchain not found. Some dependencies may not be available.
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring build system...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -A x64 %CMAKE_ARGS%
if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

cd ..

REM Success message with file locations
echo.
echo ========================================
echo Build Completed Successfully!
echo ========================================
echo.
echo Built Components:
if "%BUILD_ENGINE%"=="ON" echo   - Engine Library: build\%BUILD_TYPE%\GameEngineKiro.lib
if "%BUILD_PROJECTS%"=="ON" (
    if not "%SPECIFIC_PROJECT%"=="" (
        echo   - Project ^(%SPECIFIC_PROJECT%^): build\projects\%SPECIFIC_PROJECT%\%BUILD_TYPE%\%SPECIFIC_PROJECT%.exe
    ) else (
        echo   - Game Projects: build\projects\[ProjectName]\%BUILD_TYPE%\[ProjectName].exe
    )
)
if "%BUILD_TESTS%"=="ON" (
    if not "%SPECIFIC_TEST%"=="" (
        echo   - Test ^(%SPECIFIC_TEST%^): build\%BUILD_TYPE%\%SPECIFIC_TEST%.exe
    ) else (
        echo   - Tests: build\%BUILD_TYPE%\*Test.exe
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
echo Common Combinations:
echo   build_unified.bat                                    # Build everything
echo   build_unified.bat --engine                          # Engine only
echo   build_unified.bat --projects                        # Projects only
echo   build_unified.bat --tests                           # All tests
echo   build_unified.bat --tests ModuleerrorhandlingintegrationIntegrationTest  # Specific test only
echo   build_unified.bat --engine --tests                  # Engine + Tests
echo   build_unified.bat --engine --projects               # Engine + Projects
echo   build_unified.bat --project GameExample             # Specific project only
echo   build_unified.bat --debug --tests                   # Debug build with tests
echo   build_unified.bat --coverage                        # Coverage analysis build
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
echo   # CI/CD scenarios
echo   build_unified.bat --engine               # Build engine artifact
echo   build_unified.bat --tests                # Run test suite
echo   build_unified.bat --projects             # Build all projects
echo   build_unified.bat --coverage             # Generate coverage reports
echo.

:end