#include "model.hpp"
#include <fstream>
#include <sstream>
#include "Utils/Json/JsonReader.h"

Model::Model(const std::string_view file)
{
    LoadModel(file);
}

Model::~Model()
{

}

void Model::LoadModel(const std::string_view file)
{
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
    
    reader.StartObject();
    {
        reader.Key("mMeshes");
        size_t meshCount = 0;
        reader.StartArray(&meshCount);
        {
            mMeshes.reserve(meshCount);
            for (size_t m = 0; m < meshCount; m++)
            {
                Mesh mesh{};

                reader.StartObject();
                {
                    reader.Key("mVertices");
                    size_t count = 0;
                    reader.StartArray(&count);
                    mesh.vertices.reserve(count);
                    for (size_t i = 0; i < count; i++)
                    {
                        if (i == 13381)
                        {
                            int a = 0;
                        }
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
                        mesh.vertices.emplace_back(v);
                    }
                    reader.EndArray();

                    reader.Key("mIndices");
                    count = 0;
                    reader.StartArray(&count);
                    mesh.indices.reserve(count);
                    for (size_t i = 0; i < count; i++)
                    {
                        uint32_t val = 0;
                        reader.Value(val);
                        mesh.indices.push_back(val);
                    }
                    reader.EndArray();
                }
                reader.EndObject();

                mMeshes.emplace_back(mesh);
            }
        }
        reader.EndArray();
    }
    reader.EndObject();
}