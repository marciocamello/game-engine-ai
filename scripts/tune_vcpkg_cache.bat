@echo off
setlocal enabledelayedexpansion
REM vcpkg Cache Tuning Script for Game Engine Kiro
REM Dynamically adjusts cache settings based on usage patterns and system performance

echo ========================================
echo Game Engine Kiro - vcpkg Cache Tuner
echo ========================================
echo.

REM Parse command line arguments
set "ANALYZE_ONLY=OFF"
set "RESET_CACHE=OFF"
set "VERBOSE=OFF"

:parse_args
if "%~1"=="" goto :start_tuning

if /i "%~1"=="--analyze" (
    set ANALYZE_ONLY=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--reset" (
    set RESET_CACHE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--verbose" (
    set VERBOSE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--help" goto :help
if /i "%~1"=="-h" goto :help

echo ERROR: Unknown argument "%~1"
echo Use --help for usage information
exit /b 1

:start_tuning
echo Starting vcpkg cache analysis and tuning...
echo Timestamp: %DATE% %TIME%
echo.

REM Set cache directory
set "VCPKG_CACHE_DIR=%USERPROFILE%\.vcpkg-cache"

REM Analyze current cache state
echo Analyzing current cache state...
call :analyze_cache_state

REM Reset cache if requested
if "%RESET_CACHE%"=="ON" (
    echo Resetting vcpkg cache...
    call :reset_cache
)

REM Only analyze if requested
if "%ANALYZE_ONLY%"=="ON" (
    echo Analysis complete. No changes made.
    goto :show_recommendations
)

REM Tune cache settings based on analysis
echo Tuning cache settings based on analysis...
call :tune_cache_settings

REM Apply optimizations
echo Applying cache optimizations...
call :apply_cache_optimizations

:show_recommendations
echo.
echo ========================================
echo Cache Tuning Complete!
echo ========================================
echo.
echo Cache Statistics:
echo   Location: %VCPKG_CACHE_DIR%
echo   Size: %CACHE_SIZE_MB%MB
echo   Packages: %CACHE_PACKAGES%
echo   Hit Rate: %ESTIMATED_HIT_RATE%%%
echo.
echo Recommendations:
call :show_recommendations_detail

goto :end

:analyze_cache_state
echo   Analyzing cache directory: %VCPKG_CACHE_DIR%

REM Initialize variables
set "CACHE_SIZE_MB=0"
set "CACHE_PACKAGES=0"
set "CACHE_AGE_DAYS=0"
set "ESTIMATED_HIT_RATE=0"

REM Check if cache directory exists
if not exist "%VCPKG_CACHE_DIR%" (
    echo   Cache directory does not exist - will be created
    mkdir "%VCPKG_CACHE_DIR%" 2>nul
    goto :eof
)

REM Count cache packages
for /f %%i in ('dir /b "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| find /c /v ""') do set CACHE_PACKAGES=%%i

REM Calculate cache size
set CACHE_SIZE_BYTES=0
for /f "tokens=3" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| findstr /r "[0-9].*\.zip"') do (
    set /a CACHE_SIZE_BYTES+=%%a 2>nul
)
if %CACHE_SIZE_BYTES% gtr 0 (
    set /a CACHE_SIZE_MB=%CACHE_SIZE_BYTES%/1048576
) else (
    set CACHE_SIZE_MB=0
)

REM Estimate hit rate based on recent builds
call :estimate_hit_rate

REM Analyze cache age
call :analyze_cache_age

echo   Cache packages: %CACHE_PACKAGES%
echo   Cache size: %CACHE_SIZE_MB%MB
echo   Estimated hit rate: %ESTIMATED_HIT_RATE%%%
echo   Cache age: %CACHE_AGE_DAYS% days

goto :eof

:estimate_hit_rate
REM Estimate cache hit rate based on build performance data
set "ESTIMATED_HIT_RATE=0"

if exist "logs\build_performance.json" (
    REM Try to extract cache hit rate from recent builds
    for /f "tokens=2 delims=:" %%a in ('findstr "cache_hit_rate" "logs\build_performance.json" 2^>nul') do (
        set "ESTIMATED_HIT_RATE=%%a"
        set "ESTIMATED_HIT_RATE=!ESTIMATED_HIT_RATE: =!"
        set "ESTIMATED_HIT_RATE=!ESTIMATED_HIT_RATE:,=!"
    )
)

REM If no data available, estimate based on cache size
if %ESTIMATED_HIT_RATE% equ 0 (
    if %CACHE_PACKAGES% gtr 20 (
        set "ESTIMATED_HIT_RATE=80"
    ) else if %CACHE_PACKAGES% gtr 10 (
        set "ESTIMATED_HIT_RATE=60"
    ) else if %CACHE_PACKAGES% gtr 5 (
        set "ESTIMATED_HIT_RATE=40"
    ) else (
        set "ESTIMATED_HIT_RATE=20"
    )
)

goto :eof

:analyze_cache_age
REM Analyze cache age to determine if cleanup is needed
set "CACHE_AGE_DAYS=0"

if exist "%VCPKG_CACHE_DIR%" (
    REM Get the oldest file in cache to estimate age
    for /f "tokens=1" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" /od 2^>nul ^| findstr /r "[0-9][0-9]/[0-9][0-9]/[0-9][0-9][0-9][0-9]"') do (
        REM Simple age estimation - would need more complex date math for accuracy
        set "CACHE_AGE_DAYS=30"
        goto :age_done
    )
)

:age_done
goto :eof

:reset_cache
echo   Resetting vcpkg binary cache...

if exist "%VCPKG_CACHE_DIR%" (
    echo   Removing existing cache directory...
    rmdir /s /q "%VCPKG_CACHE_DIR%" 2>nul
)

echo   Creating fresh cache directory...
mkdir "%VCPKG_CACHE_DIR%" 2>nul
mkdir "%VCPKG_CACHE_DIR%\packages" 2>nul
mkdir "%VCPKG_CACHE_DIR%\temp" 2>nul

echo   Cache reset complete

REM Reset statistics
set "CACHE_SIZE_MB=0"
set "CACHE_PACKAGES=0"
set "ESTIMATED_HIT_RATE=0"

goto :eof

:tune_cache_settings
echo   Determining optimal cache settings...

REM Determine optimal cache mode based on system and usage
if %CACHE_SIZE_MB% gtr 2048 (
    set "OPTIMAL_CACHE_MODE=readwrite"
    echo   Large cache detected - enabling read/write mode
) else if %ESTIMATED_HIT_RATE% gtr 70 (
    set "OPTIMAL_CACHE_MODE=readwrite"
    echo   High hit rate detected - enabling read/write mode
) else (
    set "OPTIMAL_CACHE_MODE=read"
    echo   Conservative cache mode selected
)

REM Determine optimal concurrency based on system specs
for /f "tokens=2 delims=:" %%a in ('wmic computersystem get TotalPhysicalMemory /value 2^>nul ^| findstr "="') do (
    if not "%%a"=="" (
        set "TOTAL_RAM_BYTES=%%a"
        set /a TOTAL_RAM_GB=!TOTAL_RAM_BYTES!/1073741824
    )
)
if not defined TOTAL_RAM_GB set "TOTAL_RAM_GB=8"

if %TOTAL_RAM_GB% geq 16 (
    set "OPTIMAL_CONCURRENCY=4"
) else if %TOTAL_RAM_GB% geq 8 (
    set "OPTIMAL_CONCURRENCY=2"
) else (
    set "OPTIMAL_CONCURRENCY=1"
)

echo   Optimal cache mode: %OPTIMAL_CACHE_MODE%
echo   Optimal concurrency: %OPTIMAL_CONCURRENCY%

goto :eof

:apply_cache_optimizations
echo   Applying cache optimizations...

REM Update environment variables for current session
set "VCPKG_BINARY_SOURCES=files,%VCPKG_CACHE_DIR%,%OPTIMAL_CACHE_MODE%"
set "VCPKG_MAX_CONCURRENCY=%OPTIMAL_CONCURRENCY%"
set "VCPKG_FEATURE_FLAGS=manifests,binarycaching,versions"

REM Create cache configuration file
echo # vcpkg Cache Configuration > "%VCPKG_CACHE_DIR%\cache_config.txt"
echo # Auto-generated by tune_vcpkg_cache.bat >> "%VCPKG_CACHE_DIR%\cache_config.txt"
echo # >> "%VCPKG_CACHE_DIR%\cache_config.txt"
echo Cache Mode: %OPTIMAL_CACHE_MODE% >> "%VCPKG_CACHE_DIR%\cache_config.txt"
echo Max Concurrency: %OPTIMAL_CONCURRENCY% >> "%VCPKG_CACHE_DIR%\cache_config.txt"
echo Cache Directory: %VCPKG_CACHE_DIR% >> "%VCPKG_CACHE_DIR%\cache_config.txt"
echo Tuned On: %DATE% %TIME% >> "%VCPKG_CACHE_DIR%\cache_config.txt"

REM Clean old cache files if cache is too large
if %CACHE_SIZE_MB% gtr 5120 (
    echo   Cache size exceeds 5GB - cleaning old packages...
    call :clean_old_cache_files
)

echo   Cache optimizations applied

goto :eof

:clean_old_cache_files
echo     Cleaning old cache files to free space...

REM Delete cache files older than 90 days (simplified approach)
forfiles /p "%VCPKG_CACHE_DIR%" /s /m *.zip /d -90 /c "cmd /c del @path" 2>nul

REM Recalculate cache size after cleanup
set CACHE_SIZE_BYTES=0
for /f "tokens=3" %%a in ('dir "%VCPKG_CACHE_DIR%\*.zip" 2^>nul ^| findstr /r "[0-9].*\.zip"') do (
    set /a CACHE_SIZE_BYTES+=%%a 2>nul
)
if %CACHE_SIZE_BYTES% gtr 0 (
    set /a CACHE_SIZE_MB=%CACHE_SIZE_BYTES%/1048576
) else (
    set CACHE_SIZE_MB=0
)

echo     Cache cleaned - new size: %CACHE_SIZE_MB%MB

goto :eof

:show_recommendations_detail
if %CACHE_SIZE_MB% equ 0 (
    echo   - Run a few builds to populate the cache
    echo   - Use .\scripts\build_unified.bat --all to build all components
)

if %CACHE_SIZE_MB% gtr 5120 (
    echo   - Consider cleaning old cache files: %~nx0 --reset
    echo   - Cache size is very large ^(%CACHE_SIZE_MB%MB^)
)

if %ESTIMATED_HIT_RATE% lss 50 (
    echo   - Low cache hit rate - consider running more builds
    echo   - Check if cache directory is writable
)

if %ESTIMATED_HIT_RATE% gtr 80 (
    echo   - Excellent cache performance!
    echo   - Consider enabling read/write mode for better sharing
)

echo   - Monitor cache performance with build reports
echo   - Use --analyze flag to check cache status without changes

goto :eof

:help
echo Game Engine Kiro - vcpkg Cache Tuner
echo.
echo Usage: tune_vcpkg_cache.bat [options]
echo.
echo Options:
echo   --analyze         Analyze cache state without making changes
echo   --reset           Reset cache completely (removes all cached packages)
echo   --verbose         Show detailed output during tuning
echo   --help, -h        Show this help message
echo.
echo Examples:
echo   tune_vcpkg_cache.bat              # Analyze and tune cache settings
echo   tune_vcpkg_cache.bat --analyze    # Only analyze current state
echo   tune_vcpkg_cache.bat --reset      # Reset cache and apply optimal settings
echo   tune_vcpkg_cache.bat --verbose    # Detailed output
echo.
echo Cache Optimization:
echo   - Automatically detects optimal cache mode (read/readwrite)
echo   - Configures concurrency based on system RAM
echo   - Cleans old cache files when cache becomes too large
echo   - Estimates hit rates based on build performance data
echo.
echo Output:
echo   - Cache statistics and recommendations
echo   - Optimal settings applied to environment
echo   - Configuration saved to cache directory

:end