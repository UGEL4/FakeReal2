#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(set = 0, binding = 4) uniform ubo1 {mat4 inputPosition;}uo1;

vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5));
vec3 colors[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

layout(set = 0, binding = 0) uniform UniformBufferObj
{
	mat4 model;
	mat4 view;
	mat4 proj;
}ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 frColor;
layout(location = 1) out vec2 outUv;
void main()
{
	gl_Position = vec4(inPos, 1.0);
	frColor = inColor;
	outUv = inUV;
}