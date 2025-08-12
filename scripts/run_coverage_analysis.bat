@echo off
REM Game Engine Kiro - Test Coverage Analysis Script
REM This script runs basic test coverage analysis

setlocal enabledelayedexpansion

set BUILD_FIRST=false
set VERBOSE=false
set OUTPUT_DIR=coverage_reports
set OPEN_REPORT=false

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="build" set BUILD_FIRST=true
if /i "%~1"=="verbose" set VERBOSE=true
if /i "%~1"=="-v" set VERBOSE=true
if /i "%~1"=="open" set OPEN_REPORT=true
shift
goto :parse_args
:args_done

echo ========================================
echo  Game Engine Kiro - Test Coverage Analysis
echo ========================================

REM Check if OpenCppCoverage is installed
where OpenCppCoverage.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: OpenCppCoverage not found!
    echo Please install OpenCppCoverage from: https://github.com/OpenCppCoverage/OpenCppCoverage/releases
    echo Or install via Chocolatey: choco install opencppcoverage
    exit /b 1
)

echo Found OpenCppCoverage

REM Build project first if requested
if "%BUILD_FIRST%"=="true" (
    echo.
    echo Building project with coverage support...
    call build_coverage.bat
    if !errorlevel! neq 0 (
        echo Build failed! Exiting.
        exit /b 1
    )
    echo Build completed successfully.
)

REM Create output directory
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
    echo Created coverage reports directory: %OUTPUT_DIR%
)

REM Check if build directory exists
if not exist "build\Release" (
    echo ERROR: Build directory not found!
    echo Please run 'scripts\build_unified.bat --tests' first to compile the project.
    exit /b 1
)

echo.
echo Running coverage analysis...

REM Run coverage on main tests
set COVERAGE_ARGS=--sources src --export_type html:%OUTPUT_DIR%\coverage_report.html --export_type cobertura:%OUTPUT_DIR%\coverage.xml

REM Run unit tests with coverage
echo Running unit test coverage...
if exist "build\Release\MathTest.exe" (
    OpenCppCoverage.exe %COVERAGE_ARGS% --cover_children -- build\Release\MathTest.exe
    if !errorlevel! neq 0 (
        echo Coverage analysis failed for MathTest
    )
)

REM Run integration tests with coverage
echo Running integration test coverage...
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    OpenCppCoverage.exe %COVERAGE_ARGS% --cover_children -- build\Release\BulletUtilsSimpleTest.exe
    if !errorlevel! neq 0 (
        echo Coverage analysis failed for BulletUtilsSimpleTest
    )
)

REM Additional tests
if exist "build\Release\PhysicsConfigurationTest.exe" (
    OpenCppCoverage.exe %COVERAGE_ARGS% --cover_children -- build\Release\PhysicsConfigurationTest.exe
    if !errorlevel! neq 0 (
        echo Coverage analysis failed for PhysicsConfigurationTest
    )
)

echo.
echo ========================================
echo Coverage analysis completed!
echo Reports generated in: %OUTPUT_DIR%
echo ========================================

REM Open report if requested
if "%OPEN_REPORT%"=="true" (
    if exist "%OUTPUT_DIR%\coverage_report.html" (
        echo Opening coverage report...
        start "" "%OUTPUT_DIR%\coverage_report.html"
    ) else (
        echo Coverage report not found at %OUTPUT_DIR%\coverage_report.html
    )
)

echo.
echo RECOMMENDATIONS:
echo   - Review the HTML report for detailed coverage information
echo   - Focus on areas with low coverage percentages
echo   - Add tests for uncovered code paths
echo   - Run coverage analysis regularly during development

exit /b 0