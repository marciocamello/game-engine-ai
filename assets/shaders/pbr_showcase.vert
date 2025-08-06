#version 460 core

// PBR Showcase Vertex Shader
// Enhanced vertex shader for PBR material demonstration
// Supports normal mapping, tangent space calculations, and multiple light types

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Transformation matrices
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat3 u_normalMatrix;

// Camera position for view-dependent calculations
uniform vec3 u_viewPos;

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;
out vec3 ViewPos;
out mat3 TBN; // Tangent-Bitangent-Normal matrix

void main() {
    // Transform position to world space
    FragPos = vec3(u_model * vec4(aPos, 1.0));
    
    // Transform normal, tangent, and bitangent to world space
    Normal = normalize(u_normalMatrix * aNormal);
    Tangent = normalize(u_normalMatrix * aTangent);
    Bitangent = normalize(u_normalMatrix * aBitangent);
    
    // Pass through texture coordinates
    TexCoord = aTexCoord;
    
    // Pass camera position
    ViewPos = u_viewPos;
    
    // Construct TBN matrix for normal mapping
    // Re-orthogonalize tangent with respect to normal (Gram-Schmidt process)
    vec3 T = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 B = cross(Normal, T);
    TBN = mat3(T, B, Normal);
    
    // Transform to clip space
    gl_Position = u_projection * u_view * vec4(FragPos, 1.0);
}