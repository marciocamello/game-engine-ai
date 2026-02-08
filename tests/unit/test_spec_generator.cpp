#include "TestUtils.h"
#include "Power/SpecGeneration.h"

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Power::SpecGeneration;

/**
 * Test basic SpecGenerator initialization
 * Requirements: 1.1 (SpecGenerator class creation)
 */
bool TestSpecGeneratorInitialization() {
    TestOutput::PrintTestStart("SpecGenerator initialization");

    SpecGenerator generator;
    
    // Test that generator initializes without throwing
    EXPECT_TRUE(true); // If we get here, initialization succeeded

    TestOutput::PrintTestPass("SpecGenerator initialization");
    return true;
}

/**
 * Test feature spec generation with simple request
 * Requirements: 1.1, 1.2 (Complete spec generation)
 */
bool TestBasicFeatureSpecGeneration() {
    TestOutput::PrintTestStart("basic feature spec generation");

    SpecGenerator generator;
    
    FeatureRequest request;
    request.featureName = "TestFeature";
    request.description = "A test feature for validation";
    request.targetSystems = {"Graphics"};
    request.complexity = "Simple";
    request.includePropertyTests = true;
    request.includePerformanceProfiling = false;

    GeneratedSpec spec = generator.GenerateFeatureSpec(request);

    // Verify all documents are generated
    EXPECT_FALSE(spec.requirementsDocument.empty());
    EXPECT_FALSE(spec.designDocument.empty());
    EXPECT_FALSE(spec.tasksDocument.empty());

    // Verify generated files list
    EXPECT_EQUAL(spec.generatedFiles.size(), 3u);
    EXPECT_TRUE(std::find(spec.generatedFiles.begin(), spec.generatedFiles.end(), "requirements.md") != spec.generatedFiles.end());
    EXPECT_TRUE(std::find(spec.generatedFiles.begin(), spec.generatedFiles.end(), "design.md") != spec.generatedFiles.end());
    EXPECT_TRUE(std::find(spec.generatedFiles.begin(), spec.generatedFiles.end(), "tasks.md") != spec.generatedFiles.end());

    // Verify feature name appears in documents
    EXPECT_TRUE(spec.requirementsDocument.find("TestFeature") != std::string::npos);
    EXPECT_TRUE(spec.designDocument.find("TestFeature") != std::string::npos);
    EXPECT_TRUE(spec.tasksDocument.find("TestFeature") != std::string::npos);

    TestOutput::PrintTestPass("basic feature spec generation");
    return true;
}

/**
 * Test EARS pattern validation
 * Requirements: 1.4 (EARS pattern validation)
 */
bool TestEARSPatternValidation() {
    TestOutput::PrintTestStart("EARS pattern validation");

    SpecGenerator generator;

    // Test ubiquitous requirement
    std::string ubiquitousReq = "The system SHALL provide user authentication.";
    EARSPattern pattern = generator.ClassifyEARSPattern(ubiquitousReq);
    EXPECT_EQUAL(static_cast<int>(pattern), static_cast<int>(EARSPattern::Ubiquitous));

    // Test event-driven requirement
    std::string eventReq = "WHEN user clicks login button, the system SHALL validate credentials.";
    pattern = generator.ClassifyEARSPattern(eventReq);
    EXPECT_EQUAL(static_cast<int>(pattern), static_cast<int>(EARSPattern::EventDriven));

    // Test state-driven requirement
    std::string stateReq = "WHILE user is logged in, the system SHALL display user menu.";
    pattern = generator.ClassifyEARSPattern(stateReq);
    EXPECT_EQUAL(static_cast<int>(pattern), static_cast<int>(EARSPattern::StateDriven));

    // Test optional feature requirement
    std::string optionalReq = "WHERE advanced features are enabled, the system SHALL provide detailed analytics.";
    pattern = generator.ClassifyEARSPattern(optionalReq);
    EXPECT_EQUAL(static_cast<int>(pattern), static_cast<int>(EARSPattern::OptionalFeature));

    TestOutput::PrintTestPass("EARS pattern validation");
    return true;
}

/**
 * Test INCOSE quality rule checking
 * Requirements: 1.4 (INCOSE quality rule checking)
 */
bool TestINCOSEQualityRules() {
    TestOutput::PrintTestStart("INCOSE quality rule checking");

    SpecGenerator generator;

    // Test testable requirement
    std::string testableReq = "The system SHALL respond within 100ms.";
    bool isTestable = generator.CheckINCOSEQualityRule(testableReq, INCOSEQualityRule::Testability);
    EXPECT_TRUE(isTestable);

    // Test non-testable requirement
    std::string nonTestableReq = "The system SHALL be fast.";
    isTestable = generator.CheckINCOSEQualityRule(nonTestableReq, INCOSEQualityRule::Testability);
    EXPECT_FALSE(isTestable);

    // Test clear requirement
    std::string clearReq = "The system SHALL authenticate users using OAuth 2.0.";
    bool isClear = generator.CheckINCOSEQualityRule(clearReq, INCOSEQualityRule::Clarity);
    EXPECT_TRUE(isClear);

    // Test ambiguous requirement
    std::string ambiguousReq = "The system might possibly authenticate users appropriately.";
    isClear = generator.CheckINCOSEQualityRule(ambiguousReq, INCOSEQualityRule::Clarity);
    EXPECT_FALSE(isClear);

    TestOutput::PrintTestPass("INCOSE quality rule checking");
    return true;
}

/**
 * Test template management functionality
 * Requirements: 1.1, 1.2 (Template loading and management)
 */
bool TestTemplateManagement() {
    TestOutput::PrintTestStart("template management");

    SpecGenerator generator;

    // Test loading engine templates
    generator.LoadEngineTemplates();

    // Test getting available templates for Graphics system
    std::vector<std::string> graphicsTemplates = generator.GetAvailableTemplates("Graphics");
    EXPECT_FALSE(graphicsTemplates.empty());

    // Test custom template registration
    TemplateConfiguration config;
    config.templateName = "custom_test";
    config.targetSystem = "requirements";
    config.placeholders["TEST_PLACEHOLDER"] = "test_value";

    std::string customTemplate = "# Custom Template\n\n{{TEST_PLACEHOLDER}}\n\n{{FEATURE_NAME}}";
    generator.RegisterCustomTemplate("custom_test", customTemplate, config);

    // Test template update
    std::string updatedTemplate = "# Updated Custom Template\n\n{{TEST_PLACEHOLDER}}\n\n{{FEATURE_NAME}}";
    bool updateResult = generator.UpdateTemplate("custom_test", updatedTemplate);
    EXPECT_TRUE(updateResult);

    TestOutput::PrintTestPass("template management");
    return true;
}

/**
 * Test system-specific specification generation
 * Requirements: 1.2 (Engine-specific sections)
 */
bool TestSystemSpecificGeneration() {
    TestOutput::PrintTestStart("system-specific specification generation");

    SpecGenerator generator;

    FeatureRequest request;
    request.featureName = "TestSystemFeature";
    request.description = "A test feature for system-specific generation";
    request.complexity = "Moderate";

    // Test Graphics specification generation
    request.targetSystems = {"Graphics"};
    std::string graphicsSpec = generator.GenerateGraphicsSpecification(request);
    EXPECT_FALSE(graphicsSpec.empty());
    EXPECT_TRUE(graphicsSpec.find("Graphics System Requirements") != std::string::npos);
    EXPECT_TRUE(graphicsSpec.find("PrimitiveRenderer") != std::string::npos);
    EXPECT_TRUE(graphicsSpec.find("OpenGL 4.6+") != std::string::npos);

    // Test Physics specification generation
    std::string physicsSpec = generator.GeneratePhysicsSpecification(request);
    EXPECT_FALSE(physicsSpec.empty());
    EXPECT_TRUE(physicsSpec.find("Physics System Requirements") != std::string::npos);
    EXPECT_TRUE(physicsSpec.find("Bullet Physics") != std::string::npos);
    EXPECT_TRUE(physicsSpec.find("PhysX") != std::string::npos);

    // Test Audio specification generation
    std::string audioSpec = generator.GenerateAudioSpecification(request);
    EXPECT_FALSE(audioSpec.empty());
    EXPECT_TRUE(audioSpec.find("Audio System Requirements") != std::string::npos);
    EXPECT_TRUE(audioSpec.find("OpenAL") != std::string::npos);
    EXPECT_TRUE(audioSpec.find("3D spatial audio") != std::string::npos);

    TestOutput::PrintTestPass("system-specific specification generation");
    return true;
}

/**
 * Test requirements document validation
 * Requirements: 1.4 (EARS and INCOSE validation)
 */
bool TestRequirementsValidation() {
    TestOutput::PrintTestStart("requirements document validation");

    SpecGenerator generator;

    // Create a requirements document with mixed quality
    std::string requirements = R"(
# Test Requirements

## Requirements

1. The system SHALL authenticate users within 2 seconds.
2. The system might possibly handle errors appropriately.
3. WHEN user submits form, the system SHALL validate input data.
4. The system SHALL be user-friendly.
5. WHILE system is running, the system SHALL monitor performance.
)";

    // Test EARS compliance validation
    ValidationResult earsResult = generator.ValidateEARSCompliance(requirements);
    EXPECT_TRUE(earsResult.complianceScore > 0.0);
    EXPECT_TRUE(earsResult.complianceScore <= 1.0);

    // Test INCOSE compliance validation
    ValidationResult incoseResult = generator.ValidateINCOSECompliance(requirements);
    EXPECT_TRUE(incoseResult.complianceScore > 0.0);
    EXPECT_TRUE(incoseResult.complianceScore <= 1.0);

    // Should have some violations due to ambiguous language
    EXPECT_FALSE(incoseResult.violations.empty());

    TestOutput::PrintTestPass("requirements document validation");
    return true;
}

/**
 * Test traceability maintenance
 * Requirements: 1.5 (Traceability between requirements, design, and tasks)
 */
bool TestTraceabilityMaintenance() {
    TestOutput::PrintTestStart("traceability maintenance");

    SpecGenerator generator;

    FeatureRequest request;
    request.featureName = "TraceabilityTest";
    request.description = "A test feature for traceability validation";
    request.targetSystems = {"Graphics"};
    request.complexity = "Simple";
    request.includePropertyTests = true;
    request.includePerformanceProfiling = false;

    GeneratedSpec spec = generator.GenerateFeatureSpec(request);

    // Test traceability matrix generation
    std::map<std::string, std::vector<std::string>> matrix = generator.GenerateTraceabilityMatrix(spec);
    EXPECT_FALSE(matrix.empty());

    // Verify traceability links exist in design document
    EXPECT_TRUE(spec.designDocument.find("Traces to:") != std::string::npos || 
                spec.designDocument.find("Addresses Requirements:") != std::string::npos);

    TestOutput::PrintTestPass("traceability maintenance");
    return true;
}

/**
 * Test spec compliance validation
 * Requirements: 1.1, 1.4 (Spec compliance validation)
 */
bool TestSpecComplianceValidation() {
    TestOutput::PrintTestStart("spec compliance validation");

    SpecGenerator generator;

    FeatureRequest request;
    request.featureName = "ComplianceTest";
    request.description = "A test feature for compliance validation";
    request.targetSystems = {"Graphics"};
    request.complexity = "Simple";
    request.includePropertyTests = true;
    request.includePerformanceProfiling = false;

    GeneratedSpec spec = generator.GenerateFeatureSpec(request);

    // Test compliance validation
    bool isCompliant = generator.ValidateSpecCompliance(spec);
    
    // Should be compliant if no critical errors
    if (spec.validationErrors.empty()) {
        EXPECT_TRUE(isCompliant);
    } else {
        EXPECT_FALSE(isCompliant);
    }

    TestOutput::PrintTestPass("spec compliance validation");
    return true;
}

int main() {
    TestOutput::PrintHeader("SpecGenerator");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("SpecGenerator Tests");

        // Run all tests
        allPassed &= suite.RunTest("SpecGenerator Initialization", TestSpecGeneratorInitialization);
        allPassed &= suite.RunTest("Basic Feature Spec Generation", TestBasicFeatureSpecGeneration);
        allPassed &= suite.RunTest("EARS Pattern Validation", TestEARSPatternValidation);
        allPassed &= suite.RunTest("INCOSE Quality Rules", TestINCOSEQualityRules);
        allPassed &= suite.RunTest("Template Management", TestTemplateManagement);
        allPassed &= suite.RunTest("System-Specific Generation", TestSystemSpecificGeneration);
        allPassed &= suite.RunTest("Requirements Validation", TestRequirementsValidation);
        allPassed &= suite.RunTest("Traceability Maintenance", TestTraceabilityMaintenance);
        allPassed &= suite.RunTest("Spec Compliance Validation", TestSpecComplianceValidation);

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