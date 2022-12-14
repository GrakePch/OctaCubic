#version 460 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
out vec3 fFragPos;
out vec3 fNormal;
out vec4 fColor;
out vec3 fNormal_model;
out vec3 fPos_model;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 diffuseColor;

void main() {
    fFragPos = (view * model * vec4(aPosition, 1.0)).xyz;
    fNormal = mat3(transpose(inverse(view * model))) * aNormal;
    fNormal_model = aNormal;
    fPos_model = (model * vec4(aPosition, 1.0)).xyz;
    fColor = diffuseColor;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}