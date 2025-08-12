@echo off
REM Game Engine Kiro - Test Runner Script
REM This script provides easy test execution for Windows

echo ========================================
echo  Game Engine Kiro - Test Runner
echo ========================================

REM Check if build directory exists
if not exist "build" (
    echo [ERROR] Build directory not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

REM Set build configuration (default to Release)
set BUILD_CONFIG=Release
if "%1"=="debug" set BUILD_CONFIG=Debug

echo [INFO] Using build configuration: %BUILD_CONFIG%
echo.

REM Run unit tests
echo ========================================
echo  Running Unit Tests
echo ========================================

if exist "build\%BUILD_CONFIG%\MathTest.exe" (
    echo [INFO] Running MathTest...
    build\%BUILD_CONFIG%\MathTest.exe
    if errorlevel 1 (
        echo [FAILED] MathTest failed
        goto :test_failed
    ) else (
        echo [PASS] MathTest completed successfully
    )
    echo.
) else (
    echo [WARNING] Test executable not found: build\%BUILD_CONFIG%\MathTest.exe
    echo.
)

REM Run integration tests
echo ========================================
echo  Running Integration Tests
echo ========================================

for %%t in (
    BulletIntegrationTest
    BulletConversionTest
    BulletUtilsSimpleTest
    CollisionShapeFactorySimpleTest
    PhysicsQueriesTest
    PhysicsConfigurationTest
    MovementComponentComparisonTest
    PhysicsPerformanceSimpleTest
    MemoryUsageSimpleTest
    CharacterBehaviorSimpleTest
) do (
    if exist "build\%BUILD_CONFIG%\%%t.exe" (
        echo [INFO] Running %%t...
        build\%BUILD_CONFIG%\%%t.exe
        if errorlevel 1 (
            echo [FAILED] %%t failed
            goto :test_failed
        ) else (
            echo [PASS] %%t completed successfully
        )
        echo.
    ) else (
        echo [WARNING] Test executable not found: build\%BUILD_CONFIG%\%%t.exe
        echo.
    )
)

REM Summary
echo ========================================
echo  Test Summary
echo ========================================
echo [SUCCESS] All tests passed!
exit /b 0

:test_failed
echo ========================================
echo  Test Summary
echo ========================================
echo [FAILED] One or more tests failed
exit /b 1