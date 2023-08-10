#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;


layout (location = 0) out VS_OUT {
    vec3 normal;
} vs_out;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 viewPos;
} ubo;

layout(push_constant) uniform PushConsts
{
    vec4 objPos;
    layout(offset = 16) float metallic;
    layout(offset = 20) float roughness;
    layout(offset = 24) float ao;
    layout(offset = 32) float padding;
} pushConsts;

void main(void){
    mat3 normalMatrix = mat3(transpose(inverse(ubo.view * ubo.model)));
    vs_out.normal     = vec3(vec4(normalMatrix * inNormal, 0.0));
    gl_Position       = ubo.view * (pushConsts.objPos + ubo.model * vec4(inPos, 1.0)); 
}