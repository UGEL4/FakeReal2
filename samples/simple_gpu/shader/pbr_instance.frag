#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#include "pbr_math.h"

struct DirectionalLight // align:16
{
    vec3 direction; // offset 0
    float padding_direction; // offset 12
    vec3 color; // offset 16
    float padding_color; //offset 28
}; // size : 32

struct PointLight // align:16
{
    vec3 position; // offset 0
    float padding_position; // offset 12
    vec3 color; // offset 16
    float padding_color; // offset 28
    float constant; // offset 32
    float linear; // offset 36
    float quadratic; // offset 40
    float padding; // offset 44
}; // size : 48

layout(set = 0, binding = 1) uniform textureCube texIrradianMap;
layout(set = 0, binding = 2) uniform textureCube texPrefilteredMap;
layout(set = 0, binding = 3) uniform texture2D texBRDFLut;
layout(set = 0, binding = 4) uniform sampler envTexSamp;
layout(std140, set = 0, binding = 5) uniform PerframeUniformBuffer
{
    mat4 view;
    mat4 proj;
    mat4 lightSpaceMat[4];
    vec4 viewPos;
    DirectionalLight directionalLight;
    PointLight pointLight;
    float cascadeSplits[4];
} perFrameUbo;

layout(set = 1, binding = 0) uniform sampler texSamp;
layout(set = 1, binding = 1) uniform texture2D baseColor;
/* layout(set = 1, binding = 2) uniform texture2D normalMap;
layout(set = 1, binding = 3) uniform texture2D metallicMap;
layout(set = 1, binding = 4) uniform texture2D roughnessMap; */

/* layout(set = 2, binding = 0) uniform texture2DArray shadowMap;
layout(set = 2, binding = 1) uniform sampler shadowSamp; */


layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in VS_TengentOut
{
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    //vec4 lightSpacePos;
    vec4 fragViewPos;
    vec4 instance_id;
    mat3 TBN;
} fs_in;

layout(location = 0) out vec4 outColor;

float CascadedShadowCalculation(vec4 fragPosLightSpace, uint cascadeIndex, vec3 N)
{
    // perform perspective divide
    /* vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float bias = max(0.05 * (1.0 - dot(N, normalize(perFrameUbo.directionalLight.direction))), 0.0000075);
    float closestDepth = texture(sampler2DArray(shadowMap, shadowSamp), vec3(projCoords.xy, cascadeIndex)).r + 0.0000075; 
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0; */

    return 0.0;
}

void main()
{
    vec3 color      = texture(sampler2D(baseColor, texSamp), inUV).rgb;
    //float metallic  = texture(sampler2D(metallicMap, texSamp), inUV).r;
    //float roughness = texture(sampler2D(roughnessMap, texSamp), inUV).r;
    //color = vec3(1.0, 1.0, 1.0);
    // ambient
    vec3 ambient  = 0.5 * color;

    vec3 lightDir = normalize(perFrameUbo.directionalLight.direction);
    vec3 normal   = normalize(inNormal);
    float diff    = max(dot(normal, lightDir), 0.0);
    vec3 diffuse  = perFrameUbo.directionalLight.color * diff * color;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir    = normalize(perFrameUbo.viewPos.xyz - inWorldPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular   = specularStrength * spec * vec3(0.5);

    int enablePCF = 0;
    //float shadow  = (enablePCF == 1) ? filterPCF(fs_in.lightSpacePos) : ShadowCalculation(fs_in.lightSpacePos, normal);
    // Get cascade index for the current fragment's view position
	uint cascadeIndex = 0;
	for(uint i = 0; i < 4 - 1; ++i) {
		if(fs_in.fragViewPos.z < perFrameUbo.cascadeSplits[i]) {	
			cascadeIndex = i + 1;
		}
	}

    // Depth compare for shadowing
	vec4 shadowCoord = (perFrameUbo.lightSpaceMat[cascadeIndex]) * vec4(inWorldPos, 1.0);	
    float shadow  = CascadedShadowCalculation(shadowCoord, cascadeIndex, normal);
    //vec3 lighting = ambient * ((1.0 - shadow) * color);
    vec3 lighting = (ambient +  (1.0 - shadow) * (diffuse + specular));
    //vec3 lighting = (ambient +  vec3(fs_in.instance_id.xyz));
    //lighting      += CalcPointLight(perFrameUbo.pointLight, color, normal, inWorldPos, viewDir);
    outColor= vec4(lighting, 1.0);
/*     switch(cascadeIndex) {
			case 0 : 
				outColor.rgb *= vec3(1.0f, 0.0f, 0.0f);
				break;
			case 1 : 
				outColor.rgb *= vec3(0.0f, 1.0f, 0.0f);
				break;
			case 2 : 
				outColor.rgb *= vec3(0.0f, 0.0f, 1.0f);
				break;
			case 3 : 
				outColor.rgb *= vec3(1.0f, 1.0f, 1.0f);
				break;
                case 4 : 
				outColor.rgb *= vec3(0.5f, 0.0f, 0.0f);
				break;
                case 5 : 
				outColor.rgb *= vec3(0.6f, 0.0f, 0.0f);
				break;
		} */
}