# Game Engine Kiro - Test Coverage Analysis Script
# This script runs comprehensive test coverage analysis using OpenCppCoverage

param(
    [switch]$BuildFirst,
    [switch]$Verbose,
    [string]$OutputDir = "coverage_reports",
    [string]$Filter = "*",
    [float]$LineCoverageThreshold = 100.0,
    [float]$BranchCoverageThreshold = 95.0,
    [switch]$OpenReport
)

Write-Host "Game Engine Kiro - Test Coverage Analysis" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Check if OpenCppCoverage is installed
$openCppCoverage = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
if (-not $openCppCoverage) {
    Write-Host "ERROR: OpenCppCoverage not found!" -ForegroundColor Red
    Write-Host "Please install OpenCppCoverage from: https://github.com/OpenCppCoverage/OpenCppCoverage/releases" -ForegroundColor Yellow
    Write-Host "Or install via Chocolatey: choco install opencppcoverage" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found OpenCppCoverage: $($openCppCoverage.Source)" -ForegroundColor Green

# Build project first if requested
if ($BuildFirst) {
    Write-Host "`nBuilding project with coverage support..." -ForegroundColor Yellow
    & .\build.bat
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed! Exiting." -ForegroundColor Red
        exit 1
    }
    Write-Host "Build completed successfully." -ForegroundColor Green
}

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    Write-Host "Created coverage reports directory: $OutputDir" -ForegroundColor Green
}

# Define test executables for coverage analysis
$testExecutables = @(
    @{
        Name        = "BulletUtilsTest.exe"
        Description = "Bullet Physics utility functions"
        Type        = "Unit"
    },
    @{
        Name        = "CollisionShapeFactoryTest.exe"
        Description = "Collision shape factory"
        Type        = "Unit"
    },
    @{
        Name        = "PhysicsEngineTest.exe"
        Description = "Physics engine core functionality"
        Type        = "Unit"
    },
    @{
        Name        = "BulletPhysicsWorldTest.exe"
        Description = "Bullet physics world wrapper"
        Type        = "Unit"
    },
    @{
        Name        = "BulletIntegrationTest.exe"
        Description = "Bullet Physics integration"
        Type        = "Integration"
    },
    @{
        Name        = "BulletConversionTest.exe"
        Description = "Math type conversions"
        Type        = "Integration"
    },
    @{
        Name        = "BulletUtilsSimpleTest.exe"
        Description = "Simple Bullet utilities"
        Type        = "Integration"
    },
    @{
        Name        = "CollisionShapeFactorySimpleTest.exe"
        Description = "Simple collision shape factory"
        Type        = "Integration"
    },
    @{
        Name        = "PhysicsQueriesTest.exe"
        Description = "Physics queries"
        Type        = "Integration"
    },
    @{
        Name        = "PhysicsConfigurationTest.exe"
        Description = "Physics configuration"
        Type        = "Integration"
    },
    @{
        Name        = "MovementComponentComparisonTest.exe"
        Description = "Movement component comparison"
        Type        = "Integration"
    },
    @{
        Name        = "CharacterBehaviorSimpleTest.exe"
        Description = "Character behavior"
        Type        = "Integration"
    },
    @{
        Name        = "PhysicsDebugDrawerTest.exe"
        Description = "Physics debug drawer"
        Type        = "Debug"
    }
)

# Filter tests if specified
if ($Filter -ne "*") {
    $testExecutables = $testExecutables | Where-Object { $_.Name -like "*$Filter*" }
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportDir = Join-Path $OutputDir "coverage_$timestamp"
$htmlReportPath = Join-Path $reportDir "index.html"
$xmlReportPath = Join-Path $reportDir "coverage.xml"
$jsonReportPath = Join-Path $reportDir "coverage.json"

Write-Host "`nRunning coverage analysis for $($testExecutables.Count) test executables..." -ForegroundColor Yellow
Write-Host "Output directory: $reportDir" -ForegroundColor Gray

# Prepare OpenCppCoverage arguments
$coverageArgs = @(
    "--verbose"
    "--sources", "src"
    "--sources", "include"
    "--excluded_sources", "vcpkg"
    "--excluded_sources", "build"
    "--excluded_sources", "tests"
    "--excluded_sources", "examples"
    "--export_type", "html:$reportDir"
    "--export_type", "cobertura:$xmlReportPath"
    "--working_dir", "."
)

# Add module specifications for each test executable
$availableTests = @()
foreach ($test in $testExecutables) {
    $testPath = "build\Release\$($test.Name)"
    if (Test-Path $testPath) {
        $availableTests += $test
        $coverageArgs += "--modules"
        $coverageArgs += $testPath
    }
    else {
        Write-Host "  Skipping $($test.Name) (not found)" -ForegroundColor Yellow
    }
}

if ($availableTests.Count -eq 0) {
    Write-Host "ERROR: No test executables found! Run build first." -ForegroundColor Red
    exit 1
}

Write-Host "Found $($availableTests.Count) test executables for coverage analysis" -ForegroundColor Green

# Create a combined test runner script
$testRunnerScript = @"
@echo off
echo Running combined test suite for coverage analysis...
"@

foreach ($test in $availableTests) {
    $testRunnerScript += "`necho Running $($test.Name)...`n"
    $testRunnerScript += "build\Release\$($test.Name)`n"
    $testRunnerScript += "if %ERRORLEVEL% neq 0 echo WARNING: $($test.Name) failed but continuing...`n"
}

$testRunnerScript += "`necho All tests completed.`n"

$testRunnerPath = "temp_coverage_runner.bat"
$testRunnerScript | Out-File -FilePath $testRunnerPath -Encoding ASCII

try {
    # Run coverage analysis
    Write-Host "`nExecuting coverage analysis..." -ForegroundColor Yellow
    
    $coverageCommand = $coverageArgs + @("--", $testRunnerPath)
    
    if ($Verbose) {
        Write-Host "OpenCppCoverage command:" -ForegroundColor Gray
        Write-Host "OpenCppCoverage.exe $($coverageCommand -join ' ')" -ForegroundColor Gray
    }
    
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    & OpenCppCoverage.exe @coverageCommand
    $stopwatch.Stop()
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Coverage analysis failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Coverage analysis completed in $($stopwatch.ElapsedMilliseconds)ms" -ForegroundColor Green
    
}
finally {
    # Clean up temporary files
    if (Test-Path $testRunnerPath) {
        Remove-Item $testRunnerPath -Force
    }
}

# Parse coverage results from XML report
if (Test-Path $xmlReportPath) {
    Write-Host "`nParsing coverage results..." -ForegroundColor Yellow
    
    try {
        [xml]$coverageXml = Get-Content $xmlReportPath
        
        # Extract overall coverage metrics
        $coverage = $coverageXml.coverage
        $lineRate = [float]$coverage.'line-rate' * 100
        $branchRate = [float]$coverage.'branch-rate' * 100
        $linesValid = [int]$coverage.'lines-valid'
        $linesCovered = [int]$coverage.'lines-covered'
        $branchesValid = [int]$coverage.'branches-valid'
        $branchesCovered = [int]$coverage.'branches-covered'
        
        # Create coverage summary
        $coverageSummary = @{
            Timestamp       = $timestamp
            LineCoverage    = $lineRate
            BranchCoverage  = $branchRate
            LinesValid      = $linesValid
            LinesCovered    = $linesCovered
            BranchesValid   = $branchesValid
            BranchesCovered = $branchesCovered
            TestsRun        = $availableTests.Count
            ReportPath      = $htmlReportPath
        }
        
        # Save coverage summary as JSON
        $coverageSummary | ConvertTo-Json -Depth 3 | Out-File -FilePath $jsonReportPath -Encoding UTF8
        
        # Display results
        Write-Host "`n" + "="*60 -ForegroundColor Cyan
        Write-Host "COVERAGE ANALYSIS RESULTS" -ForegroundColor Cyan
        Write-Host "="*60 -ForegroundColor Cyan
        
        Write-Host "Line Coverage:   $($lineRate.ToString('F1'))% ($linesCovered/$linesValid lines)" -ForegroundColor $(if ($lineRate -ge $LineCoverageThreshold) { "Green" } elseif ($lineRate -ge 80) { "Yellow" } else { "Red" })
        Write-Host "Branch Coverage: $($branchRate.ToString('F1'))% ($branchesCovered/$branchesValid branches)" -ForegroundColor $(if ($branchRate -ge $BranchCoverageThreshold) { "Green" } elseif ($branchRate -ge 80) { "Yellow" } else { "Red" })
        Write-Host "Tests Executed:  $($availableTests.Count)" -ForegroundColor White
        Write-Host "Analysis Time:   $($stopwatch.ElapsedMilliseconds)ms" -ForegroundColor Gray
        
        # Coverage thresholds check
        Write-Host "`nTHRESHOLD ANALYSIS:" -ForegroundColor Cyan
        $lineThresholdMet = $lineRate -ge $LineCoverageThreshold
        $branchThresholdMet = $branchRate -ge $BranchCoverageThreshold
        
        Write-Host "Line Coverage Threshold ($LineCoverageThreshold%):   $(if ($lineThresholdMet) { 'PASSED' } else { 'FAILED' })" -ForegroundColor $(if ($lineThresholdMet) { "Green" } else { "Red" })
        Write-Host "Branch Coverage Threshold ($BranchCoverageThreshold%): $(if ($branchThresholdMet) { 'PASSED' } else { 'FAILED' })" -ForegroundColor $(if ($branchThresholdMet) { "Green" } else { "Red" })
        
        # File-level coverage details
        Write-Host "`nFILE COVERAGE DETAILS:" -ForegroundColor Cyan
        $packages = $coverageXml.coverage.packages.package
        if ($packages) {
            foreach ($package in $packages) {
                $classes = $package.classes.class
                if ($classes) {
                    foreach ($class in $classes) {
                        $fileName = Split-Path $class.filename -Leaf
                        $classLineRate = [float]$class.'line-rate' * 100
                        $classBranchRate = [float]$class.'branch-rate' * 100
                        
                        $coverageColor = if ($classLineRate -ge 95) { "Green" } elseif ($classLineRate -ge 80) { "Yellow" } else { "Red" }
                        Write-Host "  $fileName`: $($classLineRate.ToString('F1'))% lines, $($classBranchRate.ToString('F1'))% branches" -ForegroundColor $coverageColor
                    }
                }
            }
        }
        
        # Recommendations
        Write-Host "`nRECOMMENDATIONS:" -ForegroundColor Cyan
        if (-not $lineThresholdMet) {
            $missingLines = $linesValid - $linesCovered
            Write-Host "  - Add tests to cover $missingLines uncovered lines" -ForegroundColor Yellow
        }
        if (-not $branchThresholdMet) {
            $missingBranches = $branchesValid - $branchesCovered
            Write-Host "  - Add tests to cover $missingBranches uncovered branches" -ForegroundColor Yellow
        }
        if ($lineThresholdMet -and $branchThresholdMet) {
            Write-Host "  - Excellent coverage! Consider maintaining this level." -ForegroundColor Green
        }
        
        Write-Host "  - Review HTML report for detailed line-by-line coverage: $htmlReportPath" -ForegroundColor Cyan
        
        # Open report if requested
        if ($OpenReport -and (Test-Path $htmlReportPath)) {
            Write-Host "`nOpening coverage report..." -ForegroundColor Yellow
            Start-Process $htmlReportPath
        }
        
        # Create baseline if this is the first run
        $baselinePath = Join-Path $OutputDir "coverage_baseline.json"
        if (-not (Test-Path $baselinePath)) {
            Write-Host "`nCreating coverage baseline..." -ForegroundColor Yellow
            $coverageSummary | ConvertTo-Json -Depth 3 | Out-File -FilePath $baselinePath -Encoding UTF8
            Write-Host "Baseline created: $baselinePath" -ForegroundColor Green
        }
        else {
            # Compare with baseline
            Write-Host "`nCOMPARING WITH BASELINE:" -ForegroundColor Cyan
            try {
                $baseline = Get-Content $baselinePath | ConvertFrom-Json
                $lineDiff = $lineRate - $baseline.LineCoverage
                $branchDiff = $branchRate - $baseline.BranchCoverage
                
                Write-Host "Line Coverage Change:   $(if ($lineDiff -ge 0) { '+' })$($lineDiff.ToString('F1'))%" -ForegroundColor $(if ($lineDiff -ge 0) { "Green" } elseif ($lineDiff -ge -5) { "Yellow" } else { "Red" })
                Write-Host "Branch Coverage Change: $(if ($branchDiff -ge 0) { '+' })$($branchDiff.ToString('F1'))%" -ForegroundColor $(if ($branchDiff -ge 0) { "Green" } elseif ($branchDiff -ge -5) { "Yellow" } else { "Red" })
                
                if ($lineDiff -lt -5 -or $branchDiff -lt -5) {
                    Write-Host "WARNING: Significant coverage regression detected!" -ForegroundColor Red
                }
            }
            catch {
                Write-Host "Could not compare with baseline: $($_.Exception.Message)" -ForegroundColor Yellow
            }
        }
        
        # Exit with appropriate code
        if ($lineThresholdMet -and $branchThresholdMet) {
            Write-Host "`nCoverage analysis completed successfully!" -ForegroundColor Green
            exit 0
        }
        else {
            Write-Host "`nCoverage thresholds not met!" -ForegroundColor Red
            exit 1
        }
        
    }
    catch {
        Write-Host "Error parsing coverage results: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "Raw XML report available at: $xmlReportPath" -ForegroundColor Yellow
        exit 1
    }
}
else {
    Write-Host "ERROR: Coverage XML report not generated!" -ForegroundColor Red
    exit 1
}