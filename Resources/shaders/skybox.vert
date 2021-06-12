#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;



layout(location = 0) out vec3 texCoords;

void main() {
	texCoords = inPosition;
	vec4 pos = ubo.proj * ubo.view * vec4(inPosition, 1.0);
	gl_Position = pos.xyww;
}