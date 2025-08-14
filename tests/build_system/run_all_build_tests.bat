@echo off
setlocal enabledelayedexpansion
REM Master Build System Test Runner
REM Executes all build system tests in sequence
REM Requirements: 1.1, 1.2, 1.3, 1.4, 6.1, 6.2, 6.3

echo ========================================
echo Build System Comprehensive Test Suite
echo ========================================

set "TOTAL_FAILED=0"
set "TOTAL_TESTS=0"
set "MASTER_LOG=logs\build_system_test_results.log"

REM Create logs directory if it doesn't exist
if not exist "logs" mkdir logs

echo [%DATE% %TIME%] Starting comprehensive build system tests > "%MASTER_LOG%"
echo. >> "%MASTER_LOG%"

echo Running comprehensive build system validation...
echo.

REM Test 1: Build Functionality Tests
echo ========================================
echo Running Build Functionality Tests...
echo ========================================
set /a TOTAL_TESTS+=1
call .\tests\build_system\test_build_functionality.bat
if errorlevel 1 (
    echo [FAILED] Build Functionality Tests
    echo [%DATE% %TIME%] FAILED: Build Functionality Tests >> "%MASTER_LOG%"
    set /a TOTAL_FAILED+=1
) else (
    echo [PASSED] Build Functionality Tests
    echo [%DATE% %TIME%] PASSED: Build Functionality Tests >> "%MASTER_LOG%"
)

echo.
echo ========================================
echo Running Reliability Regression Tests...
echo ========================================
set /a TOTAL_TESTS+=1
call .\tests\build_system\test_reliability_regression.bat
if errorlevel 1 (
    echo [FAILED] Reliability Regression Tests
    echo [%DATE% %TIME%] FAILED: Reliability Regression Tests >> "%MASTER_LOG%"
    set /a TOTAL_FAILED+=1
) else (
    echo [PASSED] Reliability Regression Tests
    echo [%DATE% %TIME%] PASSED: Reliability Regression Tests >> "%MASTER_LOG%"
)

echo.
echo ========================================
echo Running Performance Benchmark Tests...
echo ========================================
set /a TOTAL_TESTS+=1
call .\tests\build_system\test_performance_benchmarks.bat
if errorlevel 1 (
    echo [FAILED] Performance Benchmark Tests
    echo [%DATE% %TIME%] FAILED: Performance Benchmark Tests >> "%MASTER_LOG%"
    set /a TOTAL_FAILED+=1
) else (
    echo [PASSED] Performance Benchmark Tests
    echo [%DATE% %TIME%] PASSED: Performance Benchmark Tests >> "%MASTER_LOG%"
)

echo.
echo ========================================
echo Running IDE Integration Tests...
echo ========================================
set /a TOTAL_TESTS+=1
call .\tests\build_system\test_ide_integration.bat
if errorlevel 1 (
    echo [FAILED] IDE Integration Tests
    echo [%DATE% %TIME%] FAILED: IDE Integration Tests >> "%MASTER_LOG%"
    set /a TOTAL_FAILED+=1
) else (
    echo [PASSED] IDE Integration Tests
    echo [%DATE% %TIME%] PASSED: IDE Integration Tests >> "%MASTER_LOG%"
)

REM Final Results
echo.
echo ========================================
echo Build System Test Suite Results
echo ========================================
echo Total Test Suites: %TOTAL_TESTS%
echo Passed: %TOTAL_PASSED%
echo Failed: %TOTAL_FAILED%

set /a TOTAL_PASSED=%TOTAL_TESTS%-%TOTAL_FAILED%

if %TOTAL_FAILED% equ 0 (
    echo.
    echo [SUCCESS] All build system test suites passed!
    echo The build system is fully validated and working correctly.
    echo.
    echo Test Coverage:
    echo - Build functionality validation
    echo - Reliability regression testing
    echo - Performance benchmarking
    echo - IDE integration compatibility
    echo - Backward compatibility verification
    echo.
    echo [%DATE% %TIME%] SUCCESS: All %TOTAL_TESTS% build system test suites passed >> "%MASTER_LOG%"
    exit /b 0
) else (
    echo.
    echo [FAILED] %TOTAL_FAILED% out of %TOTAL_TESTS% test suites failed!
    echo Build system validation incomplete.
    echo Check individual test logs for details.
    echo.
    echo [%DATE% %TIME%] FAILED: %TOTAL_FAILED% out of %TOTAL_TESTS% test suites failed >> "%MASTER_LOG%"
    exit /b 1
)