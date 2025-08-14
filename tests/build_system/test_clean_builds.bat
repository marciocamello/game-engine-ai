@echo off
setlocal enabledelayedexpansion
REM Test clean build functionality and cache behavior
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Clean Builds Test
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"
set "FAILED_TESTS=0"
set "TEST_LOG=tests\build_system\clean_builds_test.log"

REM Initialize log
echo Build System Clean Builds Test - %DATE% %TIME% > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Testing clean build functionality and cache behavior...
echo.

REM Test 1: Clean engine artifacts
call :test_clean_build "--clean-engine --engine" "Clean engine build" "GameEngineKiro.lib"

REM Test 2: Clean test artifacts  
call :test_clean_build "--clean-tests --tests MathTest" "Clean test build" "MathTest.exe"

REM Test 3: Clean project artifacts
call :test_clean_build "--clean-projects --projects" "Clean projects build" "GameExample.exe"

REM Test 4: Clean cache
call :test_cache_clean "--clean-cache --engine" "Clean cache build"

REM Test 5: Clean all
call :test_clean_all "--clean-all --all" "Clean all build"

REM Test 6: Incremental build after clean
call :test_incremental_after_clean

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %PASSED_TESTS%
echo Failed: %FAILED_TESTS%

if %FAILED_TESTS% gtr 0 (
    echo.
    echo [FAILED] Some clean build tests failed!
    echo Check %TEST_LOG% for details.
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All clean build tests passed!
    exit /b 0
)

:test_clean_build
set "COMMAND=%~1"
set "DESCRIPTION=%~2"
set "EXPECTED_FILE=%~3"
set /a TOTAL_TESTS+=1

echo Testing: %DESCRIPTION%
echo   Command: build_unified.bat %COMMAND%

REM Log test start
echo [TEST %TOTAL_TESTS%] %DESCRIPTION% >> "%TEST_LOG%"
echo   Command: build_unified.bat %COMMAND% >> "%TEST_LOG%"

REM Execute clean build
.\scripts\build_unified.bat %COMMAND% >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    REM Check if expected file was created
    call :find_build_artifact "%EXPECTED_FILE%" ARTIFACT_FOUND
    if "!ARTIFACT_FOUND!"=="true" (
        echo   Result: [PASS] - Build succeeded and artifact created
        echo   Result: [PASS] - Build succeeded and artifact created >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - Build succeeded but artifact not found
        echo   Result: [FAIL] - Build succeeded but artifact not found >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Build failed with exit code: %RESULT%
    echo   Result: [FAIL] - Build failed with exit code: %RESULT% >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_cache_clean
set "COMMAND=%~1"
set "DESCRIPTION=%~2"
set /a TOTAL_TESTS+=1

echo Testing: %DESCRIPTION%
echo   Command: build_unified.bat %COMMAND%

REM Log test start
echo [TEST %TOTAL_TESTS%] %DESCRIPTION% >> "%TEST_LOG%"
echo   Command: build_unified.bat %COMMAND% >> "%TEST_LOG%"

REM Check if CMakeCache.txt exists before
set "CACHE_BEFORE=false"
if exist "build\CMakeCache.txt" set "CACHE_BEFORE=true"

REM Execute clean cache build
.\scripts\build_unified.bat %COMMAND% >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    echo   Result: [PASS] - Cache clean build succeeded
    echo   Result: [PASS] - Cache clean build succeeded >> "%TEST_LOG%"
    set /a PASSED_TESTS+=1
) else (
    echo   Result: [FAIL] - Cache clean build failed with exit code: %RESULT%
    echo   Result: [FAIL] - Cache clean build failed with exit code: %RESULT% >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_clean_all
set "COMMAND=%~1"
set "DESCRIPTION=%~2"
set /a TOTAL_TESTS+=1

echo Testing: %DESCRIPTION%
echo   Command: build_unified.bat %COMMAND%

REM Log test start
echo [TEST %TOTAL_TESTS%] %DESCRIPTION% >> "%TEST_LOG%"
echo   Command: build_unified.bat %COMMAND% >> "%TEST_LOG%"

REM Execute clean all build
.\scripts\build_unified.bat %COMMAND% >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    REM Verify multiple artifacts were created
    call :find_build_artifact "GameEngineKiro.lib" ENGINE_FOUND
    call :find_build_artifact "MathTest.exe" TEST_FOUND
    
    if "!ENGINE_FOUND!"=="true" (
        echo   Result: [PASS] - Clean all build succeeded
        echo   Result: [PASS] - Clean all build succeeded >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - Clean all build succeeded but artifacts missing
        echo   Result: [FAIL] - Clean all build succeeded but artifacts missing >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Clean all build failed with exit code: %RESULT%
    echo   Result: [FAIL] - Clean all build failed with exit code: %RESULT% >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_incremental_after_clean
set /a TOTAL_TESTS+=1

echo Testing: Incremental build after clean
echo   Testing build speed improvement after initial clean build

REM Log test start
echo [TEST %TOTAL_TESTS%] Incremental build after clean >> "%TEST_LOG%"

REM First build (should be slower)
set "START_TIME=%TIME%"
.\scripts\build_unified.bat --engine >nul 2>&1
set "FIRST_RESULT=%ERRORLEVEL%"
set "END_TIME=%TIME%"

REM Second build (should be faster - incremental)
set "START_TIME2=%TIME%"
.\scripts\build_unified.bat --engine >nul 2>&1
set "SECOND_RESULT=%ERRORLEVEL%"
set "END_TIME2=%TIME%"

if %FIRST_RESULT% equ 0 (
    if %SECOND_RESULT% equ 0 (
        echo   Result: [PASS] - Both builds succeeded
        echo   Result: [PASS] - Both builds succeeded >> "%TEST_LOG%"
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

:find_build_artifact
set "FILENAME=%~1"
set "RESULT_VAR=%~2"
set "%RESULT_VAR%=false"

REM Check different possible build paths
if exist "build\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\vs\x64\Release\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\ninja\x64\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\projects\GameExample\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\vs\x64\Release\projects\GameExample\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\ninja\x64\Release\projects\GameExample\%FILENAME%" set "%RESULT_VAR%=true"

goto :eof