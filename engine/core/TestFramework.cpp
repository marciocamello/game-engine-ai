#include "../interfaces/TestFramework.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef GAMEENGINE_HAS_JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

namespace GameEngine {
namespace Testing {

/**
 * Default test discovery implementation
 */
class DefaultTestDiscovery : public ITestDiscovery {
public:
    std::vector<std::string> DiscoverTests(const std::string& directory, TestCategory category) override {
        std::vector<std::string> tests;
        
        if (!std::filesystem::exists(directory)) {
            return tests;
        }
        
        std::string pattern = "test_*.cpp";
        
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find("test_") == 0 && filename.ends_with(".cpp")) {
                        // Check if category matches
                        TestCategory testCat = GetTestCategory(entry.path().string());
                        if (category == TestCategory::All || category == testCat) {
                            tests.push_back(entry.path().string());
                        }
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "[ERROR] Failed to discover tests in " << directory << ": " << e.what() << std::endl;
        }
        
        return tests;
    }
    
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
        
        // Default to unit test if unclear
        return TestCategory::Unit;
    }
    
    bool ShouldIncludeTest(const std::string& testPath, const TestConfig& config) override {
        TestCategory category = GetTestCategory(testPath);
        std::string categoryStr = TestFrameworkUtils::CategoryToString(category);
        
        auto it = config.enabledCategories.find(categoryStr);
        if (it != config.enabledCategories.end()) {
            return it->second;
        }
        
        // Check individual category flags
        switch (category) {
            case TestCategory::Unit:
                return config.enableUnitTests;
            case TestCategory::Integration:
                return config.enableIntegrationTests;
            case TestCategory::Performance:
                return config.enablePerformanceTests;
            default:
                return true;
        }
    }
};

/**
 * Default test executor implementation
 */
class DefaultTestExecutor : public ITestExecutor {
public:
    TestExecutionResult ExecuteTest(const std::string& testPath, const TestConfig& config) override {
        std::string executablePath = TestFrameworkUtils::GetExecutablePath(testPath);
        std::string testName = TestFrameworkUtils::ExtractTestName(testPath);
        std::string category = TestFrameworkUtils::CategoryToString(
            DefaultTestDiscovery().GetTestCategory(testPath));
        
        if (!TestFrameworkUtils::TestExecutableExists(executablePath)) {
            return TestExecutionResult(testName, category, false, 0.0, 
                "Test executable not found: " + executablePath);
        }
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Execute the test
        int exitCode = 1;
        
#ifdef _WIN32
        STARTUPINFOA si = {};
        PROCESS_INFORMATION pi = {};
        si.cb = sizeof(si);
        
        std::string command = executablePath;
        
        if (CreateProcessA(nullptr, const_cast<char*>(command.c_str()), 
                          nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            
            DWORD timeout = static_cast<DWORD>(config.performanceTimeoutMs);
            DWORD waitResult = WaitForSingleObject(pi.hProcess, timeout);
            
            if (waitResult == WAIT_OBJECT_0) {
                DWORD processExitCode;
                if (GetExitCodeProcess(pi.hProcess, &processExitCode)) {
                    exitCode = static_cast<int>(processExitCode);
                }
            } else {
                TerminateProcess(pi.hProcess, 1);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                
                auto endTime = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                
                return TestExecutionResult(testName, category, false, elapsed, "Test timed out");
            }
            
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            return TestExecutionResult(testName, category, false, 0.0, 
                "Failed to start test process");
        }
#else
        // Unix-like systems
        exitCode = system(executablePath.c_str());
        exitCode = WEXITSTATUS(exitCode);
#endif
        
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        
        bool passed = (exitCode == 0);
        std::string errorMsg = passed ? "" : "Test failed with exit code " + std::to_string(exitCode);
        
        return TestExecutionResult(testName, category, passed, elapsed, errorMsg);
    }
    
    std::vector<TestExecutionResult> ExecuteTests(const std::vector<std::string>& testPaths, const TestConfig& config) override {
        std::vector<TestExecutionResult> results;
        results.reserve(testPaths.size());
        
        for (const auto& testPath : testPaths) {
            results.push_back(ExecuteTest(testPath, config));
        }
        
        return results;
    }
};

/**
 * Default test framework implementation
 */
class DefaultTestFramework : public ITestFramework {
private:
    TestConfig m_config;
    std::unique_ptr<ITestDiscovery> m_discovery;
    std::unique_ptr<ITestExecutor> m_executor;
    
public:
    DefaultTestFramework() 
        : m_discovery(std::make_unique<DefaultTestDiscovery>())
        , m_executor(std::make_unique<DefaultTestExecutor>()) {
    }
    
    bool LoadConfig(const std::string& configPath) override {
#ifdef GAMEENGINE_HAS_JSON
        try {
            std::ifstream file(configPath);
            if (!file.is_open()) {
                // Create default config if file doesn't exist
                return SaveConfig(configPath);
            }
            
            json j;
            file >> j;
            
            // Load enabled categories
            if (j.contains("enabledCategories")) {
                m_config.enabledCategories.clear();
                for (auto& [key, value] : j["enabledCategories"].items()) {
                    m_config.enabledCategories[key] = value.get<bool>();
                }
            }
            
            // Load test execution settings
            if (j.contains("enablePerformanceTests")) {
                m_config.enablePerformanceTests = j["enablePerformanceTests"].get<bool>();
            }
            if (j.contains("enableIntegrationTests")) {
                m_config.enableIntegrationTests = j["enableIntegrationTests"].get<bool>();
            }
            if (j.contains("enableUnitTests")) {
                m_config.enableUnitTests = j["enableUnitTests"].get<bool>();
            }
            
            // Load output settings
            if (j.contains("verboseOutput")) {
                m_config.verboseOutput = j["verboseOutput"].get<bool>();
            }
            if (j.contains("showTimings")) {
                m_config.showTimings = j["showTimings"].get<bool>();
            }
            if (j.contains("outputFormat")) {
                m_config.outputFormat = j["outputFormat"].get<std::string>();
            }
            
            // Load performance settings
            if (j.contains("performanceTimeoutMs")) {
                m_config.performanceTimeoutMs = j["performanceTimeoutMs"].get<double>();
            }
            if (j.contains("performanceIterations")) {
                m_config.performanceIterations = j["performanceIterations"].get<int>();
            }
            
            // Load test directories
            if (j.contains("testDirectories")) {
                m_config.testDirectories.clear();
                for (const auto& dir : j["testDirectories"]) {
                    m_config.testDirectories.push_back(dir.get<std::string>());
                }
            }
            
            // Load exclude patterns
            if (j.contains("excludePatterns")) {
                m_config.excludePatterns.clear();
                for (const auto& pattern : j["excludePatterns"]) {
                    m_config.excludePatterns.push_back(pattern.get<std::string>());
                }
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to load test config: " << e.what() << std::endl;
            return false;
        }
#else
        // Fallback without JSON support
        std::cerr << "[WARNING] JSON support not available, using default test configuration" << std::endl;
        return true;
#endif
    }
    
    bool SaveConfig(const std::string& configPath) const override {
#ifdef GAMEENGINE_HAS_JSON
        try {
            json j;
            
            // Save enabled categories
            j["enabledCategories"] = m_config.enabledCategories;
            
            // Save test execution settings
            j["enablePerformanceTests"] = m_config.enablePerformanceTests;
            j["enableIntegrationTests"] = m_config.enableIntegrationTests;
            j["enableUnitTests"] = m_config.enableUnitTests;
            
            // Save output settings
            j["verboseOutput"] = m_config.verboseOutput;
            j["showTimings"] = m_config.showTimings;
            j["outputFormat"] = m_config.outputFormat;
            
            // Save performance settings
            j["performanceTimeoutMs"] = m_config.performanceTimeoutMs;
            j["performanceIterations"] = m_config.performanceIterations;
            
            // Save test directories
            j["testDirectories"] = m_config.testDirectories;
            
            // Save exclude patterns
            j["excludePatterns"] = m_config.excludePatterns;
            
            std::ofstream file(configPath);
            if (!file.is_open()) {
                std::cerr << "[ERROR] Failed to open config file for writing: " << configPath << std::endl;
                return false;
            }
            
            file << j.dump(4);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to save test config: " << e.what() << std::endl;
            return false;
        }
#else
        std::cerr << "[WARNING] JSON support not available, cannot save test configuration" << std::endl;
        return false;
#endif
    }
    
    const TestConfig& GetConfig() const override {
        return m_config;
    }
    
    void SetConfig(const TestConfig& config) override {
        m_config = config;
    }
    
    std::vector<std::string> DiscoverAllTests() override {
        std::vector<std::string> allTests;
        
        for (const auto& directory : m_config.testDirectories) {
            auto tests = m_discovery->DiscoverTests(directory, TestCategory::All);
            
            // Filter tests based on configuration
            for (const auto& test : tests) {
                if (m_discovery->ShouldIncludeTest(test, m_config)) {
                    allTests.push_back(test);
                }
            }
        }
        
        return allTests;
    }
    
    std::vector<TestExecutionResult> ExecuteAllTests() override {
        auto tests = DiscoverAllTests();
        return m_executor->ExecuteTests(tests, m_config);
    }
    
    std::vector<TestExecutionResult> ExecuteTestsByCategory(TestCategory category) override {
        std::vector<std::string> categoryTests;
        
        for (const auto& directory : m_config.testDirectories) {
            auto tests = m_discovery->DiscoverTests(directory, category);
            
            for (const auto& test : tests) {
                if (m_discovery->ShouldIncludeTest(test, m_config)) {
                    categoryTests.push_back(test);
                }
            }
        }
        
        return m_executor->ExecuteTests(categoryTests, m_config);
    }
    
    void GenerateReport(const std::vector<TestExecutionResult>& results, const std::string& outputPath) override {
        if (m_config.outputFormat == "json") {
            GenerateJsonReport(results, outputPath);
        } else {
            GenerateStandardReport(results, outputPath);
        }
    }
    
private:
    void GenerateStandardReport(const std::vector<TestExecutionResult>& results, const std::string& outputPath) {
        std::ostream* output = &std::cout;
        std::ofstream file;
        
        if (!outputPath.empty()) {
            file.open(outputPath);
            if (file.is_open()) {
                output = &file;
            }
        }
        
        *output << "========================================" << std::endl;
        *output << " Game Engine Kiro - Test Report" << std::endl;
        *output << "========================================" << std::endl;
        
        // Summary by category
        std::unordered_map<std::string, int> categoryPassed;
        std::unordered_map<std::string, int> categoryTotal;
        double totalTime = 0.0;
        int totalPassed = 0;
        
        for (const auto& result : results) {
            categoryTotal[result.category]++;
            if (result.passed) {
                categoryPassed[result.category]++;
                totalPassed++;
            }
            totalTime += result.executionTimeMs;
        }
        
        *output << std::endl << "Summary by Category:" << std::endl;
        for (const auto& [category, total] : categoryTotal) {
            int passed = categoryPassed[category];
            *output << "  " << category << ": " << passed << "/" << total << " passed" << std::endl;
        }
        
        *output << std::endl << "Overall: " << totalPassed << "/" << results.size() << " passed";
        if (m_config.showTimings) {
            *output << " (" << TestFrameworkUtils::FormatExecutionTime(totalTime) << ")";
        }
        *output << std::endl;
        
        // Detailed results
        if (m_config.verboseOutput) {
            *output << std::endl << "Detailed Results:" << std::endl;
            for (const auto& result : results) {
                *output << "  [" << (result.passed ? "PASS" : "FAIL") << "] " 
                       << result.testName << " (" << result.category << ")";
                if (m_config.showTimings) {
                    *output << " - " << TestFrameworkUtils::FormatExecutionTime(result.executionTimeMs);
                }
                *output << std::endl;
                
                if (!result.passed && !result.errorMessage.empty()) {
                    *output << "    Error: " << result.errorMessage << std::endl;
                }
            }
        }
        
        *output << "========================================" << std::endl;
    }
    
    void GenerateJsonReport(const std::vector<TestExecutionResult>& results, const std::string& outputPath) {
#ifdef GAMEENGINE_HAS_JSON
        json report;
        report["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        report["totalTests"] = results.size();
        
        int passed = 0;
        double totalTime = 0.0;
        
        json testResults = json::array();
        for (const auto& result : results) {
            json testJson;
            testJson["name"] = result.testName;
            testJson["category"] = result.category;
            testJson["passed"] = result.passed;
            testJson["executionTimeMs"] = result.executionTimeMs;
            testJson["errorMessage"] = result.errorMessage;
            
            testResults.push_back(testJson);
            
            if (result.passed) passed++;
            totalTime += result.executionTimeMs;
        }
        
        report["testsPassed"] = passed;
        report["testsFailed"] = results.size() - passed;
        report["totalExecutionTimeMs"] = totalTime;
        report["results"] = testResults;
        
        std::ostream* output = &std::cout;
        std::ofstream file;
        
        if (!outputPath.empty()) {
            file.open(outputPath);
            if (file.is_open()) {
                output = &file;
            }
        }
        
        *output << report.dump(4) << std::endl;
#else
        std::cerr << "[WARNING] JSON support not available, falling back to standard report" << std::endl;
        GenerateStandardReport(results, outputPath);
#endif
    }
};

// Static instance for global access
static std::unique_ptr<ITestFramework> g_testFramework;

/**
 * Get the global test framework instance
 */
ITestFramework& GetTestFramework() {
    if (!g_testFramework) {
        g_testFramework = std::make_unique<DefaultTestFramework>();
    }
    return *g_testFramework;
}

// TestFrameworkUtils implementation
std::string TestFrameworkUtils::CategoryToString(TestCategory category) {
    switch (category) {
        case TestCategory::Unit: return "unit";
        case TestCategory::Integration: return "integration";
        case TestCategory::Performance: return "performance";
        case TestCategory::All: return "all";
        default: return "unknown";
    }
}

TestCategory TestFrameworkUtils::StringToCategory(const std::string& categoryStr) {
    if (categoryStr == "unit") return TestCategory::Unit;
    if (categoryStr == "integration") return TestCategory::Integration;
    if (categoryStr == "performance") return TestCategory::Performance;
    if (categoryStr == "all") return TestCategory::All;
    return TestCategory::Unit; // Default
}

bool TestFrameworkUtils::TestExecutableExists(const std::string& testPath) {
    return std::filesystem::exists(testPath);
}

std::string TestFrameworkUtils::GetExecutablePath(const std::string& sourcePath) {
    std::string filename = std::filesystem::path(sourcePath).stem().string();
    
    // Convert test_something.cpp to SomethingTest.exe
    if (filename.starts_with("test_")) {
        std::string baseName = filename.substr(5); // Remove "test_"
        
        // Convert to PascalCase
        std::string execName;
        bool capitalizeNext = true;
        for (char c : baseName) {
            if (c == '_') {
                capitalizeNext = true;
            } else if (capitalizeNext) {
                execName += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                capitalizeNext = false;
            } else {
                execName += c;
            }
        }
        
        execName += "Test.exe";
        return "build/Release/" + execName;
    }
    
    return "build/Release/" + filename + ".exe";
}

std::string TestFrameworkUtils::ExtractTestName(const std::string& filePath) {
    std::string filename = std::filesystem::path(filePath).stem().string();
    if (filename.starts_with("test_")) {
        return filename.substr(5); // Remove "test_" prefix
    }
    return filename;
}

std::string TestFrameworkUtils::FormatExecutionTime(double timeMs) {
    std::ostringstream oss;
    if (timeMs < 1000.0) {
        oss << std::fixed << std::setprecision(1) << timeMs << "ms";
    } else {
        oss << std::fixed << std::setprecision(2) << (timeMs / 1000.0) << "s";
    }
    return oss.str();
}

} // namespace Testing
} // namespace GameEngine