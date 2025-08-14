@echo off
setlocal enabledelayedexpansion
REM Functional Build System Test Suite
REM Tests actual build functionality with real commands
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Functional Tests
echo ========================================

set "FAILED=0"
set "TEST_LOG=logs\build_functionality_test.log"

REM Create logs directory if it doesn't exist
if not exist "logs" mkdir logs

echo [%DATE% %TIME%] Starting build functionality tests > "%TEST_LOG%"
echo.

echo Test 1: Clean build environment...
if exist build (
    rmdir /s /q build 2>nul
    if exist build (
        echo [FAIL] Could not clean build directory
        set "FAILED=1"
        goto :end_tests
    )
)
echo [PASS] Build environment cleaned

echo.
echo Test 2: Basic engine build...
.\scripts\build_unified.bat --engine
if errorlevel 1 (
    echo [FAIL] Engine build failed
    echo [%DATE% %TIME%] FAIL: Engine build failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Engine build succeeded
    echo [%DATE% %TIME%] PASS: Engine build succeeded >> "%TEST_LOG%"
    
    REM Verify engine library was created
    if exist "build\Release\GameEngineKiro.lib" (
        echo [PASS] Engine library created (manual config)
    ) else if exist "build\vs\x64\Release\Release\GameEngineKiro.lib" (
        echo [PASS] Engine library created (VS preset)
    ) else if exist "build\ninja\x64\Release\GameEngineKiro.lib" (
        echo [PASS] Engine library created (Ninja preset)
    ) else (
        echo [FAIL] Engine library not found
        set "FAILED=1"
    )
)

echo.
echo Test 3: Test build...
.\scripts\build_unified.bat --tests
if errorlevel 1 (
    echo [FAIL] Test build failed
    echo [%DATE% %TIME%] FAIL: Test build failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Test build succeeded
    echo [%DATE% %TIME%] PASS: Test build succeeded >> "%TEST_LOG%"
    
    REM Verify some test executables were created
    set "TESTS_FOUND=0"
    if exist "build\Release\MathTest.exe" set "TESTS_FOUND=1"
    if exist "build\vs\x64\Release\Release\MathTest.exe" set "TESTS_FOUND=1"
    if exist "build\ninja\x64\Release\MathTest.exe" set "TESTS_FOUND=1"
    
    if "!TESTS_FOUND!"=="1" (
        echo [PASS] Test executables created
    ) else (
        echo [FAIL] Test executables not found
        set "FAILED=1"
    )
)

echo.
echo Test 4: Specific test build...
.\scripts\build_unified.bat --tests MathTest
if errorlevel 1 (
    echo [FAIL] Specific test build failed
    echo [%DATE% %TIME%] FAIL: Specific test build failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Specific test build succeeded
    echo [%DATE% %TIME%] PASS: Specific test build succeeded >> "%TEST_LOG%"
)

echo.
echo Test 5: Run tests...
.\scripts\run_tests.bat --unit
if errorlevel 1 (
    echo [FAIL] Running tests failed
    echo [%DATE% %TIME%] FAIL: Running tests failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Tests executed successfully
    echo [%DATE% %TIME%] PASS: Tests executed successfully >> "%TEST_LOG%"
)

echo.
echo Test 6: Incremental build (should be fast)...
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine
if errorlevel 1 (
    echo [FAIL] Incremental build failed
    echo [%DATE% %TIME%] FAIL: Incremental build failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Incremental build succeeded
    echo [%DATE% %TIME%] PASS: Incremental build succeeded >> "%TEST_LOG%"
)

echo.
echo Test 7: Clean cache functionality...
.\scripts\build_unified.bat --clean-cache
if errorlevel 1 (
    echo [FAIL] Clean cache failed
    echo [%DATE% %TIME%] FAIL: Clean cache failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Clean cache succeeded
    echo [%DATE% %TIME%] PASS: Clean cache succeeded >> "%TEST_LOG%"
)

:end_tests
echo.
echo ========================================
echo Build System Test Results
echo ========================================

if "%FAILED%"=="0" (
    echo [SUCCESS] All build system tests passed!
    echo The build system is working correctly.
    echo [%DATE% %TIME%] SUCCESS: All build system tests passed >> "%TEST_LOG%"
    exit /b 0
) else (
    echo [FAILED] Some build system tests failed!
    echo Check the build system configuration.
    echo [%DATE% %TIME%] FAILED: Some build system tests failed >> "%TEST_LOG%"
    exit /b 1
)