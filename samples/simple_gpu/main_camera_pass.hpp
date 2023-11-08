#pragma once
#include "Gpu/GpuApi.h"
#include "Math/Matrix.h"

/* struct DirectionalLight
{
    glm::vec3 direction;
    float padding_direction;
    glm::vec3 color;
    float padding_color;
};

struct PointLight
{
    glm::vec3 position;
    float padding_position;
    glm::vec3 color;
    float padding_color;
    float constant;
    float linear;
    float quadratic;
    float padding;
};

struct PerframeUniformBuffer
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpaceMat[4];
    glm::vec4 viewPos;
    DirectionalLight directionalLight;
    PointLight pointLight;
    float cascadeSplits[16]; //stupid std140， 16 byte align, need 16 * 4 bytes, that means 16 * sizeof(float)
};
 */
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
    GPURenderPipelineID mPbrPipeline;
    GPURootSignatureID mRootSignature;
    GPUSwapchainID mSwapchain;
    GPUTextureViewID* mSwapchainImages;
    GPUSemaphoreID mPresentSemaphore;
    GPUFenceID* mPresenFences;
    GPUCommandPoolID* mCmdPools;
    GPUCommandBufferID* mCmds;
    GPUTextureID mDepthTex;
    GPUTextureViewID mDepthTexView;

    struct StorageBuffer
    {
        GPUBufferID buffer;
        uint32_t minAlignment;
        uint32_t maxRange;
        std::vector<uint32_t> _global_upload_ringbuffers_begin;
        std::vector<uint32_t> _global_upload_ringbuffers_end;
        std::vector<uint32_t> _global_upload_ringbuffers_size;
    };
    StorageBuffer mUploadStorageBuffer;

    uint32_t mCurrFrame {0};

    static constexpr uint32_t MeshPerDrawcallMaxInstanceCount = 64;
    struct RenderMeshInstance
    {
        FakeReal::math::Matrix4X4 model;
    };
    struct MeshPerdrawcallStorageBufferObject
    {
        RenderMeshInstance meshInstances[MeshPerDrawcallMaxInstanceCount];
    };

    GPUDescriptorSetID mDefaultMeshDescriptorSet;
    GPUDescriptorSetID mShadowMapSet;
    GPUBufferID mUBO;

public:
    const class SkyBox* mSkyBoxRef;
};