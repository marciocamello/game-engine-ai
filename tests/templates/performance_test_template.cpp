/**
 * Performance Test Template for Game Engine Kiro
 * 
 * Instructions:
 * 1. Copy this file to tests/performance/test_[component]_performance.cpp
 * 2. Replace [COMPONENT] with your component name
 * 3. Replace [Component] with PascalCase version
 * 4. Add your component's header include
 * 5. Implement performance test functions with appropriate thresholds
 * 6. Update the requirements references in comments
 * 7. Build and run your tests (note: performance tests usually disable coverage)
 */

#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <random>
#include "TestUtils.h"
#include "Core/[Component].h"  // Replace with actual component header

using namespace GameEngine;
using namespace GameEngine::Testing;

/**
 * Test basic operation performance
 * Requirements: [Add requirement references here]
 */
bool TestBasicOperationPerformance() {
    const int iterations = 10000;
    const double thresholdMs = 0.001; // 1 microsecond per operation

    return PerformanceTest::ValidatePerformance(
        "[component] basic operation",
        []() {
            // [Component] component;
            // component.DoBasicOperation();
        },
        thresholdMs,
        iterations
    );
}

/**
 * Test bulk operation performance
 * Requirements: [Add requirement references here]
 */
bool TestBulkOperationPerformance() {
    TestOutput::PrintTestStart("[component] bulk operations");

    const int bulkSize = 1000;
    const int iterations = 100;
    
    // Setup test data
    std::vector<int> testData(bulkSize);
    std::iota(testData.begin(), testData.end(), 0);

    TestTimer timer;
    
    for (int i = 0; i < iterations; ++i) {
        // Perform bulk operations
        // [Component] component;
        // component.ProcessBulkData(testData);
    }
    
    double elapsed = timer.ElapsedMs();
    double avgTime = elapsed / iterations;
    
    TestOutput::PrintTiming("[component] bulk operations", elapsed, iterations);
    
    // Validate performance threshold
    const double maxTimePerBulkOperation = 10.0; // ms
    
    if (avgTime < maxTimePerBulkOperation) {
        TestOutput::PrintTestPass("[component] bulk operations");
        return true;
    } else {
        TestOutput::PrintTestFail("[component] bulk operations",
            "< " + StringUtils::FormatFloat(maxTimePerBulkOperation) + "ms per bulk operation",
            StringUtils::FormatFloat(avgTime) + "ms per bulk operation");
        return false;
    }
}

/**
 * Test memory allocation performance
 * Requirements: [Add requirement references here]
 */
bool TestMemoryAllocationPerformance() {
    const int iterations = 1000;
    const double thresholdMs = 0.01; // 10 microseconds per allocation

    return PerformanceTest::ValidatePerformance(
        "[component] memory allocation",
        []() {
            // Test memory allocation patterns
            // auto ptr = std::make_unique<[Component]>();
            // ptr->Initialize();
        },
        thresholdMs,
        iterations
    );
}

/**
 * Test concurrent access performance
 * Requirements: [Add requirement references here]
 */
bool TestConcurrentAccessPerformance() {
    TestOutput::PrintTestStart("[component] concurrent access");

    // This is a simplified concurrent test - real implementation would use threads
    const int concurrentOperations = 1000;
    
    TestTimer timer;
    
    // Simulate concurrent access patterns
    for (int i = 0; i < concurrentOperations; ++i) {
        // [Component] component;
        // component.ThreadSafeOperation();
    }
    
    double elapsed = timer.ElapsedMs();
    TestOutput::PrintTiming("[component] concurrent access", elapsed, concurrentOperations);
    
    // Validate reasonable performance under concurrent load
    const double maxTimePerOperation = 0.1; // ms
    double avgTime = elapsed / concurrentOperations;
    
    if (avgTime < maxTimePerOperation) {
        TestOutput::PrintTestPass("[component] concurrent access");
        return true;
    } else {
        TestOutput::PrintTestFail("[component] concurrent access",
            "< " + StringUtils::FormatFloat(maxTimePerOperation) + "ms per operation",
            StringUtils::FormatFloat(avgTime) + "ms per operation");
        return false;
    }
}

/**
 * Test cache performance and data locality
 * Requirements: [Add requirement references here]
 */
bool TestCachePerformance() {
    TestOutput::PrintTestStart("[component] cache performance");

    const int dataSize = 10000;
    const int iterations = 100;
    
    // Create test data that tests cache behavior
    std::vector<float> sequentialData(dataSize);
    std::vector<int> randomIndices(dataSize);
    
    // Fill with test data
    std::iota(sequentialData.begin(), sequentialData.end(), 0.0f);
    std::iota(randomIndices.begin(), randomIndices.end(), 0);
    
    // Shuffle for random access pattern
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(randomIndices.begin(), randomIndices.end(), g);
    
    // Test sequential access
    TestTimer sequentialTimer;
    for (int i = 0; i < iterations; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < dataSize; ++j) {
            sum += sequentialData[j];
        }
        // Prevent optimization
        volatile float result = sum;
        (void)result;
    }
    double sequentialTime = sequentialTimer.ElapsedMs();
    
    // Test random access
    TestTimer randomTimer;
    for (int i = 0; i < iterations; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < dataSize; ++j) {
            sum += sequentialData[randomIndices[j]];
        }
        // Prevent optimization
        volatile float result = sum;
        (void)result;
    }
    double randomTime = randomTimer.ElapsedMs();
    
    TestOutput::PrintTiming("sequential access", sequentialTime, iterations);
    TestOutput::PrintTiming("random access", randomTime, iterations);
    
    // Sequential should be significantly faster than random for cache-friendly code
    double ratio = randomTime / sequentialTime;
    TestOutput::PrintInfo("Random/Sequential ratio: " + StringUtils::FormatFloat(ratio, 2));
    
    // This is a heuristic - adjust based on expected cache behavior
    if (ratio > 1.5 && ratio < 10.0) {
        TestOutput::PrintTestPass("[component] cache performance (good cache locality)");
        return true;
    } else if (ratio <= 1.5) {
        TestOutput::PrintWarning("[component] cache performance (no significant cache effect)");
        return true; // Not necessarily a failure
    } else {
        TestOutput::PrintTestFail("[component] cache performance (poor cache locality)");
        return false;
    }
}

/**
 * Test scalability with increasing data sizes
 * Requirements: [Add requirement references here]
 */
bool TestScalabilityPerformance() {
    TestOutput::PrintTestStart("[component] scalability");

    std::vector<int> dataSizes = {100, 1000, 10000, 100000};
    std::vector<double> times;
    
    for (int size : dataSizes) {
        TestTimer timer;
        
        // Test with increasing data size
        for (int i = 0; i < 10; ++i) { // Fewer iterations for larger sizes
            // [Component] component;
            // component.ProcessDataOfSize(size);
        }
        
        double elapsed = timer.ElapsedMs();
        times.push_back(elapsed / 10.0); // Average time
        
        TestOutput::PrintInfo("Size " + std::to_string(size) + ": " + 
                             StringUtils::FormatFloat(elapsed / 10.0) + "ms");
    }
    
    // Check if scaling is reasonable (should be roughly linear or better)
    bool scalingOk = true;
    for (size_t i = 1; i < times.size(); ++i) {
        double ratio = times[i] / times[i-1];
        double sizeRatio = static_cast<double>(dataSizes[i]) / dataSizes[i-1];
        
        // Time should not increase faster than O(n log n)
        if (ratio > sizeRatio * std::log2(sizeRatio) * 2.0) {
            scalingOk = false;
            break;
        }
    }
    
    if (scalingOk) {
        TestOutput::PrintTestPass("[component] scalability");
        return true;
    } else {
        TestOutput::PrintTestFail("[component] scalability (poor scaling behavior)");
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("[COMPONENT] Performance");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("[COMPONENT] Performance Tests");

        // Run all performance tests
        allPassed &= suite.RunTest("Basic Operation Performance", TestBasicOperationPerformance);
        allPassed &= suite.RunTest("Bulk Operation Performance", TestBulkOperationPerformance);
        allPassed &= suite.RunTest("Memory Allocation Performance", TestMemoryAllocationPerformance);
        allPassed &= suite.RunTest("Concurrent Access Performance", TestConcurrentAccessPerformance);
        allPassed &= suite.RunTest("Cache Performance", TestCachePerformance);
        allPassed &= suite.RunTest("Scalability Performance", TestScalabilityPerformance);

        // Print detailed summary
        suite.PrintSummary();

        TestOutput::PrintFooter(allPassed);
        return allPassed ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("TEST EXCEPTION: " + std::string(e.what()));
        return 1;
    } catch (...) {
        TestOutput::PrintError("UNKNOWN TEST ERROR!");
        return 1;
    }
}