#include "character_model.hpp"
#include "stb_image.h"
#include "utils.hpp"
#include "Utils/Json/JsonReader.h"
#include <map>
#include <stdint.h>
#include "Math/Matrix.h"
#include "sky_box.hpp"

MaterialInstanceID CreateMaterial(const std::vector<std::pair<MaterialTextureType, std::pair<std::string_view, bool>>>& textures,
                                  const std::vector<std::pair<EGPUShaderStage, std::string_view>>& shaders,
                                  GPUDeviceID device, GPUQueueID gfxQueue,  class SkyBox* skyBox)
{
    MaterialInstance* m = (MaterialInstance*)calloc(1, sizeof(MaterialInstance));
    m->textures.reserve(textures.size());

    std::vector<void*> pixels(textures.size());
    uint32_t totalBytes = 0u;
    for (size_t i = 0; i < textures.size(); i++)
    {
        MaterialTextureType type = textures[i].first;
        std::string_view file    = textures[i].second.first;
        bool flip                = textures[i].second.second;
        stbi_set_flip_vertically_on_load(flip);
        int w, h, n;
        pixels[i] = stbi_load(file.data(), &w, &h, &n, 4);
        if (!pixels[i])
        {
            assert(0);
            free(m);
            return nullptr;
        }

        totalBytes += w * h * 4u;

        EGPUFormat format = type == MTT_DIFFUSE ? GPU_FORMAT_R8G8B8A8_SRGB : GPU_FORMAT_R8G8B8A8_UNORM;
        auto& pack = m->textures.emplace_back();
        GPUTextureDescriptor desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = (uint32_t)w,
            .height      = (uint32_t)h,
            .depth       = 1,
            .array_size  = 1,
            .format      = format,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_COPY_DEST,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE
        };
        pack.texture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc = {
            .pTexture        = pack.texture,
            .format          = (EGPUFormat)pack.texture->format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1,
        };
        pack.textureView = GPUCreateTextureView(device, &tex_view_desc);
        pack.textureType = type;
        pack.slotIndex   = type;
    }

    GPUBufferDescriptor upload_buffer = {
        .size         = totalBytes,
        .descriptors  = GPU_RESOURCE_TYPE_NONE,
        .memory_usage = GPU_MEM_USAGE_CPU_ONLY,
        .flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT
    };
    GPUBufferID uploadBuffer = GPUCreateBuffer(device, &upload_buffer);

    GPUCommandPoolID pool              = GPUCreateCommandPool(gfxQueue);
    GPUCommandBufferDescriptor cmdDesc = { .isSecondary = false };
    GPUCommandBufferID cmd             = GPUCreateCommandBuffer(pool, &cmdDesc);
    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        std::vector<GPUTextureBarrier> barriers(m->textures.size());
        uint32_t srcOffset = 0;
        for (uint32_t i = 0; i < m->textures.size(); i++)
        {
            uint32_t size = m->textures[i].texture->width * m->textures[i].texture->height * 4;
            memcpy((uint8_t*)uploadBuffer->cpu_mapped_address + srcOffset, pixels[i], size);

            GPUBufferToTextureTransfer trans_texture_buffer_desc = {
                .dst                              = m->textures[i].texture,
                .dst_subresource.mip_level        = 0,
                .dst_subresource.base_array_layer = 0,
                .dst_subresource.layer_count      = 1,
                .src                              = uploadBuffer,
                .src_offset                       = srcOffset,
            };
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);

            srcOffset += size;

            barriers[i].texture = m->textures[i].texture;
            barriers[i].src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barriers[i].dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
        }
        GPUResourceBarrierDescriptor rs_barrer = {
            .texture_barriers       = barriers.data(),
            .texture_barriers_count = (uint32_t)barriers.size()
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
    for (size_t i = 0; i < pixels.size(); i++)
    {
        stbi_image_free(pixels[i]);
    }

    GPURenderPipelineDescriptor pipelineDesc {};
    // shader
    GPUShaderEntryDescriptor shaderEntries[5] = {nullptr};
    std::vector<GPUShaderLibraryID> shaderLibs;;
    uint32_t shaderCount = 0;
    for (size_t i = 0; i < shaders.size() && i < 5; i++)
    {
        uint32_t* shaderCode;
        uint32_t size = 0;
        ReadShaderBytes((char8_t*)shaders[i].second.data(), &shaderCode, &size);
        GPUShaderLibraryDescriptor shaderDesc = {
            .pName    = (char8_t*)shaders[i].second.data(),
            .code     = shaderCode,
            .codeSize = size,
            .stage    = shaders[i].first
        };
        shaderLibs.push_back(GPUCreateShaderLibrary(device, &shaderDesc));

        switch (shaders[i].first)
        {
            case GPU_SHADER_STAGE_VERT:
            {
                shaderEntries[shaderCount].stage    = GPU_SHADER_STAGE_VERT;
                shaderEntries[shaderCount].entry    = u8"main";
                shaderEntries[shaderCount].pLibrary = shaderLibs[i];
                pipelineDesc.pVertexShader          = &shaderEntries[shaderCount];
                shaderCount++;
            }
            break;
            case GPU_SHADER_STAGE_FRAG:
            {
                shaderEntries[shaderCount].stage    = GPU_SHADER_STAGE_FRAG;
                shaderEntries[shaderCount].entry    = u8"main";
                shaderEntries[shaderCount].pLibrary = shaderLibs[i];
                pipelineDesc.pFragmentShader        = &shaderEntries[shaderCount];
                shaderCount++;
            }
            break;
            case GPU_SHADER_STAGE_GEOM:
            {
                shaderEntries[shaderCount].stage    = GPU_SHADER_STAGE_GEOM;
                shaderEntries[shaderCount].entry    = u8"main";
                shaderEntries[shaderCount].pLibrary = shaderLibs[i];
                pipelineDesc.pGeometryShader        = &shaderEntries[shaderCount];
                shaderCount++;
            }
            break;
            case GPU_SHADER_STAGE_TESC: break;
            case GPU_SHADER_STAGE_TESE: break;
            default:
                assert(false && "Shader Stage not supported!");
                break;
        }

        free(shaderCode);
    }

    // create root signature
    GPURootSignatureDescriptor rootRSDesc = {
        .shaders      = shaderEntries,
        .shader_count = shaderCount
    };
    m->rootSignature = GPUCreateRootSignature(device, &rootRSDesc);

    // vertex layout
    GPUVertexLayout vertexLayout {};
    vertexLayout.attributeCount = 5;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[3]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 8, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[4]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 11, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
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
    EGPUFormat swapchainFormat = GPU_FORMAT_B8G8R8A8_UNORM;
    pipelineDesc.pRootSignature     = m->rootSignature;
    pipelineDesc.pVertexLayout      = &vertexLayout;
    pipelineDesc.pDepthState        = &depthDesc;
    pipelineDesc.pRasterizerState   = &rasterizerState;
    pipelineDesc.primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST;
    pipelineDesc.pColorFormats      = const_cast<EGPUFormat*>(&swapchainFormat);
    pipelineDesc.renderTargetCount  = 1;
    pipelineDesc.depthStencilFormat = GPU_FORMAT_D32_SFLOAT;
    m->pipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
    for (size_t i = 0; i < shaderLibs.size(); i++)
    {
        GPUFreeShaderLibrary(shaderLibs[i]);
    }

    GPUDescriptorSetDescriptor set_desc = {
        .root_signature = m->rootSignature,
        .set_index = 0
    };
    m->set = GPUCreateDescriptorSet(device, &set_desc);

    GPUSamplerDescriptor sampleDesc = {
        .min_filter   = GPU_FILTER_TYPE_LINEAR,
        .mag_filter   = GPU_FILTER_TYPE_LINEAR,
        .mipmap_mode  = GPU_MIPMAP_MODE_LINEAR,
        .address_u    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        .address_v    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        .address_w    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        .compare_func = GPU_CMP_NEVER,
    };
    m->sampler = GPUCreateSampler(device, &sampleDesc);

    //update descriptor set
    if (skyBox == nullptr)
    {std::vector<GPUDescriptorData> desc_data(1 + m->textures.size()); // 1 sampler + all textures
    desc_data[0].name              = u8"texSamp";
    desc_data[0].binding           = 0;
    desc_data[0].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    desc_data[0].count             = 1;
    desc_data[0].samplers          = &m->sampler;
    for (size_t i = 0; i < m->textures.size(); i++)
    {
        desc_data[i + 1].name         = nullptr;
        desc_data[i + 1].binding      = m->textures[i].slotIndex + desc_data[0].binding + 1; // todo : a better way?
        desc_data[i + 1].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
        desc_data[i + 1].count        = 1;
        desc_data[i + 1].textures     = &m->textures[i].textureView;
    }
    GPUUpdateDescriptorSet(m->set, desc_data.data(), (uint32_t)desc_data.size());}
    else {
    std::vector<GPUDescriptorData> desc_data(1 + m->textures.size() + 3); // 1 sampler + all textures + 3 IBL texture
    desc_data[0].name              = u8"texSamp";
    desc_data[0].binding           = 0;
    desc_data[0].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    desc_data[0].count             = 1;
    desc_data[0].samplers          = &m->sampler;
    for (size_t i = 0; i < m->textures.size(); i++)
    {
        desc_data[i + 1].name         = nullptr;
        desc_data[i + 1].binding      = m->textures[i].slotIndex + desc_data[0].binding + 1; // todo : a better way?
        desc_data[i + 1].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
        desc_data[i + 1].count        = 1;
        desc_data[i + 1].textures     = &m->textures[i].textureView;
    }
    desc_data[4].name              = u8"irradianceMap";
    desc_data[4].binding           = 4;
    desc_data[4].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    desc_data[4].count             = 1;
    desc_data[4].textures          = &skyBox->mIrradianceMap->mTextureView;
    desc_data[5].name              = u8"preFilteredMap";
    desc_data[5].binding           = 5;
    desc_data[5].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    desc_data[5].count             = 1;
    desc_data[5].textures          = &skyBox->mPrefilteredMap->mTextureView;
    desc_data[6].name              = u8"brdfLutTex";
    desc_data[6].binding           = 6;
    desc_data[6].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    desc_data[6].count             = 1;
    desc_data[6].textures          = &skyBox->mBRDFLut->mTextureView;
    GPUUpdateDescriptorSet(m->set, desc_data.data(), (uint32_t)desc_data.size());
    }

    return m;
}

void DestryMaterial(MaterialInstanceID m)
{
    MaterialInstance* mat = (MaterialInstance*)m;
    if (mat)
    {
        if (mat->diffuseTextureView) GPUFreeTextureView(mat->diffuseTextureView);
        if (mat->normalTextureView) GPUFreeTextureView(mat->normalTextureView);
        if (mat->maskTextureView) GPUFreeTextureView(mat->maskTextureView);
        if (mat->maskTexture) GPUFreeTexture(mat->maskTexture);
        if (mat->normalTexture) GPUFreeTexture(mat->normalTexture);
        if (mat->diffuseTexture) GPUFreeTexture(mat->diffuseTexture);
        if (mat->set) GPUFreeDescriptorSet(mat->set);
        if (m->sampler) GPUFreeSampler(m->sampler);
        if (mat->rootSignature) GPUFreeRootSignature(mat->rootSignature);
        if (mat->pipeline) GPUFreeRenderPipeline(mat->pipeline);
        for (auto& pack : mat->textures)
        {
            if(pack.textureView) GPUFreeTextureView(pack.textureView);
            if (pack.texture) GPUFreeTexture(pack.texture);
        }
        mat->textures.clear();

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

    if (mVertexBuffer) GPUFreeBuffer(mVertexBuffer);
    if (mIndexBuffer) GPUFreeBuffer(mIndexBuffer);
    if (mSampler) GPUFreeSampler(mSampler);
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
                        NewVertex v{};
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
                            reader.Value(v.tan_z);
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
    std::vector<NewVertex> tmpVertices;
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
                NewVertex& v  = meshData.vertices[i];
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

void CharacterModel::InitMaterials(GPUDeviceID device, GPUQueueID gfxQueue,  class SkyBox* skyBox)
{
    std::vector<std::pair<MaterialTextureType, std::pair<std::string_view, bool>>> m1_tex = {
        {MaterialTextureType::MTT_DIFFUSE, {"../../../../asset/objects/character/_maps/Garam_diff_1.tga", true}},
        {MaterialTextureType::MTT_NORMAL, {"../../../../asset/objects/character/_maps/Garam_norm.tga", true}},
        {MaterialTextureType::MTT_MASK, {"../../../../asset/objects/character/_maps/Garam_mask.tga", true}}
    };
    std::vector<std::pair<MaterialTextureType, std::pair<std::string_view, bool>>> m2_tex = {
        {MaterialTextureType::MTT_DIFFUSE, {"../../../../asset/objects/character/_maps/Garam_diff_1.tga", true}},
        {MaterialTextureType::MTT_NORMAL, {"../../../../asset/objects/character/_maps/Garam_norm.tga", true}},
        {MaterialTextureType::MTT_MASK, {"../../../../asset/objects/character/_maps/_fur_01.tga", true}}
    };
    std::vector<std::pair<MaterialTextureType, std::pair<std::string_view, bool>>> m3_tex = {
        {MaterialTextureType::MTT_DIFFUSE, {"../../../../asset/objects/character/_maps/_DiffuseTexture.tga", true}},
        {MaterialTextureType::MTT_NORMAL, {"../../../../asset/objects/character/_maps/Garam_norm.tga", true}},
        {MaterialTextureType::MTT_MASK, {"../../../../asset/objects/character/_maps/Garam_mask.tga", true}}
    };
    std::vector<std::pair<EGPUShaderStage, std::string_view>> m1_shaders = {
        {GPU_SHADER_STAGE_VERT, "../../../../samples/simple_gpu/shader/character.vert"},
        {GPU_SHADER_STAGE_FRAG, "../../../../samples/simple_gpu/shader/character_body.frag"}
    };
    std::vector<std::pair<EGPUShaderStage, std::string_view>> m2_shaders = {
        {GPU_SHADER_STAGE_VERT, "../../../../samples/simple_gpu/shader/character.vert"},
        {GPU_SHADER_STAGE_FRAG, "../../../../samples/simple_gpu/shader/character_fur.frag"}
    };
    std::vector<std::pair<EGPUShaderStage, std::string_view>> m3_shaders = {
        {GPU_SHADER_STAGE_VERT, "../../../../samples/simple_gpu/shader/character.vert"},
        {GPU_SHADER_STAGE_FRAG, "../../../../samples/simple_gpu/shader/character.frag"}
    };
    MaterialInstanceID m1 = CreateMaterial(m1_tex, m1_shaders, device, gfxQueue, skyBox);
    MaterialInstanceID m2 = CreateMaterial(m2_tex, m2_shaders, device, gfxQueue, skyBox);
    MaterialInstanceID m3 = CreateMaterial(m3_tex, m3_shaders, device, gfxQueue, nullptr);
    mMaterials.insert(std::make_pair(1, m1));
    mMaterials.insert(std::make_pair(2, m2));
    mMaterials.insert(std::make_pair(3, m3));
}

void CharacterModel::InitModelResource(GPUDeviceID device, GPUQueueID gfxQueue,  class SkyBox* skyBox)
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

    InitMaterials(device, gfxQueue, skyBox);
}

void CharacterModel::Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos)
{
    struct
    {
        glm::mat4 m;
        glm::mat4 v;
        glm::mat4 p;
        glm::vec3 viewPos;
    } pushData;
    uint32_t stride = sizeof(NewVertex);
    GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &stride, nullptr);
    uint32_t indexStride = sizeof(uint32_t);
    GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, indexStride);
    //反了
    for (size_t i = 0; i < mMesh.subMeshes.size(); i++)
    {
        auto& m = mMaterials[mMesh.subMeshes[i].materialIndex];
        GPURenderEncoderBindPipeline(encoder, m->pipeline);
        GPURenderEncoderBindDescriptorSet(encoder, m->set);
        //pushData.mvp = proj * view * glm::translate(glm::mat4(1.f), { 0.f, 0.f, 0.f });
        pushData.m = glm::mat4(1.f);
        pushData.v = view;
        pushData.p = proj;
        pushData.viewPos = glm::vec3(viewPos.x, viewPos.y, viewPos.z);
        GPURenderEncoderPushConstant(encoder, m->set->root_signature, &pushData);

        uint32_t indexCount = mMesh.subMeshes[i].indexCount;
        GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mMesh.subMeshes[i].indexOffset, mMesh.subMeshes[i].vertexOffset, 0);
    }
}

void CharacterModel::BindEnvTexture(GPUTextureViewID irradianceMap, GPUTextureViewID prefilteredMap, GPUTextureViewID brdfLutMap)
{
    // todo: complete this
    // test
    GPUSamplerID s = mMaterials[1]->sampler;
    std::vector<GPUDescriptorData> desc_data(1 + mMaterials[1]->textures.size() + 3); // 1 sampler + all textures + 3 IBL texture
    desc_data[0].name              = u8"texSamp";
    desc_data[0].binding           = 0;
    desc_data[0].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    desc_data[0].count             = 1;
    desc_data[0].samplers          = &s;
    for (size_t i = 0; i < mMaterials[1]->textures.size(); i++)
    {
        desc_data[i + 1].name         = nullptr;
        desc_data[i + 1].binding      = mMaterials[1]->textures[i].slotIndex + desc_data[0].binding + 1; // todo : a better way?
        desc_data[i + 1].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
        desc_data[i + 1].count        = 1;
        GPUTextureViewID tv = mMaterials[1]->textures[i].textureView;
        desc_data[i + 1].textures     = &tv;
    }
    desc_data[4].name              = u8"irradianceMap";
    desc_data[4].binding           = 4;
    desc_data[4].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    desc_data[4].count             = 1;
    desc_data[4].textures          = &irradianceMap;
    desc_data[5].name              = u8"preFilteredMap";
    desc_data[5].binding           = 5;
    desc_data[5].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    desc_data[5].count             = 1;
    desc_data[5].textures          = &prefilteredMap;
    desc_data[6].name              = u8"brdfLutTex";
    desc_data[6].binding           = 6;
    desc_data[6].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    desc_data[6].count             = 1;
    desc_data[6].textures          = &brdfLutMap;
    GPUUpdateDescriptorSet(mMaterials[1]->set, desc_data.data(), (uint32_t)desc_data.size());
}