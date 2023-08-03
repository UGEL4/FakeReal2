#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 color;
    vec4 viewPos;
    vec4 lightPos;
}ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec4 outColor;
layout(location = 4) out vec4 outViewPos;
layout(location = 5) out vec4 outlightPos;
void main()
{
    vec4 worldPos = ubo.model * vec4(inPos, 1.0);
    gl_Position   = ubo.proj * ubo.view * worldPos;
    outWorldPos   = worldPos.xyz;
    outNormal     = inNormal;
    outUV         = inUV;
    outColor      = ubo.color;
    outViewPos    = ubo.viewPos;
    outlightPos   = ubo.lightPos;
    //frColor = inColor;
}