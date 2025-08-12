@echo off
REM Game Engine Kiro - Quick Test Script
REM Simple script for running common tests quickly

echo ========================================
echo  Game Engine Kiro - Quick Test
echo ========================================

REM Check if build directory exists
if not exist "build\Release" (
    echo [ERROR] Build directory not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

REM Run Math unit test
echo [INFO] Running Math unit test...
if exist "build\Release\MathTest.exe" (
    build\Release\MathTest.exe
    if errorlevel 1 (
        echo [FAILED] Math test failed
        goto :test_failed
    ) else (
        echo [PASS] Math test completed
    )
) else (
    echo [WARNING] MathTest.exe not found
    goto :test_failed
)

echo.

REM Run a quick integration test
echo [INFO] Running quick integration test...
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    build\Release\BulletUtilsSimpleTest.exe
    if errorlevel 1 (
        echo [FAILED] Bullet utils test failed
        goto :test_failed
    ) else (
        echo [PASS] Bullet utils test completed
    )
) else (
    echo [WARNING] BulletUtilsSimpleTest.exe not found
    goto :test_failed
)

echo.
echo ========================================
echo [SUCCESS] Quick tests passed!
echo ========================================
exit /b 0

:test_failed
echo.
echo ========================================
echo [FAILED] One or more tests failed
echo ========================================
exit /b 1