#include "model_entity.hpp"
#include <fstream>
#include "Utils/Json/JsonReader.h"
#include <filesystem>
#include "global_resources.hpp"
#include "sky_box.hpp"
#include "cascade_shadow_pass.hpp"

EntityModel::EntityModel(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue)
: mDevice(device), mGfxQueue(gfxQueue)
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
    
    std::filesystem::path filePath(file);
    std::string directory(filePath.parent_path().generic_string());
    reader.StartObject();
    {
        reader.Key("components");
        size_t componentCount = 0;
        reader.StartArray(&componentCount);
        {
            for (size_t i = 0; i < componentCount; i++)
            {
                SubMeshComponent& subMesh = mMeshComp.rawMeshes.emplace_back();
                reader.StartObject();
                {
                    // mesh
                    std::string meshName;
                    reader.Key("mesh");
                    reader.Value(meshName);
                    std::string url = directory + "/mesh/" + meshName + ".json";
                    if (!global::HasMeshRes(url))
                    {
                        global::LoadMeshRes(url);
                    }
                    subMesh.meshFile = url;

                    // tranaform
                    reader.Key("transform");
                    reader.StartObject();
                    {
                        reader.Key("position");
                        size_t unused = 0;
                        reader.StartArray(&unused);
                        reader.Value(subMesh.transform.position.x);
                        reader.Value(subMesh.transform.position.y);
                        reader.Value(subMesh.transform.position.z);
                        reader.EndArray();

                        reader.Key("rotation");
                        reader.StartArray(&unused);
                        reader.Value(subMesh.transform.rotation.w);
                        reader.Value(subMesh.transform.rotation.x);
                        reader.Value(subMesh.transform.rotation.y);
                        reader.Value(subMesh.transform.rotation.z);
                        reader.EndArray();

                        reader.Key("scale");
                        reader.StartArray(&unused);
                        reader.Value(subMesh.transform.scale.x);
                        reader.Value(subMesh.transform.scale.y);
                        reader.Value(subMesh.transform.scale.z);
                        reader.EndArray();
                    }
                    reader.EndObject();

                    // material
                    std::string materialName;
                    reader.Key("materialName");
                    reader.Value(materialName);
                    url = directory + "/material/" + materialName + ".json";
                    global::LoadMaterialRes(url);
                    subMesh.materialFile = url;
                }
                reader.EndObject();
            }
        }
        reader.EndArray();
    }
    reader.EndObject();
    
}

EntityModel::~EntityModel()
{
    if (mUBO) GPUFreeBuffer(mUBO);
    if (mSampler) GPUFreeSampler(mSampler);
    if (mSet) GPUFreeDescriptorSet(mSet);
    if (mShadowMapSet) GPUFreeDescriptorSet(mShadowMapSet);
    if (mRootSignature) GPUFreeRootSignature(mRootSignature);
    if (mPbrPipeline) GPUFreeRenderPipeline(mPbrPipeline);
}

void EntityModel::UploadRenderResource(SkyBox* skyBox)
{
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

    for (size_t i = 0; i < mMeshComp.rawMeshes.size(); i++)
    {
        std::string& meshFile     = mMeshComp.rawMeshes[i].meshFile;
        std::string& materialFile = mMeshComp.rawMeshes[i].materialFile;
        global::LoadGpuMeshRes(meshFile, mDevice, mGfxQueue);
        global::LoadGpuMaterialRes(materialFile, mDevice, mGfxQueue, mRootSignature, mSampler);
    }
}

void EntityModel::UpdateShadowMapSet(GPUTextureViewID shadowMap, GPUSamplerID sampler)
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

void EntityModel::Draw(GPURenderPassEncoderID encoder, const class Camera* cam, const glm::vec4& viewPos,  const class CascadeShadowPass* shadowPass)
{
    PointLight pointLight = {
        .position  = glm::vec3(-1.0f, 0.0f, 0.0f),
        .color     = glm::vec3(0.0f, 1.0f, 0.0f),
        .constant  = 1.0f,
        .linear    = 0.045f,
        .quadratic = 0.0075f
    };
    //update uniform bffer
    PerframeUniformBuffer ubo = {
        .view                       = cam->matrices.view,
        .proj                       = cam->matrices.perspective,
        .viewPos                    = viewPos,
        .directionalLight.direction = glm::vec3(-0.5f, 0.5f, 0.f),
        .directionalLight.color     = glm::vec3(1.0, 1.0, 1.0),
        .pointLight                 = pointLight
    };
    for (uint32_t i = 0; i < CascadeShadowPass::sCascadeCount; i++)
    {
        ubo.cascadeSplits[i * 4] = shadowPass->cascades[i].splitDepth;
        ubo.lightSpaceMat[i]     = shadowPass->cascades[i].viewProjMatrix;
    }
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
    struct
    {
        glm::mat4 model;
        float offsets[8];
    } push;
    push.model = mModelMatrix;
    for (uint32_t i = 0; i < 8; i++)
    {
        push.offsets[i] = 50.f * i;
    }
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
            GPURenderEncoderPushConstant(encoder, mRootSignature, &push);
            GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mesh->indexOffset, mesh->vertexOffset, 0);
        }
    }
}