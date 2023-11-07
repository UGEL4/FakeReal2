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

                    /* auto aabbPair = global::g_cache_mesh_bounding_box.find(subMesh.meshFile);
                    if (aabbPair != global::g_cache_mesh_bounding_box.end()) mAABB.Merge(aabbPair->second); */
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