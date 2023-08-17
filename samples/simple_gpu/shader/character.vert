#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform PushConsts
{
    mat4 mvp;
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 outUV;
/* layout(location = 3) out VS_MaterialOut
{
    float metallic;
    float roughness;
    float ao;
} outMaterial; */
void main()
{
    //mat3 normalMatrix = mat3(transpose(inverse(ubo.model)));
    //vec4 worldPos     = pushConsts.objPos + ubo.model * vec4(inPos, 1.0);
    gl_Position       = pushConsts.mvp * vec4(inPos, 1.0);
    //outWorldPos       = worldPos.xyz;
    //outNormal         = normalMatrix * inNormal;
    outUV             = inUV;
    /* outMaterial.metallic = pushConsts.metallic;
    outMaterial.roughness = pushConsts.roughness;
    outMaterial.ao = pushConsts.ao; */
}