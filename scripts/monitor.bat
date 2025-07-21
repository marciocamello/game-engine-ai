@echo off
setlocal enabledelayedexpansion

echo ========================================
echo  Game Engine Kiro - Log Monitor
echo ========================================
echo.

:: Check if logs directory exists
if not exist "logs" (
    echo No logs directory found. Creating...
    mkdir logs
)

echo Log Monitor Options:
echo [1] Monitor Latest Log File
echo [2] Monitor Engine Log (engine.log)
echo [3] View All Recent Logs
echo [4] Search Logs for Errors
echo [5] Search Logs for Warnings
echo [6] Tail Live Log
echo [7] Export Logs Summary
echo [Q] Quit
echo.

set /p "choice=Select option: "

if /i "%choice%"=="q" goto :end
if "%choice%"=="1" goto :monitor_latest
if "%choice%"=="2" goto :monitor_engine
if "%choice%"=="3" goto :view_all
if "%choice%"=="4" goto :search_errors
if "%choice%"=="5" goto :search_warnings
if "%choice%"=="6" goto :tail_live
if "%choice%"=="7" goto :export_summary

echo Invalid option!
pause
goto :end

:monitor_latest
echo.
echo Monitoring Latest Log File...
powershell -Command "Get-ChildItem -Path 'logs' -Filter '*.log' | Sort-Object LastWriteTime -Descending | Select-Object -First 1 | ForEach-Object { Write-Host 'Latest log:' $_.Name; Get-Content $_.FullName -Wait -Tail 20 }"
goto :end

:monitor_engine
echo.
echo Monitoring Engine Log...
if exist "engine.log" (
    echo Press Ctrl+C to stop monitoring
    echo.
    powershell -Command "Get-Content 'engine.log' -Wait -Tail 20"
) else (
    echo engine.log not found!
    pause
)
goto :end

:view_all
echo.
echo Recent Log Files:
echo ==================
powershell -Command "if (Test-Path 'logs\*.log') { Get-ChildItem -Path 'logs' -Filter '*.log' | Sort-Object LastWriteTime -Descending | ForEach-Object { Write-Host $_.Name -ForegroundColor Green; Write-Host '  Size:' $_.Length 'bytes'; Write-Host '  Modified:' $_.LastWriteTime; Write-Host '' } } else { Write-Host 'No log files found.' -ForegroundColor Yellow }"
echo.
pause
goto :end

:search_errors
echo.
echo Searching for Errors in Logs...
echo ================================
powershell -Command "Get-ChildItem -Path 'logs' -Filter '*.log' -ErrorAction SilentlyContinue | ForEach-Object { Write-Host 'Checking' $_.Name':' -ForegroundColor Blue; Select-String -Path $_.FullName -Pattern 'error|exception|failed|crash' -CaseSensitive:$false }"
if exist "engine.log" (
    echo.
    echo Checking engine.log:
    powershell -Command "Select-String -Path 'engine.log' -Pattern 'error|exception|failed|crash' -CaseSensitive:$false"
)
echo.
pause
goto :end

:search_warnings
echo.
echo Searching for Warnings in Logs...
echo ==================================
powershell -Command "Get-ChildItem -Path 'logs' -Filter '*.log' -ErrorAction SilentlyContinue | ForEach-Object { Write-Host 'Checking' $_.Name':' -ForegroundColor Blue; Select-String -Path $_.FullName -Pattern 'warning|warn|deprecated' -CaseSensitive:$false }"
if exist "engine.log" (
    echo.
    echo Checking engine.log:
    powershell -Command "Select-String -Path 'engine.log' -Pattern 'warning|warn|deprecated' -CaseSensitive:$false"
)
echo.
pause
goto :end

:tail_live
echo.
echo Live Log Monitoring...
echo This will show new log entries as they appear
echo Press Ctrl+C to stop
echo.
set "LIVE_LOG=logs\live_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log"
set "LIVE_LOG=!LIVE_LOG: =0!"

:: Start the game in background and monitor its output
echo Starting game with live logging...
start /b "" build\Release\GameExample.exe > "!LIVE_LOG!" 2>&1
powershell -Command "Get-Content '!LIVE_LOG!' -Wait -Tail 0"
goto :end

:export_summary
echo.
echo Exporting Logs Summary...
set "SUMMARY_FILE=logs\summary_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.txt"
set "SUMMARY_FILE=!SUMMARY_FILE: =0!"

echo Game Engine Kiro - Logs Summary > "!SUMMARY_FILE!"
echo Generated: %date% %time% >> "!SUMMARY_FILE!"
echo ================================== >> "!SUMMARY_FILE!"
echo. >> "!SUMMARY_FILE!"

echo ERROR SUMMARY: >> "!SUMMARY_FILE!"
echo -------------- >> "!SUMMARY_FILE!"
powershell -Command "Get-ChildItem -Path 'logs' -Filter '*.log' -ErrorAction SilentlyContinue | ForEach-Object { Select-String -Path $_.FullName -Pattern 'error|exception|failed|crash' -CaseSensitive:$false } | Out-File -FilePath '!SUMMARY_FILE!' -Append"

echo. >> "!SUMMARY_FILE!"
echo WARNING SUMMARY: >> "!SUMMARY_FILE!"
echo ---------------- >> "!SUMMARY_FILE!"
powershell -Command "Get-ChildItem -Path 'logs' -Filter '*.log' -ErrorAction SilentlyContinue | ForEach-Object { Select-String -Path $_.FullName -Pattern 'warning|warn|deprecated' -CaseSensitive:$false } | Out-File -FilePath '!SUMMARY_FILE!' -Append"

echo Summary exported to: !SUMMARY_FILE!
echo.
pause
goto :end

:end
echo.
echo Log Monitor - Goodbye!
pause