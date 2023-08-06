#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (location = 0) in VS_OUT {
    vec3 normal;
} gs_in[];

layout (set = 0, binding = 2) uniform UBO
{
    mat4 model;
    mat4 view;
    mat4 projection;
} nv_ubo;

const float MAGNITUDE = 0.2;

void GenerateLine(int index)
{
    gl_Position = nv_ubo.projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = nv_ubo.projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main(void)
{
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}