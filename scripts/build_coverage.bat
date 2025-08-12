@echo off
echo Game Engine Kiro - Coverage Build Script
echo ========================================

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure with coverage support enabled
echo Configuring CMake with coverage support...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_TESTS=ON -DBUILD_ENGINE=ON

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build the project
echo Building project with coverage support...
cmake --build . --config Debug

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    cd ..
    exit /b 1
)

cd ..
echo.
echo ========================================
echo Coverage build completed successfully!
echo ========================================
echo.
echo This script is now deprecated. Use the unified build script instead:
echo   .\scripts\build_unified.bat --coverage
echo.
echo To generate coverage reports:
echo   .\scripts\run_coverage_analysis.bat