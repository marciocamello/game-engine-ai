#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace GameEngine::Power::SpecGeneration {

/**
 * EARS Pattern Types for requirements validation
 */
enum class EARSPattern {
    Ubiquitous,      // System shall always...
    EventDriven,     // When event occurs, system shall...
    StateDriven,     // While in state, system shall...
    UnwantedEvent,   // If unwanted event occurs, system shall...
    OptionalFeature, // Where feature is included, system shall...
    Complex          // Complex requirements combining multiple patterns
};

/**
 * INCOSE Quality Rules for requirements validation
 */
enum class INCOSEQualityRule {
    Clarity,         // Clear and unambiguous language
    Testability,     // Verifiable and measurable
    Completeness,    // All necessary information included
    Consistency,     // No contradictions with other requirements
    Correctness,     // Technically accurate and feasible
    Traceability,    // Linked to higher-level requirements
    Modifiability    // Easy to change without affecting others
};

/**
 * Feature request structure for spec generation
 */
struct FeatureRequest {
    std::string featureName;
    std::string description;
    std::vector<std::string> targetSystems; // Graphics, Physics, Audio, etc.
    std::string complexity; // Simple, Moderate, Complex
    bool includePropertyTests;
    bool includePerformanceProfiling;
    std::map<std::string, std::string> customParameters;
};

/**
 * Generated specification structure
 */
struct GeneratedSpec {
    std::string requirementsDocument;
    std::string designDocument;
    std::string tasksDocument;
    std::vector<std::string> generatedFiles;
    std::vector<std::string> warnings;
    std::vector<std::string> validationErrors;
    bool isValid;
};

/**
 * Template configuration for different engine subsystems
 */
struct TemplateConfiguration {
    std::string templateName;
    std::string targetSystem;
    std::map<std::string, std::string> placeholders;
    std::vector<std::string> requiredSections;
    std::vector<std::string> optionalSections;
};

/**
 * Validation result for EARS and INCOSE compliance
 */
struct ValidationResult {
    bool isCompliant;
    std::vector<std::string> violations;
    std::vector<std::string> suggestions;
    std::vector<std::string> warnings;
    double complianceScore; // 0.0 to 1.0
};

/**
 * SpecGenerator class - Core component for generating comprehensive feature specifications
 * 
 * This class provides automated spec creation tools for Game Engine Kiro development,
 * generating consistent requirements.md, design.md, and tasks.md templates with
 * engine-specific sections and maintaining traceability between documents.
 * 
 * Features:
 * - Template-based spec generation for Graphics, Physics, Audio, and Resource management
 * - EARS pattern validation for requirements quality
 * - INCOSE quality rule checking for professional standards
 * - Automatic traceability maintenance between requirements, design, and tasks
 * - Engine-specific template customization and management
 */
class SpecGenerator {
public:
    /**
     * Constructor - Initializes the spec generator with default templates
     */
    SpecGenerator();

    /**
     * Destructor
     */
    ~SpecGenerator();

    // Core spec generation methods
    
    /**
     * Generate complete feature specification from request
     * @param request Feature request with target systems and complexity
     * @return Generated specification with all documents and validation results
     */
    GeneratedSpec GenerateFeatureSpec(const FeatureRequest& request);

    /**
     * Validate specification compliance with EARS and INCOSE standards
     * @param spec Generated specification to validate
     * @return True if specification meets all compliance requirements
     */
    bool ValidateSpecCompliance(const GeneratedSpec& spec);

    /**
     * Generate requirements document from feature request
     * @param request Feature request with specifications
     * @return Generated requirements document content
     */
    std::string GenerateRequirementsDocument(const FeatureRequest& request);

    /**
     * Generate design document from feature request and requirements
     * @param request Feature request with specifications
     * @param requirements Generated requirements for traceability
     * @return Generated design document content
     */
    std::string GenerateDesignDocument(const FeatureRequest& request, const std::string& requirements);

    /**
     * Generate tasks document from feature request and design
     * @param request Feature request with specifications
     * @param design Generated design document for task breakdown
     * @return Generated tasks document content
     */
    std::string GenerateTasksDocument(const FeatureRequest& request, const std::string& design);

    // Template management methods

    /**
     * Load default engine templates for all subsystems
     */
    void LoadEngineTemplates();

    /**
     * Register custom template for specific use cases
     * @param name Template identifier
     * @param content Template content with placeholders
     * @param config Template configuration settings
     */
    void RegisterCustomTemplate(const std::string& name, const std::string& content, const TemplateConfiguration& config);

    /**
     * Get available templates for specific system
     * @param systemName Target system (Graphics, Physics, Audio, etc.)
     * @return Vector of available template names
     */
    std::vector<std::string> GetAvailableTemplates(const std::string& systemName) const;

    /**
     * Update existing template with new content
     * @param name Template identifier
     * @param content New template content
     * @return True if template was successfully updated
     */
    bool UpdateTemplate(const std::string& name, const std::string& content);

    // Quality assurance and validation methods

    /**
     * Validate requirements document for EARS pattern compliance
     * @param requirements Requirements document content
     * @return Validation result with compliance details
     */
    ValidationResult ValidateEARSCompliance(const std::string& requirements);

    /**
     * Validate requirements document for INCOSE quality rules
     * @param requirements Requirements document content
     * @return Validation result with quality assessment
     */
    ValidationResult ValidateINCOSECompliance(const std::string& requirements);

    /**
     * Analyze requirement text for EARS pattern classification
     * @param requirementText Single requirement statement
     * @return Detected EARS pattern type
     */
    EARSPattern ClassifyEARSPattern(const std::string& requirementText);

    /**
     * Check requirement against specific INCOSE quality rule
     * @param requirementText Single requirement statement
     * @param rule INCOSE quality rule to check
     * @return True if requirement meets the quality rule
     */
    bool CheckINCOSEQualityRule(const std::string& requirementText, INCOSEQualityRule rule);

    /**
     * Generate improvement suggestions for requirements
     * @param requirements Requirements document content
     * @return Vector of specific improvement suggestions
     */
    std::vector<std::string> GenerateImprovementSuggestions(const std::string& requirements);

    // Traceability and maintenance methods

    /**
     * Maintain traceability links between requirements, design, and tasks
     * @param spec Generated specification to update with traceability
     * @return Updated specification with traceability links
     */
    GeneratedSpec MaintainTraceability(const GeneratedSpec& spec);

    /**
     * Update specification when requirements change
     * @param originalSpec Original specification
     * @param updatedRequirements New requirements content
     * @return Updated specification with propagated changes
     */
    GeneratedSpec UpdateSpecification(const GeneratedSpec& originalSpec, const std::string& updatedRequirements);

    /**
     * Generate traceability matrix for specification
     * @param spec Specification to analyze
     * @return Traceability matrix showing requirement-design-task relationships
     */
    std::map<std::string, std::vector<std::string>> GenerateTraceabilityMatrix(const GeneratedSpec& spec);

    // Engine-specific integration methods

    /**
     * Generate Graphics subsystem specification sections
     * @param request Feature request targeting Graphics
     * @return Graphics-specific specification content
     */
    std::string GenerateGraphicsSpecification(const FeatureRequest& request);

    /**
     * Generate Physics subsystem specification sections
     * @param request Feature request targeting Physics
     * @return Physics-specific specification content
     */
    std::string GeneratePhysicsSpecification(const FeatureRequest& request);

    /**
     * Generate Audio subsystem specification sections
     * @param request Feature request targeting Audio
     * @return Audio-specific specification content
     */
    std::string GenerateAudioSpecification(const FeatureRequest& request);

    /**
     * Generate Resource management specification sections
     * @param request Feature request targeting Resource management
     * @return Resource-specific specification content
     */
    std::string GenerateResourceSpecification(const FeatureRequest& request);

    /**
     * Generate Animation subsystem specification sections
     * @param request Feature request targeting Animation
     * @return Animation-specific specification content
     */
    std::string GenerateAnimationSpecification(const FeatureRequest& request);

private:
    // Template storage
    std::map<std::string, std::string> m_requirementsTemplates;
    std::map<std::string, std::string> m_designTemplates;
    std::map<std::string, std::string> m_tasksTemplates;
    std::map<std::string, TemplateConfiguration> m_templateConfigurations;

    // Validation patterns and rules
    std::map<EARSPattern, std::vector<std::string>> m_earsPatterns;
    std::map<INCOSEQualityRule, std::vector<std::string>> m_incoseRules;

    // Engine-specific template categories
    std::map<std::string, std::vector<std::string>> m_systemTemplates;

    // Internal helper methods

    /**
     * Initialize default EARS patterns for validation
     */
    void InitializeEARSPatterns();

    /**
     * Initialize default INCOSE quality rules
     */
    void InitializeINCOSERules();

    /**
     * Load system-specific templates for engine subsystems
     */
    void LoadSystemTemplates();

    /**
     * Process template placeholders with actual values
     * @param templateContent Template with placeholders
     * @param placeholders Map of placeholder names to values
     * @return Processed template content
     */
    std::string ProcessTemplatePlaceholders(const std::string& templateContent, const std::map<std::string, std::string>& placeholders);

    /**
     * Extract requirements from generated requirements document
     * @param requirementsDocument Full requirements document
     * @return Vector of individual requirement statements
     */
    std::vector<std::string> ExtractRequirements(const std::string& requirementsDocument);

    /**
     * Generate unique requirement identifiers
     * @param featureName Base feature name
     * @param requirementIndex Index of requirement
     * @return Unique requirement identifier
     */
    std::string GenerateRequirementId(const std::string& featureName, int requirementIndex);

    /**
     * Validate template syntax and structure
     * @param templateContent Template to validate
     * @return True if template is valid
     */
    bool ValidateTemplate(const std::string& templateContent);

    /**
     * Generate default placeholders for feature request
     * @param request Feature request to process
     * @return Map of default placeholders
     */
    std::map<std::string, std::string> GenerateDefaultPlaceholders(const FeatureRequest& request);

    /**
     * Check for template conflicts and duplicates
     * @param templateName Name of template to check
     * @return True if template name is available
     */
    bool CheckTemplateConflicts(const std::string& templateName);
};

} // namespace GameEngine::Power::SpecGeneration