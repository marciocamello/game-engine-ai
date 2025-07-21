@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  Game Engine Kiro - Launcher
echo ========================================
echo.

:: Check if build exists
if not exist "build" (
    echo ERROR: Build directory not found!
    echo Please run build.bat first to compile the project.
    echo.
    pause
    exit /b 1
)

:: Check for Release build
set "RELEASE_EXE=build\Release\GameExample.exe"
set "DEBUG_EXE=build\Debug\GameExample.exe"
set "SELECTED_EXE="
set "BUILD_TYPE="

if exist "%RELEASE_EXE%" (
    set "SELECTED_EXE=%RELEASE_EXE%"
    set "BUILD_TYPE=Release"
    echo Found Release build
) else if exist "%DEBUG_EXE%" (
    set "SELECTED_EXE=%DEBUG_EXE%"
    set "BUILD_TYPE=Debug"
    echo Found Debug build
) else (
    echo ERROR: No executable found!
    echo Please run build.bat first to compile the project.
    echo.
    pause
    exit /b 1
)

echo Build Type: !BUILD_TYPE!
echo Executable: !SELECTED_EXE!
echo.

:: Show launch options
echo Launch Options:
echo [1] Start Game (Normal)
echo [2] Start Game with Console Logs
echo [3] Start Game with File Logs
echo [4] Start Game with Full Debug Logs
echo [5] Start Game in Windowed Mode
echo [6] View Recent Logs
echo [7] Clean Logs
echo [Q] Quit
echo.

set /p "choice=Select option (1-7 or Q): "

if /i "%choice%"=="q" goto :end
if "%choice%"=="1" goto :start_normal
if "%choice%"=="2" goto :start_console
if "%choice%"=="3" goto :start_file_logs
if "%choice%"=="4" goto :start_debug
if "%choice%"=="5" goto :start_windowed
if "%choice%"=="6" goto :view_logs
if "%choice%"=="7" goto :clean_logs

echo Invalid option!
pause
goto :end

:start_normal
echo.
echo Starting Game Engine Kiro (Normal Mode)...
cd build\!BUILD_TYPE!
start "" GameExample.exe
cd ..\..
echo Game started! Check the game window.
goto :end

:start_console
echo.
echo Starting Game Engine Kiro (Console Logs)...
echo Console will show real-time logs. Close console to stop game.
echo.
cd build\!BUILD_TYPE!
GameExample.exe
cd ..\..
goto :end

:start_file_logs
echo.
echo Starting Game Engine Kiro (File Logs)...
set "LOG_FILE=logs\game_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log"
set "LOG_FILE=!LOG_FILE: =0!"
if not exist "logs" mkdir logs
echo Logs will be saved to: !LOG_FILE!
cd build\!BUILD_TYPE!
GameExample.exe > "..\..\!LOG_FILE!" 2>&1
cd ..\..
echo Game completed! Logs saved to !LOG_FILE!
goto :end

:start_debug
echo.
echo Starting Game Engine Kiro (Full Debug)...
echo This will show verbose logging and keep console open.
echo.
set "LOG_FILE=logs\debug_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log"
set "LOG_FILE=!LOG_FILE: =0!"
if not exist "logs" mkdir logs
cd build\!BUILD_TYPE!
echo Starting with debug logging... > "..\..\!LOG_FILE!"
echo Debug session started. Output will be saved to !LOG_FILE!
GameExample.exe 2>&1 | powershell -Command "$input | Tee-Object -FilePath '..\..\!LOG_FILE!' -Append"
cd ..\..
goto :end

:start_windowed
echo.
echo Starting Game Engine Kiro (Windowed Mode)...
echo Game will start in windowed mode for easier debugging.
cd build\!BUILD_TYPE!
start "" GameExample.exe --windowed
cd ..\..
echo Game started in windowed mode!
goto :end

:view_logs
echo.
echo Recent Log Files:
if exist "logs" (
    powershell -Command "Get-ChildItem -Path 'logs' -Filter '*.log' | Sort-Object LastWriteTime -Descending | Select-Object -First 10 | ForEach-Object { Write-Host $_.Name }"
    echo.
    set /p "log_choice=Enter log filename to view (or press Enter to skip): "
    if not "!log_choice!"=="" (
        if exist "logs\!log_choice!" (
            echo.
            echo Showing last 50 lines of !log_choice!:
            echo ----------------------------------------
            powershell -Command "Get-Content 'logs\!log_choice!' | Select-Object -Last 50"
            echo ----------------------------------------
        ) else (
            echo Log file not found!
        )
    )
) else (
    echo No logs directory found. Run the game with logging first.
)
echo.
pause
goto :end

:clean_logs
echo.
echo Cleaning old log files...
if exist "logs" (
    del /q logs\*.log 2>nul
    echo Log files cleaned!
) else (
    echo No logs to clean.
)
echo.
pause
goto :end

:end
echo.
echo Game Engine Kiro Launcher - Goodbye!
pause