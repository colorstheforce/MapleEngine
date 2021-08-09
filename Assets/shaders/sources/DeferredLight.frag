#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
//#extension GL_EXT_debug_printf : enable





layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D uColorSampler;
layout(set = 1, binding = 1) uniform sampler2D uPositionSampler;
layout(set = 1, binding = 2) uniform sampler2D uNormalSampler;
layout(set = 1, binding = 3) uniform sampler2D uDepthSampler;
layout(set = 1, binding = 4) uniform sampler2DArray uShadowMap;
layout(set = 1, binding = 5) uniform samplerCube uShadowCubeMap;

layout(set = 1, binding = 6) uniform sampler2D uPBRSampler;
layout(set = 1, binding = 7) uniform samplerCube uIrradianceMap;
layout(set = 1, binding = 8) uniform samplerCube uEnvironmentMap;
layout(set = 1, binding = 9) uniform sampler2D uPreintegratedFG;


#define PI 3.1415926535897932384626433832795
#define GAMMA 2.2
#define MAX_LIGHTS 32
#define MAX_SHADOWMAPS 4


const int numPCFSamples = 16;
const int numBlockerSearchSamples = 16;
const float Epsilon = 0.00001;
// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 FresnelDielectric = vec3(0.04);


struct Light
{
	vec4 color;
	vec4 position;
	vec4 direction;
	
	float radius;
	float intensity;
	float type;
	float angle;
	
};

struct Material
{
	vec4 albedo;
	vec3 metallic;
	float roughness;
	vec3 emissive;
	vec3 normal;
	float ao;
	vec3 view;
	float normalDotV;//cosTheta
};


#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.1
#define SHADOW_MAX 4

layout(std140, binding = 0) uniform UniformBufferLight
{
	Light lights[MAX_LIGHTS]; //32 * 
	mat4 viewPos;
	mat4 lightProjView[4];
	mat4 lightView;
	vec4 cameraPos;
	vec4 splitDepth;

	int lightCount; //4
	int type;
	int colorCascade;
	int displayCascadeLevel;

	float bias;
	float lightSize;
	float bias2;
	float bias3;

	int prefilterLODLevel;
	int padding1;
	int padding2;
	int padding3;
} ubo;


const vec2 poissonDistribution16[16] = vec2[](
		vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725), vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
		vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464), vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
		vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420), vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
		vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590), vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790)
);

const vec2 poissonDistribution[64] = vec2[](
	vec2(-0.884081, 0.124488), vec2(-0.714377, 0.027940), vec2(-0.747945, 0.227922), vec2(-0.939609, 0.243634),
	vec2(-0.985465, 0.045534),vec2(-0.861367, -0.136222),vec2(-0.881934, 0.396908),vec2(-0.466938, 0.014526),
	vec2(-0.558207, 0.212662),vec2(-0.578447, -0.095822),vec2(-0.740266, -0.095631),vec2(-0.751681, 0.472604),
	vec2(-0.553147, -0.243177),vec2(-0.674762, -0.330730),vec2(-0.402765, -0.122087),vec2(-0.319776, -0.312166),
	vec2(-0.413923, -0.439757),vec2(-0.979153, -0.201245),vec2(-0.865579, -0.288695),vec2(-0.243704, -0.186378),
	vec2(-0.294920, -0.055748),vec2(-0.604452, -0.544251),vec2(-0.418056, -0.587679),vec2(-0.549156, -0.415877),
	vec2(-0.238080, -0.611761),vec2(-0.267004, -0.459702),vec2(-0.100006, -0.229116),vec2(-0.101928, -0.380382),
	vec2(-0.681467, -0.700773),vec2(-0.763488, -0.543386),vec2(-0.549030, -0.750749),vec2(-0.809045, -0.408738),
	vec2(-0.388134, -0.773448),vec2(-0.429392, -0.894892),vec2(-0.131597, 0.065058),vec2(-0.275002, 0.102922),
	vec2(-0.106117, -0.068327),vec2(-0.294586, -0.891515),vec2(-0.629418, 0.379387),vec2(-0.407257, 0.339748),
	vec2(0.071650, -0.384284),vec2(0.022018, -0.263793),vec2(0.003879, -0.136073),vec2(-0.137533, -0.767844),
	vec2(-0.050874, -0.906068),vec2(0.114133, -0.070053),vec2(0.163314, -0.217231),vec2(-0.100262, -0.587992),
	vec2(-0.004942, 0.125368),vec2(0.035302, -0.619310),vec2(0.195646, -0.459022),vec2(0.303969, -0.346362),
	vec2(-0.678118, 0.685099),vec2(-0.628418, 0.507978),vec2(-0.508473, 0.458753),vec2(0.032134, -0.782030),
	vec2(0.122595, 0.280353),vec2(-0.043643, 0.312119),vec2(0.132993, 0.085170),vec2(-0.192106, 0.285848),
	vec2(0.183621, -0.713242),vec2(0.265220, -0.596716),vec2(-0.009628, -0.483058),vec2(-0.018516, 0.435703)
);

vec2 samplePoisson(int index)
{
   return poissonDistribution16[index % 16];
}

vec2 samplePoisson64(int index)
{
   return poissonDistribution[index % 64];
}


const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);

// Gold Noise 2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated seeding method. 

float PHI = 1.61803398874989484820459;  // �� = Golden Ratio   

float goldNoise(vec2 xy,float seed){
	return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}


float getShadowBias(vec3 lightDirection, vec3 normal)
{
	float inputBias = ubo.bias;
	float bias = max(inputBias * (1.0 - dot(normal, lightDirection)), inputBias);
	return bias;
}

//Occlusion range calculation
vec2 searchRegionRadiusUV(float zWorld)
{
//assume the near plane is zero.
	const float lightZNear = 0.01;
	vec2 lightRadiusUV = vec2(0.05);
    return lightRadiusUV * (zWorld - lightZNear) / zWorld;
}

float getBlockerDistance(sampler2DArray shadowMap, vec4 shadowCoords, float uvLightSize, vec3 lightDirection, vec3 normal, vec3 wsPos, uint cascadeIndex)
{
	float bias = getShadowBias(lightDirection, normal);

	int blockers = 0;
	float avgBlockerDistance = 0;
	
	//shading poing -> light 
	//before projection -> in light space 
	float zEye = -(vec4(wsPos, 1.0) * ubo.lightView).z;
	//if the distance of light is very far, we can assume the range is small
	vec2 searchWidth = searchRegionRadiusUV(zEye);

	for (int i = 0; i < numBlockerSearchSamples; i++)
	{
		float z = texture(shadowMap, vec3(shadowCoords.xy + samplePoisson(i) * searchWidth , cascadeIndex)).r;
		if (z < (shadowCoords.z - bias))
		{
			blockers++;
			avgBlockerDistance += z;
		}
	}

	if (blockers > 0)
		return avgBlockerDistance / float(blockers);

	return -1;
}


float PCF_DirectionalLight(sampler2DArray shadowMap, vec4 shadowCoords, float uvRadius, vec3 lightDirection, vec3 normal, vec3 wsPos, uint cascadeIndex)
{
	float bias = getShadowBias(lightDirection, normal);
	float sum = 0;

	for (int i = 0; i < numPCFSamples; i++)
	{
		//random a index
		int index = int(float(numPCFSamples) * goldNoise(wsPos.xy, wsPos.z * i)) % numPCFSamples;
		float z = texture(shadowMap, vec3(shadowCoords.xy + samplePoisson64(index)  * uvRadius, cascadeIndex)).r;
		sum += (z < (shadowCoords.z - bias)) ? 1 : 0;
	}
	return sum / numPCFSamples;
}

/**

			----A----|--wlight--
			|		 |	     |
			|		|	   |
			|	   |	 |
			|	  |	   |
			|	 |	 |
			|   |  | 
			|  | |	 
			| ||	 
			|||		 
			---------- blocker
		  |	|
        |  ||
      |	  |	|
     | 	 |	|
   |  	|	|
 | Pen | B  |
-----------------------------

*/


float PCSS_DirectionalLight(sampler2DArray shadowMap, vec4 shadowCoords, float uvLightSize, vec3 lightDirection, vec3 normal, vec3 wsPos, uint cascadeIndex)
{
//Step1��Blocker search
	float blockerDistance = getBlockerDistance(shadowMap, shadowCoords, uvLightSize, lightDirection, normal, wsPos, cascadeIndex);
	if (blockerDistance == -1)
		return 1;		

//W(penumbra) = (d(receiver) - d(blocker)) * W(light) / d(blocker)
//Penumbra area calculation
	float penumbraWidth = ( shadowCoords.z - blockerDistance) * uvLightSize / blockerDistance;

	float NEAR = 0.01;
//range in shadow map
	float uvRadius = penumbraWidth * NEAR / shadowCoords.z;

	return 1.0 - PCF_DirectionalLight(shadowMap, shadowCoords, uvRadius, lightDirection, normal, wsPos, cascadeIndex);
}


int getCascadeLevel(vec4 fragPos){
	int cascadeIndex = 0;
	//get current frag's viewPos
	vec4 viewPos = ubo.viewPos * fragPos;
	for(int i = 0; i < 3; i++) {
		if(viewPos.z <= ubo.splitDepth[i]) {	
			cascadeIndex = i + 1;
		}
	}
	return cascadeIndex;
}


vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta ,float roughness){
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}



//(Normal Distribution Function)��
//(Fresnel Rquation)
//(Geometry Function)��
//					DFG
// BRDF = ----------------------
//			4(wo dot n)(wi dot n)

vec3 ibl(vec3 F0, vec3 Lr, Material material)
{
	vec3 irradiance = texture(uIrradianceMap, material.normal).rgb;

	vec3 F = fresnelSchlickRoughness(F0, material.normalDotV, material.roughness);

	vec3 kd = (1.0 - F) * (1.0 - material.metallic.x);

	vec3 diffuseIBL = material.albedo.xyz * irradiance;

	vec3 specularIrradiance = textureLod(uEnvironmentMap, Lr, material.roughness * ubo.prefilterLODLevel).rgb;

	vec2 specularBRDF = texture(uPreintegratedFG, vec2(material.normalDotV, 1.0 - material.roughness.x)).rg;
	vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float geometrySchlickGGX(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float geometrySmith(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return geometrySchlickGGX(cosLi, k) * geometrySchlickGGX(NdotV, k);
}


float calculateShadow(vec3 wsPos, int cascadeIndex, float bias, vec3 lightDirection, vec3 normal)
{
	vec4 shadowCoord =  biasMat * ubo.lightProjView[cascadeIndex] * vec4(wsPos, 1.0);
	return PCSS_DirectionalLight(uShadowMap, shadowCoord * ( 1.0 / shadowCoord.w), ubo.lightSize, lightDirection, normal, wsPos, cascadeIndex );
}

vec3 lighting(vec3 F0, vec3 fragPos, Material material,int cascadeLevel)
{
	vec3 result = vec3(0.0);

	for(int i = 0; i < ubo.lightCount; i++)
	{
		Light light = ubo.lights[i];

		float value = 0.0;


	    float bias = ubo.bias;
		bias = bias + (bias * tan(acos(clamp(dot(material.normal, light.direction.xyz), 0.0, 1.0))) * 0.5);
		value = calculateShadow(fragPos,cascadeLevel, bias, light.direction.xyz, material.normal);
		

		vec3 Li = light.direction.xyz;//input 
		vec3 Lradiance = light.color.xyz * light.intensity;//radiance
		vec3 Lh = normalize(Li + material.view);//half vector

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(material.normal, Li));
		float cosLh = max(0.0, dot(material.normal, Lh));

//(Fresnel Rquation)
//(Normal Distribution Function)��
//(Geometry Function)

		vec3  F = fresnelSchlick(F0, max(0.0, dot(Lh, material.view)));
		float D = ndfGGX(cosLh, material.roughness);
		float G = geometrySmith(cosLi, material.normalDotV, material.roughness);
		//diffuse
		vec3 kd = (1.0 - F) * (1.0 - material.metallic.x);
		vec3 diffuseBRDF = kd * material.albedo.xyz;
//
//				D * F * G
//BRDF = ---------------------
//			4(wo dot n)(wi dot n)

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * material.normalDotV);

		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi * value * material.ao;
	}
	return result;
}

void main()
{
	vec4 albedo = texture(uColorSampler, fragTexCoord);
	if (albedo.a < 0.1) {
		discard;
	}

	vec4 fragPosXyzw = texture(uPositionSampler, fragTexCoord);
	vec4 normalTex	 = texture(uNormalSampler, fragTexCoord);
	vec4 pbr		 = texture(uPBRSampler,fragTexCoord);


	Material material;
    material.albedo			= albedo;
    material.metallic		= vec3(pbr.x);
    material.roughness		= max(pbr.y, 0.05);
    material.normal			= normalTex.rgb;
	material.ao				= pbr.z;
	material.emissive		= vec3(fragPosXyzw.w, normalTex.w, pbr.w);
	material.view 			= normalize(ubo.cameraPos.xyz - fragPosXyzw.xyz);
	material.normalDotV     = max(dot(material.normal, material.view), 0.0);

	int cascadeIndex = getCascadeLevel(fragPosXyzw);

	vec3 Lr = 2.0 * material.normalDotV * material.normal - material.view;
	vec3 F0 = mix(FresnelDielectric, material.albedo.xyz, material.metallic.x);


	vec3 lightPart = lighting(F0, fragPosXyzw.xyz, material,cascadeIndex); 
	vec3 iblPart = ibl(F0, Lr, material) * 2.0;
	vec3 finalColour = lightPart + iblPart;// + material.emissive;
	outColor = vec4(finalColour, 1.0);


//debug ->>>>>>>>>>>>>>
	switch(ubo.type){
		case 0 : outColor =  vec4(vec3(texture(uDepthSampler, fragTexCoord).r), 1.0); break;
		case 1 : outColor =  albedo; break;
		case 2 : outColor =  vec4(fragPosXyzw.xyz,1.0); break;
		case 3 : outColor =  vec4(material.normal,1.0); break;
		case 4 : outColor =  vec4(vec3(texture(uShadowMap,  vec3(fragTexCoord,ubo.displayCascadeLevel)).r), 1.0);break;
		//case 5 : outColor =  vec4(vec3(texture(uShadowCubeMap,omniVec).r),1.0);  break;
		default: 
			//outColor = vec4(fragColor,1.0);
		break;
	}

	if(ubo.colorCascade == 1){
		switch(cascadeIndex) {
			case 0 : 
				outColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
				break;
			case 1 : 
				outColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
				break;
			case 2 : 
				outColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
				break;
			case 3 : 
				outColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
				break;
		}
	}
}


