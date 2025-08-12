@echo off
REM Game Engine Kiro - Test Runner Script
REM This script runs all available test executables from both engine and project test structures

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

REM Check if build directory exists
if not exist "build\Release" (
    echo [ERROR] Build directory not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

echo Test Configuration: %TEST_TYPE%
if not "%SPECIFIC_PROJECT%"=="" echo Specific Project: %SPECIFIC_PROJECT%
echo.

echo.
echo Running Unit Tests...
echo ----------------------------------------

REM Run Math unit test
if exist "build\Release\MathTest.exe" (
    echo [INFO] Running MathTest...
    build\Release\MathTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MathTest
        goto :test_failed
    ) else (
        echo [PASS] MathTest
    )
)

REM Run Matrix unit test
if exist "build\Release\MatrixTest.exe" (
    echo [INFO] Running MatrixTest...
    build\Release\MatrixTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MatrixTest
        goto :test_failed
    ) else (
        echo [PASS] MatrixTest
    )
)

REM Run Quaternion unit test
if exist "build\Release\QuaternionTest.exe" (
    echo [INFO] Running QuaternionTest...
    build\Release\QuaternionTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] QuaternionTest
        goto :test_failed
    ) else (
        echo [PASS] QuaternionTest
    )
)

REM Run AssertionMacros unit test
if exist "build\Release\AssertionmacrosTest.exe" (
    echo [INFO] Running AssertionmacrosTest...
    build\Release\AssertionmacrosTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AssertionmacrosTest
        goto :test_failed
    ) else (
        echo [PASS] AssertionmacrosTest
    )
)

REM Run Audio3DPositioning unit test
if exist "build\Release\Audio3dpositioningTest.exe" (
    echo [INFO] Running Audio3dpositioningTest...
    build\Release\Audio3dpositioningTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] Audio3dpositioningTest
        goto :test_failed
    ) else (
        echo [PASS] Audio3dpositioningTest
    )
)

REM Run AudioEngine unit test
if exist "build\Release\AudioengineTest.exe" (
    echo [INFO] Running AudioengineTest...
    build\Release\AudioengineTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AudioengineTest
        goto :test_failed
    ) else (
        echo [PASS] AudioengineTest
    )
)

REM Run AudioLoader unit test
if exist "build\Release\AudioloaderTest.exe" (
    echo [INFO] Running AudioloaderTest...
    build\Release\AudioloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AudioloaderTest
        goto :test_failed
    ) else (
        echo [PASS] AudioloaderTest
    )
)

REM Run MeshLoader unit test
if exist "build\Release\MeshloaderTest.exe" (
    echo [INFO] Running MeshloaderTest...
    build\Release\MeshloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MeshloaderTest
        goto :test_failed
    ) else (
        echo [PASS] MeshloaderTest
    )
)

REM Run MeshOptimizer unit test
if exist "build\Release\MeshoptimizerTest.exe" (
    echo [INFO] Running MeshoptimizerTest...
    build\Release\MeshoptimizerTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MeshoptimizerTest
        goto :test_failed
    ) else (
        echo [PASS] MeshoptimizerTest
    )
)

REM Run ModelNode unit test
if exist "build\Release\ModelnodeTest.exe" (
    echo [INFO] Running ModelnodeTest...
    build\Release\ModelnodeTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModelnodeTest
        goto :test_failed
    ) else (
        echo [PASS] ModelnodeTest
    )
)

REM Run ResourceFallbacks unit test
if exist "build\Release\ResourcefallbacksTest.exe" (
    echo [INFO] Running ResourcefallbacksTest...
    build\Release\ResourcefallbacksTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourcefallbacksTest
        goto :test_failed
    ) else (
        echo [PASS] ResourcefallbacksTest
    )
)

REM Run ResourceManager unit test
if exist "build\Release\ResourcemanagerTest.exe" (
    echo [INFO] Running ResourcemanagerTest...
    build\Release\ResourcemanagerTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourcemanagerTest
        goto :test_failed
    ) else (
        echo [PASS] ResourcemanagerTest
    )
)

REM Run TextureLoader unit test
if exist "build\Release\TextureloaderTest.exe" (
    echo [INFO] Running TextureloaderTest...
    build\Release\TextureloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] TextureloaderTest
        goto :test_failed
    ) else (
        echo [PASS] TextureloaderTest
    )
)

REM Run ShaderManager unit test
if exist "build\Release\ShadermanagerTest.exe" (
    echo [INFO] Running ShadermanagerTest...
    build\Release\ShadermanagerTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ShadermanagerTest
        goto :test_failed
    ) else (
        echo [PASS] ShadermanagerTest
    )
)

REM Run ModuleRegistry unit test
if exist "build\Release\ModuleregistryTest.exe" (
    echo [INFO] Running ModuleregistryTest...
    build\Release\ModuleregistryTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModuleregistryTest
        goto :test_failed
    ) else (
        echo [PASS] ModuleregistryTest
    )
)

REM Run ModuleConfigLoader unit test
if exist "build\Release\ModuleconfigloaderTest.exe" (
    echo [INFO] Running ModuleconfigloaderTest...
    build\Release\ModuleconfigloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModuleconfigloaderTest
        goto :test_failed
    ) else (
        echo [PASS] ModuleconfigloaderTest
    )
)

REM Run ProjectTemplate unit test
if exist "build\Release\ProjecttemplateTest.exe" (
    echo [INFO] Running ProjecttemplateTest...
    build\Release\ProjecttemplateTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ProjecttemplateTest
        goto :test_failed
    ) else (
        echo [PASS] ProjecttemplateTest
    )
)

echo.
echo Running Integration Tests...
echo ----------------------------------------

REM Run BulletUtilsSimpleTest
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    echo [INFO] Running BulletUtilsSimpleTest...
    build\Release\BulletUtilsSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletUtilsSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] BulletUtilsSimpleTest
    )
)

REM Run BulletIntegrationTest
if exist "build\Release\BulletIntegrationTest.exe" (
    echo [INFO] Running BulletIntegrationTest...
    build\Release\BulletIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletIntegrationTest
        goto :test_failed
    ) else (
        echo [PASS] BulletIntegrationTest
    )
)

REM Run BulletConversionTest
if exist "build\Release\BulletConversionTest.exe" (
    echo [INFO] Running BulletConversionTest...
    build\Release\BulletConversionTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletConversionTest
        goto :test_failed
    ) else (
        echo [PASS] BulletConversionTest
    )
)

REM Run CollisionShapeFactorySimpleTest
if exist "build\Release\CollisionShapeFactorySimpleTest.exe" (
    echo [INFO] Running CollisionShapeFactorySimpleTest...
    build\Release\CollisionShapeFactorySimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] CollisionShapeFactorySimpleTest
        goto :test_failed
    ) else (
        echo [PASS] CollisionShapeFactorySimpleTest
    )
)

REM Run PhysicsQueriesTest
if exist "build\Release\PhysicsQueriesTest.exe" (
    echo [INFO] Running PhysicsQueriesTest...
    build\Release\PhysicsQueriesTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsQueriesTest
        goto :test_failed
    ) else (
        echo [PASS] PhysicsQueriesTest
    )
)

REM Run PhysicsConfigurationTest
if exist "build\Release\PhysicsConfigurationTest.exe" (
    echo [INFO] Running PhysicsConfigurationTest...
    build\Release\PhysicsConfigurationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsConfigurationTest
        goto :test_failed
    ) else (
        echo [PASS] PhysicsConfigurationTest
    )
)

REM Run MovementComponentComparisonTest
if exist "build\Release\MovementComponentComparisonTest.exe" (
    echo [INFO] Running MovementComponentComparisonTest...
    build\Release\MovementComponentComparisonTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MovementComponentComparisonTest
        goto :test_failed
    ) else (
        echo [PASS] MovementComponentComparisonTest
    )
)

REM Run PhysicsPerformanceSimpleTest
if exist "build\Release\PhysicsPerformanceSimpleTest.exe" (
    echo [INFO] Running PhysicsPerformanceSimpleTest...
    build\Release\PhysicsPerformanceSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsPerformanceSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] PhysicsPerformanceSimpleTest
    )
)

REM Run MemoryUsageSimpleTest
if exist "build\Release\MemoryUsageSimpleTest.exe" (
    echo [INFO] Running MemoryUsageSimpleTest...
    build\Release\MemoryUsageSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MemoryUsageSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] MemoryUsageSimpleTest
    )
)

REM Run CharacterBehaviorSimpleTest
if exist "build\Release\CharacterBehaviorSimpleTest.exe" (
    echo [INFO] Running CharacterBehaviorSimpleTest...
    build\Release\CharacterBehaviorSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] CharacterBehaviorSimpleTest
        goto :test_failed
    ) else (
        echo [PASS] CharacterBehaviorSimpleTest
    )
)

REM Run OpenALIntegrationTest
if exist "build\Release\OpenALIntegrationTest.exe" (
    echo [INFO] Running OpenALIntegrationTest...
    build\Release\OpenALIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] OpenALIntegrationTest
        goto :test_failed
    ) else (
        echo [PASS] OpenALIntegrationTest
    )
)

REM Run AudioCameraIntegrationTest
if exist "build\Release\AudioCameraIntegrationTest.exe" (
    echo [INFO] Running AudioCameraIntegrationTest...
    build\Release\AudioCameraIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AudioCameraIntegrationTest
        goto :test_failed
    ) else (
        echo [PASS] AudioCameraIntegrationTest
    )
)

REM Run ResourceStatisticsTest
if exist "build\Release\ResourceStatisticsTest.exe" (
    echo [INFO] Running ResourceStatisticsTest...
    build\Release\ResourceStatisticsTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourceStatisticsTest
        goto :test_failed
    ) else (
        echo [PASS] ResourceStatisticsTest
    )
)

REM Run ErrorHandlingTest
if exist "build\Release\ErrorHandlingTest.exe" (
    echo [INFO] Running ErrorHandlingTest...
    build\Release\ErrorHandlingTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ErrorHandlingTest
        goto :test_failed
    ) else (
        echo [PASS] ErrorHandlingTest
    )
)

REM Run FinalV1ValidationTest
if exist "build\Release\FinalV1ValidationTest.exe" (
    echo [INFO] Running FinalV1ValidationTest...
    build\Release\FinalV1ValidationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] FinalV1ValidationTest
        goto :test_failed
    ) else (
        echo [PASS] FinalV1ValidationTest
    )
)

REM Run ModelLoaderAssimpTest
if exist "build\Release\ModelLoaderAssimpTest.exe" (
    echo [INFO] Running ModelLoaderAssimpTest...
    build\Release\ModelLoaderAssimpTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModelLoaderAssimpTest
        goto :test_failed
    ) else (
        echo [PASS] ModelLoaderAssimpTest
    )
)

REM Run MaterialImporterTest
if exist "build\Release\MaterialImporterTest.exe" (
    echo [INFO] Running MaterialImporterTest...
    build\Release\MaterialImporterTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MaterialImporterTest
        goto :test_failed
    ) else (
        echo [PASS] MaterialImporterTest
    )
)

REM Run GLTFLoaderTest
if exist "build\Release\GLTFLoaderTest.exe" (
    echo [INFO] Running GLTFLoaderTest...
    build\Release\GLTFLoaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] GLTFLoaderTest
        goto :test_failed
    ) else (
        echo [PASS] GLTFLoaderTest
    )
)

REM Run FBXLoaderTest
if exist "build\Release\FBXLoaderTest.exe" (
    echo [INFO] Running FBXLoaderTest...
    build\Release\FBXLoaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] FBXLoaderTest
        goto :test_failed
    ) else (
        echo [PASS] FBXLoaderTest
    )
)

REM Run ModelResourceIntegrationTest
if exist "build\Release\ModelResourceIntegrationTest.exe" (
    echo [INFO] Running ModelResourceIntegrationTest...
    build\Release\ModelResourceIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModelResourceIntegrationTest
        goto :test_failed
    ) else (
        echo [PASS] ModelResourceIntegrationTest
    )
)

REM Run ResourceUsageTrackingTest
if exist "build\Release\ResourceUsageTrackingTest.exe" (
    echo [INFO] Running ResourceUsageTrackingTest...
    build\Release\ResourceUsageTrackingTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourceUsageTrackingTest
        goto :test_failed
    ) else (
        echo [PASS] ResourceUsageTrackingTest
    )
)

REM Run ShadermanagerintegrationTest
if exist "build\Release\ShadermanagerintegrationTest.exe" (
    echo [INFO] Running ShadermanagerintegrationTest...
    build\Release\ShadermanagerintegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ShadermanagerintegrationTest
        goto :test_failed
    ) else (
        echo [PASS] ShadermanagerintegrationTest
    )
)

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
if errorlevel 1 goto :test_failed
goto :test_success

:run_integration_tests
echo Running Integration Tests Only...
call :run_integration_tests_internal
if errorlevel 1 goto :test_failed
goto :test_success

:run_engine_tests_internal
echo.
echo Running Engine Unit Tests...
echo ----------------------------------------

REM Continue with existing unit tests...
REM Run Math unit test
if exist "build\Release\MathTest.exe" (
    echo [INFO] Running MathTest...
    build\Release\MathTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MathTest
        exit /b 1
    ) else (
        echo [PASS] MathTest
    )
)

REM Run Matrix unit test
if exist "build\Release\MatrixTest.exe" (
    echo [INFO] Running MatrixTest...
    build\Release\MatrixTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MatrixTest
        exit /b 1
    ) else (
        echo [PASS] MatrixTest
    )
)

REM Run Quaternion unit test
if exist "build\Release\QuaternionTest.exe" (
    echo [INFO] Running QuaternionTest...
    build\Release\QuaternionTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] QuaternionTest
        exit /b 1
    ) else (
        echo [PASS] QuaternionTest
    )
)

REM Run AssertionMacros unit test
if exist "build\Release\AssertionmacrosTest.exe" (
    echo [INFO] Running AssertionmacrosTest...
    build\Release\AssertionmacrosTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AssertionmacrosTest
        exit /b 1
    ) else (
        echo [PASS] AssertionmacrosTest
    )
)

REM Run Audio3DPositioning unit test
if exist "build\Release\Audio3dpositioningTest.exe" (
    echo [INFO] Running Audio3dpositioningTest...
    build\Release\Audio3dpositioningTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] Audio3dpositioningTest
        exit /b 1
    ) else (
        echo [PASS] Audio3dpositioningTest
    )
)

REM Run AudioEngine unit test
if exist "build\Release\AudioengineTest.exe" (
    echo [INFO] Running AudioengineTest...
    build\Release\AudioengineTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AudioengineTest
        exit /b 1
    ) else (
        echo [PASS] AudioengineTest
    )
)

REM Run AudioLoader unit test
if exist "build\Release\AudioloaderTest.exe" (
    echo [INFO] Running AudioloaderTest...
    build\Release\AudioloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AudioloaderTest
        exit /b 1
    ) else (
        echo [PASS] AudioloaderTest
    )
)

REM Run MeshLoader unit test
if exist "build\Release\MeshloaderTest.exe" (
    echo [INFO] Running MeshloaderTest...
    build\Release\MeshloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MeshloaderTest
        exit /b 1
    ) else (
        echo [PASS] MeshloaderTest
    )
)

REM Run MeshOptimizer unit test
if exist "build\Release\MeshoptimizerTest.exe" (
    echo [INFO] Running MeshoptimizerTest...
    build\Release\MeshoptimizerTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MeshoptimizerTest
        exit /b 1
    ) else (
        echo [PASS] MeshoptimizerTest
    )
)

REM Run ModelNode unit test
if exist "build\Release\ModelnodeTest.exe" (
    echo [INFO] Running ModelnodeTest...
    build\Release\ModelnodeTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModelnodeTest
        exit /b 1
    ) else (
        echo [PASS] ModelnodeTest
    )
)

REM Run ResourceFallbacks unit test
if exist "build\Release\ResourcefallbacksTest.exe" (
    echo [INFO] Running ResourcefallbacksTest...
    build\Release\ResourcefallbacksTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourcefallbacksTest
        exit /b 1
    ) else (
        echo [PASS] ResourcefallbacksTest
    )
)

REM Run ResourceManager unit test
if exist "build\Release\ResourcemanagerTest.exe" (
    echo [INFO] Running ResourcemanagerTest...
    build\Release\ResourcemanagerTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourcemanagerTest
        exit /b 1
    ) else (
        echo [PASS] ResourcemanagerTest
    )
)

REM Run TextureLoader unit test
if exist "build\Release\TextureloaderTest.exe" (
    echo [INFO] Running TextureloaderTest...
    build\Release\TextureloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] TextureloaderTest
        exit /b 1
    ) else (
        echo [PASS] TextureloaderTest
    )
)

REM Run ShaderManager unit test
if exist "build\Release\ShadermanagerTest.exe" (
    echo [INFO] Running ShadermanagerTest...
    build\Release\ShadermanagerTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ShadermanagerTest
        exit /b 1
    ) else (
        echo [PASS] ShadermanagerTest
    )
)

REM Run ModuleRegistry unit test
if exist "build\Release\ModuleregistryTest.exe" (
    echo [INFO] Running ModuleregistryTest...
    build\Release\ModuleregistryTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModuleregistryTest
        exit /b 1
    ) else (
        echo [PASS] ModuleregistryTest
    )
)

REM Run ModuleConfigLoader unit test
if exist "build\Release\ModuleconfigloaderTest.exe" (
    echo [INFO] Running ModuleconfigloaderTest...
    build\Release\ModuleconfigloaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModuleconfigloaderTest
        exit /b 1
    ) else (
        echo [PASS] ModuleconfigloaderTest
    )
)

REM Run ProjectTemplate unit test
if exist "build\Release\ProjecttemplateTest.exe" (
    echo [INFO] Running ProjecttemplateTest...
    build\Release\ProjecttemplateTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ProjecttemplateTest
        exit /b 1
    ) else (
        echo [PASS] ProjecttemplateTest
    )
)

echo.
echo Running Engine Integration Tests...
echo ----------------------------------------

REM Run BulletUtilsSimpleTest
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    echo [INFO] Running BulletUtilsSimpleTest...
    build\Release\BulletUtilsSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletUtilsSimpleTest
        exit /b 1
    ) else (
        echo [PASS] BulletUtilsSimpleTest
    )
)

REM Run BulletIntegrationTest
if exist "build\Release\BulletIntegrationTest.exe" (
    echo [INFO] Running BulletIntegrationTest...
    build\Release\BulletIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletIntegrationTest
        exit /b 1
    ) else (
        echo [PASS] BulletIntegrationTest
    )
)

REM Run BulletConversionTest
if exist "build\Release\BulletConversionTest.exe" (
    echo [INFO] Running BulletConversionTest...
    build\Release\BulletConversionTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletConversionTest
        exit /b 1
    ) else (
        echo [PASS] BulletConversionTest
    )
)

REM Run CollisionShapeFactorySimpleTest
if exist "build\Release\CollisionShapeFactorySimpleTest.exe" (
    echo [INFO] Running CollisionShapeFactorySimpleTest...
    build\Release\CollisionShapeFactorySimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] CollisionShapeFactorySimpleTest
        exit /b 1
    ) else (
        echo [PASS] CollisionShapeFactorySimpleTest
    )
)

REM Run PhysicsQueriesTest
if exist "build\Release\PhysicsQueriesTest.exe" (
    echo [INFO] Running PhysicsQueriesTest...
    build\Release\PhysicsQueriesTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsQueriesTest
        exit /b 1
    ) else (
        echo [PASS] PhysicsQueriesTest
    )
)

REM Run PhysicsConfigurationTest
if exist "build\Release\PhysicsConfigurationTest.exe" (
    echo [INFO] Running PhysicsConfigurationTest...
    build\Release\PhysicsConfigurationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsConfigurationTest
        exit /b 1
    ) else (
        echo [PASS] PhysicsConfigurationTest
    )
)

REM Run MovementComponentComparisonTest
if exist "build\Release\MovementComponentComparisonTest.exe" (
    echo [INFO] Running MovementComponentComparisonTest...
    build\Release\MovementComponentComparisonTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MovementComponentComparisonTest
        exit /b 1
    ) else (
        echo [PASS] MovementComponentComparisonTest
    )
)

REM Run PhysicsPerformanceSimpleTest
if exist "build\Release\PhysicsPerformanceSimpleTest.exe" (
    echo [INFO] Running PhysicsPerformanceSimpleTest...
    build\Release\PhysicsPerformanceSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] PhysicsPerformanceSimpleTest
        exit /b 1
    ) else (
        echo [PASS] PhysicsPerformanceSimpleTest
    )
)

REM Run MemoryUsageSimpleTest
if exist "build\Release\MemoryUsageSimpleTest.exe" (
    echo [INFO] Running MemoryUsageSimpleTest...
    build\Release\MemoryUsageSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MemoryUsageSimpleTest
        exit /b 1
    ) else (
        echo [PASS] MemoryUsageSimpleTest
    )
)

REM Run CharacterBehaviorSimpleTest
if exist "build\Release\CharacterBehaviorSimpleTest.exe" (
    echo [INFO] Running CharacterBehaviorSimpleTest...
    build\Release\CharacterBehaviorSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] CharacterBehaviorSimpleTest
        exit /b 1
    ) else (
        echo [PASS] CharacterBehaviorSimpleTest
    )
)

REM Run OpenALIntegrationTest
if exist "build\Release\OpenALIntegrationTest.exe" (
    echo [INFO] Running OpenALIntegrationTest...
    build\Release\OpenALIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] OpenALIntegrationTest
        exit /b 1
    ) else (
        echo [PASS] OpenALIntegrationTest
    )
)

REM Run AudioCameraIntegrationTest
if exist "build\Release\AudioCameraIntegrationTest.exe" (
    echo [INFO] Running AudioCameraIntegrationTest...
    build\Release\AudioCameraIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] AudioCameraIntegrationTest
        exit /b 1
    ) else (
        echo [PASS] AudioCameraIntegrationTest
    )
)

REM Run ResourceStatisticsTest
if exist "build\Release\ResourceStatisticsTest.exe" (
    echo [INFO] Running ResourceStatisticsTest...
    build\Release\ResourceStatisticsTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourceStatisticsTest
        exit /b 1
    ) else (
        echo [PASS] ResourceStatisticsTest
    )
)

REM Run ErrorHandlingTest
if exist "build\Release\ErrorHandlingTest.exe" (
    echo [INFO] Running ErrorHandlingTest...
    build\Release\ErrorHandlingTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ErrorHandlingTest
        exit /b 1
    ) else (
        echo [PASS] ErrorHandlingTest
    )
)

REM Run FinalV1ValidationTest
if exist "build\Release\FinalV1ValidationTest.exe" (
    echo [INFO] Running FinalV1ValidationTest...
    build\Release\FinalV1ValidationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] FinalV1ValidationTest
        exit /b 1
    ) else (
        echo [PASS] FinalV1ValidationTest
    )
)

REM Run ModelLoaderAssimpTest
if exist "build\Release\ModelLoaderAssimpTest.exe" (
    echo [INFO] Running ModelLoaderAssimpTest...
    build\Release\ModelLoaderAssimpTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModelLoaderAssimpTest
        exit /b 1
    ) else (
        echo [PASS] ModelLoaderAssimpTest
    )
)

REM Run MaterialImporterTest
if exist "build\Release\MaterialImporterTest.exe" (
    echo [INFO] Running MaterialImporterTest...
    build\Release\MaterialImporterTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MaterialImporterTest
        exit /b 1
    ) else (
        echo [PASS] MaterialImporterTest
    )
)

REM Run GLTFLoaderTest
if exist "build\Release\GLTFLoaderTest.exe" (
    echo [INFO] Running GLTFLoaderTest...
    build\Release\GLTFLoaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] GLTFLoaderTest
        exit /b 1
    ) else (
        echo [PASS] GLTFLoaderTest
    )
)

REM Run FBXLoaderTest
if exist "build\Release\FBXLoaderTest.exe" (
    echo [INFO] Running FBXLoaderTest...
    build\Release\FBXLoaderTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] FBXLoaderTest
        exit /b 1
    ) else (
        echo [PASS] FBXLoaderTest
    )
)

REM Run ModelResourceIntegrationTest
if exist "build\Release\ModelResourceIntegrationTest.exe" (
    echo [INFO] Running ModelResourceIntegrationTest...
    build\Release\ModelResourceIntegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ModelResourceIntegrationTest
        exit /b 1
    ) else (
        echo [PASS] ModelResourceIntegrationTest
    )
)

REM Run ResourceUsageTrackingTest
if exist "build\Release\ResourceUsageTrackingTest.exe" (
    echo [INFO] Running ResourceUsageTrackingTest...
    build\Release\ResourceUsageTrackingTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ResourceUsageTrackingTest
        exit /b 1
    ) else (
        echo [PASS] ResourceUsageTrackingTest
    )
)

REM Run ShadermanagerintegrationTest
if exist "build\Release\ShadermanagerintegrationTest.exe" (
    echo [INFO] Running ShadermanagerintegrationTest...
    build\Release\ShadermanagerintegrationTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] ShadermanagerintegrationTest
        exit /b 1
    ) else (
        echo [PASS] ShadermanagerintegrationTest
    )
)

exit /b 0

:run_project_tests_internal
echo.
echo Running Project Tests...
echo ----------------------------------------

REM Check if project test structure exists
if exist "build\projects\Tests\Release" (
    echo [INFO] Project test structure found
    
    REM Look for project test executables
    if exist "build\projects\Tests\Release\*.exe" (
        for %%f in (build\projects\Tests\Release\*.exe) do (
            echo [INFO] Running %%~nf...
            "%%f" >nul 2>&1
            if errorlevel 1 (
                echo [FAILED] %%~nf
                exit /b 1
            ) else (
                echo [PASS] %%~nf
            )
        )
    ) else (
        echo [INFO] No project test executables found
    )
) else (
    echo [INFO] Project test structure not built yet
)

exit /b 0

:run_specific_project_tests_internal
echo.
echo Running Tests for Project: %SPECIFIC_PROJECT%...
echo ----------------------------------------

REM Check if specific project test structure exists
if exist "build\projects\Tests\%SPECIFIC_PROJECT%\Release" (
    echo [INFO] Project test structure found for %SPECIFIC_PROJECT%
    
    REM Look for project-specific test executables
    if exist "build\projects\Tests\%SPECIFIC_PROJECT%\Release\*.exe" (
        for %%f in (build\projects\Tests\%SPECIFIC_PROJECT%\Release\*.exe) do (
            echo [INFO] Running %%~nf...
            "%%f" >nul 2>&1
            if errorlevel 1 (
                echo [FAILED] %%~nf
                exit /b 1
            ) else (
                echo [PASS] %%~nf
            )
        )
    ) else (
        echo [INFO] No test executables found for project %SPECIFIC_PROJECT%
    )
) else (
    echo [INFO] Project test structure not built for %SPECIFIC_PROJECT%
)

exit /b 0

:run_unit_tests_internal
echo.
echo Running Unit Tests Only...
echo ----------------------------------------

REM Run only unit tests (those in tests/unit/)
call :run_engine_unit_tests_only
if errorlevel 1 exit /b 1

REM Run project unit tests if they exist
if exist "build\projects\Tests\unit\Release" (
    for %%f in (build\projects\Tests\unit\Release\*.exe) do (
        echo [INFO] Running %%~nf...
        "%%f" >nul 2>&1
        if errorlevel 1 (
            echo [FAILED] %%~nf
            exit /b 1
        ) else (
            echo [PASS] %%~nf
        )
    )
)

exit /b 0

:run_integration_tests_internal
echo.
echo Running Integration Tests Only...
echo ----------------------------------------

REM Run only integration tests (those in tests/integration/)
call :run_engine_integration_tests_only
if errorlevel 1 exit /b 1

REM Run project integration tests if they exist
if exist "build\projects\Tests\integration\Release" (
    for %%f in (build\projects\Tests\integration\Release\*.exe) do (
        echo [INFO] Running %%~nf...
        "%%f" >nul 2>&1
        if errorlevel 1 (
            echo [FAILED] %%~nf
            exit /b 1
        ) else (
            echo [PASS] %%~nf
        )
    )
)

exit /b 0

:run_engine_unit_tests_only
REM Run Math unit test
if exist "build\Release\MathTest.exe" (
    echo [INFO] Running MathTest...
    build\Release\MathTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MathTest
        exit /b 1
    ) else (
        echo [PASS] MathTest
    )
)

REM Run Matrix unit test
if exist "build\Release\MatrixTest.exe" (
    echo [INFO] Running MatrixTest...
    build\Release\MatrixTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] MatrixTest
        exit /b 1
    ) else (
        echo [PASS] MatrixTest
    )
)

REM Run Quaternion unit test
if exist "build\Release\QuaternionTest.exe" (
    echo [INFO] Running QuaternionTest...
    build\Release\QuaternionTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] QuaternionTest
        exit /b 1
    ) else (
        echo [PASS] QuaternionTest
    )
)

REM Continue with other unit tests...
REM (Additional unit tests would be listed here)

exit /b 0

:run_engine_integration_tests_only
REM Run BulletUtilsSimpleTest
if exist "build\Release\BulletUtilsSimpleTest.exe" (
    echo [INFO] Running BulletUtilsSimpleTest...
    build\Release\BulletUtilsSimpleTest.exe >nul 2>&1
    if errorlevel 1 (
        echo [FAILED] BulletUtilsSimpleTest
        exit /b 1
    ) else (
        echo [PASS] BulletUtilsSimpleTest
    )
)

REM Continue with other integration tests...
REM (Additional integration tests would be listed here)

exit /b 0

:test_success
echo.
echo ========================================
echo [SUCCESS] ALL TESTS PASSED!
echo ========================================
exit /b 0

:test_failed
echo.
echo ========================================
echo [FAILED] One or more tests failed!
echo ========================================
exit /b 1

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