#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MeshPerDrawcallMaxInstanceCount 64

struct MeshInstance
{
    mat4 model;
};

layout(set = 0, binding = 1) readonly buffer _unused_name_per_drawcall_dynamic
{
    MeshInstance mesh_instances[MeshPerDrawcallMaxInstanceCount];
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBiTangent;

void main()
{
    vec4 worldPos = mesh_instances[gl_InstanceIndex].model * vec4(inPos, 1.0);
    gl_Position   = worldPos;
}