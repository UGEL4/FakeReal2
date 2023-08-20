#pragma once

#include <string_view>
#include <vector>
#include "assimp/material.h"

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    float tx, ty, tz;
    float btx, bty, btz;
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
};

void ExportModel(std::string_view file, std::string_view outFile);
void ProcessNode(const struct aiNode* node);
void ProcessMesh(const struct aiMesh* mesh);
void LoadTexture(const aiMaterial* material, aiTextureType type, std::vector<TextureDate>& tex, std::string_view typeName);
void SaveFile(const std::string_view filePath);
