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



#define PI 3.1415926535897932384626433832795
#define GAMMA 2.2
#define MAX_LIGHTS 32
#define MAX_SHADOWMAPS 4

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
	float normalDotV;
};


#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.1
#define SHADOW_MAX 4

layout(std140, binding = 0) uniform UniformBufferLight
{
	Light lights[MAX_LIGHTS]; //32 * 
	mat4 viewPos;
	mat4 lightProjView[4];

	vec4 splitDepth;

	int lightCount; //4
	int type;
	int colorCascade;
	int displayCascadeLevel;

	float bias;
	float bias1;
	float bias2;
	float bias3;
} ubo;


const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);


float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex,vec3 normal,vec3 lightDirection)
{
	float shadow = 1.0;

	float bias = ubo.bias + (ubo.bias * tan(acos(clamp(dot(normal, lightDirection.xyz), 0.0, 1.0))) * 0.5);

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {
		float dist = texture(uShadowMap, vec3(shadowCoord.st + offset, cascadeIndex)).r;
		if (shadowCoord.w > 0 && dist < shadowCoord.z - ubo.bias) {
			shadow = AMBIENT_LIGHT;
		}
	}
	return shadow;

}

float filterPCF(vec4 sc, uint cascadeIndex,vec3 normal,vec3 lightDirection)
{
	ivec2 texDim = textureSize(uShadowMap, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y), cascadeIndex,normal,lightDirection);
			count++;
		}
	}
	return shadowFactor / count;
}


#define EPSILON 0.15
#define SHADOW_OPACITY 0.5


void main()
{

	vec3 fragPos = texture(uPositionSampler, fragTexCoord).xyz;
	vec3 normal = texture(uNormalSampler, fragTexCoord).rgb;
	vec4 albedo = texture(uColorSampler, fragTexCoord);

	if (albedo.a < 0.5) {
		discard;
	}

	vec3 fragColor = albedo.rgb * AMBIENT_LIGHT;
	vec3 N = normalize(normal);


	uint cascadeIndex = 0;
	vec4 viewPos = ubo.viewPos * texture(uPositionSampler, fragTexCoord).xyzw;
	//get current frag's viewPos
	

	vec3 omniVec = vec3(0);

	for(uint i = 0; i < 3; i++) {
		if(viewPos.z <= ubo.splitDepth[i]) {	
			cascadeIndex = i + 1;
		}
	}

	for(int i = 0;i<ubo.lightCount;i++)
	{
		if(ubo.lights[i].type == 3)
		{
			vec3 L = normalize(ubo.lights[i].direction.xyz);
			vec3 H = normalize(L + fragPos);
			float diffuse = max(dot(N, L), AMBIENT_LIGHT);
			vec3 color = max(ubo.lights[i].color.rgb * (diffuse * albedo.rgb), vec3(0.0));

			vec4 shadowCoord = (biasMat * ubo.lightProjView[cascadeIndex]) * vec4(fragPos, 1.0);	
			fragColor += color * filterPCF(shadowCoord/shadowCoord.w,cascadeIndex,N,ubo.lights[i].direction.xyz);

		}
		else if(ubo.lights[i].type == 0)// directional lighting
		{
			vec3 L = ubo.lights[i].position.xyz - fragPos;	
			float dist = length(L);
			L = normalize(L);	
			// Viewer to fragment
			vec3 cameraPos = inverse(ubo.viewPos)[3].xyz;

			vec3 V = cameraPos - fragPos;
			V = normalize(V);

			float lightCosInnerAngle = cos(radians(15.0));
			float lightCosOuterAngle = cos(radians(25.0));
			float lightRange = 100.0;

			// Direction vector from source to target
			vec3 dir = normalize(ubo.lights[i].direction.xyz);

			// Dual cone spot light with smooth transition between inner and outer angle
			float cosDir = dot(L, dir);
			float spotEffect = smoothstep(lightCosOuterAngle, lightCosInnerAngle, cosDir);
			float heightAttenuation = smoothstep(lightRange, 0.0f, dist);

			// Diffuse lighting
			float NdotL = max(0.0, dot(N, L));

			vec4 shadowCoord = (biasMat * ubo.lightProjView[cascadeIndex]) * vec4(fragPos, 1.0);	
			float shadow = filterPCF(shadowCoord/shadowCoord.w,cascadeIndex,N,ubo.lights[i].direction.xyz);

			vec3 diff = vec3(NdotL) * shadow;



			// Specular lighting
			vec3 R = reflect(-L, N);
			float NdotR = max(0.0, dot(R, V));
			vec3 spec = vec3(pow(NdotR, 16.0) * albedo.a * 2.5);

			fragColor += vec3((diff + spec) * spotEffect * heightAttenuation) * ubo.lights[i].color.rgb * albedo.rgb;

			
		}
		else if(ubo.lights[i].type == 2)//   OmniLight
		{
			// Lighting
			vec3 L = normalize(vec3(1.0));	
			
			vec3 eyePos = inverse(ubo.viewPos)[3].xyz;
			
			vec3 lightDir = normalize(ubo.lights[i].position.xyz - fragPos);	

			vec3 Eye = normalize(-eyePos);
			vec3 Reflected = normalize(reflect(-lightDir, N)); 

			vec4 IAmbient = vec4(vec3(0.05), 1.0);
			vec4 IDiffuse = vec4(1.0) * max(dot(N, lightDir), 0.0);

			vec4 color = vec4(IAmbient + IDiffuse * ubo.lights[i].color *albedo);		
				
			// Shadow
			vec3 lightVec = fragPos - ubo.lights[i].position.xyz;
			float sampledDist = texture(uShadowCubeMap, lightVec).r * ubo.lights[i].radius;
			float dist = length(lightVec);

			//debugPrintfEXT("distance : %f ,sampledDist : %f",dist,sampledDist);

			// Check if fragment is in shadow
			float shadow = (dist <= sampledDist + EPSILON) ? 1.0 : SHADOW_OPACITY;

			fragColor += (color * shadow).rgb;


			omniVec = lightVec;
		}
	}


	switch(ubo.type){
		case 0 : outColor =  vec4(vec3(texture(uDepthSampler, fragTexCoord).r), 1.0); break;
		case 1 : outColor =  albedo; break;
		case 2 : outColor =  vec4(fragPos,1.0); break;
		case 3 : outColor =  vec4(normal,1.0); break;
		case 4 : outColor =  vec4(vec3(texture(uShadowMap,  vec3(fragTexCoord,ubo.displayCascadeLevel)).r), 1.0);break;
		case 5 : outColor =  vec4(vec3(texture(uShadowCubeMap,omniVec).r),1.0);  break;

		default: 
			outColor = vec4(fragColor,1.0);
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


