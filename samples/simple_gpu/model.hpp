#pragma once
#include <string_view>
#include <vector>
#include <cmath>

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t materialIndex;
};

class Model
{
public:
    Model(const std::string_view file);
    ~Model();

private:
    void LoadModel(const std::string_view file);

private:
    std::vector<Mesh> mMeshes;
};

struct Sphere
{
    static std::vector<Vertex> GenSphereVertices()
    {
        std::vector<Vertex> vertices;
        const float PI      = 3.14159265359f;
        uint32_t SPHERE_DIV = 10;
        // Generate coordinates
        for (uint32_t j = 0; j <= SPHERE_DIV; j++)
        {
            float aj = j * PI / SPHERE_DIV;
            float sj = std::sin(aj);
            float cj = std::cos(aj);
            for (uint32_t i = 0; i <= SPHERE_DIV; i++)
            {
                float ai = i * 2 * PI / SPHERE_DIV;
                float si = std::sin(ai);
                float ci = std::cos(ai);

                Vertex v;
                v.x = si * sj; // X
                v.y = cj;      // Y
                v.z = ci * sj; // Z
                v.nx = v.x;
                v.ny = v.y;
                v.nz = v.z;
                vertices.emplace_back(v);
            }
        }
        return vertices;
    }

    static std::vector<uint32_t> GenSphereIndices()
    {
        std::vector<uint32_t> indices;
        uint32_t SPHERE_DIV = 10;
        for (uint32_t j = 0; j < SPHERE_DIV; j++)
        {
          for (uint32_t i = 0; i < SPHERE_DIV; i++)
          {
            uint32_t p1 = j * (SPHERE_DIV + 1) + i;
            uint32_t p2 = p1 + (SPHERE_DIV + 1);

            indices.push_back(p1);
            indices.push_back(p2);
            indices.push_back(p1 + 1);

            indices.push_back(p1 + 1);
            indices.push_back(p2);
            indices.push_back(p2 + 1);
          }
        }
        return indices;
    }

    static std::vector<Vertex> GenCubeVertices()
    {
        std::vector<Vertex> vertices = {
            { -0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 },
            { 0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0 },
            { 0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 1.0 },
            { -0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 1.0 },
            { 0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0 },
            { -0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 0.0 },
            { -0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 1.0 },
            { 0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 1.0 },
            { -0.5, -0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 0.0 },
            { -0.5, 0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 1.0 },
            { -0.5, 0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 1.0 },
            { -0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 0.0 },
            { 0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 1.0 },
            { 0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0 },
            { -0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0 },
            { 0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 1.0, 1.0 },
            { -0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 0.0, 1.0 },
            { -0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 0.0 },
            { 0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0 },
            { 0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 1.0 },
            { -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0 }
        };

        return vertices;
    }

    static std::vector<uint32_t> GenCubeIndices()
    {
        std::vector<uint32_t> indeces = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };
        return indeces;
    }
};