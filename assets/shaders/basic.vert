#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat3 u_normalMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;

void main() {
    FragPos = vec3(u_model * vec4(aPos, 1.0));
    Normal = u_normalMatrix * aNormal;
    TexCoord = aTexCoord;
    Tangent = u_normalMatrix * aTangent;
    Bitangent = u_normalMatrix * aBitangent;
    
    gl_Position = u_projection * u_view * vec4(FragPos, 1.0);
}