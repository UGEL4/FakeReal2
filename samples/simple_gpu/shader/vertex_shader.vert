#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform UniformBufferObj
{
    mat4 model;
    mat4 view;
    mat4 proj;
    vec4 viewPos;
}ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(ubo.model)));
    vec4 worldPos     = ubo.model * vec4(inPos, 1.0);
    gl_Position       = ubo.proj * ubo.view * worldPos;
    outWorldPos       = worldPos.xyz;
    outNormal         = normalMatrix * inNormal;
    outUV             = inUV;
}