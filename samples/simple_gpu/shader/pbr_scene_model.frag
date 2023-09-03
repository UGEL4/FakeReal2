#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.h"

struct DirectionalLight
{
    vec3 direction;
    float padding_direction;
    vec3 color;
    float padding_color;
};

struct PointLight
{
    vec3 position;
    float padding_position;
    vec3 color;
    float padding_color;
    float constant;
    float linear;
    float quadratic;
    float padding;
};

layout(set = 0, binding = 0) uniform textureCube texIrradianMap;
layout(set = 0, binding = 1) uniform textureCube texPrefilteredMap;
layout(set = 0, binding = 2) uniform texture2D texBRDFLut;
layout(set = 0, binding = 3) uniform sampler envTexSamp; //static sampler
layout(set = 0, binding = 4) uniform PerframeUniformBuffer
{
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMat;
    vec4 viewPos;
    DirectionalLight directionalLight;
    PointLight pointLight;
} perFrameUbo;

layout(set = 1, binding = 0) uniform sampler texSamp; //static sampler
layout(set = 1, binding = 1) uniform texture2D baseColor;
layout(set = 1, binding = 2) uniform texture2D normalMap;
layout(set = 1, binding = 3) uniform texture2D metallicMap;
layout(set = 1, binding = 4) uniform texture2D roughnessMap;

layout(set = 2, binding = 0) uniform texture2D shadowMap;
layout(set = 2, binding = 1) uniform sampler shadowSamp;


layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in VS_TengentOut
{
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
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(sampler2D(shadowMap, shadowSamp), projCoords.xy).r + 0.0000075; 
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}


float textureProj(vec4 fragPosLightSpace, vec2 off)
{
    float shadow       = 1.0;
    vec4 shadowCoord   = fragPosLightSpace / fragPosLightSpace.w;
    float currentDepth = shadowCoord.z;
    // transform to [0,1] range
    shadowCoord        = shadowCoord * 0.5 + 0.5;
    float closestDepth = texture(sampler2D(shadowMap, shadowSamp), shadowCoord.xy + off).r;
    shadow             = currentDepth > closestDepth ? 1.0 : 0.0;
    return shadow;
}

float filterPCF(vec4 sc)
{
    ivec2 texDim = textureSize(sampler2D(shadowMap, shadowSamp), 0);
    float scale = 1.5;
    float dx    = scale * 1.0 / float(texDim.x);
    float dy    = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
            count++;
        }
    }
    return shadowFactor / count;
}

vec3 CalcPointLight(PointLight light, vec3 baseColor, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float specularStrength = 0.5;
    // 合并结果
    vec3 ambient  = 1.0 * baseColor;
    vec3 diffuse  = light.color * diff * baseColor;
    vec3 specular = specularStrength * spec * vec3(0.2);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 color = texture(sampler2D(baseColor, texSamp), inUV).rgb;
    //color = vec3(1.0, 1.0, 1.0);
    // ambient
    vec3 ambient  = 0.3 * color;

    vec3 lightDir = normalize(-perFrameUbo.directionalLight.direction);
    vec3 normal   = normalize(inNormal);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 diffuse  = perFrameUbo.directionalLight.color * diff * color;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir    = normalize(perFrameUbo.viewPos.xyz - inWorldPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular   = specularStrength * spec * vec3(0.5);

    int enablePCF = 0;
    float shadow  = (enablePCF == 1) ? filterPCF(fs_in.lightSpacePos) : ShadowCalculation(fs_in.lightSpacePos);
    //vec3 lighting = ambient * ((1.0 - shadow) * color);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));
    //lighting      += CalcPointLight(perFrameUbo.pointLight, color, normal, inWorldPos, viewDir);
    outColor      = vec4(lighting, 1.0);
}