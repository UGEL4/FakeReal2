#include "model_export.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <format>

const aiScene* g_scene = nullptr;

std::vector<MeshData> g_meshes;

void ExportModel(std::string_view file, std::string_view outFile)
{
    g_meshes.clear();
    g_scene =  aiImportFile(file.data(), aiProcess_CalcTangentSpace | aiProcess_Triangulate);
    ProcessNode(g_scene->mRootNode);
    SaveFile(outFile);
}

void ProcessNode(const aiNode* node)
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        auto mesh = g_scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i]);
    }
}

void ProcessMesh(const aiMesh* mesh)
{
    MeshData meshData {};
    meshData.vertices.clear();
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex v {};
        v.x = mesh->mVertices[i].x;
        v.y = mesh->mVertices[i].y;
        v.z = mesh->mVertices[i].z;

        v.nx = mesh->mNormals[i].x;
        v.ny = mesh->mNormals[i].y;
        v.nz = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0])
        {
            v.u = mesh->mTextureCoords[0][i].x;
            v.v = mesh->mTextureCoords[0][i].y;

            v.tx = mesh->mTangents[i].x;
            v.ty = mesh->mTangents[i].y;
            v.tz = mesh->mTangents[i].z;

            v.btx = mesh->mBitangents[i].x;
            v.bty = mesh->mBitangents[i].y;
            v.btz = mesh->mBitangents[i].z;
        }
        else
        {
            v.u = 0.f;
            v.v = 0.f;
        }

        meshData.vertices.emplace_back(v);
    }

    meshData.indices.clear();
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        for (uint32_t idx = 0; idx < mesh->mFaces[i].mNumIndices; idx++)
        {
            meshData.indices.push_back(mesh->mFaces[i].mIndices[idx]);
        }
    }

    if (mesh->mMaterialIndex >= 0)
    {
        meshData.materialIndex = mesh->mMaterialIndex;
        auto material = g_scene->mMaterials[mesh->mMaterialIndex];
        LoadTexture(material, aiTextureType_DIFFUSE, meshData.diffuse, "texture_diffuse");
        LoadTexture(material, aiTextureType_SPECULAR, meshData.specular, "texture_specular");
        LoadTexture(material, aiTextureType_METALNESS, meshData.metallic, "texture_metallic");
        LoadTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, meshData.roughness, "texture_roughness");
        LoadTexture(material, aiTextureType_NORMALS, meshData.normal, "texture_normal");
    }

    g_meshes.emplace_back(meshData);
}

void LoadTexture(const aiMaterial* material, aiTextureType type, std::vector<TextureDate>& tex, std::string_view typeName)
{
    uint32_t count = material->GetTextureCount(type);
    tex.reserve(count);
    for (uint32_t i = 0; i <count; i++)
    {
        aiString path;
        material->GetTexture(type, i, &path);
        TextureDate t {};
        t.url      = path.C_Str();
        t.typeName = typeName;
        tex.emplace_back(t);
    }
}

void SaveFile(const std::string_view filePath)
{
    std::stringstream out;
    out << "{\n"
           "    \"mMeshes\": [\n";
    for (size_t i = 0; i < g_meshes.size(); i++)
    {
        //mesh begin

        //vertices
        out << "        {\n"
               "            \"mVertices\": [\n";
        
        for (size_t vn = 0; vn < g_meshes[i].vertices.size(); vn++)
        {
            out << "                {";
            auto& v = g_meshes[i].vertices[vn];
            out << std::format("\"x\":{:f}, \"y\":{:f}, \"z\":{:f}, \"nx\":{:f}, \"ny\":{:f}, \"nz\":{:f}, \"tx\":{:f}, \"ty\":{:f}, \"tanx\":{:f}, \"tany\":{:f}, \"tanz\":{:f}, \"btanx\":{:f}, \"btany\":{:f}, \"btanz\":{:f}", 
            v.x, v.y, v.z, v.nx, v.ny, v.nz, v.u, v.v, v.tx, v.ty, v.tz, v.btx, v.bty, v.btz);
            if (vn == g_meshes[i].vertices.size() - 1)
            {
                out << '}';
            }
            else
            {
                out << "},\n";
            }
        }
        out << "\n            ],\n";

        //indices
        out <<"            \"mIndices\": [\n";
        size_t c = 1;
        for (size_t idx = 0; idx < g_meshes[i].indices.size(); idx++)
        {
            if (c == 1) out <<  "                " ;
            if (idx < g_meshes[i].indices.size() - 1)
            {
                if ((c % 3) == 0)
                {
                    out << g_meshes[i].indices[idx] << ",\n";
                    c = 1;
                }
                else
                {
                    out << g_meshes[i].indices[idx] << ',';
                    c++;
                }
            }
            else
            {
                out << g_meshes[i].indices[idx];
                c++;
            }
        }
        out << "\n            ],\n";

        //texture
        out <<"            \"mTexture\": {\n";
        out << "                \"diffuse\":{\n";
        if (g_meshes[i].diffuse.size())
        {
            out << "                    \"url\":\"" << g_meshes[i].diffuse[0].url << "\",\n";
            out << "                    \"type\":\"" << g_meshes[i].diffuse[0].typeName << "\"";
        }
        out << "\n                },\n";

        out << "                \"specular\":{\n";
        if (g_meshes[i].specular.size())
        {
            out << "                    \"url\":\"" << g_meshes[i].specular[0].url << "\",\n";
            out << "                    \"type\":\"" << g_meshes[i].specular[0].typeName << "\"";
        }
        out << "\n                },\n";
        
        out << "                \"metallic\":{\n";
        if (g_meshes[i].metallic.size())
        {
            out << "                    \"url\":\"" << g_meshes[i].metallic[0].url << "\",\n";
            out << "                    \"type\":\"" << g_meshes[i].metallic[0].typeName << "\"";
        }
        out << "\n                },\n";

        out << "                \"roughness\":{\n";
        if (g_meshes[i].roughness.size())
        {
            out << "                    \"url\":\"" << g_meshes[i].roughness[0].url << "\",\n";
            out << "                    \"type\":\"" << g_meshes[i].roughness[0].typeName << "\"";
        }
        out << "\n                },\n";
        
        out << "                \"normal\":{\n";
        if (g_meshes[i].normal.size())
        {
            out << "                    \"url\":\"" << g_meshes[i].normal[0].url << "\",\n";
            out << "                    \"type\":\"" << g_meshes[i].normal[0].typeName << "\"";
        }
        out << "\n                }";
        
        out << "\n            },";
        //texture

        out <<"\n            \"mMaterialIndex\": " << g_meshes[i].materialIndex;

        //mesh end
        if (i == g_meshes.size() - 1)
        {
            out << "\n        }";
        }
        else
        {
            out << "\n        },\n";
        }
    }
    out << "\n    ]\n}";

    std::ofstream outFile(filePath.data());
    if (outFile.is_open())
    {
        outFile << out.rdbuf();
        outFile.close();
    }
}