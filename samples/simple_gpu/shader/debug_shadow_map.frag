#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform texture2D tex;
layout(set = 1, binding = 1) uniform sampler texSamp;

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

/* float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
} */

float LinearizeDepth(float depth)
{
  float n = 0.0;
  float f = 1000.0;
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}
void main()
{
    vec3 depthValue = texture(sampler2D(tex, texSamp), inUv).rgb;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    outColor = vec4(vec3(depthValue), 1.0); // orthographic
    float depth = texture(sampler2D(tex, texSamp), inUv).r;
	//outColor = vec4(vec3(1.0-LinearizeDepth(depth)), 1.0);
}