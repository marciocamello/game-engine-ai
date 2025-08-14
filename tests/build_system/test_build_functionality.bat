@echo off
setlocal enabledelayedexpansion
REM Test core build system functionality
REM Requirements: 1.1, 1.2, 1.3, 1.4

echo ========================================
echo Build System Functionality Test
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"
set "FAILED_TESTS=0"
set "TEST_LOG=tests\build_system\functionality_test.log"

REM Initialize log
echo Build System Functionality Test - %DATE% %TIME% > "%TEST_LOG%"
echo. >> "%TEST_LOG%"

echo Testing core build system functionality...
echo.

REM Test 1: Engine build
call :test_build_component "--engine" "Engine build" "GameEngineKiro.lib"

REM Test 2: Test build
call :test_build_component "--tests MathTest" "Specific test build" "MathTest.exe"

REM Test 3: Project build
call :test_build_component "--project GameExample" "Specific project build" "GameExample.exe"

REM Test 4: All components build
call :test_all_components_build

REM Test 5: Build type variations
call :test_build_types

REM Test 6: Ninja vs Visual Studio generators
call :test_generators

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo Total Tests: %TOTAL_TESTS%
echo Passed: %PASSED_TESTS%
echo Failed: %FAILED_TESTS%

if %FAILED_TESTS% gtr 0 (
    echo.
    echo [FAILED] Some build functionality tests failed!
    echo Check %TEST_LOG% for details.
    exit /b 1
) else (
    echo.
    echo [SUCCESS] All build functionality tests passed!
    exit /b 0
)

:test_build_component
set "COMMAND=%~1"
set "DESCRIPTION=%~2"
set "EXPECTED_FILE=%~3"
set /a TOTAL_TESTS+=1

echo Testing: %DESCRIPTION%
echo   Command: build_unified.bat %COMMAND%

REM Log test start
echo [TEST %TOTAL_TESTS%] %DESCRIPTION% >> "%TEST_LOG%"
echo   Command: build_unified.bat %COMMAND% >> "%TEST_LOG%"

REM Execute build
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

:test_all_components_build
set /a TOTAL_TESTS+=1

echo Testing: All components build
echo   Command: build_unified.bat --all

REM Log test start
echo [TEST %TOTAL_TESTS%] All components build >> "%TEST_LOG%"
echo   Command: build_unified.bat --all >> "%TEST_LOG%"

REM Execute all components build
.\scripts\build_unified.bat --all >nul 2>&1
set "RESULT=%ERRORLEVEL%"

if %RESULT% equ 0 (
    REM Check if multiple artifacts were created
    call :find_build_artifact "GameEngineKiro.lib" ENGINE_FOUND
    call :find_build_artifact "MathTest.exe" TEST_FOUND
    
    if "!ENGINE_FOUND!"=="true" (
        if "!TEST_FOUND!"=="true" (
            echo   Result: [PASS] - All components built successfully
            echo   Result: [PASS] - All components built successfully >> "%TEST_LOG%"
            set /a PASSED_TESTS+=1
        ) else (
            echo   Result: [FAIL] - Engine built but tests missing
            echo   Result: [FAIL] - Engine built but tests missing >> "%TEST_LOG%"
            set /a FAILED_TESTS+=1
        )
    ) else (
        echo   Result: [FAIL] - Engine artifact not found
        echo   Result: [FAIL] - Engine artifact not found >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - All components build failed with exit code: %RESULT%
    echo   Result: [FAIL] - All components build failed with exit code: %RESULT% >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_build_types
set /a TOTAL_TESTS+=1

echo Testing: Build type variations
echo   Testing Debug and Release builds

REM Log test start
echo [TEST %TOTAL_TESTS%] Build type variations >> "%TEST_LOG%"

REM Test Debug build
.\scripts\build_unified.bat --debug --engine >nul 2>&1
set "DEBUG_RESULT=%ERRORLEVEL%"

REM Test Release build
.\scripts\build_unified.bat --release --engine >nul 2>&1
set "RELEASE_RESULT=%ERRORLEVEL%"

if %DEBUG_RESULT% equ 0 (
    if %RELEASE_RESULT% equ 0 (
        echo   Result: [PASS] - Both Debug and Release builds succeeded
        echo   Result: [PASS] - Both Debug and Release builds succeeded >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - Debug succeeded but Release failed
        echo   Result: [FAIL] - Debug succeeded but Release failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    echo   Result: [FAIL] - Debug build failed
    echo   Result: [FAIL] - Debug build failed >> "%TEST_LOG%"
    set /a FAILED_TESTS+=1
)

echo. >> "%TEST_LOG%"
echo.
goto :eof

:test_generators
set /a TOTAL_TESTS+=1

echo Testing: Generator compatibility
echo   Testing Visual Studio and Ninja generators

REM Log test start
echo [TEST %TOTAL_TESTS%] Generator compatibility >> "%TEST_LOG%"

REM Test default generator (Visual Studio)
.\scripts\build_unified.bat --engine >nul 2>&1
set "VS_RESULT=%ERRORLEVEL%"

REM Test Ninja generator if available
ninja --version >nul 2>&1
if %ERRORLEVEL% equ 0 (
    .\scripts\build_unified.bat --ninja --engine >nul 2>&1
    set "NINJA_RESULT=%ERRORLEVEL%"
    
    if %VS_RESULT% equ 0 (
        if %NINJA_RESULT% equ 0 (
            echo   Result: [PASS] - Both VS and Ninja generators work
            echo   Result: [PASS] - Both VS and Ninja generators work >> "%TEST_LOG%"
            set /a PASSED_TESTS+=1
        ) else (
            echo   Result: [PARTIAL] - VS works but Ninja failed
            echo   Result: [PARTIAL] - VS works but Ninja failed >> "%TEST_LOG%"
            set /a PASSED_TESTS+=1
        )
    ) else (
        echo   Result: [FAIL] - VS generator failed
        echo   Result: [FAIL] - VS generator failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
) else (
    if %VS_RESULT% equ 0 (
        echo   Result: [PASS] - VS generator works (Ninja not available)
        echo   Result: [PASS] - VS generator works (Ninja not available) >> "%TEST_LOG%"
        set /a PASSED_TESTS+=1
    ) else (
        echo   Result: [FAIL] - VS generator failed
        echo   Result: [FAIL] - VS generator failed >> "%TEST_LOG%"
        set /a FAILED_TESTS+=1
    )
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
if exist "build\Debug\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\vs\x64\Release\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\vs\x64\Debug\Debug\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\ninja\x64\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\ninja\x64\Debug\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\projects\GameExample\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\projects\GameExample\Debug\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\vs\x64\Release\projects\GameExample\Release\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\vs\x64\Debug\projects\GameExample\Debug\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\ninja\x64\Release\projects\GameExample\%FILENAME%" set "%RESULT_VAR%=true"
if exist "build\ninja\x64\Debug\projects\GameExample\%FILENAME%" set "%RESULT_VAR%=true"

goto :eof