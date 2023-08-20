#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform PushConsts
{
    mat4 m;
    mat4 v;
    mat4 p;
    vec3 viewPos;
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outWorldPos;
layout(location = 6) out vec3 outViewPos;

layout(location = 3) out VS_TengentOut
{
    vec3 tangentLightPos;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
} vs_out;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(pushConsts.m)));
    gl_Position       = pushConsts.p * pushConsts.v * pushConsts.m * vec4(inPos, 1.0);
    outWorldPos       = (pushConsts.m * vec4(inPos, 1.0)).xyz;
    outNormal         = normalMatrix * inNormal;
    outUV             = inUV;
    outViewPos        = pushConsts.viewPos;

    vec3 lightPos = vec3(0.0, 20.0, 10.0);
    vec3 T = normalize(normalMatrix * inTangent);
    vec3 B = normalize(normalMatrix * inBiTangent);
    vec3 N = normalize(normalMatrix * inNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.tangentLightPos = TBN * lightPos;
    vs_out.tangentViewPos  = TBN * pushConsts.viewPos;
    vs_out.tangentFragPos  = TBN * (pushConsts.m * vec4(inPos, 1.0)).xyz;
}