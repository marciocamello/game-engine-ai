@echo off
REM Game Engine Kiro - Test Runner Script
REM This script runs all available test executables

echo ========================================
echo  Game Engine Kiro - Test Runner
echo ========================================

REM Check if build directory exists
if not exist "build\Release" (
    echo [ERROR] Build directory not found. Please run scripts\build_unified.bat --tests first.
    exit /b 1
)

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