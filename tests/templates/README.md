# Test Templates for Game Engine Kiro

This directory contains template files for creating new tests in the Game Engine Kiro testing system. These templates follow the established coding standards and provide a consistent starting point for new test development.

## Available Templates

### 1. Unit Test Template (`unit_test_template.cpp`)

Use this template for testing individual components in isolation.

**Usage:**

1. Copy `unit_test_template.cpp` to `tests/unit/test_[component].cpp`
2. Replace all `[COMPONENT]` placeholders with your component name (uppercase)
3. Replace all `[Component]` placeholders with PascalCase component name
4. Add the appropriate header include for your component
5. Implement the test functions according to your component's functionality
6. Update requirement references in comments
7. Build and run your tests

**Example:**

```bash
# Copy template
cp tests/templates/unit_test_template.cpp tests/unit/test_logger.cpp

# Edit the file to replace placeholders:
# [COMPONENT] -> LOGGER
# [Component] -> Logger
# Add: #include "Core/Logger.h"
```

### 2. Integration Test Template (`integration_test_template.cpp`)

Use this template for testing interactions between multiple components or systems.

**Usage:**

1. Copy `integration_test_template.cpp` to `tests/integration/test_[system]_[feature].cpp`
2. Replace `[SYSTEM]` and `[FEATURE]` placeholders appropriately
3. Add necessary component headers
4. Implement test functions that validate component interactions
5. Update requirement references in comments
6. Build and run your tests

**Example:**

```bash
# Copy template
cp tests/templates/integration_test_template.cpp tests/integration/test_physics_rendering.cpp

# Edit the file to replace placeholders:
# [SYSTEM] -> PHYSICS_RENDERING
# Add necessary headers for physics and rendering components
```

### 3. Performance Test Template (`performance_test_template.cpp`)

Use this template for testing performance characteristics and benchmarking.

**Usage:**

1. Copy `performance_test_template.cpp` to `tests/performance/test_[component]_performance.cpp`
2. Replace `[COMPONENT]` and `[Component]` placeholders
3. Add the appropriate header include for your component
4. Implement performance test functions with appropriate thresholds
5. Update requirement references in comments
6. Build and run your tests (note: performance tests usually disable coverage)

**Example:**

```bash
# Create performance directory if it doesn't exist
mkdir -p tests/performance

# Copy template
cp tests/templates/performance_test_template.cpp tests/performance/test_math_performance.cpp

# Edit the file to replace placeholders:
# [COMPONENT] -> MATH
# [Component] -> Math
# Add: #include "Core/Math.h"
```

## Template Features

All templates include:

- **Standardized Output Formatting**: Uses `TestOutput` utilities for consistent formatting
- **Error Handling**: Comprehensive exception handling with proper error reporting
- **Performance Timing**: Built-in timing utilities for performance measurement
- **Test Suite Integration**: Uses `TestSuite` class for result tracking and reporting
- **Cross-Platform Support**: Platform-specific test sections where needed
- **Helper Macros**: Convenient assertion macros for common validations
- **Documentation**: Inline comments explaining each section

## Customization Guidelines

### Adding New Test Functions

When adding new test functions to any template:

1. **Follow Naming Convention**: Use `Test[FeatureName]()` pattern
2. **Use Standard Output**: Always use `TestOutput::PrintTestStart()` and `TestOutput::PrintTestPass()`
3. **Include Error Handling**: Wrap complex operations in try-catch blocks
4. **Add to Main Function**: Register the test with the `TestSuite`
5. **Document Requirements**: Add requirement references in function comments

### Modifying Performance Thresholds

Performance test thresholds should be:

- **Realistic**: Based on actual performance requirements
- **Platform-Aware**: Consider different performance characteristics across platforms
- **Maintainable**: Easy to update as performance improves
- **Documented**: Include rationale for threshold values

### Adding Platform-Specific Tests

For platform-specific functionality:

```cpp
#ifdef _WIN32
    // Windows-specific test code
    TestOutput::PrintInfo("Running Windows-specific validation");
#elif defined(__linux__)
    // Linux-specific test code
    TestOutput::PrintInfo("Running Linux-specific validation");
#elif defined(__APPLE__)
    // macOS-specific test code
    TestOutput::PrintInfo("Running macOS-specific validation");
#endif
```

## Build Integration

Tests created from these templates will be automatically discovered and built by the CMake system:

- **Unit Tests**: Automatically discovered from `tests/unit/test_*.cpp`
- **Integration Tests**: Automatically discovered from `tests/integration/test_*.cpp`
- **Performance Tests**: Automatically discovered from `tests/performance/test_*.cpp`

The build system will:

1. Create appropriately named executables
2. Link with the engine library
3. Apply coverage settings (except for performance tests)
4. Include test utilities
5. Add platform-specific dependencies as needed

## Best Practices

### Test Organization

- **One Component Per Unit Test**: Each unit test should focus on a single component
- **Logical Integration Grouping**: Group related components in integration tests
- **Comprehensive Performance Coverage**: Include various performance aspects in performance tests

### Test Implementation

- **Clear Test Names**: Use descriptive names that explain what is being tested
- **Isolated Tests**: Each test function should be independent
- **Comprehensive Coverage**: Test normal cases, edge cases, and error conditions
- **Performance Awareness**: Consider performance impact of test code itself

### Documentation

- **Requirement Traceability**: Always reference requirements in test comments
- **Clear Comments**: Explain complex test logic and expected behaviors
- **Update Documentation**: Keep template documentation current with changes

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure all necessary headers are included and component exists
2. **Linking Errors**: Verify component is properly linked in CMakeLists.txt
3. **Test Failures**: Check that component interface matches test expectations
4. **Performance Issues**: Adjust thresholds based on actual system performance

### Getting Help

- Review existing tests in `tests/unit/` and `tests/integration/` for examples
- Check the testing standards documentation in `docs/testing-standards.md`
- Refer to the main project documentation for component interfaces

## Contributing

When contributing new templates or improvements:

1. Follow the established coding standards
2. Test templates on multiple platforms
3. Update this documentation
4. Ensure backward compatibility
5. Add appropriate examples
