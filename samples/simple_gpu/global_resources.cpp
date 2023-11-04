#include "global_resources.hpp"
#include <fstream>
#include <sstream>
#include "Utils/Json/JsonReader.h"
#include "stb_image.h"

namespace global
{
    std::unordered_map<std::string, GlobalMeshRes> g_mesh_pool;
    std::unordered_map<std::string, BoundingBox> g_cache_mesh_bounding_box;

    bool HasMeshRes(const std::string& file)
    {
        auto iter = g_mesh_pool.find(file);
        return iter != g_mesh_pool.end();
    }

    void LoadMeshRes(const std::string& file)
    {
        if (HasMeshRes(file)) return;
        std::ifstream is(file.data());
        if (!is.is_open())
        {
            return;
        }
        std::stringstream ss;
        ss << is.rdbuf();
        std::string json_str(ss.str());
        is.close();

        GlobalMeshRes tmp;
        auto result = g_mesh_pool.insert(std::make_pair(file, std::move(tmp)));
        BoundingBox aabbTmp;
        auto aabbResult = g_cache_mesh_bounding_box.insert(std::make_pair(file, std::move(aabbTmp)));
        {
            GlobalMeshRes& meshRes = result.first->second;
            BoundingBox& aabb      = aabbResult.first->second;
            uint32_t materialIndex = 0;
            FakeReal::JsonReader reader(json_str.c_str());
            reader.StartObject();
            {
                reader.Key("mMaterialIndex");
                reader.Value(materialIndex);

                reader.Key("mVertices");
                size_t count = 0;
                reader.StartArray(&count);
                meshRes.vertexBuffer.reserve(count);
                for (size_t i = 0; i < count; i++)
                {
                    GlobalMeshRes::Vertex v{};
                    reader.StartObject();
                    {
                        reader.Key("x");
                        reader.Value(v.x);
                        reader.Key("y");
                        reader.Value(v.y);
                        reader.Key("z");
                        reader.Value(v.z);
                        reader.Key("nx");
                        reader.Value(v.nx);
                        reader.Key("ny");
                        reader.Value(v.ny);
                        reader.Key("nz");
                        reader.Value(v.nz);
                        reader.Key("tx");
                        reader.Value(v.u);
                        reader.Key("ty");
                        reader.Value(v.v);
                        reader.Key("tanx");
                        reader.Value(v.tan_x);
                        reader.Key("tany");
                        reader.Value(v.tan_y);
                        reader.Key("tanz");
                        reader.Value(v.tan_z);
                        reader.Key("btanx");
                        reader.Value(v.btan_x);
                        reader.Key("btany");
                        reader.Value(v.btan_y);
                        reader.Key("btanz");
                        reader.Value(v.btan_z);
                    }
                    reader.EndObject();
                    meshRes.vertexBuffer.emplace_back(v);
                    aabb.Update(glm::vec3(v.x, v.y, v.z));
                }
                reader.EndArray();

                reader.Key("mIndices");
                count = 0;
                reader.StartArray(&count);
                meshRes.indexBuffer.reserve(count);
                for (size_t i = 0; i < count; i++)
                {
                    uint32_t val = 0;
                    reader.Value(val);
                    meshRes.indexBuffer.push_back(val);
                }
                reader.EndArray();
            }
            reader.EndObject();
        }
    }

    //////////////////////////////////////
    std::unordered_map<std::string, GlobalTextureRes> g_texture_pool;
    bool HasTextureRes(const std::string& file)
    {
        auto iter = g_texture_pool.find(file);
        return iter != g_texture_pool.end();
    }
    void LoadTextureRes(const std::string& file, EGPUFormat format, bool flip)
    {
        if (HasTextureRes(file)) return;
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
        GlobalTextureRes tmp;
        auto result = g_texture_pool.insert(std::make_pair(file, std::move(tmp)));
        assert(result.second);
        GlobalTextureRes& newTex = result.first->second;
        newTex.width             = width;
        newTex.heigh             = height;
        newTex.format            = format;
        newTex.flip              = flip;
        newTex.data              = pixel;
        newTex.dataBytes         = bytes;
    }

    void FreeTextureRes(const std::string& file)
    {
        auto iter = g_texture_pool.find(file);
        if (iter != g_texture_pool.end())
        {
            if (iter->second.data) stbi_image_free(iter->second.data);
            g_texture_pool.erase(iter);
        }
    }
    //////////////////////////////////////

    //////////////////////////////////////
    std::unordered_map<std::string, GlobalMaterialRes> g_material_pool;
    bool HasMaterialRes(const std::string& file)
    {
        auto iter = g_material_pool.find(file);
        return iter != g_material_pool.end();
    }
    void LoadMaterialRes(const std::string& file)
    {
        if (HasMaterialRes(file)) return;
        std::ifstream is(file.data());
        if (!is.is_open())
        {
            return;
        }
        std::stringstream ss;
        ss << is.rdbuf();
        std::string json_str(ss.str());
        is.close();

        GlobalMaterialRes tmp;
        auto result = g_material_pool.insert(std::make_pair(file, std::move(tmp)));
        GlobalMaterialRes& materialRes = result.first->second;
        {
            FakeReal::JsonReader reader(json_str.c_str());
            reader.StartObject();
            {
                reader.Key("textures");
                size_t count = 0;
                reader.StartArray(&count);
                for (size_t i = 0; i < count; i++)
                {
                    reader.StartObject();
                    {
                        std::string type;
                        std::string url;
                        reader.Key("type");
                        reader.Value(type);
                        reader.Key("name");
                        reader.Value(url);
                        if (type == "diffuse")
                        {
                            materialRes.baseColorTextureFile = url;
                            materialRes.withTexture          = true;
                        }
                        else if (type == "normal")
                        {
                            materialRes.normalTextureFile = url;
                            materialRes.withTexture       = true;
                        }
                        else if (type == "metallic")
                        {
                            materialRes.metallicTextureFile = url;
                            materialRes.withTexture         = true;
                        }
                        else if (type == "roughness")
                        {
                            materialRes.roughnessTextureFile = url;
                            materialRes.withTexture          = true;
                        }
                    }
                    reader.EndObject();
                }
                reader.EndArray();
            }
            reader.EndObject();

            //load texture
            if (materialRes.withTexture)
            {
                if (materialRes.baseColorTextureFile != "") LoadTextureRes(materialRes.baseColorTextureFile, EGPUFormat::GPU_FORMAT_R8G8B8A8_SRGB, true);
                if (materialRes.normalTextureFile != "") LoadTextureRes(materialRes.normalTextureFile, EGPUFormat::GPU_FORMAT_R8G8B8A8_UNORM, true);
                if (materialRes.metallicTextureFile != "") LoadTextureRes(materialRes.metallicTextureFile, EGPUFormat::GPU_FORMAT_R8G8B8A8_UNORM, true);
                if (materialRes.roughnessTextureFile != "") LoadTextureRes(materialRes.roughnessTextureFile, EGPUFormat::GPU_FORMAT_R8G8B8A8_UNORM, true);
            }
        }
    }

    bool GetMaterialRes(const std::string& file, GlobalMaterialRes& outRes)
    {
        auto iter = g_material_pool.find(file);
        if (iter == g_material_pool.end()) return false;
        outRes = iter->second;
        return true;
    }
    //////////////////////////////////////

    ///////////////////////////////////////////////////
    void FreeMeshResPool()
    {
        g_mesh_pool.clear();
        g_cache_mesh_bounding_box.clear();
    }
    void FreeTextureResPool()
    {
        for (auto iter : g_texture_pool)
        {
            if (iter.second.data) stbi_image_free(iter.second.data);
        }
        g_texture_pool.clear();
    }
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////GPU Resource ///////////////////////////////////////////////////
    std::unordered_map<std::string, GlobalGPUTextureRes> g_gpu_texture_pool;
    bool HasGpuTextureRes(const std::string& name)
    {
        auto iter = g_gpu_texture_pool.find(name);
        return iter != g_gpu_texture_pool.end();
    }
    void LoadGpuTextureRes(const std::string& name, GPUDeviceID device, GPUQueueID gfxQueue)
    {
        if (HasGpuTextureRes(name)) return;
        if (!HasTextureRes(name)) return; // do nothing
        GlobalTextureRes& texRes = g_texture_pool[name];
        if (!texRes.data) return;
        GlobalGPUTextureRes tmp;
        auto result = g_gpu_texture_pool.insert(std::make_pair(name, std::move(tmp)));
        assert(result.second);
        GlobalGPUTextureRes& newTex = result.first->second;
        {
            GPUTextureDescriptor desc = {
                .flags       = GPU_TCF_OWN_MEMORY_BIT,
                .width       = texRes.width,
                .height      = texRes.heigh,
                .depth       = 1,
                .array_size  = 1,
                .format      = texRes.format,
                .owner_queue = gfxQueue,
                .start_state = GPU_RESOURCE_STATE_COPY_DEST,
                .descriptors = GPU_RESOURCE_TYPE_TEXTURE
            };
            newTex.texture = GPUCreateTexture(device, &desc);

            GPUTextureViewDescriptor tex_view_desc = {
                .pTexture        = newTex.texture,
                .format          = texRes.format,
                .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV,
                .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
                .baseMipLevel    = 0,
                .mipLevelCount   = 1, //
                .baseArrayLayer  = 0,
                .arrayLayerCount = 1 //
            };
            newTex.textureView = GPUCreateTextureView(device, &tex_view_desc);

            // upload
            GPUBufferDescriptor upload_buffer = {
                .size         = texRes.dataBytes,
                .descriptors  = GPU_RESOURCE_TYPE_NONE,
                .memory_usage = GPU_MEM_USAGE_CPU_ONLY,
                .flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT
            };
            GPUBufferID uploadBuffer = GPUCreateBuffer(device, &upload_buffer);
            memcpy(uploadBuffer->cpu_mapped_address, texRes.data, texRes.dataBytes);
            GPUCommandPoolID pool              = GPUCreateCommandPool(gfxQueue);
            GPUCommandBufferDescriptor cmdDesc = {
                .isSecondary = false
            };
            GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
            GPUResetCommandPool(pool);
            GPUCmdBegin(cmd);
            {
                GPUBufferToTextureTransfer trans_texture_buffer_desc = {
                    .dst             = newTex.texture,
                    .dst_subresource = {
                    .mip_level        = 0,
                    .base_array_layer = 0,
                    .layer_count      = 1 },
                    .src        = uploadBuffer,
                    .src_offset = 0
                };
                GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
                GPUTextureBarrier barrier = {
                    .texture   = newTex.texture,
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
        }
    }
    void FreeGpuTexturePool()
    {
        for (auto iter : g_gpu_texture_pool)
        {
            if (iter.second.textureView) GPUFreeTextureView(iter.second.textureView);
            if (iter.second.texture) GPUFreeTexture(iter.second.texture);
        }
        g_gpu_texture_pool.clear();
    }

    bool GetGpuTextureRes(const std::string& name, GlobalGPUTextureRes& out)
    {
        auto iter = g_gpu_texture_pool.find(name);
        if (iter == g_gpu_texture_pool.end()) return false;
        out = iter->second;
        return true;
    }
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    std::unordered_map<std::string, GlobalGPUMeshRes> g_gpu_mesh_pool;
    bool HasGpuMeshRes(const std::string& name)
    {
        auto iter = g_gpu_mesh_pool.find(name);
        return iter != g_gpu_mesh_pool.end();
    }
    void LoadGpuMeshRes(const std::string& name, GPUDeviceID device, GPUQueueID gfxQueue)
    {
        if (HasGpuMeshRes(name)) return;
        if (!HasMeshRes(name)) return;
        GlobalMeshRes& meshRes = g_mesh_pool[name];

        GlobalGPUMeshRes tmp;
        auto result = g_gpu_mesh_pool.insert(std::make_pair(name, std::move(tmp)));
        assert(result.second);
        GlobalGPUMeshRes& newMesh = result.first->second;

        uint32_t vb_size           = meshRes.vertexBuffer.size() * sizeof(GlobalMeshRes::Vertex);
        GPUBufferDescriptor v_desc = {
            .size         = vb_size,
            .descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER,
            .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT
        };
        newMesh.vertexBuffer = GPUCreateBuffer(device, &v_desc);

        uint32_t ib_size           = meshRes.indexBuffer.size() * sizeof(uint32_t);
        GPUBufferDescriptor i_desc = {
            .size         = ib_size,
            .descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER,
            .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT
        };
        newMesh.indexBuffer = GPUCreateBuffer(device, &i_desc);

        uint32_t uploadBufferSize         = vb_size + ib_size;
        GPUBufferDescriptor upload_buffer = {
            .size         = uploadBufferSize,
            .descriptors  = GPU_RESOURCE_TYPE_NONE,
            .memory_usage = GPU_MEM_USAGE_CPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT,
        };
        GPUBufferID uploadBuffer = GPUCreateBuffer(device, &upload_buffer);

        GPUCommandPoolID pool               = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmd_desc = {
            .isSecondary = false
        };
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmd_desc);

        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            memcpy(uploadBuffer->cpu_mapped_address, meshRes.vertexBuffer.data(), vb_size);
            GPUBufferToBufferTransfer trans_desc = {
                .dst        = newMesh.vertexBuffer,
                .dst_offset = 0,
                .src        = uploadBuffer,
                .src_offset = 0,
                .size       = vb_size
            };
            GPUCmdTransferBufferToBuffer(cmd, &trans_desc);

            memcpy((uint8_t*)uploadBuffer->cpu_mapped_address + v_desc.size, meshRes.indexBuffer.data(), ib_size);
            trans_desc = {
                .dst        = newMesh.indexBuffer,
                .dst_offset = 0,
                .src        = uploadBuffer,
                .src_offset = vb_size, // vertexbuffer
                .size       = ib_size
            };
            GPUCmdTransferBufferToBuffer(cmd, &trans_desc);

            GPUBufferBarrier barriers[2]           = {};
            barriers[0].buffer                     = newMesh.vertexBuffer;
            barriers[0].src_state                  = GPU_RESOURCE_STATE_COPY_DEST;
            barriers[0].dst_state                  = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            barriers[1].buffer                     = newMesh.indexBuffer;
            barriers[1].src_state                  = GPU_RESOURCE_STATE_COPY_DEST;
            barriers[1].dst_state                  = GPU_RESOURCE_STATE_INDEX_BUFFER;
            GPUResourceBarrierDescriptor rs_barrer = {
                .buffer_barriers       = barriers,
                .buffer_barriers_count = 2
            };
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
    }
    void FreeGpuMeshPool()
    {
        for (auto iter : g_gpu_mesh_pool)
        {
            if (iter.second.vertexBuffer) GPUFreeBuffer(iter.second.vertexBuffer);
            if (iter.second.indexBuffer) GPUFreeBuffer(iter.second.indexBuffer);
        }
        g_gpu_mesh_pool.clear();
    }

    bool GetGpuMeshRes(const std::string& name, GlobalGPUMeshRes& out)
    {
        auto iter = g_gpu_mesh_pool.find(name);
        if (iter == g_gpu_mesh_pool.end()) return false;
        out = iter->second;
        return true;
    }
    ///////////////////////////////////////////////////

    ///////////////////////////////////////////////////
    std::unordered_map<std::string, GlobalGPUMaterialRes> g_gpu_material_pool;
    bool HasGpuMaterialRes(const std::string& name)
    {
        auto iter = g_gpu_material_pool.find(name);
        return iter != g_gpu_material_pool.end();
    }

    void LoadGpuMaterialRes(const std::string& name, GPUDeviceID device, GPUQueueID gfxQueue, GPURootSignatureID rs, GPUSamplerID sampler)
    {
        if (HasGpuMaterialRes(name)) return;
        if (!HasMaterialRes(name)) return;
        GlobalMaterialRes& materialRes = g_material_pool[name];

        GlobalGPUMaterialRes tmp;
        auto result = g_gpu_material_pool.insert(std::make_pair(name, std::move(tmp)));
        assert(result.second);
        GlobalGPUMaterialRes& newMaterial = result.first->second;

        if (materialRes.withTexture)
        {
            if (materialRes.baseColorTextureFile != "")
            {
                GlobalGPUTextureRes baseColorTexRes;
                LoadGpuTextureRes(materialRes.baseColorTextureFile, device, gfxQueue);
                GetGpuTextureRes(materialRes.baseColorTextureFile, baseColorTexRes);
                GlobalGPUMaterialRes::Pack& pack = newMaterial.textures.emplace_back();
                pack.texture     = baseColorTexRes.texture;
                pack.textureView = baseColorTexRes.textureView;
                pack.name        = materialRes.baseColorTextureFile;
                pack.slotIndex   = 0;
            }
            if (materialRes.normalTextureFile != "")
            {
                GlobalGPUTextureRes normalTexRes;
                LoadGpuTextureRes(materialRes.normalTextureFile, device, gfxQueue);
                GetGpuTextureRes(materialRes.normalTextureFile, normalTexRes);
                GlobalGPUMaterialRes::Pack& pack = newMaterial.textures.emplace_back();
                pack.texture     = normalTexRes.texture;
                pack.textureView = normalTexRes.textureView;
                pack.name        = materialRes.baseColorTextureFile;
                pack.slotIndex   = 0;
            }
            if (materialRes.metallicTextureFile != "")
            {
                GlobalGPUTextureRes metallicTexRes;
                LoadGpuTextureRes(materialRes.metallicTextureFile, device, gfxQueue);
                GetGpuTextureRes(materialRes.metallicTextureFile, metallicTexRes);
                GlobalGPUMaterialRes::Pack& pack = newMaterial.textures.emplace_back();
                pack.texture     = metallicTexRes.texture;
                pack.textureView = metallicTexRes.textureView;
                pack.name        = materialRes.baseColorTextureFile;
                pack.slotIndex   = 0;
            }
            if (materialRes.roughnessTextureFile != "")
            {
                GlobalGPUTextureRes roughnessTexRes;
                LoadGpuTextureRes(materialRes.roughnessTextureFile, device, gfxQueue);
                GetGpuTextureRes(materialRes.roughnessTextureFile, roughnessTexRes);
                GlobalGPUMaterialRes::Pack& pack = newMaterial.textures.emplace_back();
                pack.texture     = roughnessTexRes.texture;
                pack.textureView = roughnessTexRes.textureView;
                pack.name        = materialRes.baseColorTextureFile;
                pack.slotIndex   = 0;
            }
        }
        else
        {
            //TODO: default texture
        }

        GPUDescriptorSetDescriptor setDesc = {
            .root_signature = rs,
            .set_index      = 1 // 
        };
        newMaterial.set     = GPUCreateDescriptorSet(device, &setDesc);
        newMaterial.sampler = sampler;

        std::vector<GPUDescriptorData> desc_data(1 + newMaterial.textures.size()); // 1 sampler + all textures
        desc_data[0].name         = u8"texSamp";
        desc_data[0].binding      = 0;
        desc_data[0].binding_type = GPU_RESOURCE_TYPE_SAMPLER;
        desc_data[0].count        = 1;
        desc_data[0].samplers     = &newMaterial.sampler;
        for (size_t i = 0; i < newMaterial.textures.size(); i++)
        {
            desc_data[i + 1].name         = nullptr;
            desc_data[i + 1].binding      = newMaterial.textures[i].slotIndex + desc_data[0].binding + 1; // todo : a better way?
            desc_data[i + 1].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
            desc_data[i + 1].count        = 1;
            desc_data[i + 1].textures     = &newMaterial.textures[i].textureView;
        }
        GPUUpdateDescriptorSet(newMaterial.set, desc_data.data(), desc_data.size());
    }
    
    void FreeGpuMaterialPool()
    {
        for (auto iter : g_gpu_material_pool)
        {
            if (iter.second.set) GPUFreeDescriptorSet(iter.second.set);
        }
        g_gpu_material_pool.clear();
    }

    bool GetGpuMaterialRes(const std::string& name, GlobalGPUMaterialRes& out)
    {
        auto iter = g_gpu_material_pool.find(name);
        if (iter == g_gpu_material_pool.end()) return false;
        out = iter->second;
        return true;
    }
    ///////////////////////////////////////////////////
}