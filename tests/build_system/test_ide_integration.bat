@echo off
setlocal enabledelayedexpansion
REM IDE Integration Test Suite
REM Tests that all IDE integrations remain functional
REM Requirements: 6.1, 6.2, 6.3

echo ========================================
echo IDE Integration Compatibility Tests
echo ========================================

set "TEST_COUNT=0"
set "PASSED_COUNT=0"
set "FAILED_COUNT=0"
set "TEST_LOG=logs\ide_integration_test.log"

REM Create logs directory if it doesn't exist
if not exist "logs" mkdir logs

REM Initialize test log
echo [%DATE% %TIME%] Starting IDE integration tests > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Starting IDE integration validation...
echo Test results will be logged to: %TEST_LOG%
echo.

REM Test 1: CMakePresets.json exists and is valid
call :test_file_exists "CMakePresets.json exists" "CMakePresets.json"

REM Test 2: compile_commands.json generation
call :run_test "Generate compile_commands.json" ".\scripts\build_unified.bat --engine"
call :test_file_exists "compile_commands.json generated" "build\compile_commands.json"

REM Test 3: VS Code configuration files
call :test_file_exists ".vscode/settings.json exists" ".vscode\settings.json"
call :test_file_exists ".vscode/tasks.json exists" ".vscode\tasks.json"
call :test_file_exists ".vscode/launch.json exists" ".vscode\launch.json"

REM Test 4: clangd configuration
call :test_file_exists ".clangd config exists" ".clangd"

REM Test 5: Visual Studio solution generation (if using VS presets)
if exist "CMakePresets.json" (
    findstr /C:"vs-" "CMakePresets.json" >nul 2>&1
    if !errorlevel! equ 0 (
        call :run_test "Visual Studio preset configuration" "cmake --preset vs-release"
        call :test_file_exists "Visual Studio solution generated" "build\vs\x64\Release\GameEngineKiro.sln"
    ) else (
        echo [SKIP] Visual Studio preset test - VS presets not configured
        echo [%DATE% %TIME%] SKIP: Visual Studio preset test - VS presets not configured >> "%TEST_LOG%"
    )
) else (
    echo [SKIP] CMakePresets.json not found - skipping preset tests
    echo [%DATE% %TIME%] SKIP: CMakePresets.json not found - skipping preset tests >> "%TEST_LOG%"
)

REM Test 6: Ninja preset configuration (if available)
ninja --version >nul 2>&1
if !errorlevel! equ 0 (
    if exist "CMakePresets.json" (
        findstr /C:"ninja-" "CMakePresets.json" >nul 2>&1
        if !errorlevel! equ 0 (
            call :run_test "Ninja preset configuration" "cmake --preset ninja-release"
            call :test_file_exists "Ninja build files generated" "build\ninja\x64\Release\build.ninja"
        ) else (
            echo [SKIP] Ninja preset test - Ninja presets not configured
            echo [%DATE% %TIME%] SKIP: Ninja preset test - Ninja presets not configured >> "%TEST_LOG%"
        )
    )
) else (
    echo [SKIP] Ninja preset test - Ninja not available
    echo [%DATE% %TIME%] SKIP: Ninja preset test - Ninja not available >> "%TEST_LOG%"
)

REM Test 7: vcpkg integration
call :test_file_exists "vcpkg.json manifest exists" "vcpkg.json"
call :test_file_exists "vcpkg toolchain exists" "vcpkg\scripts\buildsystems\vcpkg.cmake"

REM Test 8: Test discovery for IDEs
call :run_test "Build tests for IDE discovery" ".\scripts\build_unified.bat --tests"
call :test_pattern_exists "Unit tests discoverable" "build" "*Test.exe"
call :test_pattern_exists "Integration tests discoverable" "build" "*IntegrationTest.exe"

REM Test 9: Asset copying for debugging
call :test_file_exists "Assets copied to build" "build\assets\README.md"

REM Test 10: Debug symbols generation
call :run_test "Debug build with symbols" ".\scripts\build_unified.bat --debug --engine"
call :test_pattern_exists "Debug symbols generated" "build" "*.pdb"

REM Test 11: IntelliSense support files
call :test_file_exists "CMake cache for IntelliSense" "build\CMakeCache.txt"

REM Test 12: Project structure validation
call :test_directory_exists "Include directory structure" "include"
call :test_directory_exists "Source directory structure" "src"
call :test_directory_exists "Tests directory structure" "tests"
call :test_directory_exists "Scripts directory structure" "scripts"

REM Test 13: Environment variable compatibility
set "KIRO_IDE_SESSION=1"
call :run_test "IDE session environment" ".\scripts\build_unified.bat --engine"
set "KIRO_IDE_SESSION="

REM Test 14: VS Code environment compatibility
set "VSCODE_PID=12345"
call :run_test "VS Code environment" ".\scripts\build_unified.bat --engine"
set "VSCODE_PID="

REM Final results
echo.
echo ========================================
echo IDE Integration Test Results
echo ========================================
echo Total Tests: %TEST_COUNT%
echo Passed: %PASSED_COUNT%
echo Failed: %FAILED_COUNT%

if %FAILED_COUNT% equ 0 (
    echo.
    echo [SUCCESS] All IDE integration tests passed!
    echo All IDE integrations remain functional.
    echo [%DATE% %TIME%] SUCCESS: All %TEST_COUNT% IDE integration tests passed >> "%TEST_LOG%"
    exit /b 0
) else (
    echo.
    echo [FAILED] %FAILED_COUNT% IDE integration tests failed!
    echo Some IDE integrations may be broken.
    echo Check the log file for details: %TEST_LOG%
    echo [%DATE% %TIME%] FAILED: %FAILED_COUNT% out of %TEST_COUNT% tests failed >> "%TEST_LOG%"
    exit /b 1
)

:run_test
REM Function to run a single test
REM Parameters: %1=test_name, %2=command
set /a TEST_COUNT+=1
set "test_name=%~1"
set "command=%~2"

echo [TEST %TEST_COUNT%] %test_name%...
echo [%DATE% %TIME%] TEST %TEST_COUNT%: %test_name% >> "%TEST_LOG%"
echo [%DATE% %TIME%] Command: %command% >> "%TEST_LOG%"

REM Run the command and capture result
%command% >nul 2>&1
set "result=!errorlevel!"

if !result! equ 0 (
    echo [PASS] %test_name%
    echo [%DATE% %TIME%] PASS: %test_name% >> "%TEST_LOG%"
    set /a PASSED_COUNT+=1
) else (
    echo [FAIL] %test_name% (exit code: !result!)
    echo [%DATE% %TIME%] FAIL: %test_name% (exit code: !result!) >> "%TEST_LOG%"
    set /a FAILED_COUNT+=1
)

echo. >> "%TEST_LOG%"
goto :eof

:test_file_exists
REM Function to test if a file exists
REM Parameters: %1=test_name, %2=file_path
set /a TEST_COUNT+=1
set "test_name=%~1"
set "file_path=%~2"

echo [TEST %TEST_COUNT%] %test_name%...
echo [%DATE% %TIME%] TEST %TEST_COUNT%: %test_name% >> "%TEST_LOG%"
echo [%DATE% %TIME%] File: %file_path% >> "%TEST_LOG%"

if exist "%file_path%" (
    echo [PASS] %test_name%
    echo [%DATE% %TIME%] PASS: %test_name% >> "%TEST_LOG%"
    set /a PASSED_COUNT+=1
) else (
    echo [FAIL] %test_name% - File not found: %file_path%
    echo [%DATE% %TIME%] FAIL: %test_name% - File not found: %file_path% >> "%TEST_LOG%"
    set /a FAILED_COUNT+=1
)

echo. >> "%TEST_LOG%"
goto :eof

:test_directory_exists
REM Function to test if a directory exists
REM Parameters: %1=test_name, %2=directory_path
set /a TEST_COUNT+=1
set "test_name=%~1"
set "directory_path=%~2"

echo [TEST %TEST_COUNT%] %test_name%...
echo [%DATE% %TIME%] TEST %TEST_COUNT%: %test_name% >> "%TEST_LOG%"
echo [%DATE% %TIME%] Directory: %directory_path% >> "%TEST_LOG%"

if exist "%directory_path%" (
    echo [PASS] %test_name%
    echo [%DATE% %TIME%] PASS: %test_name% >> "%TEST_LOG%"
    set /a PASSED_COUNT+=1
) else (
    echo [FAIL] %test_name% - Directory not found: %directory_path%
    echo [%DATE% %TIME%] FAIL: %test_name% - Directory not found: %directory_path% >> "%TEST_LOG%"
    set /a FAILED_COUNT+=1
)

echo. >> "%TEST_LOG%"
goto :eof

:test_pattern_exists
REM Function to test if files matching a pattern exist
REM Parameters: %1=test_name, %2=search_path, %3=pattern
set /a TEST_COUNT+=1
set "test_name=%~1"
set "search_path=%~2"
set "pattern=%~3"

echo [TEST %TEST_COUNT%] %test_name%...
echo [%DATE% %TIME%] TEST %TEST_COUNT%: %test_name% >> "%TEST_LOG%"
echo [%DATE% %TIME%] Pattern: %search_path%\%pattern% >> "%TEST_LOG%"

dir /s /b "%search_path%\%pattern%" >nul 2>&1
if !errorlevel! equ 0 (
    echo [PASS] %test_name%
    echo [%DATE% %TIME%] PASS: %test_name% >> "%TEST_LOG%"
    set /a PASSED_COUNT+=1
) else (
    echo [FAIL] %test_name% - No files matching pattern: %search_path%\%pattern%
    echo [%DATE% %TIME%] FAIL: %test_name% - No files matching pattern: %search_path%\%pattern% >> "%TEST_LOG%"
    set /a FAILED_COUNT+=1
)

echo. >> "%TEST_LOG%"
goto :eof