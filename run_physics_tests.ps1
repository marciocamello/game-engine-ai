# Game Engine Kiro - Physics Testing Script
# This script runs all physics-related tests and provides comprehensive test coverage

param(
    [switch]$Verbose,
    [switch]$BuildFirst,
    [string]$Filter = "*"
)

Write-Host "Game Engine Kiro - Physics Test Suite" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan

# Build project first if requested
if ($BuildFirst) {
    Write-Host "`nBuilding project..." -ForegroundColor Yellow
    & .\build.bat
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed! Exiting." -ForegroundColor Red
        exit 1
    }
    Write-Host "Build completed successfully." -ForegroundColor Green
}

# Define test executables with descriptions
$testSuite = @(
    @{
        Name = "BulletUtilsTest.exe"
        Description = "Bullet Physics utility functions and conversions"
        Category = "Unit Tests"
    },
    @{
        Name = "CollisionShapeFactoryTest.exe"
        Description = "Collision shape creation and management"
        Category = "Unit Tests"
    },
    @{
        Name = "BulletIntegrationTest.exe"
        Description = "Basic Bullet Physics integration"
        Category = "Integration Tests"
    },
    @{
        Name = "BulletConversionTest.exe"
        Description = "Math type conversion between engine and Bullet"
        Category = "Integration Tests"
    },
    @{
        Name = "BulletUtilsSimpleTest.exe"
        Description = "Simple Bullet utilities integration test"
        Category = "Integration Tests"
    },
    @{
        Name = "CollisionShapeFactorySimpleTest.exe"
        Description = "Simple collision shape factory integration test"
        Category = "Integration Tests"
    },
    @{
        Name = "PhysicsQueriesTest.exe"
        Description = "Physics queries (raycast, overlap, sweep)"
        Category = "Integration Tests"
    },
    @{
        Name = "PhysicsConfigurationTest.exe"
        Description = "Physics configuration and parameter management"
        Category = "Integration Tests"
    },
    @{
        Name = "MovementComponentComparisonTest.exe"
        Description = "Character movement component comparison"
        Category = "Integration Tests"
    },
    @{
        Name = "PhysicsPerformanceSimpleTest.exe"
        Description = "Physics performance benchmarking"
        Category = "Performance Tests"
    },
    @{
        Name = "MemoryUsageSimpleTest.exe"
        Description = "Memory usage and leak detection"
        Category = "Performance Tests"
    },
    @{
        Name = "CharacterBehaviorSimpleTest.exe"
        Description = "Character physics behavior validation"
        Category = "Integration Tests"
    },
    @{
        Name = "PhysicsDebugDrawerTest.exe"
        Description = "Physics debugging and visualization"
        Category = "Debug Tests"
    }
)

# Filter tests if specified
if ($Filter -ne "*") {
    $testSuite = $testSuite | Where-Object { $_.Name -like "*$Filter*" }
}

$totalTests = $testSuite.Count
$passedTests = 0
$failedTests = 0
$skippedTests = 0
$testResults = @()

Write-Host "`nRunning $totalTests physics tests..." -ForegroundColor Yellow
Write-Host "Filter: $Filter" -ForegroundColor Gray

# Group tests by category
$categories = $testSuite | Group-Object Category

foreach ($category in $categories) {
    Write-Host "`n--- $($category.Name) ---" -ForegroundColor Magenta
    
    foreach ($test in $category.Group) {
        $testPath = "build\Release\$($test.Name)"
        $testName = $test.Name -replace '\.exe$', ''
        
        Write-Host "  Running $testName..." -NoNewline
        
        if (-not (Test-Path $testPath)) {
            Write-Host " SKIPPED (not found)" -ForegroundColor Yellow
            $skippedTests++
            $testResults += @{
                Name = $testName
                Status = "SKIPPED"
                Reason = "Executable not found"
                Duration = 0
            }
            continue
        }
        
        # Measure test execution time
        $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
        
        # Run the test
        if ($Verbose) {
            Write-Host ""
            & $testPath --gtest_verbose
        } else {
            & $testPath 2>$null
        }
        
        $stopwatch.Stop()
        $duration = $stopwatch.ElapsedMilliseconds
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host " PASSED" -ForegroundColor Green -NoNewline
            Write-Host " ($($duration)ms)" -ForegroundColor Gray
            $passedTests++
            $testResults += @{
                Name = $testName
                Status = "PASSED"
                Reason = ""
                Duration = $duration
            }
        } else {
            Write-Host " FAILED" -ForegroundColor Red -NoNewline
            Write-Host " ($($duration)ms)" -ForegroundColor Gray
            $failedTests++
            $testResults += @{
                Name = $testName
                Status = "FAILED"
                Reason = "Test execution failed"
                Duration = $duration
            }
            
            if ($Verbose) {
                Write-Host "    Re-running with verbose output..." -ForegroundColor Yellow
                & $testPath --gtest_verbose
            }
        }
    }
}

# Summary
Write-Host "`n" + "="*50 -ForegroundColor Cyan
Write-Host "TEST SUMMARY" -ForegroundColor Cyan
Write-Host "="*50 -ForegroundColor Cyan

Write-Host "Total Tests:  $totalTests" -ForegroundColor White
Write-Host "Passed:       $passedTests" -ForegroundColor Green
Write-Host "Failed:       $failedTests" -ForegroundColor Red
Write-Host "Skipped:      $skippedTests" -ForegroundColor Yellow

# Calculate success rate
if ($totalTests -gt 0) {
    $successRate = [math]::Round(($passedTests / ($totalTests - $skippedTests)) * 100, 1)
    Write-Host "Success Rate: $successRate%" -ForegroundColor $(if ($successRate -eq 100) { "Green" } elseif ($successRate -ge 80) { "Yellow" } else { "Red" })
}

# Performance summary
$totalDuration = ($testResults | Measure-Object Duration -Sum).Sum
Write-Host "Total Time:   $($totalDuration)ms" -ForegroundColor Gray

# Detailed results if there were failures
if ($failedTests -gt 0) {
    Write-Host "`nFAILED TESTS:" -ForegroundColor Red
    $testResults | Where-Object { $_.Status -eq "FAILED" } | ForEach-Object {
        Write-Host "  - $($_.Name): $($_.Reason)" -ForegroundColor Red
    }
}

# Performance insights
if ($passedTests -gt 0) {
    Write-Host "`nPERFORMANCE INSIGHTS:" -ForegroundColor Cyan
    $slowestTest = $testResults | Where-Object { $_.Status -eq "PASSED" } | Sort-Object Duration -Descending | Select-Object -First 1
    $fastestTest = $testResults | Where-Object { $_.Status -eq "PASSED" } | Sort-Object Duration | Select-Object -First 1
    
    Write-Host "  Slowest: $($slowestTest.Name) ($($slowestTest.Duration)ms)" -ForegroundColor Yellow
    Write-Host "  Fastest: $($fastestTest.Name) ($($fastestTest.Duration)ms)" -ForegroundColor Green
    
    $avgDuration = [math]::Round(($testResults | Where-Object { $_.Status -eq "PASSED" } | Measure-Object Duration -Average).Average, 1)
    Write-Host "  Average: $($avgDuration)ms" -ForegroundColor Gray
}

# Recommendations
Write-Host "`nRECOMMENDATIONS:" -ForegroundColor Cyan
if ($skippedTests -gt 0) {
    Write-Host "  - Run '.\build.bat' to build missing test executables" -ForegroundColor Yellow
}
if ($failedTests -gt 0) {
    Write-Host "  - Run with -Verbose flag to see detailed failure information" -ForegroundColor Yellow
    Write-Host "  - Check individual test logs for specific error details" -ForegroundColor Yellow
}
if ($passedTests -eq ($totalTests - $skippedTests) -and $totalTests -gt 0) {
    Write-Host "  - All tests passed! Physics integration is working correctly." -ForegroundColor Green
    Write-Host "  - Consider running performance tests regularly to track regressions." -ForegroundColor Green
}

# Exit with appropriate code
if ($failedTests -gt 0) {
    Write-Host "`nTest suite completed with failures." -ForegroundColor Red
    exit 1
} elseif ($skippedTests -eq $totalTests) {
    Write-Host "`nNo tests were executed. Build the project first." -ForegroundColor Yellow
    exit 2
} else {
    Write-Host "`nAll tests completed successfully!" -ForegroundColor Green
    exit 0
}