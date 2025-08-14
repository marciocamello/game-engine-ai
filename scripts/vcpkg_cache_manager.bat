@echo off
setlocal enabledelayedexpansion
REM vcpkg Cache Manager - Validation, Statistics, and Maintenance
REM Part of Game Engine Kiro Modern Build System

echo ========================================
echo vcpkg Cache Manager
echo ========================================

set "VCPKG_CACHE_DIR=%USERPROFILE%\.vcpkg-cache"
set "ACTION=%1"

if "%ACTION%"=="" goto :show_help
if /i "%ACTION%"=="status" goto :show_status
if /i "%ACTION%"=="validate" goto :validate_cache
if /i "%ACTION%"=="clean" goto :clean_cache
if /i "%ACTION%"=="reset" goto :reset_cache
if /i "%ACTION%"=="stats" goto :detailed_stats
if /i "%ACTION%"=="help" goto :show_help

echo ERROR: Unknown action "%ACTION%"
goto :show_help

:show_status
echo Cache Status Report
echo -------------------
if not exist "%VCPKG_CACHE_DIR%" (
    echo Cache Directory: Not created
    echo Status: Cache not initialized
    goto :end
)

echo Cache Directory: %VCPKG_CACHE_DIR%
echo Directory Status: Exists

REM Check permissions
echo test > "%VCPKG_CACHE_DIR%\test_write.tmp" 2>nul
if errorlevel 1 (
    echo Permissions: Write access FAILED
    echo Status: Cache not functional
) else (
    del "%VCPKG_CACHE_DIR%\test_write.tmp" 2>nul
    echo Permissions: Read/Write OK
)

REM Count packages and calculate size
for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set CACHE_PACKAGES=%%i
set CACHE_SIZE_BYTES=0
for /f "tokens=3" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| findstr /r "[0-9].*\.zip"') do (
    set /a CACHE_SIZE_BYTES+=%%a
)
set /a CACHE_SIZE_MB=!CACHE_SIZE_BYTES!/1048576

echo Cached Packages: !CACHE_PACKAGES!
echo Cache Size: !CACHE_SIZE_MB! MB
if !CACHE_SIZE_MB! gtr 1000 echo Maintenance: Consider cleaning ^(large cache size^)

REM Check available space using PowerShell with GB conversion
for /f %%a in ('powershell -Command "try { $disk = Get-WmiObject -Class Win32_LogicalDisk | Where-Object {$_.DeviceID -eq (Split-Path '%VCPKG_CACHE_DIR%' -Qualifier)}; [math]::Round($disk.FreeSpace / 1GB, 1) } catch { 0 }"') do set FREE_SPACE_GB=%%a
if defined FREE_SPACE_GB (
    echo Available Space: !FREE_SPACE_GB! GB
    REM Convert to MB for comparison (approximate)
    for /f %%b in ('powershell -Command "!FREE_SPACE_GB! * 1024"') do set FREE_SPACE_MB=%%b
    if !FREE_SPACE_MB! lss 100 echo Warning: Low disk space
)

goto :end

:validate_cache
echo Validating Cache Integrity
echo --------------------------
if not exist "%VCPKG_CACHE_DIR%" (
    echo Cache directory does not exist
    goto :end
)

set CORRUPTED_COUNT=0
set VALIDATED_COUNT=0

echo Checking cache files...
for %%f in ("%VCPKG_CACHE_DIR%\*.zip") do (
    set /a VALIDATED_COUNT+=1
    echo Validating: %%~nxf
    powershell -Command "try { Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::OpenRead('%%f').Dispose(); Write-Host '  OK' -ForegroundColor Green } catch { Write-Host '  CORRUPTED' -ForegroundColor Red; exit 1 }" 2>nul
    if errorlevel 1 (
        set /a CORRUPTED_COUNT+=1
        echo   Removing corrupted file: %%~nxf
        del "%%f" 2>nul
    )
)

echo.
echo Validation Results:
echo   Files Checked: !VALIDATED_COUNT!
echo   Corrupted Files: !CORRUPTED_COUNT!
echo   Files Removed: !CORRUPTED_COUNT!

if !CORRUPTED_COUNT! gtr 0 (
    echo   Status: Cache cleaned, corrupted files removed
) else (
    echo   Status: Cache integrity OK
)

goto :end

:clean_cache
echo Cleaning Cache
echo --------------
if not exist "%VCPKG_CACHE_DIR%" (
    echo Cache directory does not exist
    goto :end
)

echo WARNING: This will remove all cached packages
set /p CONFIRM="Continue? (y/N): "
if /i not "%CONFIRM%"=="y" (
    echo Operation cancelled
    goto :end
)

echo Removing cached packages...
del "%VCPKG_CACHE_DIR%\*.zip" 2>nul
echo Cache cleaned successfully

goto :end

:reset_cache
echo Resetting Cache
echo ---------------
echo WARNING: This will completely remove and recreate the cache directory
set /p CONFIRM="Continue? (y/N): "
if /i not "%CONFIRM%"=="y" (
    echo Operation cancelled
    goto :end
)

echo Removing cache directory...
if exist "%VCPKG_CACHE_DIR%" rmdir /s /q "%VCPKG_CACHE_DIR%"

echo Recreating cache directory...
mkdir "%VCPKG_CACHE_DIR%" 2>nul
if errorlevel 1 (
    echo ERROR: Failed to recreate cache directory
    goto :end
)

echo Cache reset successfully

goto :end

:detailed_stats
echo Detailed Cache Statistics
echo -------------------------
if not exist "%VCPKG_CACHE_DIR%" (
    echo Cache directory does not exist
    goto :end
)

echo Cache Directory: %VCPKG_CACHE_DIR%
echo.

REM Package statistics
for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set TOTAL_PACKAGES=%%i
echo Total Packages: !TOTAL_PACKAGES!

if !TOTAL_PACKAGES! gtr 0 (
    echo.
    echo Package Details:
    echo ----------------
    for %%f in ("%VCPKG_CACHE_DIR%\*.zip") do (
        set "FILENAME=%%~nxf"
        set "FILESIZE=%%~zf"
        set /a FILESIZE_MB=!FILESIZE!/1048576
        echo   !FILENAME! ^(!FILESIZE_MB! MB^)
    )
)

REM Size statistics
set CACHE_SIZE_BYTES=0
for /f "tokens=3" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| findstr /r "[0-9].*\.zip"') do (
    set /a CACHE_SIZE_BYTES+=%%a
)
set /a CACHE_SIZE_MB=!CACHE_SIZE_BYTES!/1048576

echo.
echo Size Statistics:
echo ----------------
echo Total Size: !CACHE_SIZE_MB! MB ^(!CACHE_SIZE_BYTES! bytes^)
if !TOTAL_PACKAGES! gtr 0 (
    set /a AVG_SIZE_MB=!CACHE_SIZE_MB!/!TOTAL_PACKAGES!
    echo Average Package Size: !AVG_SIZE_MB! MB
)

REM Disk space using PowerShell with GB conversion
for /f %%a in ('powershell -Command "try { $disk = Get-WmiObject -Class Win32_LogicalDisk | Where-Object {$_.DeviceID -eq (Split-Path '%VCPKG_CACHE_DIR%' -Qualifier)}; [math]::Round($disk.FreeSpace / 1GB, 1) } catch { 0 }"') do set FREE_SPACE_GB=%%a
if defined FREE_SPACE_GB (
    echo Available Disk Space: !FREE_SPACE_GB! GB
    if !CACHE_SIZE_MB! gtr 0 (
        REM Simple usage indication
        echo Cache Size: !CACHE_SIZE_MB! MB
    )
)

goto :end

:show_help
echo vcpkg Cache Manager - Usage
echo.
echo Usage: vcpkg_cache_manager.bat [action]
echo.
echo Actions:
echo   status     Show cache status and basic statistics
echo   validate   Validate cache integrity and remove corrupted files
echo   clean      Remove all cached packages ^(interactive^)
echo   reset      Completely reset cache directory ^(interactive^)
echo   stats      Show detailed cache statistics
echo   help       Show this help message
echo.
echo Examples:
echo   vcpkg_cache_manager.bat status     # Quick status check
echo   vcpkg_cache_manager.bat validate   # Check and fix cache integrity
echo   vcpkg_cache_manager.bat stats      # Detailed statistics
echo   vcpkg_cache_manager.bat clean      # Clean cache ^(interactive^)
echo.
echo Cache Location: %USERPROFILE%\.vcpkg-cache
echo.

:end