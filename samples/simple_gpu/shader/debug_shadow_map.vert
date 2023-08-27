#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 outUV;

vec3 positions[3] = vec3[](
    vec3(-1.0, -1.0, 0.0),
    vec3(-1.0, 1.0, 0.0),
    vec3(1.0, 1.0, 0.0)
);

void main() 
{
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}