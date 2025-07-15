@echo off
echo ========================================
echo  Game Engine Kiro - Setup Test
echo ========================================
echo.

echo Testing system requirements...
echo.

REM Test Git
echo [1/3] Checking Git...
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo   [ERROR] Git not found
    echo   Please install Git from: https://git-scm.com/download/win
    set /a errors+=1
) else (
    for /f "tokens=3" %%i in ('git --version') do echo   [OK] Git found: %%i
)

REM Test CMake
echo [2/3] Checking CMake...
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo   [ERROR] CMake not found
    echo   Please install CMake from: https://cmake.org/download/
    set /a errors+=1
) else (
    for /f "tokens=3" %%i in ('cmake --version') do echo   [OK] CMake found: %%i
)

REM Test Visual Studio Build Tools
echo [3/3] Checking Visual Studio Build Tools...
where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo   [WARNING] Visual Studio compiler not in PATH
    echo   This is normal - CMake will find it automatically
    echo   Make sure you have Visual Studio 2019+ or Build Tools installed
) else (
    echo   [OK] Visual Studio compiler found in PATH
)

echo.
echo Checking project structure...

if exist "CMakeLists.txt" (
    echo   [OK] CMakeLists.txt found
) else (
    echo   [ERROR] CMakeLists.txt missing
    set /a errors+=1
)

if exist "include\Core\Engine.h" (
    echo   [OK] Engine headers found
) else (
    echo   [ERROR] Engine headers missing
    set /a errors+=1
)

if exist "src\Core\Engine.cpp" (
    echo   [OK] Engine source found
) else (
    echo   [ERROR] Engine source missing
    set /a errors+=1
)

if exist "examples\main.cpp" (
    echo   [OK] Example code found
) else (
    echo   [ERROR] Example code missing
    set /a errors+=1
)

echo.
echo All checks passed! You can now run:
echo   1. setup_dependencies.bat  (to install dependencies)
echo   2. build.bat              (to build the project)

echo.
pause