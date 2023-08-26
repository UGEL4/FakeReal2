#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObj
{
    mat4 lightSpaceMatrix;
}ubo;

layout(push_constant) uniform PushConsts
{
    mat4 model;
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;

void main()
{
    vec4 worldPos = pushConsts.model * vec4(inPos, 1.0);
    gl_Position   = ubo.lightSpaceMatrix * worldPos;
}