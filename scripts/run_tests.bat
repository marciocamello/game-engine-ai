@echo off
REM Game Engine Kiro - Test Runner Script
REM This script runs all available test executables

echo ========================================
echo  Game Engine Kiro - Test Runner
echo ========================================

REM Check if build directory exists
if not exist "build\Release" (
    echo [ERROR] Build directory not found. Please run scripts\build.bat first.
    exit /b 1
)

echo.
echo Running Unit Tests...
echo ----------------------------------------

REM Run Math unit test
if exist "build\Release\MathTest.exe" (
    echo [INFO] Running MathTest...
    build\Release\MathTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MathTest
        goto :test_failed
    ) else (
        echo [PASS] MathTest
    )
)

echo.
echo Running Integration Tests...
echo ----------------------------------------

REM Run BulletUtilsSimpleTest
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    echo [INFO] Running BulletUtilsSimpleTest...
    build\Release\BulletUtilsSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletUtilsSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] BulletUtilsSimpleTest
    )
)

REM Run BulletIntegrationTest
if exist "build\Release\BulletIntegrationTest.exe" (
    echo [INFO] Running BulletIntegrationTest...
    build\Release\BulletIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletIntegrationTest
        goto :test_failed
    ) else (
        echo [PASS] BulletIntegrationTest
    )
)

REM Run BulletConversionTest
if exist "build\Release\BulletConversionTest.exe" (
    echo [INFO] Running BulletConversionTest...
    build\Release\BulletConversionTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletConversionTest
        goto :test_failed
    ) else (
        echo [PASS] BulletConversionTest
    )
)

REM Run CollisionShapeFactorySimpleTest
if exist "build\Release\CollisionShapeFactorySimpleTest.exe" (
    echo [INFO] Running CollisionShapeFactorySimpleTest...
    build\Release\CollisionShapeFactorySimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] CollisionShapeFactorySimpleTest
        goto :test_failed
    ) else (
        echo [PASS] CollisionShapeFactorySimpleTest
    )
)

REM Run PhysicsQueriesTest
if exist "build\Release\PhysicsQueriesTest.exe" (
    echo [INFO] Running PhysicsQueriesTest...
    build\Release\PhysicsQueriesTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsQueriesTest
        goto :test_failed
    ) else (
        echo [PASS] PhysicsQueriesTest
    )
)

REM Run PhysicsConfigurationTest
if exist "build\Release\PhysicsConfigurationTest.exe" (
    echo [INFO] Running PhysicsConfigurationTest...
    build\Release\PhysicsConfigurationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsConfigurationTest
        goto :test_failed
    ) else (
        echo [PASS] PhysicsConfigurationTest
    )
)

REM Run MovementComponentComparisonTest
if exist "build\Release\MovementComponentComparisonTest.exe" (
    echo [INFO] Running MovementComponentComparisonTest...
    build\Release\MovementComponentComparisonTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MovementComponentComparisonTest
        goto :test_failed
    ) else (
        echo [PASS] MovementComponentComparisonTest
    )
)

REM Run PhysicsPerformanceSimpleTest
if exist "build\Release\PhysicsPerformanceSimpleTest.exe" (
    echo [INFO] Running PhysicsPerformanceSimpleTest...
    build\Release\PhysicsPerformanceSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsPerformanceSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] PhysicsPerformanceSimpleTest
    )
)

REM Run MemoryUsageSimpleTest
if exist "build\Release\MemoryUsageSimpleTest.exe" (
    echo [INFO] Running MemoryUsageSimpleTest...
    build\Release\MemoryUsageSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MemoryUsageSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] MemoryUsageSimpleTest
    )
)

REM Run CharacterBehaviorSimpleTest
if exist "build\Release\CharacterBehaviorSimpleTest.exe" (
    echo [INFO] Running CharacterBehaviorSimpleTest...
    build\Release\CharacterBehaviorSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] CharacterBehaviorSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] CharacterBehaviorSimpleTest
    )
)

echo.
echo ========================================
echo [SUCCESS] ALL TESTS PASSED!
echo ========================================
exit /b 0

:test_failed
echo.
echo ========================================
echo [FAILED] One or more tests failed!
echo ========================================
exit /b 1