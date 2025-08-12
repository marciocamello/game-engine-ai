@echo off
setlocal enabledelayedexpansion

title Game Engine Kiro - Development Console

:main_menu
cls
echo ========================================
echo  Game Engine Kiro - Development Hub
echo ========================================
echo.
echo Project Status:
set RELEASE_BUILD_FOUND=0
set DEBUG_BUILD_FOUND=0
if exist "build\Release\GameExample.exe" set RELEASE_BUILD_FOUND=1
if exist "build\projects\GameExample\Release\GameExample.exe" set RELEASE_BUILD_FOUND=1
if exist "build\Debug\GameExample.exe" set DEBUG_BUILD_FOUND=1
if exist "build\projects\GameExample\Debug\GameExample.exe" set DEBUG_BUILD_FOUND=1

if %RELEASE_BUILD_FOUND%==1 (
    echo   [OK] Release Build: Ready
) else (
    echo   [--] Release Build: Missing
)
if %DEBUG_BUILD_FOUND%==1 (
    echo   [OK] Debug Build: Ready
) else (
    echo   [--] Debug Build: Missing
)
if exist "vcpkg\vcpkg.exe" (
    echo   [OK] Dependencies: Installed
) else (
    echo   [--] Dependencies: Missing
)
echo.

echo Development Options:
echo.
echo Build and Setup:
echo   [1] Setup Dependencies
echo   [2] Build Project (Release)
echo   [3] Build Project (Debug)
echo   [4] Clean Build
echo   [5] Rebuild All
echo.
echo Run and Test:
echo   [6] Start Game (Release)
echo   [7] Start Game (Debug)
echo   [8] Run with Logs
echo   [9] Performance Test
echo.
echo Development Tools:
echo   [A] Monitor Logs
echo   [B] Debug Session
echo   [C] Code Analysis
echo   [D] Memory Check
echo.
echo Utilities:
echo   [E] Open Project in VS Code
echo   [F] Open Build Directory
echo   [G] View Documentation
echo   [H] System Information
echo.
echo   [Q] Quit
echo.

set /p "choice=Select option: "

if /i "%choice%"=="q" goto :end
if "%choice%"=="1" goto :setup_deps
if "%choice%"=="2" goto :build_release
if "%choice%"=="3" goto :build_debug
if "%choice%"=="4" goto :clean_build
if "%choice%"=="5" goto :rebuild_all
if "%choice%"=="6" goto :start_release
if "%choice%"=="7" goto :start_debug
if "%choice%"=="8" goto :run_logs
if "%choice%"=="9" goto :perf_test
if /i "%choice%"=="a" goto :monitor_logs
if /i "%choice%"=="b" goto :debug_session
if /i "%choice%"=="c" goto :code_analysis
if /i "%choice%"=="d" goto :memory_check
if /i "%choice%"=="e" goto :open_vscode
if /i "%choice%"=="f" goto :open_build
if /i "%choice%"=="g" goto :view_docs
if /i "%choice%"=="h" goto :system_info

echo Invalid option!
pause
goto :main_menu

:setup_deps
echo.
echo Setting up dependencies...
call setup_dependencies.bat
pause
goto :main_menu

:build_release
echo.
echo Building Release version...
call scripts\build_unified.bat --all
pause
goto :main_menu

:build_debug
echo.
echo Building Debug version...
call scripts\build_unified.bat --debug --all
pause
goto :main_menu

:clean_build
echo.
echo Cleaning build directory...
if exist "build" (
    rmdir /s /q build
    echo Build directory cleaned!
) else (
    echo Build directory already clean.
)
pause
goto :main_menu

:rebuild_all
echo.
echo Rebuilding everything...
if exist "build" rmdir /s /q build
call scripts\setup_dependencies.bat
call scripts\build_unified.bat --all
pause
goto :main_menu

:start_release
echo.
echo Starting Release version...
if exist "build\projects\GameExample\Release\GameExample.exe" (
    cd build\projects\GameExample\Release
    start "" GameExample.exe
    cd ..\..\..\..
    echo Game started!
) else if exist "build\Release\GameExample.exe" (
    cd build\Release
    start "" GameExample.exe
    cd ..\..
    echo Game started!
) else (
    echo Release build not found! Build first.
)
pause
goto :main_menu

:start_debug
echo.
echo Starting Debug version...
if exist "build\projects\GameExample\Debug\GameExample.exe" (
    cd build\projects\GameExample\Debug
    start "" GameExample.exe
    cd ..\..\..\..
    echo Debug game started!
) else if exist "build\Debug\GameExample.exe" (
    cd build\Debug
    start "" GameExample.exe
    cd ..\..
    echo Debug game started!
) else (
    echo Debug build not found! Build first.
)
pause
goto :main_menu

:run_logs
echo.
echo Starting GameExample...
cd build\projects\GameExample\Release
if exist GameExample.exe (
    start GameExample.exe
    echo GameExample started!
) else (
    echo ERROR: GameExample.exe not found! Please build first.
)
cd ..\..\..\..
goto :main_menu

:perf_test
echo.
echo Running performance test...
if exist "build\projects\GameExample\Release\GameExample.exe" (
    echo Starting performance monitoring...
    if not exist "logs" mkdir logs
    set "PERF_LOG=logs\perf_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.log"
    set "PERF_LOG=!PERF_LOG: =0!"
    cd build\projects\GameExample\Release
    echo Performance Test - %date% %time% > "..\..\..\..\!PERF_LOG!"
    GameExample.exe >> "..\..\..\..\!PERF_LOG!" 2>&1
    cd ..\..\..\..
    echo Performance test completed!
    echo Log saved to: !PERF_LOG!
) else if exist "build\Release\GameExample.exe" (
    echo Starting performance monitoring...
    if not exist "logs" mkdir logs
    set "PERF_LOG=logs\perf_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~6,2%.log"
    set "PERF_LOG=!PERF_LOG: =0!"
    cd build\Release
    echo Performance Test - %date% %time% > "..\..\!PERF_LOG!"
    GameExample.exe >> "..\..\!PERF_LOG!" 2>&1
    cd ..\..
    echo Performance test completed!
    echo Log saved to: !PERF_LOG!
) else (
    echo Release build not found!
)
pause
goto :main_menu

:monitor_logs
call monitor.bat
goto :main_menu

:debug_session
call debug.bat
goto :main_menu

:code_analysis
echo.
echo Running code analysis...
echo Checking for common issues...
echo.
echo Checking for TODO comments:
powershell -Command "Get-ChildItem -Recurse -Include *.h,*.cpp | Select-String -Pattern 'TODO|FIXME|HACK' -CaseSensitive:$false"
echo.
echo Checking for potential memory leaks:
powershell -Command "Get-ChildItem -Recurse -Include *.cpp | Select-String -Pattern 'new |malloc|delete |free' -CaseSensitive:$false | Measure-Object | Select-Object Count"
echo.
echo Code analysis completed!
pause
goto :main_menu

:memory_check
echo.
echo Running memory check...
if exist "build\projects\GameExample\Debug\GameExample.exe" (
    echo Starting memory leak detection...
    cd build\projects\GameExample\Debug
    set _CRTDBG_MAP_ALLOC=1
    GameExample.exe
    cd ..\..\..\..
) else if exist "build\Debug\GameExample.exe" (
    echo Starting memory leak detection...
    cd build\Debug
    set _CRTDBG_MAP_ALLOC=1
    GameExample.exe
    cd ..\..
) else (
    echo Debug build required for memory checking!
)
pause
goto :main_menu

:open_vscode
echo.
echo Opening project in VS Code...
code .
goto :main_menu

:open_build
echo.
echo Opening build directory...
if exist "build" (
    explorer build
) else (
    echo Build directory not found!
    pause
)
goto :main_menu

:view_docs
echo.
echo Opening documentation...
if exist "docs" (
    explorer docs
) else (
    echo Documentation directory not found!
    pause
)
goto :main_menu

:system_info
echo.
echo ========================================
echo  System Information
echo ========================================
echo.
echo Operating System:
ver
echo.
echo Processor:
wmic cpu get name /value | findstr "Name="
echo.
echo Memory:
wmic computersystem get TotalPhysicalMemory /value | findstr "TotalPhysicalMemory="
echo.
echo Graphics Card:
wmic path win32_VideoController get name /value | findstr "Name="
echo.
echo CMake Version:
cmake --version 2>nul || echo CMake not found
echo.
echo Git Version:
git --version 2>nul || echo Git not found
echo.
echo Visual Studio:
where cl 2>nul && cl 2>&1 | findstr "Version" || echo Visual Studio not found
echo.
pause
goto :main_menu

:end
echo.
echo Development session ended. Happy coding!
pause