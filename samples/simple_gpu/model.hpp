#pragma once
#include <string_view>
#include <vector>
#include <cmath>
#include <array>
#include <glm/vec4.hpp>

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
    // j [0, pi],  i [0, 2pi]
    // x = r * sin(j) * cos(i); y = r * cos(j); z = r * sin(j) * sin(i)

    static std::array<float, 3> Normalize(float x, float y, float z)
    {
        float length = sqrt(x * x + y * y + z * z);
        float div = 1.f / length;
        x *= div;
        y *= div;
        z *= div;
        return {x, y, z};
    }
    static constexpr const uint32_t SPHERE_DIV = 64;
    static constexpr const float PI            = 3.14159265359f;
    static std::vector<Vertex> GenSphereVertices()
    {
        std::vector<Vertex> vertices;
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
                std::array<float, 3> n = Normalize(v.x, v.y, v.z);
                v.u = std::atan2(n[0], n[2]) / (2 * PI) + 0.5f;
                v.v = n[1] * 0.5f + 0.5f;
                vertices.emplace_back(v);
            }
        }
        return vertices;
    }

    static std::vector<uint32_t> GenSphereIndices()
    {
        std::vector<uint32_t> indices;
        for (uint32_t j = 0; j < SPHERE_DIV; j++)
        {
          for (uint32_t i = 0; i < SPHERE_DIV; i++)
          {
            uint32_t p1 = j * (SPHERE_DIV + 1) + i;
            uint32_t p2 = p1 + (SPHERE_DIV + 1);
            /*
              p1     p1+1
                |``/|
                | / |
                |/__|
              p2     p2+1
            */
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

struct PBRMaterialParam
{
    float metallic;
    float roughness;
    float ao;
    float padding;
};

constexpr const int MAX_LIGHT_NUM = 4;
struct LightParam
{
    glm::vec4 lightColor[MAX_LIGHT_NUM];
    glm::vec4 lightPos[MAX_LIGHT_NUM];
};