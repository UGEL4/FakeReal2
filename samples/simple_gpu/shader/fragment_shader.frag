#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputPosition;
//layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputNormal;
//layout(input_attachment_index = 2, binding = 2) uniform subpassInput inputAlbedo;
//layout(input_attachment_index = 3, binding = 3) uniform subpassInput inputDepth;

//layout(set = 0, binding = 4) uniform ubo2 {mat4 inputPosition;}uo2;

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 2) uniform sampler texSamp; //static sampler

layout(location = 0) out vec4 outClor;
layout(location = 0) in vec3 frColor;
layout(location = 1) in vec2 outUv;
void main()
{
	outClor = vec4(frColor, 1.0);
	//outClor = texture(tex, outUv);
	//outClor = texture(sampler2D(tex, texSamp), outUv);
}