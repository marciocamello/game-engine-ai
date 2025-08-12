@echo off
REM Game Engine Kiro - Enhanced Test Runner Script
REM This script uses the new dual architecture test framework

echo ========================================
echo  Game Engine Kiro - Enhanced Test Runner
echo ========================================

REM Check if build directory exists
if not exist "build\Release" (
    echo [ERROR] Build directory not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

REM Check if enhanced test runner exists
if not exist "build\Release\EnhancedTestRunner.exe" (
    echo [ERROR] Enhanced test runner not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

REM Parse command line arguments
set COMMAND=--all
if "%1"=="unit" set COMMAND=--unit
if "%1"=="integration" set COMMAND=--integration
if "%1"=="performance" set COMMAND=--performance
if "%1"=="list" set COMMAND=--list
if "%1"=="help" set COMMAND=--help

echo.
echo Running enhanced test framework with command: %COMMAND%
echo.

REM Execute enhanced test runner
build\Release\EnhancedTestRunner.exe %COMMAND%

REM Check result
if errorlevel 1 (
    echo.
    echo ========================================
    echo [FAILED] Enhanced test execution failed!
    echo ========================================
    exit /b 1
) else (
    echo.
    echo ========================================
    echo [SUCCESS] Enhanced test execution completed!
    echo ========================================
    exit /b 0
)