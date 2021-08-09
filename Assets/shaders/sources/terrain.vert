#version 450 core
//#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_debug_printf : enable


layout(set = 0,binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	uint width;
	uint height;
	int calInVertex;
} ubo;

layout(set = 0,binding = 1) uniform sampler2D heightMap;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;


layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;



void calPosAndNormal(inout vec3 pos, inout vec3 normal,inout vec2 texCoord)
{
	uint y = gl_VertexIndex / ubo.width;
	uint x = gl_VertexIndex  - y * ubo.height;

	vec2 uv = vec2(y,x) / vec2(ubo.width-1.0, ubo.height-1.0);
	texCoord = uv;

	pos.xy = vec2(y,x);
	pos.z = texture(heightMap, uv).x * 255.0;

	//debugPrintfEXT("ubo.width %d ,ubo.height %d, gl_VertexIndex %d , x : %d ,y : %d",ubo.width,ubo.height,gl_VertexIndex,x,y);


////Central Differencing
    float hL = textureOffset(heightMap, uv, ivec2(-1, 0)).x;
    float hR = textureOffset(heightMap, uv, ivec2(1, 0)).x;
    float hD = textureOffset(heightMap, uv, ivec2(0, -1)).x;
    float hU = textureOffset(heightMap, uv, ivec2(0, 1)).x;

	normal.x = hL - hR;
	normal.y = hD - hU;
	normal.z = 2.0;
	normal = normalize(normal);

}


void main() {
	
	vec3 inPos = inPosition;
	vec3 normal = inNormal;
	vec2 texCoord = inTexCoord;

	if(ubo.calInVertex == 1){
		calPosAndNormal(inPos,normal,texCoord);
	}
	
	
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos, 1.0);
    fragColor = inColor;
    fragTexCoord = texCoord;
	fragNormal =  mat3(transpose(inverse(ubo.model))) * normal;
	vec4 pos =  ubo.model * vec4(inPos,1.0);
	fragPos = pos.xyz / pos.w;
}