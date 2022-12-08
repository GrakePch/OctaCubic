#version 460 core
in vec3 fFragPos;
in vec3 fNormal;
in vec4 fColor;
out vec4 FragColor;

uniform mat4 model;
uniform mat4 view;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambient;

void main() {
    vec3 normal_n = normalize(fNormal);
    vec3 lightDir = normalize((view * vec4(lightPos, 1.0)).xyz - fFragPos);
    float diff = max(dot(normal_n, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    FragColor = vec4((ambient + diffuse) * fColor.rgb, 1.0); 
    // FragColor = vec4(normal_n, 1.0); // Check fNormal
}