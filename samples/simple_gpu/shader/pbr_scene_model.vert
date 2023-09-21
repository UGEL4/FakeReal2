#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(std140, set = 0, binding = 4) uniform PerframeUniformBuffer
{
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMat[4];
    vec4 viewPos;
    DirectionalLight directionalLight;
    PointLight pointLight;
    float cascadeSplits[4];
} ubo;

layout(push_constant) uniform PushConsts
{
    mat4 model;
    layout(offset = 64) float offsets[8];
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

layout(location = 3) out VS_TengentOut
{
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    //vec4 lightSpacePos;
    vec4 fragViewPos;
    mat3 TBN;
} vs_out;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(pushConsts.model)));
    vec3 pos = inPos;
    pos.x += pushConsts.offsets[gl_InstanceIndex];
    vec4 worldPos     = pushConsts.model * vec4(pos, 1.0);
    //worldPos.x += pushConsts.offsets[gl_InstanceIndex];
    gl_Position       = ubo.proj * ubo.view * worldPos;
    outWorldPos       = worldPos.xyz;
    outNormal         = normalMatrix * inNormal;
    outUV             = inUV;

    vec3 T = normalize(normalMatrix * inTangent);
    vec3 B = normalize(normalMatrix * inBiTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.tangentViewPos  = TBN * ubo.viewPos.xyz;
    vs_out.tangentFragPos  = TBN * outWorldPos;
    vs_out.TBN             = mat3(T, B, N);
    //vs_out.lightSpacePos = ubo.lightSpaceMat * pushConsts.model * vec4(inPos, 1.0);
    vs_out.fragViewPos   = ubo.view * worldPos;
}