#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConsts
{
    mat4 view;
    layout(offset = 64) mat4 proj;
} pushConsts;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;

void main()
{
    outWorldPos   = inPos;
    gl_Position   = pushConsts.proj * pushConsts.view * vec4(inPos, 1.0);
}