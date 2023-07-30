#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(set = 0, binding = 0) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
}ubo;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outWorldPos;

void main()
{
	vec4 worldPos = ubo.model * vec4(position, 1.0);
	gl_Position   = ubo.proj * ubo.view * worldPos;
	outColor      = color;
	outTexCoord   = texCoord;
	outWorldPos   = worldPos.xyz;
}