#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <vector>
#include <array>
#include "stb_image.h"
#include "Gpu/GpuApi.h"
#include <array>
#include "Math/Matrix.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "boundingbox.hpp"

using namespace FakeReal;

class EntityModel
{
public:
    EntityModel(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue);
    ~EntityModel();

    void UploadRenderResource(class SkyBox* skyBox);

public:
    void UpdateShadowMapSet(GPUTextureViewID shadowMap, GPUSamplerID sampler);
    void Draw(GPURenderPassEncoderID encoder, const class Camera* cam, const glm::vec4& viewPos, const class CascadeShadowPass* shadowPass);

    std::string mMeshFile;
    GPUDeviceID mDevice;
    GPUQueueID mGfxQueue;
    GPURenderPipelineID mPbrPipeline;
    GPURootSignatureID mRootSignature;
    GPUDescriptorSetID mSet;
    GPUDescriptorSetID mShadowMapSet;
    GPUSamplerID mSampler;
    GPUBufferID mUBO;

    ///
    FakeReal::MeshComponent mMeshComp;
    FakeReal::TransformComponent mTransformComp;
    BoundingBox mAABB;
};