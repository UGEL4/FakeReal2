#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <string_view>
#include <vector>
#include <cmath>
#include <array>
#include "stb_image.h"
#include "Gpu/GpuApi.h"
#include <array>
#include "boundingbox.hpp"
#include "Math/Matrix.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"

using namespace FakeReal;

struct NewVertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float tan_x, tan_y, tan_z;
    float btan_x, btan_y, btan_z;

    bool operator ==(const NewVertex& other) const
    {
        return (x == other.x && y == other.y && z == other.z && nx == other.nx && ny == other.ny && nz == other.nz && u == other.u && v == other.v
        && tan_x == other.tan_x && tan_y == other.tan_y && tan_z == other.tan_z
        && btan_x == other.btan_x && btan_y == other.btan_y && btan_z == other.btan_z);
    }
};

struct MeshData
{
    std::vector<NewVertex> vertices;
    std::vector<uint32_t> indices;
};

struct SubMesh
{
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
    uint32_t materialIndex;
    std::string diffuse_tex_url;
};

struct Mesh
{
    MeshData meshData;
    std::vector<SubMesh> subMeshes;

    uint32_t GetMeshDataVerticesByteSize() const
    {
        return (uint32_t)(sizeof(NewVertex) * meshData.vertices.size());
    }

    uint32_t GetMeshDataIndicesByteSize() const
    {
        return (uint32_t)(sizeof(uint32_t) * meshData.indices.size());
    }

    uint32_t GetVertexCount() const
    {
        return (uint32_t)meshData.vertices.size();
    }

    uint32_t GetIndexCount() const
    {
        return (uint32_t)meshData.indices.size();
    }
};


enum PBRMaterialTextureType
{
    PBR_MTT_DIFFUSE = 0,
    PBR_MTT_NORMAL = 1,
    PBR_MTT_METALLIC = 2,
    PBR_MTT_ROUGHNESS = 3,
};

struct PBRMaterial
{
    struct Pack
    {
        GPUTextureID texture;
        GPUTextureViewID textureView;
        PBRMaterialTextureType textureType;
        uint32_t slotIndex;
        std::string name;
    };
    std::vector<Pack> textures;
    GPUSamplerID sampler;
    GPUDescriptorSetID set;
};

class TextureData
{
public:
    GPUTextureID mTexture {nullptr};
    GPUTextureViewID mTextureView {nullptr};

    TextureData()
    {

    };
    ~TextureData()
    {
        if (mTextureView) GPUFreeTextureView(mTextureView);
        mTextureView = nullptr;
        if (mTexture) GPUFreeTexture(mTexture);
        mTexture = nullptr;
    }

    bool IsValid() const
    {
        return mTextureView != nullptr && mTexture != nullptr;
    }

    void LoadTexture(const std::string_view& file, EGPUFormat format, GPUDeviceID device, GPUQueueID gfxQueue, bool flip = true)
    {
        // todo: opitional
        stbi_set_flip_vertically_on_load(flip);
        int width, height, comp;
        void* pixel = stbi_load(file.data(), &width, &height, &comp, STBI_rgb_alpha);
        if (!pixel)
        {
          return;
        }

        uint32_t bytes = width * height * 4; //R8G8BA8
        switch (format)
        {
            case GPU_FORMAT_R8G8B8A8_SRGB:
            case GPU_FORMAT_R8G8B8A8_UNORM:
                bytes = width * height * 4;
                break;
            default:
                assert(0 && "Unsupport texture format!");
                break;
        }

        GPUTextureDescriptor desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = uint32_t(width),
            .height      = uint32_t(height),
            .depth       = 1,
            .array_size  = 1,
            .format      = format,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_COPY_DEST,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE
        };
        mTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc = {
            .pTexture        = mTexture,
            .format          = (EGPUFormat)mTexture->format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1, // 
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1 // 
        };
        mTextureView = GPUCreateTextureView(device, &tex_view_desc);

        //upload
        GPUBufferDescriptor upload_buffer = {
            .size         = bytes,
            .descriptors  = GPU_RESOURCE_TYPE_NONE,
            .memory_usage = GPU_MEM_USAGE_CPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT
        };
        GPUBufferID uploadBuffer = GPUCreateBuffer(device, &upload_buffer);
        memcpy(uploadBuffer->cpu_mapped_address, pixel, bytes);
        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc = {
            .isSecondary = false
        };
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToTextureTransfer trans_texture_buffer_desc = {
                .dst             = mTexture,
                .dst_subresource = {
                    .mip_level        = 0,
                    .base_array_layer = 0,
                    .layer_count      = 1
                },
                .src        = uploadBuffer,
                .src_offset = 0
            };
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
            GPUTextureBarrier barrier = {
                .texture   = mTexture,
                .src_state = GPU_RESOURCE_STATE_COPY_DEST,
                .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE,
            };
            GPUResourceBarrierDescriptor rs_barrer{
                .texture_barriers       = &barrier,
                .texture_barriers_count = 1
            };
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &texture_cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);

        stbi_image_free(pixel);
    }
};

class EntityModel
{
public:
    EntityModel(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue);
    ~EntityModel();

private:
    void LoadModel(const std::string_view file);
    void LoadMaterial();

public:
    const std::vector<NewVertex>& GetVertexBufferData() const
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
    }

    void UploadResource(class SkyBox* skyBox);
    PBRMaterial* CreateMaterial(uint32_t materialIndex, const std::vector<std::pair<PBRMaterialTextureType, std::pair<std::string, bool>>>& textures);
    void Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos, const glm::mat4& lightSpaceMatrix);
    void UpdateShadowMapSet(GPUTextureViewID shadowMap, GPUSamplerID sampler);
    void Draw(GPURenderPassEncoderID encoder, const class Camera* cam, const glm::vec4& viewPos, const class CascadeShadowPass* shadowPass);

    Mesh mMesh;
    std::string mMeshFile;
    std::unordered_map<uint32_t, PBRMaterial*> mMaterials;
    std::unordered_map<std::string_view, TextureData> mTexturePool;
    GPUDeviceID mDevice;
    GPUQueueID mGfxQueue;
    GPURenderPipelineID mPbrPipeline;
    GPURootSignatureID mRootSignature;
    GPUDescriptorSetID mSet;
    GPUDescriptorSetID mShadowMapSet;
    GPUBufferID mVertexBuffer;
    GPUBufferID mIndexBuffer;
    GPUSamplerID mSampler;
    GPUBufferID mUBO;
    BoundingBox mBoundingBox;
    glm::mat4 mModelMatrix;
};

struct PBRMaterialParam
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
};