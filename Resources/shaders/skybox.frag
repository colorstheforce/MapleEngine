#version 450

layout(location = 0) in vec3 texCoords;
layout(location = 0) out vec4 outFrag;


layout(binding = 1) uniform samplerCube cubeMap;

void main()
{    
    outFrag = texture(cubeMap, texCoords);
}