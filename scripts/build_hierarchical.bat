@echo off
REM Hierarchical Build Script for Game Engine Kiro
REM Demonstrates the new modular build system capabilities

echo ========================================
echo Game Engine Kiro - Hierarchical Build
echo ========================================

REM Parse command line arguments
set BUILD_TYPE=all
set SPECIFIC_PROJECT=
set CMAKE_ARGS=

:parse_args
if "%1"=="" goto :build
if "%1"=="--engine-only" (
    set BUILD_TYPE=engine
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_ENGINE_ONLY=ON
    shift
    goto :parse_args
)
if "%1"=="--projects-only" (
    set BUILD_TYPE=projects
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PROJECTS_ONLY=ON
    shift
    goto :parse_args
)
if "%1"=="--tests-only" (
    set BUILD_TYPE=tests
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS_ONLY=ON
    shift
    goto :parse_args
)
if "%1"=="--project" (
    set SPECIFIC_PROJECT=%2
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SPECIFIC_PROJECT=%2
    shift
    shift
    goto :parse_args
)
if "%1"=="--help" goto :help
shift
goto :parse_args

:build
echo Build Type: %BUILD_TYPE%
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT%

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring build system...
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -A x64 %CMAKE_ARGS%
if errorlevel 1 (
    echo ERROR: CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Build failed!
    cd ..
    exit /b 1
)

cd ..
echo.
echo ========================================
echo Build completed successfully!
echo ========================================
goto :end

:help
echo Usage: build_hierarchical.bat [options]
echo.
echo Options:
echo   --engine-only     Build only the engine library and modules
echo   --projects-only   Build only game projects (requires pre-built engine)
echo   --tests-only      Build only test suites (requires pre-built engine)
echo   --project NAME    Build only the specified project
echo   --help            Show this help message
echo.
echo Examples:
echo   build_hierarchical.bat                    # Build everything
echo   build_hierarchical.bat --engine-only     # Build only engine
echo   build_hierarchical.bat --project GameExample  # Build only GameExample
echo   build_hierarchical.bat --tests-only      # Build only tests
echo.

:end