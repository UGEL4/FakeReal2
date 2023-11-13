#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "boundingbox.hpp"
#include "Gpu/GpuApi.h"

namespace global
{
    struct GlobalMeshRes
    {
        struct Vertex
        {
            float x, y, z;
            float nx, ny, nz;
            float u, v;
            float tan_x, tan_y, tan_z;
            float btan_x, btan_y, btan_z;

            bool operator==(const Vertex& other) const
            {
                return (x == other.x && y == other.y && z == other.z && nx == other.nx && ny == other.ny && nz == other.nz && u == other.u && v == other.v && tan_x == other.tan_x && tan_y == other.tan_y && tan_z == other.tan_z && btan_x == other.btan_x && btan_y == other.btan_y && btan_z == other.btan_z);
            }
        };

        std::vector<Vertex> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    };

    extern std::unordered_map<std::string, GlobalMeshRes> g_mesh_pool;
    extern std::unordered_map<std::string, BoundingBox> g_cache_mesh_bounding_box;
    bool HasMeshRes(const std::string& file);
    void LoadMeshRes(const std::string& file);


    ///////////////////////////////////////////////////
    struct GlobalTextureRes
    {
        uint32_t width;
        uint32_t heigh;
        uint32_t flip;
        uint32_t dataBytes;
        EGPUFormat format{EGPUFormat::GPU_FORMAT_R8G8B8A8_SRGB};
        void* data;
    };
    extern std::unordered_map<std::string, GlobalTextureRes> g_texture_pool;
    bool HasTextureRes(const std::string& file);
    void LoadTextureRes(const std::string& file, EGPUFormat format, bool flip);
    void FreeTextureRes(const std::string& file);
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    struct GlobalMaterialRes
    {
        std::string baseColorTextureFile;
        std::string normalTextureFile;
        std::string metallicTextureFile;
        std::string roughnessTextureFile;
        bool withTexture {false};
    };
    extern std::unordered_map<std::string, GlobalMaterialRes> g_material_pool;
    bool HasMaterialRes(const std::string& file);
    void LoadMaterialRes(const std::string& file);
    bool GetMaterialRes(const std::string& file, GlobalMaterialRes& outRes);
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    void FreeMeshResPool();
    void FreeTextureResPool();
    ///////////////////////////////////////////////////
    
    ///////////////////////////////////////////////////GPU Resource ///////////////////////////////////////////////////
    ///////////////////////////////////////////////////
    struct GlobalGPUTextureRes
    {
        uint32_t width;
        uint32_t heigh;
        EGPUFormat format;
        GPUTextureID texture {nullptr};
        GPUTextureViewID textureView {nullptr};
    };
    extern std::unordered_map<std::string, GlobalGPUTextureRes> g_gpu_texture_pool;
    bool HasGpuTextureRes(const std::string& name);
    void LoadGpuTextureRes(const std::string& name, GPUDeviceID device, GPUQueueID gfxQueue);
    void FreeGpuTexturePool();
    bool GetGpuTextureRes(const std::string& name, GlobalGPUTextureRes& out);
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    struct GlobalGPUMeshRes
    {
        GPUBufferID vertexBuffer;
        GPUBufferID indexBuffer;
        uint32_t vertexCount;
        uint32_t indexCount;
    };
    extern std::unordered_map<std::string, GlobalGPUMeshRes> g_gpu_mesh_pool;
    bool HasGpuMeshRes(const std::string& name);
    void LoadGpuMeshRes(const std::string& name, GPUDeviceID device, GPUQueueID gfxQueue);
    void FreeGpuMeshPool();
    bool GetGpuMeshRes(const std::string& name, GlobalGPUMeshRes*& out);
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    struct GlobalGPUMaterialRes
    {
        struct Pack
        {
            GPUTextureID texture;
            GPUTextureViewID textureView;
            uint32_t slotIndex;
            std::string name;
        };
        std::vector<Pack> textures;
        GPUSamplerID sampler;
        GPUDescriptorSetID set;
    };
    extern std::unordered_map<std::string, GlobalGPUMaterialRes> g_gpu_material_pool;
    bool HasGpuMaterialRes(const std::string& name);
    void LoadGpuMaterialRes(const std::string& name, GPUDeviceID device, GPUQueueID gfxQueue, GPURootSignatureID rs, GPUSamplerID sampler);
    void FreeGpuMaterialPool();
    bool GetGpuMaterialRes(const std::string& name, GlobalGPUMaterialRes*& out);
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    static constexpr uint32_t MeshPerDrawcallMaxInstanceCount = 64;
    struct RenderMeshInstance
    {
        FakeReal::math::Matrix4X4 model;
    };
    struct MeshPerdrawcallStorageBufferObject
    {
        RenderMeshInstance meshInstances[MeshPerDrawcallMaxInstanceCount];
    };

    struct MeshDirectionalLightShadowPerdrawcallStorageBufferObject
    {
        RenderMeshInstance meshInstances[MeshPerDrawcallMaxInstanceCount];
    };

    struct StorageBuffer
    {
        GPUBufferID buffer{nullptr};
        uint32_t minAlignment;
        uint32_t maxRange;
        std::vector<uint32_t> _global_upload_ringbuffers_begin;
        std::vector<uint32_t> _global_upload_ringbuffers_end;
        std::vector<uint32_t> _global_upload_ringbuffers_size;
    };

    struct GlobalRenderResource
    {
        StorageBuffer storage;
        void Initialize(GPUDeviceID device, GPUQueueID queue);
        void Free();
        void Reset(uint32_t frame_index);
    };

    extern GlobalRenderResource g_global_reader_resource;
    ///////////////////////////////////////////////////
} // namespace global