#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

uniform mat4 u_viewProjection;

out vec3 vertexColor;

void main() {
    vertexColor = aColor;
    gl_Position = u_viewProjection * vec4(aPosition, 1.0);
}