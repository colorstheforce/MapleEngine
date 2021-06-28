#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 uv;
layout (location = 2) in vec4 inColor;

layout(set = 0,binding = 0) uniform UniformBufferObject
{
	mat4 projView;
} ubo;

layout (location = 0) out OutData
{
	vec3 position;
	vec2 uv;
	vec4 color;
	int textureID;
} data;

void main()
{
	gl_Position =  ubo.projView * vec4(position,1.0);
	data.position = position;
	data.uv = uv.xy;
	data.textureID = int(uv.z - 0.5);
	data.color = inColor;
}