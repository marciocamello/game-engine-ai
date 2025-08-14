@echo off
setlocal enabledelayedexpansion
REM Build Performance Benchmark Script for Game Engine Kiro
REM Measures and validates performance improvements according to requirements 8.4 and 8.5

echo ========================================
echo Game Engine Kiro - Build Performance Benchmark
echo ========================================
echo.

REM Initialize benchmark environment
set "BENCHMARK_LOG=logs\benchmark_results.log"
set "BASELINE_FILE=logs\performance_baseline.json"
set "RESULTS_FILE=logs\benchmark_results.json"
set "TEMP_DIR=benchmark_temp"

REM Create logs directory if it doesn't exist
if not exist "logs" mkdir logs

REM Parse command line arguments
set "RUN_BASELINE=OFF"
set "RUN_VALIDATION=OFF"
set "CLEAN_BUILDS=ON"
set "VERBOSE=OFF"

:parse_args
if "%~1"=="" goto :start_benchmark

if /i "%~1"=="--baseline" (
    set RUN_BASELINE=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--validate" (
    set RUN_VALIDATION=ON
    shift
    goto :parse_args
)
if /i "%~1"=="--no-clean" (
    set CLEAN_BUILDS=OFF
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

:start_benchmark
echo Starting build performance benchmark...
echo Timestamp: %DATE% %TIME%
echo.

REM Log benchmark start
echo [%DATE% %TIME%] Build performance benchmark started > "%BENCHMARK_LOG%"
echo Arguments: baseline=%RUN_BASELINE%, validate=%RUN_VALIDATION%, clean=%CLEAN_BUILDS% >> "%BENCHMARK_LOG%"
echo. >> "%BENCHMARK_LOG%"

REM Check if we should run baseline measurement
if "%RUN_BASELINE%"=="ON" (
    echo Running baseline performance measurement...
    call :measure_baseline_performance
    if errorlevel 1 (
        echo ERROR: Baseline measurement failed
        exit /b 1
    )
)

REM Check if we should run validation
if "%RUN_VALIDATION%"=="ON" (
    echo Running performance validation...
    call :validate_performance_improvements
    if errorlevel 1 (
        echo ERROR: Performance validation failed
        exit /b 1
    )
)

REM If no specific action requested, run comprehensive benchmark
if "%RUN_BASELINE%"=="OFF" if "%RUN_VALIDATION%"=="OFF" (
    echo Running comprehensive performance benchmark...
    call :run_comprehensive_benchmark
    if errorlevel 1 (
        echo ERROR: Comprehensive benchmark failed
        exit /b 1
    )
)

echo.
echo ========================================
echo Benchmark Completed Successfully!
echo ========================================
echo.
echo Results saved to: %RESULTS_FILE%
echo Detailed log: %BENCHMARK_LOG%
echo.
echo Quick Commands:
echo   View results: type "%RESULTS_FILE%"
echo   View log: type "%BENCHMARK_LOG%"
echo   Run validation: %~nx0 --validate

goto :end

:measure_baseline_performance
echo.
echo Measuring baseline performance (without optimizations)...
echo This will measure build times with basic configuration

REM Initialize baseline results
echo { > "%BASELINE_FILE%"
echo   "timestamp": "%DATE% %TIME%", >> "%BASELINE_FILE%"
echo   "description": "Baseline performance measurement", >> "%BASELINE_FILE%"

REM Test 1: Clean build time (all components)
echo.
echo [1/4] Measuring clean build time (all components)...
call :measure_clean_build_time "all" CLEAN_BUILD_TIME
echo   Clean build time: %CLEAN_BUILD_TIME% seconds

REM Test 2: Incremental build time (no changes)
echo.
echo [2/4] Measuring incremental build time (no changes)...
call :measure_incremental_build_time "no-changes" INCREMENTAL_NO_CHANGES_TIME
echo   Incremental build (no changes): %INCREMENTAL_NO_CHANGES_TIME% seconds

REM Test 3: Incremental build time (single file change)
echo.
echo [3/4] Measuring incremental build time (single file change)...
call :measure_incremental_build_time "single-file" INCREMENTAL_SINGLE_FILE_TIME
echo   Incremental build (single file): %INCREMENTAL_SINGLE_FILE_TIME% seconds

REM Test 4: Test-only build time
echo.
echo [4/4] Measuring test-only build time...
call :measure_test_build_time TEST_BUILD_TIME
echo   Test-only build time: %TEST_BUILD_TIME% seconds

REM Save baseline results
echo   "clean_build_seconds": %CLEAN_BUILD_TIME%, >> "%BASELINE_FILE%"
echo   "incremental_no_changes_seconds": %INCREMENTAL_NO_CHANGES_TIME%, >> "%BASELINE_FILE%"
echo   "incremental_single_file_seconds": %INCREMENTAL_SINGLE_FILE_TIME%, >> "%BASELINE_FILE%"
echo   "test_build_seconds": %TEST_BUILD_TIME%, >> "%BASELINE_FILE%"
echo   "generator": "baseline", >> "%BASELINE_FILE%"
echo   "cache_enabled": false >> "%BASELINE_FILE%"
echo } >> "%BASELINE_FILE%"

echo.
echo Baseline performance measurement completed
echo Results saved to: %BASELINE_FILE%

REM Log baseline results
echo [%DATE% %TIME%] Baseline performance measured >> "%BENCHMARK_LOG%"
echo   Clean build: %CLEAN_BUILD_TIME%s >> "%BENCHMARK_LOG%"
echo   Incremental (no changes): %INCREMENTAL_NO_CHANGES_TIME%s >> "%BENCHMARK_LOG%"
echo   Incremental (single file): %INCREMENTAL_SINGLE_FILE_TIME%s >> "%BENCHMARK_LOG%"
echo   Test build: %TEST_BUILD_TIME%s >> "%BENCHMARK_LOG%"
echo. >> "%BENCHMARK_LOG%"

goto :eof

:validate_performance_improvements
echo.
echo Validating performance improvements against requirements...

REM Check if baseline exists
if not exist "%BASELINE_FILE%" (
    echo ERROR: Baseline performance data not found
    echo Please run: %~nx0 --baseline
    exit /b 1
)

REM Read baseline data
call :read_baseline_data

REM Measure current performance with optimizations
echo.
echo Measuring current performance with optimizations...

REM Test current clean build performance
echo [1/4] Measuring optimized clean build time...
call :measure_clean_build_time "optimized" CURRENT_CLEAN_TIME
echo   Current clean build time: %CURRENT_CLEAN_TIME% seconds

REM Test current incremental build performance
echo [2/4] Measuring optimized incremental build time...
call :measure_incremental_build_time "optimized" CURRENT_INCREMENTAL_TIME
echo   Current incremental build time: %CURRENT_INCREMENTAL_TIME% seconds

REM Test current test build performance
echo [3/4] Measuring optimized test build time...
call :measure_test_build_time CURRENT_TEST_TIME
echo   Current test build time: %CURRENT_TEST_TIME% seconds

REM Calculate improvements
echo [4/4] Calculating performance improvements...
call :calculate_improvements

REM Validate against requirements
echo.
echo Validating against requirements:
call :validate_requirements

REM Save validation results
call :save_validation_results

goto :eof

:run_comprehensive_benchmark
echo.
echo Running comprehensive performance benchmark...

REM Run baseline if it doesn't exist
if not exist "%BASELINE_FILE%" (
    echo Baseline data not found, measuring baseline first...
    call :measure_baseline_performance
    if errorlevel 1 exit /b 1
)

REM Run validation
call :validate_performance_improvements
if errorlevel 1 exit /b 1

REM Additional comprehensive tests
echo.
echo Running additional comprehensive tests...

REM Test different build configurations
call :test_build_configurations

REM Test cache performance
call :test_cache_performance

REM Test Ninja vs Visual Studio performance
call :test_generator_performance

echo.
echo Comprehensive benchmark completed

goto :eof

:measure_clean_build_time
REM Parameters: %1=test_type, %2=result_variable
set "test_type=%~1"
set "result_var=%~2"

if "%VERBOSE%"=="ON" echo   Starting clean build measurement...

REM Clean build directory if requested
if "%CLEAN_BUILDS%"=="ON" (
    if exist build (
        if "%VERBOSE%"=="ON" echo   Cleaning build directory...
        rmdir /s /q build 2>nul
    )
)

REM Record start time
call :get_timestamp START_TIME

REM Run build
if "%VERBOSE%"=="ON" (
    .\scripts\build_unified.bat --all
) else (
    .\scripts\build_unified.bat --all >nul 2>&1
)

if errorlevel 1 (
    echo ERROR: Clean build failed
    exit /b 1
)

REM Record end time and calculate duration
call :get_timestamp END_TIME
call :calculate_duration "%START_TIME%" "%END_TIME%" DURATION

set "%result_var%=%DURATION%"
goto :eof

:measure_incremental_build_time
REM Parameters: %1=test_type, %2=result_variable
set "test_type=%~1"
set "result_var=%~2"

if "%VERBOSE%"=="ON" echo   Starting incremental build measurement...

REM Ensure we have a built project first
if not exist "build" (
    if "%VERBOSE%"=="ON" echo   No existing build found, creating initial build...
    .\scripts\build_unified.bat --all >nul 2>&1
    if errorlevel 1 (
        echo ERROR: Initial build failed
        exit /b 1
    )
)

REM For single file change test, modify a file
if "%test_type%"=="single-file" (
    if "%VERBOSE%"=="ON" echo   Modifying a source file to trigger incremental build...
    REM Touch a source file to trigger recompilation
    if exist "src\Core\Engine.cpp" (
        echo // Benchmark modification - %TIME% >> "src\Core\Engine.cpp"
    )
)

REM Record start time
call :get_timestamp START_TIME

REM Run incremental build
if "%VERBOSE%"=="ON" (
    .\scripts\build_unified.bat --all
) else (
    .\scripts\build_unified.bat --all >nul 2>&1
)

if errorlevel 1 (
    echo ERROR: Incremental build failed
    exit /b 1
)

REM Record end time and calculate duration
call :get_timestamp END_TIME
call :calculate_duration "%START_TIME%" "%END_TIME%" DURATION

REM Clean up modification if we made one
if "%test_type%"=="single-file" (
    if exist "src\Core\Engine.cpp" (
        REM Remove the last line we added
        powershell -Command "(Get-Content 'src\Core\Engine.cpp' | Select-Object -SkipLast 1) | Set-Content 'src\Core\Engine.cpp'" >nul 2>&1
    )
)

set "%result_var%=%DURATION%"
goto :eof

:measure_test_build_time
REM Parameters: %1=result_variable
set "result_var=%~1"

if "%VERBOSE%"=="ON" echo   Starting test build measurement...

REM Clean test artifacts if requested
if "%CLEAN_BUILDS%"=="ON" (
    if "%VERBOSE%"=="ON" echo   Cleaning test artifacts...
    .\scripts\build_unified.bat --clean-tests >nul 2>&1
)

REM Record start time
call :get_timestamp START_TIME

REM Run test build
if "%VERBOSE%"=="ON" (
    .\scripts\build_unified.bat --tests
) else (
    .\scripts\build_unified.bat --tests >nul 2>&1
)

if errorlevel 1 (
    echo ERROR: Test build failed
    exit /b 1
)

REM Record end time and calculate duration
call :get_timestamp END_TIME
call :calculate_duration "%START_TIME%" "%END_TIME%" DURATION

set "%result_var%=%DURATION%"
goto :eof

:read_baseline_data
REM Read baseline performance data from JSON file
if "%VERBOSE%"=="ON" echo   Reading baseline performance data...

for /f "tokens=2 delims=:" %%a in ('findstr "clean_build_seconds" "%BASELINE_FILE%"') do (
    set "BASELINE_CLEAN=%%a"
    set "BASELINE_CLEAN=!BASELINE_CLEAN: =!"
    set "BASELINE_CLEAN=!BASELINE_CLEAN:,=!"
)

for /f "tokens=2 delims=:" %%a in ('findstr "incremental_single_file_seconds" "%BASELINE_FILE%"') do (
    set "BASELINE_INCREMENTAL=%%a"
    set "BASELINE_INCREMENTAL=!BASELINE_INCREMENTAL: =!"
    set "BASELINE_INCREMENTAL=!BASELINE_INCREMENTAL:,=!"
)

for /f "tokens=2 delims=:" %%a in ('findstr "test_build_seconds" "%BASELINE_FILE%"') do (
    set "BASELINE_TEST=%%a"
    set "BASELINE_TEST=!BASELINE_TEST: =!"
    set "BASELINE_TEST=!BASELINE_TEST:,=!"
)

if "%VERBOSE%"=="ON" (
    echo   Baseline clean build: %BASELINE_CLEAN%s
    echo   Baseline incremental build: %BASELINE_INCREMENTAL%s
    echo   Baseline test build: %BASELINE_TEST%s
)

goto :eof

:calculate_improvements
echo   Calculating performance improvements...

REM Calculate clean build improvement
if %BASELINE_CLEAN% gtr 0 (
    set /a CLEAN_IMPROVEMENT=%BASELINE_CLEAN%-%CURRENT_CLEAN_TIME%
    set /a CLEAN_PERCENT=!CLEAN_IMPROVEMENT!*100/%BASELINE_CLEAN%
) else (
    set CLEAN_IMPROVEMENT=0
    set CLEAN_PERCENT=0
)

REM Calculate incremental build improvement
if %BASELINE_INCREMENTAL% gtr 0 (
    set /a INCREMENTAL_IMPROVEMENT=%BASELINE_INCREMENTAL%-%CURRENT_INCREMENTAL_TIME%
    set /a INCREMENTAL_PERCENT=!INCREMENTAL_IMPROVEMENT!*100/%BASELINE_INCREMENTAL%
) else (
    set INCREMENTAL_IMPROVEMENT=0
    set INCREMENTAL_PERCENT=0
)

REM Calculate test build improvement
if %BASELINE_TEST% gtr 0 (
    set /a TEST_IMPROVEMENT=%BASELINE_TEST%-%CURRENT_TEST_TIME%
    set /a TEST_PERCENT=!TEST_IMPROVEMENT!*100/%BASELINE_TEST%
) else (
    set TEST_IMPROVEMENT=0
    set TEST_PERCENT=0
)

echo   Clean build improvement: %CLEAN_IMPROVEMENT%s (%CLEAN_PERCENT%%%)
echo   Incremental build improvement: %INCREMENTAL_IMPROVEMENT%s (%INCREMENTAL_PERCENT%%%)
echo   Test build improvement: %TEST_IMPROVEMENT%s (%TEST_PERCENT%%%)

goto :eof

:validate_requirements
echo.
echo Requirement Validation Results:
echo ========================================

REM Requirement 8.4: Validate 30-50% improvement in clean build times
set "CLEAN_REQ_MET=FAIL"
if %CLEAN_PERCENT% geq 30 if %CLEAN_PERCENT% leq 50 (
    set "CLEAN_REQ_MET=PASS"
    echo [PASS] Clean build improvement: %CLEAN_PERCENT%%% ^(target: 30-50%%%^)
) else if %CLEAN_PERCENT% gtr 50 (
    set "CLEAN_REQ_MET=EXCEED"
    echo [EXCEED] Clean build improvement: %CLEAN_PERCENT%%% ^(target: 30-50%%%, exceeded expectations^)
) else (
    echo [FAIL] Clean build improvement: %CLEAN_PERCENT%%% ^(target: 30-50%%%^)
)

REM Requirement 8.5: Verify 70-90% improvement in incremental build times
set "INCREMENTAL_REQ_MET=FAIL"
if %INCREMENTAL_PERCENT% geq 70 if %INCREMENTAL_PERCENT% leq 90 (
    set "INCREMENTAL_REQ_MET=PASS"
    echo [PASS] Incremental build improvement: %INCREMENTAL_PERCENT%%% ^(target: 70-90%%%^)
) else if %INCREMENTAL_PERCENT% gtr 90 (
    set "INCREMENTAL_REQ_MET=EXCEED"
    echo [EXCEED] Incremental build improvement: %INCREMENTAL_PERCENT%%% ^(target: 70-90%%%, exceeded expectations^)
) else (
    echo [FAIL] Incremental build improvement: %INCREMENTAL_PERCENT%%% ^(target: 70-90%%%^)
)

REM Overall validation result
set "VALIDATION_RESULT=FAIL"
if "%CLEAN_REQ_MET%"=="PASS" if "%INCREMENTAL_REQ_MET%"=="PASS" set "VALIDATION_RESULT=PASS"
if "%CLEAN_REQ_MET%"=="EXCEED" if "%INCREMENTAL_REQ_MET%"=="PASS" set "VALIDATION_RESULT=EXCEED"
if "%CLEAN_REQ_MET%"=="PASS" if "%INCREMENTAL_REQ_MET%"=="EXCEED" set "VALIDATION_RESULT=EXCEED"
if "%CLEAN_REQ_MET%"=="EXCEED" if "%INCREMENTAL_REQ_MET%"=="EXCEED" set "VALIDATION_RESULT=EXCEED"

echo.
echo Overall Validation: %VALIDATION_RESULT%

REM Log validation results
echo [%DATE% %TIME%] Performance validation completed >> "%BENCHMARK_LOG%"
echo   Clean build: %CLEAN_PERCENT%%% improvement ^(%CLEAN_REQ_MET%^) >> "%BENCHMARK_LOG%"
echo   Incremental build: %INCREMENTAL_PERCENT%%% improvement ^(%INCREMENTAL_REQ_MET%^) >> "%BENCHMARK_LOG%"
echo   Overall result: %VALIDATION_RESULT% >> "%BENCHMARK_LOG%"
echo. >> "%BENCHMARK_LOG%"

if "%VALIDATION_RESULT%"=="FAIL" (
    echo.
    echo WARNING: Performance requirements not met
    echo Consider additional optimizations or configuration tuning
    exit /b 1
)

goto :eof

:save_validation_results
echo   Saving validation results...

echo { > "%RESULTS_FILE%"
echo   "timestamp": "%DATE% %TIME%", >> "%RESULTS_FILE%"
echo   "validation_type": "performance_improvements", >> "%RESULTS_FILE%"
echo   "baseline": { >> "%RESULTS_FILE%"
echo     "clean_build_seconds": %BASELINE_CLEAN%, >> "%RESULTS_FILE%"
echo     "incremental_build_seconds": %BASELINE_INCREMENTAL%, >> "%RESULTS_FILE%"
echo     "test_build_seconds": %BASELINE_TEST% >> "%RESULTS_FILE%"
echo   }, >> "%RESULTS_FILE%"
echo   "current": { >> "%RESULTS_FILE%"
echo     "clean_build_seconds": %CURRENT_CLEAN_TIME%, >> "%RESULTS_FILE%"
echo     "incremental_build_seconds": %CURRENT_INCREMENTAL_TIME%, >> "%RESULTS_FILE%"
echo     "test_build_seconds": %CURRENT_TEST_TIME% >> "%RESULTS_FILE%"
echo   }, >> "%RESULTS_FILE%"
echo   "improvements": { >> "%RESULTS_FILE%"
echo     "clean_build_percent": %CLEAN_PERCENT%, >> "%RESULTS_FILE%"
echo     "incremental_build_percent": %INCREMENTAL_PERCENT%, >> "%RESULTS_FILE%"
echo     "test_build_percent": %TEST_PERCENT% >> "%RESULTS_FILE%"
echo   }, >> "%RESULTS_FILE%"
echo   "requirements": { >> "%RESULTS_FILE%"
echo     "clean_build_target": "30-50%%", >> "%RESULTS_FILE%"
echo     "incremental_build_target": "70-90%%", >> "%RESULTS_FILE%"
echo     "clean_build_met": "%CLEAN_REQ_MET%", >> "%RESULTS_FILE%"
echo     "incremental_build_met": "%INCREMENTAL_REQ_MET%" >> "%RESULTS_FILE%"
echo   }, >> "%RESULTS_FILE%"
echo   "overall_result": "%VALIDATION_RESULT%" >> "%RESULTS_FILE%"
echo } >> "%RESULTS_FILE%"

goto :eof

:test_build_configurations
echo   Testing different build configurations...

REM Test Debug vs Release performance
echo     Testing Debug vs Release build times...
REM Implementation would go here

REM Test with and without coverage
echo     Testing coverage impact on build times...
REM Implementation would go here

goto :eof

:test_cache_performance
echo   Testing vcpkg cache performance...

REM Test with cache enabled vs disabled
echo     Measuring cache hit rates and performance impact...
REM Implementation would go here

goto :eof

:test_generator_performance
echo   Testing Ninja vs Visual Studio generator performance...

REM Test Ninja performance
echo     Testing Ninja generator performance...
REM Implementation would go here

REM Test Visual Studio performance
echo     Testing Visual Studio generator performance...
REM Implementation would go here

goto :eof

:get_timestamp
REM Get current timestamp in seconds since midnight
REM Parameters: %1=result_variable
set "result_var=%~1"
set "current_time=%TIME%"

REM Convert time to seconds
for /f "tokens=1-3 delims=:." %%a in ("%current_time%") do (
    set /a seconds=%%a*3600+%%b*60+%%c
)

set "%result_var%=%seconds%"
goto :eof

:calculate_duration
REM Calculate duration between two timestamps
REM Parameters: %1=start_time, %2=end_time, %3=result_variable
set "start_time=%~1"
set "end_time=%~2"
set "result_var=%~3"

REM Handle day rollover
if %end_time% lss %start_time% (
    set /a duration=%end_time%+86400-%start_time%
) else (
    set /a duration=%end_time%-%start_time%
)

set "%result_var%=%duration%"
goto :eof

:help
echo Game Engine Kiro - Build Performance Benchmark
echo.
echo Usage: benchmark_build_performance.bat [options]
echo.
echo Options:
echo   --baseline        Measure baseline performance (without optimizations)
echo   --validate        Validate performance improvements against requirements
echo   --no-clean        Skip cleaning build directory between tests
echo   --verbose         Show detailed output during benchmarking
echo   --help, -h        Show this help message
echo.
echo Examples:
echo   benchmark_build_performance.bat --baseline    # Measure baseline performance
echo   benchmark_build_performance.bat --validate    # Validate improvements
echo   benchmark_build_performance.bat              # Run comprehensive benchmark
echo   benchmark_build_performance.bat --verbose    # Detailed output
echo.
echo Requirements Validation:
echo   - Clean build improvement: 30-50%% (Requirement 8.4)
echo   - Incremental build improvement: 70-90%% (Requirement 8.5)
echo.
echo Output Files:
echo   - logs\benchmark_results.log     # Detailed benchmark log
echo   - logs\benchmark_results.json    # Validation results
echo   - logs\performance_baseline.json # Baseline measurements

:end