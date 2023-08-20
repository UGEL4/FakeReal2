#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "pbr_math.h"

layout(set = 0, binding = 0) uniform sampler texSamp; //static sampler
layout(set = 0, binding = 1) uniform texture2D texDiffuse;
layout(set = 0, binding = 2) uniform texture2D texNormal;
layout(set = 0, binding = 3) uniform texture2D texMask;
layout(set = 0, binding = 4) uniform textureCube irradianceMap;
layout(set = 0, binding = 5) uniform textureCube preFilteredMap;
layout(set = 0, binding = 6) uniform texture2D brdfLutTex;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inWorldPos;
layout(location = 6) in vec3 inViewPos;
layout(location = 3) in VS_TengentOut
{
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
} fs_in;

layout(location = 0) out vec4 outColor;

/* vec3 InvertColorRGB(vec3 color, float r_factor, float g_factor, float b_factor)
{
    return vec3(color.x)
} */

vec3 prefilteredReflection(vec3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
    float lod = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    vec3 a = textureLod(samplerCube(preFilteredMap, texSamp), R, lodf).rgb;
    vec3 b = textureLod(samplerCube(preFilteredMap, texSamp), R, lodc).rgb;
    return mix(a, b, lod - lodf);
}

const float height_scale = 0.3;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height = texture(sampler2D(texMask, texSamp), texCoords).r;
    return texCoords - viewDir.xy  * (height * height_scale)/ viewDir.z;
}

void main()
{
    vec3 V  = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);
    vec2 uv = inUV * vec2(13.753, 37.592);
    uv = ParallaxMapping(inUV, V);
    // albedo
    vec4 albedo = texture(sampler2D(texDiffuse, texSamp), uv);

    float S = 0.5;
    float metallic  = 0.1;
    float roughness = 0.75;

    vec3 N = texture(sampler2D(texNormal, texSamp), uv).rgb;
    N      = normalize(N * 2.0 - 1.0);
    //N.g    = N.g * -1.0;
    vec3 R = reflect(-normalize(inViewPos - inWorldPos), N);

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo.rgb, metallic);
    vec3 Lo = vec3(0.0);
    vec3 lightColor = vec3(200.0, 200.0, 200.0);
    for (int i = 0; i < 1; i++)
    {
        vec3 L            = normalize(fs_in.tangentLightPos - fs_in.tangentFragPos);
        vec3 H            = normalize(V + L);
        float dis         = length(fs_in.tangentLightPos - fs_in.tangentFragPos);
        float attenuation = 1.0 / (dis * dis);
        vec3 radiance     = lightColor * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        vec3 Ks = F;
        vec3 Kd = vec3(1.0) - Ks;
        Kd *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (Kd * albedo.rgb / PI + specular) * radiance * NdotL;
    }
    float ao = 1.0;

    vec3 F  = fresnelSchlickRoughness(clamp(dot(N, V), 0.0, 1.0), F0, roughness);
    vec3 Ks = F;
    vec3 Kd = 1.0 - Ks;
    Kd *= (1.0 - metallic);
    vec3 irradiance = texture(samplerCube(irradianceMap, texSamp), N).rgb;
    vec3 diffuse    = irradiance * albedo.rgb;
    vec3 reflection = prefilteredReflection(R, roughness).rgb;
    vec2 brdf       = texture(sampler2D(brdfLutTex, texSamp), vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular   = reflection * (F * brdf.x + brdf.y);
    vec3 ambient    = (Kd * diffuse + specular * S) * ao;

    //vec3 ambient = vec3(0.03) * albedo.rgb * ao;

    vec3 color 	 = ambient + Lo;
    color        = color / (color + vec3(1.0));
    color        = pow(color, vec3(1.0 / 2.2));
    outColor     = vec4(color, 1.0);


    /* vec3 ambient = 1.0 * albedo.rgb;
    // Diffuse
    vec3 lightDir = normalize(inLightPos - inWorldPos);
    float diff = max(dot(lightDir, N), 0.0);
    vec3 diffuse = diff * albedo.rgb;
    // Specular
    vec3 viewDir = normalize(inViewPos - inWorldPos);
    vec3 reflectDir = reflect(-lightDir, N);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(N, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;
    outColor = vec4(ambient + diffuse + specular, 1.0f);; */
}