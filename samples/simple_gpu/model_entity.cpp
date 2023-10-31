#include "model_entity.hpp"
#include <fstream>
#include "Utils/Json/JsonReader.h"

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