#version 450

#extension GL_GOOGLE_include_directive : enable

//#include "light_math.h"

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput inputAlbedo;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput inputDepth;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	// Get G-Buffer values
	vec3 worldPos = subpassLoad(inputPosition).rgb;
	vec3 normal   = subpassLoad(inputNormal).rgb;
	vec4 albedo   = subpassLoad(inputAlbedo);
	float depth	  = subpassLoad(inputDepth).r;

	vec3 N = normalize(normal);
	float metallic  = 1.0;
	float roughness = 1.0;
	float ao 	    = 1.0;
	vec3 view 		= normalize(vec3(0.0, 0.0, 5.0) - worldPos);
	//albedo = vec4(0.5, 0.0, 0.0, 1.0);
	//vec3 TestCookTorranceBRDF(vec3 worldPos, vec3 view, vec3 normal, vec3 albedo, float roughness, float metallic)
	/* vec3 Lo 	 = TestCookTorranceBRDF(mat4(1.0), worldPos, view, N, albedo.rgb, roughness, metallic);
	vec3 ambient = vec3(0.03) * albedo.rgb * ao;
    vec3 color 	 = ambient + Lo;
	color 	 	 = color / (color + vec3(1.0));
    color  	 	 = pow(color, vec3(1.0/2.2));   */

	vec3 lightPos = vec3(0.0, -2.0, 1.0);
	vec3 lightColor = vec3(1000.0, 0.0, 0.0);
	vec3 F0 = vec3(0.98, 0.97, 0.95);
	F0 = mix(F0, albedo.rgb, metallic);
	vec3 Lo = vec3(0.0);
	for (int i = 0; i < 1; i++)
	{
		vec3 L = normalize(lightPos - worldPos);
		vec3 H = normalize(view + L);
		float dis = length(lightPos - worldPos);
		float attenuation = 1.0 / (dis * dis);
		vec3 radiance = lightColor * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G   = GeometrySmith(N, view, L, roughness);
		vec3 F    = fresnelSchlick(clamp(dot(H, view), 0.0, 1.0), F0);

		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, view), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		vec3 Ks = F;
		vec3 Kd = vec3(1.0) - Ks;
		Kd *= (1.0 - metallic);

		float NdotL = max(dot(N, L), 0.0);

		Lo += (Kd * albedo.rgb / PI + specular) * radiance * NdotL;
	}
	vec3 ambient = vec3(0.03) * albedo.rgb * ao;
    vec3 color 	 = ambient + Lo;
	color 	     = color / (color + vec3(1.0));
    color  	     = pow(color, vec3(1.0/2.2));

	outColor = vec4(color, 1.0);
	// Ambient part
	//vec3 fragcolor = albedo.rgb;
	//vec3 fragcolor = vec3(depth, depth, depth);
	//outColor = vec4(fragcolor, 1.0);
}