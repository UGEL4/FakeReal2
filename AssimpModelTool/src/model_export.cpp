#include "model_export.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>
#include <format>
#include <iostream>
#include <assimp/Exporter.hpp>
const aiScene* g_scene = nullptr;

std::vector<MeshData> g_meshes;
std::vector<Materials> g_materials;
std::unordered_map<aiMesh*, MeshData> g_meshs_map;
std::vector<MeshComp> g_mesh_comp;
uint32_t node_count = 0;

void ExportModel(std::string_view file, std::string_view outFile)
{
    g_meshes.clear();
    uint32_t flags = aiProcess_CalcTangentSpace |
                   aiProcess_Triangulate |
                   aiProcess_RemoveRedundantMaterials;
    g_scene =  aiImportFile(file.data(), flags);
    std::cout << "num of mesh : " << g_scene->mNumMeshes <<std::endl;
    ProcessMaterials();
    for (uint32_t i = 0; i < g_scene->mNumMeshes; i++)
    {
        ProcessMesh(g_scene->mMeshes[i]);
    }
    ProcessNode(g_scene->mRootNode);
    SaveFile(outFile);
    aiReleaseImport(g_scene);
    Assimp::Exporter ex;
}

void ProcessNode(const aiNode* node)
{
    aiString name = node->mName;
    uint32_t a = node->mNumMeshes;
    uint32_t b = node->mNumChildren;
    //std::cout << "name: " << name.C_Str() << ", num: " << a << std::endl; 
    /* for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        auto mesh = g_scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh);
    } */
    if (node->mNumMeshes > 0)
    {
        auto mesh = g_scene->mMeshes[node->mMeshes[0]];
        MeshComp comp;
        comp.url = mesh->mName.C_Str();
        comp.materialIndex = mesh->mMaterialIndex;
        aiMatrix4x4 localMat = node->mTransformation;
        aiVector3D scale, position;
        aiQuaternion rotation;
        localMat.Decompose(scale, rotation, position);
        comp.transform.position = FakeReal::math::Vector3(position.x, position.y, position.z);
        comp.transform.scale    = FakeReal::math::Vector3(scale.x, scale.y, scale.z);
        comp.transform.rotation = FakeReal::math::Quaternion(rotation.w, rotation.x, rotation.y, rotation.z);
        comp.id = node_count;
        node_count++;
        g_mesh_comp.push_back(comp);
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
    g_meshs_map.insert(std::make_pair(const_cast<aiMesh*>(mesh), meshData));
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
    "Sponza_Fabric_roughness.png",
    "Sponza_Fabric_roughness.png",
    "Sponza_Fabric_roughness.png",
    "Sponza_Curtain_roughness.png",
    "Sponza_Curtain_roughness.png",
    "Sponza_Curtain_roughness.png",
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
        if(SponzaRoughnessMaps[i])
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
    std::filesystem::path file(filePath);
    std::cout << file.parent_path().generic_string() << std::endl;
    std::string directory = file.has_parent_path() ? file.parent_path().generic_string() + "/" : "";
    std::stringstream out;
    out << "{\n"
           "    \"mMeshes\": [\n";
    //for (size_t i = 0; i < g_meshes.size(); i++)
    size_t i = 0;
    for (auto& iter : g_meshs_map)
    {
        //mesh begin

        const MeshData& mesh = iter.second;

        //vertices
        out << "        {\n"
               "            \"mVertices\": [\n";
        
        for (size_t vn = 0; vn < mesh.vertices.size(); vn++)
        {
            out << "                {";
            auto& v = mesh.vertices[vn];
            out << std::format("\"x\":{:f}, \"y\":{:f}, \"z\":{:f}, \"nx\":{:f}, \"ny\":{:f}, \"nz\":{:f}, \"tx\":{:f}, \"ty\":{:f}, \"tanx\":{:f}, \"tany\":{:f}, \"tanz\":{:f}, \"btanx\":{:f}, \"btany\":{:f}, \"btanz\":{:f}", 
            v.x, v.y, v.z, v.nx, v.ny, v.nz, v.u, v.v, v.tx, v.ty, v.tz, v.btx, v.bty, v.btz);
            if (vn == mesh.vertices.size() - 1)
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
        for (size_t idx = 0; idx < mesh.indices.size(); idx++)
        {
            if (c == 1) out <<  "                " ;
            if (idx < mesh.indices.size() - 1)
            {
                if ((c % 3) == 0)
                {
                    out << mesh.indices[idx] << ",\n";
                    c = 1;
                }
                else
                {
                    out << mesh.indices[idx] << ',';
                    c++;
                }
            }
            else
            {
                out << mesh.indices[idx];
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

        out <<"\n            \"mMaterialIndex\": " << mesh.materialIndex;

        //mesh end
        if (i == g_meshs_map.size() - 1)
        {
            out << "\n        }";
        }
        else
        {
            out << "\n        },\n";
        }
        i++;
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
                << "\"" << directory + mat.textures[i].url << "\", ";
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