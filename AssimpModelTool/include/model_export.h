#pragma once

#include <string_view>
#include <vector>
#include "assimp/material.h"

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct TextureDate
{
    std::string_view url;
    std::string_view typeName;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<TextureDate> diffuse;
    std::vector<TextureDate> specular;
    std::vector<TextureDate> metalness;
    std::vector<TextureDate> roughness;
};

void ExportModel(std::string_view file);
void ProcessNode(const struct aiNode* node);
void ProcessMesh(const struct aiMesh* mesh);
void LoadTexture(const aiMaterial* material, aiTextureType type, std::vector<TextureDate>& tex, std::string_view typeName);
