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
layout(set = 1, binding = 2) uniform texture2D normalMap;
layout(set = 1, binding = 3) uniform texture2D metallicMap;
layout(set = 1, binding = 4) uniform texture2D roughnessMap;

layout(set = 2, binding = 0) uniform texture2D shadowMap;


layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in VS_TengentOut
{
    vec3 viewPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    vec4 lightSpacePos;
    mat3 TBN;
} fs_in;

layout(location = 0) out vec4 outColor;

vec3 prefilteredReflection(vec3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
    float lod  = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    vec3 a = textureLod(samplerCube(texPrefilteredMap, envTexSamp), R, lodf).rgb;
    vec3 b = textureLod(samplerCube(texPrefilteredMap, envTexSamp), R, lodc).rgb;
    return mix(a, b, lod - lodf);
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(sampler2D(normalMap, texSamp), inUV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(inWorldPos);
    vec3 Q2  = dFdy(inWorldPos);
    vec2 st1 = dFdx(inUV);
    vec2 st2 = dFdy(inUV);

    vec3 N   = normalize(inNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(sampler2D(shadowMap, texSamp), projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

float textureProj(vec4 fragPosLightSpace, vec2 off)
{
	float shadow = 1.0;
    vec4 shadowCoord = fragPosLightSpace / fragPosLightSpace.w;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture(sampler2D(shadowMap, texSamp), shadowCoord.xy + off).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

void main()
{
    vec3 albedo     = pow(texture(sampler2D(baseColor, texSamp), inUV).rgb, vec3(2.0));
    //vec4 albedo     = vec4(1.0, 0.0, 0.0, 1.0);
    //vec3 N = texture(sampler2D(normalMap, texSamp), inUV).rgb;
    //N      = normalize(N * 2.0 - 1.0);
    vec3 N = getNormalFromMap();
    //vec3 N = normalize(texture(sampler2D(normalMap, texSamp), inUV).rgb * 2.0 - 1.0);
    //N = normalize(fs_in.TBN * N);
    //N.g = N.g * -1.0;
    //vec3 V = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);
    vec3 V = normalize(fs_in.viewPos - inWorldPos);
    vec3 R = reflect(-V, N);

    float metallic  = texture(sampler2D(metallicMap, texSamp), inUV).r;
    float roughness = texture(sampler2D(roughnessMap, texSamp), inUV).r;

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    /* for (int i = 0; i < MAX_LIGHT_NUM; i++)
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
    } */
    float ao = 1.0;
    vec3 F  = fresnelSchlickRoughness(clamp(dot(N, V), 0.0, 1.0), F0, roughness);
    vec3 Ks = F;
    vec3 Kd = 1.0 - Ks;
    Kd *= (1.0 - metallic);

    vec3 irradiance = texture(samplerCube(texIrradianMap, envTexSamp), N).rgb;
    vec3 reflection = prefilteredReflection(R, roughness).rgb;
    //reflection = textureLod(samplerCube(texPrefilteredMap, envTexSamp), R, roughness * 9.0).rgb;
    vec2 brdf       = texture(sampler2D(texBRDFLut, envTexSamp), vec2(max(dot(N, V), 0.0), roughness)).rg;

    float shadow = textureProj(fs_in.lightSpacePos, vec2(0.0));
    vec3 diffuse    = irradiance * albedo; diffuse *= (shadow);
    vec3 specular   = reflection * (F * brdf.x + brdf.y); specular*=shadow;
    vec3 ambient    = (Kd * diffuse + specular) * ao;
    vec3 color 	 = ambient + Lo;

    //color *= shadow;
    color        = color / (color + vec3(1.0));
    color        = pow(color, vec3(1.0 / 2.2));
    outColor     = vec4(color, 1.0);
    //outColor     = vec4(Lights.lightColor[1].xyz, 1.0);

    //outColor = vec4(inColor.rgb, 1.0);
    //outClor = texture(tex, outUv);
    //outClor = albedo;
    /* vec4 albedo = texture(sampler2D(baseColor, texSamp), inUV);
    outColor    = albedo; */
}