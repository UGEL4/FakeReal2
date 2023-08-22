#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.h"

layout(set = 0, binding = 0) uniform textureCube texIrradianMap;
layout(set = 0, binding = 1) uniform textureCube texPrefilteredMap;
layout(set = 0, binding = 2) uniform texture2D texBRDFLut;
layout(set = 0, binding = 3) uniform sampler envTexSamp; //static sampler

layout(set = 1, binding = 0) uniform sampler texSamp; //static sampler
layout(set = 1, binding = 1) uniform texture2D baseColor;
/* layout(set = 1, binding = 1) uniform texture2D normalMap;
layout(set = 1, binding = 2) uniform texture2D baseColor; */


layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outColor;
void main()
{
    /* vec4 albedo     = texture(sampler2D(tex, texSamp), inUV);
    //vec4 albedo     = vec4(1.0, 0.0, 0.0, 1.0);
    vec3 N          = normalize(inNormal);
    vec3 view       = normalize(ubo.viewPos.xyz - inWorldPos);

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo.rgb, inPBRMat.metallic);
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < MAX_LIGHT_NUM; i++)
    {
        vec3 L            = normalize(Lights.lightPos[i].xyz - inWorldPos);
        vec3 H            = normalize(view + L);
        float dis         = length(Lights.lightPos[i].xyz - inWorldPos);
        float attenuation = 1.0 / (dis * dis);
        vec3 radiance     = Lights.lightColor[i].xyz * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, inPBRMat.roughness);
        float G   = GeometrySmith(N, view, L, inPBRMat.roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, view), 0.0, 1.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, view), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        vec3 Ks = F;
        vec3 Kd = vec3(1.0) - Ks;
        Kd *= (1.0 - inPBRMat.metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (Kd * albedo.rgb / PI + specular) * radiance * NdotL;
    }
    vec3 ambient = vec3(0.03) * albedo.rgb * inPBRMat.ao;
    vec3 color 	 = ambient + Lo;
    color        = color / (color + vec3(1.0));
    color        = pow(color, vec3(1.0 / 2.2));
    outColor     = vec4(color, 1.0); */
    //outColor     = vec4(Lights.lightColor[1].xyz, 1.0);

    //outColor = vec4(inColor.rgb, 1.0);
    //outClor = texture(tex, outUv);
    //outClor = albedo;
    vec4 albedo = texture(sampler2D(baseColor, texSamp), inUV);
    outColor    = albedo;
}