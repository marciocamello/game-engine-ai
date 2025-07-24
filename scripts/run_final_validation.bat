@echo off
echo ========================================
echo Game Engine Kiro v1.0 Final Validation
echo ========================================
echo.

REM Build the project first
echo Building project...
call .\scripts\build.bat
if %ERRORLEVEL% neq 0 (
    echo Build failed! Cannot run validation tests.
    exit /b 1
)

echo.
echo ========================================
echo Running Final Integration Test
echo ========================================

REM Run the main validation test
if exist "build\Release\FinalV1ValidationTest.exe" (
    echo Running comprehensive v1.0 validation...
    build\Release\FinalV1ValidationTest.exe
    set MAIN_TEST_RESULT=%ERRORLEVEL%
) else (
    echo ERROR: FinalV1ValidationTest.exe not found!
    set MAIN_TEST_RESULT=1
)

echo.
echo ========================================
echo Running Supporting Integration Tests
echo ========================================

set TOTAL_TESTS=0
set PASSED_TESTS=0

REM Audio System Tests
echo.
echo --- Audio System Tests ---
if exist "build\Release\OpenALIntegrationTest.exe" (
    echo Running OpenAL Integration Test...
    build\Release\OpenALIntegrationTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ OpenAL Integration Test PASSED
    ) else (
        echo ❌ OpenAL Integration Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

if exist "build\Release\AudioLoaderTest.exe" (
    echo Running Audio Loader Test...
    build\Release\AudioLoaderTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Audio Loader Test PASSED
    ) else (
        echo ❌ Audio Loader Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

if exist "build\Release\AudioCameraIntegrationTest.exe" (
    echo Running Audio Camera Integration Test...
    build\Release\AudioCameraIntegrationTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Audio Camera Integration Test PASSED
    ) else (
        echo ❌ Audio Camera Integration Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

REM Resource System Tests
echo.
echo --- Resource System Tests ---
if exist "build\Release\ResourceStatisticsTest.exe" (
    echo Running Resource Statistics Test...
    build\Release\ResourceStatisticsTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Resource Statistics Test PASSED
    ) else (
        echo ❌ Resource Statistics Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

if exist "build\Release\TextureLoaderTest.exe" (
    echo Running Texture Loader Test...
    build\Release\TextureLoaderTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Texture Loader Test PASSED
    ) else (
        echo ❌ Texture Loader Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

if exist "build\Release\MeshLoaderTest.exe" (
    echo Running Mesh Loader Test...
    build\Release\MeshLoaderTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Mesh Loader Test PASSED
    ) else (
        echo ❌ Mesh Loader Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

REM Physics System Tests
echo.
echo --- Physics System Tests ---
if exist "build\Release\BulletIntegrationTest.exe" (
    echo Running Bullet Integration Test...
    build\Release\BulletIntegrationTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Bullet Integration Test PASSED
    ) else (
        echo ❌ Bullet Integration Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

if exist "build\Release\PhysicsQueriesTest.exe" (
    echo Running Physics Queries Test...
    build\Release\PhysicsQueriesTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Physics Queries Test PASSED
    ) else (
        echo ❌ Physics Queries Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

REM Error Handling Tests
echo.
echo --- Error Handling Tests ---
if exist "build\Release\ErrorHandlingTest.exe" (
    echo Running Error Handling Test...
    build\Release\ErrorHandlingTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Error Handling Test PASSED
    ) else (
        echo ❌ Error Handling Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

REM Memory Management Tests
echo.
echo --- Memory Management Tests ---
if exist "build\Release\MemoryUsageSimpleTest.exe" (
    echo Running Memory Usage Test...
    build\Release\MemoryUsageSimpleTest.exe
    if %ERRORLEVEL% equ 0 (
        set /a PASSED_TESTS+=1
        echo ✅ Memory Usage Test PASSED
    ) else (
        echo ❌ Memory Usage Test FAILED
    )
    set /a TOTAL_TESTS+=1
)

echo.
echo ========================================
echo Final Validation Summary
echo ========================================

echo Supporting Tests: %PASSED_TESTS%/%TOTAL_TESTS% passed

if %MAIN_TEST_RESULT% equ 0 (
    echo ✅ MAIN VALIDATION TEST: PASSED
) else (
    echo ❌ MAIN VALIDATION TEST: FAILED
)

echo.
if %MAIN_TEST_RESULT% equ 0 (
    if %PASSED_TESTS% geq %TOTAL_TESTS% (
        echo 🎉 GAME ENGINE KIRO v1.0 VALIDATION: COMPLETE SUCCESS
        echo All systems are working correctly and v1.0 requirements are met.
        echo.
        echo ✅ Audio System: OpenAL integration with 3D positioning
        echo ✅ Resource System: Texture, Mesh, and Audio loading
        echo ✅ Error Handling: Graceful fallbacks and recovery
        echo ✅ Memory Management: Proper cleanup and caching
        echo ✅ Performance: Acceptable under load
        echo ✅ Integration: All systems work together
        echo.
        echo Game Engine Kiro v1.0 is ready for use!
        exit /b 0
    ) else (
        echo ⚠️  GAME ENGINE KIRO v1.0 VALIDATION: PARTIAL SUCCESS
        echo Main validation passed but some supporting tests failed.
        echo Review the failed tests above for details.
        exit /b 1
    )
) else (
    echo ❌ GAME ENGINE KIRO v1.0 VALIDATION: FAILED
    echo Core validation test failed. v1.0 requirements not fully met.
    echo Review the test output above for details.
    exit /b 1
)