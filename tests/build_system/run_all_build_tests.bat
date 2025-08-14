@echo off
setlocal enabledelayedexpansion
REM Run all build system tests
REM Requirements: 6.1, 6.2, 6.3

echo ========================================
echo Game Engine Kiro - Build System Test Suite
echo ========================================

set "TOTAL_SUITES=0"
set "PASSED_SUITES=0"
set "FAILED_SUITES=0"
set "MASTER_LOG=tests\build_system\master_test_results.log"

REM Initialize master log
echo Build System Test Suite - %DATE% %TIME% > "%MASTER_LOG%"
echo. >> "%MASTER_LOG%"

echo Running comprehensive build system test suite...
echo.

REM Create logs directory if it doesn't exist
if not exist "tests\build_system" mkdir "tests\build_system"

REM Test Suite 1: Backward Compatibility
call :run_test_suite "tests\build_system\test_backward_compatibility.bat" "Backward Compatibility"

REM Test Suite 2: Build Functionality
call :run_test_suite "tests\build_system\test_build_functionality.bat" "Build Functionality"

REM Test Suite 3: Clean Builds
call :run_test_suite "tests\build_system\test_clean_builds.bat" "Clean Builds"

REM Test Suite 4: IDE Integration
call :run_test_suite "tests\build_system\test_ide_integration.bat" "IDE Integration"

REM Test Suite 5: Performance Benchmarks (if exists)
if exist "tests\build_system\test_performance_benchmarks.bat" (
    call :run_test_suite "tests\build_system\test_performance_benchmarks.bat" "Performance Benchmarks"
)

REM Test Suite 6: Reliability Regression (if exists)
if exist "tests\build_system\test_reliability_regression.bat" (
    call :run_test_suite "tests\build_system\test_reliability_regression.bat" "Reliability Regression"
)

echo.
echo ========================================
echo Master Test Results Summary
echo ========================================
echo Total Test Suites: %TOTAL_SUITES%
echo Passed: %PASSED_SUITES%
echo Failed: %FAILED_SUITES%

REM Generate detailed report
call :generate_detailed_report

if %FAILED_SUITES% gtr 0 (
    echo.
    echo [FAILED] Some build system test suites failed!
    echo Check %MASTER_LOG% and individual test logs for details.
    echo.
    echo Failed test suites:
    if exist "tests\build_system\failed_suites.tmp" (
        type "tests\build_system\failed_suites.tmp"
        del "tests\build_system\failed_suites.tmp"
    )
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All build system test suites passed!
    echo The build system maintains full backward compatibility.
    exit /b 0
)

:run_test_suite
set "TEST_SCRIPT=%~1"
set "SUITE_NAME=%~2"
set /a TOTAL_SUITES+=1

echo.
echo ----------------------------------------
echo Running: %SUITE_NAME% Tests
echo ----------------------------------------

REM Log suite start
echo [SUITE %TOTAL_SUITES%] %SUITE_NAME% Tests >> "%MASTER_LOG%"
echo   Script: %TEST_SCRIPT% >> "%MASTER_LOG%"
echo   Started: %DATE% %TIME% >> "%MASTER_LOG%"

REM Run the test suite
call "%TEST_SCRIPT%"
set "RESULT=%ERRORLEVEL%"

REM Log suite result
echo   Completed: %DATE% %TIME% >> "%MASTER_LOG%"

if %RESULT% equ 0 (
    echo [PASS] %SUITE_NAME% Tests - All tests passed
    echo   Result: [PASS] - All tests passed >> "%MASTER_LOG%"
    set /a PASSED_SUITES+=1
) else (
    echo [FAIL] %SUITE_NAME% Tests - Some tests failed
    echo   Result: [FAIL] - Some tests failed >> "%MASTER_LOG%"
    echo %SUITE_NAME% >> "tests\build_system\failed_suites.tmp"
    set /a FAILED_SUITES+=1
)

echo. >> "%MASTER_LOG%"
goto :eof

:generate_detailed_report
echo.
echo Generating detailed test report...

REM Create summary report
echo ======================================== > "tests\build_system\test_summary.txt"
echo Build System Test Suite Summary >> "tests\build_system\test_summary.txt"
echo ======================================== >> "tests\build_system\test_summary.txt"
echo. >> "tests\build_system\test_summary.txt"
echo Test Date: %DATE% %TIME% >> "tests\build_system\test_summary.txt"
echo Total Test Suites: %TOTAL_SUITES% >> "tests\build_system\test_summary.txt"
echo Passed Suites: %PASSED_SUITES% >> "tests\build_system\test_summary.txt"
echo Failed Suites: %FAILED_SUITES% >> "tests\build_system\test_summary.txt"
echo. >> "tests\build_system\test_summary.txt"

if %FAILED_SUITES% gtr 0 (
    echo FAILED SUITES: >> "tests\build_system\test_summary.txt"
    if exist "tests\build_system\failed_suites.tmp" (
        type "tests\build_system\failed_suites.tmp" >> "tests\build_system\test_summary.txt"
    )
    echo. >> "tests\build_system\test_summary.txt"
)

echo INDIVIDUAL TEST LOGS: >> "tests\build_system\test_summary.txt"
echo - Backward Compatibility: tests\build_system\compatibility_test.log >> "tests\build_system\test_summary.txt"
echo - Build Functionality: tests\build_system\functionality_test.log >> "tests\build_system\test_summary.txt"
echo - Clean Builds: tests\build_system\clean_builds_test.log >> "tests\build_system\test_summary.txt"
echo - IDE Integration: tests\build_system\ide_integration_test.log >> "tests\build_system\test_summary.txt"
echo - Master Log: tests\build_system\master_test_results.log >> "tests\build_system\test_summary.txt"

echo.
echo Detailed report saved to: tests\build_system\test_summary.txt
goto :eof