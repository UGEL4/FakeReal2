#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inWorldPos;

layout (set = 0, binding = 0) uniform texture2D tex;
layout (set = 0, binding = 1) uniform sampler texSamp;

layout (location = 0) out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv    = SampleSphericalMap(normalize(inWorldPos));
    vec3 color = texture(sampler2D(tex, texSamp), uv).rgb;
    outColor   = vec4(color, 1.0);
}