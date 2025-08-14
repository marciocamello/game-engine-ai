@echo off
setlocal enabledelayedexpansion
REM Performance Benchmarking Test Suite
REM Tests build system performance and generates benchmarks
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Performance Benchmarks
echo ========================================

set "TEST_LOG=logs\performance_benchmarks.log"
set "PERF_LOG=logs\build_performance_data.json"

REM Create logs directory if it doesn't exist
if not exist "logs" mkdir logs

echo [%DATE% %TIME%] Starting performance benchmarks > "%TEST_LOG%"
echo { >> "%PERF_LOG%"
echo   "benchmark_session": "%DATE% %TIME%", >> "%PERF_LOG%"
echo   "tests": [ >> "%PERF_LOG%"

echo.
echo Benchmark 1: Clean build performance...
if exist build rmdir /s /q build 2>nul

set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine
set "result=!errorlevel!"
set "END_TIME=%TIME%"

call :calculate_duration "%START_TIME%" "%END_TIME%" CLEAN_BUILD_TIME

if !result! equ 0 (
    echo [PASS] Clean build completed in !CLEAN_BUILD_TIME! seconds
    echo     { >> "%PERF_LOG%"
    echo       "test": "clean_build_engine", >> "%PERF_LOG%"
    echo       "duration_seconds": !CLEAN_BUILD_TIME!, >> "%PERF_LOG%"
    echo       "status": "pass" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
) else (
    echo [FAIL] Clean build failed
    echo     { >> "%PERF_LOG%"
    echo       "test": "clean_build_engine", >> "%PERF_LOG%"
    echo       "duration_seconds": !CLEAN_BUILD_TIME!, >> "%PERF_LOG%"
    echo       "status": "fail" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
)

echo.
echo Benchmark 2: Incremental build performance...
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine
set "result=!errorlevel!"
set "END_TIME=%TIME%"

call :calculate_duration "%START_TIME%" "%END_TIME%" INCREMENTAL_BUILD_TIME

if !result! equ 0 (
    echo [PASS] Incremental build completed in !INCREMENTAL_BUILD_TIME! seconds
    echo     { >> "%PERF_LOG%"
    echo       "test": "incremental_build_engine", >> "%PERF_LOG%"
    echo       "duration_seconds": !INCREMENTAL_BUILD_TIME!, >> "%PERF_LOG%"
    echo       "status": "pass" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
) else (
    echo [FAIL] Incremental build failed
    echo     { >> "%PERF_LOG%"
    echo       "test": "incremental_build_engine", >> "%PERF_LOG%"
    echo       "duration_seconds": !INCREMENTAL_BUILD_TIME!, >> "%PERF_LOG%"
    echo       "status": "fail" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
)

echo.
echo Benchmark 3: Test build performance...
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --tests
set "result=!errorlevel!"
set "END_TIME=%TIME%"

call :calculate_duration "%START_TIME%" "%END_TIME%" TEST_BUILD_TIME

if !result! equ 0 (
    echo [PASS] Test build completed in !TEST_BUILD_TIME! seconds
    echo     { >> "%PERF_LOG%"
    echo       "test": "build_all_tests", >> "%PERF_LOG%"
    echo       "duration_seconds": !TEST_BUILD_TIME!, >> "%PERF_LOG%"
    echo       "status": "pass" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
) else (
    echo [FAIL] Test build failed
    echo     { >> "%PERF_LOG%"
    echo       "test": "build_all_tests", >> "%PERF_LOG%"
    echo       "duration_seconds": !TEST_BUILD_TIME!, >> "%PERF_LOG%"
    echo       "status": "fail" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
)

echo.
echo Benchmark 4: Specific test build performance...
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --tests MathTest
set "result=!errorlevel!"
set "END_TIME=%TIME%"

call :calculate_duration "%START_TIME%" "%END_TIME%" SPECIFIC_TEST_TIME

if !result! equ 0 (
    echo [PASS] Specific test build completed in !SPECIFIC_TEST_TIME! seconds
    echo     { >> "%PERF_LOG%"
    echo       "test": "build_specific_test", >> "%PERF_LOG%"
    echo       "duration_seconds": !SPECIFIC_TEST_TIME!, >> "%PERF_LOG%"
    echo       "status": "pass" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
) else (
    echo [FAIL] Specific test build failed
    echo     { >> "%PERF_LOG%"
    echo       "test": "build_specific_test", >> "%PERF_LOG%"
    echo       "duration_seconds": !SPECIFIC_TEST_TIME!, >> "%PERF_LOG%"
    echo       "status": "fail" >> "%PERF_LOG%"
    echo     }, >> "%PERF_LOG%"
)

echo.
echo Benchmark 5: Test execution performance...
set "START_TIME=%TIME%"
.\scripts\run_tests.bat --unit
set "result=!errorlevel!"
set "END_TIME=%TIME%"

call :calculate_duration "%START_TIME%" "%END_TIME%" TEST_EXECUTION_TIME

if !result! equ 0 (
    echo [PASS] Test execution completed in !TEST_EXECUTION_TIME! seconds
    echo     { >> "%PERF_LOG%"
    echo       "test": "execute_unit_tests", >> "%PERF_LOG%"
    echo       "duration_seconds": !TEST_EXECUTION_TIME!, >> "%PERF_LOG%"
    echo       "status": "pass" >> "%PERF_LOG%"
    echo     } >> "%PERF_LOG%"
) else (
    echo [FAIL] Test execution failed
    echo     { >> "%PERF_LOG%"
    echo       "test": "execute_unit_tests", >> "%PERF_LOG%"
    echo       "duration_seconds": !TEST_EXECUTION_TIME!, >> "%PERF_LOG%"
    echo       "status": "fail" >> "%PERF_LOG%"
    echo     } >> "%PERF_LOG%"
)

REM Close JSON
echo   ] >> "%PERF_LOG%"
echo } >> "%PERF_LOG%"

echo.
echo ========================================
echo Performance Benchmark Results
echo ========================================
echo Clean Build Time: !CLEAN_BUILD_TIME! seconds
echo Incremental Build Time: !INCREMENTAL_BUILD_TIME! seconds
echo Test Build Time: !TEST_BUILD_TIME! seconds
echo Specific Test Build Time: !SPECIFIC_TEST_TIME! seconds
echo Test Execution Time: !TEST_EXECUTION_TIME! seconds
echo.

REM Performance analysis
if !INCREMENTAL_BUILD_TIME! lss !CLEAN_BUILD_TIME! (
    echo [PASS] Incremental builds are faster than clean builds
) else (
    echo [WARN] Incremental builds not significantly faster
)

if !SPECIFIC_TEST_TIME! lss !TEST_BUILD_TIME! (
    echo [PASS] Specific test builds are faster than full test builds
) else (
    echo [WARN] Specific test builds not significantly faster
)

echo.
echo Performance data saved to: %PERF_LOG%
echo [%DATE% %TIME%] Performance benchmarks completed >> "%TEST_LOG%"

exit /b 0

:calculate_duration
REM Calculate time difference in seconds
REM Parameters: %1=start_time, %2=end_time, %3=result_variable
set "start_time=%~1"
set "end_time=%~2"

REM Simple duration calculation (works for same day)
for /f "tokens=1-3 delims=:." %%a in ("%start_time%") do (
    set /a start_seconds=%%a*3600+%%b*60+%%c
)
for /f "tokens=1-3 delims=:." %%a in ("%end_time%") do (
    set /a end_seconds=%%a*3600+%%b*60+%%c
)

REM Handle day rollover
if %end_seconds% lss %start_seconds% (
    set /a end_seconds+=86400
)

set /a duration=%end_seconds%-%start_seconds%
set "%~3=%duration%"
goto :eof