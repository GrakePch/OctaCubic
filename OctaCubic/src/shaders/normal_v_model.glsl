#version 460 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
out vec3 fNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fNormal = abs(aNormal);
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}