#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObj
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 viewPos;
}ubo;

layout(push_constant) uniform PushConsts
{
    mat4 model;
    layout(offset = 64) float metallic;
    layout(offset = 68) float roughness;
    layout(offset = 72) float ao;
    layout(offset = 76) float padding;
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
layout(location = 3) out VS_MaterialOut
{
    float metallic;
    float roughness;
    float ao;
} outMaterial;
void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(pushConsts.model)));
    //mat3 normalMatrix = mat3(ubo.model);
    vec4 worldPos     = pushConsts.model * vec4(inPos, 1.0);
    //vec4 worldPos     = ubo.model * vec4(inPos, 1.0) + pushConsts.objPos;
    outWorldPos       = worldPos.xyz;
    gl_Position       = ubo.proj * ubo.view * worldPos;
    //outWorldPos       = worldPos.xyz;
    outNormal             = normalMatrix * inNormal;
    outUV                 = inUV;
    outMaterial.metallic  = pushConsts.metallic;
    outMaterial.roughness = pushConsts.roughness;
    outMaterial.ao        = pushConsts.ao;
}