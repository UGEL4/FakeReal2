#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform PerframeUBO
{
    mat4 viewProj;
} ubo;

void main()
{
    gl_Position = ubo.viewProj * vec4(inPos, 1.0);
}