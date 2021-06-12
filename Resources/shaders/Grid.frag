#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#extension GL_EXT_debug_printf : enable


layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform UniformBuffer
{
	vec4  cameraPos;
	float scale;
	float res;
	float maxDistance;
	float p1;
}ubo;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexCoord;

const float step = 100.0f;
const float subdivisions = 10.0f;

vec4 Grid(float divisions)
{
	vec2 coord = fragTexCoord.xy * divisions;

	vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
	float line = min(grid.x, grid.y);
	float lineResult = ubo.res - min(line, ubo.res);
	
	vec3 color = vec3(lineResult) * vec3(0.3, 0.3, 0.3);

	return vec4( color, 0.1 * lineResult);
}

void main()
{
	float distanceToCamera = max(distance(fragPosition, ubo.cameraPos.xyz) - abs(ubo.cameraPos.y), 0);

	float divs = ubo.scale / pow(2, round((abs(ubo.cameraPos.y) - step / subdivisions) / step));

	float decreaseDistance = ubo.maxDistance * 1.5;

	outColor = Grid(divs) + Grid(divs / subdivisions);
	outColor.a *= clamp((decreaseDistance - distanceToCamera) / decreaseDistance, 0.0f, 1.0f);
}