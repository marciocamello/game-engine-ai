@echo off
REM Game Engine Kiro - Test Runner Script
REM This script provides easy test execution for Windows

setlocal enabledelayedexpansion

echo ========================================
echo  Game Engine Kiro - Test Runner
echo ========================================

REM Check if build directory exists
if not exist "build" (
    echo [ERROR] Build directory not found. Please run build.bat first.
    exit /b 1
)

REM Set build configuration (default to Release)
set BUILD_CONFIG=Release
if "%1"=="debug" set BUILD_CONFIG=Debug

echo [INFO] Using build configuration: %BUILD_CONFIG%
echo.

REM Function to run a test and capture result
:run_test
set test_name=%1
set test_path=build\%BUILD_CONFIG%\%test_name%.exe

if exist "%test_path%" (
    echo [INFO] Running %test_name%...
    "%test_path%"
    if !errorlevel! equ 0 (
        echo [PASS] %test_name% completed successfully
    ) else (
        echo [FAILED] %test_name% failed with exit code !errorlevel!
        set /a failed_tests+=1
    )
    echo.
) else (
    echo [WARNING] Test executable not found: %test_path%
    set /a missing_tests+=1
    echo.
)
goto :eof

REM Initialize counters
set /a failed_tests=0
set /a missing_tests=0

REM Run unit tests
echo ========================================
echo  Running Unit Tests
echo ========================================
call :run_test MathTest

REM Run integration tests
echo ========================================
echo  Running Integration Tests
echo ========================================
call :run_test BulletIntegrationTest
call :run_test BulletConversionTest
call :run_test BulletUtilsSimpleTest
call :run_test CollisionShapeFactorySimpleTest
call :run_test PhysicsQueriesTest
call :run_test PhysicsConfigurationTest
call :run_test MovementComponentComparisonTest
call :run_test PhysicsPerformanceSimpleTest
call :run_test MemoryUsageSimpleTest
call :run_test CharacterBehaviorSimpleTest

REM Summary
echo ========================================
echo  Test Summary
echo ========================================
if %failed_tests% equ 0 (
    if %missing_tests% equ 0 (
        echo [SUCCESS] All tests passed!
        exit /b 0
    ) else (
        echo [WARNING] All available tests passed, but %missing_tests% tests were missing
        exit /b 0
    )
) else (
    echo [FAILED] %failed_tests% tests failed
    if %missing_tests% gtr 0 (
        echo [WARNING] %missing_tests% tests were missing
    )
    exit /b 1
)