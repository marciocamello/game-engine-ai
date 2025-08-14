@echo off
setlocal enabledelayedexpansion
REM Advanced Build Cleaning Script for Game Engine Kiro
REM Provides granular cleaning options for different build components

echo ========================================
echo Game Engine Kiro - Build Cleaner
echo ========================================

REM Parse command line arguments
set "COMMAND=%~1"
if "%COMMAND%"=="" set "COMMAND=help"

if /i "%COMMAND%"=="engine" goto :clean_engine
if /i "%COMMAND%"=="tests" goto :clean_tests
if /i "%COMMAND%"=="projects" goto :clean_projects
if /i "%COMMAND%"=="cache" goto :clean_cache
if /i "%COMMAND%"=="all" goto :clean_all
if /i "%COMMAND%"=="selective" goto :selective_clean
if /i "%COMMAND%"=="help" goto :help
if /i "%COMMAND%"=="-h" goto :help

echo ERROR: Unknown command "%COMMAND%"
echo Use "build_clean.bat help" for usage information
exit /b 1

:clean_engine
echo.
echo === Cleaning Engine Artifacts ===
echo.

call :clean_engine_files
call :report_cleaning_results "Engine"

goto :end

:clean_tests
echo.
echo === Cleaning Test Artifacts ===
echo.

call :clean_test_files
call :report_cleaning_results "Tests"

goto :end

:clean_projects
echo.
echo === Cleaning Project Artifacts ===
echo.

call :clean_project_files
call :report_cleaning_results "Projects"

goto :end

:clean_cache
echo.
echo === Cleaning Cache Artifacts ===
echo.

call :clean_cache_files
call :clean_vcpkg_cache
call :report_cleaning_results "Cache"

goto :end

:clean_all
echo.
echo === Cleaning All Build Artifacts ===
echo.

echo WARNING: This will remove all build artifacts and cache.
set /p CONFIRM="Are you sure? (y/N): "
if /i not "%CONFIRM%"=="y" (
    echo Operation cancelled.
    goto :end
)

call :clean_engine_files
call :clean_test_files
call :clean_project_files
call :clean_cache_files
call :clean_vcpkg_cache
call :clean_build_logs

call :report_cleaning_results "All Components"

goto :end

:selective_clean
echo.
echo === Selective Cleaning ===
echo.

echo Select components to clean:
echo   1. Engine artifacts
echo   2. Test artifacts
echo   3. Project artifacts
echo   4. CMake cache
echo   5. vcpkg binary cache
echo   6. Build logs
echo   7. All of the above
echo.

set /p CHOICE="Enter your choice (1-7, or comma-separated): "

REM Parse comma-separated choices
echo %CHOICE% | findstr "1" >nul && call :clean_engine_files
echo %CHOICE% | findstr "2" >nul && call :clean_test_files
echo %CHOICE% | findstr "3" >nul && call :clean_project_files
echo %CHOICE% | findstr "4" >nul && call :clean_cache_files
echo %CHOICE% | findstr "5" >nul && call :clean_vcpkg_cache
echo %CHOICE% | findstr "6" >nul && call :clean_build_logs
if "%CHOICE%"=="7" (
    call :clean_engine_files
    call :clean_test_files
    call :clean_project_files
    call :clean_cache_files
    call :clean_vcpkg_cache
    call :clean_build_logs
)

call :report_cleaning_results "Selected Components"

goto :end

:clean_engine_files
echo Cleaning engine artifacts...

set "FILES_CLEANED=0"

REM Clean engine library files
call :delete_file_if_exists "build\Release\GameEngineKiro.lib"
call :delete_file_if_exists "build\Debug\GameEngineKiro.lib"
call :delete_file_if_exists "build\vs\x64\Release\Release\GameEngineKiro.lib"
call :delete_file_if_exists "build\vs\x64\Debug\Debug\GameEngineKiro.lib"
call :delete_file_if_exists "build\ninja\x64\Release\GameEngineKiro.lib"
call :delete_file_if_exists "build\ninja\x64\Debug\GameEngineKiro.lib"

REM Clean engine object files
call :delete_dir_if_exists "build\CMakeFiles\GameEngineKiro.dir"
call :delete_dir_if_exists "build\vs\x64\Release\GameEngineKiro.dir"
call :delete_dir_if_exists "build\vs\x64\Debug\GameEngineKiro.dir"
call :delete_dir_if_exists "build\ninja\x64\Release\CMakeFiles\GameEngineKiro.dir"
call :delete_dir_if_exists "build\ninja\x64\Debug\CMakeFiles\GameEngineKiro.dir"

echo   Engine artifacts cleaned
goto :eof

:clean_test_files
echo Cleaning test artifacts...

set "FILES_CLEANED=0"

REM Clean test executables
for %%f in (build\Release\*Test.exe) do call :delete_file_if_exists "%%f"
for %%f in (build\Debug\*Test.exe) do call :delete_file_if_exists "%%f"
for %%f in (build\vs\x64\Release\Release\*Test.exe) do call :delete_file_if_exists "%%f"
for %%f in (build\vs\x64\Debug\Debug\*Test.exe) do call :delete_file_if_exists "%%f"
for %%f in (build\ninja\x64\Release\*Test.exe) do call :delete_file_if_exists "%%f"
for %%f in (build\ninja\x64\Debug\*Test.exe) do call :delete_file_if_exists "%%f"

REM Clean test object files
for /d %%d in (build\CMakeFiles\*Test.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\vs\x64\Release\*Test.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\vs\x64\Debug\*Test.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\ninja\x64\Release\CMakeFiles\*Test.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\ninja\x64\Debug\CMakeFiles\*Test.dir) do call :delete_dir_if_exists "%%d"

echo   Test artifacts cleaned
goto :eof

:clean_project_files
echo Cleaning project artifacts...

set "FILES_CLEANED=0"

REM Clean project directories
call :delete_dir_if_exists "build\projects"
call :delete_dir_if_exists "build\vs\x64\Release\projects"
call :delete_dir_if_exists "build\vs\x64\Debug\projects"
call :delete_dir_if_exists "build\ninja\x64\Release\projects"
call :delete_dir_if_exists "build\ninja\x64\Debug\projects"

REM Clean project object files
for /d %%d in (build\CMakeFiles\*Example.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\vs\x64\Release\*Example.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\vs\x64\Debug\*Example.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\ninja\x64\Release\CMakeFiles\*Example.dir) do call :delete_dir_if_exists "%%d"
for /d %%d in (build\ninja\x64\Debug\CMakeFiles\*Example.dir) do call :delete_dir_if_exists "%%d"

echo   Project artifacts cleaned
goto :eof

:clean_cache_files
echo Cleaning CMake cache...

set "FILES_CLEANED=0"

REM Clean CMake cache files
call :delete_file_if_exists "build\CMakeCache.txt"
call :delete_file_if_exists "build\vs\x64\Release\CMakeCache.txt"
call :delete_file_if_exists "build\vs\x64\Debug\CMakeCache.txt"
call :delete_file_if_exists "build\ninja\x64\Release\CMakeCache.txt"
call :delete_file_if_exists "build\ninja\x64\Debug\CMakeCache.txt"

REM Clean CMake files directories
call :delete_dir_if_exists "build\CMakeFiles"
call :delete_dir_if_exists "build\vs\x64\Release\CMakeFiles"
call :delete_dir_if_exists "build\vs\x64\Debug\CMakeFiles"
call :delete_dir_if_exists "build\ninja\x64\Release\CMakeFiles"
call :delete_dir_if_exists "build\ninja\x64\Debug\CMakeFiles"

REM Clean build state files
call :delete_file_if_exists "build\.last_build_signature"
call :delete_file_if_exists "build\.build_failed"

echo   CMake cache cleaned
goto :eof

:clean_vcpkg_cache
echo Cleaning vcpkg binary cache...

set "VCPKG_CACHE_DIR=%USERPROFILE%\.vcpkg-cache"
if exist "%VCPKG_CACHE_DIR%" (
    echo   Found vcpkg cache: %VCPKG_CACHE_DIR%
    
    REM Count files before cleaning
    for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set CACHE_FILES=%%i
    
    if !CACHE_FILES! gtr 0 (
        echo   Removing !CACHE_FILES! cached packages...
        rmdir /s /q "%VCPKG_CACHE_DIR%" 2>nul
        if not exist "%VCPKG_CACHE_DIR%" (
            echo   [OK] vcpkg binary cache cleaned
        ) else (
            echo   [WARNING] Some cache files could not be removed
        )
    ) else (
        echo   [INFO] vcpkg cache is already empty
    )
) else (
    echo   [INFO] No vcpkg binary cache found
)

goto :eof

:clean_build_logs
echo Cleaning build logs...

set "FILES_CLEANED=0"

REM Clean log files
call :delete_file_if_exists "logs\build_diagnostics.log"
call :delete_file_if_exists "logs\build_performance.json"
call :delete_file_if_exists "build_cache.log"
call :delete_file_if_exists "build_state.tmp"
call :delete_file_if_exists "build_statistics.tmp"

echo   Build logs cleaned
goto :eof

:delete_file_if_exists
REM Parameters: %1=file_path
if exist "%~1" (
    del "%~1" 2>nul
    if not exist "%~1" (
        set /a FILES_CLEANED+=1
        echo     Deleted: %~1
    ) else (
        echo     [WARNING] Could not delete: %~1
    )
)
goto :eof

:delete_dir_if_exists
REM Parameters: %1=directory_path
if exist "%~1" (
    rmdir /s /q "%~1" 2>nul
    if not exist "%~1" (
        set /a FILES_CLEANED+=1
        echo     Removed: %~1
    ) else (
        echo     [WARNING] Could not remove: %~1
    )
)
goto :eof

:report_cleaning_results
REM Parameters: %1=component_name
echo.
echo === %~1 Cleaning Complete ===
if defined FILES_CLEANED (
    if !FILES_CLEANED! gtr 0 (
        echo   Files/Directories processed: !FILES_CLEANED!
    ) else (
        echo   No files needed cleaning
    )
)
echo   Next step: Run build_unified.bat to rebuild
goto :eof

:help
echo Game Engine Kiro - Build Cleaner
echo.
echo Usage: build_clean.bat [command]
echo.
echo Commands:
echo   engine       Clean only engine artifacts (libraries, object files)
echo   tests        Clean only test artifacts (executables, object files)
echo   projects     Clean only project artifacts (executables, object files)
echo   cache        Clean CMake cache and vcpkg binary cache
echo   all          Clean all build artifacts and cache
echo   selective    Interactive selective cleaning
echo   help, -h     Show this help message
echo.
echo Examples:
echo   build_clean.bat engine      # Clean engine artifacts only
echo   build_clean.bat tests       # Clean test artifacts only
echo   build_clean.bat cache       # Clean all caches
echo   build_clean.bat all         # Clean everything
echo   build_clean.bat selective   # Interactive cleaning
echo.
echo This tool provides granular control over build artifact cleaning:
echo   - Faster than full rebuild when only specific components changed
echo   - Useful for troubleshooting incremental build issues
echo   - Helps manage disk space by cleaning unused artifacts
echo   - Safe alternative to manually deleting build files

:end
echo.