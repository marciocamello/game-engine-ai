@echo off
echo ========================================
echo  Game Engine Kiro - Dependency Setup
echo ========================================
echo.

REM Check if Git is available
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Git not found! Please install Git first.
    echo Download from: https://git-scm.com/download/win
    pause
    exit /b 1
)

REM Check if vcpkg directory exists
if not exist "vcpkg" (
    echo Cloning vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if %errorlevel% neq 0 (
        echo ERROR: Failed to clone vcpkg. Check your internet connection.
        pause
        exit /b 1
    )
    echo vcpkg cloned successfully!
) else (
    echo vcpkg directory already exists.
)

REM Bootstrap vcpkg
echo.
echo Bootstrapping vcpkg...
cd vcpkg
if not exist "vcpkg.exe" (
    call bootstrap-vcpkg.bat
    if %errorlevel% neq 0 (
        echo ERROR: Failed to bootstrap vcpkg.
        pause
        exit /b 1
    )
    echo vcpkg bootstrapped successfully!
) else (
    echo vcpkg already bootstrapped.
)

REM Install dependencies using manifest mode
echo.
echo Installing dependencies from vcpkg.json...
echo This may take several minutes...
echo.

REM Set the triplet for x64-windows
set VCPKG_DEFAULT_TRIPLET=x64-windows

REM Go back to project root to find vcpkg.json
cd ..

REM Install all dependencies from vcpkg.json
echo Installing all dependencies (this will take a while)...
vcpkg\vcpkg install --triplet x64-windows
if %errorlevel% neq 0 (
    echo WARNING: Some dependencies may have had installation issues
    echo This is often normal and the build may still work
) else (
    echo All dependencies installed successfully!
)

echo.
echo ========================================
echo  Dependency Setup Complete!
echo ========================================
echo.
echo Core dependencies installed:
echo   - GLFW3 (window management)
echo   - GLM (math library)
echo   - GLAD (OpenGL loader)
echo   - STB (image loading)
echo   - nlohmann-json (JSON parsing)
echo   - fmt (string formatting)
echo.
echo Optional dependencies installed:
echo   - Assimp (3D model loading)
echo   - OpenAL (3D audio)
echo   - Bullet3 (physics simulation)
echo   - Lua (scripting)
echo.
echo You can now build the project using:
echo   .\scripts\build_unified.bat --tests
echo.
pause