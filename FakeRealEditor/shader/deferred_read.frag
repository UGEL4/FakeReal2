#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput inputAlbedo;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput inputDepth;

layout(location = 0) out vec4 outColor;

void main()
{
	// Get G-Buffer values
	vec3 fragPos = subpassLoad(inputPosition).rgb;
	vec3 normal = subpassLoad(inputNormal).rgb;
	vec4 albedo = subpassLoad(inputAlbedo);
	float depth	= subpassLoad(inputDepth).r;

	// Ambient part
	vec3 fragcolor = albedo.rgb;
	//vec3 fragcolor = vec3(depth, depth, depth);
	outColor = vec4(fragcolor, 1.0);
}