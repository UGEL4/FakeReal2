#pragma once

#include "Gpu/GpuApi.h"
#include <string_view>
#include <vector>
#include "model.hpp"

struct Material
{

};
typedef const Material* MaterialID;

struct MaterialInstance
{
    GPURenderPipelineID pipeline;
    GPURootSignatureID rootSignature;
    GPUDescriptorSetID set;
    GPUTextureID diffuseTexture;
    GPUTextureViewID diffuseTextureView;
    GPUTextureID normalTexture;
    GPUTextureViewID normalTextureView;
    GPUTextureID maskTexture;
    GPUTextureViewID maskTextureView;
};
typedef const MaterialInstance* MaterialInstanceID;

MaterialInstanceID CreateMaterial(const std::string_view diffuse, const std::string_view normal, const std::string_view mask, GPUDeviceID device, GPUQueueID gfxQueue);
void DestryMaterial(MaterialInstanceID m);

class CharacterModel
{
public:
    CharacterModel();
    ~CharacterModel();
    void LoadModel(const std::string_view file);
    void InitMaterials(GPUDeviceID device, GPUQueueID gfxQueue);
    void InitModelResource(GPUDeviceID device, GPUQueueID gfxQueue);
    void Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos);

    std::unordered_map<uint32_t, MaterialInstanceID> mMaterials;
    Mesh mMesh;

    GPUBufferID mVertexBuffer;
    GPUBufferID mIndexBuffer;
    GPUSamplerID mSampler;
};