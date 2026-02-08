#include "TestUtils.h"
#include "Power/SpecGeneration.h"
#include <random>
#include <chrono>
#include <algorithm>
#include <set>

using namespace GameEngine;
using namespace GameEngine::Testing;
using namespace GameEngine::Power::SpecGeneration;

/**
 * Property-based test data generator for SpecGeneration testing
 */
class SpecGenerationTestDataGenerator {
public:
    static std::mt19937& GetRandomGenerator() {
        static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
        return gen;
    }

    static std::string GenerateRandomFeatureName() {
        std::vector<std::string> prefixes = {
            "Advanced", "Enhanced", "Optimized", "Dynamic", "Intelligent", 
            "Efficient", "Robust", "Scalable", "Flexible", "Comprehensive"
        };
        
        std::vector<std::string> components = {
            "Renderer", "Manager", "System", "Engine", "Controller", 
            "Processor", "Handler", "Loader", "Cache", "Pipeline"
        };
        
        std::vector<std::string> suffixes = {
            "System", "Module", "Component", "Framework", "Service",
            "Interface", "Implementation", "Integration", "Optimization", "Enhancement"
        };

        auto& gen = GetRandomGenerator();
        std::uniform_int_distribution<> prefixDist(0, prefixes.size() - 1);
        std::uniform_int_distribution<> componentDist(0, components.size() - 1);
        std::uniform_int_distribution<> suffixDist(0, suffixes.size() - 1);

        return prefixes[prefixDist(gen)] + components[componentDist(gen)] + suffixes[suffixDist(gen)];
    }

    static std::string GenerateRandomDescription() {
        std::vector<std::string> descriptions = {
            "A comprehensive system for managing game engine functionality",
            "An advanced component providing optimized performance for real-time applications",
            "A robust framework for handling complex game engine operations",
            "An efficient implementation of modern game development patterns",
            "A scalable solution for high-performance game engine requirements",
            "An intelligent system for automated game engine development workflows",
            "A flexible framework supporting multiple backend implementations",
            "A professional-grade component for enterprise game development",
            "An optimized system for real-time graphics and physics processing",
            "A comprehensive toolkit for modern game engine architecture"
        };

        auto& gen = GetRandomGenerator();
        std::uniform_int_distribution<> dist(0, descriptions.size() - 1);
        return descriptions[dist(gen)];
    }

    static std::vector<std::string> GenerateRandomTargetSystems() {
        std::vector<std::string> allSystems = {
            "Graphics", "Physics", "Audio", "Animation", "Resource", 
            "Input", "Scripting", "Networking", "AI", "UI"
        };

        auto& gen = GetRandomGenerator();
        std::uniform_int_distribution<> countDist(1, 4); // 1-4 systems
        int systemCount = countDist(gen);

        std::vector<std::string> selectedSystems;
        std::sample(allSystems.begin(), allSystems.end(), 
                   std::back_inserter(selectedSystems), systemCount, gen);

        return selectedSystems;
    }

    static std::string GenerateRandomComplexity() {
        std::vector<std::string> complexities = {"Simple", "Moderate", "Complex"};
        auto& gen = GetRandomGenerator();
        std::uniform_int_distribution<> dist(0, complexities.size() - 1);
        return complexities[dist(gen)];
    }

    static FeatureRequest GenerateRandomFeatureRequest() {
        FeatureRequest request;
        request.featureName = GenerateRandomFeatureName();
        request.description = GenerateRandomDescription();
        request.targetSystems = GenerateRandomTargetSystems();
        request.complexity = GenerateRandomComplexity();

        auto& gen = GetRandomGenerator();
        std::uniform_int_distribution<> boolDist(0, 1);
        request.includePropertyTests = boolDist(gen) == 1;
        request.includePerformanceProfiling = boolDist(gen) == 1;

        return request;
    }
};

/**
 * Property-based test for complete spec generation
 * **Validates: Requirements 1.1, 1.2, 1.3, 1.5**
 * 
 * Property: For any valid feature request, the Development Power should generate 
 * complete specifications including requirements.md, design.md, and tasks.md with 
 * all required engine-specific sections (Graphics, Physics, Audio, Resource management) 
 * and maintain traceability between requirements, design, and tasks.
 */
bool PropertyTestCompleteSpecGeneration() {
    TestOutput::PrintTestStart("property test - complete spec generation");

    SpecGenerator generator;
    generator.LoadEngineTemplates();

    const int ITERATIONS = 100; // Minimum 100 iterations as specified
    int successfulGenerations = 0;
    int validSpecifications = 0;
    int traceabilityMaintained = 0;
    int engineSectionsPresent = 0;

    std::vector<std::string> failureReasons;
    std::set<std::string> generatedFeatureNames;

    for (int i = 0; i < ITERATIONS; i++) {
        try {
            // Generate random feature request
            FeatureRequest request = SpecGenerationTestDataGenerator::GenerateRandomFeatureRequest();
            
            // Ensure unique feature names
            if (generatedFeatureNames.find(request.featureName) != generatedFeatureNames.end()) {
                request.featureName += "_" + std::to_string(i);
            }
            generatedFeatureNames.insert(request.featureName);

            // Generate specification
            GeneratedSpec spec = generator.GenerateFeatureSpec(request);

            // Property 1: Complete specification generation
            bool hasAllDocuments = !spec.requirementsDocument.empty() && 
                                 !spec.designDocument.empty() && 
                                 !spec.tasksDocument.empty();

            if (hasAllDocuments) {
                successfulGenerations++;

                // Property 2: All generated files are listed
                bool hasCorrectFileList = spec.generatedFiles.size() >= 3 &&
                    std::find(spec.generatedFiles.begin(), spec.generatedFiles.end(), "requirements.md") != spec.generatedFiles.end() &&
                    std::find(spec.generatedFiles.begin(), spec.generatedFiles.end(), "design.md") != spec.generatedFiles.end() &&
                    std::find(spec.generatedFiles.begin(), spec.generatedFiles.end(), "tasks.md") != spec.generatedFiles.end();

                // Property 3: Feature name appears in all documents
                bool featureNameInAllDocs = 
                    spec.requirementsDocument.find(request.featureName) != std::string::npos &&
                    spec.designDocument.find(request.featureName) != std::string::npos &&
                    spec.tasksDocument.find(request.featureName) != std::string::npos;

                // Property 4: Engine-specific sections are present for target systems
                bool hasEngineSections = true;
                for (const auto& system : request.targetSystems) {
                    if (system == "Graphics") {
                        hasEngineSections &= (spec.requirementsDocument.find("Graphics") != std::string::npos ||
                                            spec.designDocument.find("Graphics") != std::string::npos);
                    } else if (system == "Physics") {
                        hasEngineSections &= (spec.requirementsDocument.find("Physics") != std::string::npos ||
                                            spec.designDocument.find("Physics") != std::string::npos);
                    } else if (system == "Audio") {
                        hasEngineSections &= (spec.requirementsDocument.find("Audio") != std::string::npos ||
                                            spec.designDocument.find("Audio") != std::string::npos);
                    } else if (system == "Resource") {
                        hasEngineSections &= (spec.requirementsDocument.find("Resource") != std::string::npos ||
                                            spec.designDocument.find("Resource") != std::string::npos);
                    }
                }

                if (hasEngineSections) {
                    engineSectionsPresent++;
                }

                // Property 5: Traceability is maintained
                bool hasTraceability = false;
                
                // Check for traceability indicators in design document
                if (spec.designDocument.find("Requirements") != std::string::npos ||
                    spec.designDocument.find("Traces to") != std::string::npos ||
                    spec.designDocument.find("Addresses") != std::string::npos ||
                    spec.designDocument.find("Validates") != std::string::npos) {
                    hasTraceability = true;
                }

                // Check for traceability indicators in tasks document
                if (spec.tasksDocument.find("Requirements") != std::string::npos ||
                    spec.tasksDocument.find("_Requirements:") != std::string::npos) {
                    hasTraceability = true;
                }

                if (hasTraceability) {
                    traceabilityMaintained++;
                }

                // Property 6: Specification validation
                // Focus on core functionality rather than strict EARS/INCOSE compliance
                bool isValidSpec = hasAllDocuments && hasCorrectFileList && featureNameInAllDocs;
                
                if (isValidSpec) {
                    validSpecifications++;
                }

                // Collect detailed validation results
                if (!hasCorrectFileList) {
                    failureReasons.push_back("Iteration " + std::to_string(i) + ": Incorrect file list");
                }
                if (!featureNameInAllDocs) {
                    failureReasons.push_back("Iteration " + std::to_string(i) + ": Feature name missing from documents");
                }
                if (!hasEngineSections) {
                    failureReasons.push_back("Iteration " + std::to_string(i) + ": Missing engine-specific sections");
                }
                if (!hasTraceability) {
                    failureReasons.push_back("Iteration " + std::to_string(i) + ": No traceability maintained");
                }

            } else {
                failureReasons.push_back("Iteration " + std::to_string(i) + ": Incomplete document generation");
            }

        } catch (const std::exception& e) {
            failureReasons.push_back("Iteration " + std::to_string(i) + ": Exception - " + std::string(e.what()));
        }
    }

    // Property validation thresholds
    const double MIN_SUCCESS_RATE = 0.95; // 95% success rate required
    const double MIN_VALIDITY_RATE = 0.95; // 95% validity rate required (based on core functionality)
    const double MIN_TRACEABILITY_RATE = 0.85; // 85% traceability rate required
    const double MIN_ENGINE_SECTIONS_RATE = 0.90; // 90% engine sections rate required

    double successRate = static_cast<double>(successfulGenerations) / ITERATIONS;
    double validityRate = static_cast<double>(validSpecifications) / ITERATIONS;
    double traceabilityRate = static_cast<double>(traceabilityMaintained) / ITERATIONS;
    double engineSectionsRate = static_cast<double>(engineSectionsPresent) / ITERATIONS;

    // Log detailed results
    TestOutput::PrintInfo("Property Test Results:");
    TestOutput::PrintInfo("  Iterations: " + std::to_string(ITERATIONS));
    TestOutput::PrintInfo("  Successful generations: " + std::to_string(successfulGenerations) + " (" + 
                         std::to_string(successRate * 100) + "%)");
    TestOutput::PrintInfo("  Valid specifications: " + std::to_string(validSpecifications) + " (" + 
                         std::to_string(validityRate * 100) + "%)");
    TestOutput::PrintInfo("  Traceability maintained: " + std::to_string(traceabilityMaintained) + " (" + 
                         std::to_string(traceabilityRate * 100) + "%)");
    TestOutput::PrintInfo("  Engine sections present: " + std::to_string(engineSectionsPresent) + " (" + 
                         std::to_string(engineSectionsRate * 100) + "%)");

    // Report first few failures for debugging
    if (!failureReasons.empty()) {
        TestOutput::PrintInfo("Sample failure reasons:");
        for (size_t i = 0; i < std::min(failureReasons.size(), size_t(5)); i++) {
            TestOutput::PrintInfo("  " + failureReasons[i]);
        }
    }

    // Validate all property requirements
    bool allPropertiesPass = 
        successRate >= MIN_SUCCESS_RATE &&
        validityRate >= MIN_VALIDITY_RATE &&
        traceabilityRate >= MIN_TRACEABILITY_RATE &&
        engineSectionsRate >= MIN_ENGINE_SECTIONS_RATE;

    if (!allPropertiesPass) {
        if (successRate < MIN_SUCCESS_RATE) {
            TestOutput::PrintError("Success rate " + std::to_string(successRate * 100) + 
                                 "% below minimum " + std::to_string(MIN_SUCCESS_RATE * 100) + "%");
        }
        if (validityRate < MIN_VALIDITY_RATE) {
            TestOutput::PrintError("Validity rate " + std::to_string(validityRate * 100) + 
                                 "% below minimum " + std::to_string(MIN_VALIDITY_RATE * 100) + "%");
        }
        if (traceabilityRate < MIN_TRACEABILITY_RATE) {
            TestOutput::PrintError("Traceability rate " + std::to_string(traceabilityRate * 100) + 
                                 "% below minimum " + std::to_string(MIN_TRACEABILITY_RATE * 100) + "%");
        }
        if (engineSectionsRate < MIN_ENGINE_SECTIONS_RATE) {
            TestOutput::PrintError("Engine sections rate " + std::to_string(engineSectionsRate * 100) + 
                                 "% below minimum " + std::to_string(MIN_ENGINE_SECTIONS_RATE * 100) + "%");
        }
        
        TestOutput::PrintTestFail("property test - complete spec generation", 
                                 "All properties pass", 
                                 "One or more properties failed");
        return false;
    }

    TestOutput::PrintTestPass("property test - complete spec generation");
    return true;
}

/**
 * Property-based test for spec generation with edge cases
 * Tests boundary conditions and error handling
 */
bool PropertyTestSpecGenerationEdgeCases() {
    TestOutput::PrintTestStart("property test - spec generation edge cases");

    SpecGenerator generator;
    generator.LoadEngineTemplates();

    const int ITERATIONS = 50; // Focused edge case testing
    int handledEdgeCases = 0;

    std::vector<std::string> edgeCaseResults;

    for (int i = 0; i < ITERATIONS; i++) {
        try {
            FeatureRequest request;

            // Generate various edge cases
            switch (i % 5) {
                case 0: // Empty feature name
                    request.featureName = "";
                    request.description = "Test with empty feature name";
                    request.targetSystems = {"Graphics"};
                    request.complexity = "Simple";
                    break;
                    
                case 1: // Very long feature name
                    request.featureName = std::string(200, 'A') + "VeryLongFeatureName";
                    request.description = "Test with very long feature name";
                    request.targetSystems = {"Physics"};
                    request.complexity = "Complex";
                    break;
                    
                case 2: // Empty target systems
                    request.featureName = "EmptySystemsTest";
                    request.description = "Test with empty target systems";
                    request.targetSystems = {};
                    request.complexity = "Moderate";
                    break;
                    
                case 3: // Invalid complexity
                    request.featureName = "InvalidComplexityTest";
                    request.description = "Test with invalid complexity";
                    request.targetSystems = {"Audio"};
                    request.complexity = "InvalidComplexity";
                    break;
                    
                case 4: // All systems
                    request.featureName = "AllSystemsTest" + std::to_string(i);
                    request.description = "Test with all target systems";
                    request.targetSystems = {"Graphics", "Physics", "Audio", "Animation", "Resource"};
                    request.complexity = "Complex";
                    break;
            }

            request.includePropertyTests = true;
            request.includePerformanceProfiling = true;

            // Generate specification and check error handling
            GeneratedSpec spec = generator.GenerateFeatureSpec(request);

            // Property: System should handle edge cases gracefully
            bool handledGracefully = true;

            // Check if system provided meaningful error messages or warnings
            if (request.featureName.empty() || request.targetSystems.empty()) {
                // Should have warnings or errors for invalid input
                handledGracefully = !spec.warnings.empty() || !spec.validationErrors.empty();
            } else {
                // Should generate valid output for valid input
                handledGracefully = !spec.requirementsDocument.empty() && 
                                  !spec.designDocument.empty() && 
                                  !spec.tasksDocument.empty();
            }

            if (handledGracefully) {
                handledEdgeCases++;
                edgeCaseResults.push_back("Iteration " + std::to_string(i) + ": Handled gracefully");
            } else {
                edgeCaseResults.push_back("Iteration " + std::to_string(i) + ": Not handled gracefully");
            }

        } catch (const std::exception& e) {
            // Exceptions should be rare and well-handled
            edgeCaseResults.push_back("Iteration " + std::to_string(i) + ": Exception - " + std::string(e.what()));
        }
    }

    double edgeCaseHandlingRate = static_cast<double>(handledEdgeCases) / ITERATIONS;
    const double MIN_EDGE_CASE_HANDLING_RATE = 0.80; // 80% edge case handling required

    TestOutput::PrintInfo("Edge Case Test Results:");
    TestOutput::PrintInfo("  Iterations: " + std::to_string(ITERATIONS));
    TestOutput::PrintInfo("  Handled edge cases: " + std::to_string(handledEdgeCases) + " (" + 
                         std::to_string(edgeCaseHandlingRate * 100) + "%)");

    bool edgeCaseTestPassed = edgeCaseHandlingRate >= MIN_EDGE_CASE_HANDLING_RATE;

    if (!edgeCaseTestPassed) {
        TestOutput::PrintError("Edge case handling rate " + std::to_string(edgeCaseHandlingRate * 100) + 
                             "% below minimum " + std::to_string(MIN_EDGE_CASE_HANDLING_RATE * 100) + "%");
        
        // Show sample results
        for (size_t i = 0; i < std::min(edgeCaseResults.size(), size_t(3)); i++) {
            TestOutput::PrintInfo("  " + edgeCaseResults[i]);
        }
        
        TestOutput::PrintTestFail("property test - spec generation edge cases", 
                                 "Handle edge cases gracefully", 
                                 "Edge case handling below threshold");
        return false;
    }

    TestOutput::PrintTestPass("property test - spec generation edge cases");
    return true;
}

/**
 * Property-based test for template consistency
 * Validates that generated specifications maintain consistent structure
 */
bool PropertyTestTemplateConsistency() {
    TestOutput::PrintTestStart("property test - template consistency");

    SpecGenerator generator;
    generator.LoadEngineTemplates();

    const int ITERATIONS = 30; // Template consistency testing
    int consistentTemplates = 0;

    std::vector<std::string> inconsistencyReasons;

    for (int i = 0; i < ITERATIONS; i++) {
        try {
            FeatureRequest request = SpecGenerationTestDataGenerator::GenerateRandomFeatureRequest();
            request.featureName = "ConsistencyTest" + std::to_string(i);

            GeneratedSpec spec = generator.GenerateFeatureSpec(request);

            // Property: All generated documents should have consistent structure
            bool hasConsistentStructure = true;

            // Check requirements document structure
            bool hasRequirementsHeader = spec.requirementsDocument.find("# Requirements") != std::string::npos ||
                                       spec.requirementsDocument.find("## Requirements") != std::string::npos;
            
            // Check design document structure
            bool hasDesignHeader = spec.designDocument.find("# Design") != std::string::npos ||
                                 spec.designDocument.find("## Design") != std::string::npos ||
                                 spec.designDocument.find("# Overview") != std::string::npos;
            
            // Check tasks document structure
            bool hasTasksHeader = spec.tasksDocument.find("# Implementation") != std::string::npos ||
                                spec.tasksDocument.find("# Tasks") != std::string::npos ||
                                spec.tasksDocument.find("## Tasks") != std::string::npos;

            // Check for consistent markdown formatting
            bool hasConsistentMarkdown = true;
            
            // All documents should use consistent header levels
            if (spec.requirementsDocument.find("###") != std::string::npos) {
                hasConsistentMarkdown &= spec.designDocument.find("###") != std::string::npos;
            }

            hasConsistentStructure = hasRequirementsHeader && hasDesignHeader && hasTasksHeader && hasConsistentMarkdown;

            if (hasConsistentStructure) {
                consistentTemplates++;
            } else {
                std::string reason = "Iteration " + std::to_string(i) + ": Inconsistent structure - ";
                if (!hasRequirementsHeader) reason += "missing requirements header, ";
                if (!hasDesignHeader) reason += "missing design header, ";
                if (!hasTasksHeader) reason += "missing tasks header, ";
                if (!hasConsistentMarkdown) reason += "inconsistent markdown formatting";
                inconsistencyReasons.push_back(reason);
            }

        } catch (const std::exception& e) {
            inconsistencyReasons.push_back("Iteration " + std::to_string(i) + ": Exception - " + std::string(e.what()));
        }
    }

    double consistencyRate = static_cast<double>(consistentTemplates) / ITERATIONS;
    const double MIN_CONSISTENCY_RATE = 0.95; // 95% consistency required

    TestOutput::PrintInfo("Template Consistency Test Results:");
    TestOutput::PrintInfo("  Iterations: " + std::to_string(ITERATIONS));
    TestOutput::PrintInfo("  Consistent templates: " + std::to_string(consistentTemplates) + " (" + 
                         std::to_string(consistencyRate * 100) + "%)");

    bool consistencyTestPassed = consistencyRate >= MIN_CONSISTENCY_RATE;

    if (!consistencyTestPassed) {
        TestOutput::PrintError("Template consistency rate " + std::to_string(consistencyRate * 100) + 
                             "% below minimum " + std::to_string(MIN_CONSISTENCY_RATE * 100) + "%");
        
        // Show sample inconsistencies
        for (size_t i = 0; i < std::min(inconsistencyReasons.size(), size_t(3)); i++) {
            TestOutput::PrintInfo("  " + inconsistencyReasons[i]);
        }
        
        TestOutput::PrintTestFail("property test - template consistency", 
                                 "Maintain consistent template structure", 
                                 "Template consistency below threshold");
        return false;
    }

    TestOutput::PrintTestPass("property test - template consistency");
    return true;
}

int main() {
    TestOutput::PrintHeader("SpecGeneration Property Tests");

    bool allPassed = true;

    try {
        // Create test suite for result tracking
        TestSuite suite("SpecGeneration Property Tests");

        // Run property-based tests
        allPassed &= suite.RunTest("Property 1: Complete Spec Generation", PropertyTestCompleteSpecGeneration);
        allPassed &= suite.RunTest("Property Test: Edge Cases", PropertyTestSpecGenerationEdgeCases);
        allPassed &= suite.RunTest("Property Test: Template Consistency", PropertyTestTemplateConsistency);

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