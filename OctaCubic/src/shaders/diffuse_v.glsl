#version 460 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
out vec4 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 diffuseColor;

void main() {
    fColor = diffuseColor;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}