#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObj
{
    mat4 model;
    mat4 proj;
    vec4 viewPos;
}ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outUVW;
layout(location = 1) out vec3 outColor;

void main()
{
    gl_Position   = ubo.proj * ubo.model * vec4(inPos, 1.0);
    gl_Position.z = gl_Position.w;
    outUVW        = inPos;
    outColor      = inNormal;
}