@echo off
REM Game Engine Kiro - Test Runner Script
REM This script automatically discovers and runs all available test executables

echo ========================================
echo  Game Engine Kiro - Test Runner
echo ========================================

REM Parse command line arguments
set TEST_TYPE=all
set SPECIFIC_PROJECT=
if "%1"=="--engine" set TEST_TYPE=engine
if "%1"=="--projects" set TEST_TYPE=projects
if "%1"=="--project" (
    set TEST_TYPE=project
    set SPECIFIC_PROJECT=%2
)
if "%1"=="--unit" set TEST_TYPE=unit
if "%1"=="--integration" set TEST_TYPE=integration
if "%1"=="--help" goto :help
if "%1"=="-h" goto :help

REM Check if build directory exists and detect build structure
set "BUILD_STRUCTURE=unknown"
set "TEST_PATH="

if exist "build\vs\x64\Release\Release" (
    set "BUILD_STRUCTURE=vs-preset"
    set "TEST_PATH=build\vs\x64\Release\Release"
) else if exist "build\ninja\x64\Release" (
    set "BUILD_STRUCTURE=ninja-preset"
    set "TEST_PATH=build\ninja\x64\Release"
) else if exist "build\Release" (
    set "BUILD_STRUCTURE=manual"
    set "TEST_PATH=build\Release"
) else (
    echo [ERROR] Build directory not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

echo Build Structure: %BUILD_STRUCTURE%
echo Test Path: %TEST_PATH%

echo Test Configuration: %TEST_TYPE%
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT%
echo.

REM Test discovery will be handled automatically

REM Determine which tests to run based on TEST_TYPE
if "%TEST_TYPE%"=="all" goto :run_all_tests
if "%TEST_TYPE%"=="engine" goto :run_engine_tests
if "%TEST_TYPE%"=="projects" goto :run_project_tests
if "%TEST_TYPE%"=="project" goto :run_specific_project_tests
if "%TEST_TYPE%"=="unit" goto :run_unit_tests
if "%TEST_TYPE%"=="integration" goto :run_integration_tests

:run_all_tests
echo Running All Tests (Engine + Projects)...
call :run_engine_tests_internal
if errorlevel 1 goto :test_failed
call :run_project_tests_internal
if errorlevel 1 goto :test_failed
goto :test_success

:run_engine_tests
echo Running Engine Tests Only...
call :run_engine_tests_internal
if errorlevel 1 goto :test_failed
goto :test_success

:run_project_tests
echo Running Project Tests Only...
call :run_project_tests_internal
if errorlevel 1 goto :test_failed
goto :test_success

:run_specific_project_tests
echo Running Tests for Project: %SPECIFIC_PROJECT%...
call :run_specific_project_tests_internal
if errorlevel 1 goto :test_failed
goto :test_success

:run_unit_tests
echo Running Unit Tests Only...
call :run_unit_tests_internal
if errorlevel 2 goto :no_tests_found
if errorlevel 1 goto :test_failed
goto :test_success

:run_integration_tests
echo Running Integration Tests Only...
call :run_integration_tests_internal
if errorlevel 2 goto :no_tests_found
if errorlevel 1 goto :test_failed
goto :test_success

:run_engine_tests_internal
echo.
echo Running Engine Unit Tests...
echo ----------------------------------------
call :run_unit_tests_internal
if errorlevel 1 exit /b 1

echo.
echo Running Engine Integration Tests...
echo ----------------------------------------
call :run_integration_tests_internal
if errorlevel 1 exit /b 1

exit /b 0

:run_unit_tests_internal
REM Auto-discover and run unit tests (exclude Integration tests and EnhancedTestRunner)
set "TESTS_FOUND=0"
setlocal enabledelayedexpansion
for %%f in ("%TEST_PATH%\*Test.exe") do (
    set "testname=%%~nf"
    REM Skip integration tests and test runner utility
    echo !testname! | findstr /i "Integration" >nul
    if errorlevel 1 (
        echo !testname! | findstr /i "EnhancedTestRunner" >nul
        if errorlevel 1 (
            echo !testname! | findstr /i "TestConfigManager" >nul
            if errorlevel 1 (
                set /a TESTS_FOUND+=1
                echo [INFO] Running !testname!...
                "%%f" >nul 2>&1
                if errorlevel 1 (
                    echo [FAILED] !testname!
                    endlocal
                    exit /b 1
                ) else (
                    echo [PASS] !testname!
                )
            )
        )
    )
)

if !TESTS_FOUND! equ 0 (
    echo [INFO] No unit tests found in %TEST_PATH%
    echo [INFO] Run 'scripts\build_unified.bat --tests' to build tests first
    endlocal
    exit /b 2
)
endlocal
exit /b 0

:run_integration_tests_internal
REM Auto-discover and run integration tests
set "INTEGRATION_TESTS_FOUND=0"
setlocal enabledelayedexpansion
for %%f in ("%TEST_PATH%\*IntegrationTest.exe") do (
    set "testname=%%~nf"
    set /a INTEGRATION_TESTS_FOUND+=1
    echo [INFO] Running !testname!...
    "%%f" >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] !testname!
        endlocal
        exit /b 1
    ) else (
        echo [PASS] !testname!
    )
)

if !INTEGRATION_TESTS_FOUND! equ 0 (
    echo [INFO] No integration tests found in %TEST_PATH%
    echo [INFO] Run 'scripts\build_unified.bat --tests' to build tests first
    endlocal
    exit /b 2
)
endlocal
exit /b 0

:run_project_tests_internal
echo.
echo Running Project Tests...
echo ----------------------------------------

REM Check if project test structure exists
if exist "build\projects" (
    REM Auto-discover project tests
    for /r "build\projects" %%f in (*Test.exe) do (
        set "testname=%%~nf"
        setlocal enabledelayedexpansion
        echo [INFO] Running !testname!...
        "%%f" >nul 2>&1
        if errorlevel 1 (
            echo [FAILED] !testname!
            endlocal
            exit /b 1
        ) else (
            echo [PASS] !testname!
        )
        endlocal
    )
) else (
    echo [INFO] Project test structure not built yet
)
exit /b 0

:run_specific_project_tests_internal
REM Run tests for a specific project
if exist "build\projects\%SPECIFIC_PROJECT%" (
    for %%f in ("build\projects\%SPECIFIC_PROJECT%\*Test.exe") do (
        set "testname=%%~nf"
        setlocal enabledelayedexpansion
        echo [INFO] Running !testname!...
        "%%f" >nul 2>&1
        if errorlevel 1 (
            echo [FAILED] !testname!
            endlocal
            exit /b 1
        ) else (
            echo [PASS] !testname!
        )
        endlocal
    )
) else (
    echo [ERROR] Project %SPECIFIC_PROJECT% not found or no tests available
    exit /b 1
)
exit /b 0

:run_engine_unit_tests_only
REM Auto-discover and run unit tests only (exclude Integration tests and utilities)
for %%f in ("%TEST_PATH%\*Test.exe") do (
    set "testname=%%~nf"
    setlocal enabledelayedexpansion
    REM Skip integration tests and test runner utilities
    echo !testname! | findstr /i "Integration" >nul
    if errorlevel 1 (
        echo !testname! | findstr /i "EnhancedTestRunner" >nul
        if errorlevel 1 (
            echo !testname! | findstr /i "TestConfigManager" >nul
            if errorlevel 1 (
                echo [INFO] Running !testname!...
                "%%f" >nul 2>&1
                if errorlevel 1 (
                    echo [FAILED] !testname!
                    endlocal
                    exit /b 1
                ) else (
                    echo [PASS] !testname!
                )
            )
        )
    )
    endlocal
)
exit /b 0

:run_engine_integration_tests_only
REM Auto-discover and run integration tests only
for %%f in ("%TEST_PATH%\*IntegrationTest.exe") do (
    set "testname=%%~nf"
    setlocal enabledelayedexpansion
    echo [INFO] Running !testname!...
    "%%f" >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] !testname!
        endlocal
        exit /b 1
    ) else (
        echo [PASS] !testname!
    )
    endlocal
)
exit /b 0

:test_success
echo.
echo ========================================
echo [SUCCESS] ALL TESTS PASSED!
echo ========================================
echo.
exit /b 0

:test_failed
echo.
echo ========================================
echo [FAILED] One or more tests failed!
echo ========================================
echo.
exit /b 1

:no_tests_found
echo.
echo ========================================
echo [INFO] NO TESTS FOUND!
echo ========================================
echo.
echo No test executables were found in the build directory.
echo Please run 'scripts\build_unified.bat --tests' to build tests first.
echo.
exit /b 2

:help
echo Game Engine Kiro - Test Runner
echo.
echo Usage: run_tests.bat [options]
echo.
echo Test Categories:
echo   (no options)      Run all tests (engine + projects)
echo   --engine          Run engine tests only
echo   --projects        Run project tests only
echo   --project NAME    Run tests for specific project only
echo   --unit            Run unit tests only
echo   --integration     Run integration tests only
echo.
echo Examples:
echo   run_tests.bat                    # Run all tests
echo   run_tests.bat --engine           # Engine tests only
echo   run_tests.bat --projects         # Project tests only
echo   run_tests.bat --project GameExample # GameExample tests only
echo   run_tests.bat --unit             # Unit tests only
echo   run_tests.bat --integration      # Integration tests only
echo.
echo   --help, -h        Show this help message
echo.
goto :end

:end