#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 4) uniform UniformBufferObj
{
    mat4 view;
    mat4 proj;
    vec4 viewPos;
}ubo;

layout(push_constant) uniform PushConsts
{
    mat4 model;
    /* layout(offset = 16) float metallic;
    layout(offset = 20) float roughness;
    layout(offset = 24) float ao;
    layout(offset = 32) float padding; */
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(pushConsts.model)));
    vec4 worldPos     = pushConsts.model * vec4(inPos, 1.0);
    gl_Position       = ubo.proj * ubo.view * worldPos;
    outWorldPos       = worldPos.xyz;
    outNormal         = normalMatrix * inNormal;
    outUV             = inUV;
}