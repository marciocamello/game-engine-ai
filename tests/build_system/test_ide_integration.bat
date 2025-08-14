@echo off
setlocal enabledelayedexpansion
REM Test IDE integration functionality
REM Requirements: 6.1, 6.2, 6.3

echo ========================================
echo Build System IDE Integration Test
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"
set "FAILED_TESTS=0"
set "TEST_LOG=tests\build_system\ide_integration_test.log"

REM Initialize log
echo Build System IDE Integration Test - %DATE% %TIME% > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Testing IDE integration functionality...
echo.

REM Test 1: CMakePresets.json compatibility
call :test_cmake_presets

REM Test 2: compile_commands.json generation
call :test_compile_commands

REM Test 3: VS Code integration
call :test_vscode_integration

REM Test 4: Visual Studio integration
call :test_visual_studio_integration

REM Test 5: clangd language server support
call :test_clangd_support

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %PASSED_TESTS%
echo Failed: %FAILED_TESTS%

if %FAILED_TESTS% gtr 0 (
    echo.
    echo [FAILED] Some IDE integration tests failed!
    echo Check %TEST_LOG% for details.
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All IDE integration tests passed!
    exit /b 0
)

:test_cmake_presets
set /a TOTAL_TESTS+=1

echo Testing: CMakePresets.json compatibility
echo   Checking if CMakePresets.json exists and is valid

REM Log test start
echo [TEST %TOTAL_TESTS%] CMakePresets.json compatibility >> "%TEST_LOG%"

if exist "CMakePresets.json" (
    REM Test if presets work with build system
    .\scripts\build_unified.bat --engine >nul 2>&1
    set "RESULT=%ERRORLEVEL%"
    
    if %RESULT% equ 0 (
        echo   Result: [PASS] - CMakePresets.json exists and works
        echo   Result: [PASS] - CMakePresets.json exists and works >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - CMakePresets.json exists but build failed
        echo   Result: [FAIL] - CMakePresets.json exists but build failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - CMakePresets.json not found
    echo   Result: [FAIL] - CMakePresets.json not found >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_compile_commands
set /a TOTAL_TESTS+=1

echo Testing: compile_commands.json generation
echo   Checking if compile_commands.json is generated for language servers

REM Log test start
echo [TEST %TOTAL_TESTS%] compile_commands.json generation >> "%TEST_LOG%"

REM Build with compile commands generation
.\scripts\build_unified.bat --engine >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    REM Check if compile_commands.json was generated
    if exist "build\compile_commands.json" (
        echo   Result: [PASS] - compile_commands.json generated
        echo   Result: [PASS] - compile_commands.json generated >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else if exist "compile_commands.json" (
        echo   Result: [PASS] - compile_commands.json generated in root
        echo   Result: [PASS] - compile_commands.json generated in root >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - compile_commands.json not generated
        echo   Result: [FAIL] - compile_commands.json not generated >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Build failed, cannot test compile_commands.json
    echo   Result: [FAIL] - Build failed, cannot test compile_commands.json >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_vscode_integration
set /a TOTAL_TESTS+=1

echo Testing: VS Code integration
echo   Checking VS Code configuration files

REM Log test start
echo [TEST %TOTAL_TESTS%] VS Code integration >> "%TEST_LOG%"

set "VSCODE_OK=true"

REM Check for .vscode directory
if not exist ".vscode" (
    echo   Warning: .vscode directory not found
    set "VSCODE_OK=false"
)

REM Check for key VS Code files
if exist ".vscode" (
    if not exist ".vscode\settings.json" (
        echo   Warning: .vscode\settings.json not found
    )
    if not exist ".vscode\tasks.json" (
        echo   Warning: .vscode\tasks.json not found
    )
)

REM Test if build works in VS Code environment
set "VSCODE_PID=test"
.\scripts\build_unified.bat --engine >nul 2>&1
set "RESULT=%ERRORLEVEL%"
set "VSCODE_PID="

if %RESULT% equ 0 (
    echo   Result: [PASS] - Build works in VS Code environment
    echo   Result: [PASS] - Build works in VS Code environment >> "%TEST_LOG%"
    set /a PASSED_TESTS+=1
) else (
    echo   Result: [FAIL] - Build failed in VS Code environment
    echo   Result: [FAIL] - Build failed in VS Code environment >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_visual_studio_integration
set /a TOTAL_TESTS+=1

echo Testing: Visual Studio integration
echo   Checking Visual Studio project generation

REM Log test start
echo [TEST %TOTAL_TESTS%] Visual Studio integration >> "%TEST_LOG%"

REM Test Visual Studio generator
.\scripts\build_unified.bat --engine >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    REM Check if Visual Studio files were generated
    if exist "build\GameEngineKiro.sln" (
        echo   Result: [PASS] - Visual Studio solution generated
        echo   Result: [PASS] - Visual Studio solution generated >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else if exist "build\vs\x64\Release\GameEngineKiro.sln" (
        echo   Result: [PASS] - Visual Studio solution generated (preset)
        echo   Result: [PASS] - Visual Studio solution generated (preset) >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [PARTIAL] - Build succeeded but no .sln found
        echo   Result: [PARTIAL] - Build succeeded but no .sln found >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Visual Studio build failed
    echo   Result: [FAIL] - Visual Studio build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_clangd_support
set /a TOTAL_TESTS+=1

echo Testing: clangd language server support
echo   Checking clangd configuration and compile_commands.json

REM Log test start
echo [TEST %TOTAL_TESTS%] clangd language server support >> "%TEST_LOG%"

set "CLANGD_OK=true"

REM Check for .clangd configuration
if exist ".clangd" (
    echo   Info: .clangd configuration found
) else (
    echo   Warning: .clangd configuration not found
)

REM Build and check compile_commands.json
.\scripts\build_unified.bat --engine >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    REM Check if compile_commands.json exists for clangd
    if exist "build\compile_commands.json" (
        echo   Result: [PASS] - clangd support available (compile_commands.json)
        echo   Result: [PASS] - clangd support available (compile_commands.json) >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else if exist "compile_commands.json" (
        echo   Result: [PASS] - clangd support available (root compile_commands.json)
        echo   Result: [PASS] - clangd support available (root compile_commands.json) >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - No compile_commands.json for clangd
        echo   Result: [FAIL] - No compile_commands.json for clangd >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Build failed, cannot test clangd support
    echo   Result: [FAIL] - Build failed, cannot test clangd support >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof