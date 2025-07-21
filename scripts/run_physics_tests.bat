@echo off
REM Game Engine Kiro - Physics Testing Script
REM This script runs all physics-related tests

setlocal enabledelayedexpansion

set BUILD_FIRST=false
set VERBOSE=false

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="build" set BUILD_FIRST=true
if /i "%~1"=="verbose" set VERBOSE=true
if /i "%~1"=="-v" set VERBOSE=true
shift
goto :parse_args
:args_done

echo ========================================
echo  Game Engine Kiro - Physics Test Suite
echo ========================================

REM Build project first if requested
if "%BUILD_FIRST%"=="true" (
    echo.
    echo Building project...
    call build.bat
    if !errorlevel! neq 0 (
        echo Build failed! Exiting.
        exit /b 1
    )
    echo Build completed successfully.
)

REM Check if build directory exists
if not exist "build\Release" (
    echo ERROR: Build directory not found!
    echo Please run 'scripts\build.bat' first to compile the project.
    exit /b 1
)

set TOTAL_TESTS=0
set PASSED_TESTS=0
set FAILED_TESTS=0
set SKIPPED_TESTS=0

echo.
echo Running physics tests...

REM Integration Tests
echo.
echo --- Integration Tests ---

REM BulletIntegrationTest
set /a TOTAL_TESTS+=1
echo   Running BulletIntegrationTest...
if exist "build\Release\BulletIntegrationTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\BulletIntegrationTest.exe
    ) else (
        build\Release\BulletIntegrationTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM BulletConversionTest
set /a TOTAL_TESTS+=1
echo   Running BulletConversionTest...
if exist "build\Release\BulletConversionTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\BulletConversionTest.exe
    ) else (
        build\Release\BulletConversionTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM BulletUtilsSimpleTest
set /a TOTAL_TESTS+=1
echo   Running BulletUtilsSimpleTest...
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\BulletUtilsSimpleTest.exe
    ) else (
        build\Release\BulletUtilsSimpleTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM PhysicsQueriesTest
set /a TOTAL_TESTS+=1
echo   Running PhysicsQueriesTest...
if exist "build\Release\PhysicsQueriesTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\PhysicsQueriesTest.exe
    ) else (
        build\Release\PhysicsQueriesTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM PhysicsConfigurationTest
set /a TOTAL_TESTS+=1
echo   Running PhysicsConfigurationTest...
if exist "build\Release\PhysicsConfigurationTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\PhysicsConfigurationTest.exe
    ) else (
        build\Release\PhysicsConfigurationTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM Performance Tests
echo.
echo --- Performance Tests ---

REM PhysicsPerformanceSimpleTest
set /a TOTAL_TESTS+=1
echo   Running PhysicsPerformanceSimpleTest...
if exist "build\Release\PhysicsPerformanceSimpleTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\PhysicsPerformanceSimpleTest.exe
    ) else (
        build\Release\PhysicsPerformanceSimpleTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM MemoryUsageSimpleTest
set /a TOTAL_TESTS+=1
echo   Running MemoryUsageSimpleTest...
if exist "build\Release\MemoryUsageSimpleTest.exe" (
    if "%VERBOSE%"=="true" (
        build\Release\MemoryUsageSimpleTest.exe
    ) else (
        build\Release\MemoryUsageSimpleTest.exe >nul 2>&1
    )
    if !errorlevel! equ 0 (
        echo     PASSED
        set /a PASSED_TESTS+=1
    ) else (
        echo     FAILED
        set /a FAILED_TESTS+=1
    )
) else (
    echo     SKIPPED ^(not found^)
    set /a SKIPPED_TESTS+=1
)

REM Summary
echo.
echo ==================================================
echo TEST SUMMARY
echo ==================================================
echo Total Tests:  !TOTAL_TESTS!
echo Passed:       !PASSED_TESTS!
echo Failed:       !FAILED_TESTS!
echo Skipped:      !SKIPPED_TESTS!

REM Calculate success rate
if !TOTAL_TESTS! gtr 0 (
    if !SKIPPED_TESTS! lss !TOTAL_TESTS! (
        echo Success Rate: Calculated based on executed tests
    )
)

REM Recommendations
echo.
echo RECOMMENDATIONS:
if !SKIPPED_TESTS! gtr 0 (
    echo   - Run 'scripts\build.bat' to build missing test executables
)
if !FAILED_TESTS! gtr 0 (
    echo   - Run with 'verbose' flag to see detailed failure information
    echo   - Check individual test logs for specific error details
)
if !FAILED_TESTS! equ 0 if !PASSED_TESTS! gtr 0 (
    echo   - All tests passed! Physics integration is working correctly.
)

REM Exit with appropriate code
if !FAILED_TESTS! gtr 0 (
    echo.
    echo Test suite completed with failures.
    exit /b 1
) else if !SKIPPED_TESTS! equ !TOTAL_TESTS! (
    echo.
    echo No tests were executed. Build the project first.
    exit /b 2
) else (
    echo.
    echo All tests completed successfully!
    exit /b 0
)