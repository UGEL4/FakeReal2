#include "model_export.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <format>
#include <iostream>

const aiScene* g_scene = nullptr;

std::vector<MeshData> g_meshes;
std::vector<Materials> g_materials;

void ExportModel(std::string_view file, std::string_view outFile)
{
    g_meshes.clear();
    uint32_t flags = aiProcess_CalcTangentSpace |
                   aiProcess_Triangulate |
                   aiProcess_RemoveRedundantMaterials;
    g_scene =  aiImportFile(file.data(), flags);
    std::cout << "num of mesh : " << g_scene->mNumMeshes <<std::endl;
    ProcessMaterials();
    ProcessNode(g_scene->mRootNode);
    SaveFile(outFile);
    aiReleaseImport(g_scene);
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
    
    meshData.materialIndex = mesh->mMaterialIndex;

    if (mesh->mMaterialIndex >= 0)
    {
        meshData.materialIndex = mesh->mMaterialIndex;
        /* auto material = g_scene->mMaterials[mesh->mMaterialIndex];
        LoadTexture(material, aiTextureType_DIFFUSE, meshData.diffuse, "texture_diffuse");
        LoadTexture(material, aiTextureType_SPECULAR, meshData.specular, "texture_specular");
        LoadTexture(material, aiTextureType_METALNESS, meshData.metallic, "texture_metallic");
        LoadTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, meshData.roughness, "texture_roughness");
        LoadTexture(material, aiTextureType_NORMALS, meshData.normal, "texture_normal"); */
    }

    g_meshes.emplace_back(meshData);
}

// For some reason the roughness maps aren't coming through in the SHININESS channel after Assimp import. :(
static const char* SponzaRoughnessMaps[] = {
    "Sponza_Thorn_roughness.png",
    "VasePlant_roughness.png",
    "VaseRound_roughness.png",
    "Background_Roughness.png",
    "Sponza_Bricks_a_Roughness.png",
    "Sponza_Arch_roughness.png",
    "Sponza_Ceiling_roughness.png",
    "Sponza_Column_a_roughness.png",
    "Sponza_Floor_roughness.png",
    "Sponza_Column_c_roughness.png",
    "Sponza_Details_roughness.png",
    "Sponza_Column_b_roughness.png",
    nullptr,
    "Sponza_FlagPole_roughness.png",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "ChainTexture_Roughness.png",
    "VaseHanging_roughness.png",
    "Vase_roughness.png",
    "Lion_Roughness.png",
    "Sponza_Roof_roughness.png"
};

void ProcessMaterials()
{
    uint32_t materialNum = g_scene->mNumMaterials;
    g_materials.reserve(materialNum);
    for (uint32_t i = 0; i < materialNum; i++)
    {
        auto& material = g_materials.emplace_back();
        material.index = i;
        
        aiMaterial* mat = g_scene->mMaterials[i];
        aiString diffuseTexPath;
        if(mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexPath) == aiReturn_SUCCESS)
        {
            auto&& tex = material.textures.emplace_back();
            tex.url = diffuseTexPath.C_Str();
            tex.typeName = "diffuse";
        }

        aiString normalMapPath;
        if(mat->GetTexture(aiTextureType_NORMALS, 0, &normalMapPath) == aiReturn_SUCCESS
           || mat->GetTexture(aiTextureType_HEIGHT, 0, &normalMapPath) == aiReturn_SUCCESS)
        {
            auto&& tex = material.textures.emplace_back();
            tex.url = normalMapPath.C_Str();
            tex.typeName = "normal";
        }

        aiString roughnessMapPath;
        //if(mat->GetTexture(aiTextureType_SHININESS, 0, &roughnessMapPath) == aiReturn_SUCCESS)
        {
            auto&& tex = material.textures.emplace_back();
            tex.url = (const char*)SponzaRoughnessMaps[i];
            tex.typeName = "roughness";
        }

        // For some reason the roughness maps aren't coming through in the SHININESS channel after Assimp import. :(
        /* if(SponzaRoughnessMaps[i] != nullptr)
            material.TextureNames[uint64(MaterialTextures::Roughness)] = GetFileName(SponzaRoughnessMaps[i]); */

        aiString metallicMapPath;
        if(mat->GetTexture(aiTextureType_AMBIENT, 0, &metallicMapPath) == aiReturn_SUCCESS)
        {
            auto&& tex = material.textures.emplace_back();
            tex.url = metallicMapPath.C_Str();
            tex.typeName = "metallic";
        }
    }
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
        /* out <<"            \"mTexture\": {\n";
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
        
        out << "\n            },"; */
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
    out << "\n    ],\n";

    size_t c = 0;
    out << "    \"mMaterials\": [\n";
    for (const auto& mat : g_materials)
    {
        out << "        {\n           \"index\":" << mat.index << ",\n";
        out << "           \"textures\":[\n";
        for (size_t i = 0; i < mat.textures.size(); i++)
        {
            out << "               {";
            out << "\"name\":"
                << "\"" << "C:/Dev/FakeReal2/asset/objects/sponza/" + mat.textures[i].url << "\", ";
            out << "\"type\":"
                << "\"" << mat.textures[i].typeName << "\"";
            if (i < (mat.textures.size() - 1))
            {
                out << "},\n";
            }
            else
            {
                out << "}\n";
            }
        }
        out << "           ]\n";
        if (c < (g_materials.size() - 1))
        {
            out << "        },\n";
        }
        else
        {
            out << "        }\n";
        }
        c++;
    }
    out << "\n    ]\n}";

    std::ofstream outFile(filePath.data());
    if (outFile.is_open())
    {
        outFile << out.rdbuf();
        outFile.close();
    }
}