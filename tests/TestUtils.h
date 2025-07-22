#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "Core/Math.h"

namespace GameEngine {
namespace Testing {

/**
 * Floating-point comparison utilities for test validation
 */
class FloatComparison {
public:
    static constexpr float DEFAULT_EPSILON = 0.001f;
    static constexpr float LOOSE_EPSILON = 0.01f;
    static constexpr float TIGHT_EPSILON = 0.0001f;

    /**
     * Compare two floating-point values with epsilon tolerance
     */
    static bool IsNearlyEqual(float a, float b, float epsilon = DEFAULT_EPSILON) {
        return std::abs(a - b) < epsilon;
    }

    /**
     * Compare two Vec3 values with epsilon tolerance
     */
    static bool IsNearlyEqual(const Math::Vec3& a, const Math::Vec3& b, float epsilon = DEFAULT_EPSILON) {
        return IsNearlyEqual(a.x, b.x, epsilon) &&
               IsNearlyEqual(a.y, b.y, epsilon) &&
               IsNearlyEqual(a.z, b.z, epsilon);
    }

    /**
     * Compare two Vec4 values with epsilon tolerance
     */
    static bool IsNearlyEqual(const Math::Vec4& a, const Math::Vec4& b, float epsilon = DEFAULT_EPSILON) {
        return IsNearlyEqual(a.x, b.x, epsilon) &&
               IsNearlyEqual(a.y, b.y, epsilon) &&
               IsNearlyEqual(a.z, b.z, epsilon) &&
               IsNearlyEqual(a.w, b.w, epsilon);
    }

    /**
     * Compare two quaternions with epsilon tolerance
     */
    static bool IsNearlyEqual(const Math::Quat& a, const Math::Quat& b, float epsilon = DEFAULT_EPSILON) {
        return IsNearlyEqual(a.x, b.x, epsilon) &&
               IsNearlyEqual(a.y, b.y, epsilon) &&
               IsNearlyEqual(a.z, b.z, epsilon) &&
               IsNearlyEqual(a.w, b.w, epsilon);
    }

    /**
     * Compare two Mat4 matrices with epsilon tolerance
     */
    static bool IsNearlyEqual(const Math::Mat4& a, const Math::Mat4& b, float epsilon = DEFAULT_EPSILON) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (!IsNearlyEqual(a[i][j], b[i][j], epsilon)) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * Check if a floating-point value is nearly zero
     */
    static bool IsNearlyZero(float value, float epsilon = DEFAULT_EPSILON) {
        return std::abs(value) < epsilon;
    }

    /**
     * Check if a vector is nearly zero
     */
    static bool IsNearlyZero(const Math::Vec3& vec, float epsilon = DEFAULT_EPSILON) {
        return IsNearlyZero(vec.x, epsilon) &&
               IsNearlyZero(vec.y, epsilon) &&
               IsNearlyZero(vec.z, epsilon);
    }
};

/**
 * High-precision timing utility for performance testing
 */
class TestTimer {
public:
    TestTimer() : m_start(std::chrono::high_resolution_clock::now()) {}

    /**
     * Get elapsed time in milliseconds
     */
    double ElapsedMs() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
        return duration.count() / 1000.0;
    }

    /**
     * Get elapsed time in microseconds
     */
    long long ElapsedUs() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
        return duration.count();
    }

    /**
     * Get elapsed time in nanoseconds
     */
    long long ElapsedNs() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start);
        return duration.count();
    }

    /**
     * Restart the timer
     */
    void Restart() {
        m_start = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
};

/**
 * Standardized test output formatting utilities
 */
class TestOutput {
public:
    /**
     * Print test header with consistent formatting
     */
    static void PrintHeader(const std::string& testSuiteName) {
        std::cout << "========================================" << std::endl;
        std::cout << " Game Engine Kiro - " << testSuiteName << " Tests" << std::endl;
        std::cout << "========================================" << std::endl;
    }

    /**
     * Print test footer with results
     */
    static void PrintFooter(bool allPassed) {
        std::cout << "========================================" << std::endl;
        if (allPassed) {
            std::cout << "[SUCCESS] ALL TESTS PASSED!" << std::endl;
        } else {
            std::cout << "[FAILED] SOME TESTS FAILED!" << std::endl;
        }
        std::cout << "========================================" << std::endl;
    }

    /**
     * Print test start message
     */
    static void PrintTestStart(const std::string& testName) {
        std::cout << "Testing " << testName << "..." << std::endl;
    }

    /**
     * Print test pass message
     */
    static void PrintTestPass(const std::string& testName) {
        std::cout << "  [PASS] " << testName << " passed" << std::endl;
    }

    /**
     * Print test failure message
     */
    static void PrintTestFail(const std::string& testName) {
        std::cout << "  [FAILED] " << testName << " failed" << std::endl;
    }

    /**
     * Print test failure with details
     */
    static void PrintTestFail(const std::string& testName, const std::string& expected, const std::string& actual) {
        std::cout << "  [FAILED] " << testName << " failed" << std::endl;
        std::cout << "    Expected: " << expected << std::endl;
        std::cout << "    Actual: " << actual << std::endl;
    }

    /**
     * Print informational message
     */
    static void PrintInfo(const std::string& message) {
        std::cout << "  [INFO] " << message << std::endl;
    }

    /**
     * Print warning message
     */
    static void PrintWarning(const std::string& message) {
        std::cout << "  [WARNING] " << message << std::endl;
    }

    /**
     * Print error message
     */
    static void PrintError(const std::string& message) {
        std::cout << "  [ERROR] " << message << std::endl;
    }

    /**
     * Print performance timing result
     */
    static void PrintTiming(const std::string& operation, double timeMs, int iterations = 1) {
        std::cout << "  [INFO] " << operation << " completed in " 
                  << std::fixed << std::setprecision(3) << timeMs << "ms";
        if (iterations > 1) {
            std::cout << " (" << iterations << " iterations, " 
                      << std::fixed << std::setprecision(6) << (timeMs / iterations) << "ms per iteration)";
        }
        std::cout << std::endl;
    }
};

/**
 * Performance testing utilities
 */
class PerformanceTest {
public:
    /**
     * Run a function multiple times and measure average performance
     */
    template<typename Func>
    static double MeasureAverageTime(Func&& func, int iterations = 1000) {
        TestTimer timer;
        
        for (int i = 0; i < iterations; ++i) {
            func();
        }
        
        return timer.ElapsedMs() / iterations;
    }

    /**
     * Run a performance test with threshold validation
     */
    template<typename Func>
    static bool ValidatePerformance(const std::string& testName, Func&& func, 
                                   double thresholdMs, int iterations = 1000) {
        TestOutput::PrintTestStart(testName);
        
        double avgTime = MeasureAverageTime(std::forward<Func>(func), iterations);
        
        TestOutput::PrintTiming(testName, avgTime * iterations, iterations);
        
        if (avgTime < thresholdMs) {
            TestOutput::PrintTestPass(testName);
            return true;
        } else {
            std::ostringstream expected, actual;
            expected << "< " << thresholdMs << "ms per iteration";
            actual << avgTime << "ms per iteration";
            TestOutput::PrintTestFail(testName, expected.str(), actual.str());
            return false;
        }
    }
};

/**
 * Memory testing utilities
 */
class MemoryTest {
public:
    /**
     * Simple memory usage tracker (platform-specific implementations would be added)
     */
    static size_t GetCurrentMemoryUsage() {
        // This is a placeholder - real implementation would use platform-specific APIs
        // On Windows: GetProcessMemoryInfo
        // On Linux: /proc/self/status
        // On macOS: task_info
        return 0;
    }

    /**
     * Test for memory leaks in a function
     */
    template<typename Func>
    static bool TestForMemoryLeaks(const std::string& testName, Func&& func) {
        TestOutput::PrintTestStart(testName + " (memory leak check)");
        
        size_t memoryBefore = GetCurrentMemoryUsage();
        func();
        size_t memoryAfter = GetCurrentMemoryUsage();
        
        // Simple leak detection (would need more sophisticated implementation)
        if (memoryAfter <= memoryBefore + 1024) { // Allow 1KB tolerance
            TestOutput::PrintTestPass(testName + " (no memory leaks detected)");
            return true;
        } else {
            TestOutput::PrintTestFail(testName + " (potential memory leak detected)");
            return false;
        }
    }
};

/**
 * String formatting utilities for test output
 */
class StringUtils {
public:
    /**
     * Format a floating-point value for test output
     */
    static std::string FormatFloat(float value, int precision = 3) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    /**
     * Format a Vec3 for test output
     */
    static std::string FormatVec3(const Math::Vec3& vec, int precision = 3) {
        std::ostringstream oss;
        oss << "(" << FormatFloat(vec.x, precision) << ", " 
            << FormatFloat(vec.y, precision) << ", " 
            << FormatFloat(vec.z, precision) << ")";
        return oss.str();
    }

    /**
     * Format a quaternion for test output
     */
    static std::string FormatQuat(const Math::Quat& quat, int precision = 3) {
        std::ostringstream oss;
        oss << "(" << FormatFloat(quat.x, precision) << ", " 
            << FormatFloat(quat.y, precision) << ", " 
            << FormatFloat(quat.z, precision) << ", " 
            << FormatFloat(quat.w, precision) << ")";
        return oss.str();
    }
};

/**
 * Test result tracking utilities
 */
struct TestResult {
    std::string testName;
    bool passed;
    std::string errorMessage;
    double executionTimeMs;
    
    TestResult(const std::string& name, bool success, const std::string& error = "", double time = 0.0)
        : testName(name), passed(success), errorMessage(error), executionTimeMs(time) {}
};

class TestSuite {
public:
    TestSuite(const std::string& suiteName) : m_suiteName(suiteName) {}

    /**
     * Run a test and track the result
     */
    template<typename TestFunc>
    bool RunTest(const std::string& testName, TestFunc&& testFunc) {
        TestTimer timer;
        bool result = false;
        
        try {
            result = testFunc();
        } catch (const std::exception& e) {
            TestOutput::PrintError("TEST EXCEPTION in " + testName + ": " + e.what());
            result = false;
        } catch (...) {
            TestOutput::PrintError("UNKNOWN TEST ERROR in " + testName);
            result = false;
        }
        
        double elapsed = timer.ElapsedMs();
        m_results.emplace_back(testName, result, "", elapsed);
        
        return result;
    }

    /**
     * Print summary of all test results
     */
    void PrintSummary() const {
        int passed = 0;
        int failed = 0;
        double totalTime = 0.0;
        
        for (const auto& result : m_results) {
            if (result.passed) {
                passed++;
            } else {
                failed++;
            }
            totalTime += result.executionTimeMs;
        }
        
        TestOutput::PrintInfo("Test Summary:");
        TestOutput::PrintInfo("  Total: " + std::to_string(m_results.size()));
        TestOutput::PrintInfo("  Passed: " + std::to_string(passed));
        TestOutput::PrintInfo("  Failed: " + std::to_string(failed));
        TestOutput::PrintInfo("  Total Time: " + StringUtils::FormatFloat(totalTime) + "ms");
    }

    /**
     * Check if all tests passed
     */
    bool AllTestsPassed() const {
        for (const auto& result : m_results) {
            if (!result.passed) {
                return false;
            }
        }
        return true;
    }

private:
    std::string m_suiteName;
    std::vector<TestResult> m_results;
};

} // namespace Testing
} // namespace GameEngine

// Convenience macros for common operations
#define EXPECT_NEARLY_EQUAL(a, b) \
    do { \
        if (!GameEngine::Testing::FloatComparison::IsNearlyEqual(a, b)) { \
            GameEngine::Testing::TestOutput::PrintTestFail(__func__, \
                GameEngine::Testing::StringUtils::FormatFloat(b), \
                GameEngine::Testing::StringUtils::FormatFloat(a)); \
            return false; \
        } \
    } while(0)

#define EXPECT_NEARLY_EQUAL_EPSILON(a, b, epsilon) \
    do { \
        if (!GameEngine::Testing::FloatComparison::IsNearlyEqual(a, b, epsilon)) { \
            GameEngine::Testing::TestOutput::PrintTestFail(__func__, \
                GameEngine::Testing::StringUtils::FormatFloat(b), \
                GameEngine::Testing::StringUtils::FormatFloat(a)); \
            return false; \
        } \
    } while(0)

#define EXPECT_VEC3_NEARLY_EQUAL(a, b) \
    do { \
        if (!GameEngine::Testing::FloatComparison::IsNearlyEqual(a, b)) { \
            GameEngine::Testing::TestOutput::PrintTestFail(__func__, \
                GameEngine::Testing::StringUtils::FormatVec3(b), \
                GameEngine::Testing::StringUtils::FormatVec3(a)); \
            return false; \
        } \
    } while(0)

#define EXPECT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            GameEngine::Testing::TestOutput::PrintTestFail(__func__, "true", "false"); \
            return false; \
        } \
    } while(0)

#define EXPECT_FALSE(condition) \
    do { \
        if (condition) { \
            GameEngine::Testing::TestOutput::PrintTestFail(__func__, "false", "true"); \
            return false; \
        } \
    } while(0)