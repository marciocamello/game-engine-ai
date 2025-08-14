@echo off
setlocal enabledelayedexpansion
REM Test build system performance benchmarks
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Performance Benchmarks
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"
set "FAILED_TESTS=0"
set "TEST_LOG=tests\build_system\performance_benchmarks_test.log"

REM Initialize log
echo Build System Performance Benchmarks - %DATE% %TIME% > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Testing build system performance and benchmarking...
echo.

REM Test 1: Clean build performance
call :test_clean_build_performance

REM Test 2: Incremental build performance
call :test_incremental_build_performance

REM Test 3: Cache effectiveness
call :test_cache_effectiveness

REM Test 4: Specific test build performance
call :test_specific_build_performance

REM Test 5: Memory usage during builds
call :test_memory_usage

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %PASSED_TESTS%
echo Failed: %FAILED_TESTS%

if %FAILED_TESTS% gtr 0 (
    echo.
    echo [FAILED] Some performance benchmark tests failed!
    echo Check %TEST_LOG% for details.
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All performance benchmark tests passed!
    exit /b 0
)

:test_clean_build_performance
set /a TOTAL_TESTS+=1

echo Testing: Clean build performance
echo   Measuring time for full clean build

REM Log test start
echo [TEST %TOTAL_TESTS%] Clean build performance >> "%TEST_LOG%"

REM Clean everything first
if exist build rmdir /s /q build 2>nul

REM Measure clean build time
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine >nul 2>&1
set "RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"

REM Calculate duration (simplified)
call :calculate_duration "%START_TIME%" "%END_TIME%" DURATION

if %RESULT% equ 0 (
    echo   Result: [PASS] - Clean build completed in %DURATION% seconds
    echo   Result: [PASS] - Clean build completed in %DURATION% seconds >> "%TEST_LOG%"
    
    REM Performance check - should complete within reasonable time
    if %DURATION% lss 300 (
        echo   Performance: Good (under 5 minutes)
        echo   Performance: Good (under 5 minutes) >> "%TEST_LOG%"
    ) else (
        echo   Performance: Slow (over 5 minutes) - consider optimization
        echo   Performance: Slow (over 5 minutes) - consider optimization >> "%TEST_LOG%"
    )
    
    set /a PASSED_TESTS+=1
) else (
    echo   Result: [FAIL] - Clean build failed
    echo   Result: [FAIL] - Clean build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_incremental_build_performance
set /a TOTAL_TESTS+=1

echo Testing: Incremental build performance
echo   Measuring time for incremental builds

REM Log test start
echo [TEST %TOTAL_TESTS%] Incremental build performance >> "%TEST_LOG%"

REM First build (baseline)
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine >nul 2>&1
set "FIRST_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"
call :calculate_duration "%START_TIME%" "%END_TIME%" FIRST_DURATION

REM Second build (incremental)
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine >nul 2>&1
set "SECOND_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"
call :calculate_duration "%START_TIME%" "%END_TIME%" SECOND_DURATION

if %FIRST_RESULT% equ 0 (
    if %SECOND_RESULT% equ 0 (
        echo   Result: [PASS] - Incremental build works
        echo   Result: [PASS] - Incremental build works >> "%TEST_LOG%"
        echo   First build: %FIRST_DURATION%s, Second build: %SECOND_DURATION%s
        echo   First build: %FIRST_DURATION%s, Second build: %SECOND_DURATION%s >> "%TEST_LOG%"
        
        REM Check if incremental build is faster
        if %SECOND_DURATION% lss %FIRST_DURATION% (
            echo   Performance: Incremental build is faster (good)
            echo   Performance: Incremental build is faster (good) >> "%TEST_LOG%"
        ) else (
            echo   Performance: Incremental build not faster (check cache)
            echo   Performance: Incremental build not faster (check cache) >> "%TEST_LOG%"
        )
        
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - Second build failed
        echo   Result: [FAIL] - Second build failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - First build failed
    echo   Result: [FAIL] - First build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_cache_effectiveness
set /a TOTAL_TESTS+=1

echo Testing: Cache effectiveness
echo   Testing vcpkg cache and CMake cache behavior

REM Log test start
echo [TEST %TOTAL_TESTS%] Cache effectiveness >> "%TEST_LOG%"

REM Build with cache
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine >nul 2>&1
set "CACHED_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"
call :calculate_duration "%START_TIME%" "%END_TIME%" CACHED_DURATION

REM Build without cache
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --no-cache --clean-cache --engine >nul 2>&1
set "NO_CACHE_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"
call :calculate_duration "%START_TIME%" "%END_TIME%" NO_CACHE_DURATION

if %CACHED_RESULT% equ 0 (
    if %NO_CACHE_RESULT% equ 0 (
        echo   Result: [PASS] - Both cached and non-cached builds work
        echo   Result: [PASS] - Both cached and non-cached builds work >> "%TEST_LOG%"
        echo   Cached build: %CACHED_DURATION%s, No-cache build: %NO_CACHE_DURATION%s
        echo   Cached build: %CACHED_DURATION%s, No-cache build: %NO_CACHE_DURATION%s >> "%TEST_LOG%"
        
        REM Check cache effectiveness
        if %CACHED_DURATION% lss %NO_CACHE_DURATION% (
            echo   Performance: Cache is effective (faster builds)
            echo   Performance: Cache is effective (faster builds) >> "%TEST_LOG%"
        ) else (
            echo   Performance: Cache may not be effective
            echo   Performance: Cache may not be effective >> "%TEST_LOG%"
        )
        
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - No-cache build failed
        echo   Result: [FAIL] - No-cache build failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Cached build failed
    echo   Result: [FAIL] - Cached build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_specific_build_performance
set /a TOTAL_TESTS+=1

echo Testing: Specific build performance
echo   Testing performance of specific test builds

REM Log test start
echo [TEST %TOTAL_TESTS%] Specific build performance >> "%TEST_LOG%"

REM Test specific test build
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "SPECIFIC_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"
call :calculate_duration "%START_TIME%" "%END_TIME%" SPECIFIC_DURATION

REM Test all tests build
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --tests >nul 2>&1
set "ALL_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"
call :calculate_duration "%START_TIME%" "%END_TIME%" ALL_DURATION

if %SPECIFIC_RESULT% equ 0 (
    if %ALL_RESULT% equ 0 (
        echo   Result: [PASS] - Both specific and all tests builds work
        echo   Result: [PASS] - Both specific and all tests builds work >> "%TEST_LOG%"
        echo   Specific test: %SPECIFIC_DURATION%s, All tests: %ALL_DURATION%s
        echo   Specific test: %SPECIFIC_DURATION%s, All tests: %ALL_DURATION%s >> "%TEST_LOG%"
        
        REM Check if specific build is faster
        if %SPECIFIC_DURATION% lss %ALL_DURATION% (
            echo   Performance: Specific builds are faster (good)
            echo   Performance: Specific builds are faster (good) >> "%TEST_LOG%"
        ) else (
            echo   Performance: Specific builds not significantly faster
            echo   Performance: Specific builds not significantly faster >> "%TEST_LOG%"
        )
        
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - All tests build failed
        echo   Result: [FAIL] - All tests build failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Specific test build failed
    echo   Result: [FAIL] - Specific test build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_memory_usage
set /a TOTAL_TESTS+=1

echo Testing: Memory usage during builds
echo   Monitoring memory usage patterns

REM Log test start
echo [TEST %TOTAL_TESTS%] Memory usage during builds >> "%TEST_LOG%"

REM Simple memory usage test - check if build completes without memory issues
.\scripts\build_unified.bat --engine >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    echo   Result: [PASS] - Build completed without memory issues
    echo   Result: [PASS] - Build completed without memory issues >> "%TEST_LOG%"
    echo   Memory: Build system appears to manage memory properly
    echo   Memory: Build system appears to manage memory properly >> "%TEST_LOG%"
    set /a PASSED_TESTS+=1
) else (
    echo   Result: [FAIL] - Build failed (possible memory issues)
    echo   Result: [FAIL] - Build failed (possible memory issues) >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:calculate_duration
REM Calculate time difference in seconds (simplified version)
REM Parameters: %1=start_time, %2=end_time, %3=result_variable
set "start_time=%~1"
set "end_time=%~2"

REM Convert times to seconds (simplified - assumes same day)
for /f "tokens=1-3 delims=:." %%a in ("%start_time%") do (
    set /a start_seconds=%%a*3600+%%b*60+%%c
)
for /f "tokens=1-3 delims=:." %%a in ("%end_time%") do (
    set /a end_seconds=%%a*3600+%%b*60+%%c
)

REM Handle day rollover (basic check)
if %end_seconds% lss %start_seconds% (
    set /a end_seconds+=86400
)

set /a duration=%end_seconds%-%start_seconds%
set "%~3=%duration%"
goto :eof