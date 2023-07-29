//#pragma once

#define PI 3.14159265359

// F : Freh-nel
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    // F0+(1−F0)pow(1−(h⋅v), 5);
    vec3 val = F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); // clamp避免出现黑点
    return val;
}

// D : Normal Distribution Function
float TrowbridgeReitzGGX(vec3 n, vec3 h, float roughness)
{
    float roughness_sq = roughness * roughness;
    float roughness_sq_2 = roughness_sq * roughness_sq;
    float n_Dot_h = max(dot(n, h), 0.0);

    float denominator = (n_Dot_h * n_Dot_h) * (roughness_sq_2 - 1.0) + 1.0;

    return roughness_sq_2 / (PI * denominator * denominator);
}

// G : Geometry Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    //direction light
    float r  = roughness + 1.0;
    float rr = r * r;
    float k  = rr / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdotV, roughness);
    float ggx2  = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

//P : world pos
//lightPos : light pos
//V : view dir
//N ： normalized normal dir
//LightColor
vec3 CookTorranceBRDF(vec3 P, vec3 lightPos, vec3 V, vec3 N, vec3 lightColor, vec3 albedo, float roughness, float metallic, vec3 F0)
{
    vec3 L = normalize(lightPos - P);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - P);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    //DFG
    //NDF
    float NDF = TrowbridgeReitzGGX(N, H, roughness);
    //F
    vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    //G
    float G = GeometrySmith(N, V, L, roughness);

    vec3 ks = F;
    vec3 kd = vec3(1.0) - ks;
    kd *= 1.0 - metallic;

    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = (NDF * G * F) / denominator;

    float NdotL = max(dot(N, L), 0.0);

    return (kd * albedo / PI + specular) * radiance * NdotL;
}

vec3 TestCookTorranceBRDF(mat4 model, vec3 worldPos, vec3 view, vec3 normal, vec3 albedo, float roughness, float metallic)
{
    vec3 lightPos[1] = vec3[](vec3(0, 6, 0));
    vec3 lightColors[1] = vec3[](vec3(300.0, 300.0, 300.0));

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 1; i++)
    {
        Lo += CookTorranceBRDF(worldPos, lightPos[i], view, normal, lightColors[i], albedo, roughness, metallic, F0);
    }

    return Lo;
}