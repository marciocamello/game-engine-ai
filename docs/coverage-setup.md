# Test Coverage Analysis Setup

This document describes how to set up and use the automated test coverage analysis system for Game Engine Kiro.

## Prerequisites

### Windows (Primary Platform)

1. **Install OpenCppCoverage**

   - Download from: https://github.com/OpenCppCoverage/OpenCppCoverage/releases
   - Or install via Chocolatey: `choco install opencppcoverage`
   - Or install via winget: `winget install OpenCppCoverage.OpenCppCoverage`

2. **Verify Installation**
   ```cmd
   OpenCppCoverage.exe --help
   ```

### Linux/macOS (Future Support)

Coverage analysis for Linux/macOS will use gcov/lcov and is planned for future implementation.

## Usage

### 1. Build with Coverage Support

```cmd
# Build project with coverage-enabled debug symbols
.\build_coverage.bat
```

This will:

- Configure CMake with `ENABLE_COVERAGE=ON`
- Build in Debug mode with debug symbols
- Apply coverage settings to all test executables

### 2. Run Coverage Analysis

```powershell
# Basic coverage analysis
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat

# With verbose output
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -Verbose

# Build first, then analyze
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -BuildFirst

# Custom thresholds
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -LineCoverageThreshold 95.0 -BranchCoverageThreshold 90.0

# Open HTML report automatically
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -OpenReport
```

### 3. Coverage Reports

The script generates multiple report formats:

- **HTML Report**: `coverage_reports/coverage_YYYYMMDD_HHMMSS/index.html`
- **XML Report**: `coverage_reports/coverage_YYYYMMDD_HHMMSS/coverage.xml`
- **JSON Summary**: `coverage_reports/coverage_YYYYMMDD_HHMMSS/coverage.json`

### 4. Coverage Configuration

Edit `coverage_config.json` to customize:

- Source paths to include/exclude
- Test executables to run
- Coverage thresholds
- Report formats
- Quality gates

## Integration with Development Workflow

### Continuous Integration

Add to your CI pipeline:

```yaml
- name: Run Coverage Analysis
  run: |
    .\build_coverage.bat
    powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat
  shell: cmd
```

### Pre-commit Hooks

Add coverage validation to pre-commit hooks:

```bash
#!/bin/bash
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -LineCoverageThreshold 100.0
```

### IDE Integration

#### Visual Studio Code

Add to `.vscode/tasks.json`:

```json
{
  "label": "Run Coverage Analysis",
  "type": "shell",
  "command": "powershell",
  "args": [
    "-ExecutionPolicy",
    "Bypass",
    "-File",
    ".scripts\run_coverage_analysis.bat",
    "-OpenReport"
  ],
  "group": "test",
  "presentation": {
    "echo": true,
    "reveal": "always",
    "focus": false,
    "panel": "shared"
  }
}
```

## Coverage Targets

### Current Targets

- **Line Coverage**: 100% (all executable lines covered)
- **Branch Coverage**: 95% (all conditional branches covered)

### Test Categories

1. **Integration Tests** (High Priority)

   - `BulletIntegrationTest.exe`
   - `CollisionShapeFactoryTest.exe`

2. **Integration Tests** (High Priority)

   - `BulletIntegrationTest.exe`
   - `BulletConversionTest.exe`
   - `PhysicsQueriesTest.exe`
   - `MovementComponentComparisonTest.exe`
   - `CharacterBehaviorSimpleTest.exe`

3. **Simple Tests** (Medium Priority)

   - `BulletUtilsSimpleTest.exe`
   - `CollisionShapeFactorySimpleTest.exe`
   - `PhysicsConfigurationTest.exe`

4. **Debug Tests** (Low Priority)
   - `PhysicsDebugDrawerTest.exe`

## Troubleshooting

### Common Issues

1. **OpenCppCoverage not found**

   ```
   ERROR: OpenCppCoverage not found!
   ```

   **Solution**: Install OpenCppCoverage using one of the methods above.

2. **No test executables found**

   ```
   ERROR: No test executables found for coverage analysis!
   ```

   **Solution**: Run `.\build_coverage.bat` first to build test executables.

3. **Coverage analysis failed**
   ```
   Coverage analysis failed with exit code: 1
   ```
   **Solution**: Check that all test executables run successfully individually.

### Debug Mode

Run with verbose output to see detailed execution:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -Verbose
```

### Manual Testing

Test individual components:

```cmd
# Test a single executable
build\Debug\BulletUtilsTest.exe

# Test coverage tool
OpenCppCoverage.exe --help

# Check build directory
dir build\Debug\*.exe
```

## Advanced Usage

### Custom Test Selection

Filter specific tests:

```powershell
# Run only Bullet-related tests
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -Filter "Bullet*"

# Run only integration tests
powershell -ExecutionPolicy Bypass -File .\scripts\run_coverage_analysis.bat -Filter "*Test.exe"
```

### Baseline Comparison

The system automatically creates and compares against a baseline:

- First run creates `coverage_reports/coverage_baseline.json`
- Subsequent runs compare against baseline
- Regression detection alerts on significant coverage drops

### Performance Monitoring

Coverage analysis includes performance metrics:

- Analysis execution time
- Test execution time
- Memory usage during analysis
- Report generation time

## Files Created

The coverage system creates these files:

- `.\scripts\run_coverage_analysis.bat` - Main coverage analysis script
- `build_coverage.bat` - Coverage-enabled build script
- `coverage_config.json` - Configuration file
- `coverage_baseline.json` - Baseline coverage metrics
- `include/Core/TestCoverageManager.h` - Coverage manager header
- `src/Core/TestCoverageManager.cpp` - Coverage manager implementation
- `coverage_reports/` - Directory for coverage reports

## Next Steps

After setting up coverage analysis:

1. Install OpenCppCoverage
2. Run initial coverage analysis
3. Review HTML report to identify uncovered code
4. Add tests to improve coverage
5. Integrate into CI/CD pipeline
6. Set up automated coverage monitoring
