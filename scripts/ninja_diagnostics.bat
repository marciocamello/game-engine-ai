@echo off
setlocal enabledelayedexpansion
REM Ninja Build Diagnostics Script for Game Engine Kiro
REM Provides detailed information about Ninja builds and performance

echo ========================================
echo Game Engine Kiro - Ninja Diagnostics
echo ========================================

REM Check if Ninja is available
ninja --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Ninja not found in PATH
    echo To install Ninja: winget install Ninja-build.Ninja
    echo Or download from: https://github.com/ninja-build/ninja/releases
    exit /b 1
)

REM Get Ninja version
for /f "tokens=*" %%i in ('ninja --version 2^>nul') do set NINJA_VERSION=%%i
echo Ninja Version: %NINJA_VERSION%

REM Check for Visual Studio environment
if defined VCINSTALLDIR (
    echo Visual Studio Environment: Available
    echo   VCINSTALLDIR: %VCINSTALLDIR%
) else (
    echo Visual Studio Environment: Not detected
    echo WARNING: Ninja requires MSVC compiler environment
    echo Please run from Visual Studio Developer Command Prompt
)

REM Check for CMakePresets.json
if exist "CMakePresets.json" (
    echo CMakePresets.json: Found
) else (
    echo CMakePresets.json: Not found
    echo WARNING: CMakePresets.json is required for optimal Ninja configuration
)

echo.
echo Available Ninja Build Directories:
if exist "build\ninja" (
    for /d %%d in ("build\ninja\x64\*") do (
        echo   - %%~nxd
    )
) else (
    echo   None found - run build_unified.bat --ninja first
)

echo.
echo Ninja Build Commands:
echo   Basic build:     ninja -C build\ninja\x64\Release
echo   Parallel build:  ninja -j 8 -C build\ninja\x64\Release
echo   Explain rebuild: ninja -d explain -C build\ninja\x64\Release
echo   List targets:    ninja -t targets -C build\ninja\x64\Release
echo   Dependency graph: ninja -t graph -C build\ninja\x64\Release ^| dot -Tpng -o deps.png
echo   Build stats:     ninja -t compdb -C build\ninja\x64\Release
echo   Clean build:     ninja -t clean -C build\ninja\x64\Release

REM Parse command line arguments
set COMMAND=%1
set BUILD_DIR=%2

if "%COMMAND%"=="" goto :help
if "%BUILD_DIR%"=="" set BUILD_DIR=build\ninja\x64\Release

REM Validate build directory
if not exist "%BUILD_DIR%" (
    echo ERROR: Build directory not found: %BUILD_DIR%
    echo Available directories:
    if exist "build\ninja" (
        for /d %%d in ("build\ninja\x64\*") do echo   - build\ninja\x64\%%~nxd
    )
    exit /b 1
)

if /i "%COMMAND%"=="explain" (
    echo.
    echo Running Ninja explain for: %BUILD_DIR%
    echo This will show why files need to be rebuilt:
    echo.
    ninja -d explain -C "%BUILD_DIR%"
    goto :end
)

if /i "%COMMAND%"=="targets" (
    echo.
    echo Available targets in: %BUILD_DIR%
    echo.
    ninja -t targets -C "%BUILD_DIR%"
    goto :end
)

if /i "%COMMAND%"=="graph" (
    echo.
    echo Generating dependency graph for: %BUILD_DIR%
    echo Output will be saved as deps.dot
    echo.
    ninja -t graph -C "%BUILD_DIR%" > deps.dot
    echo Dependency graph saved to deps.dot
    echo To convert to PNG: dot -Tpng deps.dot -o deps.png
    goto :end
)

if /i "%COMMAND%"=="stats" (
    echo.
    echo Build statistics for: %BUILD_DIR%
    echo.
    ninja -t compdb -C "%BUILD_DIR%" | findstr /c:"command" | find /c """"
    echo compilation commands found
    goto :end
)

if /i "%COMMAND%"=="clean" (
    echo.
    echo Cleaning build directory: %BUILD_DIR%
    echo.
    ninja -t clean -C "%BUILD_DIR%"
    echo Clean completed
    goto :end
)

if /i "%COMMAND%"=="build" (
    echo.
    echo Building with Ninja in: %BUILD_DIR%
    echo Using 8 parallel jobs
    echo.
    ninja -j 8 -C "%BUILD_DIR%"
    goto :end
)

:help
echo.
echo Usage: ninja_diagnostics.bat [command] [build_dir]
echo.
echo Commands:
echo   explain    Show why files need to be rebuilt
echo   targets    List all available build targets
echo   graph      Generate dependency graph (saves to deps.dot)
echo   stats      Show build statistics
echo   clean      Clean build directory
echo   build      Build with optimal parallel settings
echo.
echo Build Directories (optional, defaults to build\ninja\x64\Release):
echo   build\ninja\x64\Debug
echo   build\ninja\x64\Release
echo   build\ninja\x64\RelWithDebInfo
echo.
echo Examples:
echo   ninja_diagnostics.bat explain
echo   ninja_diagnostics.bat targets build\ninja\x64\Debug
echo   ninja_diagnostics.bat build
echo   ninja_diagnostics.bat clean build\ninja\x64\Debug

:end