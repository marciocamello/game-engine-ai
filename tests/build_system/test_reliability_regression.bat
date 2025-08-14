@echo off
setlocal enabledelayedexpansion
REM Reliability Regression Test Suite
REM Tests for fixed reliability issues
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Reliability Regression Tests
echo ========================================

set "FAILED=0"
set "TEST_LOG=logs\reliability_regression_test.log"

REM Create logs directory if it doesn't exist
if not exist "logs" mkdir logs

echo [%DATE% %TIME%] Starting reliability regression tests > "%TEST_LOG%"
echo.

echo Test 1: Build failure recovery...
REM Create a temporary build failure marker to test recovery
if not exist build mkdir build
echo Build failed marker > build\.build_failed

.\scripts\build_unified.bat --engine
if errorlevel 1 (
    echo [FAIL] Build failure recovery failed
    echo [%DATE% %TIME%] FAIL: Build failure recovery failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Build failure recovery succeeded
    echo [%DATE% %TIME%] PASS: Build failure recovery succeeded >> "%TEST_LOG%"
    
    REM Verify failure marker was cleaned up
    if not exist "build\.build_failed" (
        echo [PASS] Build failure marker cleaned up
    ) else (
        echo [FAIL] Build failure marker not cleaned up
        set "FAILED=1"
    )
)

echo.
echo Test 2: CMake cache conflict resolution...
REM Test cache conflict detection and resolution
if exist "build\CMakeCache.txt" (
    REM Modify cache to simulate conflict
    echo CMAKE_BUILD_TYPE:STRING=Debug >> "build\CMakeCache.txt"
    
    .\scripts\build_unified.bat --release --engine
    if errorlevel 1 (
        echo [FAIL] Cache conflict resolution failed
        echo [%DATE% %TIME%] FAIL: Cache conflict resolution failed >> "%TEST_LOG%"
        set "FAILED=1"
    ) else (
        echo [PASS] Cache conflict resolution succeeded
        echo [%DATE% %TIME%] PASS: Cache conflict resolution succeeded >> "%TEST_LOG%"
    )
) else (
    echo [SKIP] No CMake cache to test conflict resolution
)

echo.
echo Test 3: Consecutive identical builds...
REM Test consecutive identical build detection
.\scripts\build_unified.bat --engine
if errorlevel 1 (
    echo [FAIL] First identical build failed
    set "FAILED=1"
) else (
    echo [PASS] First identical build succeeded
    
    REM Run same build again
    .\scripts\build_unified.bat --engine
    if errorlevel 1 (
        echo [FAIL] Second identical build failed
        set "FAILED=1"
    ) else (
        echo [PASS] Second identical build succeeded (should be fast)
    )
)

echo.
echo Test 4: Build state consistency...
REM Test build state tracking
.\scripts\build_unified.bat --tests MathTest
if errorlevel 1 (
    echo [FAIL] Build state consistency test failed
    echo [%DATE% %TIME%] FAIL: Build state consistency test failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] Build state consistency test succeeded
    echo [%DATE% %TIME%] PASS: Build state consistency test succeeded >> "%TEST_LOG%"
    
    REM Verify build signature was saved
    if exist "build\.last_build_signature" (
        echo [PASS] Build signature tracking works
    ) else (
        echo [FAIL] Build signature not saved
        set "FAILED=1"
    )
)

echo.
echo Test 5: Environment variable handling...
REM Test IDE environment detection
set "KIRO_IDE_SESSION=1"
.\scripts\build_unified.bat --engine
set "result1=!errorlevel!"
set "KIRO_IDE_SESSION="

if !result1! equ 0 (
    echo [PASS] IDE environment handling works
    echo [%DATE% %TIME%] PASS: IDE environment handling works >> "%TEST_LOG%"
) else (
    echo [FAIL] IDE environment handling failed
    echo [%DATE% %TIME%] FAIL: IDE environment handling failed >> "%TEST_LOG%"
    set "FAILED=1"
)

echo.
echo Test 6: vcpkg cache fallback...
REM Test cache fallback mechanism
.\scripts\build_unified.bat --no-cache --engine
if errorlevel 1 (
    echo [FAIL] vcpkg cache fallback failed
    echo [%DATE% %TIME%] FAIL: vcpkg cache fallback failed >> "%TEST_LOG%"
    set "FAILED=1"
) else (
    echo [PASS] vcpkg cache fallback succeeded
    echo [%DATE% %TIME%] PASS: vcpkg cache fallback succeeded >> "%TEST_LOG%"
)

echo.
echo Test 7: Build verification...
REM Test build result verification
.\scripts\build_unified.bat --tests MathTest
if errorlevel 1 (
    echo [FAIL] Build verification test failed
    set "FAILED=1"
) else (
    echo [PASS] Build verification test succeeded
    
    REM Verify expected executable exists
    set "EXE_FOUND=0"
    if exist "build\Release\MathTest.exe" set "EXE_FOUND=1"
    if exist "build\vs\x64\Release\Release\MathTest.exe" set "EXE_FOUND=1"
    if exist "build\ninja\x64\Release\MathTest.exe" set "EXE_FOUND=1"
    
    if "!EXE_FOUND!"=="1" (
        echo [PASS] Build verification found expected executable
    ) else (
        echo [FAIL] Build verification - executable not found
        set "FAILED=1"
    )
)

echo.
echo ========================================
echo Reliability Regression Test Results
echo ========================================

if "%FAILED%"=="0" (
    echo [SUCCESS] All reliability regression tests passed!
    echo No regressions detected in reliability fixes.
    echo [%DATE% %TIME%] SUCCESS: All reliability regression tests passed >> "%TEST_LOG%"
    exit /b 0
) else (
    echo [FAILED] Some reliability regression tests failed!
    echo Reliability issues may have been reintroduced.
    echo [%DATE% %TIME%] FAILED: Some reliability regression tests failed >> "%TEST_LOG%"
    exit /b 1
)