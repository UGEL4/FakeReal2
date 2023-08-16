#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inWorldPos;

layout (set = 0, binding = 0) uniform textureCube tex;
layout (set = 0, binding = 1) uniform sampler texSamp;

layout(push_constant) uniform PushConsts
{
    layout(offset = 128) float roughness;
    layout(offset = 132) uint numSamples;
} pushConsts;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co)
{
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt= dot(co.xy ,vec2(a,b));
    float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

vec2 Hammersley2d(uint i, uint N) 
{
    // Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return vec2(float(i) /float(N), rdi);
}

// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    /* float a = roughness*roughness;

    float phi = 2.0 * PI * Xi.x + random(N.xz) * 0.1;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec); */
    // Maps a 2D point to a hemisphere with spread based on roughness
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x + random(N.xz) * 0.1;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha*alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    // Tangent space
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, N));
    vec3 tangentY = normalize(cross(N, tangentX));

    // Convert to world Space
    return normalize(tangentX * H.x + tangentY * H.y + N * H.z);
}

// Normal Distribution function
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2)/(PI * denom*denom); 
}

void main()
{
    vec3 normal = normalize(inWorldPos);
    vec3 V      = normal;
    float resolution    = float(textureSize(samplerCube(tex, texSamp), 0).s);
    vec3 prefilterColor = vec3(0.0);
    float totalWeight   = 0.0;
    for(uint i = 0; i < pushConsts.numSamples; i++)
    {
        vec2 Xi = Hammersley2d(i, pushConsts.numSamples);
        vec3 H  = ImportanceSampleGGX(Xi, normal, pushConsts.roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = clamp(dot(normal, L), 0.0, 1.0);
        if (NdotL > 0.0)
        {
            // Filtering based on https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
            float NdotH = clamp(dot(normal, H), 0.0, 1.0);
            float VdotH = clamp(dot(H, V), 0.0, 1.0);
            // Probability Distribution Function
            float pdf   = D_GGX(NdotH, pushConsts.roughness) * NdotH / (4.0 * VdotH) + 0.0001;
            // Slid angle of current smple
            float omegaS = 1.0 / (float(pushConsts.numSamples) * pdf);
            // Solid angle of 1 pixel across all cube faces
            float omegaP = 4.0 * PI / (6.0 * resolution * resolution);
            float mipLevel = pushConsts.roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP) + 1.0, 0.0);
            prefilterColor += textureLod(samplerCube(tex, texSamp), L, mipLevel).rgb * NdotL;
            totalWeight    += NdotL;
        }
    }
    prefilterColor = prefilterColor / totalWeight;
    outColor       = vec4(prefilterColor, 1.0);
}