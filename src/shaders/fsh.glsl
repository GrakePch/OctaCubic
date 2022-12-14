#version 460 core
in vec3 fFragPos;
in vec3 fNormal;
in vec4 fColor;
out vec4 FragColor;
in vec3 fNormal_model;
in vec3 fPos_model;

uniform mat4 model;
uniform mat4 view;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambient;
uniform float specularStrength = 0;
float PhongExp = 1024;

uniform float waveStrength = 0;
uniform float time = 0;

mat4 rotationMatrix(vec3 axis, float angle) {
    // Reference: https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main() {
    vec3 normal_n;
    if (waveStrength > 0.0 && fNormal_model.y != 0.0 ) { // Generate wavy normal: exclude vertical surface
        float normal_rotate_angle =   waveStrength * .10 * sin((.5 * fPos_model.x + .5 * fPos_model.z) * 5 + time * 5);
        float normal_rotate_angle_2 = waveStrength * .08 * sin((.6 * fPos_model.x + .4 * fPos_model.z) * 5 + time * 2);
        float normal_rotate_angle_3 = waveStrength * .05 * sin((.1 * fPos_model.x + .9 * fPos_model.z) * 4 + time * 7);
        vec3 normal_addwave = mat3(transpose(inverse(view
            * rotationMatrix(vec3(.1, 0.0, .9), normal_rotate_angle_3)
            * rotationMatrix(vec3(.6, 0.0, .4), normal_rotate_angle_2)
            * rotationMatrix(vec3(.5, 0.0, .5), normal_rotate_angle)
            * model))) * fNormal_model;
        normal_n = normalize(normal_addwave);
    } else { // Flat normal
        normal_n = normalize(fNormal);
    }
    
    // Diffuse lighting
    vec3 lightDir = normalize((view * vec4(lightPos, 1.0)).xyz - fFragPos);
    float diff = max(dot(normal_n, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting: Blinn-Phong model
    vec3 viewDir = -normalize(fFragPos);
    vec3 halfVector = normalize(normalize(lightDir) + viewDir);
    float spec = pow(max(dot(normal_n, halfVector), 0.0), PhongExp);
    vec3 specular = specularStrength * spec * lightColor;
    FragColor = vec4((ambient + diffuse) * fColor.rgb + specular, 1.0);
}