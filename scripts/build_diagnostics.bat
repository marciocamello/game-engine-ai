@echo off
setlocal enabledelayedexpansion
REM Build Diagnostics and Problem Detection System
REM Analyzes build state, cache, dependencies, and common issues

echo ========================================
echo Game Engine Kiro - Build Diagnostics
echo ========================================

REM Parse command line arguments
set "COMMAND=%~1"
if "%COMMAND%"=="" set "COMMAND=analyze"

if /i "%COMMAND%"=="analyze" goto :analyze_build_state
if /i "%COMMAND%"=="cache" goto :analyze_cache_state
if /i "%COMMAND%"=="deps" goto :analyze_dependencies
if /i "%COMMAND%"=="timestamps" goto :analyze_timestamps
if /i "%COMMAND%"=="reset" goto :reset_build_state
if /i "%COMMAND%"=="help" goto :help
if /i "%COMMAND%"=="-h" goto :help

echo ERROR: Unknown command "%COMMAND%"
echo Use "build_diagnostics.bat help" for usage information
exit /b 1

:analyze_build_state
echo.
echo === Build State Analysis ===
echo.

REM Check build directory structure
call :check_build_directories

REM Check for build failures
call :check_build_failures

REM Check CMake cache consistency
call :check_cmake_cache

REM Check for incremental build issues
call :check_incremental_build_issues

REM Check build configuration consistency
call :check_build_configuration

echo.
echo === Analysis Complete ===
goto :end

:check_build_directories
echo Checking build directory structure...

if not exist "build" (
    echo   [WARNING] Build directory does not exist
    echo   Solution: Run build_unified.bat to create build directory
    goto :eof
)

REM Check for CMakePresets vs manual build conflicts
set "PRESET_BUILD_EXISTS=OFF"
set "MANUAL_BUILD_EXISTS=OFF"

if exist "build\vs" set "PRESET_BUILD_EXISTS=ON"
if exist "build\ninja" set "PRESET_BUILD_EXISTS=ON"
if exist "build\CMakeCache.txt" set "MANUAL_BUILD_EXISTS=ON"

if "%PRESET_BUILD_EXISTS%"=="ON" if "%MANUAL_BUILD_EXISTS%"=="ON" (
    echo   [WARNING] Mixed build configurations detected
    echo   Found both CMakePresets and manual build artifacts
    echo   Solution: Clean build directory and rebuild
)

REM Check for multiple build types
set "DEBUG_EXISTS=OFF"
set "RELEASE_EXISTS=OFF"

if exist "build\vs\x64\Debug" set "DEBUG_EXISTS=ON"
if exist "build\vs\x64\Release" set "RELEASE_EXISTS=ON"
if exist "build\ninja\x64\Debug" set "DEBUG_EXISTS=ON"
if exist "build\ninja\x64\Release" set "RELEASE_EXISTS=ON"
if exist "build\Debug" set "DEBUG_EXISTS=ON"
if exist "build\Release" set "RELEASE_EXISTS=ON"

if "%DEBUG_EXISTS%"=="ON" if "%RELEASE_EXISTS%"=="ON" (
    echo   [INFO] Multiple build types detected (Debug and Release)
    echo   This is normal for development
)

echo   [OK] Build directory structure checked
goto :eof

:check_build_failures
echo Checking for previous build failures...

set "FAILURE_FOUND=OFF"

REM Check for build failure markers
if exist "build\.build_failed" (
    echo   [WARNING] Build failure marker found in build directory
    for /f "delims=" %%a in (build\.build_failed) do echo   Failure: %%a
    set "FAILURE_FOUND=ON"
)

if exist "build\vs\x64\Debug\.build_failed" (
    echo   [WARNING] Debug build failure marker found
    for /f "delims=" %%a in ("build\vs\x64\Debug\.build_failed") do echo   Failure: %%a
    set "FAILURE_FOUND=ON"
)

if exist "build\vs\x64\Release\.build_failed" (
    echo   [WARNING] Release build failure marker found
    for /f "delims=" %%a in ("build\vs\x64\Release\.build_failed") do echo   Failure: %%a
    set "FAILURE_FOUND=ON"
)

if "%FAILURE_FOUND%"=="ON" (
    echo   Solution: Clean build directory or run build_unified.bat to retry
) else (
    echo   [OK] No build failure markers found
)

goto :eof

:check_cmake_cache
echo Checking CMake cache consistency...

if not exist "build\CMakeCache.txt" if not exist "build\vs\x64\Release\CMakeCache.txt" if not exist "build\ninja\x64\Release\CMakeCache.txt" (
    echo   [INFO] No CMake cache found - first build will configure from scratch
    goto :eof
)

REM Check for cache conflicts
set "CACHE_ISSUES=OFF"

REM Check manual build cache
if exist "build\CMakeCache.txt" (
    findstr /C:"CMAKE_BUILD_TYPE" "build\CMakeCache.txt" >nul 2>&1
    if !errorlevel! equ 0 (
        for /f "tokens=2 delims==" %%a in ('findstr /C:"CMAKE_BUILD_TYPE:STRING" "build\CMakeCache.txt" 2^>nul') do (
            echo   [INFO] Manual build cache found with build type: %%a
        )
    )
)

REM Check preset build caches
if exist "build\vs\x64\Release\CMakeCache.txt" (
    echo   [INFO] Visual Studio Release preset cache found
)

if exist "build\vs\x64\Debug\CMakeCache.txt" (
    echo   [INFO] Visual Studio Debug preset cache found
)

if exist "build\ninja\x64\Release\CMakeCache.txt" (
    echo   [INFO] Ninja Release preset cache found
)

if exist "build\ninja\x64\Debug\CMakeCache.txt" (
    echo   [INFO] Ninja Debug preset cache found
)

echo   [OK] CMake cache consistency checked
goto :eof

:check_incremental_build_issues
echo Checking for incremental build issues...

REM Check for timestamp issues
call :check_file_timestamps

REM Check for dependency issues
call :check_dependency_consistency

echo   [OK] Incremental build analysis complete
goto :eof

:check_file_timestamps
echo   Analyzing file timestamps...

REM Check if source files are newer than build artifacts
set "TIMESTAMP_ISSUES=OFF"

REM Find newest source file
set "NEWEST_SOURCE="
for /f "delims=" %%f in ('dir /s /b /o:d "src\*.cpp" "include\*.h" 2^>nul') do set "NEWEST_SOURCE=%%f"

if defined NEWEST_SOURCE (
    echo   Newest source file: %NEWEST_SOURCE%
    
    REM Check if any build artifacts exist and compare timestamps
    if exist "build\vs\x64\Release\Release\GameEngineKiro.lib" (
        forfiles /m "GameEngineKiro.lib" /p "build\vs\x64\Release\Release" /c "cmd /c echo   Engine library timestamp: @fdate @ftime" 2>nul
    )
    
    if exist "build\Release\GameEngineKiro.lib" (
        forfiles /m "GameEngineKiro.lib" /p "build\Release" /c "cmd /c echo   Engine library timestamp: @fdate @ftime" 2>nul
    )
)

goto :eof

:check_dependency_consistency
echo   Checking dependency consistency...

REM Check vcpkg installation
if not exist "vcpkg\scripts\buildsystems\vcpkg.cmake" (
    echo   [WARNING] vcpkg toolchain not found
    echo   Solution: Run setup_dependencies.bat or clone vcpkg
) else (
    echo   [OK] vcpkg toolchain found
)

REM Check for vcpkg.json vs installed packages
if exist "vcpkg.json" (
    echo   [INFO] vcpkg.json manifest found
    
    REM Check if vcpkg_installed directory exists
    if exist "vcpkg_installed" (
        echo   [OK] vcpkg packages appear to be installed
    ) else (
        echo   [WARNING] vcpkg packages may not be installed
        echo   Solution: Run build_unified.bat to install dependencies
    )
) else (
    echo   [WARNING] vcpkg.json manifest not found
)

goto :eof

:check_build_configuration
echo Checking build configuration consistency...

REM Check for CMakePresets.json
if exist "CMakePresets.json" (
    echo   [OK] CMakePresets.json found - modern configuration available
) else (
    echo   [INFO] CMakePresets.json not found - using manual configuration
)

REM Check for build signature consistency
if exist "build\.last_build_signature" (
    set /p LAST_SIGNATURE=<"build\.last_build_signature"
    echo   [INFO] Last successful build signature: !LAST_SIGNATURE!
) else (
    echo   [INFO] No previous build signature found
)

goto :eof

:analyze_cache_state
echo.
echo === Cache State Analysis ===
echo.

call :check_vcpkg_cache
call :check_cmake_cache_details

echo.
echo === Cache Analysis Complete ===
goto :end

:check_vcpkg_cache
echo Checking vcpkg binary cache...

set "VCPKG_CACHE_DIR=%USERPROFILE%\.vcpkg-cache"

if exist "%VCPKG_CACHE_DIR%" (
    echo   Cache Directory: %VCPKG_CACHE_DIR%
    
    REM Count cache files
    for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set CACHE_PACKAGES=%%i
    
    if !CACHE_PACKAGES! gtr 0 (
        echo   [OK] Cache contains !CACHE_PACKAGES! packages
        
        REM Calculate cache size
        set "CACHE_SIZE_BYTES=0"
        for /f "tokens=3" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| findstr /r "[0-9].*\.zip"') do (
            set /a "CACHE_SIZE_BYTES+=%%a"
        )
        set /a "CACHE_SIZE_MB=!CACHE_SIZE_BYTES!/1048576"
        echo   Cache Size: !CACHE_SIZE_MB! MB
        
        if !CACHE_SIZE_MB! gtr 1000 (
            echo   [WARNING] Cache is large ^(^>1GB^) - consider cleanup
        )
    ) else (
        echo   [INFO] Cache directory exists but is empty
    )
) else (
    echo   [INFO] No vcpkg binary cache directory found
    echo   Cache will be created on first build with cache enabled
)

goto :eof

:check_cmake_cache_details
echo Checking CMake cache details...

REM Function to analyze a specific CMake cache file
call :analyze_cmake_cache_file "build\CMakeCache.txt" "Manual Build"
call :analyze_cmake_cache_file "build\vs\x64\Release\CMakeCache.txt" "VS Release"
call :analyze_cmake_cache_file "build\vs\x64\Debug\CMakeCache.txt" "VS Debug"
call :analyze_cmake_cache_file "build\ninja\x64\Release\CMakeCache.txt" "Ninja Release"
call :analyze_cmake_cache_file "build\ninja\x64\Debug\CMakeCache.txt" "Ninja Debug"

goto :eof

:analyze_cmake_cache_file
REM Parameters: %1=cache_file_path, %2=description
if exist "%~1" (
    echo   %~2 Cache:
    
    REM Extract key information
    for /f "tokens=2 delims==" %%a in ('findstr /C:"CMAKE_BUILD_TYPE:STRING" "%~1" 2^>nul') do (
        echo     Build Type: %%a
    )
    
    for /f "tokens=2 delims==" %%a in ('findstr /C:"CMAKE_GENERATOR:INTERNAL" "%~1" 2^>nul') do (
        echo     Generator: %%a
    )
    
    for /f "tokens=2 delims==" %%a in ('findstr /C:"CMAKE_TOOLCHAIN_FILE:FILEPATH" "%~1" 2^>nul') do (
        echo     Toolchain: %%a
    )
)
goto :eof

:analyze_dependencies
echo.
echo === Dependency Analysis ===
echo.

call :check_system_dependencies
call :check_vcpkg_dependencies
call :check_build_tools

echo.
echo === Dependency Analysis Complete ===
goto :end

:check_system_dependencies
echo Checking system dependencies...

REM Check for Visual Studio
if defined VCINSTALLDIR (
    echo   [OK] Visual Studio environment detected: %VCINSTALLDIR%
) else (
    echo   [WARNING] Visual Studio environment not detected
    echo   Solution: Run from Visual Studio Developer Command Prompt
)

REM Check for CMake
cmake --version >nul 2>&1
if !errorlevel! equ 0 (
    for /f "tokens=3" %%a in ('cmake --version 2^>nul') do (
        echo   [OK] CMake found: %%a
        goto :cmake_found
    )
) else (
    echo   [ERROR] CMake not found in PATH
    echo   Solution: Install CMake and add to PATH
)
:cmake_found

REM Check for Ninja
ninja --version >nul 2>&1
if !errorlevel! equ 0 (
    for /f %%a in ('ninja --version 2^>nul') do (
        echo   [OK] Ninja found: %%a
    )
) else (
    echo   [INFO] Ninja not found (optional)
    echo   Install with: winget install Ninja-build.Ninja
)

goto :eof

:check_vcpkg_dependencies
echo Checking vcpkg dependencies...

if exist "vcpkg.json" (
    echo   [OK] vcpkg manifest found
    
    REM Parse dependencies from vcpkg.json (basic check)
    findstr /C:"dependencies" "vcpkg.json" >nul 2>&1
    if !errorlevel! equ 0 (
        echo   Dependencies defined in vcpkg.json
    )
    
    REM Check installation status
    if exist "vcpkg_installed" (
        echo   [OK] Dependencies appear to be installed
        
        REM Count installed packages
        for /f %%i in ('dir /b "vcpkg_installed\x64-windows\lib\*.lib" 2^>nul ^| find /c /v ""') do (
            if %%i gtr 0 (
                echo   Installed libraries: %%i
            )
        )
    ) else (
        echo   [WARNING] Dependencies not installed
        echo   Solution: Run build_unified.bat to install dependencies
    )
) else (
    echo   [ERROR] vcpkg.json not found
)

goto :eof

:check_build_tools
echo Checking build tools...

REM Check for MSBuild
where msbuild >nul 2>&1
if !errorlevel! equ 0 (
    for /f "tokens=*" %%a in ('msbuild -version 2^>nul ^| findstr /C:"Microsoft"') do (
        echo   [OK] MSBuild found: %%a
    )
) else (
    echo   [WARNING] MSBuild not found in PATH
)

REM Check for Git
git --version >nul 2>&1
if !errorlevel! equ 0 (
    for /f "tokens=3" %%a in ('git --version 2^>nul') do (
        echo   [OK] Git found: %%a
    )
) else (
    echo   [INFO] Git not found (optional for building)
)

goto :eof

:analyze_timestamps
echo.
echo === Timestamp Analysis ===
echo.

call :analyze_source_timestamps
call :analyze_build_timestamps
call :detect_timestamp_issues

echo.
echo === Timestamp Analysis Complete ===
goto :end

:analyze_source_timestamps
echo Analyzing source file timestamps...

REM Find newest and oldest source files
echo   Source Files:
for /f "delims=" %%f in ('dir /s /b /o:d "src\*.cpp" 2^>nul') do set "OLDEST_SOURCE=%%f"
for /f "delims=" %%f in ('dir /s /b /o:-d "src\*.cpp" 2^>nul') do set "NEWEST_SOURCE=%%f"

if defined OLDEST_SOURCE (
    echo     Oldest: %OLDEST_SOURCE%
    forfiles /m "%~nxOLDEST_SOURCE%" /p "%~dpOLDEST_SOURCE%" /c "cmd /c echo       Date: @fdate @ftime" 2>nul
)

if defined NEWEST_SOURCE (
    echo     Newest: %NEWEST_SOURCE%
    forfiles /m "%~nxNEWEST_SOURCE%" /p "%~dpNEWEST_SOURCE%" /c "cmd /c echo       Date: @fdate @ftime" 2>nul
)

REM Check header files
echo   Header Files:
for /f "delims=" %%f in ('dir /s /b /o:-d "include\*.h" 2^>nul') do (
    set "NEWEST_HEADER=%%f"
    goto :header_found
)
:header_found
if defined NEWEST_HEADER (
    echo     Newest: %NEWEST_HEADER%
    forfiles /m "%~nxNEWEST_HEADER%" /p "%~dpNEWEST_HEADER%" /c "cmd /c echo       Date: @fdate @ftime" 2>nul
)

goto :eof

:analyze_build_timestamps
echo Analyzing build artifact timestamps...

REM Check various build artifact locations
call :check_artifact_timestamp "build\Release\GameEngineKiro.lib" "Engine Library (Manual Release)"
call :check_artifact_timestamp "build\Debug\GameEngineKiro.lib" "Engine Library (Manual Debug)"
call :check_artifact_timestamp "build\vs\x64\Release\Release\GameEngineKiro.lib" "Engine Library (VS Release)"
call :check_artifact_timestamp "build\vs\x64\Debug\Debug\GameEngineKiro.lib" "Engine Library (VS Debug)"
call :check_artifact_timestamp "build\ninja\x64\Release\GameEngineKiro.lib" "Engine Library (Ninja Release)"
call :check_artifact_timestamp "build\ninja\x64\Debug\GameEngineKiro.lib" "Engine Library (Ninja Debug)"

goto :eof

:check_artifact_timestamp
REM Parameters: %1=file_path, %2=description
if exist "%~1" (
    echo   %~2:
    forfiles /m "%~nx1" /p "%~dp1" /c "cmd /c echo     Date: @fdate @ftime" 2>nul
)
goto :eof

:detect_timestamp_issues
echo Detecting timestamp-related build issues...

REM This is a simplified check - in a real implementation, you would
REM compare source file timestamps with build artifact timestamps
echo   [INFO] Timestamp issue detection is basic in this implementation
echo   For detailed dependency analysis, use build tools like Ninja with -d explain

goto :eof

:reset_build_state
echo.
echo === Resetting Build State ===
echo.

echo WARNING: This will clean the build directory and reset all build state.
set /p CONFIRM="Are you sure? (y/N): "
if /i not "%CONFIRM%"=="y" (
    echo Operation cancelled.
    goto :end
)

echo Cleaning build directory...
if exist build (
    rmdir /s /q build 2>nul
    if exist build (
        echo   [ERROR] Failed to remove build directory
        echo   Some files may be in use. Close Visual Studio and try again.
    ) else (
        echo   [OK] Build directory removed
    )
)

echo Cleaning build state files...
if exist "build\.last_build_signature" del "build\.last_build_signature"
if exist "build_state.tmp" del "build_state.tmp"
if exist "logs\build_diagnostics.log" del "logs\build_diagnostics.log"
if exist "logs\build_performance.json" del "logs\build_performance.json"

echo.
echo Build state reset complete.
echo Run build_unified.bat to rebuild from scratch.

goto :end

:help
echo Game Engine Kiro - Build Diagnostics
echo.
echo Usage: build_diagnostics.bat [command]
echo.
echo Commands:
echo   analyze      Perform comprehensive build state analysis (default)
echo   cache        Analyze cache state (vcpkg binary cache, CMake cache)
echo   deps         Analyze dependencies and build tools
echo   timestamps   Analyze file timestamps and detect issues
echo   reset        Reset build state (clean build directory)
echo   help, -h     Show this help message
echo.
echo Examples:
echo   build_diagnostics.bat                # Full analysis
echo   build_diagnostics.bat cache          # Cache analysis only
echo   build_diagnostics.bat deps           # Dependency check
echo   build_diagnostics.bat timestamps     # Timestamp analysis
echo   build_diagnostics.bat reset          # Clean build state
echo.
echo This tool helps diagnose common build issues:
echo   - Build directory structure problems
echo   - CMake cache conflicts
echo   - Dependency issues
echo   - Incremental build problems
echo   - Cache state problems
echo   - Timestamp-related issues

:end
echo.