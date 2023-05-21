#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aBlockId;
layout (location = 4) in vec3 aBlockCoord;

flat out vec3 fBlockCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fBlockCoord = aBlockCoord;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}