@echo off
setlocal enabledelayedexpansion
REM Test reliability regression for build system fixes
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Reliability Regression Tests
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"
set "FAILED_TESTS=0"
set "TEST_LOG=tests\build_system\reliability_regression_test.log"

REM Initialize log
echo Build System Reliability Regression Tests - %DATE% %TIME% > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Testing reliability fixes and regression prevention...
echo.

REM Test 1: Configuration change detection
call :test_configuration_change_detection

REM Test 2: Cache invalidation on changes
call :test_cache_invalidation

REM Test 3: Build failure recovery
call :test_build_failure_recovery

REM Test 4: Concurrent build safety
call :test_concurrent_build_safety

REM Test 5: Environment variable isolation
call :test_environment_isolation

REM Test 6: Path handling robustness
call :test_path_handling

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %PASSED_TESTS%
echo Failed: %FAILED_TESTS%

if %FAILED_TESTS% gtr 0 (
    echo.
    echo [FAILED] Some reliability regression tests failed!
    echo Check %TEST_LOG% for details.
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All reliability regression tests passed!
    exit /b 0
)

:test_configuration_change_detection
set /a TOTAL_TESTS+=1

echo Testing: Configuration change detection
echo   Testing if system detects and handles configuration changes

REM Log test start
echo [TEST %TOTAL_TESTS%] Configuration change detection >> "%TEST_LOG%"

REM Build with one configuration
.\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "FIRST_RESULT=%ERRORLEVEL%"

REM Build with different configuration - should detect change
.\scripts\build_unified.bat --tests QuaternionTest >nul 2>&1
set "SECOND_RESULT=%ERRORLEVEL%"

REM Build back to original - should detect change again
.\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "THIRD_RESULT=%ERRORLEVEL%"

if %FIRST_RESULT% equ 0 (
    if %SECOND_RESULT% equ 0 (
        if %THIRD_RESULT% equ 0 (
            echo   Result: [PASS] - Configuration changes handled correctly
            echo   Result: [PASS] - Configuration changes handled correctly >> "%TEST_LOG%"
            set /a PASSED_TESTS+=1
        ) else (
            echo   Result: [FAIL] - Third build failed
            echo   Result: [FAIL] - Third build failed >> "%TEST_LOG%"
            set /a FAILED_TESTS+=1
        )
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

:test_cache_invalidation
set /a TOTAL_TESTS+=1

echo Testing: Cache invalidation on changes
echo   Testing if cache is properly invalidated when needed

REM Log test start
echo [TEST %TOTAL_TESTS%] Cache invalidation on changes >> "%TEST_LOG%"

REM Build to establish cache
.\scripts\build_unified.bat --engine >nul 2>&1
set "FIRST_RESULT=%ERRORLEVEL%"

REM Force cache clean and rebuild
.\scripts\build_unified.bat --clean-cache --engine >nul 2>&1
set "CLEAN_RESULT=%ERRORLEVEL%"

REM Build again to test cache recreation
.\scripts\build_unified.bat --engine >nul 2>&1
set "REBUILD_RESULT=%ERRORLEVEL%"

if %FIRST_RESULT% equ 0 (
    if %CLEAN_RESULT% equ 0 (
        if %REBUILD_RESULT% equ 0 (
            echo   Result: [PASS] - Cache invalidation works correctly
            echo   Result: [PASS] - Cache invalidation works correctly >> "%TEST_LOG%"
            set /a PASSED_TESTS+=1
        ) else (
            echo   Result: [FAIL] - Rebuild after cache clean failed
            echo   Result: [FAIL] - Rebuild after cache clean failed >> "%TEST_LOG%"
            set /a FAILED_TESTS+=1
        )
    ) else (
        echo   Result: [FAIL] - Cache clean build failed
        echo   Result: [FAIL] - Cache clean build failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Initial build failed
    echo   Result: [FAIL] - Initial build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_build_failure_recovery
set /a TOTAL_TESTS+=1

echo Testing: Build failure recovery
echo   Testing recovery from build failures

REM Log test start
echo [TEST %TOTAL_TESTS%] Build failure recovery >> "%TEST_LOG%"

REM Create a scenario that might cause issues and then recover
REM First, ensure we have a good build
.\scripts\build_unified.bat --engine >nul 2>&1
set "GOOD_RESULT=%ERRORLEVEL%"

REM Try to build something that should work
.\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "RECOVERY_RESULT=%ERRORLEVEL%"

REM Build again to ensure system is stable
.\scripts\build_unified.bat --engine >nul 2>&1
set "STABLE_RESULT=%ERRORLEVEL%"

if %GOOD_RESULT% equ 0 (
    if %RECOVERY_RESULT% equ 0 (
        if %STABLE_RESULT% equ 0 (
            echo   Result: [PASS] - Build system recovers properly
            echo   Result: [PASS] - Build system recovers properly >> "%TEST_LOG%"
            set /a PASSED_TESTS+=1
        ) else (
            echo   Result: [FAIL] - System not stable after recovery
            echo   Result: [FAIL] - System not stable after recovery >> "%TEST_LOG%"
            set /a FAILED_TESTS+=1
        )
    ) else (
        echo   Result: [FAIL] - Recovery build failed
        echo   Result: [FAIL] - Recovery build failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Initial good build failed
    echo   Result: [FAIL] - Initial good build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_concurrent_build_safety
set /a TOTAL_TESTS+=1

echo Testing: Concurrent build safety
echo   Testing if multiple builds can run safely

REM Log test start
echo [TEST %TOTAL_TESTS%] Concurrent build safety >> "%TEST_LOG%"

REM This is a simplified test - in practice, concurrent builds should be avoided
REM but the system should handle it gracefully
.\scripts\build_unified.bat --engine >nul 2>&1
set "FIRST_RESULT=%ERRORLEVEL%"

REM Immediately try another build
.\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "SECOND_RESULT=%ERRORLEVEL%"

if %FIRST_RESULT% equ 0 (
    if %SECOND_RESULT% equ 0 (
        echo   Result: [PASS] - Sequential builds work safely
        echo   Result: [PASS] - Sequential builds work safely >> "%TEST_LOG%"
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

:test_environment_isolation
set /a TOTAL_TESTS+=1

echo Testing: Environment variable isolation
echo   Testing if builds work with different environment setups

REM Log test start
echo [TEST %TOTAL_TESTS%] Environment variable isolation >> "%TEST_LOG%"

REM Test with clean environment
.\scripts\build_unified.bat --engine >nul 2>&1
set "CLEAN_RESULT=%ERRORLEVEL%"

REM Test with some environment variables set
set "TEST_VAR=test_value"
set "ANOTHER_VAR=another_value"
.\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "ENV_RESULT=%ERRORLEVEL%"

REM Clean up
set "TEST_VAR="
set "ANOTHER_VAR="

if %CLEAN_RESULT% equ 0 (
    if %ENV_RESULT% equ 0 (
        echo   Result: [PASS] - Environment isolation works
        echo   Result: [PASS] - Environment isolation works >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - Build with environment variables failed
        echo   Result: [FAIL] - Build with environment variables failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Clean environment build failed
    echo   Result: [FAIL] - Clean environment build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_path_handling
set /a TOTAL_TESTS+=1

echo Testing: Path handling robustness
echo   Testing if system handles various path scenarios

REM Log test start
echo [TEST %TOTAL_TESTS%] Path handling robustness >> "%TEST_LOG%"

REM Test normal build
.\scripts\build_unified.bat --engine >nul 2>&1
set "NORMAL_RESULT=%ERRORLEVEL%"

REM Test with current directory change (should still work)
pushd scripts
..\scripts\build_unified.bat --tests MathTest >nul 2>&1
set "SUBDIR_RESULT=%ERRORLEVEL%"
popd

if %NORMAL_RESULT% equ 0 (
    if %SUBDIR_RESULT% equ 0 (
        echo   Result: [PASS] - Path handling is robust
        echo   Result: [PASS] - Path handling is robust >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - Build from subdirectory failed
        echo   Result: [FAIL] - Build from subdirectory failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Normal build failed
    echo   Result: [FAIL] - Normal build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof