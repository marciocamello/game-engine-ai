#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIds;
layout (location = 4) in vec4 aBoneWeights;

uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_boneMatrices[64];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    // Simple test - just pass through position without skinning
    FragPos = vec3(u_model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_model))) * aNormal;
    TexCoords = aTexCoords;
    
    gl_Position = u_mvp * vec4(aPos, 1.0);
}