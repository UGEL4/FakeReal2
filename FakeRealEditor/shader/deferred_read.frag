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

	float metallic = 0;
	float roughness = 1;
	float ao = 1.0;
	vec3 view = normalize(vec3(0.0, 0.0, 2.0) - worldPos);
	albedo = vec4(0.5, 0.0, 0.0, 1.0);
	//vec3 TestCookTorranceBRDF(vec3 worldPos, vec3 view, vec3 normal, vec3 albedo, float roughness, float metallic)
	vec3 Lo = TestCookTorranceBRDF(ubo.model, worldPos, view, normalize(normal), albedo.rgb, roughness, metallic);
	vec3 ambient = vec3(0.1) * albedo.rgb * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
	outColor = vec4(color, 1.0);
	// Ambient part
	vec3 fragcolor = albedo.rgb;
	//vec3 fragcolor = vec3(depth, depth, depth);
	//outColor = vec4(fragcolor, 1.0);
}