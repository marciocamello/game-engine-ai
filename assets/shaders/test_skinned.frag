#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec4 u_color;

void main() {
    // Simple test - just output the color
    FragColor = u_color;
}