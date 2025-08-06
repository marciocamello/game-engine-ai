#version 460 core

// Bloom Extract Fragment Shader
// Extracts bright pixels from HDR scene for bloom effect

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_sceneTexture;
uniform float u_bloomThreshold;
uniform float u_bloomKnee;

// Luminance calculation
float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

// Soft threshold function
float softThreshold(float value, float threshold, float knee) {
    float halfKnee = knee * 0.5;
    float soft = clamp((value - threshold + halfKnee) / knee, 0.0, 1.0);
    return threshold + soft * soft * knee;
}

void main() {
    vec3 color = texture(u_sceneTexture, TexCoord).rgb;
    float luma = luminance(color);
    
    // Apply soft threshold
    float contribution = max(0.0, luma - u_bloomThreshold);
    contribution = softThreshold(luma, u_bloomThreshold, u_bloomKnee) / max(luma, 0.0001);
    
    FragColor = vec4(color * contribution, 1.0);
}