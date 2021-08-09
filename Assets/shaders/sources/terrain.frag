#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1,binding = 0) uniform sampler2D texSampler;

layout(set = 1,binding = 1) uniform Light {
   vec3 position;
   vec3 color;
} light;

layout(set = 1,binding = 2) uniform Material{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
} material;


layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;


void main() {
    //outColor = texture(texSampler, fragTexCoord);
	
	vec3 viewPos = vec3(0,0,10);
	
	vec3 ambient = light.color * material.ambient;

    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.color  * (diff * material.diffuse);

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.color  * (spec * material.specular);  

    vec3 result = ambient + diffuse + specular;
    vec4 r2 = vec4(result, 1.0);
	
	outColor = texture(texSampler, fragTexCoord) * r2;

}