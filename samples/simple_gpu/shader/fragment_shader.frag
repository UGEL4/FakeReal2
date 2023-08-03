#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 2) uniform sampler texSamp; //static sampler

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inViewPos;
layout(location = 5) in vec4 inlightPos;
void main()
{
    /* vec4 albedo     = texture(sampler2D(tex, texSamp), inUV);
    vec3 N          = normalize(inNormal);
    float metallic  = 0.0;
    float roughness = 0.0;
    float ao        = 1.0;
    vec3 view       = normalize(inViewPos.xyz - inWorldPos);

    vec3 lightPos   = inlightPos.xyz;
    vec3 lightColor = vec3(1000.0, 0.0, 0.0);
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo.rgb, metallic);
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 1; i++)
    {
        vec3 L = normalize(lightPos - inWorldPos);
        vec3 H = normalize(view + L);
        float dis = length(lightPos - inWorldPos);
        float attenuation = 1.0 / (dis * dis);
        vec3 radiance = lightColor * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, view, L, roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, view), 0.0, 1.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, view), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        vec3 Ks = F;
        vec3 Kd = vec3(1.0) - Ks;
        Kd *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (Kd * albedo.rgb / PI + specular) * radiance * NdotL;
    }
    vec3 ambient = vec3(0.03) * albedo.rgb * ao;
    vec3 color 	 = ambient + Lo;
    color        = color / (color + vec3(1.0));
    color        = pow(color, vec3(1.0 / 2.2));
    outColor     = vec4(color, 1.0); */

    outColor = vec4(inColor.rgb, 1.0);
    //outClor = texture(tex, outUv);
    //outClor = albedo;
}