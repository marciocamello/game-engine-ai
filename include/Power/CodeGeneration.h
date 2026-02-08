#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

namespace GameEngine::Power::CodeGeneration {

/**
 * Component request structure for code generation
 */
struct ComponentRequest {
    std::string componentName;
    std::string targetModule;        // Graphics, Physics, Audio, Animation, Resource
    std::string componentType;       // Renderer, Manager, System, Controller, etc.
    std::vector<std::string> dependencies;
    bool includeOpenGLSupport;
    bool includeBulletPhysicsSupport;
    bool includePhysXSupport;
    bool includeOpenALSupport;
    std::map<std::string, std::string> customParameters;
};

/**
 * Generated component structure
 */
struct GeneratedComponent {
    std::string headerFile;
    std::string sourceFile;
    std::string testFile;
    std::string cmakeIntegration;
    std::vector<std::string> dependencies;
    std::vector<std::string> warnings;
    bool isValid;
};

/**
 * Template configuration for code generation
 */
struct TemplateConfiguration {
    std::string templateName;
    std::string targetSystem;
    std::map<std::string, std::string> placeholders;
    std::vector<std::string> requiredIncludes;
    std::vector<std::string> optionalFeatures;
};

/**
 * Shader generation request
 */
struct ShaderRequest {
    std::string shaderName;
    std::string shaderType;          // vertex, fragment, compute
    std::vector<std::string> uniforms;
    std::vector<std::string> attributes;
    bool includeSkeletalAnimation;
    bool includePBRLighting;
    bool includePostProcessing;
};

/**
 * Generated shader pair structure
 */
struct GeneratedShaderPair {
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<std::string> uniformList;
    std::vector<std::string> attributeList;
    bool isValid;
};

/**
 * CodeGenerator class - Generates properly structured C++ code following Game Engine Kiro conventions
 * 
 * This class provides automated code generation tools for Game Engine Kiro development,
 * creating header files, source files, test files, and shader code that integrate
 * seamlessly with existing engine systems.
 * 
 * Features:
 * - Template-based code generation for Graphics, Physics, Audio, Animation, and Resource components
 * - Automatic naming convention enforcement (PascalCase, camelCase, UPPER_SNAKE_CASE)
 * - Namespace uniqueness validation and symbol conflict prevention
 * - Engine subsystem integration templates (Bullet Physics, PhysX, OpenAL, OpenGL)
 * - GLSL shader generation with proper uniforms and attributes
 * - CMake integration code generation
 */
class CodeGenerator {
public:
    /**
     * Constructor - Initializes the code generator with default templates
     */
    CodeGenerator();

    /**
     * Destructor
     */
    ~CodeGenerator();

    // Core code generation methods

    /**
     * Generate complete engine component with header, source, and test files
     * @param request Component specification with target module and features
     * @return Generated component with all files and integration code
     */
    GeneratedComponent GenerateEngineComponent(const ComponentRequest& request);

    /**
     * Generate GLSL shader pair (vertex and fragment)
     * @param request Shader specification with uniforms and attributes
     * @return Generated shader pair with validation results
     */
    GeneratedShaderPair GenerateShaderPair(const ShaderRequest& request);

    /**
     * Generate test suite for component
     * @param componentName Name of component to test
     * @param properties List of properties to validate
     * @return Generated test file content
     */
    std::string GenerateTestSuite(const std::string& componentName, const std::vector<std::string>& properties);

    // Template management methods

    /**
     * Load engine-specific code templates for all subsystems
     */
    void LoadEngineCodeTemplates();

    /**
     * Get Graphics subsystem template
     * @param componentType Type of graphics component (Renderer, Shader, Material, etc.)
     * @return Template content for graphics component
     */
    std::string GetGraphicsTemplate(const std::string& componentType);

    /**
     * Get Physics subsystem template
     * @param backendType Physics backend (Bullet, PhysX)
     * @return Template content for physics component
     */
    std::string GetPhysicsTemplate(const std::string& backendType);

    /**
     * Get Audio subsystem template
     * @param componentType Type of audio component (Source, Buffer, Manager, etc.)
     * @return Template content for audio component
     */
    std::string GetAudioTemplate(const std::string& componentType);

    /**
     * Get Animation subsystem template
     * @param componentType Type of animation component (Controller, StateMachine, etc.)
     * @return Template content for animation component
     */
    std::string GetAnimationTemplate(const std::string& componentType);

    /**
     * Get Resource management template
     * @param componentType Type of resource component (Loader, Cache, Manager, etc.)
     * @return Template content for resource component
     */
    std::string GetResourceTemplate(const std::string& componentType);

    // Validation and quality assurance methods

    /**
     * Validate namespace uniqueness across entire project
     * @param namespaceName Namespace to validate
     * @return True if namespace is unique
     */
    bool ValidateNamespaceUniqueness(const std::string& namespaceName);

    /**
     * Validate class name uniqueness across entire project
     * @param className Class name to validate
     * @return True if class name is unique
     */
    bool ValidateClassNameUniqueness(const std::string& className);

    /**
     * Validate naming conventions compliance
     * @param component Generated component to validate
     * @return True if all naming conventions are followed
     */
    bool ValidateNamingConventions(const GeneratedComponent& component);

    /**
     * Check for symbol conflicts in generated code
     * @param component Generated component to check
     * @return List of detected conflicts
     */
    std::vector<std::string> CheckSymbolConflicts(const GeneratedComponent& component);

    /**
     * Generate alternative name if conflict detected
     * @param baseName Original name that conflicts
     * @param symbolType Type of symbol (class, namespace, function, etc.)
     * @return Alternative unique name
     */
    std::string GenerateAlternativeName(const std::string& baseName, const std::string& symbolType);

    // Engine integration methods

    /**
     * Generate Bullet Physics integration code
     * @param componentName Name of physics component
     * @return Integration code for Bullet Physics
     */
    std::string GenerateBulletPhysicsIntegration(const std::string& componentName);

    /**
     * Generate PhysX integration code
     * @param componentName Name of physics component
     * @return Integration code for PhysX
     */
    std::string GeneratePhysXIntegration(const std::string& componentName);

    /**
     * Generate OpenAL 3D audio integration code
     * @param componentName Name of audio component
     * @return Integration code for OpenAL
     */
    std::string GenerateOpenALIntegration(const std::string& componentName);

    /**
     * Generate OpenGL 4.6+ rendering integration code
     * @param componentName Name of graphics component
     * @return Integration code for OpenGL
     */
    std::string GenerateOpenGLIntegration(const std::string& componentName);

    /**
     * Generate skeletal animation integration code
     * @param componentName Name of animation component
     * @return Integration code for skeletal animation system
     */
    std::string GenerateSkeletalAnimationIntegration(const std::string& componentName);

private:
    // Template storage
    std::map<std::string, std::string> m_classTemplates;
    std::map<std::string, std::string> m_shaderTemplates;
    std::map<std::string, std::string> m_testTemplates;
    std::map<std::string, TemplateConfiguration> m_templateConfigurations;

    // Symbol tracking for uniqueness validation
    std::set<std::string> m_existingNamespaces;
    std::set<std::string> m_existingClasses;
    std::set<std::string> m_existingMacros;

    // Internal helper methods

    /**
     * Apply naming conventions to identifier
     * @param name Original name
     * @param type Type of identifier (class, method, variable, constant, namespace, macro)
     * @return Name with proper conventions applied
     */
    std::string ApplyNamingConventions(const std::string& name, const std::string& type);

    /**
     * Generate namespace hierarchy for component
     * @param module Target module (Graphics, Physics, etc.)
     * @param component Component name
     * @return Full namespace hierarchy
     */
    std::string GenerateNamespace(const std::string& module, const std::string& component);

    /**
     * Generate include directives for dependencies
     * @param dependencies List of dependency names
     * @return Include directives string
     */
    std::string GenerateIncludes(const std::vector<std::string>& dependencies);

    /**
     * Generate header file content
     * @param request Component request
     * @return Header file content
     */
    std::string GenerateHeaderFile(const ComponentRequest& request);

    /**
     * Generate source file content
     * @param request Component request
     * @return Source file content
     */
    std::string GenerateSourceFile(const ComponentRequest& request);

    /**
     * Generate test file content
     * @param request Component request
     * @return Test file content
     */
    std::string GenerateTestFile(const ComponentRequest& request);

    /**
     * Generate CMake integration code
     * @param request Component request
     * @return CMake code for integration
     */
    std::string GenerateCMakeIntegration(const ComponentRequest& request);

    /**
     * Load existing symbols from project
     */
    void LoadExistingSymbols();

    /**
     * Process template placeholders
     * @param templateContent Template with placeholders
     * @param placeholders Map of placeholder values
     * @return Processed template
     */
    std::string ProcessTemplatePlaceholders(const std::string& templateContent, 
                                           const std::map<std::string, std::string>& placeholders);

    /**
     * Generate default placeholders for component
     * @param request Component request
     * @return Map of default placeholders
     */
    std::map<std::string, std::string> GenerateDefaultPlaceholders(const ComponentRequest& request);

    /**
     * Validate template syntax
     * @param templateContent Template to validate
     * @return True if template is valid
     */
    bool ValidateTemplate(const std::string& templateContent);
};

} // namespace GameEngine::Power::CodeGeneration
