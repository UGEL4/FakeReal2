#pragma once
#include <string_view>
#include <vector>
#include <cmath>
#include <array>
#include <glm/glm.hpp>
#include "stb_image.h"
#include "Gpu/GpuApi.h"
#include <array>

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;

    bool operator ==(const Vertex& other) const
    {
        return (x == other.x && y == other.y && z == other.z && nx == other.nx && ny == other.ny && nz == other.nz && u == other.u && v == other.v);
    }
};

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
    //std::unordered_map<std::string, std::vector<std::string>> textures;
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

class Model
{
public:
    Model(const std::string_view file);
    ~Model();

private:
    void LoadModel(const std::string_view file);

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
//private:
    //std::vector<Mesh> mMeshes;
    Mesh mMesh;
};

struct Sphere
{
    // j [0, pi],  i [0, 2pi]
    // x = r * sin(j) * cos(i); y = r * cos(j); z = r * sin(j) * sin(i)

    static std::array<float, 3> Normalize(float x, float y, float z)
    {
        float length = sqrt(x * x + y * y + z * z);
        float div = 1.f / length;
        x *= div;
        y *= div;
        z *= div;
        return {x, y, z};
    }
    static constexpr const uint32_t SPHERE_DIV = 64;
    static constexpr const float PI            = 3.14159265359f;
    static std::vector<Vertex> GenSphereVertices()
    {
        std::vector<Vertex> vertices;
        // Generate coordinates
        for (uint32_t j = 0; j <= SPHERE_DIV; j++)
        {
            float aj = j * PI / SPHERE_DIV;
            float sj = std::sin(aj);
            float cj = std::cos(aj);
            for (uint32_t i = 0; i <= SPHERE_DIV; i++)
            {
                float ai = i * 2 * PI / SPHERE_DIV;
                float si = std::sin(ai);
                float ci = std::cos(ai);

                Vertex v;
                v.x = si * sj; // X
                v.y = cj;      // Y
                v.z = ci * sj; // Z
                v.nx = v.x;
                v.ny = v.y;
                v.nz = v.z;
                std::array<float, 3> n = Normalize(v.x, v.y, v.z);
                v.u = std::atan2(n[0], n[2]) / (2 * PI) + 0.5f;
                v.v = n[1] * 0.5f + 0.5f;
                vertices.emplace_back(v);
            }
        }
        return vertices;
    }

    static std::vector<uint32_t> GenSphereIndices()
    {
        std::vector<uint32_t> indices;
        for (uint32_t j = 0; j < SPHERE_DIV; j++)
        {
          for (uint32_t i = 0; i < SPHERE_DIV; i++)
          {
            uint32_t p1 = j * (SPHERE_DIV + 1) + i;
            uint32_t p2 = p1 + (SPHERE_DIV + 1);
            /*
              p1     p1+1
                |``/|
                | / |
                |/__|
              p2     p2+1
            */
            indices.push_back(p1);
            indices.push_back(p2);
            indices.push_back(p1 + 1);

            indices.push_back(p1 + 1);
            indices.push_back(p2);
            indices.push_back(p2 + 1);
          }
        }
        return indices;
    }

    static std::vector<Vertex> GenCubeVertices()
    {
        std::vector<Vertex> vertices = {
            { -0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 },
            { 0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0 },
            { 0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 1.0 },
            { -0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 1.0 },
            { 0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0 },
            { -0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 0.0 },
            { -0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 1.0 },
            { 0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 1.0 },
            { -0.5, -0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 0.0 },
            { -0.5, 0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 1.0 },
            { -0.5, 0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 1.0 },
            { -0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 0.0 },
            { 0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 1.0 },
            { 0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0 },
            { -0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0 },
            { 0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 1.0, 1.0 },
            { -0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 0.0, 1.0 },
            { -0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 0.0 },
            { 0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0 },
            { 0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 1.0 },
            { -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0 }
        };

        return vertices;
    }

    static std::vector<uint32_t> GenCubeIndices()
    {
        std::vector<uint32_t> indeces = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };
        return indeces;
    }

    static std::vector<Vertex> GenCubeIdentityVertices()
    {
        std::vector<Vertex> vertices = {
            { 1.0,-1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 },
            { 1.0,-1.0,-1.0, 1.0, 0.0, 0.0, 1.0, 0.0 },
            { 1.0, 1.0,-1.0, 1.0, 0.0, 0.0, 1.0, 1.0 },
            { 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0 },

            {-1.0,-1.0, 1.0,-1.0, 0.0, 0.0, 1.0, 0.0 },
            {-1.0, 1.0, 1.0,-1.0, 0.0, 0.0, 1.0, 1.0 },
            {-1.0, 1.0,-1.0,-1.0, 0.0, 0.0, 0.0, 1.0 },
            {-1.0,-1.0,-1.0,-1.0, 0.0, 0.0, 0.0, 0.0 },

            {-1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0 },
            { 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0 },
            { 1.0, 1.0,-1.0, 0.0, 1.0, 0.0, 1.0, 1.0 },
            {-1.0, 1.0,-1.0, 0.0, 1.0, 0.0, 0.0, 1.0 },

            {-1.0,-1.0,-1.0, 0.0,-1.0, 0.0, 0.0, 0.0 },
            { 1.0,-1.0,-1.0, 0.0,-1.0, 0.0, 1.0, 0.0 },
            { 1.0,-1.0, 1.0, 0.0,-1.0, 0.0, 1.0, 1.0 },
            {-1.0,-1.0, 1.0, 0.0,-1.0, 0.0, 0.0, 1.0 },

            {-1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
            {1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0 },
            { 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 },
            {-1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0 },

            { 1.0,-1.0,-1.0, 0.0, 0.0,-1.0, 0.0, 0.0 },
            {-1.0,-1.0,-1.0, 0.0, 0.0,-1.0, 1.0, 0.0 },
            {-1.0, 1.0,-1.0, 0.0, 0.0,-1.0, 1.0, 1.0 },
            { 1.0, 1.0,-1.0, 0.0, 0.0,-1.0, 0.0, 1.0 }
        };

        return vertices;
    }

    static std::vector<uint32_t> GenCubeIdentityIndices()
    {
        std::vector<uint32_t> indeces = {
            0, 3, 2, 2, 1, 0,
            4, 7, 6, 6, 5, 4,
            8, 11, 10, 10, 9, 8,
            12, 15, 14, 14, 13, 12,
            16, 19, 18, 18, 17, 16,
            20, 23, 22, 22, 21, 20
        };
        return indeces;
    }
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

struct GeomVSUniformBuffer
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class TextureData
{
public:
    uint32_t mWidth{ 0 };
    uint32_t mHeight{ 0 };
    uint32_t mDepth{ 0 };
    uint32_t mPixelBytes{ 0 };
    void* m_pPixels{ nullptr };
    GPUTextureID mTexture {nullptr};
    GPUTextureViewID mTextureView {nullptr};
    GPUDescriptorSetID mSet{nullptr};

    TextureData() = default;
    ~TextureData()
    {
        if (m_pPixels)
        {
          free(m_pPixels);
          m_pPixels = nullptr;
        }

        if (mTextureView) GPUFreeTextureView(mTextureView);
        mTextureView = nullptr;
        if (mTexture) GPUFreeTexture(mTexture);
        mTexture = nullptr;

        if (mSet) GPUFreeDescriptorSet(mSet);
        mSet = nullptr;
    }

    bool IsValid() const
    {
        return m_pPixels != nullptr;
    }

    void SetDescriptorSet(GPURootSignatureID rs)
    {
        GPUDescriptorSetDescriptor model_material_set_desc{};
        model_material_set_desc.root_signature = rs;
        model_material_set_desc.set_index      = 1;
        mSet                                   = GPUCreateDescriptorSet(rs->device, &model_material_set_desc);
    }

    void LoadTexture(const std::string& file, GPUDeviceID device, GPUQueueID gfxQueue)
    {
        // todo: opitional
        stbi_set_flip_vertically_on_load(true);
        int width, height, comp;
        m_pPixels = stbi_load(file.c_str(), &width, &height, &comp, STBI_rgb_alpha);
        if (!m_pPixels)
        {
          return;
        }
        mWidth      = width;
        mHeight     = height;
        mDepth      = 1;
        mPixelBytes = width * height * 4; //R8G8BA8

        GPUTextureDescriptor desc{};
        desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
        desc.width       = width;
        desc.height      = height;
        desc.depth       = 1;
        desc.array_size  = 1;
        desc.format      = GPU_FORMAT_R8G8B8A8_UNORM;
        desc.owner_queue = gfxQueue;
        desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE;
        mTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc{};
        tex_view_desc.pTexture        = mTexture;
        tex_view_desc.format          = (EGPUFormat)mTexture->format;
        tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel    = 0;
        tex_view_desc.mipLevelCount   = 1;
        tex_view_desc.baseArrayLayer  = 0;
        tex_view_desc.arrayLayerCount = 1;
        mTextureView                  = GPUCreateTextureView(device, &tex_view_desc);

        //upload
        GPUBufferDescriptor upload_buffer{};
        upload_buffer.size         = mPixelBytes;
        upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
        upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
        upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
        GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
        memcpy(uploadBuffer->cpu_mapped_address, m_pPixels, mPixelBytes);
        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary = false;
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToTextureTransfer trans_texture_buffer_desc{};
            trans_texture_buffer_desc.dst                              = mTexture;
            trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
            trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
            trans_texture_buffer_desc.dst_subresource.layer_count      = 1;
            trans_texture_buffer_desc.src                              = uploadBuffer;
            trans_texture_buffer_desc.src_offset                       = 0;
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
            GPUTextureBarrier barrier{};
            barrier.texture = mTexture;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
            GPUResourceBarrierDescriptor rs_barrer{};
            rs_barrer.texture_barriers      = &barrier;
            rs_barrer.texture_barriers_count = 1;
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &texture_cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
    }
};

class HDRIBLCubeMapTextureData
{
public:
    GPUTextureID mTexture {nullptr};
    GPUTextureViewID mTextureView {nullptr};
    GPUSamplerID mSampler{ nullptr };

    HDRIBLCubeMapTextureData() = default;
    virtual ~HDRIBLCubeMapTextureData()
    {
        if (mTextureView) GPUFreeTextureView(mTextureView);
        if (mTexture) GPUFreeTexture(mTexture);
        if (mSampler) GPUFreeSampler(mSampler);
    }

    virtual void Load(std::array<std::string, 6>& files, GPUDeviceID device, GPUQueueID gfxQueue, EGPUFormat format, bool flip, uint32_t desiredChannels = 4)
    {
        stbi_set_flip_vertically_on_load(flip);
        int width, height, comp;
        std::array<void*, 6> pixels = {nullptr};
        for (uint32_t i = 0; i < 6; i++)
        {
            pixels[i] = stbi_loadf(files[i].c_str(), &width, &height, &comp, desiredChannels);
            if (!pixels[i])
            {
                assert(0 && ("Load image failed : " + files[i]).c_str());
                return;
            }
        }

        uint32_t bytes = 0;
        switch (format)
        {
            case GPU_FORMAT_R32G32B32_SFLOAT:
            {
                bytes = width * height * 4 * 3;
            }
            break;
            case GPU_FORMAT_R32G32B32A32_SFLOAT:
                bytes = width * height * 4 * 4;
                break;
            default:
                assert(0);
                break;
        }

        uint32_t totalBytes = bytes * 6;

        GPUTextureDescriptor desc{};
        desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
        desc.width       = width;
        desc.height      = height;
        desc.depth       = 1;
        desc.array_size  = 6;
        desc.format      = GPU_FORMAT_R32G32B32A32_SFLOAT;
        desc.owner_queue = gfxQueue;
        desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
        mTexture         = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc{};
        tex_view_desc.pTexture        = mTexture;
        tex_view_desc.format          = (EGPUFormat)mTexture->format;
        tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel    = 0;
        tex_view_desc.mipLevelCount   = 1;
        tex_view_desc.baseArrayLayer  = 0;
        tex_view_desc.arrayLayerCount = 6;
        mTextureView                  = GPUCreateTextureView(device, &tex_view_desc);

        GPUBufferDescriptor upload_buffer{};
        upload_buffer.size         = totalBytes;
        upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
        upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
        upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
        GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
        for (uint32_t i = 0; i < 6; i++)
        {
            //uint8_t* ptr = (uint8_t*)(uploadBuffer->cpu_mapped_address) + bytes * i;
            //memcpy(ptr, pixels[i], bytes);
            memcpy(((uint8_t*)(uploadBuffer->cpu_mapped_address) + bytes * i), pixels[i], bytes);
        }
        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary    = false;
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToTextureTransfer trans_texture_buffer_desc{};
            trans_texture_buffer_desc.dst                              = mTexture;
            trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
            trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
            trans_texture_buffer_desc.dst_subresource.layer_count      = 6;
            trans_texture_buffer_desc.src                              = uploadBuffer;
            trans_texture_buffer_desc.src_offset                       = 0;
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
            GPUTextureBarrier barrier{};
            barrier.texture = mTexture;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
            GPUResourceBarrierDescriptor rs_barrer{};
            rs_barrer.texture_barriers      = &barrier;
            rs_barrer.texture_barriers_count = 1;
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &texture_cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);

        for (uint32_t i = 0; i < 6; i++)
        {
            if (pixels[i] != nullptr)
            {
                stbi_image_free(pixels[i]);
            }
        }
    }

    void GenIBLImageFromHDR(const std::string& file, GPUDeviceID device, GPUQueueID gfxQueue, EGPUFormat format, bool flip, uint32_t desiredChannels = 4)
    {
        stbi_set_flip_vertically_on_load(flip);
        int width, height, comp;
        void* pixel = stbi_loadf(file.c_str(), &width, &height, &comp, desiredChannels);
        if (!pixel)
        {
            assert(0 && "HDR texture load field!");
            return;
        }

        uint32_t bytes = 0;
        switch (format)
        {
            case GPU_FORMAT_R32G32B32_SFLOAT:
            {
                bytes = width * height * 4 * 3;
            }
            break;
            case GPU_FORMAT_R32G32B32A32_SFLOAT:
                bytes = width * height * 4 * 4;
                break;
            default:
                assert(0);
                break;
        }

        GPUTextureDescriptor desc{};
        desc.flags               = GPU_TCF_OWN_MEMORY_BIT;
        desc.width               = width;
        desc.height              = height;
        desc.depth               = 1;
        desc.array_size          = 1;
        desc.format              = GPU_FORMAT_R32G32B32A32_SFLOAT;
        desc.owner_queue         = gfxQueue;
        desc.start_state         = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors         = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
        GPUTextureID tempTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc{};
        tex_view_desc.pTexture           = mTexture;
        tex_view_desc.format             = (EGPUFormat)mTexture->format;
        tex_view_desc.usages             = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask         = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel       = 0;
        tex_view_desc.mipLevelCount      = 1;
        tex_view_desc.baseArrayLayer     = 0;
        tex_view_desc.arrayLayerCount    = 1;
        GPUTextureViewID tempTextureView = GPUCreateTextureView(device, &tex_view_desc);

        GPUBufferDescriptor upload_buffer{};
        upload_buffer.size         = bytes;
        upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
        upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
        upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
        GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
        memcpy(uploadBuffer->cpu_mapped_address, pixel, bytes);
        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary    = false;
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToTextureTransfer trans_texture_buffer_desc{};
            trans_texture_buffer_desc.dst                              = tempTexture;
            trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
            trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
            trans_texture_buffer_desc.dst_subresource.layer_count      = 1;
            trans_texture_buffer_desc.src                              = uploadBuffer;
            trans_texture_buffer_desc.src_offset                       = 0;
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
            GPUTextureBarrier barrier{};
            barrier.texture   = tempTexture;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
            GPUResourceBarrierDescriptor rs_barrer{};
            rs_barrer.texture_barriers       = &barrier;
            rs_barrer.texture_barriers_count = 1;
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &texture_cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        stbi_image_free(pixel);

        uint32_t totalBytes = bytes * 6;
        desc = {};
        desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
        desc.width       = width;
        desc.height      = height;
        desc.depth       = 1;
        desc.array_size  = 6;
        desc.format      = GPU_FORMAT_R32G32B32A32_SFLOAT;
        desc.owner_queue = gfxQueue;
        desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
        mTexture         = GPUCreateTexture(device, &desc);
        tex_view_desc = {};
        tex_view_desc.pTexture        = mTexture;
        tex_view_desc.format          = (EGPUFormat)mTexture->format;
        tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel    = 0;
        tex_view_desc.mipLevelCount   = 1;
        tex_view_desc.baseArrayLayer  = 0;
        tex_view_desc.arrayLayerCount = 6;
        mTextureView                  = GPUCreateTextureView(device, &tex_view_desc);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
    }

    void SetSampler(GPUSamplerID sampler)
    {
        mSampler = sampler;
    }

    void CreateSampler(GPUDeviceID device, const GPUSamplerDescriptor& desc)
    {
        mSampler = GPUCreateSampler(device, &desc);
    }
};