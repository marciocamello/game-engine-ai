#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace GameEngine {
namespace Testing {

/**
 * Test categories for organizing and filtering tests
 */
enum class TestCategory {
    Unit,
    Integration,
    Performance,
    All
};

/**
 * Test execution result
 */
struct TestExecutionResult {
    std::string testName;
    std::string category;
    bool passed;
    double executionTimeMs;
    std::string errorMessage;
    
    TestExecutionResult(const std::string& name, const std::string& cat, bool success, 
                       double time = 0.0, const std::string& error = "")
        : testName(name), category(cat), passed(success), executionTimeMs(time), errorMessage(error) {}
};

/**
 * Test configuration for controlling test execution
 */
struct TestConfig {
    // Test categories to enable/disable
    std::unordered_map<std::string, bool> enabledCategories;
    
    // Test execution settings
    bool enablePerformanceTests = true;
    bool enableIntegrationTests = true;
    bool enableUnitTests = true;
    
    // Output settings
    bool verboseOutput = false;
    bool showTimings = true;
    std::string outputFormat = "standard"; // "standard", "json", "xml"
    
    // Performance test settings
    double performanceTimeoutMs = 30000.0; // 30 seconds
    int performanceIterations = 1000;
    
    // Test discovery settings
    std::vector<std::string> testDirectories;
    std::vector<std::string> excludePatterns;
    
    TestConfig() {
        // Default enabled categories
        enabledCategories["unit"] = true;
        enabledCategories["integration"] = true;
        enabledCategories["performance"] = true;
        
        // Default test directories
        testDirectories.push_back("tests/unit");
        testDirectories.push_back("tests/integration");
        testDirectories.push_back("projects/Tests/unit");
        testDirectories.push_back("projects/Tests/integration");
    }
};

/**
 * Test discovery interface for finding tests in different locations
 */
class ITestDiscovery {
public:
    virtual ~ITestDiscovery() = default;
    
    /**
     * Discover tests in the specified directory
     */
    virtual std::vector<std::string> DiscoverTests(const std::string& directory, TestCategory category) = 0;
    
    /**
     * Get test category from test name or path
     */
    virtual TestCategory GetTestCategory(const std::string& testPath) = 0;
    
    /**
     * Check if test should be included based on configuration
     */
    virtual bool ShouldIncludeTest(const std::string& testPath, const TestConfig& config) = 0;
};

/**
 * Test executor interface for running discovered tests
 */
class ITestExecutor {
public:
    virtual ~ITestExecutor() = default;
    
    /**
     * Execute a single test
     */
    virtual TestExecutionResult ExecuteTest(const std::string& testPath, const TestConfig& config) = 0;
    
    /**
     * Execute multiple tests
     */
    virtual std::vector<TestExecutionResult> ExecuteTests(const std::vector<std::string>& testPaths, const TestConfig& config) = 0;
};

/**
 * Test framework interface for managing the entire test system
 */
class ITestFramework {
public:
    virtual ~ITestFramework() = default;
    
    /**
     * Load test configuration from file
     */
    virtual bool LoadConfig(const std::string& configPath) = 0;
    
    /**
     * Save test configuration to file
     */
    virtual bool SaveConfig(const std::string& configPath) const = 0;
    
    /**
     * Get current test configuration
     */
    virtual const TestConfig& GetConfig() const = 0;
    
    /**
     * Set test configuration
     */
    virtual void SetConfig(const TestConfig& config) = 0;
    
    /**
     * Discover all tests based on current configuration
     */
    virtual std::vector<std::string> DiscoverAllTests() = 0;
    
    /**
     * Execute all discovered tests
     */
    virtual std::vector<TestExecutionResult> ExecuteAllTests() = 0;
    
    /**
     * Execute tests by category
     */
    virtual std::vector<TestExecutionResult> ExecuteTestsByCategory(TestCategory category) = 0;
    
    /**
     * Generate test report
     */
    virtual void GenerateReport(const std::vector<TestExecutionResult>& results, const std::string& outputPath = "") = 0;
};

/**
 * Utility functions for test framework
 */
class TestFrameworkUtils {
public:
    /**
     * Convert test category enum to string
     */
    static std::string CategoryToString(TestCategory category);
    
    /**
     * Convert string to test category enum
     */
    static TestCategory StringToCategory(const std::string& categoryStr);
    
    /**
     * Check if test executable exists
     */
    static bool TestExecutableExists(const std::string& testPath);
    
    /**
     * Get test executable path from source path
     */
    static std::string GetExecutablePath(const std::string& sourcePath);
    
    /**
     * Extract test name from file path
     */
    static std::string ExtractTestName(const std::string& filePath);
    
    /**
     * Format test execution time
     */
    static std::string FormatExecutionTime(double timeMs);
};

/**
 * Get the global test framework instance
 */
ITestFramework& GetTestFramework();

} // namespace Testing
} // namespace GameEngine