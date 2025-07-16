@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  Game Engine Kiro - Debug Launcher
echo ========================================
echo.

:: Check if Debug build exists
if not exist "build\Debug\GameExample.exe" (
    echo ERROR: Debug build not found!
    echo.
    echo Building Debug version...
    mkdir build 2>nul
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build . --config Debug
    cd ..
    
    if not exist "build\Debug\GameExample.exe" (
        echo Failed to build Debug version!
        pause
        exit /b 1
    )
)

echo Debug build found!
echo.

:: Create logs directory
if not exist "logs" mkdir logs

:: Set log file with timestamp
set "LOG_FILE=logs\debug_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log"
set "LOG_FILE=!LOG_FILE: =0!"

echo Debug Options:
echo [1] Run with Visual Studio Debugger
echo [2] Run with Console Output
echo [3] Run with File Logging
echo [4] Run with Memory Leak Detection
echo [5] Run Performance Profiling
echo [Q] Quit
echo.

set /p "choice=Select debug option: "

if /i "%choice%"=="q" goto :end
if "%choice%"=="1" goto :vs_debug
if "%choice%"=="2" goto :console_debug
if "%choice%"=="3" goto :file_debug
if "%choice%"=="4" goto :memory_debug
if "%choice%"=="5" goto :profile_debug

echo Invalid option!
pause
goto :end

:vs_debug
echo.
echo Launching with Visual Studio Debugger...
if exist "build\GameEngineKiro.sln" (
    devenv build\GameEngineKiro.sln /debugexe build\Debug\GameExample.exe
) else (
    echo Visual Studio solution not found. Starting executable directly...
    cd build\Debug
    GameExample.exe
    cd ..\..
)
goto :end

:console_debug
echo.
echo Running Debug with Console Output...
echo Press Ctrl+C to stop debugging
echo.
cd build\Debug
GameExample.exe
cd ..\..
goto :end

:file_debug
echo.
echo Running Debug with File Logging...
echo Logs will be saved to: !LOG_FILE!
echo.
cd build\Debug
GameExample.exe > "..\..\!LOG_FILE!" 2>&1
cd ..\..
echo.
echo Debug session completed!
echo Log file: !LOG_FILE!
goto :end

:memory_debug
echo.
echo Running with Memory Leak Detection...
echo This will run slower but detect memory issues
echo.
cd build\Debug
set _CRTDBG_MAP_ALLOC=1
GameExample.exe > "..\..\!LOG_FILE!" 2>&1
cd ..\..
echo.
echo Memory debug completed!
echo Check log file for memory leak reports: !LOG_FILE!
goto :end

:profile_debug
echo.
echo Running Performance Profiling...
echo Collecting performance data...
echo.
set "PROFILE_FILE=logs\profile_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.txt"
set "PROFILE_FILE=!PROFILE_FILE: =0!"
cd build\Debug
echo Performance Profile - %date% %time% > "..\..\!PROFILE_FILE!"
echo ================================== >> "..\..\!PROFILE_FILE!"
GameExample.exe > "..\..\!PROFILE_FILE!" 2>&1
cd ..\..
echo.
echo Profiling completed!
echo Profile saved to: !PROFILE_FILE!
goto :end

:end
echo.
echo Debug session ended.
pause