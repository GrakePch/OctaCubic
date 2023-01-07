#version 460 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
out vec3 fFragPos;
out vec3 fNormal;
out vec4 fColor;
out vec3 fNormal_model;
out vec3 fPos_model;
out vec2 fTexCoord;
out vec4 fFragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 diffuseColor;
uniform int blockId = 2;
uniform int blockRes = 16;
uniform int textureRes = 256;
uniform mat4 lightSpaceMatrix;

vec2 calcUV(int id, int blockRes, int textureRes, vec2 TexCoord) {
    vec2 result = TexCoord;
    int blocksInARow = textureRes / blockRes;
    float scale = float(blockRes) / textureRes;
    result += vec2(id % blocksInARow, id / blocksInARow);
    result *= scale;
    return result;
}

void main() {
    fFragPos = (view * model * vec4(aPosition, 1.0)).xyz;
    fNormal = mat3(transpose(inverse(view * model))) * aNormal;
    fNormal_model = aNormal;
    fPos_model = (model * vec4(aPosition, 1.0)).xyz;
    fColor = diffuseColor;
    fTexCoord = calcUV(blockId, blockRes, textureRes, aTexCoord);
    fFragPosLightSpace = lightSpaceMatrix * model * vec4(aPosition, 1.0); // for shadow mapping
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}