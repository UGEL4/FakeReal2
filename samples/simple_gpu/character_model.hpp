#pragma once

#include "Gpu/GpuApi.h"
#include <string_view>
#include <vector>
#include "model.hpp"

struct Material
{

};
typedef const Material* MaterialID;

enum MaterialTextureType
{
    MTT_DIFFUSE = 0,
    MTT_NORMAL = 1,
    MTT_MASK = 2
};

struct MaterialInstance
{
    GPURenderPipelineID pipeline;
    GPURootSignatureID rootSignature;
    GPUDescriptorSetID set;
    GPUSamplerID sampler;
    GPUTextureID diffuseTexture;
    GPUTextureViewID diffuseTextureView;
    GPUTextureID normalTexture;
    GPUTextureViewID normalTextureView;
    GPUTextureID maskTexture;
    GPUTextureViewID maskTextureView;
    struct Pack
    {
        GPUTextureID texture;
        GPUTextureViewID textureView;
        MaterialTextureType textureType;
        uint32_t slotIndex;
    };
    std::vector<Pack> textures;
};
typedef const MaterialInstance* MaterialInstanceID;

//MaterialInstanceID CreateMaterial(const std::string_view diffuse, const std::string_view normal, const std::string_view mask, GPUDeviceID device, GPUQueueID gfxQueue, bool flip = true);
//std::pair<texture_type, std::pair<texture_file, flip_uv>>
MaterialInstanceID CreateMaterial(const std::vector<std::pair<MaterialTextureType, std::pair<std::string_view, bool>>>& textures,
                                  const std::vector<std::pair<EGPUShaderStage, std::string_view>>& shaders,
                                  GPUDeviceID device, GPUQueueID gfxQueue,  class SkyBox* skyBox);
void DestryMaterial(MaterialInstanceID m);

class CharacterModel
{
public:
    CharacterModel();
    ~CharacterModel();
    void LoadModel(const std::string_view file);
    void InitMaterials(GPUDeviceID device, GPUQueueID gfxQueue,  class SkyBox* skyBox);
    void InitModelResource(GPUDeviceID device, GPUQueueID gfxQueue,  class SkyBox* skyBox);
    void Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos);
    void BindEnvTexture(GPUTextureViewID irradianceMap, GPUTextureViewID prefilteredMap, GPUTextureViewID brdfLutMap);

    std::unordered_map<uint32_t, MaterialInstanceID> mMaterials;
    Mesh mMesh;

    GPUBufferID mVertexBuffer;
    GPUBufferID mIndexBuffer;
    GPUSamplerID mSampler;
};