@echo off
REM Game Engine Kiro - Advanced Test Execution Script
REM This script automatically discovers and runs all test executables

setlocal enabledelayedexpansion

REM Parse command line arguments
set TEST_TYPE=all
set VERBOSE=false
set STOP_ON_FAILURE=false
set BUILD_DIR=build\Release

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="unit" set TEST_TYPE=unit
if /i "%~1"=="integration" set TEST_TYPE=integration
if /i "%~1"=="performance" set TEST_TYPE=performance
if /i "%~1"=="verbose" set VERBOSE=true
if /i "%~1"=="-v" set VERBOSE=true
if /i "%~1"=="stop" set STOP_ON_FAILURE=true
if /i "%~1"=="-s" set STOP_ON_FAILURE=true
if /i "%~1"=="debug" set BUILD_DIR=build\Debug
shift
goto :parse_args
:args_done

echo ========================================
echo  Game Engine Kiro - Test Runner
echo ========================================

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    echo [ERROR] Build directory not found: %BUILD_DIR%
    echo Please run build.bat first to compile the tests.
    exit /b 1
)

REM Initialize counters
set /a total_tests=0
set /a passed_tests=0
set /a failed_tests=0
set start_time=%time%

REM Function to run a single test executable
:run_test
set test_path=%1
set test_name=%2

if "%VERBOSE%"=="true" (
    echo [INFO] Running test: %test_name%
)

set test_start_time=%time%
"%test_path%" >nul 2>&1
set test_exit_code=%errorlevel%
set test_end_time=%time%

set /a total_tests+=1

if %test_exit_code% equ 0 (
    echo [PASS] %test_name%
    if "%VERBOSE%"=="true" (
        echo        Execution completed successfully
    )
    set /a passed_tests+=1
) else (
    echo [FAILED] %test_name% ^(Exit code: %test_exit_code%^)
    if "%VERBOSE%"=="true" (
        echo          Test failed with exit code %test_exit_code%
    )
    set /a failed_tests+=1
    if "%STOP_ON_FAILURE%"=="true" (
        echo [ERROR] Stopping execution due to test failure.
        goto :summary
    )
)
goto :eof

REM Function to discover test executables
:discover_tests
set search_pattern=%1
set test_category=%2

echo.
echo Running %test_category% Tests...
echo ----------------------------------------

for %%f in ("%BUILD_DIR%\%search_pattern%") do (
    set test_file=%%~nf
    set test_path=%%f
    
    REM Skip non-test executables
    echo !test_file! | findstr /i "GameExample CharacterControllerTest PhysicsDebugExample" >nul
    if !errorlevel! neq 0 (
        call :run_test "!test_path!" "!test_file!"
    )
)
goto :eof

REM Run tests based on type
if "%TEST_TYPE%"=="unit" (
    call :discover_tests "MathTest.exe" "Unit"
) else if "%TEST_TYPE%"=="integration" (
    call :discover_tests "*Integration*.exe" "Integration"
    call :discover_tests "*Bullet*.exe" "Integration"
    call :discover_tests "*Physics*.exe" "Integration"
    call :discover_tests "*Memory*.exe" "Integration"
    call :discover_tests "*Character*.exe" "Integration"
    call :discover_tests "*Movement*.exe" "Integration"
    call :discover_tests "*Collision*.exe" "Integration"
) else if "%TEST_TYPE%"=="performance" (
    call :discover_tests "*Performance*.exe" "Performance"
) else (
    REM Run all tests
    call :discover_tests "MathTest.exe" "Unit"
    call :discover_tests "*Integration*.exe" "Integration"
    call :discover_tests "*Bullet*.exe" "Integration"
    call :discover_tests "*Physics*.exe" "Integration"
    call :discover_tests "*Memory*.exe" "Integration"
    call :discover_tests "*Character*.exe" "Integration"
    call :discover_tests "*Movement*.exe" "Integration"
    call :discover_tests "*Collision*.exe" "Integration"
    call :discover_tests "*Performance*.exe" "Performance"
)

:summary
set end_time=%time%

echo.
echo ========================================
echo  Test Execution Summary
echo ========================================
echo Total Tests: %total_tests%
echo Passed: %passed_tests%
echo Failed: %failed_tests%
echo Build Directory: %BUILD_DIR%

if %failed_tests% equ 0 (
    echo.
    echo [SUCCESS] ALL TESTS PASSED!
    echo ========================================
    exit /b 0
) else (
    echo.
    echo [FAILED] %failed_tests% TEST^(S^) FAILED!
    echo ========================================
    exit /b 1
)