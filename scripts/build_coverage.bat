@echo off
echo Game Engine Kiro - Coverage Build Script
echo ========================================

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure with coverage support enabled
echo Configuring CMake with coverage support...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake

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
echo Coverage build completed successfully!
echo Run "powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat" to generate coverage reports.