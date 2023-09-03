#include "model.hpp"
#include <fstream>
#include <sstream>
#include "Utils/Json/JsonReader.h"
#include <unordered_map>
#include "sky_box.hpp"

Model::Model(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue)
: mDevice(device), mGfxQueue(gfxQueue)
{
    LoadModel(file);
}

Model::~Model()
{
    for (auto& mat : mMaterials)
    {
        if (mat.second->set) GPUFreeDescriptorSet(mat.second->set);
        free(mat.second);
    }
    mTexturePool.clear();
    if (mUBO) GPUFreeBuffer(mUBO);
    if (mVertexBuffer) GPUFreeBuffer(mVertexBuffer);
    if (mIndexBuffer) GPUFreeBuffer(mIndexBuffer);
    if (mSampler) GPUFreeSampler(mSampler);
    if (mSet) GPUFreeDescriptorSet(mSet);
    if (mShadowMapSet) GPUFreeDescriptorSet(mShadowMapSet);
    if (mRootSignature) GPUFreeRootSignature(mRootSignature);
    if (mPbrPipeline) GPUFreeRenderPipeline(mPbrPipeline);
}

void Model::LoadModel(const std::string_view file)
{
    mMeshFile = file;
    //std::filesystem::path assetFilePath = GetFullPath(assetUrl);
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
    std::unordered_map<uint32_t, std::vector<MeshData>> meshGroup; //<materianIndex, meshList>
    std::unordered_map<uint32_t, std::string> diffuse_textures; //<materianIndex, >
    
    reader.StartObject();
    {
        reader.Key("mMeshes");
        size_t meshCount = 0;
        reader.StartArray(&meshCount);
        {
            //mMeshes.reserve(meshCount);

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
                            reader.Value(v.btan_z);
                        }
                        reader.EndObject();
                        meshData.vertices.emplace_back(v);
                        mBoundingBox.Update(glm::vec3(v.x, v.y, v.z));
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
                        std::string texName("");
                        if (reader.HasKey("url"))
                        {
                            reader.Key("url");
                            reader.Value(texName);
                        }
                        reader.EndObject();
                        diffuse_textures.insert(std::make_pair(materialIndex, texName));
                    }
                    reader.EndObject(); */
                    //texture end
                }
                reader.EndObject();

                auto& meshDataList = meshGroup[materialIndex];
                meshDataList.emplace_back(meshData);
                //mMeshes.emplace_back(mesh);
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
                NewVertex& v = meshData.vertices[i];
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
        
        //texture
       /*  auto tex_iter = diffuse_textures.find(mat_idx);
        if (tex_iter != diffuse_textures.end() && tex_iter->second != "")
        {
            subMesh.diffuse_tex_url = tex_iter->second;
        }
        else
        {
            subMesh.diffuse_tex_url = "";
        } */

        mMesh.subMeshes.emplace_back(subMesh);
    }
}

void Model::LoadMaterial()
{
    std::ifstream is(mMeshFile.data());
    if (!is.is_open())
    {
        return;
    }
    std::stringstream ss;
    ss << is.rdbuf();
    std::string json_str(ss.str());

    is.close();

    std::unordered_map<uint32_t, std::vector<std::pair<PBRMaterialTextureType, std::pair<std::string, bool>>>> materials;
    FakeReal::JsonReader reader(json_str.c_str());
    reader.StartObject();
    {
        size_t materialNum = 0;
        reader.Key("mMaterials");
        reader.StartArray(&materialNum);
        for (size_t i = 0; i < materialNum; i++)
        {
            std::vector<std::pair<PBRMaterialTextureType, std::pair<std::string, bool>>> textures;
            reader.StartObject();

            uint32_t materialIndex = 0;
            reader.Key("index");
            reader.Value(materialIndex);

            size_t textureNum = 0;
            reader.Key("textures");
            reader.StartArray(&textureNum);
            for (size_t t = 0; t < textureNum; t++)
            {
                reader.StartObject();
                std::string name;
                reader.Key("name");
                reader.Value(name);
                std::string type;
                reader.Key("type");
                reader.Value(type);
                reader.EndObject();
                //if (type == "diffuse")
                {
                    PBRMaterialTextureType t = PBR_MTT_DIFFUSE;
                    if (type == "diffuse")
                    {
                        t = PBR_MTT_DIFFUSE;
                    }
                    else if (type == "normal")
                    {
                        t = PBR_MTT_NORMAL;
                    }
                    else if (type == "metallic")
                    {
                        t = PBR_MTT_METALLIC;
                    }
                    else if (type == "roughness")
                    {
                        t = PBR_MTT_ROUGHNESS;
                    }
                    textures.emplace_back(std::pair<PBRMaterialTextureType, std::pair<std::string, bool>>{t, {name, true}});
                }
            }
            reader.EndArray();

            reader.EndObject();
            if (textures.size())
            {
                materials.emplace(materialIndex, textures);
            }
        }
        reader.EndArray();
    }
    reader.EndObject();

    for (const auto& pair: materials)
    {
        if (pair.second.size()) CreateMaterial(pair.first, pair.second);
    }
}

void Model::UploadResource(class SkyBox* skyBox)
{
    GPUBufferDescriptor v_desc = {
        .size         = mMesh.GetMeshDataVerticesByteSize(),
        .descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER,
        .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
        .flags        = GPU_BCF_OWN_MEMORY_BIT
    };
    mVertexBuffer = GPUCreateBuffer(mDevice, &v_desc);

    GPUBufferDescriptor i_desc = {
        .size         = mMesh.GetMeshDataIndicesByteSize(),
        .descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER,
        .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
        .flags        = GPU_BCF_OWN_MEMORY_BIT
    };
    mIndexBuffer = GPUCreateBuffer(mDevice, &i_desc);

    uint32_t uploadBufferSize         = v_desc.size + i_desc.size;
    GPUBufferDescriptor upload_buffer = {
        .size         = uploadBufferSize,
        .descriptors  = GPU_RESOURCE_TYPE_NONE,
        .memory_usage = GPU_MEM_USAGE_CPU_ONLY,
        .flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT,
    };
    GPUBufferID uploadBuffer = GPUCreateBuffer(mDevice, &upload_buffer);

    GPUCommandPoolID pool               = GPUCreateCommandPool(mGfxQueue);
    GPUCommandBufferDescriptor cmd_desc = {
        .isSecondary = false
    };
    GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmd_desc);

    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        memcpy(uploadBuffer->cpu_mapped_address, mMesh.meshData.vertices.data(), v_desc.size);
        GPUBufferToBufferTransfer trans_desc = {
            .dst        = mVertexBuffer,
            .dst_offset = 0,
            .src        = uploadBuffer,
            .src_offset = 0,
            .size       = v_desc.size
        };
        GPUCmdTransferBufferToBuffer(cmd, &trans_desc);

        memcpy((uint8_t*)uploadBuffer->cpu_mapped_address + v_desc.size, mMesh.meshData.indices.data(), i_desc.size);
        trans_desc = {
            .dst        = mIndexBuffer,
            .dst_offset = 0,
            .src        = uploadBuffer,
            .src_offset = v_desc.size,
            .size       = i_desc.size
        };
        GPUCmdTransferBufferToBuffer(cmd, &trans_desc);
        
        GPUBufferBarrier barriers[2] = {};
        barriers[0].buffer    = mVertexBuffer;
        barriers[0].src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barriers[0].dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        barriers[1].buffer    = mIndexBuffer;
        barriers[1].src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barriers[1].dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER;
        GPUResourceBarrierDescriptor rs_barrer = {
            .buffer_barriers       = barriers,
            .buffer_barriers_count = 2
        };
        GPUCmdResourceBarrier(cmd, &rs_barrer);
    }
    GPUCmdEnd(cmd);
    GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
    GPUSubmitQueue(mGfxQueue, &cpy_submit);
    GPUWaitQueueIdle(mGfxQueue);

    GPUFreeBuffer(uploadBuffer);
    GPUFreeCommandBuffer(cmd);
    GPUFreeCommandPool(pool);

    GPUSamplerDescriptor sampleDesc = {
        .min_filter   = GPU_FILTER_TYPE_LINEAR,
        .mag_filter   = GPU_FILTER_TYPE_LINEAR,
        .mipmap_mode  = GPU_MIPMAP_MODE_LINEAR,
        .address_u    = GPU_ADDRESS_MODE_REPEAT,
        .address_v    = GPU_ADDRESS_MODE_REPEAT,
        .address_w    = GPU_ADDRESS_MODE_REPEAT,
        .compare_func = GPU_CMP_NEVER,
    };
    mSampler = GPUCreateSampler(mDevice, &sampleDesc);
    const char8_t* samplerName = u8"texSamp";

    uint32_t* shaderCode;
    uint32_t size = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/pbr_scene_model.vert", &shaderCode, &size);
    GPUShaderLibraryDescriptor shaderDesc = {
        .pName    = u8"",
        .code     = shaderCode,
        .codeSize = size,
        .stage    = GPU_SHADER_STAGE_VERT
    };
    GPUShaderLibraryID vsShader = GPUCreateShaderLibrary(mDevice, &shaderDesc);
    free(shaderCode);
    shaderCode = nullptr;
    size = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/pbr_scene_model.frag", &shaderCode, &size);
    shaderDesc = {
        .pName    = u8"",
        .code     = shaderCode,
        .codeSize = size,
        .stage    = GPU_SHADER_STAGE_FRAG
    };
    GPUShaderLibraryID psShader = GPUCreateShaderLibrary(mDevice, &shaderDesc);
    free(shaderCode);
    shaderCode = nullptr;

    CGPUShaderEntryDescriptor entry_desc[2] = {};
    entry_desc[0].pLibrary = vsShader;
    entry_desc[0].entry    = u8"main";
    entry_desc[0].stage    = GPU_SHADER_STAGE_VERT;
    entry_desc[1].pLibrary = psShader;
    entry_desc[1].entry    = u8"main";
    entry_desc[1].stage    = GPU_SHADER_STAGE_FRAG;

    GPURootSignatureDescriptor rs_desc = {
        .shaders              = entry_desc,
        .shader_count         = 2,
    };
    mRootSignature = GPUCreateRootSignature(mDevice, &rs_desc);

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
    GPURenderPipelineDescriptor pipelineDesc = {
        .pRootSignature     = mRootSignature,
        .pVertexShader      = &entry_desc[0],
        .pFragmentShader    = &entry_desc[1],
        .pVertexLayout      = &vertexLayout,
        .pDepthState        = &depthDesc,
        .pRasterizerState   = &rasterizerState,
        .primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST,
        .pColorFormats      = const_cast<EGPUFormat*>(&swapchainFormat),
        .renderTargetCount  = 1,
        .depthStencilFormat = GPU_FORMAT_D32_SFLOAT
    };
    mPbrPipeline = GPUCreateRenderPipeline(mDevice, &pipelineDesc);
    GPUFreeShaderLibrary(vsShader);
    GPUFreeShaderLibrary(psShader);

    GPUDescriptorSetDescriptor setDesc = {
        .root_signature = mRootSignature,
        .set_index      = 0
    };
    mSet = GPUCreateDescriptorSet(mDevice, &setDesc);

    setDesc = {
        .root_signature = mRootSignature,
        .set_index      = 2
    };
    mShadowMapSet = GPUCreateDescriptorSet(mDevice, &setDesc);

    GPUBufferDescriptor uboDesc = {
        .size             = sizeof(PerframeUniformBuffer),
        .descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER,
        .memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU,
        .flags            = GPU_BCF_PERSISTENT_MAP_BIT,
        .prefer_on_device = true
    };
    mUBO = GPUCreateBuffer(mDevice, &uboDesc);

    GPUDescriptorData dataDesc[5] = {};
    dataDesc[0].binding           = 0;
    dataDesc[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    dataDesc[0].textures          = &skyBox->mIrradianceMap->mTextureView;
    dataDesc[0].count             = 1;
    dataDesc[1].binding           = 1;
    dataDesc[1].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    dataDesc[1].textures          = &skyBox->mPrefilteredMap->mTextureView;
    dataDesc[1].count             = 1;
    dataDesc[2].binding           = 2;
    dataDesc[2].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    dataDesc[2].textures          = &skyBox->mBRDFLut->mTextureView;
    dataDesc[2].count             = 1;
    dataDesc[3].binding           = 3;
    dataDesc[3].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    dataDesc[3].samplers          = &skyBox->mSamplerRef;
    dataDesc[3].count             = 1;
    dataDesc[4].binding           = 4;
    dataDesc[4].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    dataDesc[4].buffers           = &mUBO;
    dataDesc[4].count             = 1;
    GPUUpdateDescriptorSet(mSet, dataDesc, 5);

    //material
    LoadMaterial();
}

void Model::UpdateShadowMapSet(GPUTextureViewID shadowMap, GPUSamplerID sampler)
{
    GPUDescriptorData dataDesc[2] = {};
    dataDesc[0].binding           = 0;
    dataDesc[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    dataDesc[0].textures          = &shadowMap;
    dataDesc[0].count             = 1;
    dataDesc[1].binding           = 1;
    dataDesc[1].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    dataDesc[1].samplers          = &sampler;
    dataDesc[1].count             = 1;
    GPUUpdateDescriptorSet(mShadowMapSet, dataDesc, 2);
}

PBRMaterial* Model::CreateMaterial(uint32_t materialIndex, const std::vector<std::pair<PBRMaterialTextureType, std::pair<std::string, bool>>>& textures)
{
    //find and return
    auto mat_iter = mMaterials.find(materialIndex);
    if (mat_iter != mMaterials.end()) return mat_iter->second;

    PBRMaterial* m = (PBRMaterial*)calloc(1, sizeof(PBRMaterial));
    m->textures.reserve(textures.size());

    for (size_t i = 0; i < textures.size(); i++)
    {
        PBRMaterialTextureType type = textures[i].first;
        std::string_view file       = textures[i].second.first;
        bool flip                   = textures[i].second.second;

        auto tex_iter = mTexturePool.find(file);
        if (tex_iter != mTexturePool.end())
        {
            auto& pack = m->textures.emplace_back();
            pack.texture = tex_iter->second.mTexture;
            pack.textureView = tex_iter->second.mTextureView;
            pack.textureType = type;
            pack.slotIndex   = type;
            pack.name = file;
            continue;
        }

        EGPUFormat format = type == PBR_MTT_DIFFUSE ? GPU_FORMAT_R8G8B8A8_SRGB : GPU_FORMAT_R8G8B8A8_UNORM;
        TextureData temp;
        auto&& result = mTexturePool.emplace(file, std::move(temp));
        result.first->second.LoadTexture(file, format, mDevice, mGfxQueue, flip);
        //textureData.LoadTexture(file, format, mDevice, mGfxQueue, flip);

        auto& pack = m->textures.emplace_back();
        pack.texture     = result.first->second.mTexture;
        pack.textureView = result.first->second.mTextureView;
        pack.textureType = type;
        pack.slotIndex   = type;
        pack.name = file;
    }

    GPUDescriptorSetDescriptor setDesc = {
        .root_signature = mRootSignature,
        .set_index      = 1
    };
    m->set     = GPUCreateDescriptorSet(mDevice, &setDesc);
    m->sampler = mSampler;

    std::vector<GPUDescriptorData> desc_data(1 + m->textures.size()); // 1 sampler + all textures
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
    GPUUpdateDescriptorSet(m->set, desc_data.data(), desc_data.size());

    mMaterials.emplace(materialIndex, m);
    return m;
}

void Model::Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos, const glm::mat4& lightSpaceMatrix)
{
    PointLight pointLight = {
        .position  = glm::vec3(-1.0f, 0.0f, 0.0f),
        .color     = glm::vec3(0.0f, 1.0f, 0.0f),
        .constant  = 1.0f,
        .linear    = 0.22f,
        .quadratic = 0.2f
    };
    //update uniform bffer
    PerframeUniformBuffer ubo = {
        .view                       = view,
        .proj                       = proj,
        .lightSpaceMat              = lightSpaceMatrix,
        .viewPos                    = viewPos,
        .directionalLight.direction = glm::vec3(0.5f, -0.5f, 0.5f),
        .directionalLight.color     = glm::vec3(1.0, 1.0, 1.0),
        .pointLight                 = pointLight
    };
    memcpy(mUBO->cpu_mapped_address, &ubo, sizeof(ubo));

    // reorganize mesh
    std::unordered_map<PBRMaterial*, std::vector<SubMesh*>> drawNodesInfo;
    for (size_t i = 0; i < mMesh.subMeshes.size(); i++)
    {
        const auto itr = mMaterials.find(mMesh.subMeshes[i].materialIndex);
        if (itr != mMaterials.end())
        {
            auto cur_pair = drawNodesInfo.find(itr->second);
            if (cur_pair != drawNodesInfo.end())
            {
                cur_pair->second.push_back(&mMesh.subMeshes[i]);
            }
            else
            {
                auto& nodes = drawNodesInfo[itr->second];
                nodes.push_back(&mMesh.subMeshes[i]);
            }
        }
    }

    //draw call
    GPURenderEncoderBindPipeline(encoder, mPbrPipeline);
    GPURenderEncoderBindDescriptorSet(encoder, mSet);
    uint32_t strides = sizeof(NewVertex);
    GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &strides, nullptr);
    GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, sizeof(uint32_t));
    glm::mat4 matrices[1];
    matrices[0] = /* glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 1.f, 1.f)) */glm::mat4(1.0f);
    //matrices[1] = lightSpaceMatrix;
    //glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));
    for (auto& nodePair : drawNodesInfo)
    {
        GPURenderEncoderBindDescriptorSet(encoder, mShadowMapSet);
        //per material
        auto& mat = nodePair.first;
        GPURenderEncoderBindDescriptorSet(encoder, mat->set);
        for (size_t i = 0; i < nodePair.second.size() && i < 1 ; i++)
        {
            //per mesh
            auto& mesh = nodePair.second[i];
            uint32_t indexCount = mesh->indexCount;
            //glm::mat4 model(1.0f);
            GPURenderEncoderPushConstant(encoder, mRootSignature, matrices);
            GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mesh->indexOffset, mesh->vertexOffset, 0);

            //matrices[0] = glm::translate(matrices[0], glm::vec3(3.0, 3.0, 0.0));
            //GPURenderEncoderPushConstant(encoder, mRootSignature, &matrices);
            //GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mesh->indexOffset, mesh->vertexOffset, 0);
        }
    }
}