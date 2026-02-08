#version 460 core

// Output
out vec4 FragColor;

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec4 WorldPos;

// Material properties
uniform vec4 uColor;
uniform vec3 uEmissive;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAmbientOcclusion;

// Texture maps
uniform bool uHasAlbedoTexture;
uniform bool uHasNormalTexture;
uniform bool uHasMetallicTexture;
uniform bool uHasRoughnessTexture;
uniform bool uHasAOTexture;
uniform bool uHasEmissiveTexture;

uniform sampler2D uAlbedoTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uMetallicTexture;
uniform sampler2D uRoughnessTexture;
uniform sampler2D uAOTexture;
uniform sampler2D uEmissiveTexture;

// Lighting
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;
uniform float uLightIntensity;

// Environment
uniform vec3 uAmbientColor;
uniform float uAmbientStrength;

// PBR constants
const float PI = 3.14159265359;

// Normal mapping function
vec3 getNormalFromMap()
{
    if (!uHasNormalTexture)
        return normalize(Normal);
        
    vec3 tangentNormal = texture(uNormalTexture, TexCoord).xyz * 2.0 - 1.0;
    
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Distribution function (GGX/Trowbridge-Reitz)
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry function
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

void main()
{
    // Sample material properties from textures
    vec4 albedo = uColor;
    if (uHasAlbedoTexture)
        albedo *= texture(uAlbedoTexture, TexCoord);
    
    float metallic = uMetallic;
    if (uHasMetallicTexture)
        metallic *= texture(uMetallicTexture, TexCoord).r;
    
    float roughness = uRoughness;
    if (uHasRoughnessTexture)
        roughness *= texture(uRoughnessTexture, TexCoord).r;
    
    float ao = uAmbientOcclusion;
    if (uHasAOTexture)
        ao *= texture(uAOTexture, TexCoord).r;
    
    vec3 emissive = uEmissive;
    if (uHasEmissiveTexture)
        emissive *= texture(uEmissiveTexture, TexCoord).rgb;
    
    // Get normal from normal map or vertex normal
    vec3 N = getNormalFromMap();
    vec3 V = normalize(uViewPos - FragPos);
    
    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);
    
    // Reflectance equation
    vec3 Lo = vec3(0.0);
    
    // Calculate per-light radiance
    vec3 L = normalize(uLightPos - FragPos);
    vec3 H = normalize(V + L);
    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = uLightColor * uLightIntensity * attenuation;
    
    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    
    // Ambient lighting
    vec3 ambient = uAmbientColor * uAmbientStrength * albedo.rgb * ao;
    
    // Add emissive
    vec3 color = ambient + Lo + emissive;
    
    // HDR tonemapping (simple Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, albedo.a);
}