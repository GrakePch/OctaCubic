#version 330 core
out vec4 FragColor;

flat in vec3 fBlockCoord;

uniform float worldMaxDim;
uniform vec3 playerCoord;

void main() {
    vec3 distXYZ = fBlockCoord - floor(playerCoord);
    // max hand reaching distance
    float distMax = 16;
    // assign white for unreachable blocks, leaving XYZ in range [-16, +16)
    if (distXYZ.x >= distMax || distXYZ.x < -distMax
     || distXYZ.y >= distMax || distXYZ.y < -distMax
     || distXYZ.z >= distMax || distXYZ.z < -distMax) {
        FragColor = vec4(1.0);
        return;
    }
    // clamp XYZ to [0, 32)
    distXYZ = distXYZ + distMax;
    // transform XYZ to [0 / 256, 32 / 256)
    distXYZ = distXYZ / 256;
    FragColor = vec4(distXYZ, 1.0);
}