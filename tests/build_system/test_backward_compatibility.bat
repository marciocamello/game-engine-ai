@echo off
setlocal enabledelayedexpansion
REM Test backward compatibility for all existing build_unified.bat commands
REM Requirements: 6.1, 6.2, 6.3

echo ========================================
echo Build System Backward Compatibility Test
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"
set "FAILED_TESTS=0"
set "TEST_LOG=tests\build_system\compatibility_test.log"

REM Initialize log
echo Build System Backward Compatibility Test - %DATE% %TIME% > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Testing all existing build_unified.bat command combinations...
echo.

REM Test 1: Help command
call :test_command "--help" "Help command should work"

REM Test 2: Basic build commands
call :test_command "--engine" "Engine-only build should work"
call :test_command "--projects" "Projects-only build should work" 
call :test_command "--tests" "Tests-only build should work"

REM Test 3: Specific builds
call :test_command "--tests MathTest" "Specific test build should work"
call :test_command "--project GameExample" "Specific project build should work"

REM Test 4: Build type options
call :test_command "--debug --engine" "Debug engine build should work"
call :test_command "--release --engine" "Release engine build should work"

REM Test 5: Combined options
call :test_command "--engine --tests" "Engine and tests build should work"
call :test_command "--engine --projects" "Engine and projects build should work"

REM Test 6: Legacy compatibility
call :test_command "--engine-only" "Legacy engine-only should work"
call :test_command "--projects-only" "Legacy projects-only should work"
call :test_command "--tests-only" "Legacy tests-only should work"

REM Test 7: Advanced options
call :test_command "--coverage" "Coverage build should work"
call :test_command "--no-cache --engine" "No-cache build should work"

REM Test 8: Cleaning options
call :test_command "--clean-engine --engine" "Clean engine build should work"
call :test_command "--clean-tests --tests" "Clean tests build should work"

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %PASSED_TESTS%
echo Failed: %FAILED_TESTS%

if %FAILED_TESTS% gtr 0 (
    echo.
    echo [FAILED] Some backward compatibility tests failed!
    echo Check %TEST_LOG% for details.
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All backward compatibility tests passed!
    exit /b 0
)

:test_command
set "COMMAND=%~1"
set "DESCRIPTION=%~2"
set /a TOTAL_TESTS+=1

echo Testing: %DESCRIPTION%
echo   Command: build_unified.bat %COMMAND%

REM Log test start
echo [TEST %TOTAL_TESTS%] %DESCRIPTION% >> "%TEST_LOG%"
echo   Command: build_unified.bat %COMMAND% >> "%TEST_LOG%"

REM Execute command and capture result
.\scripts\build_unified.bat %COMMAND% >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    echo   Result: [PASS]
    echo   Result: [PASS] >> "%TEST_LOG%"
    set /a PASSED_TESTS+=1
) else (
    echo   Result: [FAIL] - Exit code: %RESULT%
    echo   Result: [FAIL] - Exit code: %RESULT% >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof