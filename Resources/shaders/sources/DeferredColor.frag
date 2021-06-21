#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PI 3.1415926535897932384626433832795
#define GAMMA 2.2
const float PBR_WORKFLOW_SEPARATE_TEXTURES = 0.0f;
const float PBR_WORKFLOW_METALLIC_ROUGHNESS = 1.0f;
const float PBR_WORKFLOW_SPECULAR_GLOSINESS = 2.0f;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragPosition;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragTangent;


layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform sampler2D metallicMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessMap;
layout(set = 1, binding = 3) uniform sampler2D normalMap;
layout(set = 1, binding = 4) uniform sampler2D aoMap;
layout(set = 1, binding = 5) uniform sampler2D emissiveMap;



layout(set = 1,binding = 6) uniform UniformMaterialData
{
	vec4  albedoColor;
	vec4  roughnessColor;
	vec4  metallicColor;
	vec4  emissiveColor;
	float usingAlbedoMap;
	float usingMetallicMap;
	float usingRoughnessMap;
	float usingNormalMap;
	float usingAOMap;
	float usingEmissiveMap;
	float workflow;
	float padding;
} materialProperties;


//bind to framebuffer
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormal;
layout(location = 3) out vec4 outPBR;


vec4 gammaCorrect(vec4 samp)
{
	//return samp;
	return vec4(pow(samp.rgb, vec3(GAMMA)), samp.a);
}


vec4 getAlbedo()
{
	return (1.0 - materialProperties.usingAlbedoMap) * materialProperties.albedoColor 
	+ materialProperties.usingAlbedoMap * gammaCorrect(texture(albedoMap, fragTexCoord));
}


vec3 getMetallic()
{
	return (1.0 - materialProperties.usingMetallicMap) * materialProperties.metallicColor.rgb 
	+ materialProperties.usingMetallicMap * gammaCorrect(texture(metallicMap, fragTexCoord)).rgb;
}

float getRoughness()
{
	return (1.0 - materialProperties.usingRoughnessMap) *  materialProperties.roughnessColor.r 
	+ materialProperties.usingRoughnessMap * gammaCorrect(texture(roughnessMap, fragTexCoord)).r;
}

float getAO()
{
	return (1.0 - materialProperties.usingAOMap) 
	+ materialProperties.usingAOMap * gammaCorrect(texture(aoMap, fragTexCoord)).r;
}

vec3 getEmissive()
{
	return (1.0 - materialProperties.usingEmissiveMap) * materialProperties.emissiveColor.rgb 
	+ materialProperties.usingEmissiveMap * gammaCorrect(texture(emissiveMap, fragTexCoord)).rgb;
}

vec3 getNormalMap()
{
	if (materialProperties.usingNormalMap < 0.1)
		return normalize(fragNormal);

	vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
	vec3 Q1 = dFdx(fragPosition.xyz);
	vec3 Q2 = dFdy(fragPosition.xyz);
	vec2 st1 = dFdx(fragTexCoord);
	vec2 st2 = dFdy(fragTexCoord);
	vec3 N = normalize(fragNormal);
	vec3 T = normalize(Q1*st2.t - Q2*st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * tangentNormal);
}


void main()
{
	vec4 texColor = getAlbedo();
	if(texColor.w < 0.4)
		discard;

	float metallic = 0.0;
	float roughness = 0.0;

	if(materialProperties.workflow == PBR_WORKFLOW_SEPARATE_TEXTURES)
	{
		metallic  = getMetallic().x;
		roughness = getRoughness();
	}
	else if( materialProperties.workflow == PBR_WORKFLOW_METALLIC_ROUGHNESS)
	{
		vec4 tex = gammaCorrect(texture(metallicMap, fragTexCoord));
		metallic = tex.b;
		roughness = tex.g;
	}
	else if( materialProperties.workflow == PBR_WORKFLOW_SPECULAR_GLOSINESS)
	{
		vec4 tex = gammaCorrect(texture(metallicMap, fragTexCoord));
		metallic = tex.b;
		roughness = tex.g;
	}

	vec3 emissive   = getEmissive();
	float ao		= getAO();

    outColor    = texColor;
	outPosition = fragPosition;
	outNormal   = vec4(getNormalMap(),1.0);
	outPBR      = vec4(metallic,roughness, ao, 1.0);

	outPosition.w = emissive.x;
	outNormal.w   = emissive.y;
	outPBR.w      = emissive.z;
}