#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;

layout(set = 0, binding = 0) uniform UniformBufferObj
{
    mat4 lightSpaceMatrix[4];
}ubo;

void main(void)
{
    for(int i=0;i<3;i++)
    {
        gl_Position = ubo.lightSpaceMatrix[gl_InvocationID]*gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}