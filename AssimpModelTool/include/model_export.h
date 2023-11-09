#pragma once

#include <string_view>
#include <vector>
#include "assimp/material.h"
#include "Math/Transform.h"
#include <unordered_map>

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float tx, ty, tz;
    float btx, bty, btz;

    bool operator ==(const Vertex& other) const
    {
        return x == other.x && y == other.y && z == other.z && nx == other.nx && ny == other.ny && nz == other.nz && u == other.u && v == other.v;
    }
};

struct TextureDate
{
    std::string url;
    std::string typeName;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<TextureDate> diffuse;
    std::vector<TextureDate> specular;
    std::vector<TextureDate> metallic;
    std::vector<TextureDate> roughness;
    std::vector<TextureDate> normal;
    uint32_t materialIndex;
    std::string meshName;
};

struct Materials
{
    uint32_t index;
    std::string name;
    std::vector<TextureDate> textures;
};

struct MeshComp
{
    std::string url;
    FakeReal::math::Transform transform;
    std::string materialName;
    uint32_t materialIndex;
    int32_t id;
    int32_t parentId;
};

void ExportModel(std::string_view file, std::string_view outFile);
void ProcessNode(const struct aiNode* node);
void ProcessMesh(const struct aiMesh* mesh);
aiMatrix4x4 GetNodeTransform(const struct aiNode* node);
void ProcessMaterials();
void LoadTexture(const aiMaterial* material, aiTextureType type, std::vector<TextureDate>& tex, std::string_view typeName);
void SaveFile(const std::string_view filePath);
