#include "model_entity.hpp"
#include <fstream>
#include "Utils/Json/JsonReader.h"
#include <filesystem>
#include "global_resources.hpp"

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