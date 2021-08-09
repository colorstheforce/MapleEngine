#version 450

layout(location = 0) in vec3 texCoords;
layout(location = 0) out vec4 outFrag;


layout(binding = 1) uniform samplerCube cubeMap;

layout(binding = 2) uniform UniformBufferObject {
    float lodLevel;
} ubo;


void main()
{    
   // vec3 color = texture(cubeMap, texCoords).rgb;
    vec3 color = textureLod(cubeMap, texCoords, ubo.lodLevel).rgb;
    //vec3 color = textureLod(cubeMap, texCoords, 1.2).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    outFrag = vec4(color, 1.0);
}