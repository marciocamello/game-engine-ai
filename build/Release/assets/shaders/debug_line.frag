#version 460 core

in vec3 vertexColor;
out vec4 FragColor;

uniform float u_alpha;

void main() {
    FragColor = vec4(vertexColor, u_alpha);
}