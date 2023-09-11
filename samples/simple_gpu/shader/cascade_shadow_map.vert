#version 450
#extension GL_ARB_separate_shader_objects : enable

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

//layout(location = 0) out vec4 outColor;

void main()
{
    vec4 worldPos = pushConsts.model * vec4(inPos, 1.0);
    worldPos.x += pushConsts.offsets[gl_InstanceIndex];
    gl_Position = worldPos;
}