#include "model_export.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

const aiScene* g_scene = nullptr;

std::vector<MeshData> g_meshes;

void ExportModel(std::string_view file)
{
    g_meshes.clear();
    g_scene =  aiImportFile(file.data(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Triangulate);
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
        auto material = g_scene->mMaterials[mesh->mMaterialIndex];
        LoadTexture(material, aiTextureType_DIFFUSE, meshData.diffuse, "texture_diffuse");
        LoadTexture(material, aiTextureType_SPECULAR, meshData.specular, "texture_specular");
        LoadTexture(material, aiTextureType_METALNESS, meshData.specular, "texture_metalness");
        LoadTexture(material, aiTextureType_DIFFUSE_ROUGHNESS, meshData.specular, "texture_roughness");
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