#include "character_model.hpp"
#include "stb_image.h"
#include "utils.hpp"
#include "Utils/Json/JsonReader.h"
#include <map>
#include <stdint.h>

MaterialInstanceID CreateMaterial(const std::string_view diffuse, const std::string_view normal, const std::string_view mask, GPUDeviceID device, GPUQueueID gfxQueue, bool flip)
{
    MaterialInstance* m = (MaterialInstance*)calloc(1, sizeof(MaterialInstance));

    stbi_set_flip_vertically_on_load(flip);
    int w, h, n;
    void* pixel = stbi_load(diffuse.data(), &w, &h, &n, 4);
    if (!pixel)
    {
        assert(0 && "load diffuse failed!");
        return nullptr;
    }
    uint32_t sizeDiffuse = w * h * 4;

    void* normal_pixel = stbi_load(normal.data(), &w, &h, &n, 4);
    if (!normal_pixel)
    {
        assert(0 && "load normal failed!");
        return nullptr;
    }
    uint32_t sizeNormal = w * h * 4;
    
    void* mask_pixel = stbi_load(mask.data(), &w, &h, &n, 4);
    if (!mask_pixel)
    {
        assert(0 && "load mask failed!");
        return nullptr;
    }
    uint32_t sizeMask = w * h * 4;

    GPUTextureDescriptor desc = {
        .flags       = GPU_TCF_OWN_MEMORY_BIT,
        .width       = (uint32_t)w,
        .height      = (uint32_t)h,
        .depth       = 1,
        .array_size  = 1,
        .format      = GPU_FORMAT_R8G8B8A8_UNORM,
        .owner_queue = gfxQueue,
        .start_state = GPU_RESOURCE_STATE_COPY_DEST,
        .descriptors = GPU_RESOURCE_TYPE_TEXTURE
    };
    m->diffuseTexture = GPUCreateTexture(device, &desc);
    GPUTextureViewDescriptor tex_view_desc = {
        .pTexture        = m->diffuseTexture,
        .format          = (EGPUFormat)m->diffuseTexture->format,
        .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV,
        .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
        .baseMipLevel    = 0,
        .mipLevelCount   = 1,
        .baseArrayLayer  = 0,
        .arrayLayerCount = 1,
    };
    m->diffuseTextureView  = GPUCreateTextureView(device, &tex_view_desc);
    m->normalTexture       = GPUCreateTexture(device, &desc);
    tex_view_desc.pTexture = m->normalTexture;
    m->normalTextureView   = GPUCreateTextureView(device, &tex_view_desc);
    m->maskTexture         = GPUCreateTexture(device, &desc);
    tex_view_desc.pTexture = m->maskTexture;
    m->maskTextureView     = GPUCreateTextureView(device, &tex_view_desc);

    //upload
    uint32_t totalSize = std::max(sizeDiffuse, std::max(sizeNormal, sizeMask));
    GPUBufferDescriptor upload_buffer{};
    upload_buffer.size         = totalSize * 3;
    upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
    upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
    GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
    GPUCommandBufferDescriptor cmdDesc = { .isSecondary = false };
    GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        memcpy(uploadBuffer->cpu_mapped_address, pixel, sizeDiffuse);
        GPUBufferToTextureTransfer trans_texture_buffer_desc = {
            .dst                              = m->diffuseTexture,
            .dst_subresource.mip_level        = 0,
            .dst_subresource.base_array_layer = 0,
            .dst_subresource.layer_count      = 1,
            .src                              = uploadBuffer,
            .src_offset                       = 0,
        };
        GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);

        memcpy((uint8_t*)uploadBuffer->cpu_mapped_address + sizeMask, normal_pixel, sizeNormal);
        trans_texture_buffer_desc.dst        = m->normalTexture;
        trans_texture_buffer_desc.src_offset = sizeMask;
        GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);

        memcpy((uint8_t*)uploadBuffer->cpu_mapped_address + sizeMask * 2, mask_pixel, sizeMask);
        trans_texture_buffer_desc.dst        = m->maskTexture;
        trans_texture_buffer_desc.src_offset = sizeMask * 2;
        GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);

        GPUTextureBarrier barrier1 = {
            .texture   = m->diffuseTexture,
            .src_state = GPU_RESOURCE_STATE_COPY_DEST,
            .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE
        };
        GPUTextureBarrier barrier2 = {
            .texture   = m->normalTexture,
            .src_state = GPU_RESOURCE_STATE_COPY_DEST,
            .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE
        };
        GPUTextureBarrier barrier3 = {
            .texture   = m->maskTexture,
            .src_state = GPU_RESOURCE_STATE_COPY_DEST,
            .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE
        };
        GPUTextureBarrier barriers[3] = {barrier1, barrier2, barrier3};
        GPUResourceBarrierDescriptor rs_barrer = {
            .texture_barriers       = barriers,
            .texture_barriers_count = 3
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
    stbi_image_free(normal_pixel);
    stbi_image_free(mask_pixel);

    // shader
    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/character.vert", &vShaderCode, &vSize);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/character.frag", &fShaderCode, &fSize);
    GPUShaderLibraryDescriptor vShaderDesc = {
        .pName    = u8"character_vertex_shader",
        .code     = vShaderCode,
        .codeSize = vSize,
        .stage    = GPU_SHADER_STAGE_VERT
    };
    GPUShaderLibraryDescriptor fShaderDesc = {
        .pName    = u8"character_fragment_shader",
        .code     = fShaderCode,
        .codeSize = fSize,
        .stage    = GPU_SHADER_STAGE_FRAG
    };
    GPUShaderLibraryID pVShader = GPUCreateShaderLibrary(device, &vShaderDesc);
    GPUShaderLibraryID pFShader = GPUCreateShaderLibrary(device, &fShaderDesc);
    free(vShaderCode);
    free(fShaderCode);

    GPUShaderEntryDescriptor shaderEntries[2] = { 0 };
    {
        shaderEntries[0].stage    = GPU_SHADER_STAGE_VERT;
        shaderEntries[0].entry    = u8"main";
        shaderEntries[0].pLibrary = pVShader;
        shaderEntries[1].stage    = GPU_SHADER_STAGE_FRAG;
        shaderEntries[1].entry    = u8"main";
        shaderEntries[1].pLibrary = pFShader;
    }

    // create root signature
    GPURootSignatureDescriptor rootRSDesc = {
        .shaders      = shaderEntries,
        .shader_count = 2
    };
    m->rootSignature = GPUCreateRootSignature(device, &rootRSDesc);

    // vertex layout
    GPUVertexLayout vertexLayout {};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
    // renderpipeline
    GPURasterizerStateDescriptor rasterizerState = {
        .cullMode             = GPU_CULL_MODE_BACK,
        .fillMode             = GPU_FILL_MODE_SOLID,
        .frontFace            = GPU_FRONT_FACE_CCW,
        .depthBias            = 0,
        .slopeScaledDepthBias = 0.f,
        .enableMultiSample    = false,
        .enableScissor        = false,
        .enableDepthClamp     = false
    };
    GPUDepthStateDesc depthDesc = {
        .depthTest  = true,
        .depthWrite = true,
        .depthFunc  = GPU_CMP_LEQUAL
    };
    EGPUFormat format = GPU_FORMAT_B8G8R8A8_UNORM;
    GPURenderPipelineDescriptor pipelineDesc = {
        .pRootSignature     = m->rootSignature,
        .pVertexShader      = &shaderEntries[0],
        .pFragmentShader    = &shaderEntries[1],
        .pVertexLayout      = &vertexLayout,
        .pDepthState        = &depthDesc,
        .pRasterizerState   = &rasterizerState,
        .primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST,
        .pColorFormats      = const_cast<EGPUFormat*>(&format),
        .renderTargetCount  = 1,
        .depthStencilFormat = GPU_FORMAT_D32_SFLOAT,
    };
    m->pipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pVShader);
    GPUFreeShaderLibrary(pFShader);

    GPUDescriptorSetDescriptor set_desc = {
        .root_signature = m->rootSignature,
        .set_index = 0
    };
    m->set = GPUCreateDescriptorSet(device, &set_desc);

    return m;
}

void DestryMaterial(MaterialInstanceID m)
{
    MaterialInstance* mat = (MaterialInstance*)m;
    if (mat)
    {
        GPUFreeTextureView(mat->diffuseTextureView);
        GPUFreeTextureView(mat->normalTextureView);
        GPUFreeTextureView(mat->maskTextureView);
        GPUFreeTexture(mat->maskTexture);
        GPUFreeTexture(mat->normalTexture);
        GPUFreeTexture(mat->diffuseTexture);
        GPUFreeDescriptorSet(mat->set);
        GPUFreeRootSignature(mat->rootSignature);
        GPUFreeRenderPipeline(mat->pipeline);

        free(mat);
    }
}

CharacterModel::CharacterModel()
{

}

CharacterModel::~CharacterModel()
{
    for (auto m : mMaterials)
    {
        DestryMaterial(m.second);
    }

    GPUFreeBuffer(mVertexBuffer);
    GPUFreeBuffer(mIndexBuffer);
    GPUFreeSampler(mSampler);
}

void CharacterModel::LoadModel(const std::string_view file)
{
    std::ifstream is(file.data());
    if (!is.is_open())
    {
        return;
    }
    std::stringstream ss;
    ss << is.rdbuf();
    std::string json_str(ss.str());

    is.close();

    FakeReal::JsonReader reader(json_str.c_str());

    //Mesh mesh;
    std::map<uint32_t, std::vector<MeshData>> meshGroup; //<materianIndex, meshList>
    std::unordered_map<uint32_t, std::string> diffuse_textures; //<materianIndex, >
    
    reader.StartObject();
    {
        reader.Key("mMeshes");
        size_t meshCount = 0;
        reader.StartArray(&meshCount);
        {
            for (size_t m = 0; m < meshCount; m++)
            {
                MeshData meshData{};
                uint32_t materialIndex = 0;
                reader.StartObject();
                {
                    reader.Key("mMaterialIndex");
                    reader.Value(materialIndex);

                    reader.Key("mVertices");
                    size_t count = 0;
                    reader.StartArray(&count);
                    meshData.vertices.reserve(count);
                    for (size_t i = 0; i < count; i++)
                    {
                        Vertex v{};
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
                        }
                        reader.EndObject();
                        meshData.vertices.emplace_back(v);
                    }
                    reader.EndArray();

                    reader.Key("mIndices");
                    count = 0;
                    reader.StartArray(&count);
                    meshData.indices.reserve(count);
                    for (size_t i = 0; i < count; i++)
                    {
                        uint32_t val = 0;
                        reader.Value(val);
                        meshData.indices.push_back(val);
                    }
                    reader.EndArray();

                    //texture begin
                    /* reader.Key("mTexture");
                    reader.StartObject();
                    if (reader.HasKey("diffuse"))
                    {
                        reader.Key("diffuse");
                        reader.StartObject();
                        reader.Key("url");
                        std::string texName;
                        reader.Value(texName);
                        reader.EndObject();
                        diffuse_textures.insert(std::make_pair(materialIndex, texName));
                    }
                    reader.EndObject(); */
                    //texture end
                }
                reader.EndObject();

                auto& meshDataList = meshGroup[materialIndex];
                meshDataList.emplace_back(meshData);
            }
        }
        reader.EndArray();
    }
    reader.EndObject();

    //
    std::vector<Vertex> tmpVertices;
    std::vector<uint32_t> tmpIndices;
    MeshData newMesh{};
    uint32_t vertexOffset = 0;
    uint32_t indexOffset  = 0;
    for (auto iter : meshGroup)
    {
        uint32_t mat_idx = iter.first;
        auto& list = iter.second;
        tmpVertices.clear();
        tmpIndices.clear();
        for (auto& meshData : list)
        {
            for (uint32_t i = 0; i < meshData.vertices.size(); i++)
            {
                Vertex& v  = meshData.vertices[i];
                uint32_t f = 0;
                for (f = 0; f < tmpVertices.size(); f++)
                {
                    if (v == tmpVertices[f]) break;
                }
                if (f == tmpVertices.size())
                {
                    tmpVertices.emplace_back(v);
                }
                tmpIndices.push_back(f);
            }
        }

        mMesh.meshData.vertices.insert(mMesh.meshData.vertices.end(), tmpVertices.begin(), tmpVertices.end());
        mMesh.meshData.indices.insert(mMesh.meshData.indices.end(), tmpIndices.begin(), tmpIndices.end());
        SubMesh subMesh{};
        subMesh.vertexOffset  = vertexOffset;
        subMesh.vertexCount   = (uint32_t)tmpVertices.size();
        subMesh.indexOffset   = indexOffset;
        subMesh.indexCount    = (uint32_t)tmpIndices.size();
        subMesh.materialIndex = mat_idx;
        vertexOffset += subMesh.vertexCount;
        indexOffset  += subMesh.indexCount;

        mMesh.subMeshes.emplace_back(subMesh);
    }
}

void CharacterModel::InitMaterials(GPUDeviceID device, GPUQueueID gfxQueue)
{
    /* for (size_t i = 0; i < mMesh.subMeshes.size(); i++)
    {
        uint32_t
    } */
    MaterialInstanceID m1 = CreateMaterial("../../../../asset/objects/character/_maps/Garam_diff_3.tga", "../../../../asset/objects/character/_maps/Garam_norm.tga", "../../../../asset/objects/character/_maps/Garam_mask.tga", device, gfxQueue);
    mMaterials.insert(std::make_pair(1, m1));
    MaterialInstanceID m2 = CreateMaterial("../../../../asset/objects/character/_maps/Garam_diff_1.tga", "../../../../asset/objects/character/_maps/Garam_norm.tga", "../../../../asset/objects/character/_maps/Garam_mask.tga", device, gfxQueue);
    mMaterials.insert(std::make_pair(2, m2));
    MaterialInstanceID m3 = CreateMaterial("../../../../asset/objects/character/_maps/_DiffuseTexture.tga", "../../../../asset/objects/character/_maps/Garam_norm.tga", "../../../../asset/objects/character/_maps/Garam_mask.tga", device, gfxQueue, false);
    mMaterials.insert(std::make_pair(3, m3));
}

void CharacterModel::InitModelResource(GPUDeviceID device, GPUQueueID gfxQueue)
{
    GPUBufferDescriptor v_desc = {
        .size         = mMesh.GetMeshDataVerticesByteSize(),
        .descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER,
        .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
        .flags        = GPU_BCF_OWN_MEMORY_BIT
    };
    mVertexBuffer = GPUCreateBuffer(device, &v_desc);

    GPUBufferDescriptor i_desc = {
        .size         = mMesh.GetMeshDataIndicesByteSize(),
        .descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER,
        .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
        .flags        = GPU_BCF_OWN_MEMORY_BIT
    };
    mIndexBuffer = GPUCreateBuffer(device, &i_desc);

    uint32_t uploadBufferSize         = v_desc.size > i_desc.size ? v_desc.size : i_desc.size;
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

    memcpy(uploadBuffer->cpu_mapped_address, mMesh.meshData.vertices.data(), v_desc.size);

    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        GPUBufferToBufferTransfer trans_desc = {
            .dst        = mVertexBuffer,
            .dst_offset = 0,
            .src        = uploadBuffer,
            .src_offset = 0,
            .size       = v_desc.size
        };
        GPUCmdTransferBufferToBuffer(cmd, &trans_desc);
        GPUBufferBarrier barrier = {
            .buffer    = mVertexBuffer,
            .src_state = GPU_RESOURCE_STATE_COPY_DEST,
            .dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        };
        GPUResourceBarrierDescriptor rs_barrer = {
            .buffer_barriers       = &barrier,
            .buffer_barriers_count = 1
        };
        GPUCmdResourceBarrier(cmd, &rs_barrer);
    }
    GPUCmdEnd(cmd);
    GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
    GPUSubmitQueue(gfxQueue, &cpy_submit);
    GPUWaitQueueIdle(gfxQueue);

    memcpy(uploadBuffer->cpu_mapped_address, mMesh.meshData.indices.data(), i_desc.size);
    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        GPUBufferToBufferTransfer trans_desc = {
            .dst        = mIndexBuffer,
            .dst_offset = 0,
            .src        = uploadBuffer,
            .src_offset = 0,
            .size       = i_desc.size
        };
        GPUCmdTransferBufferToBuffer(cmd, &trans_desc);
        GPUBufferBarrier barrier = {
            .buffer    = mIndexBuffer,
            .src_state = GPU_RESOURCE_STATE_COPY_DEST,
            .dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER
        };
        GPUResourceBarrierDescriptor rs_barrer = {
            .buffer_barriers       = &barrier,
            .buffer_barriers_count = 1
        };
        GPUCmdResourceBarrier(cmd, &rs_barrer);
    }
    GPUCmdEnd(cmd);
    GPUSubmitQueue(gfxQueue, &cpy_submit);
    GPUWaitQueueIdle(gfxQueue);

    GPUFreeBuffer(uploadBuffer);
    GPUFreeCommandBuffer(cmd);
    GPUFreeCommandPool(pool);

    InitMaterials(device, gfxQueue);

    GPUSamplerDescriptor desc{};
    desc.min_filter   = GPU_FILTER_TYPE_LINEAR;
    desc.mag_filter   = GPU_FILTER_TYPE_LINEAR;
    desc.mipmap_mode  = GPU_MIPMAP_MODE_LINEAR;
    desc.address_u    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE;
    desc.address_v    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE;
    desc.address_w    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE;
    desc.compare_func = GPU_CMP_NEVER;
    mSampler = GPUCreateSampler(device, &desc);

    for (const auto m : mMaterials)
    {
        GPUTextureViewID v = (GPUTextureViewID)(m.second->diffuseTextureView);
        GPUDescriptorData desc_data[2] = {};
        desc_data[0].name              = u8"tex"; // shader texture2D`s name
        desc_data[0].binding           = 0;
        desc_data[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
        desc_data[0].count             = 1;
        desc_data[0].textures          = &v;
        desc_data[1].name              = u8"texSamp";
        desc_data[1].binding           = 1;
        desc_data[1].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
        desc_data[1].count             = 1;
        desc_data[1].samplers          = &mSampler;
        GPUUpdateDescriptorSet(m.second->set, desc_data, 2);
    }
}

void CharacterModel::Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos)
{
    /* UniformBuffer ubo = {
        .model   = glm::mat4(glm::mat3(view)),
        .proj    = proj,
        .viewPos = viewPos
    };
    GPUBufferRange rang{};
    rang.offset = 0;
    rang.size   = sizeof(UniformBuffer);
    GPUMapBuffer(mUniformBuffer, &rang);
    memcpy(mUniformBuffer->cpu_mapped_address, &ubo, rang.size);
    GPUUnmapBuffer(mUniformBuffer); */

    struct
    {
        glm::mat4 mvp;
    } pushData;
    uint32_t stride = sizeof(Vertex);
    GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &stride, nullptr);
    uint32_t indexStride = sizeof(uint32_t);
    GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, indexStride);
    //反了
    for (size_t i = 0; i < mMesh.subMeshes.size(); i++)
    {
        auto& m = mMaterials[mMesh.subMeshes[i].materialIndex];
        GPURenderEncoderBindPipeline(encoder, m->pipeline);
        GPURenderEncoderBindDescriptorSet(encoder, m->set);
        pushData.mvp = proj * view * glm::mat4(1.0f);
        GPURenderEncoderPushConstant(encoder, m->set->root_signature, &pushData);

        uint32_t indexCount = mMesh.subMeshes[i].indexCount;
        GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mMesh.subMeshes[i].indexOffset, mMesh.subMeshes[i].vertexOffset, 0);
    }
}