#include "Graphics/ShaderVariant.h"
#include "Core/Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace GameEngine {
    
    void ShaderVariant::AddDefine(const std::string& defineName, const std::string& value) {
        if (defineName.empty()) {
            LOG_WARNING("Attempted to add empty define name to shader variant");
            return;
        }
        defines[defineName] = value;
    }

    void ShaderVariant::AddFeature(const std::string& feature) {
        if (feature.empty()) {
            LOG_WARNING("Attempted to add empty feature to shader variant");
            return;
        }
        
        // Avoid duplicates
        if (std::find(features.begin(), features.end(), feature) == features.end()) {
            features.push_back(feature);
        }
    }

    void ShaderVariant::RemoveDefine(const std::string& defineName) {
        defines.erase(defineName);
    }

    void ShaderVariant::RemoveFeature(const std::string& feature) {
        features.erase(std::remove(features.begin(), features.end(), feature), features.end());
    }

    void ShaderVariant::Clear() {
        defines.clear();
        features.clear();
        name.clear();
    }

    std::string ShaderVariant::GenerateHash() const {
        std::stringstream ss;
        
        // Include variant name in hash
        if (!name.empty()) {
            ss << "name:" << name << ";";
        }
        
        // Sort defines for consistent hashing
        std::vector<std::pair<std::string, std::string>> sortedDefines(defines.begin(), defines.end());
        std::sort(sortedDefines.begin(), sortedDefines.end());
        
        for (const auto& define : sortedDefines) {
            ss << "def:" << define.first << "=" << define.second << ";";
        }
        
        // Sort features for consistent hashing
        std::vector<std::string> sortedFeatures = features;
        std::sort(sortedFeatures.begin(), sortedFeatures.end());
        
        for (const auto& feature : sortedFeatures) {
            ss << "feat:" << feature << ";";
        }
        
        // Generate simple hash from string
        std::string hashString = ss.str();
        std::hash<std::string> hasher;
        size_t hashValue = hasher(hashString);
        
        // Convert to hex string
        std::stringstream hexStream;
        hexStream << std::hex << hashValue;
        return hexStream.str();
    }

    bool ShaderVariant::IsCompatibleWith(const ShaderVariant& other) const {
        // Two variants are compatible if they don't have conflicting defines
        for (const auto& define : defines) {
            auto it = other.defines.find(define.first);
            if (it != other.defines.end() && it->second != define.second) {
                return false; // Conflicting define values
            }
        }
        
        // Check for conflicting features (for now, assume all features are compatible)
        // This could be extended with a compatibility matrix in the future
        return true;
    }

    bool ShaderVariant::HasDefine(const std::string& defineName) const {
        return defines.find(defineName) != defines.end();
    }

    bool ShaderVariant::HasFeature(const std::string& feature) const {
        return std::find(features.begin(), features.end(), feature) != features.end();
    }

    std::string ShaderVariant::GetDefineValue(const std::string& defineName) const {
        auto it = defines.find(defineName);
        return (it != defines.end()) ? it->second : "";
    }

    bool ShaderVariant::IsEmpty() const {
        return defines.empty() && features.empty();
    }

    size_t ShaderVariant::GetDefineCount() const {
        return defines.size();
    }

    size_t ShaderVariant::GetFeatureCount() const {
        return features.size();
    }

    std::string ShaderVariant::GeneratePreprocessorString() const {
        std::stringstream ss;
        
        // Add defines
        for (const auto& define : defines) {
            ss << "#define " << define.first;
            if (!define.second.empty() && define.second != "1") {
                ss << " " << define.second;
            }
            ss << "\n";
        }
        
        // Add feature defines (features are treated as boolean defines)
        for (const auto& feature : features) {
            ss << "#define " << feature << "\n";
        }
        
        return ss.str();
    }

    bool ShaderVariant::operator==(const ShaderVariant& other) const {
        return name == other.name && 
               defines == other.defines && 
               features == other.features;
    }

    bool ShaderVariant::operator!=(const ShaderVariant& other) const {
        return !(*this == other);
    }

    std::string ShaderVariant::ToString() const {
        std::stringstream ss;
        ss << "ShaderVariant{";
        
        if (!name.empty()) {
            ss << "name='" << name << "', ";
        }
        
        ss << "defines=[";
        bool first = true;
        for (const auto& define : defines) {
            if (!first) ss << ", ";
            ss << define.first << "=" << define.second;
            first = false;
        }
        ss << "], features=[";
        
        first = true;
        for (const auto& feature : features) {
            if (!first) ss << ", ";
            ss << feature;
            first = false;
        }
        ss << "]}";
        
        return ss.str();
    }

    // Hash function implementation
    size_t ShaderVariantHash::operator()(const ShaderVariant& variant) const {
        std::hash<std::string> hasher;
        return hasher(variant.GenerateHash());
    }

    // Predefined shader variants
    namespace ShaderVariants {
        
        ShaderVariant CreateDefault() {
            ShaderVariant variant("default");
            return variant;
        }

        ShaderVariant CreateDebug() {
            ShaderVariant variant("debug");
            variant.AddDefine("DEBUG", "1");
            variant.AddDefine("ENABLE_VALIDATION", "1");
            variant.AddFeature("DEBUG_OUTPUT");
            return variant;
        }

        ShaderVariant CreateOptimized() {
            ShaderVariant variant("optimized");
            variant.AddDefine("OPTIMIZED", "1");
            variant.AddDefine("FAST_MATH", "1");
            variant.AddFeature("PERFORMANCE_MODE");
            return variant;
        }

        ShaderVariant CreateWithDirectionalLight() {
            ShaderVariant variant("directional_light");
            variant.AddDefine("HAS_DIRECTIONAL_LIGHT", "1");
            variant.AddFeature("DIRECTIONAL_LIGHTING");
            return variant;
        }

        ShaderVariant CreateWithPointLights(int maxLights) {
            ShaderVariant variant("point_lights");
            variant.AddDefine("HAS_POINT_LIGHTS", "1");
            variant.AddDefine("MAX_POINT_LIGHTS", std::to_string(maxLights));
            variant.AddFeature("POINT_LIGHTING");
            return variant;
        }

        ShaderVariant CreateWithSpotLights(int maxLights) {
            ShaderVariant variant("spot_lights");
            variant.AddDefine("HAS_SPOT_LIGHTS", "1");
            variant.AddDefine("MAX_SPOT_LIGHTS", std::to_string(maxLights));
            variant.AddFeature("SPOT_LIGHTING");
            return variant;
        }

        ShaderVariant CreateWithShadows() {
            ShaderVariant variant("shadows");
            variant.AddDefine("HAS_SHADOWS", "1");
            variant.AddDefine("SHADOW_MAP_SIZE", "1024");
            variant.AddFeature("SHADOW_MAPPING");
            return variant;
        }

        ShaderVariant CreateWithAlbedoMap() {
            ShaderVariant variant("albedo_map");
            variant.AddDefine("HAS_ALBEDO_MAP", "1");
            variant.AddFeature("ALBEDO_TEXTURE");
            return variant;
        }

        ShaderVariant CreateWithNormalMap() {
            ShaderVariant variant("normal_map");
            variant.AddDefine("HAS_NORMAL_MAP", "1");
            variant.AddFeature("NORMAL_MAPPING");
            return variant;
        }

        ShaderVariant CreateWithMetallicRoughnessMap() {
            ShaderVariant variant("metallic_roughness_map");
            variant.AddDefine("HAS_METALLIC_ROUGHNESS_MAP", "1");
            variant.AddFeature("METALLIC_ROUGHNESS_TEXTURE");
            return variant;
        }

        ShaderVariant CreateWithEmissionMap() {
            ShaderVariant variant("emission_map");
            variant.AddDefine("HAS_EMISSION_MAP", "1");
            variant.AddFeature("EMISSION_TEXTURE");
            return variant;
        }

        ShaderVariant CreateWithAOMap() {
            ShaderVariant variant("ao_map");
            variant.AddDefine("HAS_AO_MAP", "1");
            variant.AddFeature("AMBIENT_OCCLUSION_TEXTURE");
            return variant;
        }

        ShaderVariant CreateWithSkinning(int maxBones) {
            ShaderVariant variant("skinning");
            variant.AddDefine("HAS_SKINNING", "1");
            variant.AddDefine("MAX_BONES", std::to_string(maxBones));
            variant.AddFeature("VERTEX_SKINNING");
            return variant;
        }

        ShaderVariant CreateWithInstancing() {
            ShaderVariant variant("instancing");
            variant.AddDefine("HAS_INSTANCING", "1");
            variant.AddFeature("INSTANCED_RENDERING");
            return variant;
        }

        ShaderVariant CreateWithTessellation() {
            ShaderVariant variant("tessellation");
            variant.AddDefine("HAS_TESSELLATION", "1");
            variant.AddFeature("TESSELLATION_SHADERS");
            return variant;
        }

        ShaderVariant CreateWithGeometryShader() {
            ShaderVariant variant("geometry_shader");
            variant.AddDefine("HAS_GEOMETRY_SHADER", "1");
            variant.AddFeature("GEOMETRY_PROCESSING");
            return variant;
        }
    }
}