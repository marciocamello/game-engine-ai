#version 460 core

// PBR Showcase Fragment Shader
// Advanced PBR implementation with multiple material types and lighting models
// Demonstrates metallic workflow, normal mapping, and HDR rendering

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 ViewPos;
in mat3 TBN;

out vec4 FragColor;

// Material properties
uniform vec3 u_albedo;
uniform float u_metallic;
uniform float u_roughness;
uniform float u_ao;
uniform vec3 u_emission;
uniform float u_emissionStrength;
uniform float u_normalStrength;

// Material textures
uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_aoMap;
uniform sampler2D u_emissionMap;

// Lighting uniforms
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform float u_lightIntensity;

// Multiple point lights support
#define MAX_POINT_LIGHTS 8
uniform int u_numPointLights;
uniform vec3 u_pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 u_pointLightColors[MAX_POINT_LIGHTS];
uniform float u_pointLightIntensities[MAX_POINT_LIGHTS];
uniform float u_pointLightRanges[MAX_POINT_LIGHTS];

// Directional light
uniform vec3 u_dirLightDirection;
uniform vec3 u_dirLightColor;
uniform float u_dirLightIntensity;

// Environment and post-processing
uniform float u_exposure;
uniform float u_gamma;

// Material flags
uniform bool u_useAlbedoMap;
uniform bool u_useNormalMap;
uniform bool u_useMetallicMap;
uniform bool u_useRoughnessMap;
uniform bool u_useAOMap;
uniform bool u_useEmissionMap;

// Constants
const float PI = 3.14159265359;
const float EPSILON = 0.0001;

// Get normal from normal map
vec3 getNormalFromMap() {
    if (!u_useNormalMap) {
        return normalize(Normal);
    }
    
    vec3 tangentNormal = texture(u_normalMap, TexCoord).xyz * 2.0 - 1.0;
    tangentNormal.xy *= u_normalStrength;
    
    return normalize(TBN * tangentNormal);
}

// Distribution function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / max(denom, EPSILON);
}

// Geometry function (Smith's method)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / max(denom, EPSILON);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel function (Schlick approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Enhanced Fresnel with roughness
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculate lighting contribution from a point light
vec3 calculatePointLight(vec3 lightPos, vec3 lightColor, float lightIntensity, float lightRange,
                        vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness) {
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);
    
    // Attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    attenuation *= clamp(1.0 - (distance / lightRange), 0.0, 1.0);
    
    vec3 radiance = lightColor * lightIntensity * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// Calculate directional light contribution
vec3 calculateDirectionalLight(vec3 lightDir, vec3 lightColor, float lightIntensity,
                              vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness) {
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);
    
    vec3 radiance = lightColor * lightIntensity;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// Reinhard tone mapping
vec3 toneMapReinhard(vec3 color, float exposure) {
    color *= exposure;
    return color / (color + vec3(1.0));
}

// ACES filmic tone mapping
vec3 toneMapACES(vec3 color, float exposure) {
    color *= exposure;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
    // Sample material properties from textures
    vec3 albedo = u_useAlbedoMap ? 
        pow(texture(u_albedoMap, TexCoord).rgb, vec3(2.2)) * u_albedo : 
        pow(u_albedo, vec3(2.2));
    
    float metallic = u_useMetallicMap ? 
        texture(u_metallicMap, TexCoord).r * u_metallic : 
        u_metallic;
    
    float roughness = u_useRoughnessMap ? 
        texture(u_roughnessMap, TexCoord).r * u_roughness : 
        u_roughness;
    
    float ao = u_useAOMap ? 
        texture(u_aoMap, TexCoord).r * u_ao : 
        u_ao;
    
    vec3 emission = u_useEmissionMap ? 
        texture(u_emissionMap, TexCoord).rgb * u_emission * u_emissionStrength : 
        u_emission * u_emissionStrength;
    
    // Calculate vectors
    vec3 N = getNormalFromMap();
    vec3 V = normalize(ViewPos - FragPos);
    
    // Calculate base reflectivity (F0)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // Initialize lighting accumulation
    vec3 Lo = vec3(0.0);
    
    // Calculate directional light contribution
    if (u_dirLightIntensity > 0.0) {
        Lo += calculateDirectionalLight(u_dirLightDirection, u_dirLightColor, u_dirLightIntensity,
                                       N, V, F0, albedo, metallic, roughness);
    }
    
    // Calculate point light contributions
    for (int i = 0; i < min(u_numPointLights, MAX_POINT_LIGHTS); ++i) {
        Lo += calculatePointLight(u_pointLightPositions[i], u_pointLightColors[i], 
                                 u_pointLightIntensities[i], u_pointLightRanges[i],
                                 N, V, F0, albedo, metallic, roughness);
    }
    
    // Add ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo + emission;
    
    // Apply tone mapping and gamma correction
    color = toneMapACES(color, u_exposure);
    color = pow(color, vec3(1.0 / u_gamma));
    
    FragColor = vec4(color, 1.0);
}