#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inWorldPos;

layout (set = 0, binding = 0) uniform textureCube tex;
layout (set = 0, binding = 1) uniform sampler texSamp;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const float deltaPhi = (2.0f * PI) / 180.0f;
const float deltaTheta = (0.5f * PI) / 64.0f;
const float TWO_PI = PI * 2.0;
const float HALF_PI = PI * 0.5;

void main()
{
    vec3 normal       = normalize(inWorldPos);
    vec3 irradiance   = vec3(0.0);
    vec3 up           = vec3(0.0, 1.0, 0.0);
    vec3 right        = cross(up, normal);
    up                = cross(normal, right);
    //float sampleDelta = 0.005;
    uint nrSamples = 0u; 
    for(float phi = 0.0; phi < TWO_PI; phi += deltaPhi)
    {
        for(float theta = 0.0; theta < HALF_PI; theta += deltaTheta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            /* vec3 tempVec   = cos(phi) * right + sin(phi) * up;
            vec3 sampleVec = cos(theta) * normal + sin(theta) * tempVec; */

            irradiance += texture(samplerCube(tex, texSamp), sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    outColor   = vec4(irradiance, 1.0);
}