#pragma once

#include "Core/Math.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace GameEngine {
    class Shader;
    class Material;

    /**
     * Detailed uniform information for shader introspection
     * Requirements: 10.1, 10.3, 10.4
     */
    struct UniformInfo {
        std::string name;
        uint32_t location;
        uint32_t type;          // GL type (GL_FLOAT, GL_VEC3, etc.)
        int size;               // Array size (1 for non-arrays)
        bool isActive;          // Whether the uniform is actively used
        std::string typeName;   // Human-readable type name
        std::string description; // Optional description
        
        UniformInfo() = default;
        UniformInfo(const std::string& n, uint32_t loc, uint32_t t, int s, bool active)
            : name(n), location(loc), type(t), size(s), isActive(active) {}
    };

    /**
     * Detailed attribute information for shader introspection
     * Requirements: 10.1, 10.3, 10.4
     */
    struct AttributeInfo {
        std::string name;
        uint32_t location;
        uint32_t type;          // GL type
        int size;               // Array size
        bool isActive;
        std::string typeName;   // Human-readable type name
        std::string description;
        
        AttributeInfo() = default;
        AttributeInfo(const std::string& n, uint32_t loc, uint32_t t, int s, bool active)
            : name(n), location(loc), type(t), size(s), isActive(active) {}
    };

    /**
     * Storage buffer information for compute shaders
     * Requirements: 10.1, 10.3, 10.4
     */
    struct StorageBufferInfo {
        std::string name;
        uint32_t binding;
        uint32_t bufferIndex;
        int bufferDataSize;
        bool isActive;
        std::string description;
        
        StorageBufferInfo() = default;
        StorageBufferInfo(const std::string& n, uint32_t bind, uint32_t idx, int size, bool active)
            : name(n), binding(bind), bufferIndex(idx), bufferDataSize(size), isActive(active) {}
    };

    /**
     * Comprehensive shader introspection information
     * Requirements: 10.1, 10.3, 10.4
     */
    struct ShaderIntrospectionData {
        std::string shaderName;
        uint32_t programId;
        
        // Resource information
        std::vector<UniformInfo> uniforms;
        std::vector<AttributeInfo> attributes;
        std::vector<StorageBufferInfo> storageBuffers;
        
        // Shader statistics
        int activeUniforms = 0;
        int activeAttributes = 0;
        int activeStorageBuffers = 0;
        int textureUnits = 0;
        int maxTextureUnits = 0;
        
        // Performance metrics
        int estimatedInstructions = 0;
        int estimatedComplexity = 0;
        size_t estimatedMemoryUsage = 0;
        
        // Validation results
        bool isValid = true;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
        std::vector<std::string> optimizationSuggestions;
        
        void Clear() {
            uniforms.clear();
            attributes.clear();
            storageBuffers.clear();
            warnings.clear();
            errors.clear();
            optimizationSuggestions.clear();
            activeUniforms = 0;
            activeAttributes = 0;
            activeStorageBuffers = 0;
            textureUnits = 0;
            maxTextureUnits = 0;
            estimatedInstructions = 0;
            estimatedComplexity = 0;
            estimatedMemoryUsage = 0;
            isValid = true;
        }
    };

    /**
     * Shader introspection and development tools
     * Requirements: 10.1, 10.3, 10.4
     */
    class ShaderIntrospection {
    public:
        // Shader introspection methods
        static ShaderIntrospectionData IntrospectShader(std::shared_ptr<Shader> shader);
        static ShaderIntrospectionData IntrospectShaderProgram(uint32_t programId, const std::string& name = "");
        
        // Uniform introspection
        static std::vector<UniformInfo> GetShaderUniforms(uint32_t programId);
        static UniformInfo GetUniformInfo(uint32_t programId, const std::string& uniformName);
        static bool HasUniform(uint32_t programId, const std::string& uniformName);
        static std::string GetUniformTypeName(uint32_t glType);
        
        // Attribute introspection
        static std::vector<AttributeInfo> GetShaderAttributes(uint32_t programId);
        static AttributeInfo GetAttributeInfo(uint32_t programId, const std::string& attributeName);
        static bool HasAttribute(uint32_t programId, const std::string& attributeName);
        static std::string GetAttributeTypeName(uint32_t glType);
        
        // Storage buffer introspection (for compute shaders)
        static std::vector<StorageBufferInfo> GetStorageBuffers(uint32_t programId);
        static StorageBufferInfo GetStorageBufferInfo(uint32_t programId, const std::string& bufferName);
        
        // Shader analysis
        static int EstimateShaderComplexity(uint32_t programId);
        static size_t EstimateShaderMemoryUsage(uint32_t programId);
        static std::vector<std::string> AnalyzeShaderPerformance(uint32_t programId);
        static std::vector<std::string> GetOptimizationSuggestions(const ShaderIntrospectionData& data);
        
        // Shader validation
        static bool ValidateShaderResources(uint32_t programId, std::vector<std::string>& issues);
        static bool CheckUniformUsage(uint32_t programId, std::vector<std::string>& unusedUniforms);
        static bool CheckAttributeUsage(uint32_t programId, std::vector<std::string>& unusedAttributes);
        
        // Debug output generation
        static std::string GenerateShaderReport(const ShaderIntrospectionData& data);
        static std::string GenerateUniformReport(const std::vector<UniformInfo>& uniforms);
        static std::string GenerateAttributeReport(const std::vector<AttributeInfo>& attributes);
        static void DumpShaderInfo(uint32_t programId, const std::string& shaderName = "");
        
        // Shader comparison
        static std::vector<std::string> CompareShaders(const ShaderIntrospectionData& shader1, 
                                                      const ShaderIntrospectionData& shader2);
        static bool AreShaderInterfacesCompatible(const ShaderIntrospectionData& shader1,
                                                  const ShaderIntrospectionData& shader2);
        
    private:
        // Helper methods
        static std::string FormatGLType(uint32_t glType);
        static int GetTypeSize(uint32_t glType);
        static bool IsTextureType(uint32_t glType);
        static std::string GetResourceDescription(const std::string& name, uint32_t type);
    };

    /**
     * Material property inspection and runtime modification tools
     * Requirements: 10.1, 10.3, 10.4
     */
    class MaterialInspector {
    public:
        struct PropertyInfo {
            std::string name;
            std::string typeName;
            std::string currentValue;
            std::string defaultValue;
            bool isModified = false;
            bool isTexture = false;
            std::string description;
            
            // Value constraints (for numeric types)
            float minValue = 0.0f;
            float maxValue = 1.0f;
            bool hasConstraints = false;
        };
        
        struct MaterialInspectionData {
            std::string materialName;
            std::string materialType;
            std::string shaderName;
            
            std::vector<PropertyInfo> properties;
            std::vector<std::string> textures;
            std::vector<std::string> warnings;
            std::vector<std::string> suggestions;
            
            size_t memoryUsage = 0;
            bool isValid = true;
        };
        
        // Material inspection
        static MaterialInspectionData InspectMaterial(std::shared_ptr<Material> material);
        static std::vector<PropertyInfo> GetMaterialProperties(std::shared_ptr<Material> material);
        static PropertyInfo GetPropertyInfo(std::shared_ptr<Material> material, const std::string& propertyName);
        
        // Runtime property modification
        static bool SetPropertyValue(std::shared_ptr<Material> material, const std::string& propertyName, 
                                    const std::string& value);
        static std::string GetPropertyValue(std::shared_ptr<Material> material, const std::string& propertyName);
        static bool ResetProperty(std::shared_ptr<Material> material, const std::string& propertyName);
        static void ResetAllProperties(std::shared_ptr<Material> material);
        
        // Property validation
        static bool ValidatePropertyValue(const PropertyInfo& info, const std::string& value);
        static std::vector<std::string> GetValidationErrors(std::shared_ptr<Material> material);
        static std::vector<std::string> GetOptimizationSuggestions(std::shared_ptr<Material> material);
        
        // Material analysis
        static size_t CalculateMemoryUsage(std::shared_ptr<Material> material);
        static std::vector<std::string> AnalyzeMaterialPerformance(std::shared_ptr<Material> material);
        static bool IsMaterialOptimal(std::shared_ptr<Material> material);
        
        // Debug output
        static std::string GenerateMaterialReport(const MaterialInspectionData& data);
        static void DumpMaterialInfo(std::shared_ptr<Material> material);
        static std::string FormatPropertyValue(const PropertyInfo& info);
        
        // Material comparison
        static std::vector<std::string> CompareMaterials(std::shared_ptr<Material> material1,
                                                        std::shared_ptr<Material> material2);
        static bool AreMaterialsCompatible(std::shared_ptr<Material> material1,
                                          std::shared_ptr<Material> material2);
        
        // Property change tracking
        static void StartPropertyTracking(std::shared_ptr<Material> material);
        static void StopPropertyTracking(std::shared_ptr<Material> material);
        static std::vector<std::string> GetPropertyChanges(std::shared_ptr<Material> material);
        static void ClearPropertyChanges(std::shared_ptr<Material> material);
        
    private:
        // Helper methods
        static std::string MaterialTypeToString(int type);
        static std::string PropertyTypeToString(int type);
        static bool ParsePropertyValue(const std::string& value, const std::string& typeName, 
                                      float& floatVal, Math::Vec3& vec3Val, Math::Vec4& vec4Val);
        static std::string FormatPropertyConstraints(const PropertyInfo& info);
        
        // Property change tracking
        static std::unordered_map<std::shared_ptr<Material>, std::vector<std::string>> s_trackedChanges;
        static std::unordered_map<std::shared_ptr<Material>, bool> s_trackingEnabled;
    };

    /**
     * Shader performance profiling and bottleneck identification
     * Requirements: 10.1, 10.3, 10.4
     */
    class ShaderPerformanceProfiler {
    public:
        struct PerformanceBottleneck {
            std::string type;           // "uniform_updates", "texture_bindings", "state_changes", etc.
            std::string description;
            float impactScore;          // 0.0 to 1.0, higher = more impact
            std::vector<std::string> suggestions;
        };
        
        struct ShaderPerformanceProfile {
            std::string shaderName;
            uint32_t programId;
            
            // Timing metrics
            double averageFrameTime = 0.0;
            double maxFrameTime = 0.0;
            double minFrameTime = 999999.0;
            uint64_t totalFrames = 0;
            
            // Resource usage
            int uniformUpdatesPerFrame = 0;
            int textureBindingsPerFrame = 0;
            int stateChangesPerFrame = 0;
            size_t gpuMemoryUsage = 0;
            
            // Performance analysis
            std::vector<PerformanceBottleneck> bottlenecks;
            std::vector<std::string> optimizationOpportunities;
            float performanceScore = 100.0f; // 0-100, higher is better
            
            void Reset() {
                averageFrameTime = 0.0;
                maxFrameTime = 0.0;
                minFrameTime = 999999.0;
                totalFrames = 0;
                uniformUpdatesPerFrame = 0;
                textureBindingsPerFrame = 0;
                stateChangesPerFrame = 0;
                gpuMemoryUsage = 0;
                bottlenecks.clear();
                optimizationOpportunities.clear();
                performanceScore = 100.0f;
            }
        };
        
        // Performance profiling
        static void StartProfiling(const std::string& shaderName, uint32_t programId);
        static void StopProfiling(const std::string& shaderName);
        static ShaderPerformanceProfile GetPerformanceProfile(const std::string& shaderName);
        static std::vector<std::string> GetProfiledShaders();
        
        // Performance measurement
        static void RecordFrameTime(const std::string& shaderName, double frameTime);
        static void RecordUniformUpdate(const std::string& shaderName);
        static void RecordTextureBinding(const std::string& shaderName);
        static void RecordStateChange(const std::string& shaderName);
        static void RecordMemoryUsage(const std::string& shaderName, size_t bytes);
        
        // Bottleneck identification
        static std::vector<PerformanceBottleneck> IdentifyBottlenecks(const std::string& shaderName);
        static std::vector<std::string> GetOptimizationOpportunities(const std::string& shaderName);
        static float CalculatePerformanceScore(const ShaderPerformanceProfile& profile);
        
        // Performance analysis
        static std::vector<std::string> GetTopPerformers(int count = 5);
        static std::vector<std::string> GetWorstPerformers(int count = 5);
        static std::string GeneratePerformanceReport(const std::string& shaderName);
        static std::string GenerateGlobalPerformanceReport();
        
        // Performance thresholds and alerts
        static void SetPerformanceThresholds(double maxFrameTime, int maxUniformUpdates, 
                                           int maxTextureBindings, size_t maxMemoryUsage);
        static bool IsPerformanceAcceptable(const std::string& shaderName);
        static std::vector<std::string> GetPerformanceAlerts();
        
        // Profiling control
        static void ResetAllProfiles();
        static void ResetProfile(const std::string& shaderName);
        static bool IsProfilingEnabled();
        static void SetProfilingEnabled(bool enabled);
        
    private:
        static std::unordered_map<std::string, ShaderPerformanceProfile> s_profiles;
        static bool s_profilingEnabled;
        
        // Performance thresholds
        static double s_maxFrameTime;
        static int s_maxUniformUpdates;
        static int s_maxTextureBindings;
        static size_t s_maxMemoryUsage;
        
        // Helper methods
        static PerformanceBottleneck CreateBottleneck(const std::string& type, const std::string& description,
                                                     float impact, const std::vector<std::string>& suggestions);
        static std::string FormatTime(double timeMs);
        static std::string FormatMemorySize(size_t bytes);
    };

}