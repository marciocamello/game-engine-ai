@echo off
echo ========================================
echo  Game Engine Kiro - Build System
echo ========================================
echo.

REM Check if CMake is available
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found! Please install CMake first.
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Check if dependencies are installed
if not exist "vcpkg\vcpkg.exe" (
    echo Dependencies not found. Running setup...
    call setup_dependencies.bat
    if %errorlevel% neq 0 (
        echo ERROR: Failed to setup dependencies!
        pause
        exit /b 1
    )
)

REM Check if core dependencies are installed
if not exist "vcpkg\installed\x64-windows\include\GLFW\glfw3.h" (
    echo WARNING: GLFW3 headers not found in expected location.
    echo This may be normal if using a different vcpkg configuration.
    echo Continuing with build...
)

echo Dependencies verified successfully!
echo.

REM Create build directory
echo Creating build directory...
if not exist build mkdir build
cd build

REM Configure with vcpkg toolchain
echo Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -A x64
if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Possible solutions:
    echo 1. Make sure all dependencies are installed: setup_dependencies.bat
    echo 2. Check if Visual Studio or Build Tools are installed
    echo 3. Try running from Visual Studio Developer Command Prompt
    echo.
    cd ..
    pause
    exit /b 1
)

echo CMake configuration successful!
echo.

REM Build the project
echo Building project...
cmake --build . --config Release --parallel
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    echo Check the error messages above for details.
    echo.
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo  Build Completed Successfully!
echo ========================================
echo.
echo Files created:
echo   - Engine Library: build\Release\GameEngineKiro.lib
echo   - [ProjectName]: build\projects\[ProjectName]\Release\[ProjectName].exe
echo   - All Tests: build\Release\*Test.exe
echo.
echo To run examples:
echo   [ProjectName]: build/projects/[ProjectName]/Release/
echo.
echo To run tests: .\scripts\run_tests.bat
echo.

pause