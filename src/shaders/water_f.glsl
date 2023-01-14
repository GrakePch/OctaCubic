#version 460 core
out vec4 FragColor;

in vec3 fFragPos;
in vec3 fNormal;
in vec4 fColor;
in vec3 fNormal_model;
in vec3 fPos_model;
in vec4 fFragPosLightSpace;
flat in int fBlockId;

uniform mat4 model;
uniform mat4 view;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambient;
uniform float specularStrength = 0;
float PhongExp = 1024;
uniform sampler2D shadowMap;

uniform float waveStrength = 0;
uniform float time = 0;
vec3 lightDir;

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

float calcShadow(vec4 fragPosLightSpace) {
    // Reference: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    float bias = max(0.001 * (1.0 - dot(fNormal, lightDir)), 0.0001);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0; 
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow; 
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
    lightDir = normalize((view * vec4(lightPos, 1.0)).xyz - fFragPos);
    float diff = max(dot(normal_n, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting: Blinn-Phong model
    vec3 viewDir = -normalize(fFragPos);
    vec3 halfVector = normalize(normalize(lightDir) + viewDir);
    float spec = pow(max(dot(normal_n, halfVector), 0.0), PhongExp);
    vec3 specular = specularStrength * spec * lightColor;

    // Shadow
    float shadow = calcShadow(fFragPosLightSpace);
    
    vec3 FragColorRGB = (ambient + (1.0 - shadow) * diffuse) * fColor.rgb + (1.0 - shadow) * specular;
    FragColor = vec4(FragColorRGB, fColor.a);
}