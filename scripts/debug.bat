@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  Game Engine Kiro - Debug Launcher
echo ========================================
echo.

:: Check if Debug build exists (check both old and new structure)
set DEBUG_EXE_FOUND=0
if exist "build\Debug\GameExample.exe" set DEBUG_EXE_FOUND=1
if exist "build\projects\GameExample\Debug\GameExample.exe" set DEBUG_EXE_FOUND=1

if %DEBUG_EXE_FOUND%==0 (
    echo ERROR: Debug build not found!
    echo.
    echo Building Debug version...
    call scripts\build_unified.bat --debug --projects
    
    REM Check again after build
    set DEBUG_EXE_FOUND=0
    if exist "build\Debug\GameExample.exe" set DEBUG_EXE_FOUND=1
    if exist "build\projects\GameExample\Debug\GameExample.exe" set DEBUG_EXE_FOUND=1
    
    if %DEBUG_EXE_FOUND%==0 (
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
if exist "build\projects\GameExample\Debug\GameExample.exe" (
    cd build\projects\GameExample\Debug
    GameExample.exe
    cd ..\..\..\..
) else (
    cd build\Debug
    GameExample.exe
    cd ..\..
)
goto :end

:file_debug
echo.
echo Running Debug with File Logging...
echo Logs will be saved to: !LOG_FILE!
echo.
if exist "build\projects\GameExample\Debug\GameExample.exe" (
    cd build\projects\GameExample\Debug
    GameExample.exe > "..\..\..\..\!LOG_FILE!" 2>&1
    cd ..\..\..\..
) else (
    cd build\Debug
    GameExample.exe > "..\..\!LOG_FILE!" 2>&1
    cd ..\..
)
echo.
echo Debug session completed!
echo Log file: !LOG_FILE!
goto :end

:memory_debug
echo.
echo Running with Memory Leak Detection...
echo This will run slower but detect memory issues
echo.
if exist "build\projects\GameExample\Debug\GameExample.exe" (
    cd build\projects\GameExample\Debug
    set _CRTDBG_MAP_ALLOC=1
    GameExample.exe > "..\..\..\..\!LOG_FILE!" 2>&1
    cd ..\..\..\..
) else (
    cd build\Debug
    set _CRTDBG_MAP_ALLOC=1
    GameExample.exe > "..\..\!LOG_FILE!" 2>&1
    cd ..\..
)
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
if exist "build\projects\GameExample\Debug\GameExample.exe" (
    cd build\projects\GameExample\Debug
    echo Performance Profile - %date% %time% > "..\..\..\..\!PROFILE_FILE!"
    echo ================================== >> "..\..\..\..\!PROFILE_FILE!"
    GameExample.exe > "..\..\..\..\!PROFILE_FILE!" 2>&1
    cd ..\..\..\..
) else (
    cd build\Debug
    echo Performance Profile - %date% %time% > "..\..\!PROFILE_FILE!"
    echo ================================== >> "..\..\!PROFILE_FILE!"
    GameExample.exe > "..\..\!PROFILE_FILE!" 2>&1
    cd ..\..
)
echo.
echo Profiling completed!
echo Profile saved to: !PROFILE_FILE!
goto :end

:end
echo.
echo Debug session ended.
pause