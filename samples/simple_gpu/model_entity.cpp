#include "model_entity.hpp"
#include <fstream>
#include "Utils/Json/JsonReader.h"
#include <filesystem>
#include "global_resources.hpp"
#include "cascade_shadow_pass.hpp"

EntityModel::EntityModel(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue)
: mDevice(device), mGfxQueue(gfxQueue)
{
    //mTransformComp.transform.scale = math::Vector3(0.02f, 0.02f, 0.02f);
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

                    auto aabbPair = global::g_cache_mesh_bounding_box.find(subMesh.meshFile);
                    if (aabbPair != global::g_cache_mesh_bounding_box.end())
                    {
                        TransformComponent comp;
                        comp.transform = subMesh.transform;
                        mAABB.Merge(BoundingBox::BoundingBoxTransform(aabbPair->second, mTransformComp.GetMatrix() * comp.GetMatrix()));
                    }
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
    //if (mUBO) GPUFreeBuffer(mUBO);
    if (mSampler) GPUFreeSampler(mSampler); mSampler = nullptr;
}

void EntityModel::UploadRenderResource()
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
