@echo off
REM Game Engine Kiro - Test Configuration Management Script

echo ========================================
echo  Game Engine Kiro - Test Configuration
echo ========================================

REM Check if build directory exists
if not exist "build\Release" (
    echo [ERROR] Build directory not found. Please run scripts\build.bat first.
    exit /b 1
)

REM Check if test config manager exists
if not exist "build\Release\TestConfigManager.exe" (
    echo [ERROR] Test configuration manager not found. Please run scripts\build.bat first.
    exit /b 1
)

REM Execute test configuration manager with all arguments
build\Release\TestConfigManager.exe %*

REM Check result
if errorlevel 1 (
    echo.
    echo ========================================
    echo [FAILED] Test configuration operation failed!
    echo ========================================
    exit /b 1
) else (
    echo.
    echo ========================================
    echo [SUCCESS] Test configuration operation completed!
    echo ========================================
    exit /b 0
)