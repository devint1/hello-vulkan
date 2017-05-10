#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2DArray texSampler;
layout(binding = 2) uniform SceneAttributes {
	vec4 ambientColor, diffuseColor, specularColor, eyePos, lightPos, lightColor;
	float specularExp;
} ubo;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragPosition;
layout(location = 2) in mat3 tbn;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 lightDirection = normalize(ubo.lightPos.xyz - fragPosition);
	vec3 eyeDirection = normalize(ubo.eyePos.xyz - fragPosition);
	vec3 normal = tbn * normalize(texture(texSampler,
		vec3(fragTexCoord, 1)).xyz);
	vec3 reflection = reflect(-lightDirection, normal);
	float distance = length(ubo.lightPos.xyz - fragPosition);
	float attenuation = 1.0 / (pow(distance, 2.0) + 1.0);
	float diffuseComponent, specularComponent;

	if (dot(normal, lightDirection) <= 0) {
		diffuseComponent = specularComponent = 0.0;
	} else {
		diffuseComponent = max(dot(normal, lightDirection), 0.0);
		specularComponent = pow(max(dot(reflection, eyeDirection), 0.0),
			ubo.specularExp);
	}

	outColor = vec4((diffuseComponent * ubo.diffuseColor.rgb
		+ specularComponent * ubo.specularColor.rgb) * attenuation
		* ubo.lightColor.rgb + ubo.ambientColor.rgb, 1.0) * texture(texSampler,
		vec3(fragTexCoord, 0));
}

