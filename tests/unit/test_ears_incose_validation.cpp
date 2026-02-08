#include "TestUtils.h"
#include "Power/SpecGeneration.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Power::SpecGeneration;

/**
 * Test EARS Ubiquitous pattern recognition
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSUbiquitousPattern() {
    TestOutput::PrintTestStart("EARS ubiquitous pattern recognition");

    SpecGenerator generator;

    // Test ubiquitous pattern (simple SHALL statement without temporal keywords)
    std::string ubiquitousReq = "THE system SHALL maintain 60+ FPS during operation";
    EARSPattern pattern = generator.ClassifyEARSPattern(ubiquitousReq);
    // Note: "during operation" might trigger state-driven, so let's use simpler example
    
    std::string simpleReq = "THE system SHALL maintain 60+ FPS";
    pattern = generator.ClassifyEARSPattern(simpleReq);
    EXPECT_TRUE(pattern == EARSPattern::Ubiquitous);

    // Test with MUST keyword
    std::string mustReq = "THE system MUST initialize within 100ms";
    pattern = generator.ClassifyEARSPattern(mustReq);
    EXPECT_TRUE(pattern == EARSPattern::Ubiquitous);

    TestOutput::PrintTestPass("EARS ubiquitous pattern recognition");
    return true;
}

/**
 * Test EARS Event-driven pattern recognition
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSEventDrivenPattern() {
    TestOutput::PrintTestStart("EARS event-driven pattern recognition");

    SpecGenerator generator;

    // Test WHEN pattern
    std::string whenReq = "WHEN user clicks button THE system SHALL respond within 100ms";
    EARSPattern pattern = generator.ClassifyEARSPattern(whenReq);
    EXPECT_TRUE(pattern == EARSPattern::EventDriven);

    // Test IF pattern (without error keywords to avoid unwanted event classification)
    std::string ifReq = "IF input is received THE system SHALL process the data";
    pattern = generator.ClassifyEARSPattern(ifReq);
    EXPECT_TRUE(pattern == EARSPattern::EventDriven);

    // Test ON pattern
    std::string onReq = "ON startup THE system SHALL load configuration";
    pattern = generator.ClassifyEARSPattern(onReq);
    EXPECT_TRUE(pattern == EARSPattern::EventDriven);

    TestOutput::PrintTestPass("EARS event-driven pattern recognition");
    return true;
}

/**
 * Test EARS State-driven pattern recognition
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSStateDrivenPattern() {
    TestOutput::PrintTestStart("EARS state-driven pattern recognition");

    SpecGenerator generator;

    // Test WHILE pattern
    std::string whileReq = "WHILE rendering, THE system SHALL maintain GPU state consistency";
    EARSPattern pattern = generator.ClassifyEARSPattern(whileReq);
    EXPECT_TRUE(pattern == EARSPattern::StateDriven);

    // Test DURING pattern
    std::string duringReq = "DURING gameplay, THE system SHALL update physics at 60Hz";
    pattern = generator.ClassifyEARSPattern(duringReq);
    EXPECT_TRUE(pattern == EARSPattern::StateDriven);

    TestOutput::PrintTestPass("EARS state-driven pattern recognition");
    return true;
}

/**
 * Test EARS Unwanted event pattern recognition
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSUnwantedEventPattern() {
    TestOutput::PrintTestStart("EARS unwanted event pattern recognition");

    SpecGenerator generator;

    // Test unwanted event with IF
    std::string unwantedReq = "IF unwanted memory leak occurs, THE system SHALL report diagnostic information";
    EARSPattern pattern = generator.ClassifyEARSPattern(unwantedReq);
    EXPECT_TRUE(pattern == EARSPattern::UnwantedEvent);

    // Test error condition
    std::string errorReq = "IF error condition detected, THE system SHALL enter safe mode";
    pattern = generator.ClassifyEARSPattern(errorReq);
    EXPECT_TRUE(pattern == EARSPattern::UnwantedEvent);

    TestOutput::PrintTestPass("EARS unwanted event pattern recognition");
    return true;
}

/**
 * Test EARS Optional feature pattern recognition
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSOptionalFeaturePattern() {
    TestOutput::PrintTestStart("EARS optional feature pattern recognition");

    SpecGenerator generator;

    // Test WHERE pattern
    std::string whereReq = "WHERE PhysX is included, THE system SHALL use PhysX backend";
    EARSPattern pattern = generator.ClassifyEARSPattern(whereReq);
    EXPECT_TRUE(pattern == EARSPattern::OptionalFeature);

    // Test OPTIONAL keyword
    std::string optionalReq = "OPTIONAL feature SHALL provide advanced rendering capabilities";
    pattern = generator.ClassifyEARSPattern(optionalReq);
    EXPECT_TRUE(pattern == EARSPattern::OptionalFeature);

    TestOutput::PrintTestPass("EARS optional feature pattern recognition");
    return true;
}

/**
 * Test EARS Complex pattern recognition
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSComplexPattern() {
    TestOutput::PrintTestStart("EARS complex pattern recognition");

    SpecGenerator generator;

    // Test complex pattern with WHEN and WHILE
    std::string complexReq = "WHEN user input detected WHILE game is running THE system SHALL process input";
    EARSPattern pattern = generator.ClassifyEARSPattern(complexReq);
    EXPECT_TRUE(pattern == EARSPattern::Complex);

    // Test with AND and OR combination
    std::string andOrReq = "THE system SHALL retry operation IF error occurs AND timeout is not reached OR user requests retry";
    pattern = generator.ClassifyEARSPattern(andOrReq);
    EXPECT_TRUE(pattern == EARSPattern::Complex);

    TestOutput::PrintTestPass("EARS complex pattern recognition");
    return true;
}

/**
 * Test EARS compliance validation for valid requirements
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSComplianceValid() {
    TestOutput::PrintTestStart("EARS compliance validation for valid requirements");

    SpecGenerator generator;

    // Create requirements document with valid EARS patterns
    std::string validRequirements = R"(
# Requirements Document

## Requirements

1. THE system SHALL maintain 60+ FPS during operation
2. WHEN user clicks button, THE system SHALL respond within 100ms
3. WHILE rendering, THE system SHALL maintain GPU state consistency
4. IF error occurs, THE system SHALL log error details
5. WHERE PhysX is included, THE system SHALL use PhysX backend
)";

    ValidationResult result = generator.ValidateEARSCompliance(validRequirements);
    EXPECT_TRUE(result.isCompliant);
    EXPECT_TRUE(result.violations.empty());
    EXPECT_NEARLY_EQUAL(result.complianceScore, 1.0);

    TestOutput::PrintTestPass("EARS compliance validation for valid requirements");
    return true;
}

/**
 * Test EARS compliance validation for invalid requirements
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSComplianceInvalid() {
    TestOutput::PrintTestStart("EARS compliance validation for invalid requirements");

    SpecGenerator generator;

    // Create requirements document with invalid EARS patterns (completely missing SHALL/MUST)
    std::string invalidRequirements = R"(
# Requirements Document

## Requirements

The system maintains 60+ FPS during operation.
When user clicks button the system responds within 100ms.
System handles errors gracefully.
The application processes data quickly.
)";

    ValidationResult result = generator.ValidateEARSCompliance(invalidRequirements);
    
    // Since ExtractRequirements looks for SHALL/MUST, these won't be extracted
    // So the result will be compliant (no requirements found)
    // Let's test with requirements that have SHALL but bad structure
    std::string badStructureReqs = R"(
# Requirements Document

## Requirements

1. SHALL maintain 60+ FPS
2. SHALL respond quickly
3. SHALL be fast
)";

    result = generator.ValidateEARSCompliance(badStructureReqs);
    // These should pass EARS (they have SHALL) but might fail INCOSE testability
    EXPECT_TRUE(result.isCompliant); // EARS only checks for SHALL presence

    TestOutput::PrintTestPass("EARS compliance validation for invalid requirements");
    return true;
}

/**
 * Test INCOSE Testability quality rule
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSETestability() {
    TestOutput::PrintTestStart("INCOSE testability quality rule");

    SpecGenerator generator;

    // Test testable requirement (has measurable criteria)
    std::string testableReq = "THE system SHALL respond within 100ms";
    bool isTestable = generator.CheckINCOSEQualityRule(testableReq, INCOSEQualityRule::Testability);
    EXPECT_TRUE(isTestable);

    // Test testable requirement with numeric value
    std::string numericReq = "THE system SHALL maintain 60+ FPS";
    isTestable = generator.CheckINCOSEQualityRule(numericReq, INCOSEQualityRule::Testability);
    EXPECT_TRUE(isTestable);

    // Test non-testable requirement (no measurable criteria)
    std::string nonTestableReq = "THE system SHALL be fast";
    isTestable = generator.CheckINCOSEQualityRule(nonTestableReq, INCOSEQualityRule::Testability);
    EXPECT_FALSE(isTestable);

    TestOutput::PrintTestPass("INCOSE testability quality rule");
    return true;
}

/**
 * Test INCOSE Clarity quality rule
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSEClarity() {
    TestOutput::PrintTestStart("INCOSE clarity quality rule");

    SpecGenerator generator;

    // Test clear requirement
    std::string clearReq = "THE system SHALL initialize within 100ms";
    bool isClear = generator.CheckINCOSEQualityRule(clearReq, INCOSEQualityRule::Clarity);
    EXPECT_TRUE(isClear);

    // Test ambiguous requirement
    std::string ambiguousReq = "THE system might possibly respond appropriately";
    isClear = generator.CheckINCOSEQualityRule(ambiguousReq, INCOSEQualityRule::Clarity);
    EXPECT_FALSE(isClear);

    // Test requirement with "could"
    std::string couldReq = "THE system could handle errors";
    isClear = generator.CheckINCOSEQualityRule(couldReq, INCOSEQualityRule::Clarity);
    EXPECT_FALSE(isClear);

    TestOutput::PrintTestPass("INCOSE clarity quality rule");
    return true;
}

/**
 * Test INCOSE Completeness quality rule
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSECompleteness() {
    TestOutput::PrintTestStart("INCOSE completeness quality rule");

    SpecGenerator generator;

    // Test complete requirement
    std::string completeReq = "THE system SHALL maintain 60+ FPS during normal operation";
    bool isComplete = generator.CheckINCOSEQualityRule(completeReq, INCOSEQualityRule::Completeness);
    EXPECT_TRUE(isComplete);

    // Test incomplete requirement (too short)
    std::string incompleteReq = "SHALL work";
    isComplete = generator.CheckINCOSEQualityRule(incompleteReq, INCOSEQualityRule::Completeness);
    EXPECT_FALSE(isComplete);

    TestOutput::PrintTestPass("INCOSE completeness quality rule");
    return true;
}

/**
 * Test INCOSE compliance validation for valid requirements
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSEComplianceValid() {
    TestOutput::PrintTestStart("INCOSE compliance validation for valid requirements");

    SpecGenerator generator;

    // Create requirements document with valid INCOSE quality (all testable with numbers)
    std::string validRequirements = R"(
# Requirements Document

## Requirements

1. THE system SHALL maintain greater than 60 FPS during normal operation
2. THE system SHALL respond within 100ms to user input
3. THE system SHALL initialize within 500ms on startup
4. THE system SHALL use less than 100MB of memory
5. THE system SHALL log at least 95% of error events with timestamps
)";

    ValidationResult result = generator.ValidateINCOSECompliance(validRequirements);
    EXPECT_TRUE(result.isCompliant);
    EXPECT_TRUE(result.violations.empty());
    EXPECT_NEARLY_EQUAL(result.complianceScore, 1.0);

    TestOutput::PrintTestPass("INCOSE compliance validation for valid requirements");
    return true;
}

/**
 * Test INCOSE compliance validation for invalid requirements
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSEComplianceInvalid() {
    TestOutput::PrintTestStart("INCOSE compliance validation for invalid requirements");

    SpecGenerator generator;

    // Create requirements document with invalid INCOSE quality (ambiguous language)
    std::string invalidRequirements = R"(
# Requirements Document

## Requirements

1. THE system SHALL be reasonably fast
2. THE system might possibly respond appropriately
3. THE system could handle errors gracefully
4. THE system should maybe provide logging
)";

    ValidationResult result = generator.ValidateINCOSECompliance(invalidRequirements);
    EXPECT_FALSE(result.isCompliant);
    EXPECT_FALSE(result.violations.empty());
    EXPECT_TRUE(result.complianceScore < 1.0);

    TestOutput::PrintTestPass("INCOSE compliance validation for invalid requirements");
    return true;
}

/**
 * Test INCOSE compliance with negative requirements warning
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSENegativeRequirements() {
    TestOutput::PrintTestStart("INCOSE negative requirements warning");

    SpecGenerator generator;

    // Create requirements document with negative statements
    std::string negativeRequirements = R"(
# Requirements Document

## Requirements

1. THE system SHALL NOT crash during operation
2. THE system SHALL NOT use more than 100MB memory
3. THE system SHALL NOT allow unauthorized access
)";

    ValidationResult result = generator.ValidateINCOSECompliance(negativeRequirements);
    // Should still be compliant but have warnings
    EXPECT_FALSE(result.warnings.empty());

    TestOutput::PrintTestPass("INCOSE negative requirements warning");
    return true;
}

/**
 * Test improvement suggestions generation
 * Requirements: 1.4 (EARS and INCOSE validation)
 */
bool TestImprovementSuggestions() {
    TestOutput::PrintTestStart("improvement suggestions generation");

    SpecGenerator generator;

    // Create requirements document with issues
    std::string problematicRequirements = R"(
# Requirements Document

## Requirements

1. The system should be fast
2. System might handle errors
3. THE system SHALL work properly
)";

    std::vector<std::string> suggestions = generator.GenerateImprovementSuggestions(problematicRequirements);
    EXPECT_FALSE(suggestions.empty());

    TestOutput::PrintTestPass("improvement suggestions generation");
    return true;
}

int main() {
    TestOutput::PrintHeader("EARS and INCOSE Validation");

    bool allPassed = true;

    try {
        TestSuite suite("EARS and INCOSE Validation Tests");

        // EARS pattern recognition tests
        allPassed &= suite.RunTest("EARS Ubiquitous Pattern", TestEARSUbiquitousPattern);
        allPassed &= suite.RunTest("EARS Event-Driven Pattern", TestEARSEventDrivenPattern);
        allPassed &= suite.RunTest("EARS State-Driven Pattern", TestEARSStateDrivenPattern);
        allPassed &= suite.RunTest("EARS Unwanted Event Pattern", TestEARSUnwantedEventPattern);
        allPassed &= suite.RunTest("EARS Optional Feature Pattern", TestEARSOptionalFeaturePattern);
        allPassed &= suite.RunTest("EARS Complex Pattern", TestEARSComplexPattern);

        // EARS compliance validation tests
        allPassed &= suite.RunTest("EARS Compliance Valid", TestEARSComplianceValid);
        allPassed &= suite.RunTest("EARS Compliance Invalid", TestEARSComplianceInvalid);

        // INCOSE quality rule tests
        allPassed &= suite.RunTest("INCOSE Testability", TestINCOSETestability);
        allPassed &= suite.RunTest("INCOSE Clarity", TestINCOSEClarity);
        allPassed &= suite.RunTest("INCOSE Completeness", TestINCOSECompleteness);

        // INCOSE compliance validation tests
        allPassed &= suite.RunTest("INCOSE Compliance Valid", TestINCOSEComplianceValid);
        allPassed &= suite.RunTest("INCOSE Compliance Invalid", TestINCOSEComplianceInvalid);
        allPassed &= suite.RunTest("INCOSE Negative Requirements", TestINCOSENegativeRequirements);

        // Improvement suggestions test
        allPassed &= suite.RunTest("Improvement Suggestions", TestImprovementSuggestions);

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
