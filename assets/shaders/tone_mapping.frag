#version 460 core

// Tone Mapping Fragment Shader
// Multiple tone mapping operators for HDR to LDR conversion

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_hdrTexture;
uniform int u_toneMappingType; // 0=None, 1=Reinhard, 2=ACES, 3=Filmic
uniform float u_exposure;
uniform float u_gamma;

// Reinhard tone mapping
vec3 toneMapReinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

// ACES filmic tone mapping
vec3 toneMapACES(vec3 color) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Uncharted 2 filmic tone mapping
vec3 toneMapFilmic(vec3 color) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    
    vec3 curr = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    vec3 whiteScale = ((vec3(W) * (A * vec3(W) + C * B) + D * E) / (vec3(W) * (A * vec3(W) + B) + D * F)) - E / F;
    
    return curr / whiteScale;
}

void main() {
    vec3 hdrColor = texture(u_hdrTexture, TexCoord).rgb;
    
    // Apply exposure
    hdrColor *= u_exposure;
    
    vec3 mapped;
    
    // Apply tone mapping
    switch (u_toneMappingType) {
        case 0: // None (Linear)
            mapped = hdrColor;
            break;
        case 1: // Reinhard
            mapped = toneMapReinhard(hdrColor);
            break;
        case 2: // ACES
            mapped = toneMapACES(hdrColor);
            break;
        case 3: // Filmic (Uncharted 2)
            mapped = toneMapFilmic(hdrColor);
            break;
        default:
            mapped = hdrColor;
            break;
    }
    
    // Apply gamma correction
    mapped = pow(mapped, vec3(1.0 / u_gamma));
    
    FragColor = vec4(mapped, 1.0);
}