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

using namespace FakeReal;

class EntityModel
{
public:
    EntityModel(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue);
    ~EntityModel();

    void UploadRenderResource(class SkyBox* skyBox);

private:
    //void LoadModel(const std::string_view file);
    //void LoadMaterial();

public:
    /* const std::vector<NewVertex>& GetVertexBufferData() const
    {
        return mMesh.meshData.vertices;
    }

    const std::vector<uint32_t>& GetIndexBufferData() const
    {
        return mMesh.meshData.indices;
    }

    uint32_t GetMeshDataVerticesByteSize() const
    {
        return mMesh.GetMeshDataVerticesByteSize();
    }

    uint32_t GetMeshDataIndicesByteSize() const
    {
        return mMesh.GetMeshDataIndicesByteSize();
    }

    uint32_t GetVertexCount() const
    {
        return mMesh.GetVertexCount();
    }

    uint32_t GetIndexCount() const
    {
        return mMesh.GetIndexCount();
    } */

    /* void UploadResource(class SkyBox* skyBox);
    PBRMaterial* CreateMaterial(uint32_t materialIndex, const std::vector<std::pair<PBRMaterialTextureType, std::pair<std::string, bool>>>& textures);
    void Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos, const glm::mat4& lightSpaceMatrix);*/
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
};

/* struct PBRMaterialParam
{
    float metallic;
    float roughness;
    float ao;
    float padding;
};

constexpr const int MAX_LIGHT_NUM = 4;
struct LightParam
{
    glm::vec4 lightColor[MAX_LIGHT_NUM];
    glm::vec4 lightPos[MAX_LIGHT_NUM];
};

struct PushConstant
{
    glm::vec4 objOffsetPos;
    float metallic;
    float roughness;
    float ao;
    float padding;
};

struct UniformBuffer
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 viewPos;
};

struct CommonUniformBuffer
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 lightSpaceMat;;
    glm::vec4 viewPos;
};

struct GeomVSUniformBuffer
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct DirectionalLight
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
}; */