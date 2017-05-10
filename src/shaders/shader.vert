#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform MVPMatrices {
	mat4 model, view, proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 tangent;
layout(location = 2) in vec3 bitangent;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosition;
layout(location = 2) out mat3 tbn;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragPosition = (ubo.model * vec4(inPosition, 1.0)).xyz;
	fragTexCoord = inTexCoord;
	tbn = mat3(mat3(ubo.model) * normalize(tangent),
		mat3(ubo.model) * normalize(bitangent),
		mat3(ubo.model) * normalize(normal));
}

