#include "TestUtils.h"
#include <iostream>

using namespace GameEngine::Testing;

/**
 * Minimal test to check basic functionality
 */
bool TestMinimal() {
    TestOutput::PrintTestStart("minimal test");
    
    std::cout << "Test is running..." << std::endl;
    
    TestOutput::PrintTestPass("minimal test");
    return true;
}

int main() {
    TestOutput::PrintHeader("MinimalTest");

    bool allPassed = true;

    try {
        TestSuite suite("Minimal Tests");
        allPassed &= suite.RunTest("Minimal Test", TestMinimal);
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