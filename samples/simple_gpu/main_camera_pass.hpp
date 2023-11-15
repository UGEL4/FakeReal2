#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Gpu/GpuApi.h"
#include "Math/Matrix.h"

class MainCameraPass
{
public:
    MainCameraPass();
    ~MainCameraPass();

    void Initialize(GPUDeviceID device, GPUQueueID gfxQueue, GPUSwapchainID swapchain,
                    GPUTextureViewID* swapchainImages,
                    GPUSemaphoreID presentSemaphore,
                    GPUFenceID* presenFences,
                    GPUCommandPoolID* cmdPools,
                    GPUCommandBufferID* cmds,
                    const class SkyBox* skyBox);
    void DrawForward(const class EntityModel* modelEntity, const class Camera* cam, const class CascadeShadowPass* shadowPass);
    void DrawMeshLighting(GPURenderPassEncoderID encoder, const class EntityModel* modelEntity);
    void SetupRenderPipeline();
    void UpdateShadowMapSet(GPUTextureViewID shadowMap, GPUSamplerID sampler);

    GPUDeviceID mDevice;
    GPUQueueID mGfxQueue;
    GPURenderPipelineID mPbrPipeline{nullptr};
    GPURootSignatureID mRootSignature{nullptr};
    GPUSwapchainID mSwapchain;
    GPUTextureViewID* mSwapchainImages;
    GPUSemaphoreID mPresentSemaphore;
    GPUFenceID* mPresenFences;
    GPUCommandPoolID* mCmdPools;
    GPUCommandBufferID* mCmds;
    GPUTextureID mDepthTex{nullptr};
    GPUTextureViewID mDepthTexView{nullptr};


    uint32_t mCurrFrame {0};

    GPUDescriptorSetID mDefaultMeshDescriptorSet{nullptr};
    GPUDescriptorSetID mShadowMapSet{nullptr};
    GPUBufferID mUBO{nullptr};

public:
    const class SkyBox* mSkyBoxRef;
};