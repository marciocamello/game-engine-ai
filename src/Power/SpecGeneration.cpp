#include "Power/SpecGeneration.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include <chrono>
#include <iomanip>

namespace GameEngine::Power::SpecGeneration {

SpecGenerator::SpecGenerator() {
    LoadEngineTemplates();
    InitializeEARSPatterns();
    InitializeINCOSERules();
    LoadSystemTemplates();
}

SpecGenerator::~SpecGenerator() = default;

GeneratedSpec SpecGenerator::GenerateFeatureSpec(const FeatureRequest& request) {
    GeneratedSpec spec;
    spec.isValid = false;

    try {
        // Generate requirements document first
        spec.requirementsDocument = GenerateRequirementsDocument(request);
        
        // Generate design document based on requirements
        spec.designDocument = GenerateDesignDocument(request, spec.requirementsDocument);
        
        // Generate tasks document based on design
        spec.tasksDocument = GenerateTasksDocument(request, spec.designDocument);

        // Validate generated specification
        ValidationResult requirementsValidation = ValidateEARSCompliance(spec.requirementsDocument);
        ValidationResult incoseValidation = ValidateINCOSECompliance(spec.requirementsDocument);

        // Collect validation warnings and errors
        spec.warnings.insert(spec.warnings.end(), requirementsValidation.warnings.begin(), requirementsValidation.warnings.end());
        spec.warnings.insert(spec.warnings.end(), incoseValidation.warnings.begin(), incoseValidation.warnings.end());

        if (!requirementsValidation.isCompliant) {
            spec.validationErrors.insert(spec.validationErrors.end(), requirementsValidation.violations.begin(), requirementsValidation.violations.end());
        }

        if (!incoseValidation.isCompliant) {
            spec.validationErrors.insert(spec.validationErrors.end(), incoseValidation.violations.begin(), incoseValidation.violations.end());
        }

        // Maintain traceability between documents
        spec = MaintainTraceability(spec);

        // Mark as valid if no critical errors
        spec.isValid = spec.validationErrors.empty();

        // Add generated file paths
        spec.generatedFiles.push_back("requirements.md");
        spec.generatedFiles.push_back("design.md");
        spec.generatedFiles.push_back("tasks.md");

    } catch (const std::exception& e) {
        spec.validationErrors.push_back("Exception during spec generation: " + std::string(e.what()));
        spec.isValid = false;
    }

    return spec;
}

bool SpecGenerator::ValidateSpecCompliance(const GeneratedSpec& spec) {
    if (!spec.isValid) {
        return false;
    }

    // Check that all required documents are present
    if (spec.requirementsDocument.empty() || spec.designDocument.empty() || spec.tasksDocument.empty()) {
        return false;
    }

    // Validate EARS compliance
    ValidationResult earsResult = ValidateEARSCompliance(spec.requirementsDocument);
    if (!earsResult.isCompliant) {
        return false;
    }

    // Validate INCOSE compliance
    ValidationResult incoseResult = ValidateINCOSECompliance(spec.requirementsDocument);
    if (!incoseResult.isCompliant) {
        return false;
    }

    return true;
}

std::string SpecGenerator::GenerateRequirementsDocument(const FeatureRequest& request) {
    std::stringstream ss;
    auto placeholders = GenerateDefaultPlaceholders(request);

    // Get appropriate template based on complexity and target systems
    std::string templateName = "requirements_" + request.complexity;
    if (m_requirementsTemplates.find(templateName) == m_requirementsTemplates.end()) {
        templateName = "requirements_default";
    }

    std::string templateContent = m_requirementsTemplates[templateName];

    // Process system-specific sections
    std::string systemSections;
    for (const auto& system : request.targetSystems) {
        if (system == "Graphics") {
            systemSections += GenerateGraphicsSpecification(request);
        } else if (system == "Physics") {
            systemSections += GeneratePhysicsSpecification(request);
        } else if (system == "Audio") {
            systemSections += GenerateAudioSpecification(request);
        } else if (system == "Resource") {
            systemSections += GenerateResourceSpecification(request);
        } else if (system == "Animation") {
            systemSections += GenerateAnimationSpecification(request);
        }
    }

    placeholders["SYSTEM_SPECIFIC_SECTIONS"] = systemSections;

    // Process template with placeholders
    std::string processedContent = ProcessTemplatePlaceholders(templateContent, placeholders);

    return processedContent;
}

std::string SpecGenerator::GenerateDesignDocument(const FeatureRequest& request, const std::string& requirements) {
    std::stringstream ss;
    auto placeholders = GenerateDefaultPlaceholders(request);

    // Extract requirements for traceability
    auto extractedRequirements = ExtractRequirements(requirements);
    
    // Generate comprehensive design sections based on requirements
    std::string designSections;
    
    // Group requirements by system for better organization
    std::map<std::string, std::vector<std::pair<int, std::string>>> requirementsBySystem;
    
    for (size_t i = 0; i < extractedRequirements.size(); ++i) {
        std::string req = extractedRequirements[i];
        bool assigned = false;
        
        // Categorize requirement by target system
        for (const auto& system : request.targetSystems) {
            if (req.find(system) != std::string::npos) {
                requirementsBySystem[system].push_back({static_cast<int>(i + 1), req});
                assigned = true;
                break;
            }
        }
        
        // If not system-specific, add to general category
        if (!assigned) {
            requirementsBySystem["General"].push_back({static_cast<int>(i + 1), req});
        }
    }
    
    // Generate design sections organized by system
    for (const auto& systemPair : requirementsBySystem) {
        const std::string& systemName = systemPair.first;
        const auto& systemRequirements = systemPair.second;
        
        designSections += "\n## " + systemName + " Design\n\n";
        designSections += "### Architecture Overview\n\n";
        designSections += "The " + systemName + " component of " + request.featureName + " provides:\n\n";
        
        // List addressed requirements
        for (const auto& reqPair : systemRequirements) {
            std::string reqId = GenerateRequirementId(request.featureName, reqPair.first);
            designSections += "- **" + reqId + "**: " + reqPair.second.substr(0, 100);
            if (reqPair.second.length() > 100) designSections += "...";
            designSections += "\n";
        }
        
        designSections += "\n### Component Structure\n\n";
        designSections += "```cpp\n";
        designSections += "namespace GameEngine::" + systemName + "::" + placeholders["MODULE_NAMESPACE"] + " {\n\n";
        designSections += "class " + placeholders["MAIN_CLASS_NAME"] + systemName + " {\n";
        designSections += "public:\n";
        designSections += "    " + placeholders["MAIN_CLASS_NAME"] + systemName + "();\n";
        designSections += "    ~" + placeholders["MAIN_CLASS_NAME"] + systemName + "();\n\n";
        designSections += "    // Core functionality\n";
        designSections += "    bool Initialize();\n";
        designSections += "    void Update(float deltaTime);\n";
        designSections += "    void Shutdown();\n\n";
        designSections += "private:\n";
        designSections += "    // Implementation details\n";
        designSections += "};\n\n";
        designSections += "} // namespace GameEngine::" + systemName + "::" + placeholders["MODULE_NAMESPACE"] + "\n";
        designSections += "```\n\n";
        
        designSections += "### Implementation Strategy\n\n";
        designSections += "The " + systemName + " implementation follows Game Engine Kiro patterns:\n\n";
        designSections += "1. **Initialization**: Set up " + systemName + " resources and validate configuration\n";
        designSections += "2. **Update Loop**: Process " + systemName + " operations each frame\n";
        designSections += "3. **Resource Management**: Efficient handling of " + systemName + " assets\n";
        designSections += "4. **Error Handling**: Graceful degradation with detailed logging\n";
        designSections += "5. **Integration**: Seamless connection with existing " + systemName + " systems\n\n";
        
        designSections += "**Traces to Requirements:** ";
        for (size_t i = 0; i < systemRequirements.size(); ++i) {
            if (i > 0) designSections += ", ";
            designSections += GenerateRequirementId(request.featureName, systemRequirements[i].first);
        }
        designSections += "\n\n";
    }

    placeholders["DESIGN_SECTIONS"] = designSections;
    placeholders["REQUIREMENTS_TRACEABILITY"] = requirements.substr(0, 500) + "..."; // Summary for traceability

    // Get design template
    std::string templateName = "design_" + request.complexity;
    if (m_designTemplates.find(templateName) == m_designTemplates.end()) {
        templateName = "design_default";
    }

    std::string templateContent = m_designTemplates[templateName];
    std::string processedContent = ProcessTemplatePlaceholders(templateContent, placeholders);

    return processedContent;
}

std::string SpecGenerator::GenerateTasksDocument(const FeatureRequest& request, const std::string& design) {
    std::stringstream ss;
    auto placeholders = GenerateDefaultPlaceholders(request);

    // Extract design sections to create detailed task breakdown
    std::string taskSections;
    int taskCounter = 1;
    int subTaskCounter = 1;

    // Parse design document to extract components and systems
    std::map<std::string, std::vector<std::string>> componentsBySystem;
    
    for (const auto& system : request.targetSystems) {
        // Find system-specific design sections
        size_t systemPos = design.find("## " + system + " Design");
        if (systemPos != std::string::npos) {
            componentsBySystem[system].push_back(placeholders["MAIN_CLASS_NAME"] + system);
        }
    }
    
    // If no specific systems found, use general approach
    if (componentsBySystem.empty()) {
        componentsBySystem["Core"].push_back(placeholders["MAIN_CLASS_NAME"]);
    }

    // Generate main implementation tasks organized by system
    taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Implement core " + request.featureName + " architecture\n";
    
    for (const auto& systemPair : componentsBySystem) {
        const std::string& systemName = systemPair.first;
        const auto& components = systemPair.second;
        
        for (const auto& component : components) {
            taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Create " + component + " class\n";
            taskSections += "    - Implement header file in include/" + systemName + "/" + component + ".h\n";
            taskSections += "    - Create implementation in src/" + systemName + "/" + component + ".cpp\n";
            taskSections += "    - Follow Game Engine Kiro naming conventions and namespace rules\n";
            taskSections += "    - Add proper documentation and comments\n";
            taskSections += "    - _Traces to: Design " + systemName + " section_\n";
        }
    }
    taskSections += "\n";
    subTaskCounter = 1;

    // System-specific integration tasks
    for (const auto& system : request.targetSystems) {
        taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Implement " + system + " system integration\n";
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Create " + system + "-specific components\n";
        taskSections += "    - Implement " + system + " integration interfaces\n";
        taskSections += "    - Add compatibility with existing " + system + " systems\n";
        taskSections += "    - Ensure proper resource management\n";
        taskSections += "    - _Traces to: " + system + " requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Add " + system + " error handling\n";
        taskSections += "    - Implement graceful degradation strategies\n";
        taskSections += "    - Add comprehensive error logging\n";
        taskSections += "    - Create fallback mechanisms\n";
        taskSections += "    - _Traces to: Error handling requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Validate " + system + " integration\n";
        taskSections += "    - Test integration with existing " + system + " components\n";
        taskSections += "    - Verify backward compatibility\n";
        taskSections += "    - Ensure performance standards are met\n";
        taskSections += "    - _Traces to: Integration requirements_\n";
        taskSections += "\n";
        subTaskCounter = 1;
    }

    // Testing tasks with detailed breakdown
    if (request.includePropertyTests) {
        taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Create comprehensive test suite\n";
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Write unit tests\n";
        taskSections += "    - Create test file in tests/unit/test_" + placeholders["MAIN_CLASS_NAME"] + ".cpp\n";
        taskSections += "    - Follow testing-standards.md template exactly\n";
        taskSections += "    - Test individual component functionality\n";
        taskSections += "    - Validate error conditions and edge cases\n";
        taskSections += "    - _Traces to: Testing requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Create property-based tests\n";
        taskSections += "    - Implement property tests with minimum 100 iterations\n";
        taskSections += "    - Test universal properties across all valid inputs\n";
        taskSections += "    - Ensure proper TestUtils.h integration\n";
        taskSections += "    - Tag tests with feature and property references\n";
        taskSections += "    - _Traces to: Property testing requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Validate test coverage\n";
        taskSections += "    - Ensure all requirements have corresponding tests\n";
        taskSections += "    - Verify test pass rate is 100%\n";
        taskSections += "    - Check code coverage metrics\n";
        taskSections += "    - _Traces to: Quality assurance requirements_\n";
        taskSections += "\n";
        subTaskCounter = 1;
    }

    // Performance profiling tasks
    if (request.includePerformanceProfiling) {
        taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Implement performance monitoring and optimization\n";
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Add CPU performance profiling\n";
        taskSections += "    - Implement execution time measurement\n";
        taskSections += "    - Add CPU usage tracking\n";
        taskSections += "    - Create performance benchmarks\n";
        taskSections += "    - _Traces to: Performance requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Add GPU performance profiling\n";
        taskSections += "    - Implement render time measurement\n";
        taskSections += "    - Track GPU usage and VRAM\n";
        taskSections += "    - Monitor draw calls and state changes\n";
        taskSections += "    - _Traces to: Graphics performance requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Implement memory profiling\n";
        taskSections += "    - Add memory allocation tracking\n";
        taskSections += "    - Detect memory leaks\n";
        taskSections += "    - Monitor memory fragmentation\n";
        taskSections += "    - _Traces to: Memory management requirements_\n";
        
        taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Generate optimization suggestions\n";
        taskSections += "    - Analyze performance bottlenecks\n";
        taskSections += "    - Create optimization recommendations\n";
        taskSections += "    - Implement critical optimizations\n";
        taskSections += "    - _Traces to: Optimization requirements_\n";
        taskSections += "\n";
        subTaskCounter = 1;
    }

    // Documentation tasks
    taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Create comprehensive documentation\n";
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Write API documentation\n";
    taskSections += "    - Document all public interfaces\n";
    taskSections += "    - Add usage examples\n";
    taskSections += "    - Create integration guides\n";
    taskSections += "    - _Traces to: Documentation requirements_\n";
    
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Generate architecture diagrams\n";
    taskSections += "    - Create Mermaid diagrams for complex systems\n";
    taskSections += "    - Document component relationships\n";
    taskSections += "    - Illustrate data flow\n";
    taskSections += "    - _Traces to: Architecture documentation requirements_\n";
    
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Create troubleshooting guide\n";
    taskSections += "    - Document common issues and solutions\n";
    taskSections += "    - Add debugging tips\n";
    taskSections += "    - Include error message reference\n";
    taskSections += "    - _Traces to: Support documentation requirements_\n";
    taskSections += "\n";
    subTaskCounter = 1;

    // Build system integration
    taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Integrate with build system\n";
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Update CMakeLists.txt\n";
    taskSections += "    - Add new source files to build\n";
    taskSections += "    - Configure test targets\n";
    taskSections += "    - Set up dependencies\n";
    taskSections += "    - _Traces to: Build system requirements_\n";
    
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Verify compilation\n";
    taskSections += "    - Run build_unified.bat --tests\n";
    taskSections += "    - Fix any compilation errors\n";
    taskSections += "    - Ensure no warnings\n";
    taskSections += "    - _Traces to: Quality assurance requirements_\n";
    taskSections += "\n";
    subTaskCounter = 1;

    // Final validation checkpoint
    taskSections += "- [ ] " + std::to_string(taskCounter++) + ". Final integration and validation checkpoint\n";
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Run complete test suite\n";
    taskSections += "    - Execute all unit tests\n";
    taskSections += "    - Run all property-based tests\n";
    taskSections += "    - Verify 100% pass rate\n";
    taskSections += "    - _Traces to: All testing requirements_\n";
    
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Validate against requirements\n";
    taskSections += "    - Verify all requirements are implemented\n";
    taskSections += "    - Check traceability matrix completeness\n";
    taskSections += "    - Ensure all acceptance criteria are met\n";
    taskSections += "    - _Traces to: All requirements_\n";
    
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Performance validation\n";
    taskSections += "    - Verify performance meets specifications\n";
    taskSections += "    - Check memory usage is within limits\n";
    taskSections += "    - Validate frame rate requirements\n";
    taskSections += "    - _Traces to: Performance requirements_\n";
    
    taskSections += "  - [ ] " + std::to_string(taskCounter) + "." + std::to_string(subTaskCounter++) + " Integration testing\n";
    taskSections += "    - Test with actual Game Engine Kiro scenarios\n";
    taskSections += "    - Verify backward compatibility\n";
    taskSections += "    - Ensure seamless integration with existing systems\n";
    taskSections += "    - _Traces to: Integration requirements_\n";

    placeholders["TASK_SECTIONS"] = taskSections;
    placeholders["DESIGN_TRACEABILITY"] = design.substr(0, 500) + "..."; // Summary for traceability

    // Get tasks template
    std::string templateName = "tasks_" + request.complexity;
    if (m_tasksTemplates.find(templateName) == m_tasksTemplates.end()) {
        templateName = "tasks_default";
    }

    std::string templateContent = m_tasksTemplates[templateName];
    std::string processedContent = ProcessTemplatePlaceholders(templateContent, placeholders);

    return processedContent;
}

void SpecGenerator::LoadEngineTemplates() {
    // Load default requirements template
    m_requirementsTemplates["requirements_default"] = R"(# Requirements Document: {{FEATURE_NAME}}

## Introduction

{{FEATURE_DESCRIPTION}}

This feature is designed for Game Engine Kiro, targeting the following systems: {{TARGET_SYSTEMS}}.

## Glossary

{{GLOSSARY_TERMS}}

## Requirements

{{SYSTEM_SPECIFIC_SECTIONS}}

## Quality Attributes

### Performance Requirements
- THE system SHALL maintain 60+ FPS during normal operation
- THE system SHALL use less than 100MB additional memory
- THE system SHALL initialize within 100ms

### Reliability Requirements  
- THE system SHALL handle errors gracefully without crashing
- THE system SHALL provide meaningful error messages
- THE system SHALL maintain backward compatibility

### Maintainability Requirements
- THE system SHALL follow Game Engine Kiro naming conventions
- THE system SHALL include comprehensive documentation
- THE system SHALL provide unit and property-based tests
)";

    // Load default design template
    m_designTemplates["design_default"] = R"(# Design Document: {{FEATURE_NAME}}

## Overview

{{FEATURE_DESCRIPTION}}

This design addresses the requirements specified in the requirements document and provides a comprehensive implementation strategy for Game Engine Kiro.

## Architecture

{{DESIGN_SECTIONS}}

## Error Handling

The system implements comprehensive error handling following Game Engine Kiro patterns:

- Graceful degradation for non-critical failures
- Detailed error logging with context
- Recovery mechanisms where possible
- Clear error reporting to calling systems

## Testing Strategy

### Unit Tests
- Test individual component functionality
- Validate error conditions and edge cases
- Ensure proper resource cleanup

### Property-Based Tests
- Validate universal properties across all inputs
- Test with randomized data within valid ranges
- Minimum 100 iterations per property test

## Performance Considerations

- Memory-efficient data structures
- Cache-friendly access patterns
- Minimal dynamic allocations
- OpenGL state management optimization
)";

    // Load default tasks template
    m_tasksTemplates["tasks_default"] = R"(# Implementation Plan: {{FEATURE_NAME}}

## Overview

This implementation plan converts the {{FEATURE_NAME}} design into discrete coding tasks that integrate seamlessly with Game Engine Kiro's existing architecture.

## Tasks

{{TASK_SECTIONS}}

## Notes

- All tasks follow Game Engine Kiro conventions and standards
- Each task includes proper testing and validation
- Implementation maintains backward compatibility
- All generated code must compile successfully
- Tests must pass before task completion
)";
}

void SpecGenerator::RegisterCustomTemplate(const std::string& name, const std::string& content, const TemplateConfiguration& config) {
    if (CheckTemplateConflicts(name) && ValidateTemplate(content)) {
        if (config.targetSystem == "requirements") {
            m_requirementsTemplates[name] = content;
        } else if (config.targetSystem == "design") {
            m_designTemplates[name] = content;
        } else if (config.targetSystem == "tasks") {
            m_tasksTemplates[name] = content;
        }
        m_templateConfigurations[name] = config;
    }
}

ValidationResult SpecGenerator::ValidateEARSCompliance(const std::string& requirements) {
    ValidationResult result;
    result.isCompliant = true;
    result.complianceScore = 1.0;

    auto extractedRequirements = ExtractRequirements(requirements);
    
    if (extractedRequirements.empty()) {
        result.isCompliant = true; // No requirements to validate
        return result;
    }
    
    for (const auto& req : extractedRequirements) {
        EARSPattern pattern = ClassifyEARSPattern(req);
        
        // Check if requirement has proper SHALL/MUST keyword
        std::string upperReq = req;
        std::transform(upperReq.begin(), upperReq.end(), upperReq.begin(), ::toupper);
        
        bool hasShall = upperReq.find("SHALL") != std::string::npos;
        bool hasMust = upperReq.find("MUST") != std::string::npos;
        
        if (!hasShall && !hasMust) {
            result.isCompliant = false;
            result.violations.push_back("Requirement missing SHALL/MUST keyword: " + req.substr(0, 100) + "...");
            result.suggestions.push_back("Add SHALL or MUST to make requirement mandatory");
        }
        
        // Validate pattern-specific structure
        switch (pattern) {
            case EARSPattern::EventDriven:
                // Should have trigger (WHEN/IF/ON) and action (SHALL)
                if (!hasShall && !hasMust) {
                    result.violations.push_back("Event-driven requirement missing SHALL/MUST: " + req.substr(0, 100) + "...");
                }
                break;
                
            case EARSPattern::StateDriven:
                // Should have state (WHILE/DURING) and action (SHALL)
                if (!hasShall && !hasMust) {
                    result.violations.push_back("State-driven requirement missing SHALL/MUST: " + req.substr(0, 100) + "...");
                }
                break;
                
            case EARSPattern::UnwantedEvent:
                // Should have condition (IF) with error/unwanted and action (SHALL)
                if (!hasShall && !hasMust) {
                    result.violations.push_back("Unwanted event requirement missing SHALL/MUST: " + req.substr(0, 100) + "...");
                }
                break;
                
            case EARSPattern::OptionalFeature:
                // Should have condition (WHERE/OPTIONAL) and action (SHALL)
                if (!hasShall && !hasMust) {
                    result.violations.push_back("Optional feature requirement missing SHALL/MUST: " + req.substr(0, 100) + "...");
                }
                break;
                
            case EARSPattern::Ubiquitous:
            case EARSPattern::Complex:
                // Already validated above
                break;
        }
    }

    // Calculate compliance score
    if (!result.violations.empty()) {
        result.complianceScore = std::max(0.0, 1.0 - (static_cast<double>(result.violations.size()) / extractedRequirements.size()));
        result.isCompliant = false;
    }

    return result;
}

ValidationResult SpecGenerator::ValidateINCOSECompliance(const std::string& requirements) {
    ValidationResult result;
    result.isCompliant = true;
    result.complianceScore = 1.0;

    auto extractedRequirements = ExtractRequirements(requirements);
    
    for (const auto& req : extractedRequirements) {
        // Check clarity - no ambiguous words
        std::vector<std::string> ambiguousWords = {"maybe", "possibly", "might", "could", "should", "appropriate", "reasonable"};
        for (const auto& word : ambiguousWords) {
            if (req.find(word) != std::string::npos) {
                result.violations.push_back("Requirement contains ambiguous language: " + word + " in: " + req.substr(0, 100) + "...");
                result.suggestions.push_back("Replace ambiguous terms with specific, measurable criteria");
                result.isCompliant = false;
            }
        }

        // Check testability - must have measurable criteria
        if (!CheckINCOSEQualityRule(req, INCOSEQualityRule::Testability)) {
            result.violations.push_back("Requirement is not testable: " + req.substr(0, 100) + "...");
            result.suggestions.push_back("Add specific, measurable acceptance criteria");
            result.isCompliant = false;
        }

        // Check for negative statements (avoid negative requirements)
        std::string upperReq = req;
        std::transform(upperReq.begin(), upperReq.end(), upperReq.begin(), ::toupper);
        
        if (upperReq.find("SHALL NOT") != std::string::npos || upperReq.find("MUST NOT") != std::string::npos) {
            // Only warn if there's no exception clause
            if (upperReq.find("EXCEPT") == std::string::npos && upperReq.find("UNLESS") == std::string::npos) {
                result.warnings.push_back("Consider rephrasing negative requirement as positive: " + req.substr(0, 100) + "...");
            }
        }
    }

    // Calculate compliance score
    if (!result.violations.empty()) {
        result.complianceScore = std::max(0.0, 1.0 - (static_cast<double>(result.violations.size()) / extractedRequirements.size()));
    }

    return result;
}

EARSPattern SpecGenerator::ClassifyEARSPattern(const std::string& requirementText) {
    std::string upperReq = requirementText;
    std::transform(upperReq.begin(), upperReq.end(), upperReq.begin(), ::toupper);

    // Check for complex patterns first (most specific)
    bool hasWhen = upperReq.find(" WHEN ") != std::string::npos || upperReq.find("WHEN ") == 0;
    bool hasWhile = upperReq.find(" WHILE ") != std::string::npos || upperReq.find("WHILE ") == 0;
    bool hasIf = upperReq.find(" IF ") != std::string::npos || upperReq.find("IF ") == 0;
    bool hasAnd = upperReq.find(" AND ") != std::string::npos;
    bool hasOr = upperReq.find(" OR ") != std::string::npos;
    bool hasShall = upperReq.find("SHALL") != std::string::npos;
    
    // Complex: multiple temporal/conditional keywords or AND/OR combinations
    if (hasShall && (((hasWhen || hasIf) && hasWhile) || (hasAnd && hasOr))) {
        return EARSPattern::Complex;
    }
    
    // Check for optional feature patterns
    if (hasShall && ((upperReq.find(" WHERE ") != std::string::npos || upperReq.find("WHERE ") == 0) ||
         upperReq.find("OPTIONAL") != std::string::npos)) {
        return EARSPattern::OptionalFeature;
    }
    
    // Check for unwanted event patterns (IF with error/unwanted keywords)
    if (hasShall && hasIf && 
        (upperReq.find("UNWANTED") != std::string::npos || 
         upperReq.find("ERROR") != std::string::npos ||
         upperReq.find("FAILURE") != std::string::npos)) {
        return EARSPattern::UnwantedEvent;
    }
    
    // Check for state-driven patterns
    if (hasShall && (hasWhile || upperReq.find(" DURING ") != std::string::npos || upperReq.find("DURING ") == 0)) {
        return EARSPattern::StateDriven;
    }
    
    // Check for event-driven patterns (WHEN, IF, ON)
    if (hasShall && (hasWhen || hasIf || 
        upperReq.find(" ON ") != std::string::npos || upperReq.find("ON ") == 0 ||
        upperReq.find(" UPON ") != std::string::npos || upperReq.find("UPON ") == 0)) {
        return EARSPattern::EventDriven;
    }
    
    // Default to ubiquitous if it has SHALL/MUST but no specific patterns
    if (hasShall || upperReq.find("MUST") != std::string::npos) {
        return EARSPattern::Ubiquitous;
    }
    
    // If no SHALL/MUST, still default to ubiquitous
    return EARSPattern::Ubiquitous;
}

bool SpecGenerator::CheckINCOSEQualityRule(const std::string& requirementText, INCOSEQualityRule rule) {
    switch (rule) {
        case INCOSEQualityRule::Testability:
        {
            // Check for measurable criteria
            std::string upperReq = requirementText;
            std::transform(upperReq.begin(), upperReq.end(), upperReq.begin(), ::toupper);
            
            bool hasShall = upperReq.find("SHALL") != std::string::npos || upperReq.find("MUST") != std::string::npos;
            bool hasMeasurableCriteria = 
                upperReq.find("WITHIN") != std::string::npos ||
                upperReq.find("LESS THAN") != std::string::npos ||
                upperReq.find("GREATER THAN") != std::string::npos ||
                upperReq.find("EQUAL TO") != std::string::npos ||
                upperReq.find("AT LEAST") != std::string::npos ||
                upperReq.find("AT MOST") != std::string::npos ||
                upperReq.find("BETWEEN") != std::string::npos ||
                std::regex_search(requirementText, std::regex(R"(\d+\+)")) || // Matches "60+"
                std::regex_search(requirementText, std::regex(R"(\d+)"));     // Matches any number
            
            return hasShall && hasMeasurableCriteria;
        }
        
        case INCOSEQualityRule::Clarity:
        {
            // Check for clear, unambiguous language
            std::vector<std::string> ambiguousWords = {"maybe", "possibly", "might", "could", "appropriate", "reasonable"};
            for (const auto& word : ambiguousWords) {
                if (requirementText.find(word) != std::string::npos) {
                    return false;
                }
            }
            return true;
        }
        
        case INCOSEQualityRule::Completeness:
        {
            // Check for complete information (has subject, action, object)
            std::string upperReq = requirementText;
            std::transform(upperReq.begin(), upperReq.end(), upperReq.begin(), ::toupper);
            bool hasShall = upperReq.find("SHALL") != std::string::npos || upperReq.find("MUST") != std::string::npos;
            return hasShall && requirementText.length() > 20;
        }
        
        default:
            return true;
    }
}

GeneratedSpec SpecGenerator::MaintainTraceability(const GeneratedSpec& spec) {
    GeneratedSpec updatedSpec = spec;
    
    // Extract requirements for traceability
    auto requirements = ExtractRequirements(spec.requirementsDocument);
    
    // Create comprehensive traceability links in design document
    std::string updatedDesign = spec.designDocument;
    
    // Add traceability section at the beginning of design document
    std::string traceabilitySection = "\n## Requirements Traceability\n\n";
    traceabilitySection += "This design document addresses the following requirements:\n\n";
    
    for (size_t i = 0; i < requirements.size(); ++i) {
        std::string reqId = GenerateRequirementId("feature", static_cast<int>(i + 1));
        traceabilitySection += "- **" + reqId + "**: " + requirements[i].substr(0, 150);
        if (requirements[i].length() > 150) traceabilitySection += "...";
        traceabilitySection += "\n";
    }
    traceabilitySection += "\n";
    
    // Insert traceability section after overview
    size_t overviewPos = updatedDesign.find("## Overview");
    if (overviewPos != std::string::npos) {
        size_t nextSectionPos = updatedDesign.find("\n## ", overviewPos + 11);
        if (nextSectionPos != std::string::npos) {
            updatedDesign.insert(nextSectionPos, traceabilitySection);
        }
    }
    
    // Add traceability markers to each design section
    for (size_t i = 0; i < requirements.size(); ++i) {
        std::string reqId = GenerateRequirementId("feature", static_cast<int>(i + 1));
        std::string traceabilityMarker = "\n**Traces to:** " + reqId + "\n";
        
        // Find design sections and add traceability
        std::string designSectionPattern = "Design";
        size_t pos = 0;
        while ((pos = updatedDesign.find(designSectionPattern, pos)) != std::string::npos) {
            // Check if this section is related to the requirement
            size_t sectionEnd = updatedDesign.find("\n## ", pos + 1);
            if (sectionEnd == std::string::npos) sectionEnd = updatedDesign.length();
            
            std::string sectionContent = updatedDesign.substr(pos, sectionEnd - pos);
            
            // If section doesn't already have traceability and is relevant
            if (sectionContent.find("**Traces to:**") == std::string::npos) {
                // Add traceability marker before next section
                size_t insertPos = sectionEnd;
                if (insertPos < updatedDesign.length()) {
                    updatedDesign.insert(insertPos, traceabilityMarker);
                }
            }
            
            pos = sectionEnd + 1;
        }
    }
    
    updatedSpec.designDocument = updatedDesign;
    
    // Create comprehensive traceability links in tasks document
    std::string updatedTasks = spec.tasksDocument;
    
    // Add traceability matrix section at the beginning of tasks document
    std::string tasksTraceabilitySection = "\n## Traceability Matrix\n\n";
    tasksTraceabilitySection += "This implementation plan traces back to requirements and design:\n\n";
    tasksTraceabilitySection += "| Task | Design Section | Requirements |\n";
    tasksTraceabilitySection += "|------|----------------|-------------|\n";
    
    // Parse tasks and create traceability entries
    std::istringstream taskStream(updatedTasks);
    std::string line;
    int taskNum = 1;
    
    while (std::getline(taskStream, line)) {
        if (line.find("- [ ]") != std::string::npos && line.find(".") != std::string::npos) {
            // Extract task description
            size_t descStart = line.find(". ");
            if (descStart != std::string::npos) {
                std::string taskDesc = line.substr(descStart + 2);
                if (taskDesc.length() > 50) taskDesc = taskDesc.substr(0, 47) + "...";
                
                // Find related design section and requirements
                std::string designRef = "Design Section " + std::to_string(taskNum);
                std::string reqRef = GenerateRequirementId("feature", taskNum);
                
                tasksTraceabilitySection += "| Task " + std::to_string(taskNum) + " | " + designRef + " | " + reqRef + " |\n";
                taskNum++;
            }
        }
    }
    
    tasksTraceabilitySection += "\n";
    
    // Insert traceability matrix after overview
    size_t tasksOverviewPos = updatedTasks.find("## Overview");
    if (tasksOverviewPos != std::string::npos) {
        size_t nextSectionPos = updatedTasks.find("\n## ", tasksOverviewPos + 11);
        if (nextSectionPos != std::string::npos) {
            updatedTasks.insert(nextSectionPos, tasksTraceabilitySection);
        }
    }
    
    updatedSpec.tasksDocument = updatedTasks;
    
    // Add bidirectional traceability links
    // Requirements -> Design -> Tasks
    std::string updatedRequirements = spec.requirementsDocument;
    
    // Add forward traceability section to requirements
    std::string forwardTraceability = "\n## Forward Traceability\n\n";
    forwardTraceability += "Each requirement is addressed in the design and implementation:\n\n";
    
    for (size_t i = 0; i < requirements.size(); ++i) {
        std::string reqId = GenerateRequirementId("feature", static_cast<int>(i + 1));
        forwardTraceability += "- **" + reqId + "**\n";
        forwardTraceability += "  - Design: See Design Section " + std::to_string(i + 1) + "\n";
        forwardTraceability += "  - Implementation: See Task " + std::to_string(i + 1) + "\n";
    }
    forwardTraceability += "\n";
    
    // Append forward traceability to requirements document
    updatedRequirements += forwardTraceability;
    updatedSpec.requirementsDocument = updatedRequirements;
    
    return updatedSpec;
}

std::string SpecGenerator::GenerateGraphicsSpecification(const FeatureRequest& request) {
    std::stringstream ss;
    ss << "\n### Graphics System Requirements\n\n";
    ss << "**User Story:** As a graphics programmer, I want " << request.featureName << " integration with the rendering pipeline, ";
    ss << "so that I can create high-quality visual effects following OpenGL 4.6+ standards.\n\n";
    ss << "#### Acceptance Criteria\n\n";
    ss << "1. THE " << request.featureName << " SHALL integrate with existing PrimitiveRenderer and Material systems\n";
    ss << "2. THE " << request.featureName << " SHALL support OpenGL 4.6+ rendering features\n";
    ss << "3. THE " << request.featureName << " SHALL maintain 60+ FPS performance during normal operation\n";
    ss << "4. THE " << request.featureName << " SHALL provide shader hot-reloading capabilities\n";
    ss << "5. THE " << request.featureName << " SHALL follow Game Engine Kiro graphics conventions\n\n";
    return ss.str();
}

std::string SpecGenerator::GeneratePhysicsSpecification(const FeatureRequest& request) {
    std::stringstream ss;
    ss << "\n### Physics System Requirements\n\n";
    ss << "**User Story:** As a physics programmer, I want " << request.featureName << " integration with physics simulation, ";
    ss << "so that I can create realistic physical interactions using Bullet Physics and planned PhysX backends.\n\n";
    ss << "#### Acceptance Criteria\n\n";
    ss << "1. THE " << request.featureName << " SHALL integrate with existing Bullet Physics systems\n";
    ss << "2. THE " << request.featureName << " SHALL support planned PhysX backend integration\n";
    ss << "3. THE " << request.featureName << " SHALL maintain deterministic physics simulation\n";
    ss << "4. THE " << request.featureName << " SHALL provide collision detection and response\n";
    ss << "5. THE " << request.featureName << " SHALL follow physics engine performance standards\n\n";
    return ss.str();
}

std::string SpecGenerator::GenerateAudioSpecification(const FeatureRequest& request) {
    std::stringstream ss;
    ss << "\n### Audio System Requirements\n\n";
    ss << "**User Story:** As an audio programmer, I want " << request.featureName << " integration with 3D spatial audio, ";
    ss << "so that I can create immersive audio experiences using OpenAL.\n\n";
    ss << "#### Acceptance Criteria\n\n";
    ss << "1. THE " << request.featureName << " SHALL integrate with existing OpenAL 3D spatial audio system\n";
    ss << "2. THE " << request.featureName << " SHALL support positional audio with distance attenuation\n";
    ss << "3. THE " << request.featureName << " SHALL provide audio streaming capabilities\n";
    ss << "4. THE " << request.featureName << " SHALL maintain low-latency audio processing\n";
    ss << "5. THE " << request.featureName << " SHALL follow audio engine performance standards\n\n";
    return ss.str();
}

std::string SpecGenerator::GenerateResourceSpecification(const FeatureRequest& request) {
    std::stringstream ss;
    ss << "\n### Resource Management Requirements\n\n";
    ss << "**User Story:** As a resource manager, I want " << request.featureName << " integration with asset loading, ";
    ss << "so that I can efficiently manage game assets with proper caching and lifetime management.\n\n";
    ss << "#### Acceptance Criteria\n\n";
    ss << "1. THE " << request.featureName << " SHALL integrate with existing Resource Manager for asset loading\n";
    ss << "2. THE " << request.featureName << " SHALL provide efficient asset caching mechanisms\n";
    ss << "3. THE " << request.featureName << " SHALL support hot-reloading of assets during development\n";
    ss << "4. THE " << request.featureName << " SHALL manage asset lifetime and memory usage\n";
    ss << "5. THE " << request.featureName << " SHALL follow resource management best practices\n\n";
    return ss.str();
}

std::string SpecGenerator::GenerateAnimationSpecification(const FeatureRequest& request) {
    std::stringstream ss;
    ss << "\n### Animation System Requirements\n\n";
    ss << "**User Story:** As an animation programmer, I want " << request.featureName << " integration with skeletal animation, ";
    ss << "so that I can create complex character animations with blend trees and state machines.\n\n";
    ss << "#### Acceptance Criteria\n\n";
    ss << "1. THE " << request.featureName << " SHALL integrate with existing skeletal animation system\n";
    ss << "2. THE " << request.featureName << " SHALL support animation blend trees and state machines\n";
    ss << "3. THE " << request.featureName << " SHALL provide bone matrix management and skinning\n";
    ss << "4. THE " << request.featureName << " SHALL maintain smooth animation transitions\n";
    ss << "5. THE " << request.featureName << " SHALL follow animation system performance standards\n\n";
    return ss.str();
}

void SpecGenerator::InitializeEARSPatterns() {
    m_earsPatterns[EARSPattern::Ubiquitous] = {"SHALL", "MUST", "WILL"};
    m_earsPatterns[EARSPattern::EventDriven] = {"WHEN", "IF", "ON", "UPON"};
    m_earsPatterns[EARSPattern::StateDriven] = {"WHILE", "DURING", "AS LONG AS"};
    m_earsPatterns[EARSPattern::UnwantedEvent] = {"IF", "WHEN", "SHOULD", "UNWANTED"};
    m_earsPatterns[EARSPattern::OptionalFeature] = {"WHERE", "OPTIONAL", "IF INCLUDED"};
    m_earsPatterns[EARSPattern::Complex] = {"AND", "OR", "BUT", "EXCEPT"};
}

void SpecGenerator::InitializeINCOSERules() {
    m_incoseRules[INCOSEQualityRule::Clarity] = {"clear", "unambiguous", "specific", "precise"};
    m_incoseRules[INCOSEQualityRule::Testability] = {"measurable", "verifiable", "testable", "quantifiable"};
    m_incoseRules[INCOSEQualityRule::Completeness] = {"complete", "comprehensive", "thorough", "detailed"};
    m_incoseRules[INCOSEQualityRule::Consistency] = {"consistent", "coherent", "compatible", "aligned"};
    m_incoseRules[INCOSEQualityRule::Correctness] = {"correct", "accurate", "valid", "feasible"};
    m_incoseRules[INCOSEQualityRule::Traceability] = {"traceable", "linked", "connected", "mapped"};
    m_incoseRules[INCOSEQualityRule::Modifiability] = {"modifiable", "maintainable", "flexible", "adaptable"};
}

void SpecGenerator::LoadSystemTemplates() {
    m_systemTemplates["Graphics"] = {"graphics_renderer", "graphics_shader", "graphics_material"};
    m_systemTemplates["Physics"] = {"physics_bullet", "physics_physx", "physics_collision"};
    m_systemTemplates["Audio"] = {"audio_openal", "audio_3d", "audio_streaming"};
    m_systemTemplates["Resource"] = {"resource_loader", "resource_cache", "resource_manager"};
    m_systemTemplates["Animation"] = {"animation_skeletal", "animation_blend", "animation_state"};
}

std::string SpecGenerator::ProcessTemplatePlaceholders(const std::string& templateContent, const std::map<std::string, std::string>& placeholders) {
    std::string result = templateContent;
    
    for (const auto& placeholder : placeholders) {
        std::string token = "{{" + placeholder.first + "}}";
        size_t pos = 0;
        while ((pos = result.find(token, pos)) != std::string::npos) {
            result.replace(pos, token.length(), placeholder.second);
            pos += placeholder.second.length();
        }
    }
    
    return result;
}

std::vector<std::string> SpecGenerator::ExtractRequirements(const std::string& requirementsDocument) {
    std::vector<std::string> requirements;
    std::istringstream iss(requirementsDocument);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Look for lines that contain "SHALL" - these are likely requirements
        if (line.find("SHALL") != std::string::npos || line.find("MUST") != std::string::npos) {
            // Clean up the line
            std::string cleanLine = line;
            // Remove leading numbers and bullets
            std::regex numberPattern(R"(^\s*\d+\.\s*)");
            cleanLine = std::regex_replace(cleanLine, numberPattern, "");
            
            if (!cleanLine.empty()) {
                requirements.push_back(cleanLine);
            }
        }
    }
    
    return requirements;
}

std::string SpecGenerator::GenerateRequirementId(const std::string& featureName, int requirementIndex) {
    return featureName + "_REQ_" + std::to_string(requirementIndex);
}

bool SpecGenerator::ValidateTemplate(const std::string& templateContent) {
    // Basic validation - check for balanced placeholders
    size_t openCount = 0;
    size_t closeCount = 0;
    
    size_t pos = 0;
    while ((pos = templateContent.find("{{", pos)) != std::string::npos) {
        openCount++;
        pos += 2;
    }
    
    pos = 0;
    while ((pos = templateContent.find("}}", pos)) != std::string::npos) {
        closeCount++;
        pos += 2;
    }
    
    return openCount == closeCount;
}

std::map<std::string, std::string> SpecGenerator::GenerateDefaultPlaceholders(const FeatureRequest& request) {
    std::map<std::string, std::string> placeholders;
    
    placeholders["FEATURE_NAME"] = request.featureName;
    placeholders["FEATURE_DESCRIPTION"] = request.description;
    placeholders["COMPLEXITY"] = request.complexity;
    
    // Join target systems
    std::string targetSystems;
    for (size_t i = 0; i < request.targetSystems.size(); ++i) {
        if (i > 0) targetSystems += ", ";
        targetSystems += request.targetSystems[i];
    }
    placeholders["TARGET_SYSTEMS"] = targetSystems;
    
    // Generate module namespace from feature name
    std::string moduleNamespace = request.featureName;
    std::replace(moduleNamespace.begin(), moduleNamespace.end(), ' ', '_');
    std::transform(moduleNamespace.begin(), moduleNamespace.end(), moduleNamespace.begin(), ::toupper);
    placeholders["MODULE_NAMESPACE"] = moduleNamespace;
    
    // Generate main class name
    std::string className = request.featureName;
    std::replace(className.begin(), className.end(), ' ', '_');
    // Convert to PascalCase
    bool capitalizeNext = true;
    for (char& c : className) {
        if (c == '_') {
            capitalizeNext = true;
        } else if (capitalizeNext) {
            c = static_cast<char>(std::toupper(c));
            capitalizeNext = false;
        } else {
            c = static_cast<char>(std::tolower(c));
        }
    }
    className.erase(std::remove(className.begin(), className.end(), '_'), className.end());
    placeholders["MAIN_CLASS_NAME"] = className;
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    placeholders["TIMESTAMP"] = ss.str();
    
    // Generate glossary terms
    std::string glossaryTerms = "- **" + request.featureName + "**: " + request.description + "\n";
    for (const auto& system : request.targetSystems) {
        glossaryTerms += "- **" + system + "_Integration**: Integration with Game Engine Kiro " + system + " subsystem\n";
    }
    placeholders["GLOSSARY_TERMS"] = glossaryTerms;
    
    // Generate data models placeholder
    placeholders["DATA_MODELS"] = "### " + request.featureName + " Data Structures\n\nData models and structures specific to " + request.featureName + " implementation.";
    
    return placeholders;
}

bool SpecGenerator::CheckTemplateConflicts(const std::string& templateName) {
    return m_requirementsTemplates.find(templateName) == m_requirementsTemplates.end() &&
           m_designTemplates.find(templateName) == m_designTemplates.end() &&
           m_tasksTemplates.find(templateName) == m_tasksTemplates.end();
}

std::vector<std::string> SpecGenerator::GetAvailableTemplates(const std::string& systemName) const {
    auto it = m_systemTemplates.find(systemName);
    if (it != m_systemTemplates.end()) {
        return it->second;
    }
    return {};
}

bool SpecGenerator::UpdateTemplate(const std::string& name, const std::string& content) {
    if (!ValidateTemplate(content)) {
        return false;
    }
    
    if (m_requirementsTemplates.find(name) != m_requirementsTemplates.end()) {
        m_requirementsTemplates[name] = content;
        return true;
    }
    if (m_designTemplates.find(name) != m_designTemplates.end()) {
        m_designTemplates[name] = content;
        return true;
    }
    if (m_tasksTemplates.find(name) != m_tasksTemplates.end()) {
        m_tasksTemplates[name] = content;
        return true;
    }
    
    return false;
}

std::vector<std::string> SpecGenerator::GenerateImprovementSuggestions(const std::string& requirements) {
    std::vector<std::string> suggestions;
    
    ValidationResult earsResult = ValidateEARSCompliance(requirements);
    ValidationResult incoseResult = ValidateINCOSECompliance(requirements);
    
    suggestions.insert(suggestions.end(), earsResult.suggestions.begin(), earsResult.suggestions.end());
    suggestions.insert(suggestions.end(), incoseResult.suggestions.begin(), incoseResult.suggestions.end());
    
    return suggestions;
}

GeneratedSpec SpecGenerator::UpdateSpecification(const GeneratedSpec& originalSpec, const std::string& updatedRequirements) {
    GeneratedSpec updatedSpec = originalSpec;
    updatedSpec.requirementsDocument = updatedRequirements;
    
    // Re-validate the updated requirements
    ValidationResult earsResult = ValidateEARSCompliance(updatedRequirements);
    ValidationResult incoseResult = ValidateINCOSECompliance(updatedRequirements);
    
    updatedSpec.validationErrors.clear();
    updatedSpec.warnings.clear();
    
    if (!earsResult.isCompliant) {
        updatedSpec.validationErrors.insert(updatedSpec.validationErrors.end(), 
                                          earsResult.violations.begin(), earsResult.violations.end());
    }
    
    if (!incoseResult.isCompliant) {
        updatedSpec.validationErrors.insert(updatedSpec.validationErrors.end(), 
                                          incoseResult.violations.begin(), incoseResult.violations.end());
    }
    
    updatedSpec.warnings.insert(updatedSpec.warnings.end(), earsResult.warnings.begin(), earsResult.warnings.end());
    updatedSpec.warnings.insert(updatedSpec.warnings.end(), incoseResult.warnings.begin(), incoseResult.warnings.end());
    
    updatedSpec.isValid = updatedSpec.validationErrors.empty();
    
    return MaintainTraceability(updatedSpec);
}

std::map<std::string, std::vector<std::string>> SpecGenerator::GenerateTraceabilityMatrix(const GeneratedSpec& spec) {
    std::map<std::string, std::vector<std::string>> matrix;
    
    auto requirements = ExtractRequirements(spec.requirementsDocument);
    
    for (size_t i = 0; i < requirements.size(); ++i) {
        std::string reqId = GenerateRequirementId("feature", static_cast<int>(i + 1));
        
        // Find corresponding design sections
        std::vector<std::string> designSections;
        std::string designSection = "Design Section " + std::to_string(i + 1);
        if (spec.designDocument.find(designSection) != std::string::npos) {
            designSections.push_back(designSection);
        }
        
        // Find corresponding tasks
        std::vector<std::string> tasks;
        std::string taskRef = "Task " + std::to_string(i + 1);
        if (spec.tasksDocument.find(taskRef) != std::string::npos) {
            tasks.push_back(taskRef);
        }
        
        matrix[reqId] = designSections;
        matrix[reqId].insert(matrix[reqId].end(), tasks.begin(), tasks.end());
    }
    
    return matrix;
}

} // namespace GameEngine::Power::SpecGeneration