#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace GameEngine {
    
    struct ShaderVariant {
        std::unordered_map<std::string, std::string> defines;
        std::vector<std::string> features;
        std::string name;

        // Default constructor
        ShaderVariant() = default;
        
        // Constructor with name
        explicit ShaderVariant(const std::string& variantName) : name(variantName) {}

        // Helper methods
        void AddDefine(const std::string& defineName, const std::string& value = "1");
        void AddFeature(const std::string& feature);
        void RemoveDefine(const std::string& defineName);
        void RemoveFeature(const std::string& feature);
        void Clear();
        
        // Hash generation for efficient caching
        std::string GenerateHash() const;
        
        // Compatibility checking
        bool IsCompatibleWith(const ShaderVariant& other) const;
        bool HasDefine(const std::string& defineName) const;
        bool HasFeature(const std::string& feature) const;
        std::string GetDefineValue(const std::string& defineName) const;
        
        // Utility methods
        bool IsEmpty() const;
        size_t GetDefineCount() const;
        size_t GetFeatureCount() const;
        
        // Generate preprocessor string for shader compilation
        std::string GeneratePreprocessorString() const;
        
        // Comparison operators
        bool operator==(const ShaderVariant& other) const;
        bool operator!=(const ShaderVariant& other) const;
        
        // String representation for debugging
        std::string ToString() const;
    };

    // Hash function for using ShaderVariant as key in unordered containers
    struct ShaderVariantHash {
        size_t operator()(const ShaderVariant& variant) const;
    };

    // Predefined common shader variants
    namespace ShaderVariants {
        // Basic variants
        ShaderVariant CreateDefault();
        ShaderVariant CreateDebug();
        ShaderVariant CreateOptimized();
        
        // Lighting variants
        ShaderVariant CreateWithDirectionalLight();
        ShaderVariant CreateWithPointLights(int maxLights = 8);
        ShaderVariant CreateWithSpotLights(int maxLights = 4);
        ShaderVariant CreateWithShadows();
        
        // Material variants
        ShaderVariant CreateWithAlbedoMap();
        ShaderVariant CreateWithNormalMap();
        ShaderVariant CreateWithMetallicRoughnessMap();
        ShaderVariant CreateWithEmissionMap();
        ShaderVariant CreateWithAOMap();
        
        // Advanced features
        ShaderVariant CreateWithSkinning(int maxBones = 64);
        ShaderVariant CreateWithInstancing();
        ShaderVariant CreateWithTessellation();
        ShaderVariant CreateWithGeometryShader();
    }
}