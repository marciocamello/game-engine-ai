#include "../interfaces/TestFramework.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace GameEngine::Testing;

/**
 * Enhanced test runner that supports the dual architecture
 */
class EnhancedTestRunner {
private:
    ITestFramework& m_framework;
    
public:
    EnhancedTestRunner() : m_framework(GetTestFramework()) {}
    
    /**
     * Run all tests with current configuration
     */
    int RunAllTests() {
        std::cout << "========================================" << std::endl;
        std::cout << " Game Engine Kiro - Enhanced Test Runner" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Load configuration
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, using defaults" << std::endl;
        }
        
        // Discover and execute all tests
        auto results = m_framework.ExecuteAllTests();
        
        // Generate report
        m_framework.GenerateReport(results);
        
        // Return appropriate exit code
        bool allPassed = true;
        for (const auto& result : results) {
            if (!result.passed) {
                allPassed = false;
                break;
            }
        }
        
        return allPassed ? 0 : 1;
    }
    
    /**
     * Run tests by category
     */
    int RunTestsByCategory(const std::string& categoryStr) {
        TestCategory category = TestFrameworkUtils::StringToCategory(categoryStr);
        
        std::cout << "========================================" << std::endl;
        std::cout << " Running " << categoryStr << " tests" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Load configuration
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, using defaults" << std::endl;
        }
        
        // Execute tests by category
        auto results = m_framework.ExecuteTestsByCategory(category);
        
        // Generate report
        m_framework.GenerateReport(results);
        
        // Return appropriate exit code
        bool allPassed = true;
        for (const auto& result : results) {
            if (!result.passed) {
                allPassed = false;
                break;
            }
        }
        
        return allPassed ? 0 : 1;
    }
    
    /**
     * List all discovered tests
     */
    void ListTests() {
        std::cout << "========================================" << std::endl;
        std::cout << " Discovered Tests" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Load configuration
        std::string configPath = "projects/Tests/config/test_config.json";
        if (!m_framework.LoadConfig(configPath)) {
            std::cout << "[WARNING] Could not load test config, using defaults" << std::endl;
        }
        
        auto tests = m_framework.DiscoverAllTests();
        
        // Group by category
        std::vector<std::string> unitTests, integrationTests, performanceTests;
        
        // Create discovery instance to categorize tests
        class DefaultTestDiscovery : public ITestDiscovery {
        public:
            std::vector<std::string> DiscoverTests(const std::string&, TestCategory) override { return {}; }
            TestCategory GetTestCategory(const std::string& testPath) override {
                std::string path = testPath;
                std::replace(path.begin(), path.end(), '\\', '/');
                
                if (path.find("/unit/") != std::string::npos) {
                    return TestCategory::Unit;
                } else if (path.find("/integration/") != std::string::npos) {
                    return TestCategory::Integration;
                } else if (path.find("/performance/") != std::string::npos) {
                    return TestCategory::Performance;
                }
                return TestCategory::Unit;
            }
            bool ShouldIncludeTest(const std::string&, const TestConfig&) override { return true; }
        };
        
        DefaultTestDiscovery discovery;
        for (const auto& test : tests) {
            TestCategory category = discovery.GetTestCategory(test);
            switch (category) {
                case TestCategory::Unit:
                    unitTests.push_back(test);
                    break;
                case TestCategory::Integration:
                    integrationTests.push_back(test);
                    break;
                case TestCategory::Performance:
                    performanceTests.push_back(test);
                    break;
                default:
                    break;
            }
        }
        
        std::cout << std::endl << "Unit Tests (" << unitTests.size() << "):" << std::endl;
        for (const auto& test : unitTests) {
            std::cout << "  " << TestFrameworkUtils::ExtractTestName(test) << std::endl;
        }
        
        std::cout << std::endl << "Integration Tests (" << integrationTests.size() << "):" << std::endl;
        for (const auto& test : integrationTests) {
            std::cout << "  " << TestFrameworkUtils::ExtractTestName(test) << std::endl;
        }
        
        std::cout << std::endl << "Performance Tests (" << performanceTests.size() << "):" << std::endl;
        for (const auto& test : performanceTests) {
            std::cout << "  " << TestFrameworkUtils::ExtractTestName(test) << std::endl;
        }
        
        std::cout << std::endl << "Total: " << tests.size() << " tests" << std::endl;
        std::cout << "========================================" << std::endl;
    }
    
    /**
     * Show help information
     */
    void ShowHelp() {
        std::cout << "Enhanced Test Runner - Usage:" << std::endl;
        std::cout << "  --all                Run all tests" << std::endl;
        std::cout << "  --unit               Run unit tests only" << std::endl;
        std::cout << "  --integration        Run integration tests only" << std::endl;
        std::cout << "  --performance        Run performance tests only" << std::endl;
        std::cout << "  --list               List all discovered tests" << std::endl;
        std::cout << "  --help               Show this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "Configuration file: projects/Tests/config/test_config.json" << std::endl;
    }
};

/**
 * Main entry point for enhanced test runner
 */
int main(int argc, char* argv[]) {
    EnhancedTestRunner runner;
    
    if (argc < 2) {
        // Default behavior - run all tests
        return runner.RunAllTests();
    }
    
    std::string command = argv[1];
    
    if (command == "--all") {
        return runner.RunAllTests();
    } else if (command == "--unit") {
        return runner.RunTestsByCategory("unit");
    } else if (command == "--integration") {
        return runner.RunTestsByCategory("integration");
    } else if (command == "--performance") {
        return runner.RunTestsByCategory("performance");
    } else if (command == "--list") {
        runner.ListTests();
        return 0;
    } else if (command == "--help") {
        runner.ShowHelp();
        return 0;
    } else {
        std::cerr << "[ERROR] Unknown command: " << command << std::endl;
        runner.ShowHelp();
        return 1;
    }
}