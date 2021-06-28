#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 outColor;

layout (location = 0) in OutData
{
	vec3 position;
	vec2 uv;
	vec4 color;
	int textureID;
} data;

layout(set = 1, binding = 0) uniform sampler2D textures[16];

void main()
{
	vec4 texColor = data.color;
    if (data.textureID >= 0)
    {
        texColor *= texture(textures[data.textureID], data.uv);
    }
	outColor = texColor;
}
