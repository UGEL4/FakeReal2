#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform textureCube tex;
layout(set = 0, binding = 2) uniform sampler texSamp; //static sampler

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;
void main()
{
    vec3 color = texture(samplerCube(tex, texSamp), inUVW).rgb;
    /* color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
    // Gamma correction
    color = pow(color, vec3(1.0f / uboParams.gamma)); */
    outColor = vec4(color, 1.0);
}